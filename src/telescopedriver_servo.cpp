//**************************************************************************
//*	Name:			telescopedriver_servo.cpp
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
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar 25,	2022	<MLS> Created telescopedriver_Servo.cpp
//*	Mar 25,	2022	<MLS> Servo version is being created for Ron S <RNS>
//*	Apr  6,	2022	<MLS> Switched to using generic config reading routines
//*	May 11,	2022	<MLS> Now using servo config processing routines by <RNS>
//*	May 11,	2022	<MLS> Added OutputHTML_Part2() to telescope servo driver
//*****************************************************************************
//*	LM628/629 Si,
//*****************************************************************************

#ifdef _ENABLE_TELESCOPE_SERVO_

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
//#include	"helper_functions.h"
//#include	"readconfigfile.h"

//*	servo controller routines using RoboClaws controller
#include	"servo_scope_cfg.h"
#include	"servo_scope.h"
#include	"servo_std_defs.h"
#include	"servo_time.h"

#include	"telescopedriver.h"
#include	"telescopedriver_servo.h"


static void	ProcessServoConfig(const char *keyword, const char *value);

//**************************************************************************************
TelescopeDriverServo::TelescopeDriverServo(void)
	:TelescopeDriver()
{
int	cfgStatus;

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name,		"AlpacaPi-Mount-Servo");
	strcpy(cCommonProp.Description,	"Mount using RoboClaw servo controller");

	//*	setup the options for this driver
	cTelescopeProp.AlginmentMode	=	kAlignmentMode_algGermanPolar;
	cTelescopeProp.CanFindHome		=	false;
	cTelescopeProp.CanMoveAxis		=	true;
	cTelescopeProp.CanSetTracking	=	true;
	cTelescopeProp.CanSlewAsync		=	false;
	cTelescopeProp.CanSync			=	true;
	cTelescopeProp.CanUnpark		=	false;


//	ReadGenericConfigFile("ss_scope.cfg", ':', &ProcessServoConfig);
#define kLOCAL_CFG_FILE "servo_location.cfg"

	cServoConfigIsValid	=	false;
//	cfgStatus			=	Servo_Read_Scope_Cfg(NULL, &cServoConfig);
	cfgStatus			=	Servo_init(kSCOPE_CFG_FILE, kLOCAL_CFG_FILE);
	if (cfgStatus == 0)
	{
		cServoConfigIsValid	=	true;
	}
	CONSOLE_DEBUG_W_NUM("cServoConfigIsValid\t=", cServoConfigIsValid);

	AlpacaConnect();

	CONSOLE_DEBUG_W_NUM("cTelescopeProp.CanUnpark\t=", cTelescopeProp.CanUnpark);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
TelescopeDriverServo::~TelescopeDriverServo(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	AlpacaDisConnect();
}

//*****************************************************************************
void	TelescopeDriverServo::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
char		lineBuffer[512];
FILE		*filePointer;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	print the config file to the web page,
	//*	this makes it much easier to see what the config is on a remote system

	filePointer	=	fopen(kLOCAL_CFG_FILE, "r");
	if (filePointer != NULL)
	{
		SocketWriteData(reqData->socket, "<HR>\r\n");
		SocketWriteData(reqData->socket, "<P><CENTER>Location Config File</CENTER>\r\n");

		sprintf(lineBuffer, "Config file:%ss<P>\r\n",	kLOCAL_CFG_FILE);
		SocketWriteData(reqData->socket, lineBuffer);

		SocketWriteData(reqData->socket, "<PRE>\r\n");
		while (fgets(lineBuffer, 500, filePointer) != NULL)
		{
			SocketWriteData(reqData->socket, lineBuffer);
		}
		SocketWriteData(reqData->socket, "</PRE>\r\n");

		fclose(filePointer);
	}


	filePointer	=	fopen(kSCOPE_CFG_FILE, "r");
	if (filePointer != NULL)
	{
		SocketWriteData(reqData->socket, "<HR>\r\n");
		SocketWriteData(reqData->socket, "<P><CENTER>Servo Config File</CENTER>\r\n");

		sprintf(lineBuffer, "Config file:%s is %s<P>\r\n",	kSCOPE_CFG_FILE,
															(cServoConfigIsValid ? "valid" : "NOT valid"));
		SocketWriteData(reqData->socket, lineBuffer);

		SocketWriteData(reqData->socket, "<PRE>\r\n");
		while (fgets(lineBuffer, 500, filePointer) != NULL)
		{
			SocketWriteData(reqData->socket, lineBuffer);
		}
		SocketWriteData(reqData->socket, "</PRE>\r\n");

		fclose(filePointer);
	}
}

//*****************************************************************************
//*	returns delay time in micro-seconds
//*****************************************************************************
int32_t	TelescopeDriverServo::RunStateMachine(void)
{
TYPE_MOVE	currentMoveState;

	//*	this is where your periodic code goes
	//*	update cTelescopeProp values here

	CONSOLE_DEBUG(__FUNCTION__);

#warning "Ron, you want to put your update status calls here"
//	currentMoveState	=	Servo_state();

	//*	5 * 1000 * 1000 means you might not get called again for 5 seconds
	//*	you might get called earlier
	return(5 * 1000 * 1000);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode;

	alpacaErrCode	=	kASCOM_Err_Success;
	Servo_stop_all();

	cTelescopeProp.Slewing	=	false;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_FindHome(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_MoveAxis(const int axisNum, const double moveRate_degPerSec, char *alpacaErrMsg)
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
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_Park(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_SetPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_SlewToAltAz(const double newAlt_Degrees, const double newAz_Degrees, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_SlewToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_SyncToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_TrackingOnOff(	const bool	newTrackingState,
																	char		*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					servoStatus;

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	kASCOM_Err_Success;
	if (newTrackingState)
	{
		servoStatus	=	Servo_start_tracking(SERVO_RA_AXIS);
		if (servoStatus != kSTATUS_OK)
		{
		//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_start_tracking failed");
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not connected");
			alpacaErrCode	=	kASCOM_Err_NotConnected;
		}
	}
	else
	{
		servoStatus	=	Servo_stop_tracking(SERVO_RA_AXIS);
		if (servoStatus != kSTATUS_OK)
		{
		//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_stop_tracking failed");
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not connected");
			alpacaErrCode	=	kASCOM_Err_NotConnected;
		}
	}

	if (servoStatus == kSTATUS_OK)
	{
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_UnPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);

}



#endif // _ENABLE_TELESCOPE_SERVO_
