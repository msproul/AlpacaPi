//*****************************************************************************
//*		controller_switch.cpp		(c) 2020 by Mark Sproul
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
//*	Feb 29,	2020	<MLS> Created controller_switch.cpp
//*	Feb 29,	2020	<MLS> Started on switch object for Alpaca switch controller
//*	Mar  1,	2020	<MLS> Switch control fully working.
//*	Mar 13,	2020	<MLS> Switch controller now shows offline same as camera controller
//*	Mar 22,	2020	<MLS> Added AlpacaGetStatus_ReadAll()
//*	Apr  3,	2020	<MLS> Added AlpacaGetStatus_OneAAT()
//*	Apr 15,	2020	<MLS> Started on analog switch display
//*	Apr 16,	2020	<MLS> Analog switches display slider bar properly
//*	Apr 21,	2020	<MLS> Added about box to switch controller
//*	May  6,	2020	<MLS> Added AlpacaGetStartupData_OneAAT()
//*	May  6,	2020	<MLS> Using ReadAll for switch startup info, much faster
//*	Jan 14,	2021	<MLS> Added AlpacaProcessSupportedActions()
//*	Jan 14,	2021	<MLS> Switch controller working with ASCOM Remote Server Console/simulator
//*	Feb 12,	2021	<MLS> Added driver info display to switch controller
//*	Sep  8,	2021	<MLS> Working on supporting "canWrite" property
//*	Jan  2,	2022	<MLS> Moved TurnAllSwitchesOff() from window tab to controller
//*	Jun 18,	2023	<MLS> Added UpdateSupportedActions() to switch controller
//*	Jun 19,	2023	<MLS> Updated constructor to use TYPE_REMOTE_DEV
//*	Jun 21,	2023	<MLS> Added DeviceState window to switch controller
//*	Jun 23,	2023	<MLS> Removed RunBackgroundTasks(), using default in parent class
//*	Jun 27,	2023	<MLS> Moved AlpacaGetStatus() function back into parent class
//*	Jun 27,	2023	<MLS> Deleted UpdateCommonProperties() from switch controller
//*	Jun 27,	2023	<MLS> Switch class updated to use more base class methods
//*	Jul  3,	2023	<MLS> Added GetStartUpData_SubClass() & GetStatus_SubClass() to switch controller
//*	Jul  3,	2023	<MLS> Converted to using switch properties structure TYPE_SwitchProperties
//*	Jul 14,	2023	<MLS> Added UpdateOnlineStatus() to switch controller
//*****************************************************************************

#ifdef _ENABLE_CTRL_SWITCHES_

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include	"discovery_lib.h"
#include	"sendrequest_lib.h"
#include	"helper_functions.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	535
#define	kWindowHeight	725

#include	"alpaca_defs.h"
#include	"controller.h"
#include	"windowtab_switch.h"
#include	"windowtab_DeviceState.h"
#include	"windowtab_drvrInfo.h"
#include	"controller_switch.h"

//*	not using the command table for the switch because there are too many special cases in readall
#include	"switch_AlpacaCmds.h"
#include	"switch_AlpacaCmds.cpp"

//**************************************************************************************
enum
{
	kTab_Switch	=	1,
	kTab_DeviceState,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerSwitch::ControllerSwitch(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight, true, alpacaDevice)
{
int		iii;

	memset((void *)&cSwitchProp, 0, sizeof(TYPE_SwitchProperties));
	cSwitchProp.MaxSwitch	=	8;
	cFirstDataRead			=	true;
	cLastUpdate_milliSecs	=	millis();
	cDriverInfoTabNum		=	kTab_DriverInfo;
	strcpy(cAlpacaDeviceTypeStr,	"switch");

	//*	set the default values for the switch info
	for (iii=0; iii<kMaxSwitches; iii++)
	{
		cSwitchProp.SwitchTable[iii].minswitchvalue	=	0.0;
		cSwitchProp.SwitchTable[iii].maxswitchvalue	=	1.0;
		cSwitchProp.SwitchTable[iii].switchvalue	=	0.0;
		cSwitchProp.SwitchTable[iii].canWrite		=	false;
	}

	SetupWindowControls();

	//*	display the IPaddres/port
	if (cValidIPaddr)
	{
	char	lineBuff[128];
	char	ipString[32];

		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);
		SetWindowIPaddrInfo(lineBuff, true);

		CheckConnectedState();		//*	check connected and connect if not already connected

	}

#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerSwitch::~ControllerSwitch(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	//-----------------------------
	DELETE_OBJ_IF_VALID(cSwitchTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
	DELETE_OBJ_IF_VALID(cDeviceStateTabObjPtr);
}

//**************************************************************************************
void	ControllerSwitch::SetupWindowControls(void)
{
char	lineBuff[64];
char	ipString[32];

//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_Switch,			"Switch");
	SetTabText(kTab_DeviceState,	"Dev State");
	SetTabText(kTab_DriverInfo,		"Driver Info");
	SetTabText(kTab_About,			"About");

	//--------------------------------------------
	cSwitchTabObjPtr	=	new WindowTabSwitch(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSwitchTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Switch,	cSwitchTabObjPtr);
		cSwitchTabObjPtr->SetParentObjectPtr(this);
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

	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}

	//*	display the IPaddres/port
	if (cValidIPaddr)
	{
		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);
		SetWidgetFont(kTab_Switch, kSwitchBox_IPaddr, kFont_Medium);
		SetWidgetText(kTab_Switch, kSwitchBox_IPaddr, lineBuff);
	}
}

//**************************************************************************************
void	ControllerSwitch::UpdateConnectedStatusIndicator(void)
{
	UpdateConnectedIndicator(kTab_Switch,		kSwitchBox_Connected);
}

//*****************************************************************************
void	ControllerSwitch::AlpacaProcessSupportedActions(const char	*deviceTypeStr,
														const int	deviveNum,
														const char	*valueString)
{

	if (strcasecmp(valueString, "readall") == 0)
	{
		cHas_readall	=	true;
	}
	else if (strcasecmp(valueString, "foo") == 0)
	{
		//*	you get the idea
	}
}

//*****************************************************************************
//*	Get data One At A Time
//*****************************************************************************
bool	ControllerSwitch::AlpacaGetStartupData_OneAAT(void)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
char			dataString[128];
int				jjj;
int				switchNum;
double			myDoubleValue;
bool			myCanWriteFlag;

//	CONSOLE_DEBUG(__FUNCTION__);

	validData	=	AlpacaGetIntegerValue("switch", "maxswitch",	NULL,	&cSwitchProp.MaxSwitch);
	if (validData)
	{
		CONSOLE_DEBUG_W_NUM("cSwitchProp.MaxSwitch\t=", cSwitchProp.MaxSwitch);
		if (cSwitchTabObjPtr != NULL)
		{
			cSwitchTabObjPtr->SetActiveSwitchCount(cSwitchProp.MaxSwitch);
		}
	}

	validData	=	AlpacaGetStringValue("switch", "description",	NULL,	alpacaString);
	if (validData)
	{
		strcpy(cAlpacaVersionString, alpacaString);
	}

	//*	Start by getting the switch names and descriptions
	switchNum	=	0;
//	CONSOLE_DEBUG_W_NUM("cSwitchProp.MaxSwitch\t=",	cSwitchProp.MaxSwitch);
	while (switchNum < cSwitchProp.MaxSwitch)
	{
//		CONSOLE_DEBUG(__FUNCTION__);
		//===============================================================================
		//*	get the switch name
		SJP_Init(&jsonParser);
		sprintf(alpacaString,	"/api/v1/switch/%d/getswitchname?Id=%d", cAlpacaDevNum, switchNum);
		sprintf(dataString,		"Id=%d", switchNum);
		validData	=	GetJsonResponse(	&cDeviceAddress,
											cPort,
											alpacaString,
											dataString,
											&jsonParser);
		if (validData)
		{
			for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
			{
				if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
				{
					strcpy(cSwitchProp.SwitchTable[switchNum].switchName, jsonParser.dataList[jjj].valueString);
				}
			}
		}
		else
		{
			CONSOLE_DEBUG("Error from GetJsonResponse");
			cReadFailureCnt++;
		}

		//===============================================================================
		//*	get the switch description
		SJP_Init(&jsonParser);
		sprintf(alpacaString,	"/api/v1/switch/%d/getswitchdescription?Id=%d", cAlpacaDevNum, switchNum);
//		sprintf(dataString,		"Id=%d", switchNum);
		validData	=	GetJsonResponse(	&cDeviceAddress,
											cPort,
											alpacaString,
											NULL,
											&jsonParser);
		if (validData)
		{
			for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
			{
				if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
				{
					strcpy(cSwitchProp.SwitchTable[switchNum].switchDescription, jsonParser.dataList[jjj].valueString);
				}
			}
		}
		else
		{
			cReadFailureCnt++;
		}

		//===============================================================================
		//*	Get the minimum switch value
		sprintf(dataString,		"Id=%d", switchNum);
		validData	=	AlpacaGetDoubleValue(	"switch", "minswitchvalue",	dataString,	&myDoubleValue);
		if (validData)
		{
			cSwitchProp.SwitchTable[switchNum].minswitchvalue	=	myDoubleValue;
		}

		//===============================================================================
		//*	Get the maximum switch value
		sprintf(dataString,		"Id=%d", switchNum);
		validData	=	AlpacaGetDoubleValue(	"switch", "maxswitchvalue",	dataString,	&myDoubleValue);
		if (validData)
		{
			cSwitchProp.SwitchTable[switchNum].maxswitchvalue	=	myDoubleValue;
		}

		//===============================================================================
		//*	Get the canwrite value
		sprintf(alpacaString,	"canwrite?Id=%d", switchNum);
		validData	=	AlpacaGetBooleanValue(	"switch", alpacaString,	NULL,	&myCanWriteFlag);
		if (validData)
		{
			cSwitchProp.SwitchTable[switchNum].canWrite	=	myCanWriteFlag;
//			CONSOLE_DEBUG_W_NUM("switchNum\t\t=",		switchNum);
//			CONSOLE_DEBUG_W_NUM("myCanWriteFlag\t=",	myCanWriteFlag);
		}

		switchNum++;
	}
	return(validData);
}

//*****************************************************************************
void	ControllerSwitch::AlpacaGetCapabilities(void)
{
	//*	switch does not have any "Capabilities"
	//*	required for base class
}

//*****************************************************************************
void	ControllerSwitch::GetStartUpData_SubClass(void)
{
int				jjj;
int				boxNumber;

//	CONSOLE_DEBUG(__FUNCTION__);

	AlpacaGetStartupData_OneAAT();

	if (cSwitchTabObjPtr != NULL)
	{
		cSwitchTabObjPtr->SetActiveSwitchCount(cSwitchProp.MaxSwitch);
	}


	for (jjj=0; jjj<cSwitchProp.MaxSwitch; jjj++)
	{
//		CONSOLE_DEBUG_W_STR("switchName\t=",	cSwitchProp.SwitchTable[jjj].switchName);
		boxNumber	=	kSwitchBox_Name01 + (kBoxesPerSwitch * jjj);
		SetWidgetText(kTab_Switch, boxNumber, cSwitchProp.SwitchTable[jjj].switchName);

		boxNumber	=	kSwitchBox_Description01 + (kBoxesPerSwitch * jjj);
		SetWidgetText(kTab_Switch, boxNumber, cSwitchProp.SwitchTable[jjj].switchDescription);

		//*	check to see if it is a slider control
		if (cSwitchProp.SwitchTable[jjj].maxswitchvalue > 1.0)
		{
			boxNumber	=	kSwitchBox_Description01 + (kBoxesPerSwitch * jjj);
			SetWidgetType(kTab_Switch, boxNumber, kWidgetType_Slider);
			SetWidgetSliderLimits(	kTab_Switch,
									boxNumber,
									cSwitchProp.SwitchTable[jjj].minswitchvalue,
									cSwitchProp.SwitchTable[jjj].maxswitchvalue);
		}

		//*	check to see if we can write to the switch
		boxNumber	=	kSwitchBox_State01 + (kBoxesPerSwitch * jjj);
		SetWidgetValid(kTab_Switch, boxNumber, cSwitchProp.SwitchTable[jjj].canWrite);
	}
}

//*****************************************************************************
void	ControllerSwitch::GetStatus_SubClass(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	//*	if cHas_readall is false, AlpacaGetStatus_OneAAT gets called from the parrent class
	if (cHas_readall)
	{
		AlpacaGetStatus_OneAAT();
	}
}

//*****************************************************************************
//*	Get Status, One At A Time
//*****************************************************************************
bool	ControllerSwitch::AlpacaGetStatus_OneAAT(void)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
int				switchNum;
int				boxNumber;
int				myFailureCount;
int				switchValue;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cSwitchProp.MaxSwitch\t=", cSwitchProp.MaxSwitch);

	//*	Get switch status
	myFailureCount	=	0;
	switchNum		=	0;
	while ((switchNum < kMaxSwitches) && (switchNum < cSwitchProp.MaxSwitch))
	{
//		CONSOLE_DEBUG_W_NUM("working on switch #", switchNum);
		//-------------------------------------------------------------------------------
		//*	get the switch state
		SJP_Init(&jsonParser);
		sprintf(alpacaString,	"/api/v1/switch/%d/getswitch?Id=%d", cAlpacaDevNum, switchNum);
		validData	=	GetJsonResponse(	&cDeviceAddress,
											cPort,
											alpacaString,
											NULL,
											&jsonParser);
		if (validData)
		{
			for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
			{
				if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
				{
					boxNumber	=	kSwitchBox_State01 + (kBoxesPerSwitch * switchNum);

					if (boxNumber >= kSwitchBox_LastCmdString)
					{
						CONSOLE_DEBUG_W_NUM("switchNum\t=", switchNum);
						CONSOLE_DEBUG_W_NUM("boxNumber\t=", boxNumber);
						CONSOLE_ABORT(__FUNCTION__);
					}
					if (strcasecmp(jsonParser.dataList[jjj].valueString, "true") == 0)
					{
						cSwitchProp.SwitchTable[switchNum].switchState	=	true;
						SetWidgetText(kTab_Switch, boxNumber, "ON");
					}
					else if (strcasecmp(jsonParser.dataList[jjj].valueString, "false") == 0)
					{
						cSwitchProp.SwitchTable[switchNum].switchState	=	false;
						SetWidgetText(kTab_Switch, boxNumber, "OFF");
					}
				}
			}
		}
		else
		{
			CONSOLE_DEBUG("Error from GetJsonResponse");
			cReadFailureCnt++;
			myFailureCount++;
		}

		//-------------------------------------------------------------------------------
		//*	get the switch value
		SJP_Init(&jsonParser);
		sprintf(alpacaString,	"/api/v1/switch/%d/getswitchvalue?Id=%d", cAlpacaDevNum, switchNum);
		validData	=	GetJsonResponse(	&cDeviceAddress,
											cPort,
											alpacaString,
											NULL,
											&jsonParser);
		if (validData)
		{
			for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
			{
				if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
				{
					boxNumber	=	kSwitchBox_Value01 + (kBoxesPerSwitch * switchNum);

					if (boxNumber >= kSwitchBox_LastCmdString)
					{
						CONSOLE_DEBUG_W_NUM("switchNum\t=", switchNum);
						CONSOLE_DEBUG_W_NUM("boxNumber\t=", boxNumber);
						CONSOLE_ABORT(__FUNCTION__);
					}
					switchValue	=	atoi(jsonParser.dataList[jjj].valueString);
					SetWidgetNumber(kTab_Switch, boxNumber, switchValue);
				}
			}
		}
		else
		{
			CONSOLE_DEBUG("Error from GetJsonResponse");
			cReadFailureCnt++;
			myFailureCount++;
		}

		switchNum++;
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

//**************************************************************************************
void	ControllerSwitch::UpdateStartupData(void)
{
	UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);
}

//**************************************************************************************
void	ControllerSwitch::UpdateStatusData(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	UpdateConnectedIndicator(kTab_Switch,	kSwitchBox_Connected);

}

//**************************************************************************************
void	ControllerSwitch::UpdateOnlineStatus(void)
{
cv::Scalar	bgColor;
cv::Scalar	txtColor;

	bgColor		=	cOnLine ? CV_RGB(0,		0,	0)	: CV_RGB(255,	0,	0);
	txtColor	=	cOnLine ? CV_RGB(255,	0,	0)	: CV_RGB(0,		0,	0);

	SetWidgetBGColor(	kTab_Switch,		kSwitchBox_IPaddr,		bgColor);
	SetWidgetTextColor(	kTab_Switch,		kSwitchBox_IPaddr,		txtColor);

	SetWidgetBGColor(	kTab_DeviceState,	kDeviceState_IPaddr,	bgColor);
	SetWidgetTextColor(	kTab_DeviceState,	kDeviceState_IPaddr,	txtColor);

	SetWidgetBGColor(	kTab_DriverInfo,	kDriverInfo_IPaddr,		bgColor);
	SetWidgetTextColor(	kTab_DriverInfo,	kDriverInfo_IPaddr,		txtColor);
}

//*****************************************************************************
bool	ControllerSwitch::AlpacaProcessReadAll(	const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString)
{
int			switchNum;
int			boxNumber;
double		switchValueDbl;
int			switchValueInt;
bool		dataWasHandled	=	true;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR(keywordString, valueString);
	if (strcasecmp(keywordString, "connected") == 0)
	{
		cCommonProp.Connected	=  IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "version") == 0)
	{
		//*	"version": "AlpacaPi - V0.2.2-beta build #32",
		strcpy(cAlpacaVersionString, valueString);
	}
	else if (strcasecmp(keywordString, "maxswitch") == 0)
	{
		cSwitchProp.MaxSwitch	=	atoi(valueString);
	}
	else if (strncasecmp(keywordString, "getswitchdescription-", 21) == 0)
	{
		switchNum	=	atoi(&keywordString[21]);
		if ((switchNum >= 0) && (switchNum < kMaxSwitches))
		{
			strcpy(cSwitchProp.SwitchTable[switchNum].switchDescription, valueString);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Switch number out of range\t=", switchNum);
		}
	}
	else if (strncasecmp(keywordString, "getswitchname-", 14) == 0)
	{
		switchNum	=	atoi(&keywordString[14]);
		if ((switchNum >= 0) && (switchNum < kMaxSwitches))
		{
			strcpy(cSwitchProp.SwitchTable[switchNum].switchName, valueString);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Switch number out of range\t=", switchNum);
		}
	}
	else if (strncasecmp(keywordString, "getswitch-", 10) == 0)
	{
		switchNum	=	atoi(&keywordString[10]);
//		CONSOLE_DEBUG_W_NUM("switchNum\t=", switchNum);
		if ((switchNum >= 0) && (switchNum < kMaxSwitches))
		{
			boxNumber	=	kSwitchBox_State01 + (kBoxesPerSwitch * switchNum);
			if (strcasecmp(valueString, "true") == 0)
			{
				cSwitchProp.SwitchTable[switchNum].switchState	=	true;
				SetWidgetText(kTab_Switch, boxNumber, "ON");
			}
			else if (strcasecmp(valueString, "false") == 0)
			{
				cSwitchProp.SwitchTable[switchNum].switchState	=	false;
				SetWidgetText(kTab_Switch, boxNumber, "OFF");
			}
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Switch number out of range\t=", switchNum);
		}
	}
	else if (strncasecmp(keywordString, "getswitchvalue-", 15) == 0)
	{
		switchNum	=	atoi(&keywordString[15]);
		if ((switchNum >= 0) && (switchNum < kMaxSwitches))
		{
			boxNumber		=	kSwitchBox_Value01 + (kBoxesPerSwitch * switchNum);
			switchValueInt	=	atoi(valueString);
			switchValueDbl	=	AsciiToDouble(valueString);
			SetWidgetNumber(kTab_Switch, boxNumber, switchValueInt);

			boxNumber		=	kSwitchBox_Description01 + (kBoxesPerSwitch * switchNum);
			SetWidgetSliderValue(kTab_Switch, boxNumber, switchValueDbl);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Switch number out of range\t=", switchNum);
		}
	}
	else if (strncasecmp(keywordString, "maxswitchvalue-", 15) == 0)
	{
		switchNum	=	atoi(&keywordString[15]);
		if ((switchNum >= 0) && (switchNum < kMaxSwitches))
		{
			cSwitchProp.SwitchTable[switchNum].maxswitchvalue	=	AsciiToDouble(valueString);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Switch number out of range\t=", switchNum);
		}
	}
	else if (strncasecmp(keywordString, "canwrite", 8) == 0)
	{
	}
	else
	{
		dataWasHandled	=	false;
	}
	return(dataWasHandled);
}

//*****************************************************************************
void	ControllerSwitch::UpdateSupportedActions(void)
{

	SetWidgetValid(kTab_Switch,			kSwitchBox_Readall,			cHas_readall);
	SetWidgetValid(kTab_Switch,			kSwitchBox_DeviceState,		cHas_DeviceState);

	SetWidgetValid(kTab_DeviceState,	kDeviceState_Readall,		cHas_readall);
	SetWidgetValid(kTab_DeviceState,	kDeviceState_DeviceState,	cHas_DeviceState);

	SetWidgetValid(kTab_DriverInfo,		kDriverInfo_Readall,		cHas_readall);
	SetWidgetValid(kTab_DriverInfo,		kDriverInfo_DeviceState,	cHas_DeviceState);

	if (cHas_DeviceState == false)
	{
		cDeviceStateTabObjPtr->SetDeviceStateNotSupported();
	}
}

//*****************************************************************************
void	ControllerSwitch::ToggleSwitchState(const int switchNum)
{
bool			validData;
char			dataString[128];
bool			currentState;
char			newStateString[16];

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("switchNum=", switchNum);
	if ((switchNum >= 0) && (switchNum < kMaxSwitches))
	{
		//-------------------------------------------------------------------------------
		//*	SET the switch state
		currentState	=	cSwitchProp.SwitchTable[switchNum].switchState;
		if (currentState)
		{
			strcpy(newStateString, "false");
		}
		else
		{
			strcpy(newStateString, "true");
		}
		sprintf(dataString,		"Id=%d&State=%s", switchNum, newStateString);
//		CONSOLE_DEBUG_W_STR("dataString=", dataString);
		validData	=	AlpacaSendPutCmd(	"switch",	"setswitch",		dataString);

		if (validData == false)
		{
			CONSOLE_DEBUG("Failed to set switch state");
		}
		ForceAlpacaUpdate();
	}
}

//*****************************************************************************
void	ControllerSwitch::TurnAllSwitchesOff(void)
{
int		switchNum;
bool	validData;
char	dataString[128];

//	CONSOLE_DEBUG(__FUNCTION__);

	for (switchNum=0; switchNum < cSwitchProp.MaxSwitch; switchNum++)
	{
//		CONSOLE_DEBUG_W_NUM("Switch #", switchNum);
		if (cSwitchProp.SwitchTable[switchNum].canWrite)
		{
			sprintf(dataString,		"Id=%d&State=false", switchNum);
//			CONSOLE_DEBUG_W_STR("dataString=", dataString);
			validData	=	AlpacaSendPutCmd(	"switch",	"setswitch",		dataString);
			if (validData == false)
			{
				CONSOLE_DEBUG("AlpacaSendPutCmd() failed");
			}
		}
		else
		{
//			CONSOLE_DEBUG_W_NUM("Switch is read only", switchNum);
		}
	}
}
#endif // _ENABLE_CTRL_SWITCHES_
