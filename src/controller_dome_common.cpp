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

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"alpacadriver_helper.h"
#include	"controller.h"



#ifdef _ENABLE_SKYTRAVEL_
	#define	PARENT_CLASS	ControllerSkytravel
	#include	"controller_skytravel.h"
	#include	"SkyTravelExternal.h"
#elif defined(_ENABLE_CTRL_DOME_)
	#define	PARENT_CLASS	ControllerDome
	#include	"controller_dome.h"
#endif

//*****************************************************************************
bool	PARENT_CLASS::AlpacaGetStartupData(void)
{
//SJP_Parser_t	jsonParser;
bool			validData;
char			returnString[128];
//char			dataString[128];

	CONSOLE_DEBUG(__FUNCTION__);
	//===============================================================
	//*	get supportedactions
#ifdef _ENABLE_SKYTRAVEL_
	validData	=	AlpacaGetSupportedActions(&cDomeIpAddress, cDomeIpPort, "dome", cDomeAlpacaDeviceNum);
#else
	validData	=	AlpacaGetSupportedActions("dome", cAlpacaDevNum);
#endif // _ENABLE_SKYTRAVEL_
	if (validData)
	{
	#ifdef _ENABLE_SKYTRAVEL_
		SetWidgetValid(kTab_Dome,			kDomeBox_Readall,		cDomeHas_readall);
		if (cDomeHas_readall == false)
		{
			cDeviceAddress	=	cDomeIpAddress;
			cPort			=	cDomeIpPort;
			cAlpacaDevNum	=	cDomeAlpacaDeviceNum;
			validData	=	AlpacaGetStringValue(	"dome", "driverversion",	NULL,	returnString);
			if (validData)
			{
				strcpy(cAlpacaVersionString, returnString);
				SetWidgetText(kTab_Dome,		kDomeBox_AlpacaDrvrVersion,		cAlpacaVersionString);
			}
		}
	#else
		SetWidgetValid(kTab_Dome,			kDomeBox_Readall,		cHas_readall);
		SetWidgetValid(kTab_SlitTracker,	kSlitTracker_Readall,	cHas_readall);
		SetWidgetValid(kTab_SlitGraph,		kSlitGraph_Readall,		cHas_readall);
		SetWidgetValid(kTab_About,			kAboutBox_Readall,		cHas_readall);

		if (cHas_readall == false)
		{
			validData	=	AlpacaGetStringValue(	"dome", "driverversion",	NULL,	returnString);
			if (validData)
			{
				strcpy(cAlpacaVersionString, returnString);
				SetWidgetText(kTab_Dome,		kDomeBox_AlpacaDrvrVersion,		cAlpacaVersionString);
			}
		}
	#endif // _ENABLE_SKYTRAVEL_

	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}



	cLastUpdate_milliSecs	=	millis();

	return(validData);
}



//*****************************************************************************
//*	Get Status, One At A Time
//*****************************************************************************
bool	PARENT_CLASS::AlpacaGetStatus_OneAAT(void)
{
bool		validData;
int			myFailureCount;
double		argDouble;
int			argInt;

	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _ENABLE_SKYTRAVEL_
	cDeviceAddress	=	cDomeIpAddress;
	cPort			=	cDomeIpPort;
	cAlpacaDevNum	=	cDomeAlpacaDeviceNum;
#endif // _ENABLE_SKYTRAVEL_


	myFailureCount	=	0;
	//========================================================
	validData	=	AlpacaGetBooleanValue(	"dome", "athome",	NULL,	&cAtHome);
	if (validData)
	{
		CONSOLE_DEBUG_W_NUM("cAtHome\t=",	cAtHome);
	}
	else
	{
		cReadFailureCnt++;
		myFailureCount++;
	}
	//========================================================
	validData	=	AlpacaGetBooleanValue(	"dome", "atpark",	NULL,	&cAtPark);
	if (validData)
	{
		CONSOLE_DEBUG_W_NUM("cAtPark\t=",	cAtPark);
	}
	else
	{
		cReadFailureCnt++;
		myFailureCount++;
	}
	//========================================================
	validData	=	AlpacaGetBooleanValue(	"dome", "slewing",	NULL,	&cSlewing);
	if (validData)
	{
		CONSOLE_DEBUG_W_NUM("cSlewing\t=",	cSlewing);
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
		CONSOLE_DEBUG_W_DBL("argDouble\t=",	argDouble);
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
		CONSOLE_DEBUG_W_DBL("argDouble\t=",	argDouble);
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
		CONSOLE_DEBUG_W_NUM("argInt\t=",	argInt);
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
void	PARENT_CLASS::AlpacaProcessReadAllDome(	const int	deviceNum,
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
		SetWidgetText(kTab_Dome,		kDomeBox_AlpacaDrvrVersion,		cAlpacaVersionString);
	#ifndef _ENABLE_SKYTRAVEL_
		SetWidgetText(kTab_SlitTracker,	kSlitTracker_AlpacaDrvrVersion,	cAlpacaVersionString);
		SetWidgetText(kTab_SlitGraph,	kSlitGraph_AlpacaDrvrVersion,	cAlpacaVersionString);
		SetWidgetText(kTab_About,		kAboutBox_AlpacaDrvrVersion,	cAlpacaVersionString);
	#endif
	}
	else if (strcasecmp(keywordString, "canfindhome") == 0)
	{
		cCanFindHome	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "canpark") == 0)
	{
		cCanPark		=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "cansetaltitude") == 0)
	{
		cCanSetAltitude	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "cansetazimuth") == 0)
	{
		cCanSetAzimuth	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "cansetpark") == 0)
	{
		cCanSetPark		=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "cansetshutter") == 0)
	{
		cCanSetShutter	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "canslave") == 0)
	{
		cCanSlave		=	IsTrueFalse(valueString);
		if (cCanSlave)
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
		cCanSyncAzimuth	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "slaved") == 0)
	{
		cSlaved			=	IsTrueFalse(valueString);
		if (cCanSlave)
		{
			SetWidgetText(kTab_Dome,	kDomeBox_SlavedStatus,		(cSlaved ? "Yes" : "No"));
			SetWidgetText(kTab_Dome,	kDomeBox_ToggleSlaveMode, 	(cSlaved ? "Disable Slave mode" : "Enable Slave mode"));
		}
	}
	else if (strcasecmp(keywordString, "athome") == 0)
	{
		cAtHome			=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "atpark") == 0)
	{
		cAtPark			=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "azimuth") == 0)
	{
		argDouble	=	atof(valueString);
		UpdateDomeAzimuth(argDouble);
	}
	else if (strcasecmp(keywordString, "altitude") == 0)
	{
		//*	if we are talking to the shutter directly dont update from here
//		if (cShutterInfoValid == false)
		{
		double	myNewAltitude;

			myNewAltitude	=	atof(valueString);
			UpdateShutterAltitude(myNewAltitude);
		}
	}
	else if (strcasecmp(keywordString, "slewing") == 0)
	{
		cSlewing		=	IsTrueFalse(valueString);
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
}


//*****************************************************************************
void	PARENT_CLASS::AlpacaProcessReadAll(	const char	*deviceType,
											const int	deviceNum,
											const char	*keywordString,
											const char *valueString)
{
//	CONSOLE_DEBUG_W_2STR("json=",	keywordString, valueString);
	if (strcasecmp(deviceType, "Dome") == 0)
	{
		AlpacaProcessReadAllDome(deviceNum, keywordString, valueString);
	}
#ifdef _ENABLE_SKYTRAVEL_
	else if (strcasecmp(valueString, "Telescope") == 0)
	{
		//*	you get the idea
	}
#endif // _ENABLE_SKYTRAVEL_
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
//*	if this routine gets overloaded, the first part, checking for "readall" must be preserved
//*****************************************************************************
void	PARENT_CLASS::AlpacaProcessSupportedActions(const char *deviceType, const int deviveNum, const char *valueString)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceType);

	if (strcasecmp(deviceType, "Dome") == 0)
	{
		AlpacaProcessSupportedActions_Dome(deviveNum, valueString);
	}
	else if (strcasecmp(valueString, "Telescope") == 0)
	{
		//*	you get the idea
	}
}


//*****************************************************************************
void	PARENT_CLASS::UpdateDomeAzimuth(const double newAzimuth)
{
char	textString[32];

	cAzimuth_Dbl	=	newAzimuth;
	sprintf(textString, "%1.1f", cAzimuth_Dbl);
	SetWidgetText(kTab_Dome, kDomeBox_Azimuth, textString);
#ifdef _ENABLE_SKYTRAVEL_

	//*	skytravel is backwards from Alpaca
	gDomeAzimuth_degrees	=	360.0 - newAzimuth;
#endif
}


//*****************************************************************************
void	PARENT_CLASS::UpdateShutterAltitude(const double newAltitude)
{
char	textString[32];

	cAltitude_Dbl	=	newAltitude;
	sprintf(textString, "%1.1f %%", cAltitude_Dbl);
	SetWidgetText(kTab_Dome, kDomeBox_Altitude, textString);
}

//*****************************************************************************
void	PARENT_CLASS::UpdateShutterStatus(const int newShutterStatus)
{
char	statusString[16];

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("newShutterStatus\t=", newShutterStatus);

	if (newShutterStatus != cShutterStatus)
	{
		cShutterStatus	=	newShutterStatus;
		switch(cShutterStatus)
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
	SetWidgetText(kTab_Dome, kDomeBox_ErrorMsg, errorMsgString);
}


#endif // defined(_ENABLE_SKYTRAVEL_) || defined(_ENABLE_CTRL_DOME_)
