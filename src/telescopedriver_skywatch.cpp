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
	:TelescopeDriver()
{

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cDeviceName,			"Telescope-SkyWatch");
	strcpy(cDeviceDescription,	"Telescope control using SkyWatch protocol");

	cDeviceConnType	=	connectionType;
	strcpy(cDeviceConnPath,	devicePath);

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
int32_t	TelescopeDriverSkyWatch::RunStateMachine(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	return(15 * 1000 * 1000);
}

//*****************************************************************************
bool	TelescopeDriverSkyWatch::AlpacaConnect(void)
{
bool	openOK;

	CONSOLE_DEBUG(__FUNCTION__);

	openOK	=	false;
	switch(cDeviceConnType)
	{
		case kDevCon_Ethernet:
			break;

		case kDevCon_USB:
			break;

		case kDevCon_Serial:
			cDeviceConnFileDesc	=	open(cDeviceConnPath, O_RDWR);	//* connect to port
			if (cDeviceConnFileDesc >= 0)
			{
				openOK	=	true;
				Set_Serial_attribs(cDeviceConnFileDesc, B9600, 0);	//*	set the baud rate
			}
			else
			{
				CONSOLE_DEBUG_W_STR("failed to open", cDeviceConnPath);
				openOK	=	false;
			}
			break;
	}
	return(openOK);
}

//*****************************************************************************
bool	TelescopeDriverSkyWatch::AlpacaDisConnect(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	return(true);
}

//*****************************************************************************
//*	needs to be over-ridden
TYPE_ASCOM_STATUS	TelescopeDriverSkyWatch::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);


//	alpacaErrCode	=	kASCOM_Err_Success;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverSkyWatch::Telescope_SlewToRA_DEC(	const double	newRA,
																	const double	newDec,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

//		alpacaErrCode			=	kASCOM_Err_Success;

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverSkyWatch::Telescope_SyncToRA_DEC(	const double	newRA,
																	const double	newDec,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
bool				lx2000ReturnCode;


	return(alpacaErrCode);
}

#endif // _ENABLE_TELESCOPE_LX200_
