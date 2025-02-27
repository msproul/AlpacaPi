//**************************************************************************
//*	Name:			telescopedriver_skywatch.cpp
//*
//*	Author:			Mark Sproul (C) 2021
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*					This driver implements an Alpaca Telescope
//*					talking to an SkyWatcher DQ6
//*					via serial port
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
//*	Jan 30,	2021	<MLS> Created telescopedriver_skywatch.cpp
//*	Mar 31,	2021	<MLS> A bunch of work on EQ6 support
//*	Mar 31,	2021	<MLS> Added SendCmdsFromQueue()
//*****************************************************************************


#ifdef _ENABLE_TELESCOPE_SKYWATCH_

#include	<ctype.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<math.h>

#include	<termios.h>
#include	<fcntl.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"serialport.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"


#include	"telescopedriver.h"
#include	"telescopedriver_skywatch.h"


//**************************************************************************************
void	CreateTelescopeObjects_SkyWatch(void)
{
	new TelescopeDriverSkyWatch(kDevCon_Serial, "/dev/ttyS0");
}


//**************************************************************************************
//*		/dev/ttyS0
//**************************************************************************************
TelescopeDriverSkyWatch::TelescopeDriverSkyWatch(DeviceConnectionType connectionType, const char *devicePath)
	:TelescopeDriverComm(connectionType, devicePath)
{

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name,		"Telescope-SkyWatch");
	strcpy(cCommonProp.Description,	"Telescope control using SkyWatch protocol");


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
TelescopeDriverSkyWatch::~TelescopeDriverSkyWatch(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//**************************************************************************************
bool	TelescopeDriverSkyWatch::SendCmdsFromQueue(void)
{
int		returnByteCNt;
char	returnBuffer[500];
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
	while (cQueuedCmdCnt > 0)
	{
		CONSOLE_DEBUG_W_STR("Sending", cCmdQueue[0].cmdString);
//		returnByteCNt	=	LX200_SendCommand(	cSocket_desc,
//												cCmdQueue[0].cmdString,
//												returnBuffer,
//												400);
		if (returnByteCNt > 0)
		{
			CONSOLE_DEBUG_W_STR("returnBuffer\t=", returnBuffer);
		}
		for (iii=0; iii<cQueuedCmdCnt; iii++)
		{
			cCmdQueue[iii]	=	cCmdQueue[iii + 1];
		}
		cQueuedCmdCnt--;
		if (cQueuedCmdCnt > 0)
		{
			usleep(500);
		}
	}
	return(false);
}




//**************************************************************************************
bool	TelescopeDriverSkyWatch::SendCmdsPeriodic(void)
{
bool	isValid;
char	receiveBuff[64];

//	CONSOLE_DEBUG(__FUNCTION__);
	isValid			=	false;
	receiveBuff[0]	=	0;
	//--------------------------------------------------------------------------
	//*	Right Ascension
	Serial_Send_Data(cDeviceConnFileDesc, ":j1\r", true);
	Serial_Read_Data(cDeviceConnFileDesc, receiveBuff, 32);
//	CONSOLE_DEBUG_W_STR("receiveBuff\t=", receiveBuff);

	//--------------------------------------------------------------------------
	//*	Declination
	Serial_Send_Data(cDeviceConnFileDesc, ":j2\r", true);
	Serial_Read_Data(cDeviceConnFileDesc, receiveBuff, 32);
//	CONSOLE_DEBUG_W_STR("receiveBuff\t=", receiveBuff);


	return(isValid);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverSkyWatch::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverSkyWatch::Telescope_MoveAxis(const int axisNum, const double moveRate_degPerSec, char *alpacaErrMsg)
{
	TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverSkyWatch::Telescope_SlewToRA_DEC(	const double	newRtAscen_Hours,
																	const double	newDeclination_Degrees,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverSkyWatch::Telescope_SyncToRA_DEC(	const double	newRtAscen_Hours,
																	const double	newDeclination_Degrees,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverSkyWatch::Telescope_TrackingOnOff(const bool newTrackingState, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}


#endif // _ENABLE_TELESCOPE_LX200_
