//**************************************************************************
//*	Name:			managementdriver.cpp
//*
//*	Author:			Mark Sproul (C) 2019-2020
//*
//*	Description:	C++ Driver for Alpaca protocol
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
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Nov 20,	2019	<MLS> Created managementdriver.cpp
//*	Nov 21,	2019	<MLS> management driver working
//*	Nov 25,	2019	<MLS> Added error msgs to unfinished commands
//*	Feb  1,	2020	<MLS> Added version string to configured devices response
//*	Mar 18,	2020	<MLS> Finished Get_Apiversions() and Get_Description()
//*	Apr 27,	2020	<MLS> Added cpustats and readall to management driver commands
//*	May  4,	2020	<MLS> Added library versions to keep track of different machines
//*	May  4,	2020	<MLS> Added AddLibraryVersion() & Get_Libraries()
//*	Apr  8,	2022	<MLS> Removed double quotes in version string, JSON doesn't like it
//*	Jun  2,	2022	<MLS> Added cpu temp and uptime to configureddevices response
//*	Oct 16,	2022	<MLS> Management driver stores CPU temp in temperature log
//*	Nov 27,	2022	<MLS> Added ReportOneDevice() to clean up configureddevices code
//*	Nov 30,	2022	<MLS> Peter Simpson relaxed conformu to allow non-standard devices
//*	Dec 22,	2022	<MLS> Added timestamp to configured devices output
//*****************************************************************************


#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"RequestData.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"JsonResponse.h"
#include	"eventlogging.h"
#include	"cpu_stats.h"
#include	"helper_functions.h"

#include	"managementdriver.h"

#define	kMaxLen_DeviceType	24
#define	kMaxLen_Manuf		48
#define	kMaxLen_Version		32


//*****************************************************************************
static TYPE_CmdEntry	gManagementCmdTable[]	=
{
	{	"apiversions",			kCmd_Managment_apiversions,			kCmdType_GET	},
	{	"description",			kCmd_Managment_description,			kCmdType_GET	},
	{	"configureddevices",	kCmd_Managment_configureddevices,	kCmdType_GET	},

	//*	items added by MLS
	{	"--extras",				kCmd_Managment_Extras,				kCmdType_GET	},
	{	"cpustats",				kCmd_Managment_cpustats,			kCmdType_GET	},
	{	"libraries",			kCmd_Managment_libraries,			kCmdType_GET	},
	{	"readall",				kCmd_Managment_readall,				kCmdType_GET	},

	{	"",						-1,	0x00	}
};

//**************************************************************************************
typedef struct
{
	char	device[kMaxLen_DeviceType];
	char	manuf[kMaxLen_Manuf];
	char	versionString[kMaxLen_Version];

} TYPE_LIBRARY_VERSION;

#define	kMaxLibraries	20
TYPE_LIBRARY_VERSION	gLibraryVersions[kMaxLibraries];
int						gLibraryIndex	=	-1;

//**************************************************************************************
void	AddLibraryVersion(const char *device, const char *manuf, const char *versionString)
{
int		iii;
int		sLen;
	if (gLibraryIndex < 0)
	{
		for (iii=0; iii<kMaxLibraries; iii++)
		{
			memset(&gLibraryVersions[iii], 0, sizeof(TYPE_LIBRARY_VERSION));
		}
		gLibraryIndex	=	0;
	}

	if (gLibraryIndex < kMaxLibraries)
	{
		strcpy(gLibraryVersions[gLibraryIndex].device,			device);
		strcpy(gLibraryVersions[gLibraryIndex].manuf,			manuf);
		strcpy(gLibraryVersions[gLibraryIndex].versionString,	versionString);
		//*	make sure there are no double quotes in the string
		sLen	=	strlen(gLibraryVersions[gLibraryIndex].versionString);
		//*	Apr  8,	2022	<MLS> Removed double quotes in version string, JSON doesn't like it
		for (iii = 0; iii < sLen; iii++)
		{
			if (gLibraryVersions[gLibraryIndex].versionString[iii] == '"')
			{
				gLibraryVersions[gLibraryIndex].versionString[iii]	=	'\'';
			}
		}
		gLibraryIndex++;
	}
}

//**************************************************************************************
//*	this will get moved to the individual implentations later
void	CreateManagementObject(void)
{
	new ManagementDriver(0);
}

//**************************************************************************************
ManagementDriver::ManagementDriver(const int argDevNum)
	:AlpacaDriver(kDeviceType_Management)
{
	cDriverCmdTablePtr				=	gManagementCmdTable;
	strcpy(cCommonProp.Name,		"ManagementDriver");
	strcpy(cCommonProp.Description,	"AlpacaPi Management driver");

	TemperatureLog_SetDescription("CPU Temperature");
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ManagementDriver::~ManagementDriver(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
//*	return value 0 = OK
//*****************************************************************************
int	ManagementDriver::UpdateProperties(void)
{
int			returnCode;
double		cpuTemp_DegC;
double		cpuTemp_DegF;

	returnCode	=	0;

	cpuTemp_DegC	=	CPUstats_GetTemperature(NULL);
	cpuTemp_DegF	=	((cpuTemp_DegC * (9.0/5.0)) + 32);

	TemperatureLog_AddEntry(cpuTemp_DegF);


	return(returnCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ManagementDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					cmdEnumValue;
int					cmdType;
char				alpacaErrMsg[256];
int					mySocket;


//	CONSOLE_DEBUG("------------------------------------------");
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("htmlData\t=", reqData->htmlData);
//	DumpRequestStructure(__FUNCTION__, reqData);

	strcpy(alpacaErrMsg, "");

	//*	make local copies of the data structure to make the code easier to read
	mySocket		=	reqData->socket;
	alpacaErrCode	=	kASCOM_Err_Success;

	//*	set up the json response
	JsonResponse_CreateHeader(reqData->jsonTextBuffer);

	if (reqData->clientIs_ConformU == false)
	{
		//*	this is not part of the protocol, I am using it for testing
		JsonResponse_Add_String(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"Device",
									cCommonProp.Name,
									INCLUDE_COMMA);

		JsonResponse_Add_String(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"Command",
									reqData->deviceCommand,
									INCLUDE_COMMA);

	}
	//*	look up the command
	cmdEnumValue	=	FindCmdFromTable(reqData->deviceCommand, gManagementCmdTable, &cmdType);

//	CONSOLE_DEBUG_W_NUM("cmdEnumValue\t\t=", cmdEnumValue);
//	if (cmdEnumValue == kCmd_Managment_configureddevices)
//	{
//		CONSOLE_DEBUG_W_STR("reqData->htmlData     \t=",	reqData->htmlData);
//		CONSOLE_DEBUG_W_STR("reqData->deviceCommand\t=",	reqData->deviceCommand);
//		CONSOLE_DEBUG_W_STR("reqData->cmdBuffer    \t=",	reqData->cmdBuffer);
//		CONSOLE_DEBUG_W_STR("reqData->contentData  \t=",	reqData->contentData);
//	}

	switch(cmdEnumValue)
	{
		//----------------------------------------------------------------------------------------
		//*	Common commands that we want to over ride
		//----------------------------------------------------------------------------------------
		case kCmd_Common_supportedactions:	//*	Returns the list of action names supported by this driver.
			alpacaErrCode	=	Get_SupportedActions(reqData, gManagementCmdTable);
			break;

		//----------------------------------------------------------------------------------------
		//*	ManagementDriver specific commands
		//----------------------------------------------------------------------------------------
		case kCmd_Managment_apiversions:
			alpacaErrCode	=	Get_Apiversions(reqData, alpacaErrMsg);
			break;

		case kCmd_Managment_description:
			alpacaErrCode	=	Get_Description(reqData, alpacaErrMsg);
			break;

		case kCmd_Managment_configureddevices:
			alpacaErrCode	=	Get_Configureddevices(reqData, alpacaErrMsg);
			break;

		case kCmd_Managment_cpustats:
			alpacaErrCode	=	Get_Readall_CPUstats(reqData, alpacaErrMsg);
			break;

		case kCmd_Managment_libraries:
			alpacaErrCode	=	Get_Libraries(reqData, alpacaErrMsg);
			break;

		case kCmd_Managment_readall:
			alpacaErrCode	=	Get_Readall(reqData, alpacaErrMsg);
			break;



		//----------------------------------------------------------------------------------------
		//*	let anything undefined go to the common command processor
		//----------------------------------------------------------------------------------------
		default:
			alpacaErrCode	=	ProcessCommand_Common(reqData, cmdEnumValue, alpacaErrMsg);
			break;
	}
	RecordCmdStats(cmdEnumValue, reqData->get_putIndicator, alpacaErrCode);

	if (alpacaErrCode != kASCOM_Err_Success)
	{
		CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=",	reqData->alpacaErrCode);
	}

	//*	send the response information
	JsonResponse_Add_Int32(	mySocket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"ClientTransactionID",
							gClientTransactionID,
							INCLUDE_COMMA);

	JsonResponse_Add_Int32(	mySocket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"ServerTransactionID",
							gServerTransactionID,
							INCLUDE_COMMA);

	JsonResponse_Add_Int32(	mySocket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"ErrorNumber",
							alpacaErrCode,
							INCLUDE_COMMA);

	JsonResponse_Add_String(mySocket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"ErrorMessage",
							alpacaErrMsg,
							NO_COMMA);

	JsonResponse_Add_Finish(mySocket,
							reqData->jsonTextBuffer,
							(cHttpHeaderSent == false));

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	ManagementDriver::Get_Apiversions(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS			alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);

	JsonResponse_Add_ArrayStart(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Value");


	JsonResponse_Add_RawText(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"1");

	JsonResponse_Add_ArrayEnd(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
//*
//*		{
//*		  "Value": {
//*		    "ServerName": "Random Alpaca Device",
//*		    "Manufacturer": "The Briliant Company",
//*		    "ManufacturerVersion": "v1.0.0",
//*		    "Location": "Horsham, UK"
//*		  },
//*		  "ClientTransactionID": 9876,
//*		  "ServerTransactionID": 54321
//*		}
//*****************************************************************************
TYPE_ASCOM_STATUS	ManagementDriver::Get_Description(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS				alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);

	JsonResponse_Add_RawText(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"\t\t\"Value\":\r\n\t\t{\r\n");

	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"ServerName",
							"Alpaca driver by Mark Sproul",
							INCLUDE_COMMA);

	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"Manufacturer",
							"msproul@skychariot.com",
							INCLUDE_COMMA);

	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"ManufacturerVersion",
							kVersionString,
							INCLUDE_COMMA);

	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"Location",
							"Pennsylvania, USA",
							NO_COMMA);

	JsonResponse_Add_RawText(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"\t\t},\r\n");

	return(alpacaErrCode);
}

//*****************************************************************************
void	ManagementDriver::ReportOneDevice(TYPE_GetPutRequestData *reqData, AlpacaDriver *devicePtr, bool includeComma)
{
char		deviceTypeString[32];
char		uniqueIDstring[64];

//	CONSOLE_DEBUG(__FUNCTION__);
	GetDeviceTypeFromEnum(devicePtr->cDeviceType, deviceTypeString);

	JsonResponse_Add_RawText(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"\t\t{\r\n");

	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"DeviceType",
							deviceTypeString,
							INCLUDE_COMMA);

	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"DeviceName",
							devicePtr->cCommonProp.Name,
							INCLUDE_COMMA);

	JsonResponse_Add_Int32(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"DeviceNumber",
							devicePtr->cAlpacaDeviceNum,
							INCLUDE_COMMA);

	sprintf(uniqueIDstring, "%08X-%04X-%04X-%04X-%012X",
							devicePtr->cUUID.part1,
							devicePtr->cUUID.part2,
							devicePtr->cUUID.part3,
							devicePtr->cUUID.part4,
							devicePtr->cUUID.part5);

	JsonResponse_Add_String(reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"UniqueID",
						//	"277C652F-2AA9-4E86-A6A6-9230C42876FA",
						//	"11111111-2222-3333-4444-555555555555",
							uniqueIDstring,
							NO_COMMA);
//	if (ii < (gDeviceCnt-2))
	if (includeComma)
	{
		JsonResponse_Add_RawText(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"\t\t},\r\n");
	}
	else
	{
		JsonResponse_Add_RawText(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"\t\t}\r\n");
	}

}

//*****************************************************************************
//*	https://ascom-standards.org/api/?urls.primaryName=ASCOM%20Alpaca%20Management%20API#/Management%20Interface%20(JSON)/get_management_v1_description
//*****************************************************************************
TYPE_ASCOM_STATUS	ManagementDriver::Get_Configureddevices(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					iii;
AlpacaDriver		*devicePtr;
double				cpuTemp_DegC;
double				cpuTemp_DegF;
uint32_t			upTime;
int					upTime_Days;
bool				includeCommaFlag;
int					standardDeviceCnt;
bool				displayNonStandardDevices;
int					displayedCnt;
struct timeval		timeStamp;
char				timeStampString[128];

//	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Version",
								gFullVersionString,
								INCLUDE_COMMA);

		//====================================================
		//*	add a time stamp
		gettimeofday(&timeStamp, NULL);
		FormatDateTimeString_Local(&timeStamp, timeStampString);
		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"TimeStamp",
								timeStampString,
								INCLUDE_COMMA);

		//====================================================
		upTime		=	CPUstats_GetUptime();
		upTime_Days	=	upTime / (24 * 60 * 60);

		JsonResponse_Add_Int32(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"upTime_Days",
								upTime_Days,
								INCLUDE_COMMA);


		//====================================================
		cpuTemp_DegC	=	CPUstats_GetTemperature(NULL);
		cpuTemp_DegF	=	((cpuTemp_DegC * (9.0/5.0)) + 32);
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"cpuTemp_DegF",
								cpuTemp_DegF,
								INCLUDE_COMMA);

		JsonResponse_Add_ArrayStart(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"Value");
		JsonResponse_Add_RawText(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"\r\n");

		//----------------------------------------------------------------------------
		//*	to keep the new CONFORMU happy, we have to ONLY report standard devices
		//*	calculate the the standard device count.
		displayNonStandardDevices	=	true;
//		if (reqData->clientIs_ConformU)
//		{
//			displayNonStandardDevices	=	false;
//		}

		//*	determine the number of standard devices.
		//*	the TOTAL number of devices is gDeviceCnt
		standardDeviceCnt	=	0;
		for (iii=0; iii<gDeviceCnt; iii++)
		{
			if (gAlpacaDeviceList[iii] != NULL)
			{
				devicePtr	=	gAlpacaDeviceList[iii];
				switch(devicePtr->cDeviceType)
				{
					case kDeviceType_Camera:
					case kDeviceType_CoverCalibrator:
					case kDeviceType_Dome:
					case kDeviceType_Filterwheel:
					case kDeviceType_Focuser:
					case kDeviceType_Observingconditions:
					case kDeviceType_Rotator:
					case kDeviceType_SafetyMonitor:
					case kDeviceType_Switch:
					case kDeviceType_Telescope:
						standardDeviceCnt++;
						break;

					default:
						break;
				}
			}
		}

		displayedCnt		=	0;
		for (iii=0; iii<gDeviceCnt; iii++)
		{
			if (gAlpacaDeviceList[iii] != NULL)
			{
				//*	figure out if we need a comma or not
				if (displayNonStandardDevices)
				{
					includeCommaFlag	=	(displayedCnt < (gDeviceCnt - 1));
				}
				else
				{
					includeCommaFlag	=	(displayedCnt < (standardDeviceCnt - 1));
				}
				//*	get the pointer to the device object
				devicePtr	=	gAlpacaDeviceList[iii];
				switch (devicePtr->cDeviceType)
				{
					case kDeviceType_Camera:
					case kDeviceType_CoverCalibrator:
					case kDeviceType_Dome:
					case kDeviceType_Filterwheel:
					case kDeviceType_Focuser:
					case kDeviceType_Observingconditions:
					case kDeviceType_Rotator:
					case kDeviceType_SafetyMonitor:
					case kDeviceType_Switch:
					case kDeviceType_Telescope:
						ReportOneDevice(reqData, devicePtr, includeCommaFlag);
						displayedCnt++;
						break;

					case kDeviceType_Management:
					default:
						//*	these are NOT to be included when talking to ConformU
						//*	this got fixed in ConformU on Nov 30, 2022
						if (displayNonStandardDevices)
						{
							ReportOneDevice(reqData, devicePtr, includeCommaFlag);
							displayedCnt++;
						}
						break;
				}
			}
		}

		JsonResponse_Add_ArrayEnd(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ManagementDriver::Get_Libraries(TYPE_GetPutRequestData	*reqData,
										char					*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					iii;
char				lineBuff1[32];
char				lineBuff2[128];

	CONSOLE_DEBUG(__FUNCTION__);

	for (iii=0; iii<gLibraryIndex; iii++)
	{
		sprintf(lineBuff1, "library-%d", (iii + 1));
		sprintf(lineBuff2, "%s-%s-%s",	gLibraryVersions[iii].device,
										gLibraryVersions[iii].manuf,
										gLibraryVersions[iii].versionString);

		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									lineBuff1,
									lineBuff2,
									INCLUDE_COMMA);
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	ManagementDriver::Get_Readall(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					mySocket;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		//*	do the common ones first
		Get_Readall_Common(	reqData, alpacaErrMsg);

		//*	make local copies of the data structure to make the code easier to read
		mySocket	=	reqData->socket;


		//===============================================================
		JsonResponse_Add_String(mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Comment",
								"Non-standard alpaca commands follow",
								INCLUDE_COMMA);

		JsonResponse_Add_String(mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"version",
								gFullVersionString,
								INCLUDE_COMMA);





		alpacaErrCode	=	kASCOM_Err_Success;
		strcpy(alpacaErrMsg, "");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

#pragma mark -

//*****************************************************************************
void	ManagementDriver::OutputHTML(TYPE_GetPutRequestData *reqData)
{
int			mySocketFD;

	if (reqData != NULL)
	{
		mySocketFD		=	reqData->socket;
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<H2>Management</H2>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	}
}

//*****************************************************************************
bool	ManagementDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gManagementCmdTable, getPut);
	return(foundIt);
}

//*****************************************************************************
void	ManagementDriver::WatchDog_TimeOut(void)
{
	//*	easiest way to make sure management driver doesnt do anything
}
