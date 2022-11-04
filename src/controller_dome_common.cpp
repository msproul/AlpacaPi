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
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jan 10,	2021	<MLS> Created controller_dome_common.cpp
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


#ifndef _HELPER_FUNCTIONS_H_
	#include	"helper_functions.h"
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
		UpdateShutterStatus(argInt);
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
void	PARENT_CLASS::AlpacaProcessReadAll_Dome(const int	deviceNum,
												const char	*keywordString,
												const char	*valueString)
{
double	argDouble;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR(keywordString, valueString);

	if (strcasecmp(keywordString, "version") == 0)
	{
		//*	"version": "AlpacaPi - V0.2.2-beta build #32",
		strcpy(cAlpacaVersionString, valueString);
	}
	else if (strcasecmp(keywordString, "canfindhome") == 0)
	{
		cDomeProp.CanFindHome	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "canpark") == 0)
	{
		cDomeProp.CanPark		=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "cansetaltitude") == 0)
	{
		cDomeProp.CanSetAltitude	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "cansetazimuth") == 0)
	{
		cDomeProp.CanSetAzimuth	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "cansetpark") == 0)
	{
		cDomeProp.CanSetPark		=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "cansetshutter") == 0)
	{
		cDomeProp.CanSetShutter	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "canslave") == 0)
	{
		cDomeProp.CanSlave		=	IsTrueFalse(valueString);
		if (cDomeProp.CanSlave)
		{
			SetWidgetBGColor(kTab_Dome, kDomeBox_ToggleSlaveMode,	CV_RGB(255,	255,	255));
		}
		else
		{
			SetWidgetText(kTab_Dome,	kDomeBox_SlavedStatus,	"N/A");
			SetWidgetBGColor(kTab_Dome, kDomeBox_ToggleSlaveMode,	CV_RGB(128,	128,	128));
		}
	}
	else if (strcasecmp(keywordString, "cansyncazimuth") == 0)
	{
//		CONSOLE_DEBUG_W_STR(keywordString, valueString);
		cDomeProp.CanSyncAzimuth	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "slaved") == 0)
	{
		cDomeProp.Slaved			=	IsTrueFalse(valueString);
		if (cDomeProp.CanSlave)
		{
			SetWidgetText(kTab_Dome,	kDomeBox_SlavedStatus,		(cDomeProp.Slaved ? "Yes" : "No"));
			SetWidgetText(kTab_Dome,	kDomeBox_ToggleSlaveMode, 	(cDomeProp.Slaved ? "Disable Slave mode" : "Enable Slave mode"));
		}
	}
	else if (strcasecmp(keywordString, "athome") == 0)
	{
		cDomeProp.AtHome			=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "atpark") == 0)
	{
		cDomeProp.AtPark			=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "azimuth") == 0)
	{
		argDouble	=	AsciiToDouble(valueString);
		UpdateDomeAzimuth(argDouble);
	}
	else if (strcasecmp(keywordString, "altitude") == 0)
	{
		//*	if we are talking to the shutter directly dont update from here
//		if (cShutterInfoValid == false)
		{
		double	myNewAltitude;

			myNewAltitude	=	AsciiToDouble(valueString);
			UpdateShutterAltitude(myNewAltitude);
		}
	}
	else if (strcasecmp(keywordString, "slewing") == 0)
	{
		cDomeProp.Slewing		=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "shutterstatus") == 0)
	{
		//*	if we are talking to the shutter directly dont update from here
//		if (cShutterInfoValid == false)
		{
		int		newShutterStatus;

			newShutterStatus	=	atoi(valueString);
			UpdateShutterStatus(newShutterStatus);
		}
	}
	else
	{
		//*	process the common stuff
		AlpacaProcessReadAll_Common(	"dome",
										deviceNum,
										keywordString,
										valueString);
	}
}




//*****************************************************************************
void	PARENT_CLASS::AlpacaProcessSupportedActions_Dome(const int deviveNum, const char *valueString)
{
//	CONSOLE_DEBUG_W_STR("valueString=",	valueString);

	if (strcasecmp(valueString, "readall") == 0)
	{
#ifdef _ENABLE_SKYTRAVEL_
		cDomeHas_readall	=	true;
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
void	PARENT_CLASS::UpdateShutterStatus(const int newShutterStatus)
{
char	statusString[16];

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("newShutterStatus\t=", newShutterStatus);

	if (newShutterStatus != cDomeProp.ShutterStatus)
	{
		cDomeProp.ShutterStatus	=	newShutterStatus;
		switch(cDomeProp.ShutterStatus)
		{
			case kShutterStatus_Open:		strcpy(statusString,	"Open");	break;
			case kShutterStatus_Closed:		strcpy(statusString,	"Closed");	break;
			case kShutterStatus_Opening:	strcpy(statusString,	"Opening");	break;
			case kShutterStatus_Closing:	strcpy(statusString,	"Closing");	break;
			case kShutterStatus_Error:		strcpy(statusString,	"Error");	break;

			case kShutterStatus_Unknown:
			default:						strcpy(statusString,	"unknown");	break;
		}

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
