//*****************************************************************************
//*		controller_covercalib.cpp		(c) 2021 by Mark Sproul
//*
//*
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
//*	Edit History
//*****************************************************************************
//*	Feb 12,	2021	<MLS> Created controller_covercalib.cpp
//*	Feb 12,	2021	<MLS> CoverCalibration controller fully working
//*****************************************************************************

#define _ENABLE_COVER_CALIBRATION_


#ifdef _ENABLE_COVER_CALIBRATION_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	450
#define	kWindowHeight	700


#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"windowtab_covercalib.h"
#include	"windowtab_drvrInfo.h"
#include	"windowtab_about.h"

#include	"controller.h"
#include	"controller_covercalib.h"

//**************************************************************************************
enum
{
	kTab_Cover	=	1,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerCoverCalib::ControllerCoverCalib(	const char			*argWindowName,
											struct sockaddr_in	*deviceAddress,
											const int			port,
											const int			deviceNum)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight, kNoBackgroundTask)
{
	memset(&cCoverCalibrationProp, 0, sizeof(TYPE_CoverCalibrationProperties));

	cCoverCalibTabObjPtr	=	NULL;
	cDriverInfoTabObjPtr	=	NULL;
	cAboutBoxTabObjPtr		=	NULL;

	cAlpacaDevNum			=	deviceNum;
	if (deviceAddress != NULL)
	{
		cDeviceAddress	=	*deviceAddress;
		cPort			=	port;
		cValidIPaddr	=	true;
	}

	SetupWindowControls();

	if (cCoverCalibTabObjPtr != NULL)
	{
		cCoverCalibTabObjPtr->SetCoverCalibPropPtr(&cCoverCalibrationProp);
	}

	SetWidgetText(kTab_Cover,		kCoverCalib_AlpacaDrvrVersion,		gFullVersionString);

	AlpacaSetConnected("covercalibrator", true);
#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_

}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerCoverCalib::~ControllerCoverCalib(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	DELETE_OBJ_IF_VALID(cCoverCalibTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}


//**************************************************************************************
void	ControllerCoverCalib::SetupWindowControls(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_Cover,		"Cover/Calib");
	SetTabText(kTab_DriverInfo,	"Driver Info");
	SetTabText(kTab_About,		"About");


	cCoverCalibTabObjPtr	=	new WindowTabCoverCalib(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cCoverCalibTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Cover,	cCoverCalibTabObjPtr);
		cCoverCalibTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cDriverInfoTabObjPtr		=	new WindowTabDriverInfo(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDriverInfoTabObjPtr != NULL)
	{
		SetTabWindow(kTab_DriverInfo,	cDriverInfoTabObjPtr);
		cDriverInfoTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}

	SetWidgetFont(kTab_Cover,		kCoverCalib_IPaddr, kFont_Medium);

	//*	display the IPaddres/port
	if (cValidIPaddr)
	{
	char	ipString[32];
	char	lineBuff[64];

		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);

		SetWindowIPaddrInfo(lineBuff, true);
	}
}

//**************************************************************************************
void	ControllerCoverCalib::RunBackgroundTasks(void)
{
uint32_t	currentMillis;
uint32_t	deltaSeconds;
bool		validData;
bool		needToUpdate;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cReadStartup)
	{
		CONSOLE_DEBUG_W_STR("Reading startup information for", cWindowName);
		AlpacaGetCommonProperties_OneAAT("covercalibrator");
		AlpacaGetStartupData();
		cReadStartup	=	false;
	}

	needToUpdate	=	false;
	currentMillis	=	millis();
	deltaSeconds	=	(currentMillis - cLastUpdate_milliSecs) / 1000;

	if (deltaSeconds >= 5)
	{
		needToUpdate	=	true;
	}
	if (cForceAlpacaUpdate)	//*	force update is set when a switch is clicked
	{
		needToUpdate		=	true;
		cForceAlpacaUpdate	=	false;
	}

	if (needToUpdate)
	{
		//*	is the IP address valid
		if (cValidIPaddr)
		{
			validData	=	AlpacaGetStatus();

			if (validData == false)
			{
			//	CONSOLE_DEBUG("Failed to get data")
			}
			UpdateConnectedIndicator(kTab_Cover,		kCoverCalib_Connected);
		}
	}
}

//*****************************************************************************
bool	ControllerCoverCalib::AlpacaGetStartupData(void)
{
bool			validData;
int				integerValue;

	CONSOLE_DEBUG(__FUNCTION__);
	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions("covercalibrator", cAlpacaDevNum);
	if (validData)
	{
		//*	AlpacaGetSupportedActions() sets the cHas_readall appropriately
		UpdateSupportedActions();
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}

	CONSOLE_DEBUG(__FUNCTION__);
	//------------------------------------------------------------------
	validData	=	AlpacaGetIntegerValue("covercalibrator", "coverstate",	NULL,	&integerValue);
	if (validData)
	{
		cCoverCalibrationProp.CoverState	=	(CoverStatus)integerValue;
		if (cCoverCalibrationProp.CoverState == kCover_NotPresent)
		{
		//	SetWidgetText(		kTab_Cover,	kCoverCalib_Cover_Label,	"No Cover available");
			SetWidgetText(kTab_Cover,	kCoverCalib_Cover_State,	"Not Present");
			SetWidgetBGColor(	kTab_Cover,	kCoverCalib_Cover_Open,		CV_RGB(100, 100, 100));
			SetWidgetBGColor(	kTab_Cover,	kCoverCalib_Cover_Close,	CV_RGB(100, 100, 100));
			SetWidgetBGColor(	kTab_Cover,	kCoverCalib_Cover_Halt,		CV_RGB(100, 100, 100));
		}
	}
	else
	{
		CONSOLE_DEBUG("Read failure - coverstate");
		cReadFailureCnt++;
	}

	CONSOLE_DEBUG(__FUNCTION__);
	//------------------------------------------------------------------
	validData	=	AlpacaGetIntegerValue("covercalibrator", "calibratorstate",	NULL,	&integerValue);
	if (validData)
	{
		cCoverCalibrationProp.CalibratorState	=	(CalibratorStatus)integerValue;
		if (cCoverCalibrationProp.CalibratorState == kCalibrator_NotPresent)
		{

		}
		else
		{
			//------------------------------------------------------------------
			validData	=	AlpacaGetIntegerValue("covercalibrator", "maxbrightness",	NULL,	&integerValue);
			if (validData)
			{
				cCoverCalibrationProp.MaxBrightness	=	integerValue;
				SetWidgetSliderLimits(	kTab_Cover,	kCoverCalib_Brightness_Slider,	0.0, integerValue);
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("Read failure - calibratorstate");
		cReadFailureCnt++;
	}
	CONSOLE_DEBUG(__FUNCTION__);
	return(validData);
}

//*****************************************************************************
void	ControllerCoverCalib::UpdateCommonProperties(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetText(	kTab_DriverInfo,	kDriverInfo_Name,				cCommonProp.Name);
	SetWidgetText(	kTab_DriverInfo,	kDriverInfo_Description,		cCommonProp.Description);
	SetWidgetText(	kTab_DriverInfo,	kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
	SetWidgetText(	kTab_DriverInfo,	kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
	SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);
}

//*****************************************************************************
void	ControllerCoverCalib::UpdateSupportedActions(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetValid(kTab_Cover,		kCoverCalib_Readall,		cHas_readall);
	SetWidgetValid(kTab_DriverInfo,	kDriverInfo_Readall,		cHas_readall);

}

//*****************************************************************************
bool	ControllerCoverCalib::AlpacaGetStatus(void)
{
bool	validData;
bool	previousOnLineState;
int		integerValue;
char	stateString[32];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	previousOnLineState	=	cOnLine;

	validData	=	AlpacaGetCommonConnectedState("covercalibrator");
	validData	=	AlpacaGetIntegerValue("covercalibrator", "brightness",	NULL,	&integerValue);
	if (validData)
	{
		cCoverCalibrationProp.Brightness	=	integerValue;
		SetWidgetSliderValue(	kTab_Cover,	kCoverCalib_Brightness_Slider,	integerValue);
		SetWidgetNumber(		kTab_Cover,	kCoverCalib_Brightness_Value,	integerValue);

		//*	check to see if we were one line before
		if (cOnLine == false)
		{
			//*	if we go from offline back to online, re-do the startup info
			cReadStartup	=	true;
		}
		cOnLine	=	true;
	}
	else
	{
		cReadFailureCnt++;
		CONSOLE_DEBUG_W_STR("Offline-", cWindowName);
		cOnLine	=	false;
	}

	//*	dont bother checking if we determined its off line above
	if (cOnLine && (cCoverCalibrationProp.CoverState != kCover_NotPresent))
	{
		validData	=	AlpacaGetIntegerValue("covercalibrator", "coverstate",	NULL,	&integerValue);
		if (validData)
		{
			cCoverCalibrationProp.CoverState	=	(CoverStatus)integerValue;
			switch(cCoverCalibrationProp.CoverState)
			{
				case kCover_NotPresent:	strcpy(stateString,	"Not Present");	break;
				case kCover_Closed:		strcpy(stateString,	"Closed");		break;
				case kCover_Moving:		strcpy(stateString,	"Moving");		break;
				case kCover_Open:		strcpy(stateString,	"Open");		break;
				case kCover_Unknown:	strcpy(stateString,	"Unknown");		break;
				case kCover_Error:		strcpy(stateString,	"Error");		break;
			}
			SetWidgetText(kTab_Cover,	kCoverCalib_Cover_State,	stateString);
		}
		else
		{
			cReadFailureCnt++;
			CONSOLE_DEBUG_W_STR("Offline-", cWindowName);
			cOnLine	=	false;
		}
	}

	if (cOnLine != previousOnLineState)
	{
		SetWindowIPaddrInfo(NULL, cOnLine);
	}

	cLastUpdate_milliSecs	=	millis();
	return(validData);
}

#endif // _ENABLE_COVER_CALIBRATION_

