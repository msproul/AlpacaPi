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
//*****************************************************************************


#ifdef _ENABLE_CTRL_SWITCHES_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"
#include	"helper_functions.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	535
#define	kWindowHeight	700

#include	"alpaca_defs.h"
#include	"controller.h"
#include	"controller_switch.h"
#include	"windowtab_drvrInfo.h"
#include	"windowtab_switch.h"

//**************************************************************************************
enum
{
	kTab_Switch	=	1,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerSwitch::ControllerSwitch(	const char			*argWindowName,
									struct sockaddr_in	*deviceAddress,
									const int			port,
									const int			deviceNum)
	:Controller(	argWindowName,
					kWindowWidth,
					kWindowHeight)
{
int		iii;

	cMaxSwitch				=	8;
	cAlpacaDevNum			=	deviceNum;
	cFirstDataRead			=	true;
	cLastUpdate_milliSecs	=	millis();
	strcpy(cAlpacaDeviceTypeStr,	"switch");

	//*	set the default values for the switch info
	for (iii=0; iii<kMaxSwitches; iii++)
	{
		memset(&cSwitchInfo[iii], 0, sizeof(TYPE_SWITCH_INFO));

		cSwitchInfo[iii].minswitchvalue	=	0.0;
		cSwitchInfo[iii].maxswitchvalue	=	1.0;
		cSwitchInfo[iii].switchvalue	=	0.0;
		cSwitchInfo[iii].canWrite		=	true;
	}


	if (deviceAddress != NULL)
	{
		cDeviceAddress	=	*deviceAddress;
		cPort			=	port;
		cValidIPaddr	=	true;
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
	CONSOLE_DEBUG(__FUNCTION__);
	//-----------------------------
	DELETE_OBJ_IF_VALID(cSwitchTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}


//**************************************************************************************
void	ControllerSwitch::SetupWindowControls(void)
{
char	lineBuff[64];
char	ipString[32];

//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_Switch,		"Switch");
	SetTabText(kTab_DriverInfo,	"Driver Info");
	SetTabText(kTab_About,		"About");


	cSwitchTabObjPtr	=	new WindowTabSwitch(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSwitchTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Switch,	cSwitchTabObjPtr);
		cSwitchTabObjPtr->SetParentObjectPtr(this);
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
void	ControllerSwitch::RunBackgroundTasks(bool enableDebug)
{
uint32_t	currentMillis;
uint32_t	deltaSeconds;
bool		validData;
bool		needToUpdate;

	if (cReadStartup)
	{
		AlpacaGetStartupData();
		AlpacaGetCommonProperties_OneAAT("switch");
		cReadStartup	=	false;
	}


	needToUpdate	=	false;
	currentMillis	=	millis();
	deltaSeconds	=	(currentMillis - cLastUpdate_milliSecs) / 1000;

	if (cFirstDataRead || (deltaSeconds > 10))
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
				CONSOLE_DEBUG("Failed to get data");
			}

			UpdateConnectedIndicator(kTab_Switch,		kSwitchBox_Connected);
		}
	}
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

	CONSOLE_DEBUG(__FUNCTION__);

	validData	=	AlpacaGetIntegerValue("switch", "maxswitch",	NULL,	&cMaxSwitch);
	if (validData)
	{
		CONSOLE_DEBUG_W_NUM("cMaxSwitch\t=", cMaxSwitch);
		if (cSwitchTabObjPtr != NULL)
		{
			cSwitchTabObjPtr->SetActiveSwitchCount(cMaxSwitch);
		}
	}

	validData	=	AlpacaGetStringValue("switch", "description",	NULL,	alpacaString);
	if (validData)
	{
		CONSOLE_DEBUG_W_STR("description\t=", alpacaString);
		strcpy(cAlpacaVersionString, alpacaString);
		SetWidgetText(kTab_Switch, kSwitchBox_AlpacaDrvrVersion, alpacaString);
	}

	//*	Start by getting the switch names and descriptions
	switchNum	=	0;
	CONSOLE_DEBUG_W_NUM("cMaxSwitch\t=",	cMaxSwitch);
	while (switchNum < cMaxSwitch)
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
					strcpy(cSwitchInfo[switchNum].switchName, jsonParser.dataList[jjj].valueString);
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
					strcpy(cSwitchInfo[switchNum].switchDescription, jsonParser.dataList[jjj].valueString);
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
			cSwitchInfo[switchNum].minswitchvalue	=	myDoubleValue;
		}

		//===============================================================================
		//*	Get the maximum switch value
		sprintf(dataString,		"Id=%d", switchNum);
		validData	=	AlpacaGetDoubleValue(	"switch", "maxswitchvalue",	dataString,	&myDoubleValue);
		if (validData)
		{
			cSwitchInfo[switchNum].maxswitchvalue	=	myDoubleValue;
		}

		//===============================================================================
		//*	Get the canwrite value
		sprintf(alpacaString,	"canwrite?Id=%d", switchNum);
		validData	=	AlpacaGetBooleanValue(	"switch", alpacaString,	NULL,	&myCanWriteFlag);
		if (validData)
		{
			cSwitchInfo[switchNum].canWrite	=	myCanWriteFlag;
		//	CONSOLE_DEBUG_W_NUM("switchNum\t\t=",		switchNum);
			CONSOLE_DEBUG_W_NUM("myCanWriteFlag\t=",	myCanWriteFlag);
		}

		switchNum++;
	}

//	CONSOLE_ABORT(__FUNCTION__);
	return(validData);
}

//*****************************************************************************
bool	ControllerSwitch::AlpacaGetStartupData(void)
{
bool			validData;
int				jjj;
int				boxNumber;

	CONSOLE_DEBUG(__FUNCTION__);
	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions("switch", cAlpacaDevNum);

	if (validData)
	{
		SetWidgetValid(kTab_Switch,		kSwitchBox_Readall,		cHas_readall);
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}

	if (cHas_readall)
	{
		//*	use readall to get the startup data
		validData	=	AlpacaGetStatus_ReadAll("switch", cAlpacaDevNum);
	}
	else
	{
		validData	=	AlpacaGetStartupData_OneAAT();
	}

	//===================================================================
	//*	now update all of the data.
	CONSOLE_DEBUG_W_NUM("cMaxSwitch\t=",	cMaxSwitch);

	if (cSwitchTabObjPtr != NULL)
	{
		cSwitchTabObjPtr->SetActiveSwitchCount(cMaxSwitch);
	}
	SetWidgetText(kTab_Switch,	kSwitchBox_AlpacaDrvrVersion,	cAlpacaVersionString);

	for (jjj=0; jjj<cMaxSwitch; jjj++)
	{
		CONSOLE_DEBUG_W_STR("switchName\t=",	cSwitchInfo[jjj].switchName);
		boxNumber	=	kSwitchBox_Name01 + (kBoxesPerSwitch * jjj);
		SetWidgetText(kTab_Switch, boxNumber, cSwitchInfo[jjj].switchName);

		boxNumber	=	kSwitchBox_Description01 + (kBoxesPerSwitch * jjj);
		SetWidgetText(kTab_Switch, boxNumber, cSwitchInfo[jjj].switchDescription);

		//*	check to see if it is a slider control
		if (cSwitchInfo[jjj].maxswitchvalue > 1.0)
		{
			boxNumber	=	kSwitchBox_Description01 + (kBoxesPerSwitch * jjj);
			SetWidgetType(kTab_Switch, boxNumber, kWidgetType_Slider);
			SetWidgetSliderLimits(	kTab_Switch,
									boxNumber,
									cSwitchInfo[jjj].minswitchvalue,
									cSwitchInfo[jjj].maxswitchvalue);
		}

		//*	check to see if we can write to the switch
		boxNumber	=	kSwitchBox_State01 + (kBoxesPerSwitch * jjj);
		SetWidgetValid(kTab_Switch, boxNumber++, cSwitchInfo[jjj].canWrite);
	}
	cLastUpdate_milliSecs	=	millis();

	return(validData);
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

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("cMaxSwitch\t=", cMaxSwitch);

	//*	Get switch status
	myFailureCount	=	0;
	switchNum		=	0;
	while ((switchNum < kMaxSwitches) && (switchNum < cMaxSwitch))
	{
		CONSOLE_DEBUG_W_NUM("working on switch #", switchNum);
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
						cSwitchInfo[switchNum].switchState	=	true;
						SetWidgetText(kTab_Switch, boxNumber, "ON");
					}
					else if (strcasecmp(jsonParser.dataList[jjj].valueString, "false") == 0)
					{
						cSwitchInfo[switchNum].switchState	=	false;
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
	CONSOLE_DEBUG(__FUNCTION__);
	return(validData);
}

//*****************************************************************************
bool	ControllerSwitch::AlpacaGetStatus(void)
{
bool			validData;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll("switch", cAlpacaDevNum);
	}
	else
	{
		validData	=	AlpacaGetStatus_OneAAT();
//		CONSOLE_DEBUG(__FUNCTION__);
		validData	=	AlpacaGetCommonConnectedState("switch");
//		CONSOLE_DEBUG(__FUNCTION__);
	}

	if (validData)
	{
		if (cOnLine == false)
		{
			//*	if we were previously off line, force reading startup again
			cReadStartup	=	true;
		}
		SetWidgetBGColor(	kTab_Switch,	kSwitchBox_IPaddr,	CV_RGB(0,	0,	0));
		SetWidgetTextColor(	kTab_Switch,	kSwitchBox_IPaddr,	CV_RGB(255,	0,	0));

		cOnLine	=	true;
	}
	else
	{
		SetWidgetBGColor(	kTab_Switch,	kSwitchBox_IPaddr,	CV_RGB(255,	0,	0));
		SetWidgetTextColor(	kTab_Switch,	kSwitchBox_IPaddr,	CV_RGB(0,	0,	0));
		cOnLine	=	false;
	}

	cLastUpdate_milliSecs	=	millis();
	cFirstDataRead			=	false;
	return(validData);
}

//*****************************************************************************
void	ControllerSwitch::AlpacaProcessReadAll(	const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString)
{
int			switchNum;
int			boxNumber;
double		switchValueDbl;
int			switchValueInt;

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
		cMaxSwitch	=	atoi(valueString);
	}
	else if (strncasecmp(keywordString, "getswitchdescription-", 21) == 0)
	{
		switchNum	=	atoi(&keywordString[21]);
		if ((switchNum >= 0) && (switchNum < kMaxSwitches))
		{
			strcpy(cSwitchInfo[switchNum].switchDescription, valueString);
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
			strcpy(cSwitchInfo[switchNum].switchName, valueString);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Switch number out of range\t=", switchNum);
		}
	}
	else if (strncasecmp(keywordString, "getswitch-", 10) == 0)
	{
		switchNum	=	atoi(&keywordString[10]);
		if ((switchNum >= 0) && (switchNum < kMaxSwitches))
		{
			boxNumber	=	kSwitchBox_State01 + (kBoxesPerSwitch * switchNum);
			if (strcasecmp(valueString, "true") == 0)
			{
				cSwitchInfo[switchNum].switchState	=	true;
				SetWidgetText(kTab_Switch, boxNumber, "ON");
			}
			else if (strcasecmp(valueString, "false") == 0)
			{
				cSwitchInfo[switchNum].switchState	=	false;
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
			switchValueDbl	=	atof(valueString);
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
			cSwitchInfo[switchNum].maxswitchvalue	=	atof(valueString);
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
		AlpacaProcessReadAll_Common(	deviceTypeStr,
										deviceNum,
										keywordString,
										valueString);
	}
}

//*****************************************************************************
void	ControllerSwitch::UpdateCommonProperties(void)
{
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Name,				cCommonProp.Name);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Description,		cCommonProp.Description);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
	SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);
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
		currentState	=	cSwitchInfo[switchNum].switchState;
		if (currentState)
		{
			strcpy(newStateString, "false");
		}
		else
		{
			strcpy(newStateString, "true");
		}
		sprintf(dataString,		"Id=%d&State=%s", switchNum, newStateString);
		CONSOLE_DEBUG_W_STR("dataString=", dataString);
		validData	=	AlpacaSendPutCmd(	"switch",	"setswitch",		dataString);

		if (validData == false)
		{
			CONSOLE_DEBUG("Failed to set switch state");
		}
		cForceAlpacaUpdate	=	true;
	}
}

#endif // _ENABLE_CTRL_SWITCHES_
