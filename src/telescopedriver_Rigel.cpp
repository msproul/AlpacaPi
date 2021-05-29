//**************************************************************************
//*	Name:			telescopedriver_Rigel.cpp
//*
//*	Author:			Mark Sproul (C) 2021
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*					This driver implements an Alpaca Telescope
//*					talking to an LX200 compatible mount
//*					via ethernet, USB or serial port
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
//*	Apr 20,	2021	<MLS> Created telescopedriver_Rigel.cpp
//*	Apr 20,	2021	<MLS> Rigel version is being created for Jim H <JMH>
//*****************************************************************************


#ifdef _ENABLE_TELESCOPE_RIGEL_

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
#include	"helper_functions.h"


#include	"telescopedriver.h"
#include	"telescopedriver_Rigel.h"

//**************************************************************************************
TelescopeDriverRigel::TelescopeDriverRigel(void)
	:TelescopeDriver()
{

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name,		"Telescope-Rigel");
	strcpy(cCommonProp.Description,	"Telescope control using Rigel protocol");

	//*	setup the options for this driver
	cTelescopeProp.AlginmentMode	=	kAlignmentMode_algGermanPolar;
	cTelescopeProp.CanSlewAsync		=	false;
	cTelescopeProp.CanSync			=	false;
	cTelescopeProp.CanSetTracking	=	false;
	cTelescopeProp.CanMoveAxis		=	false;
	cTelescopeProp.CanUnpark		=	false;

	AlpacaConnect();

	CONSOLE_DEBUG_W_NUM("cTelescopeProp.CanUnpark\t=", cTelescopeProp.CanUnpark);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
TelescopeDriverRigel::~TelescopeDriverRigel(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	AlpacaDisConnect();
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);

}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_FindHome(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_MoveAxis(const int axisNum, const double moveRate_degPerSec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);
	switch(axisNum)
	{
		case 0:
			if (moveRate_degPerSec > 0)
			{

			}
			else
			{

			}
			cTelescopeProp.Slewing	=	true;
			break;

		case 1:
			if (moveRate_degPerSec > 0)
			{

			}
			else
			{

			}
			cTelescopeProp.Slewing	=	true;
			break;

		default:
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
			break;

	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_Park(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_SetPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_SlewToAltAz(const double newAlt_Degrees, const double newAz_Degrees, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_SlewToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_SyncToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_TrackingOnOff(const bool newTrackingState, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

	if (newTrackingState)
	{

		alpacaErrCode	=	kASCOM_Err_NotImplemented;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");

	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_UnPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);

}




#endif // _ENABLE_TELESCOPE_RIGEL_
