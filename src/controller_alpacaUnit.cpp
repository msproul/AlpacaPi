//*****************************************************************************
//*
//*	Name:			controller_alpacaUnit.cpp
//*
//*	Author:			Mark Sproul (C) 2019-2020
//*
//*	Description:	Alpaca discovery library
//*
//*	Limitations:
//*
//*	Usage notes:
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Nov 30,	2022	<MLS> Created controller_alpacaUnit.cpp
//*	Feb 10,	2023	<MLS> Fixed initialization bug for focuser temp log
//*	Jul  1,	2023	<MLS> Added GetStatus_SubClass() to AlpacaUnit
//*	Jul  1,	2023	<MLS> Added GetStartUpData_SubClass() to AlpacaUnit
//*****************************************************************************

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>

#include	"discovery_lib.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"windowtab_alpacaUnit.h"
#include	"controller_alpacaUnit.h"

#define	kWindowWidth	999
#define	kWindowHeight	700

//**************************************************************************************
enum
{
	kTab_AlpacaUnit	=	1,
	kTab_About,
	kTab_Unit_Count

};

//**************************************************************************************
void	CreateAlpacaUnitWindow(TYPE_ALPACA_UNIT	*alpacaUnit)
{
char	windowName[128];

	if (strlen(alpacaUnit->hostName)  > 0)
	{
		strcpy(windowName, alpacaUnit->hostName);
	}
	else
	{
		//*	put in the ip address
		inet_ntop(AF_INET, &(alpacaUnit->deviceAddress.sin_addr), windowName, INET_ADDRSTRLEN);
	}
	new ControllerAlpacaUnit(windowName, alpacaUnit);
}

//**************************************************************************************
ControllerAlpacaUnit::ControllerAlpacaUnit(	const char			*argWindowName,
											TYPE_ALPACA_UNIT	*alpacaUnit)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cAlpacaDeviceNameStr,	"management");
	strcpy(cAlpacaDeviceTypeStr,	"management");

	cAlpacaDeviceType		=	kDeviceType_Management;
	CONSOLE_DEBUG_W_NUM("cAlpacaDeviceType    \t=", cAlpacaDeviceType);

	cAlpacaUnitTabObjPtr	=	NULL;
	cAboutBoxTabObjPtr		=	NULL;
	cConfiguredDevIndex		=	0;
	cHasCamera				=	false;
	cHasFocuser				=	false;

	cCPUTtempCnt			=	0;
	cCameraTempCnt			=	0;
	cFocusTempCnt			=	0;
	memset(cCPUtempLog,		0, sizeof(cCPUtempLog));
	memset(cCameraTempLog,	0, sizeof(cCameraTempLog));
	memset(cFocusTempLog,	0, sizeof(cFocusTempLog));

	//*	copy the device address info
	if (alpacaUnit != NULL)
	{
		cValidIPaddr	=	true;
		cAlpacaUnit		=	*alpacaUnit;
		cDeviceAddress	=	alpacaUnit->deviceAddress;
		cPort			=	alpacaUnit->port;
		cAlpacaDevNum	=	0;
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}

	SetupWindowControls();
#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerAlpacaUnit::~ControllerAlpacaUnit(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	DELETE_OBJ_IF_VALID(cAlpacaUnitTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
void	ControllerAlpacaUnit::SetupWindowControls(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Unit_Count);
	SetTabText(kTab_AlpacaUnit	,	"Alpaca-Unit");
	SetTabText(kTab_About,			"About");

	//--------------------------------------------
	cAlpacaUnitTabObjPtr	=	new WindowAlpacaUnit(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAlpacaUnitTabObjPtr != NULL)
	{
		SetTabWindow(kTab_AlpacaUnit,	cAlpacaUnitTabObjPtr);
		cAlpacaUnitTabObjPtr->SetParentObjectPtr(this);

	}

	//--------------------------------------------
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}
}

//*****************************************************************************
void	ControllerAlpacaUnit::UpdateSupportedActions(void)
{
	if (cHas_temperaturelog == false)
	{
		SetWidgetType(kTab_AlpacaUnit, kAlpacaUnit_CPUtempGraph, kWidgetType_TextBox);
		SetWidgetFont(kTab_AlpacaUnit, kAlpacaUnit_CPUtempGraph, kFont_Large);
		SetWidgetText(kTab_AlpacaUnit, kAlpacaUnit_CPUtempGraph, "Temperature data not available");
	}
	cHas_DeviceState	=	false;
}

//*****************************************************************************
void	ControllerAlpacaUnit::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
}

//*****************************************************************************
void	ControllerAlpacaUnit::GetStartUpData_SubClass(void)
{
int		txtBoxIdx;
int		iii;
char	deviceListString[512];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	//------------------------------------------------------
	//*	get the list of configured devices
	cConfiguredDevIndex		=	0;
	GetConfiguredDevices();
	if (cConfiguredDevIndex > 0)
	{

		for (iii=0; iii<cConfiguredDevIndex; iii++)
		{
			strcpy(deviceListString, cDeviceList[iii].Type);
			strcat(deviceListString, "\t");
			strcat(deviceListString, cDeviceList[iii].Name);

			txtBoxIdx	=	kAlpacaUnit_AlpacaDev_01 + iii;
			if (txtBoxIdx < kAlpacaUnit_Outline)
			{
				SetWidgetText(kTab_AlpacaUnit, txtBoxIdx, deviceListString);
			}
			else
			{
				SetWidgetText(kTab_AlpacaUnit, (kAlpacaUnit_Outline - 1), "Out of space!!!!!!!!!!!!!!!!!");
			}
		}
	}
	GetTemperatureLogs();
}

//**************************************************************************************
void	ControllerAlpacaUnit::GetStatus_SubClass(void)
{
	GetTemperatureLogs();
	cUpdateWindow		=	true;
	cUpdateDelta_secs	=	120;
}

//*****************************************************************************
bool	ControllerAlpacaUnit::AlpacaProcessReadAll(	const char	*deviceTypeStr,
													const int	deviceNum,
													const char	*keywordString,
													const char	*valueString)
{
//	CONSOLE_DEBUG(cWindowName);
	CONSOLE_DEBUG_W_2STR("json=",	keywordString, valueString);
	return(false);
}

//*****************************************************************************
void	ControllerAlpacaUnit::UpdateCommonProperties(void)
{
	UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);
	UpdateAboutBoxRemoteDevice(kTab_AlpacaUnit,	kAlpacaUnit_CPUinfo);
}

//*****************************************************************************
void	ControllerAlpacaUnit::ProcessConfiguredDevices(const char *keyword, const char *valueString)
{
//	CONSOLE_DEBUG_W_2STR("kw:val", keyword, valueString);

	if (strcasecmp(keyword, "DEVICETYPE") == 0)
	{
		if (cConfiguredDevIndex < kMaxDevices)
		{
			strcpy(cDeviceList[cConfiguredDevIndex].Type, valueString);
		}

		//*	look for special cases that may have temperature logs
		if (strcasecmp(valueString, "CAMERA") == 0)
		{
			cHasCamera				=	true;
		}
		if (strcasecmp(valueString, "FOCUSER") == 0)
		{
			cHasFocuser				=	true;
		}
	}
	else if (strcasecmp(keyword, "DeviceName") == 0)
	{
		if (cConfiguredDevIndex < kMaxDevices)
		{
			strcpy(cDeviceList[cConfiguredDevIndex].Name, valueString);
		}
	}
	else if (strcasecmp(keyword, "ARRAY") == 0)
	{
		cConfiguredDevIndex	=	0;
	}
	else if (strcasecmp(keyword, "ARRAY-NEXT") == 0)
	{
		cConfiguredDevIndex++;
	}
}

//**************************************************************************************
void	ControllerAlpacaUnit::GetTemperatureLogs(void)
{
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	cCPUTtempCnt	=	Alpaca_GetTemperatureLog("management", 0, cCPUtempLog, kTemperatureLogEntries);

	if (cHasCamera)
	{
//		CONSOLE_DEBUG("Requesting camera temperature log");
		cCameraTempCnt	=	Alpaca_GetTemperatureLog("camera", 0, cCameraTempLog, kTemperatureLogEntries);
		for (iii=0; iii<cCameraTempCnt; iii++)
		{
			cCameraTempLog[iii]	=	DEGREES_F(cCameraTempLog[iii]);
		}
	}
	if (cHasFocuser)
	{
//		CONSOLE_DEBUG("Requesting focuser temperature log");
		cFocusTempCnt	=	Alpaca_GetTemperatureLog("focuser", 0, cFocusTempLog, kTemperatureLogEntries);
		for (iii=0; iii<cFocusTempCnt; iii++)
		{
			cFocusTempLog[iii]	=	DEGREES_F(cFocusTempLog[iii]);
		}
	}
}
