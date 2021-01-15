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
#include	"managementdriver.h"

#define	kMaxLen_DeviceType	24
#define	kMaxLen_Manuf		48
#define	kMaxLen_Version		32

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
	strcpy(cDeviceName, "ManagementDriver");

}

//**************************************************************************************
// Destructor
//**************************************************************************************
ManagementDriver::~ManagementDriver(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}



//*****************************************************************************
const TYPE_CmdEntry	gManagementCmdTable[]	=
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


//*****************************************************************************
TYPE_ASCOM_STATUS	ManagementDriver::Get_Apiversions(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS			alpacaErrCode	=	kASCOM_Err_Success;

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
//*	https://ascom-standards.org/api/?urls.primaryName=ASCOM%20Alpaca%20Management%20API#/Management%20Interface%20(JSON)/get_management_v1_description
//*****************************************************************************
TYPE_ASCOM_STATUS	ManagementDriver::Get_Configureddevices(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					ii;
AlpacaDriver		*devicePtr;
char				deviceTypeString[32];
char				uniqueIDstring[64];

	if (reqData != NULL)
	{
		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Version",
								gFullVersionString,
								INCLUDE_COMMA);

		JsonResponse_Add_ArrayStart(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"Value");
		JsonResponse_Add_RawText(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"\r\n");
		for (ii=0; ii<gDeviceCnt; ii++)
		{
			if (gAlpacaDeviceList[ii] != NULL)
			{
				devicePtr	=	gAlpacaDeviceList[ii];
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
										devicePtr->cDeviceName,
										INCLUDE_COMMA);

				JsonResponse_Add_Int32(reqData->socket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										"DeviceNumber",
										devicePtr->cDeviceNum,
										INCLUDE_COMMA);

				sprintf(uniqueIDstring, "%08X-%04X-%04X-%04X-%012X",
										devicePtr->cUniqueID.part1,
										devicePtr->cUniqueID.part2,
										devicePtr->cUniqueID.part3,
										devicePtr->cUniqueID.part4,
										devicePtr->cUniqueID.part5);

				JsonResponse_Add_String(reqData->socket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										"UniqueID",
									//	"277C652F-2AA9-4E86-A6A6-9230C42876FA",
									//	"11111111-2222-3333-4444-555555555555",
										uniqueIDstring,
										NO_COMMA);
				if (ii < (gDeviceCnt-1))
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
TYPE_ASCOM_STATUS	ManagementDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					cmdEnumValue;
int					cmdType;
char				alpacaErrMsg[256];
int					mySocket;


	CONSOLE_DEBUG_W_STR("htmlData\t=", reqData->htmlData);
	CONSOLE_DEBUG("------------------------------------------");

	strcpy(alpacaErrMsg, "");

	//*	make local copies of the data structure to make the code easier to read
	mySocket	=	reqData->socket;

	alpacaErrCode	=	kASCOM_Err_Success;


	//*	set up the json response
	JsonResponse_CreateHeader(reqData->jsonTextBuffer, kMaxJsonBuffLen);

	//*	this is not part of the protocol, I am using it for testing
	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Device",
								cDeviceName,
								INCLUDE_COMMA);

	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Command",
								reqData->deviceCommand,
								INCLUDE_COMMA);

//	CONSOLE_DEBUG_W_STR("reqData->deviceCommand\t=", reqData->deviceCommand);
//	CONSOLE_DEBUG_W_STR("reqData->cmdBuffer\t\t=", reqData->cmdBuffer);
//	CONSOLE_DEBUG_W_STR("reqData->contentData\t=", reqData->contentData);
	//*	look up the command
	cmdEnumValue	=	FindCmdFromTable(reqData->deviceCommand, gManagementCmdTable, &cmdType);
//	CONSOLE_DEBUG_W_NUM("cmdEnumValue\t\t=", cmdEnumValue);
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
							kMaxJsonBuffLen,
							kInclude_HTTP_Header);

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
//char		lineBuffer[128];

//	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		mySocketFD		=	reqData->socket;
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");

		SocketWriteData(mySocketFD,	"<H2>Management</H2>\r\n");


		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		//*	now generate links to all of the commands
		GenerateHTMLcmdLinkTable(mySocketFD, "management", 0, gManagementCmdTable);
	}
}

//*****************************************************************************
bool	ManagementDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gManagementCmdTable, getPut);
	return(foundIt);
}
