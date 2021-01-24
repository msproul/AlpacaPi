//**************************************************************************
//*	Name:			telescopedriver_lx200.cpp
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*					This driver implements an Alpaca Telescope
//*					talking to an LX200 compatible mount
//*					vi ethernet, USB or serial port
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
//*	Jan 13,	2021	<MLS> Created telescopedriver_lx200.cpp
//*	Jan 21,	2021	<MLS> Added  AlpacaConnect() & AlpacaDisConnect() to telescope
//*	Jan 21,	2021	<MLS> TelescopeLX200 now talking via ethernet to LX200 (TSC)
//*****************************************************************************


#ifdef _ENABLE_TELESCOPE_LX200_

#include	<ctype.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<math.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"JsonResponse.h"
#include	"sidereal.h"

#include	"observatory_settings.h"

#include	"telescopedriver.h"
#include	"telescopedriver_lx200.h"
#include	"lx200_com.h"

//**************************************************************************************
void	CreateTelescopeObjects(void)
{
	new TelescopeDriverLX200(kLX200_Ethernet, "192.168.1.104:49152");
}


//**************************************************************************************
//*	the device path is one of these options
//*		192.168.1.104:49152
//*		/dev/ttyUSB0
//*		/dev/ttyS0
//**************************************************************************************
TelescopeDriverLX200::TelescopeDriverLX200(LX200ConnectionType connectionType, const char *devicePath)
	:TelescopeDriver()
{

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cDeviceName,			"Telescope-LX200");
	strcpy(cDeviceDescription,	"Telescope control using LX200 protocol");

	cLX200connType	=	connectionType;
	strcpy(cLX200devicePath,	devicePath);

	//*	setup the options for this driver
	cTelescopeProp.AlginmentMode	=	kAlignmentMode_algGermanPolar;
	cTelescopeProp.CanSlewAsync		=	true;
	cTelescopeProp.CanSync			=	true;
	cTelescopeProp.CanSetTracking	=	true;

	AlpacaConnect();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
TelescopeDriverLX200::~TelescopeDriverLX200(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
int32_t	TelescopeDriverLX200::RunStateMachine(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	if (gTelescopeUpdated)
	{
//		CONSOLE_DEBUG("gTelescopeUpdated");
		cTelescopeProp.RightAscension	=	gTelescopeRA_Hours;
		cTelescopeProp.Declination		=	gTelescopeDecl_Degrees;
//		CONSOLE_DEBUG_W_DBL("cRightAscension\t=",	cRightAscension);
//		CONSOLE_DEBUG_W_DBL("cDeclination\t=",		cDeclination);

		if (gLX200_ThreadActive)
		{
			if (cDeviceConnected == false)
			{
				//*	if it was disconnected, print out a message
				CONSOLE_DEBUG("LX200 connection Established");
			}
			cDeviceConnected	=	true;
		}
		else
		{
			if (cDeviceConnected)
			{
				//*	if it was connected, print out a message
				CONSOLE_DEBUG("LX200 connection failed!!!!!!!!!!!!!!!!!!");
			}
			cDeviceConnected	=	false;
		}

		gTelescopeUpdated	=	false;

		cTelescopeProp.Slewing	=	false;
	}
	return(15 * 1000 * 1000);
}

//*****************************************************************************
void	TelescopeDriverLX200::AlpacaConnect(void)
{
char	ipAddrString[128];
int		tcpPort;
char	*colonPtr;
int		lx200ErrCode;
char	lx200ErrMsg[128];

	CONSOLE_DEBUG(__FUNCTION__);

	switch(cLX200connType)
	{
		case kLX200_Ethernet:
			strcpy(ipAddrString, cLX200devicePath);
			colonPtr	=	strchr(ipAddrString, ':');
			if (colonPtr != NULL)
			{
				*colonPtr	=	0;
				colonPtr++;
				tcpPort		=	atoi(colonPtr);
				CONSOLE_DEBUG_W_STR("ipAddrString\t=",	ipAddrString);
				CONSOLE_DEBUG_W_NUM("tcpPort\t=",	tcpPort);
				lx200ErrCode	=	LX200_StartThread(ipAddrString, tcpPort, lx200ErrMsg);
			}
			break;

	}
}

//*****************************************************************************
void	TelescopeDriverLX200::AlpacaDisConnect(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	LX200_StopThread();
}

//*****************************************************************************
//*	needs to be over-ridden
TYPE_ASCOM_STATUS	TelescopeDriverLX200::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

	LX200_StopMovement();

	alpacaErrCode	=	kASCOM_Err_Success;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverLX200::Telescope_SlewToRA_DEC(	const double	newRA,
																	const double	newDec,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
bool				lx2000ReturnCode;

	CONSOLE_DEBUG(__FUNCTION__);

	lx2000ReturnCode	=	LX200_SlewScopeDegrees(newRA, newDec, alpacaErrMsg);
	if (lx2000ReturnCode)
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
TYPE_ASCOM_STATUS	TelescopeDriverLX200::Telescope_SyncToRA_DEC(	const double	newRA,
																	const double	newDec,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
bool				lx2000ReturnCode;

	CONSOLE_DEBUG("-------------------------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);

	lx2000ReturnCode	=	LX200_SyncScopeDegrees(newRA, newDec, alpacaErrMsg);
	if (lx2000ReturnCode)
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

#endif // _ENABLE_TELESCOPE_LX200_
