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
//*	Jun 23,	2021	<MLS> Updated FocuserDriver cCommonProp.InterfaceVersion to 3
//*	Oct 10,	2022	<MLS> Started migrating NiteCrawler support to use TYPE_RotatorProperties
//*	Oct 20,	2022	<MLS> Added DumpFocuserProperties()
//*	Nov  4,	2022	<MLS> Added GetCommandArgumentString()
//*	Nov  8,	2022	<MLS> Fixed bug in JSON for temperatureLog in all drivers.
//*	Jun 18,	2023	<MLS> Added DeviceState_Add_Content() to focuser driver
//*	May 17,	2024	<MLS> Added http error 400 processing to focuser driver
//*	Jun 28,	2024	<MLS> Removed all "if (reqData != NULL)" from focuserdriver.cpp
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


#define _DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"


#include	"JsonResponse.h"
#include	"focuserdriver.h"

#ifdef _ENABLE_FOCUSER_MOONLITE_
	#include	"focuserdriver_nc.h"
#endif
#ifdef _ENABLE_FOCUSER_ZWO_
	#include	"focuserdriver_ZWO.h"
#endif
#ifdef _ENABLE_FOCUSER_SIMULATOR_
	#include	"focuserdriver_sim.h"
#endif

#ifdef	_ENABLE_ROTATOR_
	#include	"rotatordriver.h"
#endif

#include	"focuser_AlpacaCmds.h"
#include	"focuser_AlpacaCmds.cpp"


//*****************************************************************************
int	CreateFocuserObjects(void)
{
int		focuserCnt;

	focuserCnt	=	0;
#ifdef _ENABLE_FOCUSER_MOONLITE_
	focuserCnt	+=	CreateFocuserObjects_MoonLite();
#endif

#ifdef _ENABLE_FOCUSER_SIMULATOR_
	focuserCnt	+=	CreateFocuserObjects_SIM();
#endif
#ifdef _ENABLE_FOCUSER_ZWO_
	focuserCnt	+=	CreateFocuserObjects_ZWO();
#endif

	return(focuserCnt);
}



//**************************************************************************************
FocuserDriver::FocuserDriver(void)
	:AlpacaDriver(kDeviceType_Focuser)
{
	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name,		"Focuser");
	strcpy(cCommonProp.Description,	"Generic Focuser");
	cCommonProp.InterfaceVersion	=	3;
	cDriverCmdTablePtr				=	gFocuserCmdTable;

	memset(&cFocuserProp, 0, sizeof(TYPE_FocuserProperties));
	memset(&cRotatorProp, 0, sizeof(TYPE_RotatorProperties));

	cFocuserProp.MaxStep			=	10;
	cFocuserProp.MaxIncrement		=	10;
	cFocuserProp.Position			=	-1;
	cFocuserProp.IsMoving			=	false;

	cPrevFocuserPosition			=	-1;
	cNewFocuserPosition				=	-1;

	//*	all of the following is for support of Moonlite NiteCrawler
	cIsNiteCrawler					=	false;
	cFocuserSupportsRotation		=	false;
	cRotatorPosition				=	-1;
	cPrevRotatorPosition			=	-1;
	cNewRotatorPosition				=	-1;

	cFocuserSupportsAux				=	false;
	cAuxPosition					=	-1;
	cPrevAuxPosition				=	-1;
	cNewAuxPosition					=	-1;
	cAuxIsMoving					=	false;

	cFocuserHasVoltage				=	false;
	cFocuserVoltage					=	0.0;
	cFocuserHasTemperature			=	false;
	cFocuserProp.Temperature_DegC	=	0.0;

	cSwitchIN						=	false;
	cSwitchOUT						=	false;
	cSwitchROT						=	false;
	cSwitchAUX1						=	false;
	cSwitchAUX2						=	false;
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
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This routine should be over-ridden");
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

	strcpy(cLastDeviceErrMsg, "");

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

	if (cSendJSONresponse)	//*	False for setupdialog and camera binary data
	{
		//*	send the response information
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Uint32(		mySocket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										"ClientTransactionID",
										reqData->ClientTransactionID,
										INCLUDE_COMMA);

		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Uint32(		mySocket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										"ServerTransactionID",
										gServerTransactionID,
										INCLUDE_COMMA);

		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Int32(		mySocket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										"ErrorNumber",
										alpacaErrCode,
										INCLUDE_COMMA);

		cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(	mySocket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										"ErrorMessage",
										alpacaErrMsg,
										NO_COMMA);

		//*	Nov  8,	2022	<MLS> Fixed bug in JSON for temperatureLog in all drivers.
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Finish(	mySocket,
																reqData->httpRetCode,
																reqData->jsonTextBuffer,
																(cHttpHeaderSent == false));	//*	required for long JSON output
	}
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

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cFocuserProp.TempComp,
							INCLUDE_COMMA);
	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Put_Tempcomp(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
bool				foundKeyWord;
char				argumentString[32];
bool				validData;

	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"TempComp",
											argumentString,
											(sizeof(argumentString) -1));
	validData	=	false;
	if (foundKeyWord)
	{
		if (IsValidTrueFalseString(argumentString))
		{
			validData	=	true;
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TempComp is non-boolean");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "keyword 'TempComp' not specified");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	if (cFocuserProp.TempCompAvailable)
	{
		if (validData)
		{
			cFocuserProp.TempComp	=	IsTrueFalse(argumentString);
			alpacaErrCode			=	kASCOM_Err_Success;
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "TempComp not supported");
		CONSOLE_DEBUG(alpacaErrMsg);
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
//*	this relies on the the RunStateMachine to update the temperature on a regular basis
//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Get_Temperature(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cFocuserProp.Temperature_DegC,
								INCLUDE_COMMA);

	JsonResponse_Add_Double(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Degrees-F",
								DEGREES_F(cFocuserProp.Temperature_DegC),
								INCLUDE_COMMA);


	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Put_Halt(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	HaltFocuser(alpacaErrMsg);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Put_Move(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
bool				foundKeyWord;
char				argumentString[32];
int32_t				newPosition;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "**********************************************************");
//	SETUP_TIMING();

	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Position",
											argumentString,
											(sizeof(argumentString) -1));
	if (foundKeyWord)
	{
		newPosition		=	atoi(argumentString);
//	DEBUG_TIMING(__FUNCTION__);
		alpacaErrCode	=	SetFocuserPosition(newPosition, alpacaErrMsg);
//	DEBUG_TIMING(__FUNCTION__);
		if (alpacaErrCode != kASCOM_Err_Success)
		{
			CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=",	alpacaErrCode);
			CONSOLE_DEBUG_W_STR("alpacaErrMsg \t=",	alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "keyword 'Position' not specified");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

//	DEBUG_TIMING(__FUNCTION__);
//	CONSOLE_DEBUG("EXIT **********************************************************");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Put_MoveRelative(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
bool				foundKeyWord;
char				argumentString[32];
int32_t				newPosition;

	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Position",
											argumentString,
											(sizeof(argumentString) -1));
	if (foundKeyWord)
	{
		if (IsValidNumericString(argumentString))
		{
			//*	new position is the current position plus the new offset
			newPosition		=	cFocuserProp.Position + atoi(argumentString);
			alpacaErrCode	=	SetFocuserPosition(newPosition, alpacaErrMsg);
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Position is non-numeric");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Position' not specified");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//IsMoving
//Position
//Temperature
//*****************************************************************************
bool	FocuserDriver::DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	DeviceState_Add_Bool(socketFD,	jsonTextBuffer, maxLen,	"IsMoving",		cFocuserProp.IsMoving);
	DeviceState_Add_Int(socketFD,	jsonTextBuffer, maxLen,	"Position",		cFocuserProp.Position);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"Temperature",	cFocuserProp.Temperature_DegC);

	return(true);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriver::Get_Readall(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

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
								cRotatorProp.IsMoving,
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
	return(alpacaErrCode);
}


#pragma mark -



//*****************************************************************************
void	FocuserDriver::OutputHTML(TYPE_GetPutRequestData *reqData)
{
int			mySocketFD;
char		lineBuffer[128];

//	CONSOLE_DEBUG(__FUNCTION__);
	mySocketFD	=	reqData->socket;


	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H2>AlpacaPi Focuser</H2>\r\n");

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
	sprintf(lineBuffer, "%3.1f&deg;C / %3.1f&deg;F", cFocuserProp.Temperature_DegC,  DEGREES_F(cFocuserProp.Temperature_DegC));
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

//*****************************************************************************
void	FocuserDriver::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
//	CONSOLE_DEBUG(__FUNCTION__);
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
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "This routine needs to be over-ridden");
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
	return(cFocuserProp.Temperature_DegC);
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
	if (cRotatorProp.IsMoving == false)
	{
//		CONSOLE_DEBUG("Calling RunStateMachine()");
		RunStateMachine();
//		CONSOLE_DEBUG_W_NUM("cRotatorProp.IsMoving\t=", cRotatorProp.IsMoving);
	}
	return(cRotatorProp.IsMoving);
}

//*****************************************************************************
void	FocuserDriver::DumpFocuserProperties(const char *callingFunctionName)
{
	DumpCommonProperties(callingFunctionName);

	CONSOLE_DEBUG(			"------------------------------------");
	CONSOLE_DEBUG_W_BOOL(	"cFocuserProp.Absolute         \t=",	cFocuserProp.Absolute);
	CONSOLE_DEBUG_W_BOOL(	"cFocuserProp.IsMoving         \t=",	cFocuserProp.IsMoving);
	CONSOLE_DEBUG_W_NUM(	"cFocuserProp.MaxIncrement     \t=",	cFocuserProp.MaxIncrement);
	CONSOLE_DEBUG_W_NUM(	"cFocuserProp.MaxStep          \t=",	cFocuserProp.MaxStep);
	CONSOLE_DEBUG_W_NUM(	"cFocuserProp.Position         \t=",	cFocuserProp.Position);
	CONSOLE_DEBUG_W_DBL(	"cFocuserProp.StepSize         \t=",	cFocuserProp.StepSize);
	CONSOLE_DEBUG_W_BOOL(	"cFocuserProp.TempComp         \t=",	cFocuserProp.TempComp);
	CONSOLE_DEBUG_W_BOOL(	"cFocuserProp.TempCompAvailable\t=",	cFocuserProp.TempCompAvailable);
	CONSOLE_DEBUG_W_DBL(	"cFocuserProp.Temperature_DegC \t=",	cFocuserProp.Temperature_DegC);
}

//*****************************************************************************
bool	FocuserDriver::GetCommandArgumentString(const int cmdENum, char *agumentString, char *commentString)
{
bool	foundFlag	=	true;

	switch(cmdENum)
	{
		case kCmd_Focuser_tempcomp:				strcpy(agumentString, "TempComp=BOOL");	break;

		case kCmd_Focuser_move:
		case kCmd_Focuser_moverelative:			strcpy(agumentString, "Position=INT");	break;



		case kCmd_Focuser_absolute:				//*	Indicates whether the focuser is capable of absolute position.
		case kCmd_Focuser_ismoving:				//*	Indicates whether the focuser is currently moving.
		case kCmd_Focuser_maxincrement:			//*	Returns the focuser's maximum increment size.
		case kCmd_Focuser_maxstep:				//*	Returns the focuser's maximum step size.
		case kCmd_Focuser_position:				//*	Returns the focuser's current position.
		case kCmd_Focuser_stepsize:				//*	Returns the focuser's step size.
		case kCmd_Focuser_tempcompavailable:	//*	Indicates whether the focuser has temperature compensation.
		case kCmd_Focuser_temperature:			//*	Returns the focuser's current temperature.
		case kCmd_Focuser_halt:					//*	Immediately stops focuser motion.
		case kCmd_Focuser_readall:				//*	Reads all of the values
			strcpy(agumentString, "-none-");
			break;


		default:
			strcpy(agumentString, "");
			foundFlag	=	false;
			break;
	}
	return(foundFlag);
}


#endif	//	_ENABLE_FOCUSER_
