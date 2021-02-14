//**************************************************************************
//*	Name:			telescopedriver_lx200.cpp
//*
//*	Author:			Mark Sproul (C) 2021
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*					This driver implements an Alpaca Telescope
//*					talking to  via ethernet, USB or serial port
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
//*
//*	Usage notes:	This driver does not implement any actual device,
//*					you must create a sub-class that does the actual control
//*
//*	References:		https://ascom-standards.org/api/
//*					https://ascom-standards.org/Help/Developer/html/N_ASCOM_DeviceInterface.htm
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Feb  7,	2021	<MLS> Created telescopedriver_comm.cpp
//*	Feb  9,	2021	<MLS> Moved device comm variables from main class to comm class
//*****************************************************************************


#ifdef _ENABLE_TELESCOPE_

#include	<ctype.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<math.h>
#include	<errno.h>
#include	<termios.h>
#include	<fcntl.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"serialport.h"



#include	"telescopedriver.h"
#include	"telescopedriver_comm.h"
#include	"telescopedriver_skywatch.h"
#include	"telescopedriver_lx200.h"

static void	*Telescope_Comm_Thread(void *arg);

//**************************************************************************************
void	CreateTelescopeObjects(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

//	new TelescopeDriverComm(kDevCon_Ethernet, "192.168.1.104:49152");
//	new TelescopeDriverSkyWatch(kDevCon_Serial, "/dev/ttyS0");
	new TelescopeDriverLX200(kDevCon_Ethernet, "192.168.1.104:49152");
}

//**************************************************************************************
//*	the device path is one of these options (or similar)
//*		192.168.1.104:49152
//*		/dev/ttyUSB0
//*		/dev/ttyS0
//**************************************************************************************
TelescopeDriverComm::TelescopeDriverComm(DeviceConnectionType connectionType, const char *devicePath)
	:TelescopeDriver()
{
char	*colonPtr;

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name,		"Telescope-Comm");
	strcpy(cCommonProp.Description,	"Telescope control using ??? protocol");

	strcpy(cDeviceIPaddress,	"0.0.0.0-Not set");
	cIPaddrValid	=	false;

	cDeviceConnType	=	connectionType;
	strcpy(cDeviceConnPath,	devicePath);

	if (connectionType == kDevCon_Ethernet)
	{
		strcpy(cDeviceIPaddress, cDeviceConnPath);
		colonPtr	=	strchr(cDeviceIPaddress, ':');
		if (colonPtr != NULL)
		{
			*colonPtr	=	0;
			colonPtr++;
			cTCPportNum		=	atoi(colonPtr);
			CONSOLE_DEBUG_W_STR("cDeviceIPaddress\t=",	cDeviceIPaddress);
			CONSOLE_DEBUG_W_NUM("cTCPportNum\t=",	cTCPportNum);

			cIPaddrValid			=	true;
		}

	}

	//*	setup the options for this driver
	cTelescopeProp.AlginmentMode	=	kAlignmentMode_algGermanPolar;
	cTelescopeProp.CanSlewAsync		=	true;
	cTelescopeProp.CanSync			=	true;
	cTelescopeProp.CanSetTracking	=	true;


//	AlpacaConnect();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
TelescopeDriverComm::~TelescopeDriverComm(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	AlpacaDisConnect();
}

//**************************************************************************************
int32_t	TelescopeDriverComm::RunStateMachine(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	if (cThreadIsActive)
	{
		if (cDeviceConnected == false)
		{
			//*	if it was disconnected, print out a message
			CONSOLE_DEBUG("Connection Established");
		}
		cDeviceConnected	=	true;
	}
	else
	{
		if (cDeviceConnected)
		{
			//*	if it was connected, print out a message
			CONSOLE_DEBUG_W_STR("Connection failed!!!!!!!!!!!!!!!!!!", cCommonProp.Name);
		}
		cDeviceConnected	=	false;
	}

#ifdef _NOT_FINISHED
	if (gTelescopeUpdated)
	{
//		CONSOLE_DEBUG("gTelescopeUpdated");
		cTelescopeProp.RightAscension	=	gTelescopeRA_Hours;
		cTelescopeProp.Declination		=	gTelescopeDecl_Degrees;
//		CONSOLE_DEBUG_W_DBL("cRightAscension\t=",	cRightAscension);
//		CONSOLE_DEBUG_W_DBL("cDeclination\t=",		cDeclination);


		gTelescopeUpdated	=	false;

		cTelescopeProp.Slewing	=	false;
	}
#endif
	return(15 * 1000 * 1000);
}

//*****************************************************************************
bool	TelescopeDriverComm::AlpacaConnect(void)
{
char	*colonPtr;
bool	connectionOKflag;

	CONSOLE_DEBUG(__FUNCTION__);
	connectionOKflag	=	false;
	switch(cDeviceConnType)
	{
		case kDevCon_Ethernet:
			CONSOLE_DEBUG("kDevCon_Ethernet");
			if (cIPaddrValid)
			{
				connectionOKflag	=	true;
			}
			break;

		case kDevCon_USB:
			CONSOLE_DEBUG("kDevCon_USB");
			connectionOKflag	=	false;
			break;

		case kDevCon_Serial:
			CONSOLE_DEBUG("kDevCon_Serial");
			cDeviceConnFileDesc	=	open(cDeviceConnPath, O_RDWR);	//* connect to port
			if (cDeviceConnFileDesc >= 0)
			{
				CONSOLE_DEBUG_W_STR("Serial port opened OK", cDeviceConnPath);
				connectionOKflag	=	true;
				Set_Serial_attribs(cDeviceConnFileDesc, B9600, 0);	//*	set the baud rate
			}
			else
			{
				CONSOLE_DEBUG_W_STR("failed to open", cDeviceConnPath);
				connectionOKflag	=	false;
			}
			break;
	}
	if (connectionOKflag)
	{
		StartThread();
	}
	else
	{
		CONSOLE_DEBUG("Connection failure");
	}
	return(connectionOKflag);
}

//*****************************************************************************
bool	TelescopeDriverComm::AlpacaDisConnect(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	StopThread();

	return(true);
}

//*****************************************************************************
//*	needs to be over-ridden
TYPE_ASCOM_STATUS	TelescopeDriverComm::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

	StopMovement();
	alpacaErrCode	=	kASCOM_Err_Success;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverComm::Telescope_SlewToRA_DEC(	const double	newRA,
																	const double	newDec,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
bool				returnCode;

	CONSOLE_DEBUG(__FUNCTION__);

	returnCode	=	false;

	returnCode	=	SlewScopeDegrees(newRA, newDec, alpacaErrMsg);
	if (returnCode)
	{
		cTelescopeProp.Slewing	=	true;
		alpacaErrCode			=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverComm::Telescope_SyncToRA_DEC(	const double	newRA,
																	const double	newDec,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
bool				returnCode;

	CONSOLE_DEBUG("-------------------------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);

	returnCode	=	SyncScopeDegrees(newRA, newDec, alpacaErrMsg);
	if (returnCode)
	{
		CONSOLE_DEBUG("kASCOM_Err_Success");
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		CONSOLE_DEBUG("kASCOM_Err_NotConnected");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	return(alpacaErrCode);
}

//*****************************************************************************
bool	TelescopeDriverComm::StartThread(void)
{
int		threadErr;
bool	successFlag;
char	errorMsg[64];

	CONSOLE_DEBUG(__FUNCTION__);
	//*	check to make sure a thread is not already running
	successFlag	=	false;
	if (cThreadIsActive == false)
	{
		cKeepRunningFlag	=	true;
		threadErr			=	pthread_create(&cThreadID, NULL, &Telescope_Comm_Thread, this);
		if (threadErr == 0)
		{
			successFlag	=	true;
		}
		else
		{
			sprintf(errorMsg, "Error creating thread err=%d", threadErr);
			CONSOLE_DEBUG(errorMsg);
		}
	}
	else
	{
		strcpy(errorMsg, "Thread already running");
		CONSOLE_DEBUG(errorMsg);
		threadErr	=	-1;
		CONSOLE_ABORT(__FUNCTION__);
	}
	return(successFlag);
}


//*****************************************************************************
void	TelescopeDriverComm::StopThread(void)
{
	cKeepRunningFlag	=	false;

}

//*****************************************************************************
bool	TelescopeDriverComm::SlewScopeDegrees(	const double	newRtAscen_Hours,
												const double	newDeclination_Degrees,
												char			*returnErrMsg)
{
	return(false);

}

//*****************************************************************************
bool	TelescopeDriverComm::SyncScope(	const double	newRtAscen_Radians,
										const double	new_Declination_Radians,
										char			*returnErrMsg)
{
	return(false);

}

//*****************************************************************************
bool	TelescopeDriverComm::SyncScopeDegrees(	const double	newRtAscen_Hours,
												const double	newDeclination_Degrees,
												char			*returnErrMsg)
{
	return(false);

}

//*****************************************************************************
bool	TelescopeDriverComm::StopMovement(void)
{
	return(false);

}


//*****************************************************************************
int	TelescopeDriverComm::OpenSocket(	struct sockaddr_in	*deviceAddress,
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
			sprintf(cTelescopeErrorString, "Connection refused - %s", ipString);
			CONSOLE_DEBUG(cTelescopeErrorString);

//+			gTelescopeThreadKeepRunning	=	false;
			cNewTelescopeDataAvailble	=	true;
		}
		else
		{
			inet_ntop(AF_INET, &deviceAddress->sin_addr.s_addr, ipString, INET_ADDRSTRLEN);

			sprintf(cTelescopeErrorString, "Connection error, ipaddress = %s", ipString);
			CONSOLE_DEBUG(cTelescopeErrorString);
			CONSOLE_DEBUG_W_NUM("errno\t\t\t=", errno);
//+			gTelescopeThreadKeepRunning	=	false;
			cNewTelescopeDataAvailble			=	true;
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("socket_desc\t=", socket_desc);
		CONSOLE_DEBUG_W_NUM("errno\t\t=", errno);
	}
	return(socket_desc);
}

//*****************************************************************************
int	TelescopeDriverComm::OpenSocket(const char *ipAddress, const int port)
{
struct sockaddr_in	deviceAddress;
char				outputIPaddr[64];
int					socket_desc;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("ipAddress\t\t=",	ipAddress);

	inet_pton(AF_INET, ipAddress, &(deviceAddress.sin_addr));
	inet_ntop(AF_INET, &(deviceAddress.sin_addr), outputIPaddr, INET_ADDRSTRLEN);
	CONSOLE_DEBUG_W_STR("Connecting to\t=",		outputIPaddr);
	CONSOLE_DEBUG_W_NUM("on TCP port\t=",		port);

	socket_desc	=	OpenSocket(&deviceAddress, cTCPportNum);
//	CONSOLE_DEBUG_W_NUM("socket_desc\t=",		socket_desc);

	return(socket_desc);
}

//*****************************************************************************
bool	TelescopeDriverComm::SendCmdsFromQueue(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	//*	this must be over ridden
	return(false);
}

//*****************************************************************************
bool	TelescopeDriverComm::SendCmdsPeriodic(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	//*	this must be over ridden
	return(false);
}


//*****************************************************************************
//*	This gets called by the "C" thread routine,
//*	The real work is done here so that it can be in the class instead of outside
//*
//*	this can be over ridden but should be able to handle most cases
//*****************************************************************************
void	*TelescopeDriverComm::RunThread(void)
{
bool		connectionOpen;
int			shutDownRetCode;
int			closeRetCode;
int			errorCount;
bool		sendOK;

	cThreadIsActive	=	true;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("cDeviceIPaddress\t=",	cDeviceIPaddress);
	CONSOLE_DEBUG_W_NUM("cTCPportNum\t=",	cTCPportNum);


	//########################################################
	//*	open the connection
	connectionOpen	=	false;
	switch(cDeviceConnType)
	{
		case kDevCon_Ethernet:
			cSocket_desc	=	OpenSocket(cDeviceIPaddress, cTCPportNum);
			if (cSocket_desc > 0)
			{
				connectionOpen	=	true;
			}
			break;

		case kDevCon_USB:
			connectionOpen		=	false;
			break;

		case kDevCon_Serial:
			connectionOpen	=	true;
			break;
	}

	//########################################################
	//*	did we open the connection OK
	if (connectionOpen)
	{
		//*	this is our main loop for the communications thread
		//*	we will do the following
		//*		check for and send queued commands
		//*		send on going GET INFO commands
		//*		parse the info coming back from the telescope
		//*		update as appropriate
		errorCount	=	0;
		while (cKeepRunningFlag)
		{
			//*	now we are going to send commands to the telescope
			if (cQueuedCmdCnt > 0)
			{
				sendOK	=	SendCmdsFromQueue();
				if (sendOK == false)
				{
					errorCount++;
				}
				sleep(1);
			}
			else
			{
				//*	send periodic commands
				sendOK	=	SendCmdsPeriodic();
				if (sendOK == false)
				{
					errorCount++;
				}
				sleep(5);
			}
		}
		//########################################################
		//*	close the connection
		switch(cDeviceConnType)
		{
			case kDevCon_Ethernet:
				shutDownRetCode	=	shutdown(cSocket_desc, SHUT_RDWR);
				if (shutDownRetCode != 0)
				{
					CONSOLE_DEBUG_W_NUM("shutDownRetCode\t=", shutDownRetCode);
					CONSOLE_DEBUG_W_NUM("errno\t=", errno);
				}
				CONSOLE_DEBUG("close");
				closeRetCode	=	close(cSocket_desc);
				if (closeRetCode != 0)
				{
					CONSOLE_DEBUG_W_NUM("closeRetCode\t=", closeRetCode);
					CONSOLE_DEBUG_W_NUM("errno\t=", errno);
				}
				cSocket_desc	=	-1;
				break;

			case kDevCon_USB:
				break;

			case kDevCon_Serial:
				break;
		}
	}
	else
	{
		CONSOLE_DEBUG("Failed to open connection");
	}
	CONSOLE_DEBUG("Thread EXIT!!!!!!!!!!!!!!!!!!");

	cThreadIsActive	=	false;
	return(NULL);
}



//*****************************************************************************
static void	*Telescope_Comm_Thread(void *arg)
{
//int					socket_desc;
int					returnByteCNt;
//char				dataBuffer[500];
//int					shutDownRetCode;
//int					closeRetCode;
//bool				isValid;
TelescopeDriverComm	*telscopeCommPtr;
void				*returnValue;

	CONSOLE_DEBUG(__FUNCTION__);

	returnValue		=	NULL;
	telscopeCommPtr	=	(TelescopeDriverComm *)arg;
	if (telscopeCommPtr != NULL)
	{
		//*	normally this never returns
		returnValue	=	telscopeCommPtr->RunThread();
	}
	else
	{
		CONSOLE_DEBUG("telscopeCommPtr is NULL");
		CONSOLE_ABORT(__FUNCTION__);
	}

#if 0
//+	gLX200_ThreadActive	=	true;
//+	gLX200_SocketErrCnt	=	0;
//+	gLX200CmdQueCnt		=	0;

	socket_desc	=	telscopeCommPtr->OpenSocket();


	if (socket_desc > 0)
	{
//+		while (gLX200_keepRunning && (gLX200_SocketErrCnt < 20))
		{

			//*	if we have commands queued up, get them done first
//+			if (gLX200CmdQueCnt > 0)
			{
//+				SendCmdsFromQueue(socket_desc);
			}
//+			else
			{
				//--------------------------------------------------------------------------
				//*	Right Ascension
//+				returnByteCNt	=	LX200_SendCommand(socket_desc, "GR", dataBuffer, 400);
				if (returnByteCNt > 0)
				{
//+					isValid			=	LX200_Process_GR_RtAsc(dataBuffer);
					if (isValid)
					{
//+						gTelescopeInfoValid	=	true;
					}
					else
					{
//+						strcpy(gTelescopeRA_String, "RA failed");
//+						gLX200_SocketErrCnt++;
//+						gTelescopeInfoValid	=	false;
//+						CONSOLE_DEBUG_W_NUM("gLX200_SocketErrCnt\t=", gLX200_SocketErrCnt);
					}
					usleep(1000);
				}
				else
				{
//+					gLX200_SocketErrCnt++;
//+					CONSOLE_DEBUG_W_NUM("gLX200_SocketErrCnt\t=", gLX200_SocketErrCnt);
				}

				//--------------------------------------------------------------------------
				//*	Declination
//+				returnByteCNt	=	LX200_SendCommand(socket_desc, "GD", dataBuffer, 400);
				if (returnByteCNt > 0)
				{
//+					isValid			=	LX200_Process_GD(dataBuffer);
					if (isValid)
					{
//+						gTelescopeInfoValid	=	true;
					}
					else
					{
//+						strcpy(gTelescopeDecl_String, "DEC failed");
//+						gLX200_SocketErrCnt++;
//+						gTelescopeInfoValid	=	false;
					}
					usleep(1000);
				}
				else
				{
//+					gLX200_SocketErrCnt++;
//+					CONSOLE_DEBUG_W_NUM("gLX200_SocketErrCnt\t=", gLX200_SocketErrCnt);
				}

				//--------------------------------------------------------------------------
				//*	TrackingRate
//+				returnByteCNt	=	LX200_SendCommand(socket_desc, "GT", dataBuffer, 400);
				if (returnByteCNt > 0)
				{
//+					isValid			=	LX200_Process_GT(dataBuffer);
					if (isValid)
					{
//+						gTelescopeInfoValid	=	true;
					}
					else
					{
					}
					usleep(1000);
				}
				else
				{
//+					gLX200_SocketErrCnt++;
//+					CONSOLE_DEBUG_W_NUM("gLX200_SocketErrCnt\t=", gLX200_SocketErrCnt);
				}
				//--------------------------------------------------------------------------



//+				gTelescopeUpdateCnt++;
				gTelescopeUpdated		=	true;
				sleep(1);
			}



//+			if (gLX200_OutOfBoundsCnt > 0)
			{
//+				CONSOLE_DEBUG_W_NUM("gLX200_OutOfBoundsCnt\t=", gLX200_OutOfBoundsCnt);
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
//+		strcpy(cTelescopeErrorString, "Socket failed to open");
	}
//+	strcpy(gTelescopeRA_String, "Thread exit");
//+	gLX200_ThreadActive		=	false;
	gTelescopeUpdated		=	true;
#endif // 0

	return(returnValue);
}





#endif // _ENABLE_TELESCOPE_LX200_
