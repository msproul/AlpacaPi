//**************************************************************************
//*	Name:			switchdriver.cpp
//*
//*	Author:			Mark Sproul (C) 2019
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
//*	References:
//*	https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*	https://github.com/OpenPHDGuiding/phd2/tree/master/cameras/zwolibs
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Dec 26,	2019	<MLS> Created switchdriver.cpp
//*	Dec 26,	2019	<MLS> Switch driver basics working
//*	Dec 27,	2019	<MLS> Switch driver basics finished
//*	Dec 28,	2019	<MLS> Finished switchdescription.txt file processing
//*	Mar 20,	2020	<MLS> Added readall to switch driver
//*	Apr  2,	2020	<MLS> CONFORM-switch -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	Apr 14,	2020	<MLS> Added SetSwitchValue()
//*	Jun 23,	2021	<MLS> Updated Switchdriver cCommonProp.InterfaceVersion to 2
//*	Jun 23,	2021	<MLS> Added switch number range testing to several methods
//*	Jun 24,	2021	<MLS> CONFORM-switch -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	Jan  1,	2022	<MLS> Added GetSwitchValue()
//*	Jan  2,	2022	<MLS> Read-only switches did not pass CONFORM the first try
//*	Jan  2,	2022	<MLS> CONFORM-switch -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	Jul  3,	2022	<MLS> Started converting to using switch property structure
//*	Jun 19,	2023	<MLS> Added DeviceState_Add_Content() to switch driver
//*	Sep 12,	2023	<MLS> Changed routine names to CamelCase
//*	Sep 30,	2023	<MLS> Adding debugging code in verbose mode
//*	May 17,	2023	<MLS> Updated GetSwitchID() to generate http errors
//*	May 17,	2023	<MLS> CONFORUMU -> Found 0 errors, 3 issues and 3 information messages.
//*	May 17,	2024	<MLS> Started on http 400 error support for switch driver
//*	Jun 28,	2024	<MLS> Removed all "if (reqData != NULL)" from switchdriver.cpp
//*	Jul 20,	2024	<MLS> Updated set switch functions to return true if successful
//*****************************************************************************

#ifdef _ENABLE_SWITCH_

#include	<ctype.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<stdint.h>
#include	<unistd.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"JsonResponse.h"

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"switchdriver.h"

#include	"helper_functions.h"
#ifdef __arm__
	#include	"switchdriver_rpi.h"
#endif

#ifdef _ENABLE_SWITCH_SIMULATOR_
	#include	"switchdriver_sim.h"
#endif
#ifdef _ENABLE_SWITCH_STEPPER_
	#include	"switchdriver_stepper.h"
#endif

#include	"switch_AlpacaCmds.cpp"

//#define _DEBUG_CONFORM_

//**************************************************************************************
void	CreateSwitchObjects(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
#if defined(_ENABLE_SWITCH_RPI_) && defined(__arm__)
	CreateSwitchObjects_RPi();
#endif
#ifdef _ENABLE_SWITCH_STEPPER_
	CreateSwitchObjects_Stepper();
#endif

#ifdef _ENABLE_SWITCH_SIMULATOR_
	CreateSwitchObjects_SIM();
#endif
}

//**************************************************************************************
SwitchDriver::SwitchDriver(void)
	:AlpacaDriver(kDeviceType_Switch)
{
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name, 		"Switch");
	strcpy(cCommonProp.Description,	"Generic Switch");
	cCommonProp.InterfaceVersion	=	2;
	cDriverCmdTablePtr				=	gSwitchCmdTable;

	cUUID.part3						=	'SW';					//*	model number

	memset((void *)&cSwitchProp, 0, sizeof(TYPE_SwitchProperties));

	cSwitchProp.MaxSwitch			=	8;

	//*	zero out the switch description table
	for (iii=0; iii<kMaxSwitchCnt; iii++)
	{
		memset(&cSwitchTable[iii], 0, sizeof(TYPE_SwitchDescription));

		sprintf(cSwitchTable[iii].switchName, "Switch#%d", iii);
		strcpy(cSwitchTable[iii].switchDescription, "Not defined");

		cSwitchTable[iii].switchType	=	kSwitchType_Relay;
		cMinSwitchValue[iii]			=	0.0;
		cMaxSwitchValue[iii]			=	1.0;
		cCurSwitchValue[iii]			=	0.0;
	}
	ReadSwitchDataFile();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
SwitchDriver::~SwitchDriver(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				alpacaErrMsg[256];
int					cmdEnumValue;
int					cmdType;
int					mySocket;

//#ifdef _DEBUG_CONFORM_
	if (gVerbose)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
		DumpRequestStructure(__FUNCTION__, reqData);
	}
//#endif

	//*	make local copies of the data structure to make the code easier to read
	mySocket	=	reqData->socket;

	strcpy(alpacaErrMsg, "");
	alpacaErrCode	=	kASCOM_Err_Success;

	//*	set up the json response
	JsonResponse_CreateHeader(reqData->jsonTextBuffer);

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

	//*	look up the command
	cmdEnumValue	=	FindCmdFromTable(reqData->deviceCommand, gSwitchCmdTable, &cmdType);
	switch(cmdEnumValue)
	{
		//----------------------------------------------------------------------------------------
		//*	Common commands that we want to over ride
		//----------------------------------------------------------------------------------------
		case kCmd_Common_supportedactions:	//*	Returns the list of action names supported by this driver.
			alpacaErrCode	=	Get_SupportedActions(reqData, gSwitchCmdTable);
			break;

		//----------------------------------------------------------------------------------------
		//*	Device specific commands
		//----------------------------------------------------------------------------------------
		case kCmd_Switch_maxswitch:				//*	The number of switch devices managed by this driver
			alpacaErrCode	=	Get_Maxswitch(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Switch_canwrite:				//*	Indicates whether the specified switch device can be written to
			alpacaErrCode	=	Get_Canwrite(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Switch_getswitch:				//*	Return the state of switch device id as a boolean
			alpacaErrCode	=	Get_GetSwitch(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_getswitchdescription:	//*	Gets the description of the specified switch device
			alpacaErrCode	=	Get_GetSwitchDescription(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_getswitchname:			//*	Gets the name of the specified switch device
			alpacaErrCode	=	Get_GetSwitchName(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_getswitchvalue:		//*	Gets the value of the specified switch device as a double
			alpacaErrCode	=	Get_GetSwitchValue(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_minswitchvalue:		//*	Gets the minimum value of the specified switch device as a double
			alpacaErrCode	=	Get_MinSwitchValue(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_maxswitchvalue:		//*	Gets the maximum value of the specified switch device as a double
			alpacaErrCode	=	Get_MaxSwitchValue(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_setswitch:				//*	Sets a switch controller device to the specified state, true or false
			alpacaErrCode	=	Put_SetSwitch(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_setswitchname:			//*	Sets a switch device name to the specified value
			alpacaErrCode	=	Put_SetSwitchName(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_setswitchvalue:		//*	Sets a switch device value to the specified value
			alpacaErrCode	=	Put_SetSwitchValue(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_switchstep:			//*	Returns the step size that this device supports (the difference between successive values of the device).
			alpacaErrCode	=	Get_SwitchStep(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Switch_setswitchdescription:
//			alpacaErrCode	=	Put_SetSwitchDescription(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_readall:
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

	if (cSendJSONresponse)	//*	False for setupdialog and camera binary data
	{
		//*	send the response information
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Uint32(mySocket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"ClientTransactionID",
															reqData->ClientTransactionID,
															INCLUDE_COMMA);

		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Uint32(mySocket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"ServerTransactionID",
															gServerTransactionID,
															INCLUDE_COMMA);

		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Int32(	mySocket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"ErrorNumber",
															alpacaErrCode,
															INCLUDE_COMMA);

		cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(mySocket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"ErrorMessage",
															alpacaErrMsg,
															NO_COMMA);

		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Finish(	mySocket,
																reqData->httpRetCode,
																reqData->jsonTextBuffer,
																(cHttpHeaderSent == false));
	}
	//*	this is for the logging function
	strcpy(reqData->alpacaErrMsg, alpacaErrMsg);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	returns -1 if not found
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::GetSwitchID(TYPE_GetPutRequestData *reqData, int *idNum, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
bool				foundId;
char				idString[32];
int					switchNum;

	switchNum	=	-1;
	foundId		=	GetKeyWordArgument(	reqData->contentData,
										"Id",
										idString,
										sizeof(idString)-1);
	if (foundId == false)
	{
		foundId		=	GetKeyWordArgument(	reqData->contentData,
											"iD",
											idString,
											sizeof(idString)-1);
	}
	if (foundId)
	{
		if (IsValidNumericString(idString))
		{
			switchNum		=	atoi(idString);
			if ((switchNum >= 0) && (switchNum < cSwitchProp.MaxSwitch))
			{
				*idNum			=	switchNum;
				alpacaErrCode	=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Switch number out of range");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "ID is non-numeric");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "ID not found");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	The number of switch devices managed by this driver
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_Maxswitch(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cSwitchProp.MaxSwitch,
							INCLUDE_COMMA);

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Indicates whether the specified switch device can be written to
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_Canwrite(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InvalidValue;
int					switchNum;
bool				canWriteSwitch;
#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif

	alpacaErrCode	=	GetSwitchID(reqData, &switchNum, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		canWriteSwitch	=	true;
		if (cSwitchTable[switchNum].switchType == kSwitchType_Status)
		{
			canWriteSwitch	=	false;
		}
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								canWriteSwitch,
								INCLUDE_COMMA);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Return the state of switch device id as a boolean
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_GetSwitch(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					switchNum;
bool				switchState;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif

	alpacaErrCode	=	GetSwitchID(reqData, &switchNum, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		JsonResponse_Add_Int32(		reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"Switchnum",
									switchNum,
									INCLUDE_COMMA);

		switchState	=	GetSwitchState(switchNum);
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								gValueString,
								switchState,
								INCLUDE_COMMA);


		alpacaErrCode	=	kASCOM_Err_Success;
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Gets the description of the specified switch device
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_GetSwitchDescription(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("cSwitchProp.MaxSwitch\t=", cSwitchProp.MaxSwitch);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif

	alpacaErrCode	=	GetSwitchID(reqData, &switchNum, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									gValueString,
									cSwitchTable[switchNum].switchDescription,
									INCLUDE_COMMA);


		alpacaErrCode	=	kASCOM_Err_Success;
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Gets the name of the specified switch device
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_GetSwitchName(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif

	alpacaErrCode	=	GetSwitchID(reqData, &switchNum, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									gValueString,
									cSwitchTable[switchNum].switchName,
									INCLUDE_COMMA);


		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Gets the value of the specified switch device as a double
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_GetSwitchValue(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif

	alpacaErrCode	=	GetSwitchID(reqData, &switchNum, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								gValueString,
								cCurSwitchValue[switchNum],
								INCLUDE_COMMA);
		alpacaErrCode	=	kASCOM_Err_Success;
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Gets the minimum value of the specified switch device as a double
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_MinSwitchValue(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif

	alpacaErrCode	=	GetSwitchID(reqData, &switchNum, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								gValueString,
								cMinSwitchValue[switchNum],
								INCLUDE_COMMA);
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Gets the maximum value of the specified switch device as a double
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_MaxSwitchValue(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif

	alpacaErrCode	=	GetSwitchID(reqData, &switchNum, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								gValueString,
								cMaxSwitchValue[switchNum],
								INCLUDE_COMMA);
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	curl -X PUT "https://virtserver.swaggerhub.com/ASCOMInitiative/api/v1/switch/0/setswitch"
//*		-H  "accept: application/json"
//*		-H  "Content-Type: application/x-www-form-urlencoded"
//*		-d "Id=2&State=true&ClientID=5&ClientTransactionID=7"
//*****************************************************************************
//*	Sets a switch controller device to the specified state, true or false
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Put_SetSwitch(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				foundState;
char				stateString[32];
int					switchNum;
bool				newSwitchState;

	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif

	alpacaErrCode	=	GetSwitchID(reqData, &switchNum, alpacaErrMsg);
	foundState		=	GetKeyWordArgument(	reqData->contentData,
											"State",
											stateString,
											sizeof(stateString)-1);
	if ((alpacaErrCode == kASCOM_Err_Success) && foundState)
	{
		//*	make sure it is a switch and not a status
		if (cSwitchTable[switchNum].switchType != kSwitchType_Status)
		{
			if (IsValidTrueFalseString(stateString))
			{
				newSwitchState	=	IsTrueFalse(stateString);
				SetSwitchState(switchNum, newSwitchState);

				CONSOLE_DEBUG_W_STR(	"stateString   \t=", stateString);
				CONSOLE_DEBUG_W_BOOL(	"newSwitchState\t=", newSwitchState);

				if (newSwitchState)
				{
					cCurSwitchValue[switchNum]	=	cMaxSwitchValue[switchNum];
				}
				else
				{
					cCurSwitchValue[switchNum]	=	cMinSwitchValue[switchNum];
				}
				alpacaErrCode	=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "State is not boolean");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			CONSOLE_DEBUG("Trying to write to a read-only switch");
			alpacaErrCode			=	kASCOM_Err_MethodNotImplemented;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "CanWrite is false");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "parameters missing");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Sets a switch device name to the specified value
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Put_SetSwitchName(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				foundName;
char				nameString[kMaxSwitchNameLen];
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif

	alpacaErrCode	=	GetSwitchID(reqData, &switchNum, alpacaErrMsg);
	foundName		=	GetKeyWordArgument(	reqData->contentData,
											"Name",
											nameString,
											(kMaxSwitchNameLen - 1));
	if ((alpacaErrCode == kASCOM_Err_Success) && foundName)
	{
		strcpy(cSwitchTable[switchNum].switchName, nameString);

		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									gValueString,
									cSwitchTable[switchNum].switchName,
									INCLUDE_COMMA);

		WriteSwitchDataFile();
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "parameters missing");
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Sets a switch device value to the specified value as a double
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Put_SetSwitchValue(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
bool				foundValue;
int					switchNum;
char				valueString[32];
double				newSwitchValue;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif

	alpacaErrCode	=	GetSwitchID(reqData, &switchNum, alpacaErrMsg);
	foundValue		=	GetKeyWordArgument(	reqData->contentData,
											"Value",
											valueString,
											(sizeof(valueString) - 1));
	if ((alpacaErrCode == kASCOM_Err_Success) && foundValue)
	{
		//*	make sure it is a switch and not a status
		if (cSwitchTable[switchNum].switchType != kSwitchType_Status)
		{
			if (IsValidNumericString(valueString))
			{
				newSwitchValue				=	AsciiToDouble(valueString);
				if ((newSwitchValue >= cMinSwitchValue[switchNum]) &&
					(newSwitchValue <= cMaxSwitchValue[switchNum]))
				{
					SetSwitchValue(switchNum, newSwitchValue);
					cCurSwitchValue[switchNum]	=	AsciiToDouble(valueString);
					alpacaErrCode				=	kASCOM_Err_Success;
				}
				else
				{
					alpacaErrCode			=	kASCOM_Err_InvalidValue;
					reqData->httpRetCode	=	400;
					GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Value out of range");
					CONSOLE_DEBUG(alpacaErrMsg);
				}
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_MethodNotImplemented;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Value is non-numeric");
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_MethodNotImplemented;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "CanWrite is false");
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		if (foundValue == false)
		{
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Value is missing");
		}
		CONSOLE_DEBUG(alpacaErrMsg);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Returns the step size that this device supports (the difference between successive values of the device).
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_SwitchStep(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					switchNum;

	alpacaErrCode	=	GetSwitchID(reqData, &switchNum, alpacaErrMsg);
	if (alpacaErrCode == kASCOM_Err_Success)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								1.0,
								INCLUDE_COMMA);
	}
//	DumpRequestStructure(__FUNCTION__, reqData);
	return(alpacaErrCode);
}

//*****************************************************************************
//GetSwitch0 (Assumes that the number of available
//GetSwitch1 switches (N) has already been determined by
//GetSwitch2 the application)
//...
//GetSwitchN
//GetSwitchValue0
//GetSwitchValue1
//GetSwitchValue2
//...
//GetSwitchValueN
//*****************************************************************************
bool	SwitchDriver::DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen)
{
int		iii;
char	switchNameStr[48];
bool	switchState;

	for (iii=0; iii<cSwitchProp.MaxSwitch; iii++)
	{
		sprintf(switchNameStr,	"GetSwitch%d", iii);
		switchState	=	GetSwitchState(iii);
		DeviceState_Add_Bool(socketFD,	jsonTextBuffer, maxLen,	switchNameStr,	switchState);
	}

	for (iii=0; iii<cSwitchProp.MaxSwitch; iii++)
	{
		sprintf(switchNameStr,	"GetSwitchValue%d", iii);
		DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	switchNameStr,	cCurSwitchValue[iii]);
	}

	return(true);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_Readall(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					mySocket;
char				textBuffer[128];
int					iii;
bool				switchState;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif

	//*	do the common ones first
	Get_Readall_Common(	reqData, alpacaErrMsg);

	//*	make local copies of the data structure to make the code easier to read
	mySocket	=	reqData->socket;

	Get_Maxswitch(	reqData, alpacaErrMsg, "maxswitch");	//*	The number of switch devices managed by this driver

	for (iii=0; iii<cSwitchProp.MaxSwitch; iii++)
	{
		GetSwitchValue(iii);

		sprintf(textBuffer, "getswitchname-%d", iii);
		JsonResponse_Add_String(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									textBuffer,
									cSwitchTable[iii].switchName,
									INCLUDE_COMMA);

		sprintf(textBuffer, "getswitchdescription-%d", iii);
		JsonResponse_Add_String(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									textBuffer,
									cSwitchTable[iii].switchDescription,
									INCLUDE_COMMA);

		switchState	=	GetSwitchState(iii);
		sprintf(textBuffer, "getswitch-%d", iii);
		JsonResponse_Add_Bool(		mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									textBuffer,
									switchState,
									INCLUDE_COMMA);

		sprintf(textBuffer, "minswitchvalue-%d", iii);
		JsonResponse_Add_Double(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									textBuffer,
									cMinSwitchValue[iii],
									INCLUDE_COMMA);

		sprintf(textBuffer, "maxswitchvalue-%d", iii);
		JsonResponse_Add_Double(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									textBuffer,
									cMaxSwitchValue[iii],
									INCLUDE_COMMA);


//		if (cSwitchType[iii] == kSwitchType_Analog)
		{
			sprintf(textBuffer, "getswitchvalue-%d", iii);
			JsonResponse_Add_Double(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									textBuffer,
									cCurSwitchValue[iii],
									INCLUDE_COMMA);
		}
	}

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
	return(alpacaErrCode);
}


//*****************************************************************************
void	SwitchDriver::OutputHTML(TYPE_GetPutRequestData *reqData)
{
int		mySocketFD;
int		ii;
bool	mySwitchState;
char	lineBuffer[32];

//	CONSOLE_DEBUG(__FUNCTION__);
	mySocketFD	=	reqData->socket;

	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H2>AlpacaPi Switch</H2>\r\n");
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");


	SocketWriteData(mySocketFD,	"<TR>\r\n\t<TH>");
	SocketWriteData(mySocketFD,	"#");
	SocketWriteData(mySocketFD,	"</TH>\r\n\t<TH>");
	SocketWriteData(mySocketFD,	"Switch");
	SocketWriteData(mySocketFD,	"</TH>\r\n\t<TH>");
	SocketWriteData(mySocketFD,	"Description");
	SocketWriteData(mySocketFD,	"</TH>\r\n\t<TH>");
	SocketWriteData(mySocketFD,	"State");
	SocketWriteData(mySocketFD,	"</TH>\r\n</TR>\r\n");

	for (ii=0; ii<cSwitchProp.MaxSwitch; ii++)
	{

		SocketWriteData(mySocketFD,	"<TR>\r\n\t<TD>");
		sprintf(lineBuffer, "%d", ii);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</TD>\r\n\t<TD>");
		SocketWriteData(mySocketFD,	cSwitchTable[ii].switchName);
		SocketWriteData(mySocketFD,	"</TD>\r\n\t<TD>");
		SocketWriteData(mySocketFD,	cSwitchTable[ii].switchDescription);
		SocketWriteData(mySocketFD,	"</TD>\r\n\t<TD>");
		mySwitchState	=	GetSwitchState(ii);
		if (mySwitchState)
		{
			strcpy(lineBuffer, "ON");
		}
		else
		{
			strcpy(lineBuffer, "OFF");
		}

		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"</TD>\r\n</TR>\r\n");
	}
	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<P>\r\n");
}

//*****************************************************************************
bool	SwitchDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gSwitchCmdTable, getPut);
	return(foundIt);
}

//*****************************************************************************
bool	SwitchDriver::ConfigureSwitch(	const int	switchNumber,
										const int	switchType,
										const int	hardWarePinNumber,
										const int	trueValue)		//*	default is 1, for inverted logic pass 0
{
bool	successFlag	=	false;

	if (switchNumber < kMaxSwitchCnt)
	{
		successFlag	=	true;
		cSwitchTable[switchNumber].switchType	=	switchType;
		cSwitchTable[switchNumber].hwPinNumber	=	hardWarePinNumber;
		cSwitchTable[switchNumber].valueForTrue	=	trueValue;

		if (switchNumber >= cSwitchProp.MaxSwitch)
		{
			cSwitchProp.MaxSwitch	=	switchNumber + 1;
		}
//		CONSOLE_DEBUG_W_NUM("cSwitchProp.MaxSwitch\t=", cSwitchProp.MaxSwitch);
	}
	return(successFlag);
}

#define	kSwitchDataFileName	"switchdescription.txt"
//**************************************************************************
void	SwitchDriver::ReadSwitchDataFile(void)
{
int		ii;
int		jj;
int		cc;
FILE	*filePointer;
char	lineBuff[256];
char	argBuf[256];
int		switchNum;
int		slen;
int		argNum;

	CONSOLE_DEBUG(__FUNCTION__);

	filePointer	=	fopen(kSwitchDataFileName, "r");
	if (filePointer != NULL)
	{
		while (fgets(lineBuff, 200, filePointer))
		{
			//*	get rid of the trailing CR/LF
			slen	=	strlen(lineBuff);
			for (ii=0; ii<slen; ii++)
			{
				if ((lineBuff[ii] == 0x0d) || (lineBuff[ii] == 0x0a))
				{
					lineBuff[ii]	=	0;
					break;
				}
			}
//			CONSOLE_DEBUG(lineBuff);
			slen	=	strlen(lineBuff);
			if ((slen > 3) && (lineBuff[0] != '#'))
			{
				if (isdigit(lineBuff[0]))
				{
					switchNum	=	atoi(lineBuff);
//					CONSOLE_DEBUG_W_NUM("switchNum\t=",	switchNum);
					if ((switchNum >= 0) && (switchNum < kMaxSwitchCnt))
					{
						//*	extract the parameters
						argNum	=	0;
						jj		=	1;
						cc		=	0;
						while ((jj <= slen) && (cc <=slen)  && (argNum <=3))
						{
							if (lineBuff[jj] < 0x20)
							{
								argBuf[cc]	=	0;

								switch(argNum)
								{
									case 0:
										//*	do nothing, this is the number
										break;

									case 1:
										//*	this is the switch name
										strcpy(cSwitchTable[switchNum].switchName, argBuf);
										break;

									case 2:
										//*	this is the switch description
										strcpy(cSwitchTable[switchNum].switchDescription, argBuf);
										break;

								}
								argNum++;
								cc	=	0;
							}
							else
							{
								argBuf[cc]	=	lineBuff[jj];
								cc++;
							}
							jj++;
						}

					}
				}
				else
				{
					CONSOLE_DEBUG("1st char is not a digit");
				}
			}
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG("Failed to read switch data file");
	}
}

//**************************************************************************
void	SwitchDriver::WriteSwitchDataFile(void)
{
int		ii;
FILE	*filePointer;

	filePointer	=	fopen(kSwitchDataFileName, "w");
	if (filePointer != NULL)
	{
		fprintf(filePointer, "#########################################\r\n");
		fprintf(filePointer, "# File generated by Alpaca switch driver\r\n");
		fprintf(filePointer, "# It will get overwritten each time a setswitchname or set description\r\n");
		fprintf(filePointer, "# You can edit this file\r\n");
		fprintf(filePointer, "# One tab (0x09) between each field\r\n");
		fprintf(filePointer, "#########################################\r\n");
		for (ii=0; ii<cSwitchProp.MaxSwitch; ii++)
		{
			fprintf(filePointer, "%d\t",	ii);
			fprintf(filePointer, "%s\t",	cSwitchTable[ii].switchName);
			fprintf(filePointer, "%s\t",	cSwitchTable[ii].switchDescription);
			fprintf(filePointer, "\r\n");
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create switch data file");
	}
}

//*****************************************************************************
bool	SwitchDriver::GetSwitchState(const int switchNumber)
{
	//*	this function meant to be overloaded
	return(false);
}

//*****************************************************************************
//*	returns true if successful
//*****************************************************************************
bool	SwitchDriver::SetSwitchState(const int switchNumber, bool on_off)
{
	//*	this function meant to be overloaded
	return(false);
}

//*****************************************************************************
//*	returns true if successful
//*****************************************************************************
bool	SwitchDriver::SetSwitchValue(const int switchNumber, double switchValue)
{
	//*	this function meant to be overloaded
	return(false);
}

//*****************************************************************************
double	SwitchDriver::GetSwitchValue(const int switchNumber)
{
double	switchValue;

//	CONSOLE_DEBUG(__FUNCTION__);
	switchValue	=	0.0;
	if ((switchNumber >= 0) && (switchNumber < cSwitchProp.MaxSwitch))
	{
		switch(cSwitchTable[switchNumber].switchType)
		{
			case kSwitchType_Relay:
			case kSwitchType_Status:
				switchValue	=	GetSwitchState(switchNumber);
				cCurSwitchValue[switchNumber]	=	switchValue;
				break;

			case kSwitchType_Analog:
				break;

		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Switch number out of bounds:", switchNumber);
	}
	return(switchValue);
}

#endif	//*	_ENABLE_SWITCH_

