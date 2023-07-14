//*****************************************************************************
//*		controller_dome_common.cpp		(c) 2021 by Mark Sproul
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jan 10,	2021	<MLS> Created controller_dome_common.cpp
//*	Jun 28,	2023	<MLS> Switched to table based processall
//*	Jun 28,	2023	<MLS> Added AlpacaProcessReadAllIdx_Dome()
//*****************************************************************************

#if defined(_ENABLE_SKYTRAVEL_) || defined(_ENABLE_CTRL_DOME_)


#ifndef PARENT_CLASS
	#define _ENABLE_CONSOLE_DEBUG_
	#include	"ConsoleDebug.h"

	#include	"alpaca_defs.h"
	#include	"alpacadriver_helper.h"
	#include	"helper_functions.h"
	#include	"controller.h"

	#ifdef _ENABLE_SKYTRAVEL_
		#define	PARENT_CLASS	ControllerSkytravel
		#include	"controller_skytravel.h"
		#include	"SkyTravelExternal.h"

		#define	kTab_Dome kTab_ST_Dome
	#elif defined(_ENABLE_CTRL_DOME_)
		#define	PARENT_CLASS	ControllerDome
		#include	"controller_dome.h"
	#endif
#endif

#ifndef	_WINDOWTAB_DOME_H_
	#include	"windowtab_dome.h"
#endif

#ifndef kTab_Dome
	#define	kTab_Dome	1
#endif

#ifndef _HELPER_FUNCTIONS_H_
	#include	"helper_functions.h"
#endif

#include	"dome_AlpacaCmds.h"

#ifndef _ENABLE_SKYTRAVEL_
	#include	"dome_AlpacaCmds.cpp"
#endif

//*****************************************************************************
//*	Get Status, One At A Time
//*****************************************************************************
bool	PARENT_CLASS::AlpacaGetStatus_DomeOneAAT(void)
{
bool		validData;
int			myFailureCount;
double		argDouble;
int			argInt;

//	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _ENABLE_SKYTRAVEL_
	cDeviceAddress	=	cDomeIpAddress;
	cPort			=	cDomeIpPort;
	cAlpacaDevNum	=	cDomeAlpacaDeviceNum;
#endif // _ENABLE_SKYTRAVEL_


	myFailureCount	=	0;
	//========================================================
	validData	=	AlpacaGetBooleanValue(	"dome", "athome",	NULL,	&cDomeProp.AtHome);
	if (validData)
	{
//		CONSOLE_DEBUG_W_NUM("cAtHome\t=",	cDomeProp.AtHome);
	}
	else
	{
		cReadFailureCnt++;
		myFailureCount++;
	}
	//========================================================
	validData	=	AlpacaGetBooleanValue(	"dome", "atpark",	NULL,	&cDomeProp.AtPark);
	if (validData)
	{
//		CONSOLE_DEBUG_W_NUM("cAtPark\t=",	cDomeProp.AtPark);
	}
	else
	{
		cReadFailureCnt++;
		myFailureCount++;
	}
	//========================================================
	validData	=	AlpacaGetBooleanValue(	"dome", "slewing",	NULL,	&cDomeProp.Slewing);
	if (validData)
	{
//		CONSOLE_DEBUG_W_NUM("cSlewing\t=",	cDomeProp.Slewing);
	}
	else
	{
		cReadFailureCnt++;
		myFailureCount++;
	}

	//========================================================
	validData	=	AlpacaGetDoubleValue(	"dome", "azimuth",	NULL,	&argDouble);
	if (validData)
	{
//		CONSOLE_DEBUG_W_DBL("argDouble\t=",	argDouble);
		UpdateDomeAzimuth(argDouble);
	}
	else
	{
		cReadFailureCnt++;
		myFailureCount++;
	}

	//========================================================
	validData	=	AlpacaGetDoubleValue(	"dome", "altitude",	NULL,	&argDouble);
	if (validData)
	{
//		CONSOLE_DEBUG_W_DBL("argDouble\t=",	argDouble);
		UpdateShutterAltitude(argDouble);
	}
	else
	{
		cReadFailureCnt++;
		myFailureCount++;
	}

	//========================================================
	validData	=	AlpacaGetIntegerValue(	"dome", "shutterstatus",	NULL,	&argInt);
	if (validData)
	{
//		CONSOLE_DEBUG_W_NUM("argInt\t=",	argInt);
		UpdateShutterStatus((TYPE_ShutterStatus)argInt);
	}
	else
	{
		cReadFailureCnt++;
		myFailureCount++;
	}


	if (myFailureCount < 2)
	{
		validData	=	true;
	}
	else
	{
		validData	=	false;
	}
	return(validData);
}

//*****************************************************************************
bool	PARENT_CLASS::AlpacaProcessReadAllIdx_Dome(	const int	deviceNum,
													const int	keywordEnum,
													const char	*valueString)
{
bool	dataWasHandled	=	true;
double	argDouble;

	switch(keywordEnum)
	{
		case kCmd_Dome_altitude:		//*	The dome altitude
			{
			double	myNewAltitude;

				myNewAltitude	=	AsciiToDouble(valueString);
				UpdateShutterAltitude(myNewAltitude);
			}
			break;

		case kCmd_Dome_athome:			//*	Indicates whether the dome is in the home position.
			cDomeProp.AtHome		=	IsTrueFalse(valueString);
			break;

		case kCmd_Dome_atpark:			//*	Indicates whether the telescope is at the park position
			cDomeProp.AtPark		=	IsTrueFalse(valueString);
			break;

		case kCmd_Dome_azimuth:			//*	The dome azimuth
			argDouble	=	AsciiToDouble(valueString);
			UpdateDomeAzimuth(argDouble);
			break;

		case kCmd_Dome_canfindhome:		//*	Indicates whether the dome can find the home position.
			cDomeProp.CanFindHome	=	IsTrueFalse(valueString);
			break;

		case kCmd_Dome_canpark:			//*	Indicates whether the dome can be parked.
			cDomeProp.CanPark		=	IsTrueFalse(valueString);
			break;

		case kCmd_Dome_cansetaltitude:	//*	Indicates whether the dome altitude can be set
			cDomeProp.CanSetAltitude	=	IsTrueFalse(valueString);
			break;

		case kCmd_Dome_cansetazimuth:	//*	Indicates whether the dome azimuth can be set
			cDomeProp.CanSetAzimuth	=	IsTrueFalse(valueString);
			break;

		case kCmd_Dome_cansetpark:		//*	Indicates whether the dome park position can be set
			cDomeProp.CanSetPark		=	IsTrueFalse(valueString);
			break;

		case kCmd_Dome_cansetshutter:	//*	Indicates whether the dome shutter can be opened
			cDomeProp.CanSetShutter		=	IsTrueFalse(valueString);
			break;

		case kCmd_Dome_canslave:		//*	Indicates whether the dome supports slaving to a telescope
			cDomeProp.CanSlave			=	IsTrueFalse(valueString);
			if (cDomeProp.CanSlave)
			{
				SetWidgetBGColor(	kTab_Dome, kDomeBox_ToggleSlaveMode,	CV_RGB(255,	255,	255));
			}
			else
			{
				SetWidgetText(		kTab_Dome,	kDomeBox_SlavedStatus,	"N/A");
				SetWidgetBGColor(	kTab_Dome, kDomeBox_ToggleSlaveMode,	CV_RGB(128,	128,	128));
			}
			break;

		case kCmd_Dome_cansyncazimuth:	//*	Indicates whether the dome azimuth position can be synched
			cDomeProp.CanSyncAzimuth	=	IsTrueFalse(valueString);
			break;

		case kCmd_Dome_shutterstatus:	//*	Status of the dome shutter or roll-off roof
			{
			TYPE_ShutterStatus	newShutterStatus;

				newShutterStatus	=	(TYPE_ShutterStatus)atoi(valueString);
				UpdateShutterStatus(newShutterStatus);
			}
			break;

		case kCmd_Dome_slaved:			//*	GET--Indicates whether the dome is slaved to the telescope
			cDomeProp.Slaved			=	IsTrueFalse(valueString);
			if (cDomeProp.CanSlave)
			{
				SetWidgetText(kTab_Dome,	kDomeBox_SlavedStatus,		(cDomeProp.Slaved ? "Yes" : "No"));
				SetWidgetText(kTab_Dome,	kDomeBox_ToggleSlaveMode, 	(cDomeProp.Slaved ? "Disable Slave mode" : "Enable Slave mode"));
			}
			break;

		case kCmd_Dome_slewing:			//*	Indicates whether the any part of the dome is moving
			cDomeProp.Slewing		=	IsTrueFalse(valueString);
			break;

		//==============================================================
		//*	extra commands added by MLS
		case kCmd_Dome_powerstatus:		//*	Return power status
			break;

		case kCmd_Dome_auxiliarystatus:	//*	Return auxiliary status
			break;

		case kCmd_Dome_currentstate:	//*	What is the current state of the state machine
			break;

		default:
			dataWasHandled	=	false;
			CONSOLE_DEBUG_W_NUM("keywordEnum\t=", keywordEnum);
			CONSOLE_DEBUG_W_STR("valueString\t=", valueString);
			break;

	}
	return(dataWasHandled);
}

//*****************************************************************************
void	PARENT_CLASS::AlpacaProcessSupportedActions_Dome(const int deviveNum, const char *valueString)
{
//	CONSOLE_DEBUG_W_STR("valueString=",	valueString);

	if (strcasecmp(valueString, "readall") == 0)
	{
#ifdef _ENABLE_SKYTRAVEL_
		cDomeHas_Readall	=	true;
#else
		cHas_readall	=	true;
#endif
	}
	else if (strcasecmp(valueString, "findhome") == 0)
	{
		SetWidgetValid(		kTab_Dome,	kDomeBox_GoHome,	true);
		SetWidgetBGColor(	kTab_Dome,	kDomeBox_GoHome,	CV_RGB(255,	255,	255));
	}
	else if (strcasecmp(valueString, "park") == 0)
	{
		SetWidgetValid(		kTab_Dome,	kDomeBox_GoPark,	true);
		SetWidgetBGColor(	kTab_Dome,	kDomeBox_GoPark,	CV_RGB(255,	255,	255));
	}
	else if (strcasecmp(valueString, "goleft") == 0)
	{
		SetWidgetValid(		kTab_Dome,	kDomeBox_GoLeft,	true);
		SetWidgetBGColor(	kTab_Dome,	kDomeBox_GoLeft,	CV_RGB(255,	255,	255));
	}
	else if (strcasecmp(valueString, "goright") == 0)
	{
		SetWidgetValid(		kTab_Dome,	kDomeBox_GoRight,	true);
		SetWidgetBGColor(	kTab_Dome,	kDomeBox_GoRight,	CV_RGB(255,	255,	255));
	}
	else if (strcasecmp(valueString, "slowleft") == 0)
	{
		SetWidgetValid(		kTab_Dome,	kDomeBox_SlowLeft,	true);
		SetWidgetBGColor(	kTab_Dome,	kDomeBox_SlowLeft,	CV_RGB(255,	255,	255));
	}
	else if (strcasecmp(valueString, "slowright") == 0)
	{
		SetWidgetValid(		kTab_Dome,	kDomeBox_SlowRight,	true);
		SetWidgetBGColor(	kTab_Dome,	kDomeBox_SlowRight,	CV_RGB(255,	255,	255));
	}
	else if (strcasecmp(valueString, "bumpLeft") == 0)
	{
		SetWidgetValid(		kTab_Dome,	kDomeBox_BumpLeft,	true);
		SetWidgetBGColor(	kTab_Dome,	kDomeBox_BumpLeft,	CV_RGB(255,	255,	255));
	}
	else if (strcasecmp(valueString, "bumpright") == 0)
	{
		SetWidgetValid(		kTab_Dome,	kDomeBox_BumpRight,	true);
		SetWidgetBGColor(	kTab_Dome,	kDomeBox_BumpRight,	CV_RGB(255,	255,	255));
	}
	else if (strcasecmp(valueString, "abortslew") == 0)
	{
		SetWidgetValid(		kTab_Dome,	kDomeBox_Stop,	true);
	//	SetWidgetBGColor(	kTab_Dome,	kDomeBox_Stop,	CV_RGB(255,	255,	255));
	}
	else if (strcasecmp(valueString, "slaved") == 0)
	{
		SetWidgetValid(		kTab_Dome,	kDomeBox_ToggleSlaveMode,	true);
	//	SetWidgetBGColor(	kTab_Dome,	kDomeBox_Stop,	CV_RGB(255,	255,	255));
	}
}



//*****************************************************************************
void	PARENT_CLASS::UpdateDomeAzimuth(const double newAzimuth)
{
char	textString[32];

//	CONSOLE_DEBUG_W_STR("cWindowName\t=",	cWindowName);

	cDomeProp.Azimuth	=	newAzimuth;
	sprintf(textString, "%1.1f", cDomeProp.Azimuth);
	SetWidgetText(kTab_Dome, kDomeBox_Azimuth, textString);

//	CONSOLE_DEBUG_W_DBL("cDomeProp.Azimuth\t=", cDomeProp.Azimuth);

#ifdef _ENABLE_SKYTRAVEL_
	//*	skytravel is backwards from Alpaca
//	gDomeAzimuth_degrees	=	360.0 - newAzimuth;
	gDomeAzimuth_degrees	=	newAzimuth;
#endif
}


//*****************************************************************************
void	PARENT_CLASS::UpdateShutterAltitude(const double newAltitude)
{
char	textString[32];

	cDomeProp.Altitude	=	newAltitude;
	sprintf(textString, "%1.1f %%", cDomeProp.Altitude);
	SetWidgetText(kTab_Dome, kDomeBox_Altitude, textString);
}

//*****************************************************************************
void	PARENT_CLASS::UpdateShutterStatus(const TYPE_ShutterStatus newShutterStatus)
{
char	statusString[16];

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("newShutterStatus\t=", newShutterStatus);

	if (newShutterStatus != cDomeProp.ShutterStatus)
	{
		cDomeProp.ShutterStatus	=	newShutterStatus;
		GetDomeShutterStatusString(cDomeProp.ShutterStatus, statusString);
		SetWidgetText(kTab_Dome, kDomeBox_ShutterStatus, statusString);
	}
//	else
//	{
//		CONSOLE_DEBUG("Did not update");
//	}
}


//*****************************************************************************
void	PARENT_CLASS::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
	CONSOLE_DEBUG_W_STR("Alpaca error=", errorMsgString);
	SetWidgetText(kTab_Dome, kDomeBox_AlpacaErrorMsg, errorMsgString);
#ifdef _ENABLE_SKYTRAVEL_
	SetWidgetTextColor(	kTab_SkyTravel, kSkyTravel_MsgTextBox, CV_RGB(255, 0, 0));
	SetWidgetText(		kTab_SkyTravel, kSkyTravel_MsgTextBox, errorMsgString);
#endif
}


#endif // defined(_ENABLE_SKYTRAVEL_) || defined(_ENABLE_CTRL_DOME_)
