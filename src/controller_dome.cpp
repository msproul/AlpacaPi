//*****************************************************************************
//*		controller_dome.cpp		(c) 2020 by Mark Sproul
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
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Apr 18,	2020	<MLS> Created controller_dome.cpp
//*	Apr 18,	2020	<MLS> Started on dome controller app
//*	Apr 18,	2020	<MLS> Dome controller communicating with dome
//*	Apr 19,	2020	<MLS> Dome controller working with ReadAll or One at a time
//*	Apr 21,	2020	<MLS> Added About box to dome controller
//*	May  1,	2020	<MLS> Added processing for shutter and slaved status
//*	May  1,	2020	<MLS> Added AlpacaGetShutterReadAll()
//*	May  2,	2020	<MLS> Added SetAlpacaSlitTrackerInfo()
//*	May  2,	2020	<MLS> Slit tracker display working through the network
//*	May  8,	2020	<MLS> Added UpdateShutterAltitude()
//*	May  8,	2020	<MLS> Added Slit distance logging to disk
//*	May  9,	2020	<MLS> Added Slit distance logging to memory for real time graphing
//*	May 10,	2020	<MLS> Added Slit Graph window tab
//*	May 10,	2020	<MLS> Added Slit graphing and averaging
//*	May 23,	2020	<MLS> Added UpdateSlitLog()
//*	May 31,	2020	<MLS> Added gravity vector processing
//*	Jan 14,	2021	<MLS> Dome controller working with ASCOM/Remote
//*	Jan 24,	2021	<MLS> Converted DomeController to use properties struct
//*	Feb 12,	2021	<MLS> Added driver info display to dome controller
//*	Feb 20,	2021	<MLS> Added compile time option for slit tracker tabs
//*	Feb 20,	2021	<MLS> Added capability list to dome controller
//*	Mar  9,	2023	<MLS> Removed SlitTracker code from dome controller
//*	Mar 10,	2023	<MLS> Removed SlitTracker Direct code from dome controller
//*****************************************************************************


#ifdef _ENABLE_CTRL_DOME_



#include	<math.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>

#include	"alpaca_defs.h"
#include	"helper_functions.h"

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"controller.h"
#include	"windowtab_dome.h"
#include	"windowtab_about.h"
#include	"controller_dome.h"



//**************************************************************************************
ControllerDome::ControllerDome(	const char			*argWindowName,
								TYPE_REMOTE_DEV		*alpacaDevice)
	:Controller(argWindowName, kDomeWindowWidth,  kDomeWindowHeight)
{
char	ipAddrStr[32];

	CONSOLE_DEBUG(__FUNCTION__);

	memset(&cDomeProp, 0, sizeof(TYPE_DomeProperties));

	strcpy(cAlpacaDeviceTypeStr,	"dome");
	cDomeProp.ShutterStatus	=	kShutterStatus_Unknown;

#ifdef _ENABLE_EXTERNAL_SHUTTER_
	cShutterInfoValid		=	false;
#endif // _ENABLE_EXTERNAL_SHUTTER_

	//*	window object ptrs
	cDomeTabObjPtr			=	NULL;
	cCapabilitiesTabObjPtr	=	NULL;
	cAboutBoxTabObjPtr		=	NULL;

	cFirstDataRead			=	true;
	cLastUpdate_milliSecs	=	millis();
	cDomeUpdateDelta		=	kDefaultUpdateDelta;

	if (alpacaDevice != NULL)
	{
		cValidIPaddr	=	true;

		cAlpacaDevNum	=	alpacaDevice->alpacaDeviceNum;
		cDeviceAddress	=	alpacaDevice->deviceAddress;
		cPort			=	alpacaDevice->port;

		strcpy(cAlpacaDeviceNameStr,	alpacaDevice->deviceNameStr);
//		CONSOLE_DEBUG_W_NUM("deviceTypeEnum\t=", alpacaDevice->deviceTypeEnum);
		if (alpacaDevice->deviceTypeEnum == kDeviceType_Shutter)
		{
			CONSOLE_DEBUG("Switching to SHUTTER!!!!!!!!!!!!!!!!");
			strcpy(cAlpacaDeviceTypeStr,	"shutter");
		}
	#ifdef _ENABLE_SKYTRAVEL_
		//*	make a 2nd copy if we are in SKYTRAVEL
	//	cDomeIpAddress			=	cDeviceAddress;
	//	cDomeIpPort				=	cPort;
	//	cDomeAlpacaDeviceNum	=	cPort;
	#endif

		CheckConnectedState();		//*	check connected and connect if not already connected
	}

	inet_ntop(AF_INET, &(cDeviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
	CONSOLE_DEBUG_W_STR("IP address=", ipAddrStr);

	SetupWindowControls();

#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerDome::~ControllerDome(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	DELETE_OBJ_IF_VALID(cDomeTabObjPtr);
	DELETE_OBJ_IF_VALID(cCapabilitiesTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
void	ControllerDome::SetupWindowControls(void)
{
char	lineBuff[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Dome_Count);
	SetTabText(kTab_Dome,			"Dome");
	SetTabText(kTab_Capabilities,	"Capabilities");
	SetTabText(kTab_DriverInfo,		"Driver Info");
	SetTabText(kTab_About,			"About");



	//=============================================================
	cDomeTabObjPtr	=	new WindowTabDome(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDomeTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Dome,	cDomeTabObjPtr);
		cDomeTabObjPtr->SetParentObjectPtr(this);
		cDomeTabObjPtr->SetDomePropertiesPtr(&cDomeProp);
	}

	//--------------------------------------------
	cCapabilitiesTabObjPtr		=	new WindowTabCapabilities(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cCapabilitiesTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Capabilities,	cCapabilitiesTabObjPtr);
		cCapabilitiesTabObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	cDriverInfoTabObjPtr		=	new WindowTabDriverInfo(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDriverInfoTabObjPtr != NULL)
	{
		SetTabWindow(kTab_DriverInfo,	cDriverInfoTabObjPtr);
		cDriverInfoTabObjPtr->SetParentObjectPtr(this);
	}
	//=============================================================
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

		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);

		SetWindowIPaddrInfo(lineBuff, true);
	}
}

//*****************************************************************************
void	ControllerDome::UpdateCommonProperties(void)
{
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Name,				cCommonProp.Name);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Description,		cCommonProp.Description);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
	SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);

}

//**************************************************************************************
void	ControllerDome::SetAlpacaShutterInfo(TYPE_REMOTE_DEV *alpacaDevice)
{
	CONSOLE_DEBUG(__FUNCTION__);

	if (alpacaDevice != NULL)
	{
#ifdef _ENABLE_EXTERNAL_SHUTTER_
		cShutterInfoValid		=	true;

		cShutterDeviceAddress	=	alpacaDevice->deviceAddress;
		cShutterPort			=	alpacaDevice->port;
		cShutterAlpacaDevNum	=	alpacaDevice->alpacaDeviceNum;

		SetWidgetText(kTab_Dome,		kDomeBox_Readall,		"RS");
#endif // _ENABLE_EXTERNAL_SHUTTER_
	}
}

//**************************************************************************************
void	ControllerDome::RunBackgroundTasks(const char *callingFunction, bool enableDebug)
{
uint32_t	currentMillis;
uint32_t	deltaSeconds;
bool		validData;
bool		needToUpdate;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cReadStartup)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		AlpacaGetStartupData();
		AlpacaGetCommonProperties_OneAAT(cAlpacaDeviceTypeStr);
		UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);

		cDomeTabObjPtr->UpdateControls();

		cReadStartup	=	false;
	}

//	CONSOLE_DEBUG_W_NUM("cDomeUpdateDelta\t=", cDomeUpdateDelta);

	needToUpdate	=	false;
	currentMillis	=	millis();
	deltaSeconds	=	(currentMillis - cLastUpdate_milliSecs) / 1000;

	if (cFirstDataRead || (deltaSeconds >= cDomeUpdateDelta))
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
	//	CONSOLE_DEBUG_W_NUM("Updating Dome info: cDomeUpdateDelta\t=", cDomeUpdateDelta);
		//*	is the IP address valid
		if (cValidIPaddr)
		{
			validData	=	AlpacaGetStatus();
			if (validData == false)
			{
				CONSOLE_DEBUG("Failed to get data")
			}
			UpdateConnectedIndicator(kTab_Dome,		kDomeBox_Connected);
		}
	}
}

//*****************************************************************************
bool	ControllerDome::AlpacaGetStartupData(void)
{
bool			validData;
char			returnString[128];

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("cAlpacaDeviceTypeStr\t=",	cAlpacaDeviceTypeStr);
	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions(cAlpacaDeviceTypeStr, cAlpacaDevNum);
	if (validData)
	{
		SetWidgetValid(kTab_Dome,			kDomeBox_Readall,		cHas_readall);
		if (cHas_readall == false)
		{
			validData	=	AlpacaGetStringValue(	cAlpacaDeviceTypeStr, "driverversion",	NULL,	returnString);
			if (validData)
			{
				strcpy(cAlpacaVersionString, returnString);
			}
		}
//		CONSOLE_DEBUG_W_NUM("cDomeProp.CanSyncAzimuth\t=",	cDomeProp.CanSyncAzimuth);
		ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"canfindhome",		"CanFindHome",		&cDomeProp.CanFindHome);
		ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"canpark",			"CanPark",			&cDomeProp.CanPark);
		ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"cansetaltitude",	"CanSetAltitude",	&cDomeProp.CanSetAltitude);
		ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"cansetazimuth",	"CanSetAzimuth",	&cDomeProp.CanSetAzimuth);
		ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"cansetpark",		"CanSetPark",		&cDomeProp.CanSetPark);
		ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"cansetshutter",	"CanSetShutter",	&cDomeProp.CanSetShutter);
		ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"canslave",			"CanSlave",			&cDomeProp.CanSlave);
		ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"cansyncazimuth",	"CanSyncAzimuth",	&cDomeProp.CanSyncAzimuth);
//		CONSOLE_DEBUG_W_NUM("cDomeProp.CanSyncAzimuth\t=",	cDomeProp.CanSyncAzimuth);
//		CONSOLE_ABORT(__FUNCTION__);

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
bool	ControllerDome::AlpacaGetStatus(void)
{
bool	validData;
bool	previousOnLineState;

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, cDebugCounter++);

	previousOnLineState	=	cOnLine;
	if (cHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll(cAlpacaDeviceTypeStr, cAlpacaDevNum);
	}
	else
	{
		validData	=	AlpacaGetStatus_DomeOneAAT();
		validData	=	AlpacaGetCommonConnectedState(cAlpacaDeviceTypeStr);
	}
#ifdef _ENABLE_EXTERNAL_SHUTTER_
	//=================================================
	if (cShutterInfoValid)
	{
		AlpacaGetShutterReadAll();
	}
#endif // _ENABLE_EXTERNAL_SHUTTER_

	if (validData)
	{
		if (cOnLine == false)
		{
			//*	if we were previously off line, force reading startup again
			cReadStartup	=	true;
		}
		cOnLine	=	true;

		//======================================================================
		cDomeUpdateDelta	=	kDefaultUpdateDelta;
		if (cDomeProp.Slewing)
		{
			//*	if we slewing, we want to update more often
			cDomeUpdateDelta	=	1;
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Slewing");
		}
		else if (cDomeProp.AtHome)
		{
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Home");
		}
		else if (cDomeProp.AtPark)
		{
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Park");
		}
		else
		{
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Stopped");
		}

		//*	if we shutter is opening or closing, we want to update more often
		if ((cDomeProp.ShutterStatus == kShutterStatus_Opening) || (cDomeProp.ShutterStatus == kShutterStatus_Closing))
		{
			cDomeUpdateDelta	=	1;
		}
	}
	else
	{
		cOnLine	=	false;
	}
	if (cOnLine != previousOnLineState)
	{
		SetWindowIPaddrInfo(NULL, cOnLine);
	}

	cLastUpdate_milliSecs	=	millis();
	cFirstDataRead			=	false;
	return(validData);
}

//*****************************************************************************
void	ControllerDome::AlpacaProcessReadAll(	const char	*deviceType,
											const int	deviceNum,
											const char	*keywordString,
											const char *valueString)
{
//	CONSOLE_DEBUG_W_2STR("json=",	keywordString, valueString);
	if (strcasecmp(deviceType, cAlpacaDeviceTypeStr) == 0)
	{
		AlpacaProcessReadAll_Dome(deviceNum, keywordString, valueString);
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
}

//*****************************************************************************
//*	if this routine gets overloaded, the first part, checking for "readall" must be preserved
//*****************************************************************************
void	ControllerDome::AlpacaProcessSupportedActions(const char *deviceType, const int deviveNum, const char *valueString)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceType);

	if (strcasecmp(deviceType, cAlpacaDeviceTypeStr) == 0)
	{
		AlpacaProcessSupportedActions_Dome(deviveNum, valueString);
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
}

//**************************************************************************************
void	ControllerDome::UpdateCapabilityList(void)
{
	UpdateCapabilityListID(kTab_Capabilities, kCapabilities_TextBox1, kCapabilities_TextBoxN);
}

#ifdef  _ENABLE_EXTERNAL_SHUTTER_
//*****************************************************************************
void	ControllerDome::AlpacaGetShutterReadAll(void)
{
SJP_Parser_t		jsonParser;
bool				validData;
char				alpacaString[128];
int					jjj;
TYPE_ShutterStatus	newShutterStatus;

//	CONSOLE_DEBUG(__FUNCTION__);

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", "shutter", cShutterAlpacaDevNum, "readall");

	validData	=	GetJsonResponse(	&cShutterDeviceAddress,
										cShutterPort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
		cLastAlpacaErrNum	=	kASCOM_Err_Success;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
//			CONSOLE_DEBUG_W_STR(	jsonParser.dataList[jjj].keyword,
//									jsonParser.dataList[jjj].valueString);

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "shutterstatus") == 0)
			{
				newShutterStatus	=	(TYPE_ShutterStatus)atoi(jsonParser.dataList[jjj].valueString);
				UpdateShutterStatus(newShutterStatus);
			}
			else if (strcasecmp(jsonParser.dataList[jjj].keyword, "altitude") == 0)
			{
			double	myNewAltitude;

				myNewAltitude	=	AsciiToDouble(jsonParser.dataList[jjj].valueString);
				UpdateShutterAltitude(myNewAltitude);

			}
		}
		if (cShutterCommFailed)
		{
			SetWidgetText(kTab_Dome, kDomeBox_AlpacaErrorMsg, "---");
			SetWidgetTextColor(kTab_Dome,	kDomeBox_ShutterStatus,	CV_RGB(0,	255,	0));
		}
		cShutterCommFailed	=	false;
		cShutterCommFailCnt	=	0;
	}
	else
	{
		cShutterCommFailed	=	true;
		cShutterCommFailCnt++;
		sprintf(alpacaString, "Failed to read data from shutter - %d", cShutterCommFailCnt);
		SetWidgetText(kTab_Dome, kDomeBox_AlpacaErrorMsg, alpacaString);

//?		cShutterStatus	=	kShutterStatus_Unknown;
		SetWidgetText(kTab_Dome, kDomeBox_ShutterStatus, "Unknown");
		SetWidgetTextColor(kTab_Dome,	kDomeBox_ShutterStatus,	CV_RGB(255,	0,	0));
	}
}

int	gClientID				=	1368;
int	gClientTransactionID	=	0;
//*****************************************************************************
//*	this is identical to AlpacaSendPutCmd but it sends it to the shutter device address
//*****************************************************************************
bool	ControllerDome::ShutterSendPutCmd(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString)
{
SJP_Parser_t	jsonParser;
bool			sucessFlag;
char			alpacaString[128];
char			myDataString[512];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);

	SJP_Init(&jsonParser);

	sprintf(alpacaString, "/api/v1/%s/%d/%s", alpacaDevice, cShutterAlpacaDevNum, alpacaCmd);
	if (strlen(dataString) > 0)
	{
		sprintf(myDataString, "%s&ClientID=%d&ClientTransactionID=%d",
												dataString,
												gClientID,
												gClientTransactionID);
	}
	else
	{
		sprintf(myDataString, "ClientID=%d&ClientTransactionID=%d",
												gClientID,
												gClientTransactionID);
	}
	CONSOLE_DEBUG_W_STR("Sending", myDataString);
	sucessFlag	=	SendPutCommand(	&cShutterDeviceAddress,
									cShutterPort,
									alpacaString,
									"",
									&jsonParser);
	strcpy(cLastAlpacaCmdString, alpacaString);
	gClientTransactionID++;

	cForceAlpacaUpdate	=	true;
	return(sucessFlag);
}
#endif // _ENABLE_EXTERNAL_SHUTTER_


//*****************************************************************************
void	ControllerDome::SendShutterCommand(const char *shutterCmd)
{

	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _ENABLE_EXTERNAL_SHUTTER_
	//*	this is for a separate alpaca device called "shutter"
	if (cShutterInfoValid)
	{
	bool	sucessFlag;

		sucessFlag	=	ShutterSendPutCmd("shutter", shutterCmd, "");
		if (sucessFlag == false)
		{
			CONSOLE_DEBUG_W_STR("ShutterSendPutCmd failed", shutterCmd);
		}

		//*	any time we send a command to the shutter, increase the update rate
		cDomeUpdateDelta	=	2;
	}
	else
	{
		CONSOLE_DEBUG("No shutter info");
		SetWidgetText(kTab_Dome, kDomeBox_AlpacaErrorMsg, "Shutter controller not found");
	}
#else
	//*	normal, send command to the dome controller.
	#warning "SendShutterCommand needs to be finished"

#endif
}


//*****************************************************************************
void	ControllerDome::ToggleSwitchState(const int switchNum)
{
//SJP_Parser_t	jsonParser;
//bool			validData;
//char			alpacaString[128];
//char			dataString[128];


}


#ifdef _ENABLE_SKYTRAVEL_
	#undef _ENABLE_SKYTRAVEL_
	#define	PARENT_CLASS	ControllerDome
	#include	"controller_dome_common.cpp"
#endif


#endif // _ENABLE_CTRL_DOME_
