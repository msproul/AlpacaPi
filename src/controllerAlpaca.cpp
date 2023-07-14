//*****************************************************************************
//*		controllerAlpaca.cpp		(c) 2020 by Mark Sproul
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
//*	Mar  2,	2020	<MLS> Added AlpacaGetIntegerValue()
//*	Mar  3,	2020	<MLS> Added AlpacaGetBooleanValue()
//*	Mar 17,	2020	<MLS> Created controllerAlpaca.cpp
//*	Mar 17,	2020	<MLS> Added AlpacaGetSupportedActions()
//*	Mar 17,	2020	<MLS> Added AlpacaProcessSupportedActions()
//*	Mar 17,	2020	<MLS> Added AlpacaCheckForErrors()
//*	Mar 19,	2020	<MLS> Added ClientID and ClientTransactionID to data request
//*	Apr 15,	2020	<MLS> Fixed dataString bug in AlpacaGet... routines
//*	Jun 22,	2020	<MLS> Added AlpacaGetIntegerArray()
//*	Jun 24,	2020	<MLS> Added UpdateDownloadProgress()
//*	Jan  9,	2021	<MLS> Added new version of AlpacaGetStatus_ReadAll()
//*	Jan  9,	2021	<MLS> Added new version of AlpacaGetSupportedActions()
//*	Jan 10,	2021	<MLS> Added new version of AlpacaSendPutCmdwResponse()
//*	Jan 12,	2021	<MLS> Added AlpacaGetStringValue()
//*	Jan 30,	2021	<MLS> Added AlpacaGetImageArray()
//*	Jan 31,	2021	<MLS> Finished debugging AlpacaGetImageArray()
//*	Feb  9,	2021	<MLS> Added AlpacaGetCommonProperties_OneAAT()
//*	Feb 13,	2021	<MLS> Added UpdateCommonProperties()
//*	Sep  8,	2021	<MLS> Added AlpacaSetConnected()
//*	Dec 14,	2021	<MLS> Started working on support for imageBinary
//*	Dec 15,	2021	<MLS> AlpacaGetImageArray() binary working with 16 bit monochrome
//*	Dec 16,	2021	<MLS> AlpacaGetImageArray() binary working with 16 bit Color (RGB)
//*	Sep  4,	2022	<MLS> Added Alpaca_GetRemoteCPUinfo()
//*	Sep  4,	2022	<MLS> Added UpdateAboutBoxRemoteDevice()
//*	Sep  5,	2022	<MLS> About box now updated to display specs of remote device
//*	Sep 26,	2022	<MLS> Fixed return data valid bug in all routines
//*	Oct 16,	2022	<MLS> Added Alpaca_GetTemperatureLog()
//*	Oct 26,	2022	<MLS> Fixed return data valid bug in AlpacaGetStringValue()
//*	Dec 17,	2022	<MLS> Added GetConfiguredDevices() (moved from controller_camera)
//*	Dec 17,	2022	<MLS> Added ProcessConfiguredDevices()
//*	Mar  8,	2023	<MLS> Added AlpacaGetStringValue() with ipaddr arg
//*	Mar 10,	2023	<MLS> Added new version of  AlpacaGetBooleanValue()
//*	May 29,	2023	<MLS> Fixed bug in Alpaca_GetTemperatureLog(), un-initialized variable
//*	Jun 18,	2023	<MLS> Added AlpacaCheckForDeviceState() & AlpacaGetStatus_DeviceState()
//*	Jun 23,	2023	<MLS> Added UpdateDeviceStateEntry()
//*	Jun 23,	2023	<MLS> Added SetCommandLookupTable()
//*	Jun 25,	2023	<MLS> Added AlpacaGetStartupData_OneAAT() to controller class
//*	Jun 25,	2023	<MLS> Added AlpacaGetCapabilities()
//*	Jun 26,	2023	<MLS> Added AlpacaProcessReadAllIdx()
//*	Jul  1,	2023	<MLS> Added SetCommandLookupTable() with TYPE_CmdEntry
//*	Jul  1,	2023	<MLS> Added LookupCmdInCmdTable()
//*	Jul  1,	2023	<MLS> Added SetAlternateLookupTable()
//*****************************************************************************

#ifdef _CONTROLLER_USES_ALPACA_

#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>
#include	<errno.h>



#include	"discovery_lib.h"
#include	"sendrequest_lib.h"
#include	"helper_functions.h"

#define	_DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"widget.h"
#include	"controller.h"
#include	"windowtab_drvrInfo.h"

static int	gClientID				=	1;
static int	gClientTransactionID	=	1;

#include	"common_AlpacaCmds.h"
#include	"common_AlpacaCmds.cpp"

//*****************************************************************************
void	Controller::SetCommandLookupTable(TYPE_CmdEntry *newLookupTable)
{
	cCommandEntryPtr	=	newLookupTable;
}
//*****************************************************************************
void	Controller::SetAlternateLookupTable(TYPE_CmdEntry *newLookupTable)
{
	cAlternateEntryPtr	=	newLookupTable;
}

//*****************************************************************************
//*	this should be over-ridden
//*****************************************************************************
bool	Controller::AlpacaGetStartupData(void)
{
//	CONSOLE_DEBUG("AlpacaGetStartupData() has not been implemented for this controller");
//	CONSOLE_DEBUG("This routine has been replaced with GetStartUpData_SubClass()");
//	CONSOLE_ABORT(__FUNCTION__);
	return(false);
}

//*****************************************************************************
//*	this should be over-ridden
//*****************************************************************************
void	Controller::AlpacaGetCapabilities(void)
{
//	CONSOLE_DEBUG("AlpacaGetCapabilities() has not been implemented for this controller");
//	CONSOLE_DEBUG(cWindowName);
//	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
//curl -X PUT "https://virtserver.swaggerhub.com/ASCOMInitiative/api/v1/telescope/0/connected"
//		-H  "accept: application/json"
//		-H  "Content-Type: application/x-www-form-urlencoded"
//		-d "Connected=true&ClientID=11&ClientTransactionID=22"
//*****************************************************************************
bool	Controller::AlpacaSetConnected(const char *deviceTypeStr, const bool newConnectedState)
{
bool			validData;
char			dataString[128];
SJP_Parser_t	jsonParser;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);


	CONSOLE_DEBUG(__FUNCTION__);
	sprintf(dataString,		"Connected=%s", (newConnectedState ? "true" : "false"));
	CONSOLE_DEBUG_W_STR("deviceTypeStr\t=", deviceTypeStr);
	CONSOLE_DEBUG_W_STR("dataString\t=", dataString);
	validData	=	AlpacaSendPutCmdwResponse(	deviceTypeStr,
												"connected",
												dataString,
												&jsonParser);

	if (validData)
	{
//		CONSOLE_DEBUG("CONNECT Success");
	}
	else
	{
		CONSOLE_DEBUG("CONNECT failed");
	}
//	SJP_DumpJsonData(&jsonParser);
//	CONSOLE_ABORT(__FUNCTION__);

	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaGetCommonConnectedState(const char *deviceTypeStr)
{
bool	validData;
bool	myConnectedFlag;
bool	prevConnectedState;
bool	returnedValid;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceTypeStr);
	validData		=	false;

	//-----------------------------------------------------------------------------------------
//	CONSOLE_DEBUG_W_STR("deviceTypeStr=", deviceTypeStr);
	validData	=	AlpacaGetBooleanValue(	deviceTypeStr,
											"connected",
											NULL,
											&myConnectedFlag,
											&returnedValid,
											false);
	if (validData)
	{
	//	CONSOLE_DEBUG(__FUNCTION__);
		//*	keep track of the previous state
		prevConnectedState		=	cCommonProp.Connected;
		cCommonProp.Connected	=	myConnectedFlag;
//		CONSOLE_DEBUG_W_NUM("cCommonProp.Connected\t=", cCommonProp.Connected);
		if (cCommonProp.Connected && (prevConnectedState == false))
		{
			//*	if we are going from un-connected to connected, read the start up data
			cReadStartup	=	true;
		}
	}
	else
	{
		CONSOLE_DEBUG("Failed to get connected status");
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	return(validData);
}

//*****************************************************************************
void	Controller::UpdateConnectedIndicator(const int tabNum, const int widgetNum)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cCommonProp.Connected)
	{
		SetWidgetText(		tabNum,	widgetNum,	"C");
		SetWidgetTextColor(	tabNum,	widgetNum,	CV_RGB(0,	255,	0));
	}
	else
	{
		SetWidgetText(		tabNum,	widgetNum,	"NC");
		SetWidgetTextColor(	tabNum,	widgetNum,	CV_RGB(255,	0,	0));
		SetWidgetTextColor(	tabNum,	widgetNum,	CV_RGB(255,	255,	255));
	}
}

//*****************************************************************************
bool	Controller::AlpacaGetCommonProperties_OneAAT(const char *deviceTypeStr)
{
char	returnString[256];
bool	validData;
bool	myOnLineFlag;
bool	myConnectedFlag;
int		validCnt;
int		returnStrLen;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("Reading common properties for", cWindowName);
//	CONSOLE_DEBUG_W_BOOL("Has read all\t=", cHas_readall);
	validCnt		=	0;
	myOnLineFlag	=	true;	//*	assume it is on line
	validData		=	false;

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
//		CONSOLE_DEBUG_W_STR("deviceTypeStr\t=", deviceTypeStr);
		validData	=	AlpacaGetBooleanValue(	deviceTypeStr, "connected",	NULL,	&myConnectedFlag);
		if (validData)
		{
			cCommonProp.Connected	=	myConnectedFlag;
			validCnt++;
//			CONSOLE_DEBUG_W_BOOL("cCommonProp.Connected\t=", cCommonProp.Connected);
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		validData	=	AlpacaGetStringValue(	deviceTypeStr, "description",	NULL,	returnString);
		if (validData)
		{
			returnStrLen	=	strlen(returnString);
			if (returnStrLen < kCommonPropMaxStrLen)
			{
				strcpy(cCommonProp.Description,	returnString);
			}
			else
			{
				CONSOLE_DEBUG_W_STR(	"returnString    \t=", returnString);
				CONSOLE_DEBUG_W_SIZE(	"returnString-len\t=", strlen(returnString));
			}
			validCnt++;
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		validData	=	AlpacaGetStringValue(	deviceTypeStr, "driverinfo",	NULL,	returnString);
		if (validData)
		{
			returnStrLen	=	strlen(returnString);
			if (returnStrLen < kCommonPropMaxStrLen)
			{
				strcpy(cCommonProp.DriverInfo,	returnString);
			}
			else
			{
				CONSOLE_DEBUG_W_STR(	"returnString    \t=", returnString);
				CONSOLE_DEBUG_W_SIZE(	"returnString-len\t=", strlen(returnString));
			}
			validCnt++;
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}
//	CONSOLE_DEBUG(__FUNCTION__);

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		validData	=	AlpacaGetStringValue(	deviceTypeStr, "driverinfo",	NULL,	returnString);
		if (validData)
		{
			returnStrLen	=	strlen(returnString);
			if (returnStrLen < kCommonPropMaxStrLen)
			{
				strcpy(cCommonProp.DriverInfo,	returnString);
			}
			else
			{
				CONSOLE_DEBUG_W_STR(	"returnString    \t=", returnString);
				CONSOLE_DEBUG_W_SIZE(	"returnString-len\t=", strlen(returnString));
			}
			validCnt++;
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}
//	CONSOLE_DEBUG(__FUNCTION__);

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		validData	=	AlpacaGetStringValue(	deviceTypeStr, "interfaceversion",	NULL,	returnString);
		if (validData)
		{
			returnStrLen	=	strlen(returnString);
			if (returnStrLen < kCommonPropMaxStrLen)
			{
				cCommonProp.InterfaceVersion	=	atoi(returnString);
			}
			else
			{
				CONSOLE_DEBUG_W_STR(	"returnString    \t=",	returnString);
				CONSOLE_DEBUG_W_SIZE(	"returnString-len\t=",	strlen(returnString));
			}
			validCnt++;
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		validData	=	AlpacaGetStringValue(	deviceTypeStr, "driverversion",	NULL,	returnString);
		if (validData)
		{
			returnStrLen	=	strlen(returnString);
			if (returnStrLen < kCommonPropMaxStrLen)
			{
				strcpy(cCommonProp.DriverVersion,	returnString);
			}
			else
			{
				CONSOLE_DEBUG_W_STR(	"returnString    \t=",	returnString);
				CONSOLE_DEBUG_W_SIZE(	"returnString-len\t=",	strlen(returnString));
			}
			validCnt++;
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}
	CONSOLE_DEBUG(__FUNCTION__);

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		validData	=	AlpacaGetStringValue(	deviceTypeStr, "name",	NULL,	returnString, NULL);
		if (validData)
		{
			returnStrLen	=	strlen(returnString);
			if (returnStrLen < kCommonPropMaxStrLen)
			{
				strcpy(cCommonProp.Name,	returnString);
			}
			else
			{
				CONSOLE_DEBUG_W_STR(	"returnString    \t=", returnString);
				CONSOLE_DEBUG_W_SIZE(	"returnString-len\t=", strlen(returnString));
			}
			validCnt++;
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}
//	CONSOLE_DEBUG_W_NUM("validCnt\t=", validCnt);
//	CONSOLE_DEBUG_W_BOOL("Has read all\t=", cHas_readall);

	Alpaca_GetRemoteCPUinfo();

	UpdateCommonProperties();
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaGetStartupData_OneAAT(void)
{
	//*	needs to be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be overloaded!!!!!!!!!!!!!!!!!!");
	CONSOLE_DEBUG(cWindowName);
//	CONSOLE_ABORT(__FUNCTION__);
	return(false);
}

//*****************************************************************************
void	Controller::UpdateCommonProperties(void)
{
	if (cDriverInfoTabNum >= 0)
	{
		SetWidgetText(		cDriverInfoTabNum,	kDriverInfo_Name,				cCommonProp.Name);
		SetWidgetText(		cDriverInfoTabNum,	kDriverInfo_Description,		cCommonProp.Description);
		SetWidgetText(		cDriverInfoTabNum,	kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
		SetWidgetText(		cDriverInfoTabNum,	kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
		SetWidgetNumber(	cDriverInfoTabNum,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);
	}
	else
	{
		//*	needs to be overloaded
		CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be overloaded!!!!!!!!!!!!!!!!!!");
	}
}


//*****************************************************************************
bool	Controller::AlpacaGetSupportedActions(	sockaddr_in	*deviceAddress,
												int			devicePort,
												const char	*deviceTypeStr,
												const int	deviceNum)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;

	if (gVerbose)
	{
	char			ipAddrStr[32];
		CONSOLE_DEBUG_W_STR("Windowname\t=", cWindowName);
		CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceTypeStr);

		inet_ntop(AF_INET, &(deviceAddress->sin_addr), ipAddrStr, INET_ADDRSTRLEN);
		CONSOLE_DEBUG_W_STR("IP address=", ipAddrStr);
		CONSOLE_DEBUG_W_NUM("devicePort=", devicePort);
		CONSOLE_DEBUG_W_NUM("deviceNum=", deviceNum);
	}

	//===============================================================
	//*	get supportedactions
	SJP_Init(&jsonParser);
//	CONSOLE_DEBUG("Dumping EMPTY Json parser");
//	SJP_DumpJsonData(&jsonParser);
//	CONSOLE_DEBUG_W_NUM("tokenCount_Data\t=", jsonParser.tokenCount_Data);
	sprintf(alpacaString,	"/api/v1/%s/%d/supportedactions", deviceTypeStr, deviceNum);
	validData	=	GetJsonResponse(	deviceAddress,
										devicePort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
		jjj	=	0;
		while (jjj < jsonParser.tokenCount_Data)
		{
//			if (strcmp(deviceTypeStr, "dome") == 0)
//			{
//				CONSOLE_DEBUG_W_NUM("jjj\t=", jjj);
//				CONSOLE_DEBUG_W_2STR("kw:val=", jsonParser.dataList[jjj].keyword,
//												jsonParser.dataList[jjj].valueString);
//			}
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "ARRAY") == 0)
			{
				jjj++;	//*	skip over the ARRAY entry

//				CONSOLE_DEBUG_W_NUM("ARRAY found, jjj\t=", jjj);
//				CONSOLE_DEBUG_W_STR("keyword\t=", jsonParser.dataList[jjj].keyword);
				while ((jjj < jsonParser.tokenCount_Data) &&
						(jsonParser.dataList[jjj].keyword[0] != ']'))
				{
//					CONSOLE_DEBUG_W_STR("Calling AlpacaProcessSupportedActions", deviceTypeStr);
					AlpacaProcessSupportedActions(	deviceTypeStr,
													deviceNum,
													jsonParser.dataList[jjj].valueString);
					jjj++;
				}
			}
			jjj++;
		}
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	return(validData);
}


//*****************************************************************************
bool	Controller::AlpacaGetSupportedActions(const char *deviceTypeStr, const int deviceNum)
{
bool			validData;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceTypeStr);
	cHas_readall	=	false;
	validData		=	AlpacaGetSupportedActions(	&cDeviceAddress,
													cPort,
													deviceTypeStr,
													deviceNum);
	return(validData);
}

//*****************************************************************************
//*	if this routine gets overloaded, the first part, checking for "readall" must be preserved
//*****************************************************************************
void	Controller::AlpacaProcessSupportedActions(const char *deviceTypeStr, const int deviveNum, const char *valueString)
{
//	CONSOLE_DEBUG(cWindowName);
//	CONSOLE_DEBUG_W_STR(deviceTypeStr, valueString);

	if (strcasecmp(valueString, "readall") == 0)
	{
		cHas_readall	=	true;
	}
	else if (strcasecmp(valueString, "devicestate") == 0)
	{
		cHas_DeviceState	=	true;
	}
	else if (strcasecmp(valueString, "temperaturelog") == 0)
	{
		cHas_temperaturelog	=	true;
	}
	else if (strcasecmp(valueString, "foo") == 0)
	{
		//*	you get the idega
	}
//	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
int	Controller::LookupCmdInCmdTable(const char *commandString, TYPE_CmdEntry *commandTable, TYPE_CmdEntry *alternateTable)
{
int		iii;
int		enumValue;

//	CONSOLE_DEBUG_W_STR("commandString\t=", commandString);
	enumValue	=	-1;
	iii			=	0;
	while ((commandTable[iii].commandName[0] != 0) && (enumValue < 0))
	{
		if (strcasecmp(commandString, commandTable[iii].commandName) == 0)
		{
			enumValue	=	commandTable[iii].enumValue;
		}
		iii++;
	}

	if ((enumValue < 0) && (alternateTable != NULL))
	{
		iii			=	0;
		while ((alternateTable[iii].commandName[0] != 0) && (enumValue < 0))
		{
			if (strcasecmp(commandString, alternateTable[iii].commandName) == 0)
			{
				enumValue	=	alternateTable[iii].enumValue;
			}
			iii++;
		}
	}
	return(enumValue);
}


//*****************************************************************************
//*	added new version of this 1/9/2021 to allow multiple devices
//*****************************************************************************
bool	Controller::AlpacaGetStatus_ReadAll(	sockaddr_in	*deviceAddress,
												int			devicePort,
												const char	*deviceTypeStr,
												const int	deviceNum,
												const bool	enableDebug)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
bool			dataWasHandled	=	true;
int				keywordEnum;
int				notHandledCnt;

#ifdef _DEBUG_READALL_
	CONSOLE_DEBUG("-----------------------------------------------------------------");
	CONSOLE_DEBUG(cWindowName);
	CONSOLE_DEBUG_W_STR("Requesting 'readall' for", deviceTypeStr);
#endif

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/readall", deviceTypeStr, deviceNum);
	validData	=	GetJsonResponse(	deviceAddress,
										devicePort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
		if (enableDebug)
		{
			SJP_DumpJsonData(&jsonParser, __FUNCTION__);
		}
		cLastAlpacaErrNum	=	kASCOM_Err_Success;

		//----------------------------------------------------------------
		//*	there are 2 ways of doing this, the 2nd way was added Jun 26,2023
		//*	this new way looks up in pre-defined table the command
		//*	and passes the enum value for the keyword instead of the keyword string.
		//*	this makes the subclass readall parser easier to read
		//*	each subclass should only implement ONE of these methods.
		//*	however there is nothing stopping the subclass from doing the lookup itself
		//----------------------------------------------------------------
//		SJP_DumpJsonData(&jsonParser, __FUNCTION__);
		notHandledCnt	=	0;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
			//*	check for valid string
			if (strlen(jsonParser.dataList[jjj].keyword) > 0)
			{
				//*	special debugging
//				if (cAlpacaDeviceType == kDeviceType_Telescope)
//				{
//					CONSOLE_DEBUG_W_2STR(	deviceTypeStr,
//											jsonParser.dataList[jjj].keyword,
//											jsonParser.dataList[jjj].valueString);
//				}
				dataWasHandled	=	false;
				//-------------------------------------------------------------------------------------
				//*	Look for the command in the COMMON command list AND the Extras list
				keywordEnum		=	LookupCmdInCmdTable(jsonParser.dataList[jjj].keyword, gCommonCmdTable, gExtrasCmdTable);
				if (keywordEnum >= 0)
				{
					dataWasHandled	=	AlpacaProcessReadAll_CommonIdx(	deviceTypeStr,
																		deviceNum,
																		keywordEnum,
																		jsonParser.dataList[jjj].valueString);
				}
				else if (cCommandEntryPtr != NULL)
				{
					keywordEnum	=	LookupCmdInCmdTable(jsonParser.dataList[jjj].keyword, cCommandEntryPtr, cAlternateEntryPtr);
				}

				if (dataWasHandled == false)
				{
					if (keywordEnum >= 0)
					{
						dataWasHandled	=	AlpacaProcessReadAllIdx(deviceTypeStr,
																	deviceNum,
																	keywordEnum,
																	jsonParser.dataList[jjj].valueString);
					}
					else if (strncasecmp(jsonParser.dataList[jjj].keyword, "COMMENT", 7) == 0)
					{
						dataWasHandled	=	true;
					}
					else if (strcasestr(jsonParser.dataList[jjj].keyword, "-STR") != NULL)
					{
						dataWasHandled	=	true;
					}
					else
					{
//						CONSOLE_DEBUG_W_NUM("cAlpacaDeviceType    \t=", cAlpacaDeviceType);
//						if (cAlpacaDeviceType == kDeviceType_Management)
//						{
//							CONSOLE_DEBUG_W_STR("deviceTypeStr\t=", deviceTypeStr);
//							CONSOLE_DEBUG_W_2STR(	"Calling AlpacaProcessReadAll():",
//													jsonParser.dataList[jjj].keyword,
//													jsonParser.dataList[jjj].valueString);
//						}
						dataWasHandled	=	AlpacaProcessReadAll(	deviceTypeStr,
																	deviceNum,
																	jsonParser.dataList[jjj].keyword,
																	jsonParser.dataList[jjj].valueString);
//						CONSOLE_DEBUG_W_BOOL("dataWasHandled\t=", dataWasHandled);
					}
					if (dataWasHandled == false)
					{
						notHandledCnt++;
					#ifdef _DEBUG_READALL_
						CONSOLE_DEBUG_W_2STR(	"NOT HANDLED:",
												jsonParser.dataList[jjj].keyword,
												jsonParser.dataList[jjj].valueString);
					#endif
					}
				}
//				CONSOLE_DEBUG_W_BOOL("dataWasHandled\t=",	dataWasHandled);
			}
		}
	}
	else
	{
		dataWasHandled	=	false;
	}
#ifdef _DEBUG_READALL_
	CONSOLE_DEBUG_W_NUM("notHandledCnt\t=", notHandledCnt);
#endif
	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaGetStatus_ReadAll(const char *deviceTypeStr, const int deviceNum, const bool	enableDebug)
{
bool			validData;

//	CONSOLE_DEBUG(cWindowName);

	validData	=	AlpacaGetStatus_ReadAll(&cDeviceAddress,
											cPort,
											deviceTypeStr,
											deviceNum,
											enableDebug);
	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaProcessReadAll(	const char	*deviceTypeStr,
											const int	deviceNum,
											const char	*keywordString,
											const char	*valueString)
{
	//*	this function should be overloaded
//	CONSOLE_DEBUG(cWindowName);
//	CONSOLE_DEBUG_W_2STR(deviceTypeStr,	keywordString, valueString);
//	CONSOLE_ABORT(cWindowName)
	return(false);
}

//*****************************************************************************
bool	Controller::AlpacaProcessReadAllIdx(const char	*deviceTypeStr,
											const int	deviceNum,
											const int	keywordEnum,
											const char	*valueString)
{
	//*	this function should be overloaded
	CONSOLE_DEBUG("New version of AlpacaProcessReadAll(), with enum");
	CONSOLE_ABORT(cWindowName);
	return(false);
}


//*****************************************************************************
bool	Controller::AlpacaProcessReadAll_CommonIdx(const char	*deviceTypeStr,
													const int	deviceNum,
													const int	keywordEnum,
													const char	*valueString)
{
bool	dataWasHandled;

//	CONSOLE_DEBUG(__FUNCTION__);

	dataWasHandled	=	true;
	switch(keywordEnum)
	{
		case kCmd_Common_action:	//*	Invokes the specified device-specific action.
			break;

		case kCmd_Common_commandblind:				//*	Transmits an arbitrary string to the device
			break;

		case kCmd_Common_commandbool:				//*	Transmits an arbitrary string to the device and returns a boolean value from the device.
			break;

		case kCmd_Common_commandstring:				//*	Transmits an arbitrary string to the device and returns a string value from the device.
			break;

		case kCmd_Common_connected:					//*	GET--Retrieves the connected state of the device
			cCommonProp.Connected	=	IsTrueFalse(valueString);
			break;

		case kCmd_Common_Connect:
			break;

		case kCmd_Common_Connecting:
			cCommonProp.Connecting	=	IsTrueFalse(valueString);
			break;

		case kCmd_Common_Disconnect:
			break;

		case kCmd_Common_DeviceState:
			break;

		case kCmd_Common_description:				//*	Device description
			strcpy(cCommonProp.Description,		valueString);
			break;

		case kCmd_Common_driverinfo:				//*	Device driver description
			strcpy(cCommonProp.DriverInfo,		valueString);
			break;

		case kCmd_Common_driverversion:				//*	Driver Version
			strcpy(cCommonProp.DriverVersion,	valueString);
			break;

		case kCmd_Common_interfaceversion:			//*	The ASCOM Device interface version number that this device supports.
			cCommonProp.InterfaceVersion	=	atoi(valueString);
			break;

		case kCmd_Common_name:						//*	Device name
			strcpy(cCommonProp.Name,			valueString);
			break;

		case kCmd_Common_supportedactions:			//*	Returns the list of action names supported by this driver.
			break;


		//*	Extra commands
		//*	these are data values that I have added to the alpaca json stream NOT in the standard Alpaca spec
		case kCmd_Extra_Version:
			strcpy(cAlpacaVersionString, valueString);	//*	"version": "AlpacaPi - V0.2.2-beta build #32",
			break;

		case kCmd_Extra_ClientTransactionID:
		case kCmd_Extra_Command:
		case kCmd_Extra_Comment:
		case kCmd_Extra_Device:
		case kCmd_Extra_ErrorMessage:
		case kCmd_Extra_ErrorNumber:
		case kCmd_Extra_ReadAll:
		case kCmd_Extra_ServerTransactionID:
		case kCmd_Extra_Array:
		case kCmd_Extra_UTCDate:
		case kCmd_Extra_Bracket:
//			CONSOLE_DEBUG("Extras processed correctly");
			dataWasHandled	=	true;
			break;

		default:
			CONSOLE_DEBUG_W_NUM("Not handled, keywordEnum\t=", keywordEnum);
			CONSOLE_DEBUG_W_STR("valueString=", valueString);
			dataWasHandled	=	false;
	}

	return(dataWasHandled);
}

//*****************************************************************************
bool	Controller::AlpacaProcessReadAll_Common(const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString)
{
bool	dataWasHandled;
int		keywordEnum;

	CONSOLE_DEBUG(cWindowName);
	CONSOLE_DEBUG_W_STR("json=", valueString);

	dataWasHandled	=	false;
	keywordEnum		=	LookupCmdInCmdTable(keywordString, gCommonCmdTable, gExtrasCmdTable);
	if (keywordEnum >= 0)
	{
		dataWasHandled	=	AlpacaProcessReadAll_CommonIdx(deviceTypeStr, deviceNum, keywordEnum, valueString);
	}
	return(dataWasHandled);
}

//*****************************************************************************
bool	Controller::AlpacaSendPutCmdwResponse(	sockaddr_in		*deviceAddress,
												int				devicePort,
												const char		*alpacaDevice,
												const int		alpacaDevNum,
												const char		*alpacaCmd,
												const char		*dataString,
												SJP_Parser_t	*jsonParser)
{
char		alpacaString[128];
bool		sucessFlag;
char		myDataString[512]	=	"";
int			dataStrLen;
//bool		myReturnDataIsValid;


//	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);

	if (jsonParser  != NULL)
	{
		SJP_Init(jsonParser);

		sprintf(alpacaString, "/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);

		dataStrLen	=	0;
		if (dataString != NULL)
		{
			dataStrLen	=	strlen(dataString);
		}

		if (dataStrLen > 0)
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
//		CONSOLE_DEBUG_W_STR("Sending:", myDataString);
		sucessFlag	=	SendPutCommand(	deviceAddress,
										devicePort,
										alpacaString,
										myDataString,
										jsonParser);
		if (sucessFlag == false)
		{
			CONSOLE_DEBUG_W_STR("SendPutCommand Failed!!!", alpacaString);
			CONSOLE_DEBUG_W_STR("cLastAlpacaErrStr\t=", cLastAlpacaErrStr);
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(jsonParser, cLastAlpacaErrStr, true);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
			CONSOLE_DEBUG_W_STR("alpacaString       \t=",	alpacaString);
			CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum  \t=",	cLastAlpacaErrNum);
			SJP_DumpJsonData(jsonParser, __FUNCTION__);
//			myReturnDataIsValid	=	false;
		}

		ForceAlpacaUpdate();
		gClientTransactionID++;

		strcpy(cLastAlpacaCmdString, alpacaString);
	}
	else
	{
		CONSOLE_ABORT("Internal error, jsonParser is NULL");
	}

	return(sucessFlag);
}

//*****************************************************************************
//*
//*		curl -X PUT "https://virtserver.swaggerhub.com/ASCOMInitiative/api/v1/camera/0/gain"
//*			-H  "accept: application/json"
//*			-H  "Content-Type: application/x-www-form-urlencoded"
//*			-d "Gain=77&ClientID=34&ClientTransactionID=12"
//*****************************************************************************
bool	Controller::AlpacaSendPutCmdwResponse(	const char		*alpacaDevice,
												const char		*alpacaCmd,
												const char		*dataString,
												SJP_Parser_t	*jsonParser)
{
bool			sucessFlag;

//	CONSOLE_DEBUG(__FUNCTION__);

	sucessFlag	=	AlpacaSendPutCmdwResponse(	&cDeviceAddress,
												cPort,
												alpacaDevice,
												cAlpacaDevNum,
												alpacaCmd,
												dataString,
												jsonParser);
	return(sucessFlag);
}

//*****************************************************************************
bool	Controller::AlpacaSendPutCmd(	const char	*alpacaDevice,
										const char	*alpacaCmd,
										const char	*dataString)
{
SJP_Parser_t	jsonParser;
bool			sucessFlag;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);

	sucessFlag	=	AlpacaSendPutCmdwResponse(alpacaDevice, alpacaCmd, dataString, &jsonParser);

	return(sucessFlag);
}

//*****************************************************************************
bool	Controller::AlpacaGetIntegerValue(	struct sockaddr_in	deviceAddress,
											int					port,
											int					alpacaDevNum,
											const char			*alpacaDevice,
											const char			*alpacaCmd,
											const char			*dataString,
											int					*returnValue,
											bool				*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
int				myIntgerValue;
bool			myReturnDataIsValid	=	true;

//	CONSOLE_DEBUG(__FUNCTION__);

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);
//	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);

	validData	=	GetJsonResponse(	&deviceAddress,
										port,
										alpacaString,
										dataString,
										&jsonParser);
	if (validData)
	{
		cLastAlpacaErrNum	=	kASCOM_Err_Success;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				myIntgerValue	=	atoi(jsonParser.dataList[jjj].valueString);
				if (returnValue != NULL)
				{
					*returnValue	=	myIntgerValue;
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
			CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum        \t=",	cLastAlpacaErrNum);
			myReturnDataIsValid	=	false;
		}
	}
	else
	{
		myReturnDataIsValid	=	false;
		cReadFailureCnt++;
	}
	//*	does the calling routine want to know if the data was good
	if (rtnValidData != NULL)
	{
		*rtnValidData	=	myReturnDataIsValid;
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	return(validData);
}


//*****************************************************************************
bool	Controller::AlpacaGetIntegerValue(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString,
											int			*returnValue,
											bool		*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				myIntgerValue;
int				jjj;
bool			myReturnDataIsValid	=	true;

	//*	set the default valid data flag to TRUE
	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, cAlpacaDevNum, alpacaCmd);
	if (gVerbose)
	{
	char			ipAddrStr[32];

		CONSOLE_DEBUG("------------------------------");
		CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

		inet_ntop(AF_INET, &(cDeviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
		CONSOLE_DEBUG_W_STR("IP address   \t=",	ipAddrStr);
		CONSOLE_DEBUG_W_NUM("cPort        \t=",	cPort);
		CONSOLE_DEBUG_W_STR("alpacaString \t=",	alpacaString);
		CONSOLE_DEBUG_W_NUM("cAlpacaDevNum\t=",	cAlpacaDevNum);
	}
	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										alpacaString,
										dataString,
										&jsonParser);
	if (validData)
	{
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
//			CONSOLE_DEBUG_W_2STR("json=",	jsonParser.dataList[jjj].keyword,
//											jsonParser.dataList[jjj].valueString);

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				myIntgerValue	=	atoi(jsonParser.dataList[jjj].valueString);
				if (returnValue != NULL)
				{
					*returnValue	=	myIntgerValue;
				}
			}
		}
//		SJP_DumpJsonData(&jsonParser, __FUNCTION__);
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
			CONSOLE_DEBUG("Alpaca error occured.................");
			CONSOLE_DEBUG_W_STR("alpacaString     \t=",	alpacaString);
			CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t=",	cLastAlpacaErrNum);
			myReturnDataIsValid	=	false;
		}
	}
	else
	{
		cReadFailureCnt++;
		myReturnDataIsValid	=	false;
	}
	//*	does the calling routine want to know if the data was good
	if (rtnValidData != NULL)
	{
		*rtnValidData	=	myReturnDataIsValid;
	}
	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaGetStringValue(	struct sockaddr_in	deviceAddress,
											int					port,
											int					alpacaDevNum,
											const char			*alpacaDevice,
											const char			*alpacaCmd,
											const char			*dataString,
											char				*returnString,
											bool				*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
bool			myReturnDataIsValid	=	true;

//	CONSOLE_DEBUG(__FUNCTION__);
	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, cAlpacaDevNum, alpacaCmd);

	validData	=	GetJsonResponse(	&deviceAddress,
										port,
										alpacaString,
										dataString,
										&jsonParser);

	if (validData)
	{
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				if (returnString != NULL)
				{
					strcpy(returnString, jsonParser.dataList[jjj].valueString);
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
			CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum        \t=",	cLastAlpacaErrNum);
			CONSOLE_DEBUG_W_STR("alpacaString     \t=",	alpacaString);
			myReturnDataIsValid	=	false;
		}
	}
	else
	{
		myReturnDataIsValid	=	false;
		cReadFailureCnt++;
	}
	//*	does the calling routine want to know if the data was good
	if (rtnValidData != NULL)
	{
		*rtnValidData	=	myReturnDataIsValid;
	}
//	Set_SendRequestLibDebug(false);
	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaGetStringValue(	const char			*alpacaDevice,
											const char			*alpacaCmd,
											const char			*dataString,
											char				*returnString,
											bool				*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
bool			myReturnDataIsValid	=	true;

//	CONSOLE_DEBUG(__FUNCTION__);
	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, cAlpacaDevNum, alpacaCmd);

//	Set_SendRequestLibDebug(true);
//	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);
	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										alpacaString,
										dataString,
										&jsonParser);
//	CONSOLE_DEBUG_W_BOOL("validData\t=", validData);
	if (validData)
	{
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
//			CONSOLE_DEBUG_W_2STR("json=",	jsonParser.dataList[jjj].keyword,
//											jsonParser.dataList[jjj].valueString);

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				if (returnString != NULL)
				{
					strcpy(returnString, jsonParser.dataList[jjj].valueString);
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
			CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t=",	cLastAlpacaErrNum);
			CONSOLE_DEBUG_W_NUM("cLastAlpacaErrStr\t=",	cLastAlpacaErrStr);
			CONSOLE_DEBUG_W_STR("alpacaString     \t=",	alpacaString);
			myReturnDataIsValid	=	false;
		}
	}
	else
	{
		myReturnDataIsValid	=	false;
		cReadFailureCnt++;
	}
	//*	does the calling routine want to know if the data was good
	if (rtnValidData != NULL)
	{
		*rtnValidData	=	myReturnDataIsValid;
	}
//	Set_SendRequestLibDebug(false);
	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaGetDoubleValue(	struct sockaddr_in	deviceAddress,
											int					port,
											int					alpacaDevNum,
											const char			*alpacaDevice,
											const char			*alpacaCmd,
											const char			*dataString,
											double				*returnValue,
											bool				*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
double			myDoubleValue;
bool			myReturnDataIsValid	=	true;


	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);
//	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);
	validData	=	GetJsonResponse(	&deviceAddress,
										port,
										alpacaString,
										dataString,
										&jsonParser);
//	CONSOLE_DEBUG(__FUNCTION__);
	if (validData)
	{
		cLastAlpacaErrNum	=	kASCOM_Err_Success;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
//				CONSOLE_DEBUG_W_STR("VALUE string\t=", jsonParser.dataList[jjj].valueString);

				myDoubleValue	=	AsciiToDouble(jsonParser.dataList[jjj].valueString);

				//*	make sure there is someplace valid to put it
				if (returnValue != NULL)
				{
					*returnValue	=	myDoubleValue;
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
//			CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum        \t=",	cLastAlpacaErrNum);
			myReturnDataIsValid	=	false;
		}
	}
	else
	{
		myReturnDataIsValid	=	false;
		cReadFailureCnt++;
	}
	//*	does the calling routine want to know if the data was good
	if (rtnValidData != NULL)
	{
		*rtnValidData	=	myReturnDataIsValid;
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	return(validData);
}



//*****************************************************************************
//*	return value is true if the message was sent and a response was received
//*	The *validData flag indicates if data was found
//*****************************************************************************
bool	Controller::AlpacaGetDoubleValue(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString,
											double		*returnValue,
											bool		*rtnValidData)
{
bool		validData;

	validData	=	AlpacaGetDoubleValue(	cDeviceAddress,
											cPort,
											cAlpacaDevNum,
											alpacaDevice,
											alpacaCmd,
											dataString,
											returnValue,
											rtnValidData);
	return(validData);
}

//*****************************************************************************
//*	this is a very special case, but it gets used at least twice by the telescope controller
//*****************************************************************************
bool	Controller::AlpacaGetMinMax(	const char	*alpacaDevice,
										const char	*alpacaCmd,
										const char	*dataString,
										double		*returnMinValue,
										double		*returnMaxValue,
										bool		*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
double			myDoubleValue;
bool			myReturnDataIsValid	=	true;

//	CONSOLE_DEBUG(__FUNCTION__);
	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, cAlpacaDevNum, alpacaCmd);

	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										alpacaString,
										dataString,
										&jsonParser);
	if (validData)
	{
		cLastAlpacaErrNum	=	kASCOM_Err_Success;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "MINIMUM") == 0)
			{
				myDoubleValue	=	AsciiToDouble(jsonParser.dataList[jjj].valueString);
				if (returnMinValue != NULL)
				{
					*returnMinValue	=	myDoubleValue;
				}
			}
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "MAXIMUM") == 0)
			{
				myDoubleValue	=	AsciiToDouble(jsonParser.dataList[jjj].valueString);
				if (returnMaxValue != NULL)
				{
					*returnMaxValue	=	myDoubleValue;
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
			CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t=",	cLastAlpacaErrNum);
			CONSOLE_DEBUG_W_STR("alpacaString     \t=",	alpacaString);
			myReturnDataIsValid	=	false;
		}
	}
	else
	{
		myReturnDataIsValid	=	false;
		cReadFailureCnt++;
	}
	//*	does the calling routine want to know if the data was good
	if (rtnValidData != NULL)
	{
		*rtnValidData	=	myReturnDataIsValid;
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaGetBooleanValue(	struct sockaddr_in	deviceAddress,
											int					port,
											int					alpacaDevNum,
											const char			*alpacaDevice,
											const char			*alpacaCmd,
											const char			*dataString,
											bool				*returnValue,
											bool				*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
bool			myBooleanValue;
bool			myReturnDataIsValid	=	true;


	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);
//	if (printDebug)
//	{
//		CONSOLE_DEBUG_W_2STR(__FUNCTION__, alpacaDevice, alpacaCmd);
//		CONSOLE_DEBUG(alpacaString);
//	}

	validData	=	GetJsonResponse(	&deviceAddress,
										port,
										alpacaString,
										dataString,
										&jsonParser);
	if (validData)
	{
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
			//*	is debugging enabled
//			if (printDebug)
//			{
//				CONSOLE_DEBUG_W_2STR("json=",	jsonParser.dataList[jjj].keyword,
//												jsonParser.dataList[jjj].valueString);
//			}

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				myBooleanValue	=	IsTrueFalse(jsonParser.dataList[jjj].valueString);

				if (returnValue != NULL)
				{
					*returnValue	=	myBooleanValue;
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
//			CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t=",	cLastAlpacaErrNum);
//			CONSOLE_DEBUG_W_STR("alpacaString     \t=",	alpacaString);
			myReturnDataIsValid	=	false;
		}
	}
	else
	{
		myReturnDataIsValid	=	false;
		cReadFailureCnt++;
		CONSOLE_DEBUG_W_2STR("Failed", alpacaString, dataString);
	}
	//*	does the calling routine want to know if the data was good
	if (rtnValidData != NULL)
	{
		*rtnValidData	=	myReturnDataIsValid;
	}
	return(validData);

}

//*****************************************************************************
bool	Controller::AlpacaGetBooleanValue(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString,
											bool		*returnValue,
											bool		*rtnValidData,
											bool		printDebug)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
bool			myBooleanValue;
bool			myReturnDataIsValid	=	true;


	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, cAlpacaDevNum, alpacaCmd);
	if (printDebug)
	{
		CONSOLE_DEBUG_W_2STR(__FUNCTION__, alpacaDevice, alpacaCmd);
		CONSOLE_DEBUG(alpacaString);
	}

//	if (strcmp(alpacaDevice, "switch") == 0)
//	{
//		CONSOLE_DEBUG_W_2STR("alpacaString\t=", alpacaString, dataString);
//	}

	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										alpacaString,
										dataString,
										&jsonParser);
	if (validData)
	{
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
			//*	is debugging enabled
			if (printDebug)
			{
				CONSOLE_DEBUG_W_2STR("json=",	jsonParser.dataList[jjj].keyword,
												jsonParser.dataList[jjj].valueString);
			}

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				myBooleanValue	=	IsTrueFalse(jsonParser.dataList[jjj].valueString);

				if (returnValue != NULL)
				{
					*returnValue	=	myBooleanValue;
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
//			CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t=",	cLastAlpacaErrNum);
//			CONSOLE_DEBUG_W_STR("alpacaString     \t=",	alpacaString);
			myReturnDataIsValid	=	false;
		}
	}
	else
	{
		myReturnDataIsValid	=	false;
		cReadFailureCnt++;
		CONSOLE_DEBUG_W_2STR("Failed", alpacaString, dataString);
	}
	//*	does the calling routine want to know if the data was good
	if (rtnValidData != NULL)
	{
		*rtnValidData	=	myReturnDataIsValid;
	}
	return(validData);
}


//*****************************************************************************
void	Controller::UpdateDownloadProgress(const int unitsRead, const int totalUnits)
{
	//*	this is to be over loaded if needed
	CONSOLE_ABORT(__FUNCTION__);
}


#if 0
//*****************************************************************************
static int	CountChars(const char *theString, const char theChar)
{
int		charCnt;
int		iii;

	charCnt	=	0;
	iii		=	0;
	while (theString[iii] != 0)
	{
		if (theString[iii] == theChar)
		{
			charCnt++;
		}
		iii++;
	}
//	CONSOLE_DEBUG_W_NUM("charCnt\t=", charCnt);
	return(charCnt);
}
#endif // 0


//*****************************************************************************
TYPE_ASCOM_STATUS	Controller::AlpacaCheckForErrors(	SJP_Parser_t	*jsonParser,
														char			*errorMsg,
														bool 			reportError)
{
int					jjj;
TYPE_ASCOM_STATUS	alpacaErrorCode;
bool				foundErrNum;
bool				foundErrStr;
char				errorReportStr[256];

//	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrorCode	=	kASCOM_Err_UnspecifiedError;
	foundErrNum		=	false;
	foundErrStr		=	false;
	strcpy(errorMsg, "");
	if (jsonParser != NULL)
	{
		for (jjj=0; jjj<jsonParser->tokenCount_Data; jjj++)
		{
			if (strcasecmp(jsonParser->dataList[jjj].keyword, "ErrorNumber") == 0)
			{
				alpacaErrorCode	=	(TYPE_ASCOM_STATUS)atoi(jsonParser->dataList[jjj].valueString);
				foundErrNum		=	true;
			}
			else if (strcasecmp(jsonParser->dataList[jjj].keyword, "ErrorMessage") == 0)
			{
				if (strlen(jsonParser->dataList[jjj].valueString) > 0)
				{
					foundErrStr		=	true;
					strcpy(errorMsg, jsonParser->dataList[jjj].valueString);
				}
			}
		}
		if (reportError)
		{
			if (foundErrNum && foundErrStr)
			{
				sprintf(errorReportStr, "E#%d - %s", alpacaErrorCode, errorMsg);
			}
			else
			{
				strcpy(errorReportStr, errorMsg);
			}
			AlpacaDisplayErrorMessage(errorReportStr);
		}
	}
	else
	{
		CONSOLE_DEBUG("jsonParser is NULL");
	}
	return(alpacaErrorCode);
}


//*****************************************************************************
int	Controller::Alpaca_GetRemoteCPUinfo(void)
{
bool			validData;
SJP_Parser_t	jsonParser;
int				returnCode;
int				jjj;
char			alpacaString[128];

//	CONSOLE_DEBUG(__FUNCTION__);
	returnCode	=	-1;

	if (cGetCPUinfoCallCnt > 0)
	{
		CONSOLE_DEBUG_W_NUM("Alpaca_GetRemoteCPUinfo had already been called!!! Count=", cGetCPUinfoCallCnt);
	}


	cGetCPUinfoCallCnt++;
	//*	 if it has readall, then it has cpustats
	if (cHas_readall)
	{
		SJP_Init(&jsonParser);
		//	http://wo102:6800/api/v1/management/0/cpustats
		sprintf(alpacaString,	"/api/v1/management/%d/cpustats", 0);	//*	the device number for management is always 0

		validData	=	GetJsonResponse(	&cDeviceAddress,
											cPort,
											alpacaString,
											NULL,
											&jsonParser);
		if (validData)
		{
			returnCode	=	0;
//			SJP_DumpJsonData(&jsonParser, __FUNCTION__);
			for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
			{

				if (strcasecmp(jsonParser.dataList[jjj].keyword, "PLATFORM") == 0)
				{
					strcpy(cRemote_Platform, jsonParser.dataList[jjj].valueString);
				}
				else if (strcasecmp(jsonParser.dataList[jjj].keyword, "CPUINFO") == 0)
				{
					strcpy(cRemote_CPUinfo, jsonParser.dataList[jjj].valueString);
				}
				else if (strcasecmp(jsonParser.dataList[jjj].keyword, "OPERATINGSYSTEM") == 0)
				{
					strcpy(cRemote_OperatingSystem, jsonParser.dataList[jjj].valueString);
				}
				else if (strcasecmp(jsonParser.dataList[jjj].keyword, "VERSION") == 0)
				{
					strcpy(cRemote_Version, jsonParser.dataList[jjj].valueString);
				}

			}
//			CONSOLE_DEBUG(cRemote_Platform);
//			CONSOLE_DEBUG(cRemote_CPUinfo);
//			CONSOLE_DEBUG(cRemote_OperatingSystem);
//			CONSOLE_DEBUG(cRemote_Version);
		}
		else
		{
			CONSOLE_DEBUG("Invalid return data");
		}
	}
	else
	{
		CONSOLE_DEBUG_W_STR("device does not have Readall", cWindowName);
		cGetCPUinfoCallCnt	=	0;
	}
	return(returnCode);
}

//*****************************************************************************
void	Controller::UpdateAboutBoxRemoteDevice(const int tabNumber, const int widgetNumber)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	if (cHas_readall)
	{
	char	multieLineText[800];

		if (cGetCPUinfoCallCnt == 0)
		{
//			CONSOLE_DEBUG("Calling Alpaca_GetRemoteCPUinfo()");
			Alpaca_GetRemoteCPUinfo();
		}

		strcpy(multieLineText,	"REMOTE DEVICE:\r\n");
		strcat(multieLineText,	cRemote_Platform);
		strcat(multieLineText,	"\r\n");
		strcat(multieLineText,	cRemote_CPUinfo);
		strcat(multieLineText,	"\r\n");
		strcat(multieLineText,	cRemote_OperatingSystem);
		strcat(multieLineText,	"\r\n");
		strcat(multieLineText,	cRemote_Version);
		strcat(multieLineText,	"\r\n");

		SetWidgetText(tabNumber, widgetNumber, multieLineText);

//		CONSOLE_DEBUG(multieLineText);
	}
	else
	{
//		CONSOLE_DEBUG_W_STR("device does not have Readall", cWindowName);
//		CONSOLE_DEBUG(cRemote_Platform);
//		CONSOLE_DEBUG(cRemote_CPUinfo);
//		CONSOLE_DEBUG(cRemote_OperatingSystem);
//		CONSOLE_DEBUG(cRemote_Version);
//		CONSOLE_ABORT(__FUNCTION__);
	}
}

#define		kTempLogBuffLen	10000

//*****************************************************************************
//*	returns number of data points read
//*	-1 means error
//*****************************************************************************
int	Controller::Alpaca_GetTemperatureLog(	const char	*alpacaDeviceString,
											const int	alpacaDeviceNumber,
											double		*temperatureLog,
											const int	maxBufferSize)
{
char			alpacaString[128];
int				shutDownRetCode;
int				closeRetCode;
char			theChar;
char			linebuf[kReadBuffLen];
bool			readingHttpHeader;
int				ccc;
char			keywordStr[kLineBufSize];
char			valueStr[kLineBufSize];
char			returnedData[kTempLogBuffLen + 10];
int				recvdByteCnt;
int				totalBytesRead;
int				linesProcessed;
int				ranOutOfRoomCnt;
bool			keepReading;
int				socketReadCnt;
int				data_iii;
int				socket_desc;
bool			valueFoundFlag;
int				dataIndex;
int				firstCharNotDigitCnt	=	0;
int				braceCnt;
double			myDoubleValue;
int				commaCounter;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaDeviceString);
	if ((temperatureLog == NULL) || (maxBufferSize <= 0))
	{
		CONSOLE_DEBUG("No place to put data");
		return(-1);
	}

	dataIndex		=	0;
	commaCounter	=	0;
	braceCnt		=	0;
	sprintf(alpacaString,	"/api/v1/%s/%d/temperaturelog", alpacaDeviceString, alpacaDeviceNumber);

	strcpy(cLastAlpacaCmdString, alpacaString);
//	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);

	socket_desc	=	OpenSocketAndSendRequest(	&cDeviceAddress,
												cPort,
												"GET",	//*	must be either GET or PUT
												alpacaString,
												"",
												false);
	if (socket_desc >= 0)
	{
//		CONSOLE_DEBUG("Success: Connection open and data sent");
		valueFoundFlag		=	false;
		keepReading			=	true;
		readingHttpHeader	=	true;
		data_iii			=	0;
		linesProcessed		=	0;
		totalBytesRead		=	0;
		socketReadCnt		=	0;
		ranOutOfRoomCnt		=	0;
		ccc					=	0;
		while (keepReading)
		{
			recvdByteCnt	=	recv(socket_desc, returnedData , kTempLogBuffLen , 0);
			if (recvdByteCnt > 0)
			{
				socketReadCnt++;

				totalBytesRead				+=	recvdByteCnt;
				returnedData[recvdByteCnt]	=	0;
				data_iii					=	0;

//				CONSOLE_DEBUG_W_NUM("recvdByteCnt      	\t=", recvdByteCnt);
//				CONSOLE_DEBUG_W_NUM("totalBytesRead     \t=", totalBytesRead);
//				CONSOLE_DEBUG_W_BOOL("readingHttpHeader\t=", readingHttpHeader);
//				CONSOLE_DEBUG_W_BOOL("valueFoundFlag    \t=", valueFoundFlag);
//				CONSOLE_DEBUG(returnedData);

			#if 0
				Alpaca_ProcessTempLogData(returnedData);
			#else
				//----------------------------------------------------------------
				//*	this part reads and processes the HTTP header
				while (readingHttpHeader && (data_iii < recvdByteCnt))
				{
					theChar	=	returnedData[data_iii];
					if ((theChar == 0x0d) || (theChar == 0x0a))
					{
						//*	null terminate the line
						linebuf[ccc]	=	0;
//						CONSOLE_DEBUG_W_STR("linebuf     \t=", linebuf);

						if (strlen(linebuf) > 0)
						{
							//*	process the header line
							JSON_ExtractKeyword_Value(linebuf, keywordStr, valueStr);
//							CONSOLE_DEBUG_W_2STR("JSON:", keywordStr, valueStr);
						//	ProcessHTTPheaderLine(linebuf, &cHttpHdrStruct);
						}
						else
						{
							//*	Done with the header
//							CONSOLE_DEBUG("Done with the header");
							readingHttpHeader	=	false;
						}

						//*	reset for next line
						ccc				=	0;
						linebuf[ccc]	=	0;

						//*	check for the lf of cr/lf
						if ((returnedData[data_iii + 1] == 0x0a))
						{
							data_iii++;
						}
					}
					else
					{
						linebuf[ccc]	=	theChar;
						ccc++;
					}
					data_iii++;
				}
//				CONSOLE_DEBUG_W_NUM("data_iii    \t=", data_iii);
//				CONSOLE_DEBUG_W_NUM("recvdByteCnt\t=", recvdByteCnt);

				//*	dont reset data_iii
				for (; data_iii<recvdByteCnt; data_iii++)
				{
					theChar	=	returnedData[data_iii];
					if (theChar == '{')
					{
						braceCnt++;
					}
					else if (theChar == '}')
					{
						braceCnt--;
					}

					if (valueFoundFlag && ((theChar == ',') || (theChar == ']')))
					{
						linebuf[ccc]	=	0;
						if (isdigit(linebuf[0]) || (linebuf[0] == '-'))
						{
							myDoubleValue	=	atof(linebuf);
//							if (myDoubleValue > 1.0)
//							{
//								CONSOLE_DEBUG_W_DBL("myDoubleValue\t=", myDoubleValue);
//							}
							if (dataIndex < maxBufferSize)
							{
								temperatureLog[dataIndex]	=	myDoubleValue;
								dataIndex++;
							}
							else
							{
								ranOutOfRoomCnt++;
								if (ranOutOfRoomCnt < 10)
								{
									CONSOLE_DEBUG("Ran out of room");
								}
							}
						}
						else
						{
//							CONSOLE_DEBUG_W_STR("First char not digit=", linebuf);
							firstCharNotDigitCnt++;
						}
						ccc				=	0;
						linebuf[ccc]	=	0;

						if ((theChar == ']') || (returnedData[data_iii + 1] == ','))
						{
//							CONSOLE_DEBUG_W_NUM("Skipping at \t=", data_iii);
							data_iii++;
						}
					}
					else
					if (theChar == ',')
					{
						commaCounter++;
						linebuf[ccc]	=	0;
//						if (strlen(linebuf) > 0)
//						{
//							CONSOLE_DEBUG_W_STR("linebuf     \t=", linebuf);
//						}
						if ((strlen(linebuf) > 3) && (linebuf[0] != 0x30))
						{
							JSON_ExtractKeyword_Value(linebuf, keywordStr, valueStr);
//							CONSOLE_DEBUG_W_2STR("KW:VAL\t=", keywordStr, valueStr);

							if (strcasecmp(keywordStr, "value") == 0)
							{
//								CONSOLE_DEBUG("'value' found!!!!");
								valueFoundFlag	=	true;
							}
						}
						ccc	=	0;
					}
					else if ((theChar == 0x0d) || (theChar == 0x0a))
					{
						linesProcessed++;
//						CONSOLE_DEBUG_W_NUM("linesProcessed\t=", linesProcessed);
					}
					else if (braceCnt > 0)
					{
						if (ccc < kReadBuffLen)
						{
							if ((theChar != '[') && (theChar != ']') && (theChar > 0x20))
							{
								linebuf[ccc++]	=	theChar;
								linebuf[ccc]	=	0;
							}
						}
						else
						{
							CONSOLE_DEBUG("Line to long");
//							CONSOLE_ABORT(__FUNCTION__);
						}
					}
				}
//				CONSOLE_DEBUG_W_NUM("dataIndex\t=", dataIndex);
		#endif
			}
			else
			{
				keepReading		=	false;
			}
		}
		shutDownRetCode	=	shutdown(socket_desc, SHUT_RDWR);
		if (shutDownRetCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("shutDownRetCode\t=", shutDownRetCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}

		closeRetCode	=	close(socket_desc);
		if (closeRetCode != 0)
		{
			CONSOLE_DEBUG("Close error");
		}
//		CONSOLE_ABORT(__FUNCTION__);
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
	}
//	CONSOLE_DEBUG_W_NUM("commaCounter\t=", commaCounter);

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");
	return(dataIndex);
}

//*****************************************************************************
void	Controller::ProcessConfiguredDevices(const char *keyword, const char *valueString)
{
	//*	this routine is meant to be over-ridden
}



//*****************************************************************************
//*	returns the number of data tokens
//*****************************************************************************
int	Controller::GetConfiguredDevices(void)
{
SJP_Parser_t	jsonParser;
bool			validData;
int				jjj;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	SJP_Init(&jsonParser);
	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										"/management/v1/configureddevices",
										NULL,
										&jsonParser);
	if (validData)
	{
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
			ProcessConfiguredDevices(jsonParser.dataList[jjj].keyword, jsonParser.dataList[jjj].valueString);
		}
	}
	else
	{
		cReadFailureCnt++;
		cOnLine	=	false;
		SetWindowIPaddrInfo(NULL, cOnLine);
	}
	return(jsonParser.tokenCount_Data);
}

//*****************************************************************************
bool	Controller::AlpacaCheckForDeviceState(void)
{
SJP_Parser_t	jsonParser;
char			alpacaString[128];
bool			validData;
bool			has_devicestate;

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("cAlpacaDeviceTypeStr\t=", cAlpacaDeviceTypeStr);
//	CONSOLE_DEBUG_W_NUM("cAlpacaDevNum        \t=", cAlpacaDevNum);
	has_devicestate	=	false;
	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/devicestate", cAlpacaDeviceTypeStr, cAlpacaDevNum);
//	CONSOLE_DEBUG(alpacaString);
	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
//		SJP_DumpJsonData(&jsonParser, __FUNCTION__);
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr, true);
		CONSOLE_DEBUG_W_NUM("devicestate returned", cLastAlpacaErrNum);
		if (cLastAlpacaErrNum == kASCOM_Err_Success)
		{
			cHas_DeviceState	=	true;
		}
	}
	CONSOLE_DEBUG_W_BOOL("cHas_DeviceState   \t=",	cHas_DeviceState);
	return(has_devicestate);
}

//*****************************************************************************
//*	returns true if data is received
//*****************************************************************************
bool	Controller::AlpacaGetStatus_DeviceState(	sockaddr_in	*deviceAddress,
													int			devicePort,
													const char	*deviceTypeStr,
													const int	deviceNum,
													const bool	enableDebug)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
bool			foundName;
bool			foundValue;
char			nameString[64];
char			valueString[128];
int				valuePairIdx;
int				keywordEnum;
bool			dataWasHandled;

//	CONSOLE_DEBUG(cWindowName);
//	CONSOLE_DEBUG_W_STR("Requesting 'DeviceState' for", deviceTypeStr);
//	CONSOLE_DEBUG_W_NUM("on port                     ", devicePort);
//	CONSOLE_DEBUG_W_BOOL("enableDebug                ", enableDebug);

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/devicestate", deviceTypeStr, deviceNum);
//	CONSOLE_DEBUG_W_STR("alpacaString\t=", alpacaString);

	validData	=	GetJsonResponse(	deviceAddress,
										devicePort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
		cDeviceStateReadCnt++;
		if (enableDebug)
		{
			SJP_DumpJsonData(&jsonParser, __FUNCTION__);
		}
		foundName		=	false;
		foundValue		=	false;
		valuePairIdx	=	0;
		cLastAlpacaErrNum	=	kASCOM_Err_Success;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
//			CONSOLE_DEBUG_W_STR(jsonParser.dataList[jjj].keyword, jsonParser.dataList[jjj].valueString);
			if (strncasecmp(jsonParser.dataList[jjj].keyword, "ARRAY", 5) == 0)
			{
				foundName	=	false;
				foundValue	=	false;
			}
			else if (strcasecmp(jsonParser.dataList[jjj].keyword, "NAME") == 0)
			{
				foundName	=	true;
				strcpy(nameString, jsonParser.dataList[jjj].valueString);
			}
			else if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				foundValue	=	true;
				strcpy(valueString, jsonParser.dataList[jjj].valueString);
			}
			if (foundName && foundValue)
			{
				//*	is the command table present
				if (cCommandEntryPtr != NULL)
				{
					keywordEnum	=	LookupCmdInCmdTable(jsonParser.dataList[jjj].keyword, cCommandEntryPtr);
					if (keywordEnum >= 0)
					{
						dataWasHandled	=	AlpacaProcessReadAllIdx(deviceTypeStr,
																	deviceNum,
																	keywordEnum,
																	jsonParser.dataList[jjj].valueString);
						if (dataWasHandled == false)
						{
							CONSOLE_DEBUG_W_STR("NOT HANDLED", jsonParser.dataList[jjj].keyword);
						}
					}
				}
				else
				{
	//				CONSOLE_DEBUG_W_STR(nameString, valueString);
					AlpacaProcessReadAll(	deviceTypeStr,
											deviceNum,
											nameString,
											valueString);

				}
				//*	this will allow the controller to update the DeviceState window if it wants to
				UpdateDeviceStateEntry(valuePairIdx, nameString, valueString);
				valuePairIdx++;

				foundName	=	false;
				foundValue	=	false;
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("GetJsonResponse failed")
	}
	return(validData);
}


//*****************************************************************************
bool	Controller::AlpacaGetStatus_DeviceState(const char *deviceTypeStr, const int deviceNum)
{
bool			validData;

//	CONSOLE_DEBUG(cWindowName);

	validData	=	AlpacaGetStatus_DeviceState(&cDeviceAddress,
												cPort,
												deviceTypeStr,
												deviceNum);
	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaGetStatus_DeviceState(void)
{
bool			validData;

//	CONSOLE_DEBUG(cWindowName);

	validData	=	AlpacaGetStatus_DeviceState(&cDeviceAddress,
												cPort,
												cAlpacaDeviceTypeStr,
												cAlpacaDevNum);
	return(validData);
}

//*****************************************************************************
void	Controller::UpdateDeviceStateEntry(const int index, const char *nameString, const char *valueString)
{
int			nameBoxNum;
int			valueBoxNum;
uint32_t	deltaTime_ms;
double		deltaTime_secs;
double		deltaTime_mins;
double		dsUpdatesPerSec;
double		dsUpdatesPerMin;
char		textBuf[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cDeviceStateTabNum > 0)
	{
		if ((cDeviceStateNameStart > 0) && (cDeviceStateValueStart > 0))
		{
			nameBoxNum	=   cDeviceStateNameStart + (index * 2);
			valueBoxNum	=   cDeviceStateValueStart + (index * 2);
			SetWidgetText(cDeviceStateTabNum,	nameBoxNum,		nameString);
			SetWidgetText(cDeviceStateTabNum,	valueBoxNum,	valueString);

			SetWidgetValid(	cDeviceStateTabNum,	nameBoxNum,		true);
			SetWidgetValid(	cDeviceStateTabNum,	valueBoxNum,	true);
		}
		if (cDeviceStateStats > 0)
		{
			deltaTime_ms	=	millis() - cContlerCreated_milliSecs;
			deltaTime_secs	=	deltaTime_ms / 1000.0;
			deltaTime_mins	=	deltaTime_secs / 60.0;
			if (deltaTime_mins > 2.0)
			{
				dsUpdatesPerMin	=	cDeviceStateReadCnt / deltaTime_mins;
				sprintf(textBuf, "DeviceState read Cnt=%d, rate=%2.1f / min", cDeviceStateReadCnt, dsUpdatesPerMin);
			}
			else if (deltaTime_secs > 2.0)
			{
				dsUpdatesPerSec	=	cDeviceStateReadCnt / deltaTime_secs;
				sprintf(textBuf, "DeviceState read Cnt=%d, rate=%2.1f / sec", cDeviceStateReadCnt, dsUpdatesPerSec);
			}
			else
			{
				sprintf(textBuf, "DeviceState read Cnt=%d", cDeviceStateReadCnt);

			}
			SetWidgetText(	cDeviceStateTabNum,	cDeviceStateStats, textBuf);
			SetWidgetValid(	cDeviceStateTabNum,	cDeviceStateStats,	true);
		}
	}
}

//*****************************************************************************
void	JSON_ExtractKeyword_Value(const char *linebuf, char *keywordStr, char *valueStr)
{
char	myLineBuf[kLineBufSize];
int		ccc;
int		sLen;
char	*colonPtr;

//	CONSOLE_DEBUG(__FUNCTION__);

	keywordStr[0]	=	0;
	valueStr[0]		=	0;

	sLen	=	strlen(linebuf);
	//*	get rid of leading spaces
	ccc	=	0;
	while (((linebuf[ccc] == 0x20) || (linebuf[ccc] == 0x09)) && (ccc < sLen))
	{
		ccc++;
	}
	strcpy(myLineBuf, &linebuf[ccc]);


	//*	separate out the keyword
	if (myLineBuf[0] == '"')
	{
		strcpy(keywordStr, &myLineBuf[1]);
	}
	else
	{
		strcpy(keywordStr, myLineBuf);
	}
	colonPtr	=	strchr(keywordStr, ':');
	if (colonPtr != NULL)
	{
		*colonPtr	=	0;
	}
	//*	check for trailing quote
	sLen	=	strlen(keywordStr);
	if (keywordStr[sLen - 1] == '"')
	{
		keywordStr[sLen - 1]	=	0;
	}
	//*	now go back to the original and look for the value
	colonPtr	=	strchr(myLineBuf, ':');
	if (colonPtr != NULL)
	{
		//*	we have a value string
		colonPtr++;	//*	skip the colon
		if (*colonPtr == '"')
		{
			colonPtr++;
		}
		strcpy(valueStr, colonPtr);

		//*	check for trailing coma
		sLen	=	strlen(valueStr);
		if (valueStr[sLen - 1] == ',')
		{
			valueStr[sLen - 1]	=	0;
		}

		//*	check for trailing quote
		sLen	=	strlen(valueStr);
		if (valueStr[sLen - 1] == '"')
		{
			valueStr[sLen - 1]	=	0;
		}
//		CONSOLE_DEBUG_W_2STR("kw:value=", keywordStr, valueStr);
	}
}


#endif // _CONTROLLER_USES_ALPACA_

