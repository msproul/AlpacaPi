//**************************************************************************
//*	Name:			telescopedriver_comm.cpp
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
//*	Mar 31,	2021	<MLS> Moved command queue buffer to comm class
//*	Sep 21,	2023	<MLS> Switching telescope comm thread to use driver class threads
//*	Sep 21,	2023	<MLS> Added RunThread_Startup() & RunThread_Loop()
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
#include	"linuxerrors.h"



#include	"telescopedriver.h"
#include	"telescopedriver_comm.h"
#include	"telescopedriver_lx200.h"
#include	"telescopedriver_Rigel.h"
#include	"telescopedriver_skywatch.h"
#include	"telescopedriver_servo.h"
#include	"telescopedriver_sim.h"

//static void	*Telescope_Comm_Thread(void *arg);


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
	//*	set default conditions
	strcpy(cDeviceIPaddress,	"0.0.0.0-Not set");
	cIPaddrValid			=	false;

	//*	set the parameters
	strcpy(cCommonProp.Name,		"Telescope-Comm");
	strcpy(cCommonProp.Description,	"Telescope control using ??? protocol");
	cDeviceConnType			=	connectionType;
	strcpy(cDeviceConnPath,		devicePath);

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


	cQueuedCmdCnt	=	0;

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

	if (cDriverThreadIsActive)
	{
		if (cCommonProp.Connected == false)
		{
			//*	if it was disconnected, print out a message
			CONSOLE_DEBUG("Connection Established");
		}
		cCommonProp.Connected	=	true;
	}
	else
	{
		if (cCommonProp.Connected)
		{
			//*	if it was connected, print out a message
			CONSOLE_DEBUG_W_STR("Connection failed!!!!!!!!!!!!!!!!!!", cCommonProp.Name);
		}
		cCommonProp.Connected	=	false;
	}

#ifdef _NOT_FINISHED
	if (gTelescopeUpdated)
	{
//		CONSOLE_DEBUG("gTelescopeUpdated");
		cTelescopeProp.RightAscension	=	gTelescopeRA_Hours;
		cTelescopeProp.Declination		=	gTelescopeDecl_Degrees;
//		CONSOLE_DEBUG_W_DBL("cRightAscension\t=",	cRightAscension);
//		CONSOLE_DEBUG_W_DBL("cDeclination\t=",		cDeclination);

		gTelescopeUpdated		=	false;

		cTelescopeProp.Slewing	=	false;
	}
#endif
	return(15 * 1000 * 1000);
}

//*****************************************************************************
bool	TelescopeDriverComm::AlpacaConnect(void)
{
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
			//*	check to make sure its a valid serial port
			if (strncmp(cDeviceConnPath, "/dev/tty", 8) == 0)
			{
				connectionOKflag	=	true;
			}
			break;

		case kDevCon_Custom:
			break;
	}
	if (connectionOKflag)
	{
		if (cDriverThreadIsActive == false)
		{
			StartDriverThread();
		}
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

	StopDriverThread();

	return(true);
}

//*****************************************************************************
//*	needs to be over-ridden
TYPE_ASCOM_STATUS	TelescopeDriverComm::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
//*	needs to be over-ridden
TYPE_ASCOM_STATUS	TelescopeDriverComm::Telescope_MoveAxis(const int axisNum, const double moveRate_degPerSec, char *alpacaErrMsg)
{
	TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverComm::Telescope_SlewToRA_DEC(	const double	newRtAscen_Hours,
																	const double	newDeclination_Degrees,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverComm::Telescope_SyncToRA_DEC(	const double	newRtAscen_Hours,
																	const double	newDeclination_Degrees,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
//*	needs to be over-ridden
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverComm::Telescope_TrackingOnOff(const bool newTrackingState, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
int	TelescopeDriverComm::OpenSocket(struct sockaddr_in	*deviceAddress,
									const int			port)
{
int					socket_desc;
struct sockaddr_in	remoteDev;
int					connRetCode;
char				ipString[32];
struct timeval		timeoutLength;
int					setOptRetCode;
char				linuxErrorStr[256];

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
			CONSOLE_DEBUG("setsockopt(SO_RCVTIMEO) failed");
		//	perror("setsockopt(SO_RCVTIMEO) failed");
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
			GetLinuxErrorString(errno, linuxErrorStr);
			CONSOLE_DEBUG(linuxErrorStr);
//+			gTelescopeThreadKeepRunning	=	false;
			cNewTelescopeDataAvailble	=	true;
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("socket_desc\t=", socket_desc);
		CONSOLE_DEBUG_W_NUM("errno\t\t=", errno);
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
	return(socket_desc);
}

//*****************************************************************************
int	TelescopeDriverComm::OpenSocket(const char *ipAddress, const int port)
{
struct sockaddr_in	deviceAddress;
char				outputIPaddr[64];
int					socket_desc;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("ipAddress\t\t=",	ipAddress);

	inet_pton(AF_INET, ipAddress, &(deviceAddress.sin_addr));
	inet_ntop(AF_INET, &(deviceAddress.sin_addr), outputIPaddr, INET_ADDRSTRLEN);
	CONSOLE_DEBUG_W_STR("Connecting to\t=",		outputIPaddr);
	CONSOLE_DEBUG_W_NUM("on TCP port\t=",		port);

	socket_desc	=	OpenSocket(&deviceAddress, cTCPportNum);
//	CONSOLE_DEBUG_W_NUM("socket_desc\t=",		socket_desc);

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
	return(socket_desc);
}

//*****************************************************************************
//*	This can be overloaded but does not have to be
//*****************************************************************************
void	TelescopeDriverComm::AddCmdToQueue(const char *cmdString)
{
	CONSOLE_DEBUG_W_STR("cmdString\t\t=", cmdString);
	if (cQueuedCmdCnt < kMaxTelescopeCmds)
	{
		strcpy(cCmdQueue[cQueuedCmdCnt].cmdString, cmdString);
		cQueuedCmdCnt++;
	}
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
void	TelescopeDriverComm::RunThread_Startup(void)
{

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("cDeviceIPaddress\t=",	cDeviceIPaddress);
	CONSOLE_DEBUG_W_NUM("cTCPportNum\t=",	cTCPportNum);

	//--------------------------------------------------------
	//*	open the connection
	cTelescopeConnectionOpen	=	false;
	cTelescopeCommErrCnt		=	0;
	switch(cDeviceConnType)
	{
		case kDevCon_Ethernet:
			CONSOLE_DEBUG("Calling OpenSocket()");
			cSocket_desc	=	OpenSocket(cDeviceIPaddress, cTCPportNum);
			CONSOLE_DEBUG("Returned from OpenSocket()");
			if (cSocket_desc > 0)
			{
				CONSOLE_DEBUG("Connection is open");
				cTelescopeConnectionOpen	=	true;
			}
			else
			{
				CONSOLE_DEBUG("Failed to open socket");
			}
			CONSOLE_DEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!");
			break;

		case kDevCon_USB:
			cTelescopeConnectionOpen		=	false;
			break;

		case kDevCon_Serial:
			cDeviceConnFileDesc	=	open(cDeviceConnPath, O_RDWR);	//* connect to port
			if (cDeviceConnFileDesc >= 0)
			{
				CONSOLE_DEBUG_W_STR("Serial port opened OK", cDeviceConnPath);

				Serial_Set_Attribs(cDeviceConnFileDesc, B9600, 0);	//*	set the baud rate
				Serial_Set_Blocking (cDeviceConnFileDesc, false);

				cTelescopeConnectionOpen	=	true;
			}
			else
			{
				CONSOLE_DEBUG_W_STR("failed to open", cDeviceConnPath);
				cTelescopeConnectionOpen	=	false;
			}
			break;

		case kDevCon_Custom:
			break;
	}

	CONSOLE_DEBUG_W_BOOL("cTelescopeConnectionOpen  \t=", cTelescopeConnectionOpen);
	CONSOLE_DEBUG_W_BOOL("cDriverThreadKeepRunning\t=", cDriverThreadKeepRunning);
}

//*****************************************************************************
//*	The real work is done here so that it can be in the class instead of outside
//*	this can be over ridden but should be able to handle most cases
//*****************************************************************************
void	TelescopeDriverComm::RunThread_Loop(void)
{
int			shutDownRetCode;
int			closeRetCode;
bool		sendOK;

	//--------------------------------------------------------
	//*	this is inside of the while loop so that we can re-open the connection if it drops.

	//--------------------------------------------------------
	//*	did we open the connection OK
	if (cTelescopeConnectionOpen)
	{
		//*	this is our main loop for the communications thread
		//*	we will do the following
		//*		check for and send queued commands
		//*		send on going GET INFO commands
		//*		parse the info coming back from the telescope
		//*		update as appropriate
		//*	now we are going to send commands to the telescope
		if (cQueuedCmdCnt > 0)
		{
			sendOK	=	SendCmdsFromQueue();
			if (sendOK == false)
			{
				cTelescopeCommErrCnt++;
			}
		}
		else
		{
			//*	send periodic commands
			sendOK	=	SendCmdsPeriodic();
			if (sendOK == false)
			{
				cTelescopeCommErrCnt++;
			}
		}
		usleep(500000);

		//*	if the error count gets too big, shut down and re-open the connection
		if (cTelescopeCommErrCnt > 20)
		{
			CONSOLE_DEBUG("Closing connection due to error count, will try to re-open");
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
					cSocket_desc				=	-1;
					cTelescopeConnectionOpen	=	false;
					break;

				case kDevCon_USB:
					break;

				case kDevCon_Serial:
					break;

				case kDevCon_Custom:
					break;
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("Telescope connection is not open!!!!!!!!!!!!!!!!!!!");
		//*	this will close the thread and the background routine will restart it.
		StopDriverThread();
	}
}

#endif // _ENABLE_TELESCOPE_LX200_
