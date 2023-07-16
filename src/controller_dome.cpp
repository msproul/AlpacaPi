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
//*	Jun 19,	2023	<MLS> Updated constructor to use TYPE_REMOTE_DEV
//*	Jun 23,	2023	<MLS> Removed RunBackgroundTasks(), using default in parent class
//*	Jun 25,	2023	<MLS> Added AlpacaGetCapabilities()
//*	Jun 25,	2123	<ADD> Add slaved to DeviceState
//*	Jun 27,	2023	<MLS> Added AlpacaProcessReadAllIdx() to dome controller
//*	Jun 28,	2023	<MLS> Added DeviceState window to dome controller
//*	Jul 14,	2023	<MLS> Fixed Dome controller title
//*	Jul 14,	2023	<MLS> Added UpdateOnlineStatus() to dome controller
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
#include	"windowtab_DeviceState.h"
#include	"windowtab_drvrInfo.h"
#include	"windowtab_about.h"
#include	"controller_dome.h"
#include	"alpacadriver_helper.h"
#include	"dome_AlpacaCmds.h"

//**************************************************************************************
enum
{
	kTab_Dome	=	1,
	kTab_Capabilities,
	kTab_DeviceState,
	kTab_DriverInfo,
	kTab_About,

	kTab_Dome_Count

};

//**************************************************************************************
ControllerDome::ControllerDome(	const char			*argWindowName,
								TYPE_REMOTE_DEV		*alpacaDevice)
	:Controller(argWindowName, kDomeWindowWidth,  kDomeWindowHeight, true, alpacaDevice)
{
char	ipAddrStr[32];

	CONSOLE_DEBUG(__FUNCTION__);

	memset(&cDomeProp, 0, sizeof(TYPE_DomeProperties));

	SetCommandLookupTable(gDomeCmdTable);
	SetAlternateLookupTable(gDomeExtrasTable);

	strcpy(cAlpacaDeviceTypeStr,	"dome");
	cDomeProp.ShutterStatus	=	kShutterStatus_Unknown;
#ifdef _ENABLE_EXTERNAL_SHUTTER_
	cShutterInfoValid		=	false;
#endif // _ENABLE_EXTERNAL_SHUTTER_

	cDriverInfoTabNum		=	kTab_DriverInfo;
	//*	window object ptrs
	cDomeTabObjPtr			=	NULL;
	cCapabilitiesTabObjPtr	=	NULL;
	cAboutBoxTabObjPtr		=	NULL;
	cFirstDataRead			=	true;
	cLastUpdate_milliSecs	=	millis();

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
	DELETE_OBJ_IF_VALID(cDeviceStateTabObjPtr);
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
	SetTabText(kTab_DeviceState,	"Dev State");
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

	//=============================================================
	cCapabilitiesTabObjPtr		=	new WindowTabCapabilities(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cCapabilitiesTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Capabilities,	cCapabilitiesTabObjPtr);
		cCapabilitiesTabObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	cDeviceStateTabObjPtr		=	new WindowTabDeviceState(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDeviceStateTabObjPtr != NULL)
	{
		SetTabWindow(kTab_DeviceState,	cDeviceStateTabObjPtr);
		cDeviceStateTabObjPtr->SetParentObjectPtr(this);
		SetDeviceStateTabInfo(kTab_DeviceState, kDeviceState_FirstBoxName, kDeviceState_FirstBoxValue, kDeviceState_Stats);
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
bool	ControllerDome::AlpacaProcessReadAllIdx(	const char	*deviceTypeStr,
													const int	deviceNum,
													const int	keywordEnum,
													const char	*valueString)
{
bool	dataWasHandled;
//	CONSOLE_DEBUG_W_2STR("json=",	keywordString, valueString);
	dataWasHandled	=	false;
	if (strcasecmp(deviceTypeStr, "dome") == 0)
	{
		dataWasHandled	=	AlpacaProcessReadAllIdx_Dome(deviceNum, keywordEnum, valueString);
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
	return(dataWasHandled);
}

//*****************************************************************************
void	ControllerDome::UpdateSupportedActions(void)
{
	SetWidgetValid(kTab_Dome,			kDomeBox_Readall,			cHas_readall);
	SetWidgetValid(kTab_Dome,			kDomeBox_DeviceState,		cHas_DeviceState);

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

		SetWidgetText(kTab_Dome,	kDomeBox_Readall,		"RS");
#endif // _ENABLE_EXTERNAL_SHUTTER_
	}
}

//**************************************************************************************
void	ControllerDome::AlpacaGetCapabilities(void)
{
	ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"canfindhome",		"CanFindHome",		&cDomeProp.CanFindHome);
	ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"canpark",			"CanPark",			&cDomeProp.CanPark);
	ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"cansetaltitude",	"CanSetAltitude",	&cDomeProp.CanSetAltitude);
	ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"cansetazimuth",	"CanSetAzimuth",	&cDomeProp.CanSetAzimuth);
	ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"cansetpark",		"CanSetPark",		&cDomeProp.CanSetPark);
	ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"cansetshutter",	"CanSetShutter",	&cDomeProp.CanSetShutter);
	ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"canslave",			"CanSlave",			&cDomeProp.CanSlave);
	ReadOneDriverCapability(cAlpacaDeviceTypeStr,	"cansyncazimuth",	"CanSyncAzimuth",	&cDomeProp.CanSyncAzimuth);
}

//**************************************************************************************
void	ControllerDome::UpdateConnectedStatusIndicator(void)
{
	UpdateConnectedIndicator(kTab_Dome,		kDomeBox_Connected);
}

//**************************************************************************************
void	ControllerDome::UpdateStartupData(void)
{
	SetWidgetText(				kTab_Dome,	kDomeBox_Title,	cCommonProp.Name);
	UpdateAboutBoxRemoteDevice(	kTab_About, kAboutBox_CPUinfo);
}

//**************************************************************************************
void	ControllerDome::UpdateStatusData(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	UpdateConnectedIndicator(kTab_Dome,		kDomeBox_Connected);
	//======================================================================
	cUpdateDelta_secs	=	kDefaultUpdateDelta;
	if (cDomeProp.Slewing)
	{
		//*	if we slewing, we want to update more often
		cUpdateDelta_secs	=	1;
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
		cUpdateDelta_secs	=	1;
	}
}

//**************************************************************************************
void	ControllerDome::UpdateOnlineStatus(void)
{
cv::Scalar	bgColor;
cv::Scalar	txtColor;

	bgColor		=	cOnLine ? CV_RGB(0,		0,	0)	: CV_RGB(255,	0,	0);
	txtColor	=	cOnLine ? CV_RGB(255,	0,	0)	: CV_RGB(0,		0,	0);

	SetWidgetBGColor(	kTab_Dome,			kDomeBox_IPaddr,		bgColor);
	SetWidgetTextColor(	kTab_Dome,			kDomeBox_IPaddr,		txtColor);

	SetWidgetBGColor(	kTab_Capabilities,	kCapabilities_IPaddr,	bgColor);
	SetWidgetTextColor(	kTab_Capabilities,	kCapabilities_IPaddr,	txtColor);

	SetWidgetBGColor(	kTab_DeviceState,	kDeviceState_IPaddr,	bgColor);
	SetWidgetTextColor(	kTab_DeviceState,	kDeviceState_IPaddr,	txtColor);

	SetWidgetBGColor(	kTab_DriverInfo,	kDriverInfo_IPaddr,		bgColor);
	SetWidgetTextColor(	kTab_DriverInfo,	kDriverInfo_IPaddr,		txtColor);
}

//*****************************************************************************
bool	ControllerDome::AlpacaGetStartupData_OneAAT(void)
{
bool	validData;

	//*	nested so that the dome window in SkyTravel works
	validData	=	AlpacaGetStatus_DomeOneAAT();
	return(validData);
}

//*****************************************************************************
void	ControllerDome::GetStartUpData_SubClass(void)
{
	//*	this is ONLY to be implemented at the subclss level
#ifdef _ENABLE_EXTERNAL_SHUTTER_
	//=================================================
	if (cShutterInfoValid)
	{
		AlpacaGetShutterReadAll();
	}
#endif // _ENABLE_EXTERNAL_SHUTTER_
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

	ForceAlpacaUpdate();
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
		cUpdateDelta_secs	=	2;
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

#ifdef _ENABLE_SKYTRAVEL_
	#undef _ENABLE_SKYTRAVEL_
	#define	PARENT_CLASS	ControllerDome
	#include	"controller_dome_common.cpp"
#endif


#endif // _ENABLE_CTRL_DOME_
