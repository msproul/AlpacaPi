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
//*	Mar 25,	2023	<MLS> Added capability list to cover/calibrator controller
//*	Jun 18,	2023	<MLS> Added UpdateSupportedActions() to CoverCalib controller
//*	Jun 19,	2023	<MLS> Updated constructor to use TYPE_REMOTE_DEV
//*	Jun 22,	2023	<MLS> Removed RunBackgroundTasks(), using default in parent class
//*	Jun 25,	2023	<ADD> Add brightness to DeviceState
//*	Jun 25,	2023	<MLS> Added DeviceState window to covercalibration controller
//*	Jun 28,	2023	<MLS> Updated from AlpacaProcessReadAll() to AlpacaProcessReadAllIdx()
//*	Jul 14,	2023	<MLS> Added UpdateOnlineStatus() to covercalibration controller
//*	Jul 16,	2023	<MLS> More work on UpdateStatusData()
//*	Jul 16,	2023	<MLS> AlpacaGetStatus_OneAAT() & UpdateStatusData() now working with simulator
//*****************************************************************************

#define _ENABLE_COVER_CALIBRATION_


#ifdef _ENABLE_COVER_CALIBRATION_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	450
#define	kWindowHeight	700


#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"windowtab_covercalib.h"
#include	"windowtab_DeviceState.h"
#include	"windowtab_drvrInfo.h"
#include	"windowtab_about.h"

#include	"controller.h"
#include	"controller_covercalib.h"

#include	"covercalib_AlpacaCmds.h"
#include	"covercalib_AlpacaCmds.cpp"


//**************************************************************************************
enum
{
	kTab_Cover	=	1,
	kTab_Capabilities,
	kTab_DeviceState,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerCoverCalib::ControllerCoverCalib(	const char			*argWindowName,
											TYPE_REMOTE_DEV		*alpacaDevice)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight, true, alpacaDevice)
{
	memset(&cCoverCalibrationProp, 0, sizeof(TYPE_CoverCalibrationProperties));
	strcpy(cAlpacaDeviceTypeStr,	"covercalibrator");

	cDriverInfoTabNum		=	kTab_DriverInfo;
	cCoverCalibTabObjPtr	=	NULL;
	cDriverInfoTabObjPtr	=	NULL;
	cAboutBoxTabObjPtr		=	NULL;
	cForceAlpacaUpdate		=	true;

	SetCommandLookupTable(gCalibrationCmdTable);

	SetupWindowControls();

	if (cCoverCalibTabObjPtr != NULL)
	{
		cCoverCalibTabObjPtr->SetCoverCalibPropPtr(&cCoverCalibrationProp);
	}

#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerCoverCalib::~ControllerCoverCalib(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	DELETE_OBJ_IF_VALID(cCoverCalibTabObjPtr);
	DELETE_OBJ_IF_VALID(cCapabilitiesTabObjPtr);
	DELETE_OBJ_IF_VALID(cDeviceStateTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
void	ControllerCoverCalib::SetupWindowControls(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_Cover,			"Cover/Calib");
	SetTabText(kTab_Capabilities,	"Capabilities");
	SetTabText(kTab_DeviceState,	"Dev State");
	SetTabText(kTab_DriverInfo,		"Driver Info");
	SetTabText(kTab_About,			"About");


	cCoverCalibTabObjPtr	=	new WindowTabCoverCalib(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cCoverCalibTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Cover,	cCoverCalibTabObjPtr);
		cCoverCalibTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cCapabilitiesTabObjPtr		=	new WindowTabCapabilities(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cCapabilitiesTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Capabilities,	cCapabilitiesTabObjPtr);
		cCapabilitiesTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cDeviceStateTabObjPtr		=	new WindowTabDeviceState(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDeviceStateTabObjPtr != NULL)
	{
		SetTabWindow(kTab_DeviceState,	cDeviceStateTabObjPtr);
		cDeviceStateTabObjPtr->SetParentObjectPtr(this);
		SetDeviceStateTabInfo(kTab_DeviceState, kDeviceState_FirstBoxName, kDeviceState_FirstBoxValue, kDeviceState_Stats);
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
void	ControllerCoverCalib::UpdateStartupData(void)
{
	UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);
	if (cCoverCalibrationProp.CoverState == kCover_NotPresent)
	{
		SetWidgetText(		kTab_Cover,	kCoverCalib_Cover_State,	"Not Present");
		SetWidgetBGColor(	kTab_Cover,	kCoverCalib_Cover_Open,		CV_RGB(100, 100, 100));
		SetWidgetBGColor(	kTab_Cover,	kCoverCalib_Cover_Close,	CV_RGB(100, 100, 100));
		SetWidgetBGColor(	kTab_Cover,	kCoverCalib_Cover_Halt,		CV_RGB(100, 100, 100));
	}
	if (cCoverCalibrationProp.CalibratorState == kCalibrator_NotPresent)
	{

	}
	else
	{
		SetWidgetSliderLimits(	kTab_Cover,	kCoverCalib_Brightness_Slider,	0.0, cCoverCalibrationProp.MaxBrightness);
	}
}

//*****************************************************************************
void	ControllerCoverCalib::UpdateConnectedStatusIndicator(void)
{
	UpdateConnectedIndicator(kTab_Cover,	kCoverCalib_Connected);
}

//**************************************************************************************
void	ControllerCoverCalib::UpdateOnlineStatus(void)
{
cv::Scalar	bgColor;
cv::Scalar	txtColor;

	bgColor		=	cOnLine ? CV_RGB(0,		0,	0)	: CV_RGB(255,	0,	0);
	txtColor	=	cOnLine ? CV_RGB(255,	0,	0)	: CV_RGB(0,		0,	0);

	SetWidgetBGColor(	kTab_Cover,			kCoverCalib_IPaddr,		bgColor);
	SetWidgetTextColor(	kTab_Cover,			kCoverCalib_IPaddr,		txtColor);

	SetWidgetBGColor(	kTab_Capabilities,	kCapabilities_IPaddr,	bgColor);
	SetWidgetTextColor(	kTab_Capabilities,	kCapabilities_IPaddr,	txtColor);

	SetWidgetBGColor(	kTab_DeviceState,	kDeviceState_IPaddr,	bgColor);
	SetWidgetTextColor(	kTab_DeviceState,	kDeviceState_IPaddr,	txtColor);

	SetWidgetBGColor(	kTab_DriverInfo,	kDriverInfo_IPaddr,		bgColor);
	SetWidgetTextColor(	kTab_DriverInfo,	kDriverInfo_IPaddr,		txtColor);
}

//**************************************************************************************
void	ControllerCoverCalib::UpdateStatusData(void)
{
char	stateString[32];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//	DumpCoverCalibProp(&cCoverCalibrationProp, __FUNCTION__);
	UpdateConnectedIndicator(kTab_Cover,	kCoverCalib_Connected);
	SetWidgetSliderValue(	kTab_Cover,	kCoverCalib_Brightness_Slider,	cCoverCalibrationProp.Brightness);
	SetWidgetNumber(		kTab_Cover,	kCoverCalib_Brightness_Value,	cCoverCalibrationProp.Brightness);

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
	cUpdateWindow	=	true;
}

//*****************************************************************************
bool	ControllerCoverCalib::AlpacaGetStartupData_OneAAT(void)
{
bool			validData;
int				integerValue;

//	CONSOLE_DEBUG(__FUNCTION__);
	//------------------------------------------------------------------
	validData	=	AlpacaGetStatus_OneAAT();

	//------------------------------------------------------------------
	validData	=	AlpacaGetIntegerValue("covercalibrator", "maxbrightness",	NULL,	&integerValue);
	if (validData)
	{
		cCoverCalibrationProp.MaxBrightness	=	integerValue;
	}
	return(validData);
}

//*****************************************************************************
bool	ControllerCoverCalib::AlpacaGetStatus_OneAAT(void)
{
bool	validData;
int		integerValue;
//bool	previousOnLineState;
//char	stateString[32];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	//------------------------------------------------------------------
//	previousOnLineState	=	cOnLine;
	validData	=	AlpacaGetIntegerValue("covercalibrator", "coverstate",	NULL,	&integerValue);
	if (validData)
	{
		cCoverCalibrationProp.CoverState	=	(CoverStatus)integerValue;
	}
	else
	{
		CONSOLE_DEBUG("Read failure - coverstate");
		cReadFailureCnt++;
		cOnLine	=	false;
	}

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
		}
	}
	else
	{
		CONSOLE_DEBUG("Read failure - calibratorstate");
		cReadFailureCnt++;
		cOnLine	=	false;
	}
	validData	=	AlpacaGetIntegerValue("covercalibrator", "brightness",	NULL,	&integerValue);
	if (validData)
	{
		cCoverCalibrationProp.Brightness	=	integerValue;
	}
	else
	{
		CONSOLE_DEBUG("Read failure - brightness");
		cReadFailureCnt++;
		cOnLine	=	false;
	}

//	DumpCoverCalibProp(&cCoverCalibrationProp, __FUNCTION__);
	return(validData);
}

//*****************************************************************************
void	ControllerCoverCalib::AlpacaGetCapabilities(void)
{
	ReadOneDriverCapability(cAlpacaDeviceTypeStr,
							"canadjustaperture",
							"CanAdjustAperture",
							&cCoverCalibrationProp.CanSetAperture);
}

//*****************************************************************************
void	ControllerCoverCalib::UpdateSupportedActions(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetValid(kTab_Cover,			kCoverCalib_Readall,		cHas_readall);
	SetWidgetValid(kTab_Cover,			kCoverCalib_DeviceSelect,	cHas_DeviceState);

	SetWidgetValid(kTab_Capabilities,	kCapabilities_Readall,		cHas_readall);
	SetWidgetValid(kTab_Capabilities,	kCapabilities_DeviceState,	cHas_DeviceState);

	SetWidgetValid(kTab_DeviceState,	kDeviceState_Readall,		cHas_readall);
	SetWidgetValid(kTab_DeviceState,	kDeviceState_DeviceState,	cHas_DeviceState);

	SetWidgetValid(kTab_DriverInfo,		kDriverInfo_Readall,		cHas_readall);
	SetWidgetValid(kTab_DriverInfo,		kDriverInfo_DeviceState,	cHas_DeviceState);

	if (cHas_DeviceState == false)
	{
		cDeviceStateTabObjPtr->SetDeviceStateNotSupported();
	}
}

//**************************************************************************************
void	ControllerCoverCalib::UpdateCapabilityList(void)
{
	UpdateCapabilityListID(kTab_Capabilities, kCapabilities_TextBox1, kCapabilities_TextBoxN);
}

//*****************************************************************************
void	ControllerCoverCalib::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
//	CONSOLE_DEBUG_W_STR("Alpaca error=", errorMsgString);
	SetWidgetText(kTab_Cover, kCoverCalib_AlpacaErrorMsg, errorMsgString);
}

//*****************************************************************************
bool	ControllerCoverCalib::AlpacaProcessReadAllIdx(	const char	*deviceTypeStr,
														const int	deviceNum,
														const int	keywordEnum,
														const char	*valueString)
{
int		integerValue;
bool	dataWasHandled;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//	CONSOLE_DEBUG_W_NUM(cWindowName, keywordEnum);

	dataWasHandled	=	true;
	integerValue	=	atoi(valueString);
	switch(keywordEnum)
	{
		case kCmd_Calibration_brightness:			//*	Returns the current calibrator brightness
			cCoverCalibrationProp.Brightness		=	integerValue;
			break;

		case kCmd_Calibration_calibratorstate:		//*	Returns the state of the calibration device
			cCoverCalibrationProp.CalibratorState	=	(CalibratorStatus)integerValue;
			break;

		case kCmd_Calibration_coverstate:			//*	Returns the state of the device cover"
			cCoverCalibrationProp.CoverState		=	(CoverStatus)integerValue;
			break;

		case kCmd_Calibration_maxbrightness:		//*	Returns the calibrator's maximum Brightness value.
			cCoverCalibrationProp.MaxBrightness		=	integerValue;
			break;

		case kCmd_Calibration_CalibratorReady:
			cCoverCalibrationProp.CalibratorReady	=	IsTrueFalse(valueString);
			break;

		case kCmd_Calibration_CoverMoving:
			cCoverCalibrationProp.CoverMoving	=	IsTrueFalse(valueString);
			break;

		case kCmd_Calibration_aperture:				//*	GET/PUT aperture opening in percentage
			cCoverCalibrationProp.Aperture		=	atof(valueString);
			break;

		case kCmd_Calibration_canadjustaperture:	//*	true if adjustable aperture is available
			cCoverCalibrationProp.CanSetAperture	=	IsTrueFalse(valueString);
			break;

		default:
			dataWasHandled	=	true;
			break;

	}
	return(dataWasHandled);
}


#endif // _ENABLE_COVER_CALIBRATION_

