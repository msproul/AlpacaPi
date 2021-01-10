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
#include	"controller.h"



#ifdef _ENABLE_SKYTRAVEL_
	#define	PARENT_CLASS	ControllerSkytravel
	#include	"controller_skytravel.h"
#elif defined(_ENABLE_CTRL_DOME_)
	#define	PARENT_CLASS	ControllerDome
	#include	"controller_dome.h"
#endif

//*****************************************************************************
bool	PARENT_CLASS::AlpacaGetStartupData(void)
{
//SJP_Parser_t	jsonParser;
bool			validData;
//char			alpacaString[128];
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
		SetWidgetValid(kTab_Dome,			kDomeBox_Readall,		cDomeHasReadAll);
	#else
		SetWidgetValid(kTab_Dome,			kDomeBox_Readall,		cHasReadAll);
		SetWidgetValid(kTab_SlitTracker,	kSlitTracker_Readall,	cHasReadAll);
		SetWidgetValid(kTab_SlitGraph,		kSlitGraph_Readall,		cHasReadAll);
		SetWidgetValid(kTab_About,			kAboutBox_Readall,		cHasReadAll);
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
	//	SetWidgetText(kTab_SlitTracker,	kSlitTracker_AlpacaDrvrVersion,	cAlpacaVersionString);
	//	SetWidgetText(kTab_SlitGraph,	kSlitGraph_AlpacaDrvrVersion,	cAlpacaVersionString);
	//	SetWidgetText(kTab_About,		kAboutBox_AlpacaDrvrVersion,	cAlpacaVersionString);
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
void	PARENT_CLASS::AlpacaProcessSupportedActionDome(const int deviveNum, const char *valueString)
{
//	CONSOLE_DEBUG_W_STR("valueString=",	valueString);

	if (strcasecmp(valueString, "readall") == 0)
	{
#ifdef _ENABLE_SKYTRAVEL_
		cDomeHasReadAll	=	true;
#else
		cHasReadAll	=	true;
#endif
	}
	else if (strcasecmp(valueString, "foo") == 0)
	{
		//*	you get the idea
	}
}

//*****************************************************************************
//*	if this routine gets overloaded, the first part, checking for "readall" must be preserved
//*****************************************************************************
void	PARENT_CLASS::AlpacaProcessSupportedAction(const char *deviceType, const int deviveNum, const char *valueString)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceType);

	if (strcasecmp(deviceType, "Dome") == 0)
	{
		AlpacaProcessSupportedActionDome(deviveNum, valueString);
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


#endif // defined(_ENABLE_SKYTRAVEL_) || defined(_ENABLE_CTRL_DOME_)
