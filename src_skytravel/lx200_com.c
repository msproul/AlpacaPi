//*****************************************************************************
//*		lx200_com.c		(c) 2021 by Mark Sproul
//*
//*	Description:	Routines to talk to LX200 via TCP/IP
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.  You
//*	must determine the suitability of this source code for your use.
//*
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jan  1,	2021	<MLS> Created lx200_com.c
//*	Jan  1,	2021	<MLS> Using a thread to handle LX2000 communications
//*	Jan  2,	2021	<MLS> Fixed 0->24 hour bug in handling of Right Ascension
//*	Jan  3,	2021	<MLS> Added error msg to LX200_StartThread()
//*	Jan  3,	2021	<MLS> Added bounds checking to RA/DEC values rcvd from LX200
//*	Jan  3,	2021	<MLS> Added LX200_SyncScope()
//*	Jan  3,	2021	<MLS> Sync working with TSC telescope controller
//*****************************************************************************
//*	reference
//*		https://astro-physics.info/index.htm?tech_support/tech_support
//*		https://astro-physics.info/tech_support/mounts/command_lang.htm
//*		https://astro-physics.info/tech_support/mounts/protocol-d.pdf
//*		https://astro-physics.info/tech_support/mounts/protocol-cp3-cp4.pdf
//*****************************************************************************

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<stdbool.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<netinet/in.h>
#include	<errno.h>
#include	<ctype.h>
#include	<pthread.h>
#include	<math.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"lx200_com.h"


//*****************************************************************************
typedef struct
{
	char	cmdString[32];
} TYPE_LX200CmdQue;

#define	kMaxCmds	16
TYPE_LX200CmdQue	gLX200CmdQueue[kMaxCmds];
int					gLX200CmdQueCnt		=	0;

bool	gTelescopeUpdated				=	false;
bool	gTelescopeInfoValid				=	false;
double	gTelescopeRA_Radians			=	1.0;
double	gTelescopeDecl_Radians			=	1.0;

char	gTelescopeRA_String[64]			=	"";
char	gTelescopeDecl_String[64]		=	"";
int		gTelescopeUpdateCnt				=	0;
char	gTelescopeErrorString[64]		=	"";

int		gLX200_SocketErrCnt				=	0;
bool	gLX200_ThreadActive				=	false;
int		gLX200_OutOfBoundsCnt			=	0;

static bool			gLX200_keepRunning	=	false;
static pthread_t	gLX200threadID;


//*****************************************************************************
//*	returns a socket description
//*****************************************************************************
int	LX200_OpenSocket(	struct sockaddr_in	*deviceAddress,
						const int			port)
{
int					socket_desc;
struct sockaddr_in	remoteDev;
int					connRetCode;
char				ipString[32];
struct timeval		timeoutLength;
int					setOptRetCode;

	CONSOLE_DEBUG(__FUNCTION__);
	socket_desc	=	socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc >= 0)
	{
		//*	set a timeout
		timeoutLength.tv_sec	=	0;
		timeoutLength.tv_usec	=	500000;
		setOptRetCode			=	setsockopt(	socket_desc,
												SOL_SOCKET,
												SO_RCVTIMEO,
												&timeoutLength,
												sizeof(timeoutLength));
		if (setOptRetCode < 0)
		{
			perror("setsockopt(SO_RCVTIMEO) failed");
		}
		remoteDev.sin_addr.s_addr	=	deviceAddress->sin_addr.s_addr;
		remoteDev.sin_family		=	AF_INET;
		remoteDev.sin_port			=	htons(port);
		//*	Connect to remote device
		connRetCode	=	connect(socket_desc , (struct sockaddr *)&remoteDev , sizeof(remoteDev));
		if (connRetCode >= 0)
		{

			CONSOLE_DEBUG("Open successful");

		}
		else if (errno == ECONNREFUSED)
		{
			inet_ntop(AF_INET, &deviceAddress->sin_addr.s_addr, ipString, INET_ADDRSTRLEN);
			sprintf(gTelescopeErrorString, "Connection refused - %s", ipString);
			CONSOLE_DEBUG(gTelescopeErrorString);

			gLX200_keepRunning	=	false;
			gTelescopeUpdated	=	true;
		}
		else
		{
			inet_ntop(AF_INET, &deviceAddress->sin_addr.s_addr, ipString, INET_ADDRSTRLEN);

			sprintf(gTelescopeErrorString, "Connection error, ipaddress = %s", ipString);
			CONSOLE_DEBUG(gTelescopeErrorString);
			CONSOLE_DEBUG_W_NUM("errno\t\t\t=", errno);
			gLX200_keepRunning	=	false;
			gTelescopeUpdated	=	true;
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("socket_desc\t=", socket_desc);
		CONSOLE_DEBUG_W_NUM("errno\t\t=", errno);
	}
	return(socket_desc);
}

#define	kLX200ReadBuffLen		100

//*****************************************************************************
//*	returns # of bytes received
//*	<0 for error
//*****************************************************************************
int	LX200_SendCommand(int socket_desc, const char *cmdString, char *dataBuffer, unsigned int dataBufferLen)
{
char	xmitBuffer[32];
char	returnedData[kLX200ReadBuffLen];
bool	keepReading;
int		dataLen;
int		sendRetCode;
int		recvByteCnt;


	memset(dataBuffer, 0, dataBufferLen);

	strcpy(xmitBuffer, ":");
	strcat(xmitBuffer, cmdString);
	strcat(xmitBuffer, "#");
	dataLen			=	-1;

#ifndef _ENABLE_SKYTRAVEL_
	CONSOLE_DEBUG_W_STR("sending\t=",		xmitBuffer);
#endif // _ENABLE_SKYTRAVEL_

	sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , 0);
	if (sendRetCode >= 0)
	{
		keepReading		=	true;
		dataBuffer[0]	=	0;
		while (keepReading && ((strlen(dataBuffer) + kLX200ReadBuffLen) < dataBufferLen))
		{
		//	sleep(10);
			recvByteCnt	=	recv(socket_desc, returnedData , kLX200ReadBuffLen , 0);
			if (recvByteCnt > 0)
			{
				returnedData[recvByteCnt]	=	0;
			//	CONSOLE_DEBUG_W_STR("returnedData\t=",	returnedData);
				strcat(dataBuffer, returnedData);
			}
			else
			{
				keepReading		=	false;
			}
		}
		dataLen	=	strlen(dataBuffer);


	}
	else
	{
		CONSOLE_DEBUG_W_NUM("sendRetCode\t=", sendRetCode);
		strcpy(gTelescopeErrorString, "Socket failed to send");
		gTelescopeUpdated	=	true;
	}
#ifndef _ENABLE_SKYTRAVEL_
	CONSOLE_DEBUG_W_STR("dataBuffer\t=", dataBuffer);
#endif // _ENABLE_SKYTRAVEL_
	return(dataLen);
}



//*****************************************************************************
//*	returns a socket description
//*****************************************************************************
int	LX200_Connect(const char *ipAddressString, int port)
{
struct sockaddr_in	lx200DeviceAddr;
char				outputIPaddr[64];
int					socket_desc;

	inet_pton(AF_INET, ipAddressString, &(lx200DeviceAddr.sin_addr));
	inet_ntop(AF_INET, &(lx200DeviceAddr.sin_addr), outputIPaddr, INET_ADDRSTRLEN);
	CONSOLE_DEBUG_W_STR("outputIPaddr\t=",		outputIPaddr);

	socket_desc	=	LX200_OpenSocket(&lx200DeviceAddr, port);
	return(socket_desc);
}

//*****************************************************************************
static bool	CheckForValidResponse(const char *lx200ResponseString)
{
bool	isValid;
int		strLen;

	isValid	=	false;
	strLen		=	strlen(lx200ResponseString);
	if (strLen > 3)
	{
		//*	check for valid data
		if (lx200ResponseString[strLen-1] == '#')
		{
			//*	looking good
			isValid	=	true;
		}
	}
	return(isValid);
}

//*****************************************************************************
//	dataBuffer	= +40*58:56#
//*****************************************************************************
static double	LX200_ParseDegMinSec(char *dataBuffer)
{
int		degrees;
int		minutes;
int		seconds;
int		plusMinus;
char	*charPtr;
double	degrees_Dbl;
bool	isValid;

//	CONSOLE_DEBUG_W_STR("dataBuffer\t=",		dataBuffer);
	degrees		=	0;
	minutes		=	0;
	seconds		=	0;
	degrees_Dbl	=	0.0;
	plusMinus	=	1;
	isValid		=	CheckForValidResponse(dataBuffer);
	if (isValid)
	{
		//*	looking good
		charPtr	=	dataBuffer;
		if (*charPtr == '+')
		{
			plusMinus	=	1;
			charPtr++;
		}
		if (*charPtr == '-')
		{
			plusMinus	=	-1;
			charPtr++;
		}
		degrees	=	atoi(charPtr);
		while (isdigit(*charPtr))
		{
			charPtr++;
		}
		charPtr++;
		minutes	=	atoi(charPtr);
		while (isdigit(*charPtr))
		{
			charPtr++;
		}
		charPtr++;
		seconds	=	atoi(charPtr);

//			CONSOLE_DEBUG_W_NUM("degrees\t=",		degrees);
//			CONSOLE_DEBUG_W_NUM("minutes\t=",		minutes);
//			CONSOLE_DEBUG_W_NUM("seconds\t=",		seconds);

		degrees_Dbl	=	degrees;
		degrees_Dbl	+=	(1.0 * minutes) / 60.0;
		degrees_Dbl	+=	(1.0 * seconds) / 3600.0;

		degrees_Dbl	=	plusMinus * degrees_Dbl;
	}
	return(degrees_Dbl);
}

//*****************************************************************************
//*	process Right Ascension
//*****************************************************************************
static bool	LX200_Process_GR_RtAsc(char *dataBuffer)
{
double	degrees_Dbl;
bool	isValid;

	isValid		=	CheckForValidResponse(dataBuffer);
	if (isValid)
	{
		degrees_Dbl	=	LX200_ParseDegMinSec(dataBuffer);
	//	if ((degrees_Dbl >= 0.0) && (degrees_Dbl <= 24.0))
		if ((degrees_Dbl >= 0.0))
		{
			if (strlen(dataBuffer) < 32)
			{
				strcpy(gTelescopeRA_String, dataBuffer);
			}

			while (degrees_Dbl > 24.0)
			{
				degrees_Dbl	-=	24.0;
			}
			//*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//*	Important note:
			//*	rtAscension is 0->24 not 0->360 / 0->2_pi
			//*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			gTelescopeRA_Radians			=	(degrees_Dbl / 24.0) * (2.0 * M_PI);
		}
		else
		{
			gLX200_OutOfBoundsCnt++;
		}
	}
	return(isValid);
}

//*****************************************************************************
//*	process Declination
//*****************************************************************************
static bool	LX200_Process_GD(char *dataBuffer)
{
double	degrees_Dbl;

bool	isValid;

	isValid		=	CheckForValidResponse(dataBuffer);
	if (isValid)
	{
		degrees_Dbl	=	LX200_ParseDegMinSec(dataBuffer);
		if ((degrees_Dbl >= -90.0) && (degrees_Dbl <= 90.0))
		{
			if (strlen(dataBuffer) < 32)
			{
				strcpy(gTelescopeDecl_String, dataBuffer);
			}
			gTelescopeDecl_Radians	=	RADIANS(degrees_Dbl);
		}
		else
		{
			gLX200_OutOfBoundsCnt++;
		}
	}
	return(isValid);
}



//*****************************************************************************
static void	SendCmdsFromQueue(int socket_desc)
{
int		returnByteCNt;
char	returnBuffer[500];
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	while (gLX200CmdQueCnt > 0)
	{
		CONSOLE_DEBUG_W_STR("Sending", gLX200CmdQueue[0].cmdString);
		returnByteCNt	=	LX200_SendCommand(	socket_desc,
												gLX200CmdQueue[0].cmdString,
												returnBuffer,
												400);
		if (returnByteCNt > 0)
		{
			CONSOLE_DEBUG_W_STR("returnBuffer\t=", returnBuffer);
		}
		for (iii=0; iii<gLX200CmdQueCnt; iii++)
		{
			gLX200CmdQueue[iii]	=	gLX200CmdQueue[iii + 1];
		}
		gLX200CmdQueCnt--;
		usleep(100);
	}
}

//*****************************************************************************
static void	*LX200CommThread(void *arg)
{
int		socket_desc;
int		returnByteCNt;
char	dataBuffer[500];
int		shutDownRetCode;
int		closeRetCode;
bool	isValid;

	gLX200_ThreadActive	=	true;
	gLX200_SocketErrCnt	=	0;
	gLX200CmdQueCnt		=	0;
	socket_desc	=	LX200_Connect("192.168.1.104", 49152);
	if (socket_desc > 0)
	{
		while (gLX200_keepRunning && (gLX200_SocketErrCnt < 20))
		{
#ifndef _ENABLE_SKYTRAVEL_
			CONSOLE_DEBUG("---------------------------------------------");
#endif // _ENABLE_SKYTRAVEL_

			//*	if we have commands queued up, get them done first
			if (gLX200CmdQueCnt > 0)
			{
				SendCmdsFromQueue(socket_desc);
			}
			else
			{
				returnByteCNt	=	LX200_SendCommand(socket_desc, "GR", dataBuffer, 400);
				if (returnByteCNt > 0)
				{
					isValid			=	LX200_Process_GR_RtAsc(dataBuffer);
					if (isValid)
					{
						gTelescopeInfoValid	=	true;
					}
					else
					{
						strcpy(gTelescopeRA_String, "RA failed");
						gLX200_SocketErrCnt++;
						gTelescopeInfoValid	=	false;
					}
					usleep(1000);
				}

				returnByteCNt	=	LX200_SendCommand(socket_desc, "GD", dataBuffer, 400);
				if (returnByteCNt > 0)
				{
					isValid			=	LX200_Process_GD(dataBuffer);
					if (isValid)
					{
						gTelescopeInfoValid	=	true;
					}
					else
					{
						strcpy(gTelescopeDecl_String, "DEC failed");
						gLX200_SocketErrCnt++;
						gTelescopeInfoValid	=	false;
					}
					usleep(1000);
				}

				gTelescopeUpdateCnt++;
				gTelescopeUpdated		=	true;
				sleep(1);
			}

#ifndef _ENABLE_SKYTRAVEL_
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GC", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "Gc", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "Gd", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "Ge", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GF", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GG", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "Gg", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GL", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GM", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GN", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GO", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GP", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "Gr", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GT", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "Gt", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GVD", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GVN", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GVP", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GVT", dataBuffer, 400);
			returnByteCNt	=	LX200_SendCommand(socket_desc, "GZ", dataBuffer, 400);
#endif // _ENABLE_SKYTRAVEL_



			if (gLX200_OutOfBoundsCnt > 0)
			{
				CONSOLE_DEBUG_W_NUM("gLX200_OutOfBoundsCnt\t=", gLX200_OutOfBoundsCnt);
			}
		}
		CONSOLE_DEBUG("shutdown");

		shutDownRetCode	=	shutdown(socket_desc, SHUT_RDWR);
		if (shutDownRetCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("shutDownRetCode\t=", shutDownRetCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}
		CONSOLE_DEBUG("close");
		closeRetCode	=	close(socket_desc);
		if (closeRetCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("closeRetCode\t=", closeRetCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}
	}
	else
	{
		strcpy(gTelescopeErrorString, "Socket failed to open");
	}
	strcpy(gTelescopeRA_String, "Thread exit");
	gLX200_ThreadActive		=	false;
	gTelescopeUpdated		=	true;

	return(NULL);
}


//*****************************************************************************
//*	returns 0 on success, -1 if already running, thread error if there is an error
//*****************************************************************************
int LX200_StartThread(char *errorMsg)
{
int			threadErr;

	CONSOLE_DEBUG(__FUNCTION__);
	threadErr	=	-1;
	errorMsg[0]	=	0;

	//*	check to make sure a thread is not already running
	if (gLX200_ThreadActive == false)
	{
		gLX200_keepRunning	=	true;
		threadErr			=	pthread_create(&gLX200threadID, NULL, &LX200CommThread, NULL);
		if (threadErr != 0)
		{
			sprintf(errorMsg, "Error creating thread err=%d", threadErr);
			CONSOLE_DEBUG(errorMsg);
		}
	}
	else
	{
		strcpy(errorMsg, "LX200 Thread already running");
		CONSOLE_DEBUG(errorMsg);
		threadErr	=	-1;
	}
	return(threadErr);
}

//*****************************************************************************
void LX200_StopThread(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	gLX200_keepRunning	=	false;
}

//*****************************************************************************
static 	void	AddCmdToLX200queue(const char *cmdString)
{
	CONSOLE_DEBUG_W_STR("cmdString\t\t=", cmdString);
	if (gLX200CmdQueCnt < kMaxCmds)
	{
		strcpy(gLX200CmdQueue[gLX200CmdQueCnt].cmdString, cmdString);
		gLX200CmdQueCnt++;
	}
}

//*****************************************************************************
//*	Right Ascension is never negitive (0->24) and therefore does not need a sign
//*****************************************************************************
static void	FormatHHMMSS(const double argDegreeValue, char *timeString, bool includeSign)
{
double	myDegreeValue;
double	minutes_dbl;
double	seconds_dbl;
int		degrees;
int		minutes;
int		seconds;
char	signChar;

	signChar		=	'+';
	myDegreeValue	=	argDegreeValue;
	if (myDegreeValue < 0)
	{
		myDegreeValue	=	-argDegreeValue;
		signChar		=	'-';
	}
	degrees		=	myDegreeValue;
	minutes_dbl	=	myDegreeValue - (1.0 * degrees);
	minutes		=	minutes_dbl * 60.0;
	seconds_dbl	=	(minutes_dbl * 60) - (1.0 * minutes);
	seconds		=	seconds_dbl * 60;;

	if (includeSign)
	{
		sprintf(timeString, "%c%02d:%02d:%02d", signChar, degrees, minutes, seconds);
	}
	else
	{
		sprintf(timeString, "%02d:%02d:%02d", degrees, minutes, seconds);
	}
}

//*****************************************************************************
//*	:SrHH:MM.T#
//*	:SrHH:MM:SS#
//*	Set target object RA to HH:MM.T or HH:MM:SS depending on the current precision setting.
//*	Returns:
//*	0 – Invalid
//*	1 - Valid
//*
//*	:SdsDD*MM#
//*	Set target object declination to sDD*MM or sDD*MM:SS depending on the current precision setting
//*	Returns:
//*	1 - Dec Accepted
//*	0 – Dec invalid
//*
//*	:CM#
//*	Synchronizes the telescope's position with the currently selected database object's coordinates.
//*	Returns:
//*	LX200's - a "#" terminated string with the name of the object that was synced.
//*****************************************************************************
bool	LX200_SyncScope(const double newRtAscen_Radians, const double new_Declination_Radians, char *returnErrMsg)
{
double	myRtAscenDeg360;
double	myRtAscenDeg24;
double	declDegrees;
bool	successFlg;
char	timeString[32];
char	commandString[32];

	CONSOLE_DEBUG(__FUNCTION__);
	successFlg		=	false;

	//-------------------------------------------------
	//*	deal with the right ascension
	//*	remember, 0->2 Pi   ==>>  0 ->24 hours

	myRtAscenDeg360	=	DEGREES(newRtAscen_Radians);
	if (myRtAscenDeg360 < 0.0)
	{
		myRtAscenDeg360	+=	360.0;
	}
	myRtAscenDeg24	=	(24.0 / 360.0) * myRtAscenDeg360;

	FormatHHMMSS(myRtAscenDeg24, timeString, false);

	//*	create the LX200 command SrHH:MM:SS
	strcpy(commandString, "Sr");
	strcat(commandString, timeString);
	AddCmdToLX200queue(commandString);

	//-------------------------------------------------
	//*	now deal with the declination, its easy
	declDegrees		=	DEGREES(new_Declination_Radians);

	FormatHHMMSS(declDegrees, timeString, true);

	//*	create the LX200 command SdsDD*MM
	strcpy(commandString, "Sd");
	strcat(commandString, timeString);
	AddCmdToLX200queue(commandString);

	AddCmdToLX200queue("CM");

	successFlg		=	true;

	return(successFlg);
}



#ifndef _ENABLE_SKYTRAVEL_
//*****************************************************************************
int main(int argc, char *argv[])
{
char	errorMsg[128];

	LX200_StartThread(errorMsg);
	sleep(30);
	LX200_StopThread();
	sleep(5);
}

#endif

#if 0

 148  sending	= :GR#
 181  dataBuffer	= 01:32:59#
 148  sending	= :GD#
 181  dataBuffer	= +33*48:19#
 148  sending	= :GC#
 181  dataBuffer	=
 148  sending	= :Gc#			Get Calendar Format
 181  dataBuffer	= 24#
 148  sending	= :Gd#
 181  dataBuffer	=
 148  sending	= :Ge#
 181  dataBuffer	=
 148  sending	= :GF#
 181  dataBuffer	=
 148  sending	= :GG#			Get UTC offset time
 181  dataBuffer	= -05#
 148  sending	= :Gg#			Get Current Site Longitude
 181  dataBuffer	= -074*0-1#
 148  sending	= :GL#
 181  dataBuffer	=
 148  sending	= :GM#			Get Site 1 Name
 181  dataBuffer	= TSC-MLS#
 148  sending	= :GN#
 181  dataBuffer	=
 148  sending	= :GO#
 181  dataBuffer	=
 148  sending	= :GP#
 181  dataBuffer	=
 148  sending	= :Gr#
 181  dataBuffer	=
 148  sending	= :GT#			Get tracking rate
 181  dataBuffer	= 50.0#
 148  sending	= :Gt#			Get Current Site Latitdue
 181  dataBuffer	= +41*22#
 148  sending	= :GVD#
 181  dataBuffer	=
 148  sending	= :GVN#
 181  dataBuffer	=
 148  sending	= :GVP#
 181  dataBuffer	=
 148  sending	= :GVT#
 181  dataBuffer	=
 148  sending	= :GZ#
 181  dataBuffer	=
#endif // 0
