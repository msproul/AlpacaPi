//**************************************************************************
//*	Name:			focuserdriver.cpp
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
//*	May 24,	2019	<MLS> Started implementing focuser
//*	Dec  4,	2019	<MLS> Started on C++ version of focuser driver
//*	Dec  9,	2019	<MLS> Added Get_Temperature()
//*	Dec 14,	2019	<MLS> The Moonlite NiteCrawler focuser also supports rotation
//*	Dec 14,	2019	<MLS> Adding rotator support to the focuser class
//*	Dec 14,	2019	<MLS> Added RotationSupported()
//*	Dec 19,	2019	<MLS> Added HaltFocuser()
//*	Feb 29,	2020	<MLS> Added moonlite switch info to ReadAll
//*	Apr  2,	2020	<MLS> CONFORM-focuser -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	Jan 24,	2021	<MLS> Converted FocuserDriver to use properties struct
//*****************************************************************************

#ifdef _ENABLE_FOCUSER_

#include	<math.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<time.h>
#include	<unistd.h>



#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"


#include	"JsonResponse.h"
#include	"focuserdriver.h"

#ifdef	_ENABLE_ROTATOR_
	#include	"rotatordriver.h"
#endif // _ENABLE_ROTATOR_



//*****************************************************************************
const TYPE_CmdEntry	gFocuserCmdTable[]	=
{

	{	"absolute",				kCmd_Focuser_absolute,			kCmdType_GET	},
	{	"ismoving",				kCmd_Focuser_ismoving,			kCmdType_GET	},
	{	"maxincrement",			kCmd_Focuser_maxincrement,		kCmdType_GET	},
	{	"maxstep",				kCmd_Focuser_maxstep,			kCmdType_GET	},
	{	"position",				kCmd_Focuser_position,			kCmdType_GET	},
	{	"stepsize",				kCmd_Focuser_stepsize,			kCmdType_GET	},
	{	"tempcomp",				kCmd_Focuser_tempcomp,			kCmdType_BOTH	},
	{	"tempcompavailable",	kCmd_Focuser_tempcompavailable,	kCmdType_GET	},
	{	"temperature",			kCmd_Focuser_temperature,		kCmdType_GET	},
	{	"halt",					kCmd_Focuser_halt,				kCmdType_PUT	},
	{	"move",					kCmd_Focuser_move,				kCmdType_PUT	},

	//*	items added by MLS
	{	"--extras",				kCmd_Focuser_Extras,			kCmdType_GET	},
	{	"moverelative",			kCmd_Focuser_moverelative,		kCmdType_PUT	},
	{	"readall",				kCmd_Focuser_readall,			kCmdType_GET	},


	{	"",						-1,	0x00	}
};



//**************************************************************************************
FocuserDriver::FocuserDriver(const int argDevNum)
	:AlpacaDriver(kDeviceType_Focuser)
{

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name, "Focuser");

	memset(&cFocuserProp, 0, sizeof(TYPE_FocuserProperties));

	cFocuserProp.MaxStep		=	10;
	cFocuserProp.MaxIncrement	=	10;
	cFocuserProp.Position		=	-1;


	cPrevFocuserPosition		=	-1;
	cNewFocuserPosition			=	-1;
//-	cFocuserStepSize			=	0.0;
//-	cHasTempComp				=	false;
//-	cTempCompEnabled			=	false;

	//*	all of the following is for support of Moonlite NiteCrawler
	cIsNiteCrawler				=	false;
	cFocuserSupportsRotation	=	false;
	cRotatorPosition			=	-1;
	cPrevRotatorPosition		=	-1;
	cNewRotatorPosition			=	-1;
	cRotatorIsMoving			=	false;

	cFocuserSupportsAux			=	false;
	cAuxPosition				=	-1;
	cPrevAuxPosition			=	-1;
	cNewAuxPosition				=	-1;
	cAuxIsMoving				=	false;

	cFocuserHasVoltage			=	false;
	cFocuserVoltage				=	0.0;
	cFocuserHasTemperature		=	false;
	cFocuserTemp				=	0.0;

	cSwitchIN					=	false;
	cSwitchOUT					=	false;
	cSwitchROT					=	false;
	cSwitchAUX1					=	false;
	cSwitchAUX2					=	false;

}

//**************************************************************************************
// Destructor
//**************************************************************************************
FocuserDriver::~FocuserDriver(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
int32_t	FocuserDriver::RunStateMachine(void)
{
	return(1000 * 1000);
}


//*****************************************************************************
bool	FocuserDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gFocuserCmdTable, getPut);
	return(foundIt);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				alpacaErrMsg[256];
int					cmdEnumValue;
int					cmdType;
int					mySocket;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, reqData->deviceCommand);

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
								cCommonProp.Name,
								INCLUDE_COMMA);

	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Command",
								reqData->deviceCommand,
								INCLUDE_COMMA);

	//*	look up the command
	cmdEnumValue	=	FindCmdFromTable(reqData->deviceCommand, gFocuserCmdTable, &cmdType);
	switch(cmdEnumValue)
	{
		//----------------------------------------------------------------------------------------
		//*	Common commands that we want to over ride
		//----------------------------------------------------------------------------------------
		case kCmd_Common_supportedactions:	//*	Returns the list of action names supported by this driver.
			alpacaErrCode	=	Get_SupportedActions(reqData, gFocuserCmdTable);
			break;

		//----------------------------------------------------------------------------------------
		//*	Device specific commands
		//----------------------------------------------------------------------------------------
		case kCmd_Focuser_absolute:				//*	Indicates whether the focuser is capable of absolute position.
			alpacaErrCode	=	Get_Absolute(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Focuser_ismoving:				//*	Indicates whether the focuser is currently moving.
			alpacaErrCode	=	Get_Ismoving(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Focuser_maxincrement:			//*	Returns the focuser's maximum increment size.
			alpacaErrCode	=	Get_Maxincrement(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Focuser_maxstep:				//*	Returns the focuser's maximum step size.
			alpacaErrCode	=	Get_Maxstep(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Focuser_position:				//*	Returns the focuser's current position.
			alpacaErrCode	=	Get_Position(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Focuser_stepsize:				//*	Returns the focuser's step size.
			alpacaErrCode	=	Get_Stepsize(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Focuser_tempcomp:				//*	Retrieves the state of temperature compensation mode
												//*	Sets the device's temperature compensation mode.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_Tempcomp(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_Tempcomp(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Focuser_tempcompavailable:	//*	Indicates whether the focuser has temperature compensation.
			alpacaErrCode	=	Get_Tempcompavailable(reqData, alpacaErrMsg, gValueString);
			break;
			break;

		case kCmd_Focuser_temperature:			//*	Returns the focuser's current temperature.
			alpacaErrCode	=	Get_Temperature(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Focuser_halt:					//*	Immediately stops focuser motion.
			alpacaErrCode	=	Put_Halt(reqData, alpacaErrMsg);
			break;

		case kCmd_Focuser_move:					//*	Moves the focuser to a new position.
			alpacaErrCode	=	Put_Move(reqData, alpacaErrMsg);
			break;

		case kCmd_Focuser_moverelative:					//*	Moves the focuser to a new position.
			alpacaErrCode	=	Put_MoveRelative(reqData, alpacaErrMsg);
			break;


		case kCmd_Focuser_readall:
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
TYPE_ASCOM_STATUS	FocuserDriver::Get_Absolute(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cFocuserProp.Absolute,
							INCLUDE_COMMA);


	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Get_Ismoving(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cFocuserProp.IsMoving,
							INCLUDE_COMMA);

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Get_Maxincrement(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cFocuserProp.MaxIncrement,
							INCLUDE_COMMA);

	alpacaErrCode	=	kASCOM_Err_Success;


	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Get_Maxstep(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cFocuserProp.MaxStep,
							INCLUDE_COMMA);

	alpacaErrCode	=	kASCOM_Err_Success;

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Get_Position(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Int32(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cFocuserProp.Position,
							INCLUDE_COMMA);

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Get_Stepsize(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cFocuserProp.StepSize,
								INCLUDE_COMMA);

	JsonResponse_Add_String(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Comment",
								"Stepsize in microns",
								INCLUDE_COMMA);

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Get_Tempcomp(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cFocuserProp.TempComp,
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
TYPE_ASCOM_STATUS	FocuserDriver::Put_Tempcomp(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
bool				foundKeyWord;
char				argumentString[32];

	if (reqData != NULL)
	{
		if (cFocuserProp.TempCompAvailable)
		{
			foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
													"TempComp",
													argumentString,
													(sizeof(argumentString) -1));
			if (foundKeyWord)
			{
				cFocuserProp.TempComp	=	IsTrueFalse(argumentString);;
				alpacaErrCode			=	kASCOM_Err_Success;
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "keyword 'TempComp' not specified");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TempComp not supported");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Get_Tempcompavailable(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cFocuserProp.TempCompAvailable,
							INCLUDE_COMMA);


	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Get_Temperature(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cFocuserTemp,
								INCLUDE_COMMA);

	JsonResponse_Add_Double(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Degrees-F",
								DEGREES_F(cFocuserTemp),
								INCLUDE_COMMA);


	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Put_Halt(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		alpacaErrCode	=	HaltFocuser(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Put_Move(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
bool				foundKeyWord;
char				argumentString[32];
int32_t				newPosition;

	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);

		foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
												"Position",
												argumentString,
												(sizeof(argumentString) -1));
		if (foundKeyWord)
		{
			newPosition		=	atoi(argumentString);
			alpacaErrCode	=	SetFocuserPosition(newPosition, alpacaErrMsg);
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "keyword 'Position' not specified");
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
TYPE_ASCOM_STATUS	FocuserDriver::Put_MoveRelative(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
bool				foundKeyWord;
char				argumentString[32];
int32_t				newPosition;

	if (reqData != NULL)
	{
		foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
												"Position",
												argumentString,
												(sizeof(argumentString) -1));
		if (foundKeyWord)
		{
			//*	new position is the current position plus the new offset
			newPosition		=	cFocuserProp.Position + atoi(argumentString);
			alpacaErrCode	=	SetFocuserPosition(newPosition, alpacaErrMsg);
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Position' not specified");
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
TYPE_ASCOM_STATUS	FocuserDriver::Get_Readall(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		//*	do the common ones first
		Get_Readall_Common(			reqData, alpacaErrMsg);

		//*	do the standard Alpaca fields first
		Get_Absolute(			reqData,	alpacaErrMsg,	"absolute");
		Get_Ismoving(			reqData,	alpacaErrMsg,	"ismoving");
		Get_Maxincrement(		reqData,	alpacaErrMsg,	"maxincrement");
		Get_Maxstep(			reqData,	alpacaErrMsg,	"maxstep");
		Get_Position(			reqData,	alpacaErrMsg,	"position");
		Get_Stepsize(			reqData,	alpacaErrMsg,	"stepsize");
		Get_Tempcomp(			reqData,	alpacaErrMsg,	"tempcomp");
		Get_Tempcompavailable(	reqData,	alpacaErrMsg,	"tempcompavailable");
		Get_Temperature(		reqData,	alpacaErrMsg,	"temperature");


		//==========================================================
		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"version",
									gFullVersionString,
									INCLUDE_COMMA);

		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"Model",
									cDeviceModel,
									INCLUDE_COMMA);

		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"SerialNum",
									cDeviceSerialNum,
									INCLUDE_COMMA);


		if (cIsNiteCrawler)
		{
			JsonResponse_Add_Bool(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"SwitchIN",
									cSwitchIN,
									INCLUDE_COMMA);

			JsonResponse_Add_Bool(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"SwitchOUT",
									cSwitchOUT,
									INCLUDE_COMMA);
		}

		if (cFocuserSupportsRotation)
		{
			JsonResponse_Add_Int32(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"RotatorPosition",
									cRotatorPosition,
									INCLUDE_COMMA);

			JsonResponse_Add_Bool(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"RotatorIsMoving",
									cRotatorIsMoving,
									INCLUDE_COMMA);

			JsonResponse_Add_Bool(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"SwitchROT",
									cSwitchROT,
									INCLUDE_COMMA);

		}


		if (cFocuserSupportsAux)
		{
			JsonResponse_Add_Int32(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"AuxPosition",
									cAuxPosition,
									INCLUDE_COMMA);

			JsonResponse_Add_Bool(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"AuxIsMoving",
									cAuxIsMoving,
									INCLUDE_COMMA);

			JsonResponse_Add_Bool(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"SwitchAUX1",
									cSwitchAUX1,
									INCLUDE_COMMA);

			JsonResponse_Add_Bool(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"SwitchAUX2",
									cSwitchAUX2,
									INCLUDE_COMMA);
		}


		if (cFocuserHasVoltage)
		{
			JsonResponse_Add_Double(	reqData->socket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										"Voltage",
										cFocuserVoltage,
										INCLUDE_COMMA);

		}
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}


#pragma mark -



//*****************************************************************************
void	FocuserDriver::OutputHTML(TYPE_GetPutRequestData *reqData)
{
int			mySocketFD;
char		lineBuffer[128];

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		mySocketFD	=	reqData->socket;


		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<H2>Focuser</H2>\r\n");

		SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");


		//*-----------------------------------------------------------
		OutputHTMLrowData(mySocketFD,	"Focuser",		cCommonProp.Name);
		OutputHTMLrowData(mySocketFD,	"Model",		cDeviceModel);
		OutputHTMLrowData(mySocketFD,	"Version",		cDeviceVersion);
		OutputHTMLrowData(mySocketFD,	"Serial Num",	cDeviceSerialNum);

		//*	focuser position
		sprintf(lineBuffer, "%d", cFocuserProp.Position);
		OutputHTMLrowData(mySocketFD,	"Focuser position",	lineBuffer);

		//*	rotator position
		sprintf(lineBuffer, "%d", cRotatorPosition);
		OutputHTMLrowData(mySocketFD,	"Rotator position",	lineBuffer);

		//*	Aux position
		sprintf(lineBuffer, "%d", cAuxPosition);
		OutputHTMLrowData(mySocketFD,	"Aux position",	lineBuffer);

		//*	Temperature
		sprintf(lineBuffer, "%3.1f&deg;C / %3.1f&deg;F", cFocuserTemp,  DEGREES_F(cFocuserTemp));
		OutputHTMLrowData(mySocketFD,	"Temperature",	lineBuffer);


		sprintf(lineBuffer, "%3.1f VDC", cFocuserVoltage);
		OutputHTMLrowData(mySocketFD,	"Voltage",	lineBuffer);

		//*	error count
		sprintf(lineBuffer, "%d", cInvalidStringErrCnt);
		OutputHTMLrowData(mySocketFD,	"Error count",	lineBuffer);



		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<P>\r\n");
	}
}

//*****************************************************************************
void	FocuserDriver::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
	CONSOLE_DEBUG(__FUNCTION__);
		//*	now generate links to all of the commands
	GenerateHTMLcmdLinkTable(reqData->socket, "focuser", cDeviceNum, gFocuserCmdTable);
}

//*****************************************************************************
TYPE_ASCOM_STATUS		FocuserDriver::SetFocuserPosition(const int32_t newPosition, char *alpacaErrMsg)
{
	//*	this should be overridden by the subclass
	return(kASCOM_Err_NotImplemented);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::HaltFocuser(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);

//	strcpy(lastCameraErrMsg, "Command not implemented: ");
//	strcat(lastCameraErrMsg, __FUNCTION__);
	alpacaErrCode	=	kASCOM_Err_NotImplemented;
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Position' not specified");
	CONSOLE_DEBUG(alpacaErrMsg);

	return(alpacaErrCode);
}

//*****************************************************************************
int32_t	FocuserDriver::GetFocuserPosition(void)
{
	return(cFocuserProp.Position);
}

//*****************************************************************************
void	FocuserDriver::GetFocuserManufacturer(char *manufactString)
{
	strcpy(manufactString,	cDeviceManufacturer);
}

//*****************************************************************************
void	FocuserDriver::GetFocuserModel(char *modelName)
{
	strcpy(modelName,	cDeviceModel);
}

//*****************************************************************************
void	FocuserDriver::GetFocuserVersion(char *versionString)
{
	strcpy(versionString,	cDeviceVersion);
}

//*****************************************************************************
void	FocuserDriver::GetFocuserSerialNumber(char *serialNumString)
{
	strcpy(serialNumString,	cDeviceSerialNum);
}


//*****************************************************************************
double	FocuserDriver::GetFocuserTemperature(void)
{
	return(cFocuserTemp);
}

//*****************************************************************************
double	FocuserDriver::GetFocuserVoltage(void)
{
	return(cFocuserVoltage);
}


#pragma mark -
//*****************************************************************************
bool	FocuserDriver::RotationSupported(void)
{
	return(cFocuserSupportsRotation);
}

//*****************************************************************************
int32_t	FocuserDriver::GetRotatorPosition(void)
{
	return(cRotatorPosition);
}

//*****************************************************************************
int32_t	FocuserDriver::GetRotatorStepsPerRev(void)
{
	return(cRotatorStepsPerRev);
}

//*****************************************************************************
bool	FocuserDriver::GetRotatorIsMoving(void)
{
	if (cRotatorIsMoving == false)
	{
		CONSOLE_DEBUG("Calling RunStateMachine()");
		RunStateMachine();
		CONSOLE_DEBUG_W_NUM("cRotatorIsMoving\t=", cRotatorIsMoving);
	}
	return(cRotatorIsMoving);
}




#endif	//	_ENABLE_FOCUSER_
