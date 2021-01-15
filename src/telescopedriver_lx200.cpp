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

	//*	set the defaults, everything to false or zero
	cAlginmentMode				=	kAlignmentMode_algGermanPolar;
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


}


#endif // _ENABLE_TELESCOPE_LX200_
