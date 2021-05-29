//*****************************************************************************
//*		controller_filterwheel.cpp		(c) 2021 by Mark Sproul
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
//*	May 26,	2021	<MLS> Created controller_filterwheel.cpp
//*****************************************************************************



#ifdef _ENABLE_FILTERWHEEL_CONTROLLER_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include	"discovery_lib.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	450
#define	kWindowHeight	700

#include	"windowtab_filterwheel.h"
#include	"windowtab_drvrInfo.h"
#include	"windowtab_about.h"
#include	"helper_functions.h"
#include	"sendrequest_lib.h"

#include	"controller.h"
#include	"controller_filterwheel.h"

//**************************************************************************************
enum
{
	kTab_FilterWheel	=	1,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerFilterWheel::ControllerFilterWheel(	const char			*argWindowName,
												TYPE_REMOTE_DEV		*alpacaDevice)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight)
{
	CONSOLE_DEBUG(__FUNCTION__);

	cFilterWheelTabObjPtr	=	NULL;
	cDriverInfoTabObjPtr	=	NULL;
	cAboutBoxTabObjPtr		=	NULL;

	memset(&cFilterWheelProp, 0, sizeof(TYPE_FilterWheelProperties));
	cPositionCount			=	9;

	//*	copy the device address info
	if (alpacaDevice != NULL)
	{
		cValidIPaddr	=	true;

		cAlpacaDevNum	=	alpacaDevice->alpacaDeviceNum;
		cDeviceAddress	=	alpacaDevice->deviceAddress;
		cPort			=	alpacaDevice->port;

		strcpy(cAlpacaDeviceTypeStr,	alpacaDevice->deviceTypeStr);
		strcpy(cAlpacaDeviceNameStr,	alpacaDevice->deviceNameStr);
		strcpy(cAlpacaVersionString,	alpacaDevice->versionString);
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}

	SetupWindowControls();

	SetWidgetText(kTab_FilterWheel,		kFilterWheel_AlpacaDrvrVersion,		cAlpacaVersionString);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerFilterWheel::~ControllerFilterWheel(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	DELETE_OBJ_IF_VALID(cFilterWheelTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}


//**************************************************************************************
void	ControllerFilterWheel::SetupWindowControls(void)
{

	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_FilterWheel,	"FilterWheel");
	SetTabText(kTab_DriverInfo,		"Driver Info");
	SetTabText(kTab_About,			"About");


	//--------------------------------------------
	cFilterWheelTabObjPtr	=	new WindowTabFilterWheel(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cFilterWheelTabObjPtr != NULL)
	{
		SetTabWindow(kTab_FilterWheel,	cFilterWheelTabObjPtr);
		cFilterWheelTabObjPtr->SetParentObjectPtr(this);

		cFilterWheelTabObjPtr->SetPositonCount(cPositionCount);
		cFilterWheelTabObjPtr->SetFilterWheelPropPtr(&cFilterWheelProp);
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
void	ControllerFilterWheel::RunBackgroundTasks(void)
{
long	delteaMillSecs;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cReadStartup)
	{
		CONSOLE_DEBUG_W_STR("cReadStartup", cWindowName);

		//*	so the window shows up
		HandleWindowUpdate();
		cvWaitKey(60);

		AlpacaGetCommonProperties("filterwheel");
		AlpacaGetStartupData();


		cReadStartup	=	false;
	}

	delteaMillSecs	=	millis() - cLastUpdate_milliSecs;
	if (delteaMillSecs > 5000)
	{
		AlpacaGetFilterWheelStatus();
		cLastUpdate_milliSecs	=	millis();
	}

}

//*****************************************************************************
bool	ControllerFilterWheel::AlpacaGetStartupData(void)
{
bool			validData;
int				jjj;
int				boxNumber;

	CONSOLE_DEBUG(__FUNCTION__);
	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions("filterwheel", cAlpacaDevNum);

	if (validData)
	{
		SetWidgetValid(kTab_FilterWheel,	kFilterWheel_Readall,		cHas_readall);
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}

	if (cHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll("filterwheel", cAlpacaDevNum);
	}
	else
	{
//		validData	=	AlpacaGetStartupData_OneAAT();
	}

	AlpacaGetFilterWheelStartup();
	if (cFilterWheelTabObjPtr != NULL)
	{
		cFilterWheelTabObjPtr->SetPositonCount(cPositionCount);
	}

	AlpacaGetFilterWheelStatus();

	cLastUpdate_milliSecs	=	millis();

	return(validData);
}

//*****************************************************************************
void	ControllerFilterWheel::UpdateCommonProperties(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetText(	kTab_DriverInfo,	kDriverInfo_Name,				cCommonProp.Name);
	SetWidgetText(	kTab_DriverInfo,	kDriverInfo_Description,		cCommonProp.Description);
	SetWidgetText(	kTab_DriverInfo,	kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
	SetWidgetText(	kTab_DriverInfo,	kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
	SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);
}


//*****************************************************************************
void	ControllerFilterWheel::UpdateFilterWheelInfo(void)
{

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	SetWidgetText(	kTab_FilterWheel,	kFilterWheel_Name,	cFilterWheelName);
	cUpdateWindow	=	true;
}

//*****************************************************************************
void	ControllerFilterWheel::UpdateFilterWheelPosition(void)
{

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//+	SetWidgetText(	kTab_Camera,	kCameraBox_FilterWheelName,	cFilterWheelName);
	cUpdateWindow	=	true;
}

#define	PARENT_CLASS	ControllerFilterWheel

#include "controller_fw_common.cpp"

#endif // _ENABLE_USB_FOCUSERS_
