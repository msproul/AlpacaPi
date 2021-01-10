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

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"


#include	"JsonResponse.h"
#include	"switchdriver.h"

//#define _DEBUG_CONFORM_

//*****************************************************************************
const TYPE_CmdEntry	gSwitchCmdTable[]	=
{

	{	"maxswitch",			kCmd_Switch_maxswitch,				kCmdType_GET	},
	{	"canwrite",				kCmd_Switch_canwrite,				kCmdType_GET	},
	{	"getswitch",			kCmd_Switch_getswitch,				kCmdType_GET	},
	{	"getswitchdescription",	kCmd_Switch_getswitchdescription,	kCmdType_GET	},
	{	"getswitchname",		kCmd_Switch_getswitchname,			kCmdType_GET	},
	{	"getswitchvalue",		kCmd_Switch_getswitchvalue,			kCmdType_GET	},
	{	"minswitchvalue",		kCmd_Switch_minswitchvalue,			kCmdType_GET	},
	{	"maxswitchvalue",		kCmd_Switch_maxswitchvalue,			kCmdType_GET	},
	{	"setswitch",			kCmd_Switch_setswitch,				kCmdType_PUT	},
	{	"setswitchname",		kCmd_Switch_setswitchname,			kCmdType_PUT	},
	{	"setswitchvalue",		kCmd_Switch_setswitchvalue,			kCmdType_PUT	},
	{	"switchstep",			kCmd_Switch_switchstep,				kCmdType_GET	},

	//*	added by MLS
	{	"--extras",				kCmd_Switch_Extras,					kCmdType_GET	},
	{	"setswitchdescription",	kCmd_Switch_setswitchdescription,	kCmdType_GET	},
	{	"readall",				kCmd_Switch_readall,				kCmdType_GET	},

	{	"",						-1,	0x00	}
};


//**************************************************************************************
void	CreateSwitchObjects(void)
{
	new SwitchDriver();
}

//**************************************************************************************
SwitchDriver::SwitchDriver(void)
	:AlpacaDriver(kDeviceType_Switch)
{
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cDeviceName, "Switch");
	cNumSwitches	=	8;

	//*	zero out the switch description table
	for (iii=0; iii<kMaxSwitchCnt; iii++)
	{
		memset(&cSwitchTable[iii], 0, sizeof(TYPE_SwitchDescription));

		sprintf(cSwitchTable[iii].switchName, "Switch#%d", iii);
		strcpy(cSwitchTable[iii].switchDesciption, "Not defined");

		cSwitchType[iii]		=	kSwitchType_Bool;
		cMinSwitchValue[iii]	=	0.0;
		cMaxSwitchValue[iii]	=	1.0;
		cCurSwitchValue[iii]	=	0.0;
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

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	//*	make local copies of the data structure to make the code easier to read
	mySocket	=	reqData->socket;

	strcpy(alpacaErrMsg, "");
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
			alpacaErrCode	=	Get_Getswitch(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_getswitchdescription:	//*	Gets the description of the specified switch device
			alpacaErrCode	=	Get_Getswitchdescription(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_getswitchname:			//*	Gets the name of the specified switch device
			alpacaErrCode	=	Get_Getswitchname(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_getswitchvalue:		//*	Gets the value of the specified switch device as a double
			alpacaErrCode	=	Get_Getswitchvalue(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_minswitchvalue:		//*	Gets the minimum value of the specified switch device as a double
			alpacaErrCode	=	Get_Minswitchvalue(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_maxswitchvalue:		//*	Gets the maximum value of the specified switch device as a double
			alpacaErrCode	=	Get_Maxswitchvalue(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_setswitch:				//*	Sets a switch controller device to the specified state, true or false
			alpacaErrCode	=	Put_Setswitch(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_setswitchname:			//*	Sets a switch device name to the specified value
			alpacaErrCode	=	Put_Setswitchname(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_setswitchvalue:		//*	Sets a switch device value to the specified value
			alpacaErrCode	=	Put_Setswitchvalue(reqData, alpacaErrMsg);
			break;

		case kCmd_Switch_switchstep:			//*	Returns the step size that this device supports (the difference between successive values of the device).
			alpacaErrCode	=	Get_Switchstep(reqData, alpacaErrMsg, gValueString);
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

	//*	send the response information
	JsonResponse_Add_Int32(		mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ClientTransactionID",
								gClientTransactionID,
								INCLUDE_COMMA);

	JsonResponse_Add_Int32(		mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ServerTransactionID",
								gServerTransactionID,
								INCLUDE_COMMA);

	JsonResponse_Add_Int32(		mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ErrorNumber",
								alpacaErrCode,
								INCLUDE_COMMA);

	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ErrorMessage",
								alpacaErrMsg,
								NO_COMMA);

	JsonResponse_Add_Finish(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								kInclude_HTTP_Header);

	//*	this is for the logging function
	strcpy(reqData->alpacaErrMsg, alpacaErrMsg);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	returns -1 if not found
//*****************************************************************************
int	SwitchDriver::GetSwitchID(TYPE_GetPutRequestData *reqData)
{
bool	foundId;
char	idString[32];
int		switchNum;

	switchNum	=	-1;
	foundId	=	GetKeyWordArgument(	reqData->contentData,
									"Id",
									idString,
									31);
	if (foundId)
	{
		switchNum	=	atoi(idString);
	}
	return(switchNum);
}


//*****************************************************************************
//*	The number of switch devices managed by this driver
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_Maxswitch(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Int32(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cNumSwitches,
								INCLUDE_COMMA);

		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Indicates whether the specified switch device can be written to
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_Canwrite(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								true,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
//*	Return the state of switch device id as a boolean
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_Getswitch(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					switchNum;
bool				switchState;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		switchNum	=	GetSwitchID(reqData);
		if ((switchNum >= 0) && (switchNum < cNumSwitches))
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
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			sprintf(alpacaErrMsg, "Switch number (Id) not specified %s", __FUNCTION__);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Gets the description of the specified switch device
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_Getswitchdescription(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				foundId;
char				idString[32];
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		foundId	=	GetKeyWordArgument(	reqData->contentData,
										"Id",
										idString,
										31);
		if (foundId)
		{
			switchNum	=	atoi(idString);
			if ((switchNum >= 0) && (switchNum < cNumSwitches))
			{
				JsonResponse_Add_String(	reqData->socket,
											reqData->jsonTextBuffer,
											kMaxJsonBuffLen,
											gValueString,
											cSwitchTable[switchNum].switchDesciption,
											INCLUDE_COMMA);


				alpacaErrCode	=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Switch number out of range");
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			sprintf(alpacaErrMsg, "Switch number (Id) not specified %s", __FUNCTION__);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Gets the name of the specified switch device
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_Getswitchname(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				foundKeyWord;
char				idString[32];
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
												"Id",
												idString,
												31);
		if (foundKeyWord)
		{
			switchNum	=	atoi(idString);
			if ((switchNum >= 0) && (switchNum < cNumSwitches))
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
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Switch number out of range");
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			sprintf(alpacaErrMsg, "Switch number (Id) not specified %s", __FUNCTION__);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
//*	Gets the value of the specified switch device as a double
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_Getswitchvalue(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		switchNum	=	GetSwitchID(reqData);
		CONSOLE_DEBUG_W_NUM("switchNum\t\t=",		switchNum);
		CONSOLE_DEBUG_W_DBL("cCurSwitchValue\t=",	cCurSwitchValue[switchNum]);
		if ((switchNum >= 0) && (switchNum < cNumSwitches))
		{
			JsonResponse_Add_Double(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									gValueString,
									cCurSwitchValue[switchNum],
									INCLUDE_COMMA);
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			sprintf(alpacaErrMsg, "Switch number (Id) not specified %s", __FUNCTION__);
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		CONSOLE_DEBUG("kASCOM_Err_InternalError");
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Gets the minimum value of the specified switch device as a double
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_Minswitchvalue(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		switchNum	=	GetSwitchID(reqData);

		if ((switchNum >= 0) && (switchNum < kMaxSwitchCnt))
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
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Switch number out of range");
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Gets the maximum value of the specified switch device as a double
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_Maxswitchvalue(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		switchNum	=	GetSwitchID(reqData);
		CONSOLE_DEBUG_W_NUM("switchNum\t\t=",		switchNum);
		CONSOLE_DEBUG_W_DBL("cMaxSwitchValue\t=",	cMaxSwitchValue[switchNum]);

		if ((switchNum >= 0) && (switchNum < kMaxSwitchCnt))
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
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Switch number out of range");
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
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
TYPE_ASCOM_STATUS	SwitchDriver::Put_Setswitch(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				foundState;
char				stateString[32];
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		switchNum	=	GetSwitchID(reqData);

		if ((switchNum >= 0) && (switchNum < kMaxSwitchCnt))
		{
			foundState	=	GetKeyWordArgument(	reqData->contentData,
												"State",
												stateString,
												31);
			if (foundState)
			{
				if (strcasecmp(stateString, "true") == 0)
				{
					//*	turn the relay ON
					SetSwitchState(switchNum, true);

					cCurSwitchValue[switchNum]	=	cMaxSwitchValue[switchNum];
				}
				else
				{
					//*	turn the relay OFF
					SetSwitchState(switchNum, false);
					cCurSwitchValue[switchNum]	=	cMinSwitchValue[switchNum];
				}
				alpacaErrCode	=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "State not specified");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			sprintf(alpacaErrMsg, "Switch number (Id) not specified %s", __FUNCTION__);
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Sets a switch device name to the specified value
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Put_Setswitchname(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				foundName;
char				nameString[kMaxSwitchNameLen];
int					switchNum;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		switchNum	=	GetSwitchID(reqData);
		if ((switchNum >= 0) && (switchNum < cNumSwitches))
		{
			foundName	=	GetKeyWordArgument(	reqData->contentData,
												"Name",
												nameString,
												(kMaxSwitchNameLen - 1));
			if (foundName)
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
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Name not specified");
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			sprintf(alpacaErrMsg, "Switch number (Id) not specified %s", __FUNCTION__);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Sets a switch device value to the specified value as a double
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Put_Setswitchvalue(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
bool				foundValue;
int					switchNum;
char				valueString[32];
double				newSwitchValue;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		switchNum	=	GetSwitchID(reqData);
		if ((switchNum >= 0) && (switchNum < cNumSwitches))
		{
			foundValue	=	GetKeyWordArgument(	reqData->contentData,
												"Value",
												valueString,
												(sizeof(valueString) - 1));
			if (foundValue)
			{
				newSwitchValue				=	atof(valueString);
				if ((newSwitchValue >= cMinSwitchValue[switchNum]) &&
					(newSwitchValue <= cMaxSwitchValue[switchNum]))
				{
					SetSwitchValue(switchNum, newSwitchValue);
					cCurSwitchValue[switchNum]	=	atof(valueString);
					alpacaErrCode				=	kASCOM_Err_Success;
				}
				else
				{
					alpacaErrCode	=	kASCOM_Err_InvalidValue;
					sprintf(alpacaErrMsg, "Value out of range %s", __FUNCTION__);
				}
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			sprintf(alpacaErrMsg, "Switch number (Id) not specified %s", __FUNCTION__);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
//	CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=", alpacaErrCode);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Returns the step size that this device supports (the difference between successive values of the device).
//*****************************************************************************
TYPE_ASCOM_STATUS	SwitchDriver::Get_Switchstep(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								1.0,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
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
#endif // _DEBUG_CONFORM_

	if (reqData != NULL)
	{
		//*	do the common ones first
		Get_Readall_Common(	reqData, alpacaErrMsg);

		//*	make local copies of the data structure to make the code easier to read
		mySocket	=	reqData->socket;

		Get_Maxswitch(	reqData, alpacaErrMsg, "maxswitch");	//*	The number of switch devices managed by this driver
		Get_Canwrite(	reqData, alpacaErrMsg, "canwrite");		//*	Indicates whether the specified switch device can be written to

		for (iii=0; iii<cNumSwitches; iii++)
		{
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
										cSwitchTable[iii].switchDesciption,
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


			if (cSwitchType[iii] == kSwitchType_Analog)
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
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
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
	if (reqData != NULL)
	{
		mySocketFD	=	reqData->socket;

		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
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

		for (ii=0; ii<cNumSwitches; ii++)
		{

			SocketWriteData(mySocketFD,	"<TR>\r\n\t<TD>");
			sprintf(lineBuffer, "%d", ii);
			SocketWriteData(mySocketFD,	lineBuffer);
			SocketWriteData(mySocketFD,	"</TD>\r\n\t<TD>");
			SocketWriteData(mySocketFD,	cSwitchTable[ii].switchName);
			SocketWriteData(mySocketFD,	"</TD>\r\n\t<TD>");
			SocketWriteData(mySocketFD,	cSwitchTable[ii].switchDesciption);
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

		GenerateHTMLcmdLinkTable(mySocketFD, "switch", cDeviceNum, gSwitchCmdTable);
	}
}

//*****************************************************************************
bool	SwitchDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gSwitchCmdTable, getPut);
	return(foundIt);
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
										strcpy(cSwitchTable[switchNum].switchDesciption, argBuf);
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
		for (ii=0; ii<cNumSwitches; ii++)
		{
			fprintf(filePointer, "%d\t",	ii);
			fprintf(filePointer, "%s\t",	cSwitchTable[ii].switchName);
			fprintf(filePointer, "%s\t",	cSwitchTable[ii].switchDesciption);
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
void	SwitchDriver::SetSwitchState(const int switchNumber, bool on_off)
{
	//*	this function meant to be overloaded
}

//*****************************************************************************
void	SwitchDriver::SetSwitchValue(const int switchNumber, double switchValue)
{
	//*	this function meant to be overloaded
}



#endif	//*	_ENABLE_SWITCH_

