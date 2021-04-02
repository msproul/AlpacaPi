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
//*	Jan 22,	2021	<MLS> Added LX200_SyncScopeDegrees(), LX200_StopMovement()
//*	Jan 30,	2021	<MLS> Added _ENABLE_LX200_COM_ compile flag
//*****************************************************************************
//*	reference
//*		https://astro-physics.info/index.htm?tech_support/tech_support
//*		https://astro-physics.info/tech_support/mounts/command_lang.htm
//*		https://astro-physics.info/tech_support/mounts/protocol-d.pdf
//*		https://astro-physics.info/tech_support/mounts/protocol-cp3-cp4.pdf
//*****************************************************************************

#ifdef	_ENABLE_LX200_COM_


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
double	gTelescopeRA_Hours				=	1.0;
double	gTelescopeDecl_Degrees			=	1.0;

double	gTelescopeRA_Radians			=	1.0;
double	gTelescopeDecl_Radians			=	1.0;
double	gTelescopeTrackingRate			=	0.0;


char	gTelescopeRA_String[64]			=	"";
char	gTelescopeDecl_String[64]		=	"";
int		gTelescopeUpdateCnt				=	0;
char	gTelescopeErrorString[64]		=	"";

int		gLX200_SocketErrCnt				=	0;
bool	gLX200_ThreadActive				=	false;
int		gLX200_OutOfBoundsCnt			=	0;

static bool			gLX200_keepRunning	=	false;
static pthread_t	gLX200threadID;
static char			gIpAddrString[32]	=	"";
static int			gTCPport			=	0;

//*****************************************************************************
//*	returns a socket description
//*****************************************************************************
static int	LX200_OpenSocket(	struct sockaddr_in	*deviceAddress,
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

#define	kLX200ReadBuffLen		48

//*****************************************************************************
//*	returns # of bytes received
//*	<0 for error
//*****************************************************************************
int	LX200_SendCommand(int socket_desc, const char *cmdString, char *returnBuffer, unsigned int returnBufferLen)
{
char	xmitBuffer[32];
char	readData[kLX200ReadBuffLen + 8];
bool	keepReading;
int		dataLen;
int		sendRetCode;
int		recvByteCnt;


//	CONSOLE_DEBUG("----------------------------------------------");
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cmdString);
	memset(returnBuffer, 0, returnBufferLen);

	strcpy(xmitBuffer, ":");
	strcat(xmitBuffer, cmdString);
	strcat(xmitBuffer, "#");
	dataLen			=	-1;

#ifdef _ENABLE_STANDALONE_
	CONSOLE_DEBUG_W_STR("sending\t=",		xmitBuffer);
#endif // _ENABLE_STANDALONE_

	sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , 0);
	if (sendRetCode >= 0)
	{
		keepReading		=	true;
		while (keepReading && ((strlen(returnBuffer) + kLX200ReadBuffLen) < returnBufferLen))
		{
			recvByteCnt	=	recv(socket_desc, readData , kLX200ReadBuffLen , 0);
//			CONSOLE_DEBUG_W_NUM("recvByteCnt\t=", recvByteCnt);
			if (recvByteCnt > 0)
			{
				readData[recvByteCnt]	=	0;
//				CONSOLE_DEBUG_W_STR("readData\t\t=",	readData);
				strcat(returnBuffer, readData);
//				CONSOLE_DEBUG_W_STR("returnBuffer\t=",	returnBuffer);
			}
			else
			{
				keepReading		=	false;
			}
		}
		dataLen	=	strlen(returnBuffer);
//		CONSOLE_DEBUG_W_NUM("dataLen\t=", dataLen);
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("sendRetCode\t=", sendRetCode);
		strcpy(gTelescopeErrorString, "Socket failed to send");
		gTelescopeUpdated	=	true;
	}
#ifdef _ENABLE_STANDALONE_
	CONSOLE_DEBUG_W_STR("returnBuffer\t=", returnBuffer);
#endif // _ENABLE_STANDALONE_
	return(dataLen);
}



//*****************************************************************************
//*	returns a socket description
//*****************************************************************************
int		LX200_Connect(const char *ipAddressString, int port)
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
			gTelescopeRA_Hours		=	degrees_Dbl;
			gTelescopeRA_Radians	=	(degrees_Dbl / 24.0) * (2.0 * M_PI);
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
			gTelescopeDecl_Degrees	=	degrees_Dbl;
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
//*	process Tracking
//*****************************************************************************
static bool	LX200_Process_GT(char *dataBuffer)
{
bool	isValid;

	isValid		=	CheckForValidResponse(dataBuffer);
	if (isValid)
	{
		gTelescopeTrackingRate	=	atof(dataBuffer);
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

	CONSOLE_ABORT(__FUNCTION__);

	gLX200_ThreadActive	=	true;
	gLX200_SocketErrCnt	=	0;
	gLX200CmdQueCnt		=	0;
//	socket_desc	=	LX200_Connect("192.168.1.104", 49152);
	socket_desc	=	LX200_Connect(gIpAddrString, gTCPport);


	if (socket_desc > 0)
	{
		while (gLX200_keepRunning && (gLX200_SocketErrCnt < 20))
		{
#ifdef _ENABLE_STANDALONE_
			CONSOLE_DEBUG("---------------------------------------------");
#endif // _ENABLE_STANDALONE_
//			CONSOLE_DEBUG(__FUNCTION__);

			//*	if we have commands queued up, get them done first
			if (gLX200CmdQueCnt > 0)
			{
				SendCmdsFromQueue(socket_desc);
			}
			else
			{
				//--------------------------------------------------------------------------
				//*	Right Ascension
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
						CONSOLE_DEBUG_W_NUM("gLX200_SocketErrCnt\t=", gLX200_SocketErrCnt);
					}
					usleep(1000);
				}
				else
				{
					gLX200_SocketErrCnt++;
					CONSOLE_DEBUG_W_NUM("gLX200_SocketErrCnt\t=", gLX200_SocketErrCnt);
				}

				//--------------------------------------------------------------------------
				//*	Declination
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
				else
				{
					gLX200_SocketErrCnt++;
					CONSOLE_DEBUG_W_NUM("gLX200_SocketErrCnt\t=", gLX200_SocketErrCnt);
				}

				//--------------------------------------------------------------------------
				//*	TrackingRate
				returnByteCNt	=	LX200_SendCommand(socket_desc, "GT", dataBuffer, 400);
				if (returnByteCNt > 0)
				{
					isValid			=	LX200_Process_GT(dataBuffer);
					if (isValid)
					{
						gTelescopeInfoValid	=	true;
					}
					else
					{
					}
					usleep(1000);
				}
				else
				{
					gLX200_SocketErrCnt++;
					CONSOLE_DEBUG_W_NUM("gLX200_SocketErrCnt\t=", gLX200_SocketErrCnt);
				}
				//--------------------------------------------------------------------------



				gTelescopeUpdateCnt++;
				gTelescopeUpdated		=	true;
				sleep(1);
			}

#ifdef _ENABLE_STANDALONE_
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

#endif // _ENABLE_STANDALONE_



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
int		LX200_StartThread(const char *ipAddrString, const int tcpPort, char *errorMsg)
{
int			threadErr;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_ABORT(__FUNCTION__);
	threadErr	=	-1;
	errorMsg[0]	=	0;

	strcpy(gIpAddrString,	ipAddrString);
	gTCPport			=	tcpPort;



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
bool	LX200_SlewScopeDegrees(	const double	newRtAscen_Hours,
								const double	newDeclination_Degrees,
								char			*returnErrMsg)
{
bool	successFlg;
char	timeString[32];
char	commandString[32];

	CONSOLE_DEBUG(__FUNCTION__);
	successFlg		=	false;
	if (gLX200_ThreadActive)
	{
		FormatHHMMSS(newRtAscen_Hours, timeString, false);

		//*	create the LX200 command SrHH:MM:SS
		strcpy(commandString, "Sr");
		strcat(commandString, timeString);
		AddCmdToLX200queue(commandString);

		//-------------------------------------------------

		FormatHHMMSS(newDeclination_Degrees, timeString, true);

		//*	create the LX200 command SdsDD*MM
		strcpy(commandString, "Sd");
		strcat(commandString, timeString);
		AddCmdToLX200queue(commandString);

		//*	Slew command
		//*	:MS# Slew to Target Object
		//		Returns:
		//		0				Slew is Possible
		//		1<string>#		Object Below Horizon w/string message
		//		2<string>#		Object Below Higher w/string message
		AddCmdToLX200queue("MS");

		successFlg		=	true;
	}
	return(successFlg);
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
bool	LX200_SyncScopeDegrees(	const double	newRtAscen_Hours,
								const double	newDeclination_Degrees,
								char			*returnErrMsg)
{
bool	successFlg;
char	timeString[32];
char	commandString[32];

	CONSOLE_DEBUG(__FUNCTION__);
	successFlg		=	false;
	if (gLX200_ThreadActive)
	{
		FormatHHMMSS(newRtAscen_Hours, timeString, false);

		//*	create the LX200 command SrHH:MM:SS
		strcpy(commandString, "Sr");
		strcat(commandString, timeString);
		AddCmdToLX200queue(commandString);

		//-------------------------------------------------

		FormatHHMMSS(newDeclination_Degrees, timeString, true);

		//*	create the LX200 command SdsDD*MM
		strcpy(commandString, "Sd");
		strcat(commandString, timeString);
		AddCmdToLX200queue(commandString);

		//*	syncCommand
		//*	:CM# Synchronizes the telescope's position with the currently selected database object's coordinates
		AddCmdToLX200queue("CM");

		successFlg		=	true;
	}
	return(successFlg);
}

//*****************************************************************************
bool	LX200_SyncScope(const double newRtAscen_Radians, const double newDeclination_Radians, char *returnErrMsg)
{
double	myRtAscenDeg360;
double	myRtAscenDeg24;
double	declDegrees;
bool	successFlg;

	CONSOLE_DEBUG(__FUNCTION__);
	//-------------------------------------------------
	//*	deal with the right ascension
	//*	remember, 0->2 Pi   ==>>  0 ->24 hours

	myRtAscenDeg360	=	DEGREES(newRtAscen_Radians);
	if (myRtAscenDeg360 < 0.0)
	{
		myRtAscenDeg360	+=	360.0;
	}
	myRtAscenDeg24	=	(24.0 / 360.0) * myRtAscenDeg360;


	//-------------------------------------------------
	//*	now deal with the declination, its easy
	declDegrees		=	DEGREES(newDeclination_Radians);

	successFlg	=	LX200_SyncScopeDegrees(myRtAscenDeg24, declDegrees, returnErrMsg);

	return(successFlg);
}

//*****************************************************************************
bool	LX200_StopMovement(void)
{
bool	successFlg;

	CONSOLE_DEBUG(__FUNCTION__);
	//*	this wipes out all existing commands in the queue
	gLX200CmdQueCnt	=	0;

	AddCmdToLX200queue("Q");
	AddCmdToLX200queue("QQ");	//*	QQ is NOT part of the LX200 standard. Added into TSC

	successFlg		=	true;

	return(successFlg);
}


#ifdef _ENABLE_STANDALONE_
//*****************************************************************************
int main(int argc, char *argv[])
{
char	errorMsg[128];

	LX200_StartThread("192.168.1.104", 49152, errorMsg);
	sleep(30);
	LX200_StopThread();
	sleep(5);
}

#endif

#if 0
	LX200Commands.getDecl = QString("GD");
	LX200Commands.getRA = QString("GR");
	LX200Commands.getHiDef = QString("U");
	LX200Commands.stopMotion = QString("Q");
	LX200Commands.slewRA = QString("Sr");
	LX200Commands.slewDecl = QString("Sd");
	LX200Commands.slewPossible = QString("MS");
	LX200Commands.syncCommand = QString("CM");
	LX200Commands.moveEast = QString("Me");
	LX200Commands.moveWest = QString("Mw");
	LX200Commands.moveNorth = QString("Mn");
	LX200Commands.moveSouth = QString("Ms");
	LX200Commands.stopMoveEast = QString("Qe");
	LX200Commands.stopMoveWest = QString("Qw");
	LX200Commands.stopMoveNorth = QString("Qn");
	LX200Commands.stopMoveSouth = QString("Qs");
	LX200Commands.setCenterSpeed = QString("RC");
	LX200Commands.setGuideSpeed = QString("RG");
	LX200Commands.setFindSpeed = QString("RM");
	LX200Commands.setGOTOSpeed = QString("RS");
	LX200Commands.getCalendarFormat  = QString("Gc");
	LX200Commands.getDate = QString("GC");
	LX200Commands.getName = QString("GM");
	LX200Commands.getTrackingRate = QString("GT");
	LX200Commands.getLatitude = QString("Gt");
	LX200Commands.getLongitude = QString("Gg");
	LX200Commands.getUTCOffset = QString("GG");
	LX200Commands.setUTCOffset = QString("SG");
	LX200Commands.getLocalTime = QString("GL");
	LX200Commands.setLocalTime = QString("SL");
	LX200Commands.setLocalDate = QString("SC");
	LX200Commands.setLongitude = QString("Sg");
	LX200Commands.setLatitude = QString("St");


437 [LX200CommThread	 ] LX200CommThread
178 [LX200_SendCommand   ] sending	= :GR#
210 [LX200_SendCommand   ] dataBuffer	= 326:16:45#
178 [LX200_SendCommand   ] sending	= :GD#
210 [LX200_SendCommand   ] dataBuffer	= +32*58:31#
178 [LX200_SendCommand   ] sending	= :GC#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :Gc#
210 [LX200_SendCommand   ] dataBuffer	= 24#
178 [LX200_SendCommand   ] sending	= :Gd#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :Ge#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :GF#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :GG#
210 [LX200_SendCommand   ] dataBuffer	= -05#
178 [LX200_SendCommand   ] sending	= :Gg#
210 [LX200_SendCommand   ] dataBuffer	= -074*0-1#
178 [LX200_SendCommand   ] sending	= :GL#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :GM#
210 [LX200_SendCommand   ] dataBuffer	= TSC-MLS#
178 [LX200_SendCommand   ] sending	= :GN#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :GO#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :GP#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :Gr#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :GT#
210 [LX200_SendCommand   ] dataBuffer	= 50.0#
178 [LX200_SendCommand   ] sending	= :Gt#
210 [LX200_SendCommand   ] dataBuffer	= +41*22#
178 [LX200_SendCommand   ] sending	= :GVD#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :GVN#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :GVP#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :GVT#
210 [LX200_SendCommand   ] dataBuffer	=
178 [LX200_SendCommand   ] sending	= :GZ#
210 [LX200_SendCommand   ] dataBuffer	=
#endif // 0




#endif // _ENABLE_LX200_COM_
