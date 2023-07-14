//*****************************************************************************
//*		controller_multicam.cpp		(c) 2023 by Mark Sproul
//*
//*	Description:
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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jun 16,	2023	<MLS> Created controller_multicam.cpp
//*	Jun 17,	2023	<MLS> Multicam controller finished and working
//*	Jun 18,	2023	<MLS> Added UpdateSupportedActions() to CoverCalib controller
//*****************************************************************************

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"helper_functions.h"
#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#include	"windowtab_multicam.h"
#include	"windowtab_about.h"

#include	"controller.h"
#include	"controller_multicam.h"
//#include	"controller_startup.h"

#define	kWindowWidth	450
#define	kWindowHeight	650


//**************************************************************************************
enum
{
	kTab_Multicam	=	1,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};



//**************************************************************************************
ControllerMulticam::ControllerMulticam(	const char			*argWindowName,
										TYPE_REMOTE_DEV		*alpacaDevice)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight)
{
char	ipAddrStr[32];
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cAlpacaDeviceTypeStr,	"multicam");


	//*	window object ptrs
	cMultiCamTabObjPtr			=	NULL;
	cDriverInfoTabObjPtr		=	NULL;
	cAboutBoxTabObjPtr			=	NULL;

	cFirstDataRead				=	true;
	cHas_readall				=	true;

	if (alpacaDevice != NULL)
	{
		cValidIPaddr	=	true;

		cAlpacaDevNum	=	alpacaDevice->alpacaDeviceNum;
		cDeviceAddress	=	alpacaDevice->deviceAddress;
		cPort			=	alpacaDevice->port;

		strcpy(cAlpacaDeviceNameStr,	alpacaDevice->deviceNameStr);

		CheckConnectedState();		//*	check connected and connect if not already connected
	}

	inet_ntop(AF_INET, &(cDeviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
	CONSOLE_DEBUG_W_STR("IP address=", ipAddrStr);

	SetupWindowControls();

	for (iii=0; iii < kMulticamMaxCameraCnt; iii++)
	{
		memset(&cCameraList[iii], 0, sizeof(TYPE_CameraList));
	}


#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_

}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerMulticam::~ControllerMulticam(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	DELETE_OBJ_IF_VALID(cMultiCamTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
void	ControllerMulticam::SetupWindowControls(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	SetTabCount(kTab_Count);
	SetTabText(kTab_Multicam,	"Multicam");
	SetTabText(kTab_DriverInfo,	"Driver Info");
	SetTabText(kTab_About,		"About");

	//=============================================================
	cMultiCamTabObjPtr	=	new WindowTabMulticam(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cMultiCamTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Multicam,	cMultiCamTabObjPtr);
		cMultiCamTabObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	cDriverInfoTabObjPtr		=	new WindowTabDriverInfo(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDriverInfoTabObjPtr != NULL)
	{
	CONSOLE_DEBUG(__FUNCTION__);
		SetTabWindow(kTab_DriverInfo,	cDriverInfoTabObjPtr);
		cDriverInfoTabObjPtr->SetParentObjectPtr(this);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create WindowTabDriverInfo!!!");
		CONSOLE_ABORT(__FUNCTION__);
	}

	//=============================================================
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
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

//*****************************************************************************
void	ControllerMulticam::UpdateSupportedActions(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetValid(kTab_Multicam,		kMultiCam_Readall,		cHas_readall);
	SetWidgetValid(kTab_DriverInfo,		kDriverInfo_Readall,	cHas_readall);
}

//*****************************************************************************
void	ControllerMulticam::UpdateCommonProperties(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Name,				cCommonProp.Name);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Description,		cCommonProp.Description);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
	SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);
}

////**************************************************************************************
//void	ControllerMulticam::RunBackgroundTasks(const char *callingFunction, bool enableDebug)
//{
//uint32_t	currentMillis;
//uint32_t	deltaSeconds;
//bool		needToUpdate;
//bool		validData;
//
////	CONSOLE_DEBUG(__FUNCTION__);
//	if (cReadStartup)
//	{
//		AlpacaGetStartupData();
//		AlpacaGetCommonProperties_OneAAT("multicam");
//		UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);
//
//		cReadStartup	=	false;
//	}
//
//
//	needToUpdate	=	false;
//	currentMillis	=	millis();
//	deltaSeconds	=	(currentMillis - cLastUpdate_milliSecs) / 1000;
//
//	if (cFirstDataRead || (deltaSeconds > 10))
//	{
//		needToUpdate	=	true;
//		cFirstDataRead	=	false;
//	}
//	if (cForceAlpacaUpdate)	//*	force update is set when a switch is clicked
//	{
//		needToUpdate		=	true;
//		cForceAlpacaUpdate	=	false;
//	}
//
//	if (needToUpdate)
//	{
//		//*	is the IP address valid
//		if (cValidIPaddr)
//		{
//			validData	=	AlpacaGetStatus();
//			if (validData == false)
//			{
//				CONSOLE_DEBUG("Failed to get data");
//			}
//
//			UpdateConnectedIndicator(kTab_Multicam,		kMultiCam_Connected);
//		}
//	}
//}


//*****************************************************************************
bool	ControllerMulticam::AlpacaGetStartupData(void)
{
bool		validData;
char		cameraNameBuffer[512];
int			iii;

	CONSOLE_DEBUG(__FUNCTION__);
	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions("multicam", cAlpacaDevNum);

	if (validData)
	{
		UpdateSupportedActions();
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}

	if (cHas_readall)
	{
		//*	use readall to get the startup data
		validData	=	AlpacaGetStatus_ReadAll("multicam", cAlpacaDevNum);

		cameraNameBuffer[0]	=	0;
		iii	=	0;
		while ((strlen(cCameraList[iii].CameraName) > 0) && (iii < kMulticamMaxCameraCnt))
		{
			strcat(cameraNameBuffer, cCameraList[iii].CameraName);
			strcat(cameraNameBuffer, "\r");
			iii++;
		}
		SetWidgetText(kTab_Multicam, kMultiCam_CameraList, cameraNameBuffer);
	}


	cLastUpdate_milliSecs	=	millis();

	return(validData);
}

//*****************************************************************************
bool	ControllerMulticam::AlpacaGetStatus(void)
{
bool	validData;
//char	lineBuff[128];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	validData	=	false;
	if (cHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll("multicam", cAlpacaDevNum);


		SetWindowIPaddrInfo(NULL, true);

		cLastUpdate_milliSecs	=	millis();
	}
	else
	{
//		validData	=	AlpacaGetStatus_OneAAT();	//*	One At A Time
	}

	if (validData)
	{
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
		CONSOLE_DEBUG_W_STR("Offline-", cWindowName);
		cOnLine	=	false;
	}

	SetWindowIPaddrInfo(NULL, cOnLine);


	cLastUpdate_milliSecs	=	millis();
	return(validData);
}


//*****************************************************************************
bool	ControllerMulticam::AlpacaProcessReadAll(	const char	*deviceType,
													const int	deviceNum,
													const char	*keywordString,
													const char	*valueString)
{
int			cameraIndex;
bool		dataWasHandled	=	true;
//double		argDouble;

//	CONSOLE_DEBUG_W_STR("deviceType\t=", deviceType);
//	CONSOLE_DEBUG_W_STR(keywordString, valueString);

	if (strncasecmp(keywordString, "camera-", 7) == 0)
	{
		//*	extract the camera index number
		cameraIndex	=	atoi(&keywordString[7]);
		if ((cameraIndex >= 0) && (cameraIndex < kMulticamMaxCameraCnt))
		{
			strcpy(cCameraList[cameraIndex].CameraName, valueString);
		}
	}
	else
	{
		dataWasHandled	=	false;
	}
	return(dataWasHandled);
}

