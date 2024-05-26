//**************************************************************************
//*	Name:			rotatordriver.cpp
//*
//*	Author:			Mark Sproul (C) 2019-2024
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
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Dec 14,	2019	<MLS> Started on rotator driver
//*	Dec 25,	2019	<MLS> Changed long to int32_t
//*	Jan 11,	2020	<MLS> Added GetRotatorSerialNumber()
//*	Apr  1,	2020	<MLS> Rotator position is supposed to be in degrees
//*	Apr  1,	2020	<MLS> Added ReadCurrentPoisiton_steps()
//*	Apr  1,	2020	<MLS> Added ReadCurrentPoisiton_degs()
//*	Apr  1,	2020	<MLS> Added SetCurrentPoisiton_steps()
//*	Apr  1,	2020	<MLS> Added SetCurrentPoisiton_degs()
//*	Apr  2,	2020	<MLS> CONFORM-rotator -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	Apr 19,	2020	<MLS> Finished ReadAll for Rotator
//*	May 19,	2020	<MLS> Started on implementing Rotator REVERSE functionality
//*	Oct  8,	2022	<MLS> Working on implementing IRotatorV3 functionality
//*	Oct  8,	2022	<MLS> Added Get_MechanicalPosition()
//*	Oct  8,	2022	<MLS> Added Put_MoveMechanical()
//*	Oct  8,	2022	<MLS> Added Put_Sync()
//*	Oct 10,	2022	<MLS> Added RunStateMachine() to Rotatordriver
//*	Oct 10,	2022	<MLS> Added UpdateRotorPosition()
//*	Oct 11,	2022	<MLS> Put_Move() now working properly (relative move)
//*	Mar  1,	2023	<MLS> Added DumpRotatorProperties()
//*	Mar  1,	2023	<MLS> Working on Reverse property to pass CONFORMU
//*	Mar  2,	2023	<MLS> Working on Sync method to pass CONFORMU
//*	Jun 18,	2023	<MLS> Added DeviceState_Add_Content() to rotator driver
//*	May 18,	2024	<MLS> Started on http 400 error support for rotator driver
//*****************************************************************************

#ifdef _ENABLE_ROTATOR_

#include	<stdlib.h>
#include	<string.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"
#include	"eventlogging.h"

#include	"helper_functions.h"
#include	"JsonResponse.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"rotatordriver.h"

#ifdef _ENABLE_ROTATOR_SIMULATOR_
	#include	"rotatordriver_sim.h"
#endif


#include	"rotator_AlpacaCmds.h"
#include	"rotator_AlpacaCmds.cpp"


//*****************************************************************************
void	CreateRotatorObjects(void)
{
//*	the night crawler rotator is created from the focuser object because they have to be interlinked

#ifdef _ENABLE_ROTATOR_SIMULATOR_
	CreateRotatorObjects_SIM();
#endif
}



//*****************************************************************************
static double	AdjustDegrees0_360(const double degreeValue)
{
double	newDegreeValue;

	newDegreeValue	=	degreeValue;
	while (newDegreeValue < 0.0)
	{
		newDegreeValue	+=	360.0;
	}
	while (newDegreeValue >= 360.0)
	{
		newDegreeValue	-=	360.0;
	}
	return(newDegreeValue);
}

//**************************************************************************************
RotatorDriver::RotatorDriver(const int argDevNum)
	:AlpacaDriver(kDeviceType_Rotator)
{

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name,		"Rotator");
	strcpy(cCommonProp.Description,	"Generic Rotator");
	cCommonProp.InterfaceVersion	=	2;
	cDriverCmdTablePtr				=	gRotatorCmdTable;

	cRotatorManufacturer[0]	=	0;
	cRotatorModel[0]		=	0;
	cRotatorSerialNum[0]	=	0;

	//-------------------------------------------------------
	memset(&cRotatorProp, 0, sizeof(TYPE_RotatorProperties));
	cRotatorProp.CanReverse		=	false;		//*	True if the rotation and angular direction must be reversed for the optics
	cRotatorProp.IsMoving		=	false;
	cRotatorProp.Reverse		=	false;
	cRotatorProp.Position		=	0.0;
	cRotatorProp.StepSize		=	1.0;
	cRotatorProp.SyncOffset		=	0.0;
	cRotatorProp.TargetPosition	=	0.0;

	cRotatorStepsPerRev			=	360;
	cRotatorPosition_steps		=	0;
//	cRotatorPosition_degs		=	0.0;
}


//**************************************************************************************
// Destructor
//**************************************************************************************
RotatorDriver::~RotatorDriver(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
int32_t		RotatorDriver::RunStateMachine(void)
{
uint32_t	currentMilliSecs;
uint32_t	deltaMilliSecs;

	if (cRunStartupOperations)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		UpdateRotorPosition(true);

//		CONSOLE_DEBUG_W_DBL("cRotatorProp.MechanicalPosition\t=",	cRotatorProp.MechanicalPosition);
//		CONSOLE_DEBUG_W_DBL("cRotatorProp.Position          \t=",	cRotatorProp.Position);
//		CONSOLE_DEBUG_W_DBL("cRotatorProp.TargetPosition    \t=",	cRotatorProp.TargetPosition);

		cRunStartupOperations		=	false;
	}

	currentMilliSecs	=	millis();
	deltaMilliSecs		=	currentMilliSecs - cLastUpdate_milliSecs;
	if (deltaMilliSecs > 2000)
	{
		if (cRotatorProp.IsMoving)
		{
			UpdateRotorPosition(false);
		}
		else
		{
			UpdateRotorPosition(true);
		}

		cLastUpdate_milliSecs	=	currentMilliSecs;
	}

	return(5 * 1000 * 1000);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
char				alpacaErrMsg[256];
int					cmdEnumValue;
int					cmdType;
int					mySocket;
//int				myDeviceNum;

//	CONSOLE_DEBUG(__FUNCTION__);


	//*	make local copies of the data structure to make the code easier to read
	mySocket	=	reqData->socket;
//	myDeviceNum	=	reqData->deviceNumber;

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
	cmdEnumValue	=	FindCmdFromTable(reqData->deviceCommand, gRotatorCmdTable, &cmdType);
	switch(cmdEnumValue)
	{
		//----------------------------------------------------------------------------------------
		//*	Common commands that we want to over ride
		//----------------------------------------------------------------------------------------
		case kCmd_Common_supportedactions:	//*	Returns the list of action names supported by this driver.
			alpacaErrCode	=	Get_SupportedActions(reqData, gRotatorCmdTable);
			break;

		//----------------------------------------------------------------------------------------
		//*	Device specific commands
		//----------------------------------------------------------------------------------------
		case kCmd_Rotator_canreverse:			//*	Indicates whether the Rotator supports the Reverse method.
			alpacaErrCode	=	Get_Canreverse(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Rotator_ismoving:				//*	Indicates whether the focuser is currently moving.
			alpacaErrCode	=	Get_Ismoving(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Rotator_mechanicalposition:	//*	Returns the rotator's mechanical current position.
			alpacaErrCode	=	Get_MechanicalPosition(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Rotator_position:				//*	Returns the focuser's current position.
			alpacaErrCode	=	Get_Position(reqData, alpacaErrMsg, gValueString);
			break;


		case kCmd_Rotator_reverse:				//*	Returns the rotator's Reverse state.
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_Reverse(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_Reverse(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Rotator_stepsize:				//*	Returns the minimum StepSize
			alpacaErrCode	=	Get_Stepsize(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Rotator_targetposition:		//*	Returns the destination position angle.
			alpacaErrCode	=	Get_Targetposition(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Rotator_halt:					//*	Immediately stops rotator motion.
			alpacaErrCode	=	Put_Halt(reqData, alpacaErrMsg);
			break;

		case kCmd_Rotator_move:					//*	Moves the rotator to a new relative position.
			alpacaErrCode	=	Put_Move(reqData, alpacaErrMsg);
			break;

		case kCmd_Rotator_moveabsolute:			//*	Moves the rotator to a new absolute position.
			alpacaErrCode	=	Put_Moveabsolute(reqData, alpacaErrMsg);
			break;

		case kCmd_Rotator_movemechanical:
			alpacaErrCode	=	Put_MoveMechanical(reqData, alpacaErrMsg);
			break;

		case kCmd_Rotator_sync:
			alpacaErrCode	=	Put_Sync(reqData, alpacaErrMsg);
			break;

		//----------------------------------------------------------------------------------
		//*	Extras added by MLS
		case kCmd_Rotator_step:					//*	Moves the rotator to a new relative position.
			alpacaErrCode	=	Put_Step(reqData, alpacaErrMsg);
			break;

		case kCmd_Rotator_stepabsolute:			//*	Moves the rotator to a new absolute position.
			alpacaErrCode	=	Put_Stepabsolute(reqData, alpacaErrMsg);
			break;

		case kCmd_Rotator_readall:
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
TYPE_ASCOM_STATUS	RotatorDriver::Get_Canreverse(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cRotatorProp.CanReverse,
							INCLUDE_COMMA);

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Get_Ismoving(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cRotatorProp.IsMoving	=	IsRotatorMoving();
	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cRotatorProp.IsMoving,
							INCLUDE_COMMA);

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Get_MechanicalPosition(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	UpdateRotorPosition(false);

	JsonResponse_Add_Double(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cRotatorProp.Position,
								INCLUDE_COMMA);

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Get_Position(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	UpdateRotorPosition(false);

	while (cRotatorProp.Position < 0.0)
	{
		cRotatorProp.Position	+=	360.0;
	}
	while (cRotatorProp.Position >= 360.0)
	{
		cRotatorProp.Position	-=	360.0;
	}
	JsonResponse_Add_Double(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cRotatorProp.Position,
								INCLUDE_COMMA);

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Get_Targetposition(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cRotatorProp.TargetPosition,
								INCLUDE_COMMA);

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Get_Reverse(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (cRotatorProp.CanReverse || (cCommonProp.InterfaceVersion >= 3))
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cRotatorProp.Reverse,
								INCLUDE_COMMA);

		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Reverse not implemented");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Put_Reverse(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
bool				foundKeyWord;
char				argumentString[32];

	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Reverse",
											argumentString,
											(sizeof(argumentString) -1),
											false);

	if (cRotatorProp.CanReverse || (cCommonProp.InterfaceVersion >= 3))
	{
		if (foundKeyWord)
		{
			cRotatorProp.Reverse	=	IsTrueFalse(argumentString);
			alpacaErrCode			=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Reverse' not specified");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Reverse not implemented");
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Get_Stepsize(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Double(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cRotatorProp.StepSize,
								INCLUDE_COMMA);

	JsonResponse_Add_String(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Comment",
								"Stepsize in degrees",
								INCLUDE_COMMA);

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Put_Halt(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	HaltMovement();

	//*	update the position and reset the target
	UpdateRotorPosition(true);

	return(alpacaErrCode);
}

//*****************************************************************************
//*	this is move REALITIVE
//*	https://ascom-standards.org/Help/Developer/html/M_ASCOM_DeviceInterface_IRotatorV3_Move.htm
//*	Calling Move causes the TargetPosition property to change to the sum of
//*	the current angular position and the value of the Position parameter (modulo 360 degrees),
//*	then starts rotation to TargetPosition.
//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Put_Move(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				foundKeyWord;
char				argumentString[32];
double				newPositionOffset_deg;

	CONSOLE_DEBUG(__FUNCTION__);

	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Position",
											argumentString,
											(sizeof(argumentString) -1),
											kArgumentIsNumeric);
	if (foundKeyWord)
	{
		newPositionOffset_deg		=	atof(argumentString);

		CONSOLE_DEBUG_W_DBL("newPositionOffset_deg\t=", newPositionOffset_deg);

		cRotatorProp.TargetPosition	+=	newPositionOffset_deg;
		cRotatorProp.TargetPosition	=	AdjustDegrees0_360(cRotatorProp.TargetPosition);
		alpacaErrCode				=	SetCurrentPoisiton_degs(cRotatorProp.TargetPosition);

		LogEvent(	"rotator",
					__FUNCTION__,
					NULL,
					kASCOM_Err_Success,
					argumentString);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Position' not specified");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Put_Moveabsolute(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
bool	foundKeyWord;
char	argumentString[32];
double	newPosition_degs;

	CONSOLE_DEBUG(__FUNCTION__);

	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Position",
											argumentString,
											(sizeof(argumentString) -1),
											kArgumentIsNumeric);
	if (foundKeyWord)
	{
		newPosition_degs		=	atof(argumentString);
		if ((newPosition_degs >= 0.0) && (newPosition_degs < 360.0))
		{
			alpacaErrCode				=	SetCurrentPoisiton_degs(newPosition_degs);
			cRotatorProp.TargetPosition	=	newPosition_degs;

			LogEvent(	"rotator",
						__FUNCTION__,
						NULL,
						kASCOM_Err_Success,
						argumentString);
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Value is out of range");
			CONSOLE_DEBUG(alpacaErrMsg);
			LogEvent(	"rotator",
						alpacaErrMsg,
						NULL,
						kASCOM_Err_Success,
						argumentString);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Position' not specified");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Put_MoveMechanical(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

	return(alpacaErrCode);
}

//*****************************************************************************
//*	https://ascom-standards.org/Help/Developer/html/M_ASCOM_DeviceInterface_IRotatorV3_Sync.htm
//*	Once this method has been called and the sync offset determined,
//*	both the MoveAbsolute(Single) method and the Position property
//*	must function in synced coordinates rather than mechanical coordinates.
//*	The sync offset must persist across driver starts and device reboots.
//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Put_Sync(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
bool				foundKeyWord;
char				argumentString[32];
double				newPosition;
double				newPositionDelta;

	CONSOLE_DEBUG(__FUNCTION__);

	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Position",
											argumentString,
											(sizeof(argumentString) -1));
	if (foundKeyWord)
	{
		newPosition			=	atoi(argumentString);
		CONSOLE_DEBUG_W_DBL("newPosition\t=", newPosition);
		if ((newPosition >= 0.0) && (newPosition < 360.0))
		{
			newPositionDelta	=	newPosition - cRotatorProp.Position;
			alpacaErrCode		=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "'Position' out of range");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Position' not specified");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Put_Step(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				foundKeyWord;
char				argumentString[32];
int32_t				newPositionOffset;
int32_t				newPosition;

	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Position",
											argumentString,
											(sizeof(argumentString) -1));
	if (foundKeyWord)
	{
		//*	its a relative move so get the current position and add
		cRotatorPosition_steps	=	ReadCurrentPoisiton_steps();
		cRotatorProp.Position	=	ReadCurrentPoisiton_degs();

		newPositionOffset	=	atoi(argumentString);
		newPosition			=	cRotatorPosition_steps + newPositionOffset;
		alpacaErrCode		=	SetCurrentPoisiton_steps(newPosition);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Position' not specified");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Put_Stepabsolute(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
bool				foundKeyWord;
char				argumentString[32];
int32_t				newPosition;

	CONSOLE_DEBUG(__FUNCTION__);

	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Position",
											argumentString,
											(sizeof(argumentString) -1));
	if (foundKeyWord)
	{
		newPosition		=	atoi(argumentString);
		alpacaErrCode	=	SetCurrentPoisiton_steps(newPosition);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Position' not specified");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//IsMoving
//MechanicalPosition
//Position
//*****************************************************************************
bool	RotatorDriver::DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen)
{
	DeviceState_Add_Bool(socketFD,	jsonTextBuffer, maxLen,	"IsMoving",				cRotatorProp.IsMoving);
	DeviceState_Add_Int(socketFD,	jsonTextBuffer, maxLen,	"MechanicalPosition",	cRotatorProp.MechanicalPosition);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"Position",				cRotatorProp.Position);

	return(true);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Get_Readall(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	//*	do the common ones first
	Get_Readall_Common(	reqData, alpacaErrMsg);

	Get_Canreverse(			reqData, alpacaErrMsg, "canreverse");
	Get_Ismoving(			reqData, alpacaErrMsg, "ismoving");
	Get_MechanicalPosition(	reqData, alpacaErrMsg, "mechanicalposition");
	Get_Position(			reqData, alpacaErrMsg, "position");
	Get_Reverse(			reqData, alpacaErrMsg, "reverse");
	Get_Stepsize(			reqData, alpacaErrMsg, "stepsize");
	Get_Targetposition(		reqData, alpacaErrMsg, "targetposition");

	strcpy(alpacaErrMsg, "");
	return(alpacaErrCode);
}

//*****************************************************************************
void	RotatorDriver::GetRotatorManufacturer(char *manufactString)
{
	strcpy(manufactString,	cRotatorManufacturer);
}

//*****************************************************************************
void	RotatorDriver::GetRotatorModel(char *modelName)
{
	strcpy(modelName,	cRotatorModel);
}

//*****************************************************************************
void	RotatorDriver::GetRotatorSerialNumber(char *serialNumber)
{
	strcpy(serialNumber,	cRotatorSerialNum);
}

//*****************************************************************************
void	RotatorDriver::UpdateRotorPosition(bool updateTargetPosition)
{
double		currPositionDeg;

	cRotatorPosition_steps			=	ReadCurrentPoisiton_steps();
	currPositionDeg					=	ReadCurrentPoisiton_degs();
	currPositionDeg					=	AdjustDegrees0_360(currPositionDeg);

	cRotatorProp.MechanicalPosition	=	currPositionDeg;
	cRotatorProp.Position			=	currPositionDeg;
	if (updateTargetPosition)
	{
		cRotatorProp.TargetPosition	=	currPositionDeg;
	}
}

//*****************************************************************************
void	RotatorDriver::OutputHTML(TYPE_GetPutRequestData *reqData)
{
int			mySocketFD;
char		lineBuffer[128];

//	CONSOLE_DEBUG(__FUNCTION__);

	mySocketFD		=	reqData->socket;
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H2>AlpacaPi Rotator</H2>\r\n");
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

	OutputHTMLrowData(mySocketFD,	"Rotator",		cCommonProp.Name);
	OutputHTMLrowData(mySocketFD,	"Model",		cRotatorModel);

	//*	rotator position
	cRotatorPosition_steps	=	ReadCurrentPoisiton_steps();
	cRotatorProp.Position	=	ReadCurrentPoisiton_degs();
	sprintf(lineBuffer, "%d steps", cRotatorPosition_steps);
	OutputHTMLrowData(mySocketFD,	"Rotator position",	lineBuffer);

	sprintf(lineBuffer, "%2.1f&deg;", cRotatorProp.Position);
	OutputHTMLrowData(mySocketFD,	"Rotator position",	lineBuffer);

	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<P>\r\n");
}

//*****************************************************************************
bool	RotatorDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gRotatorCmdTable, getPut);
	return(foundIt);
}

//*****************************************************************************
int32_t	RotatorDriver::ReadCurrentPoisiton_steps(void)
{
	//*	this should be overloaded by the hardware implementation
	return(0);
}

//*****************************************************************************
double	RotatorDriver::ReadCurrentPoisiton_degs(void)
{
	//*	this should be overloaded by the hardware implementation
	//*	this is in HARDWARE units, not adjusted
	return(0);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::SetCurrentPoisiton_steps(const int32_t newPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	//*	this should be overloaded by the hardware implementation
	alpacaErrCode	=	kASCOM_Err_NotImplemented;
//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented at this layer");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::SetCurrentPoisiton_degs(const double newPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	//*	this should be overloaded by the hardware implementation
	alpacaErrCode	=	kASCOM_Err_NotImplemented;
//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented at this layer");
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::HaltMovement(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;


	return(alpacaErrCode);
}

//*****************************************************************************
bool	RotatorDriver::IsRotatorMoving(void)
{
	//*	this should be overloaded by the hardware implementation
	return(false);
}

//*****************************************************************************
void	RotatorDriver::DumpRotatorProperties(const char *callingFunctionName)
{
	DumpCommonProperties(callingFunctionName);
	CONSOLE_DEBUG(			"------------------------------------");
	CONSOLE_DEBUG_W_BOOL(	"cRotatorProp.CanReverse        \t=",	cRotatorProp.CanReverse);
	CONSOLE_DEBUG_W_BOOL(	"cRotatorProp.IsMoving          \t=",	cRotatorProp.IsMoving);
	CONSOLE_DEBUG_W_DBL(	"cRotatorProp.MechanicalPosition\t=",	cRotatorProp.MechanicalPosition);
	CONSOLE_DEBUG_W_DBL(	"cRotatorProp.Position          \t=",	cRotatorProp.Position);
	CONSOLE_DEBUG_W_BOOL(	"cRotatorProp.Reverse           \t=",	cRotatorProp.Reverse);
	CONSOLE_DEBUG_W_DBL(	"cRotatorProp.StepSize          \t=",	cRotatorProp.StepSize);
	CONSOLE_DEBUG_W_DBL(	"cRotatorProp.SyncOffset          \t=",	cRotatorProp.SyncOffset);
	CONSOLE_DEBUG_W_DBL(	"cRotatorProp.TargetPosition    \t=",	cRotatorProp.TargetPosition);
	CONSOLE_DEBUG(			"*************************************************************");
}


#endif // _ENABLE_ROTATOR_
