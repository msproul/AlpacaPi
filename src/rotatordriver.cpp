//**************************************************************************
//*	Name:			rotatordriver.cpp
//*
//*	Author:			Mark Sproul (C) 2019, 2020
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
//*****************************************************************************

#ifdef _ENABLE_ROTATOR_

#include	<stdlib.h>
#include	<string.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"
#include	"eventlogging.h"


#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"rotatordriver.h"

#include	"JsonResponse.h"


//*****************************************************************************
TYPE_CmdEntry	gRotatorCmdTable[]	=
{

	{	"canreverse",			kCmd_Rotator_canreverse,		kCmdType_GET	},
	{	"ismoving",				kCmd_Rotator_ismoving,			kCmdType_GET	},
	{	"position",				kCmd_Rotator_position,			kCmdType_GET	},
	{	"reverse",				kCmd_Rotator_reverse,			kCmdType_BOTH	},
	{	"stepsize",				kCmd_Rotator_stepsize,			kCmdType_GET	},
	{	"targetposition",		kCmd_Rotator_targetposition,	kCmdType_GET	},
	{	"halt",					kCmd_Rotator_halt,				kCmdType_PUT	},
	{	"move",					kCmd_Rotator_move,				kCmdType_PUT	},
	{	"moveabsolute",			kCmd_Rotator_moveabsolute,		kCmdType_PUT	},

	//*	added by MLS
	{	"--extras",				kCmd_Rotator_Extras,			kCmdType_GET	},
	{	"step",					kCmd_Rotator_step,				kCmdType_PUT	},
	{	"stepabsolute",			kCmd_Rotator_stepabsolute,		kCmdType_PUT	},
	{	"readall",				kCmd_Rotator_readall,			kCmdType_GET	},

	{	"",						-1,	0x00	}
};


//**************************************************************************************
RotatorDriver::RotatorDriver(const int argDevNum)
	:AlpacaDriver(kDeviceType_Rotator)
{

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cDeviceName, "Rotator");

	memset(&cRotatorProp, 0, sizeof(TYPE_RotatorProperties));


	cRotatorManufacturer[0]	=	0;
	cRotatorModel[0]		=	0;
	cRotatorSerialNum[0]	=	0;

	cRotatorProp.CanReverse		=	false;		//*	True if the rotation and angular direction must be reversed for the optics
	cRotatorProp.Reverse		=	false;
	cRotatorProp.IsMoving		=	false;
	cRotatorProp.StepSize		=	1.0;

	cRotatorStepsPerRev			=	360;
	cRotatorPos_step			=	0;
	cRotatorPos_degs			=	0.0;
	cRotatorTrgtPos_degs		=	0.0;
}


//**************************************************************************************
// Destructor
//**************************************************************************************
RotatorDriver::~RotatorDriver(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
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
TYPE_ASCOM_STATUS	RotatorDriver::Get_Canreverse(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cRotatorProp.CanReverse,
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
TYPE_ASCOM_STATUS	RotatorDriver::Get_Ismoving(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		cRotatorProp.IsMoving	=	IsRotatorMoving();
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cRotatorProp.IsMoving,
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
TYPE_ASCOM_STATUS	RotatorDriver::Get_Position(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		cRotatorPos_step	=	ReadCurrentPoisiton_steps();
		cRotatorPos_degs	=	ReadCurrentPoisiton_degs();

		while (cRotatorPos_degs < 0.0)
		{
			cRotatorPos_degs	+=	360.0;
		}
		while (cRotatorPos_degs >= 360.0)
		{
			cRotatorPos_degs	-=	360.0;
		}
		JsonResponse_Add_Double(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									responseString,
									cRotatorPos_degs,
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
TYPE_ASCOM_STATUS	RotatorDriver::Get_Reverse(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cRotatorReverseState,
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
TYPE_ASCOM_STATUS	RotatorDriver::Put_Reverse(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Command not implemented");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Get_Stepsize(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
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
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Get_Targetposition(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Double(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									responseString,
									cRotatorTrgtPos_degs,
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
TYPE_ASCOM_STATUS	RotatorDriver::Put_Halt(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	HaltMovement();

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Put_Move(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				foundKeyWord;
char				argumentString[32];
double				newPositionOffset_deg;
double				newPosition_degs;

	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
												"Position",
												argumentString,
												31);
		if (foundKeyWord)
		{
			newPositionOffset_deg	=	atof(argumentString);


			//*	its a relative move so get the current position and add
			cRotatorPos_degs		=	ReadCurrentPoisiton_degs();

			newPosition_degs		=	cRotatorPos_degs + newPositionOffset_deg;
			if ((newPosition_degs >= 0.0) && (newPosition_degs < 360.0))
			{
				alpacaErrCode			=	SetCurrentPoisiton_degs(newPosition_degs);
				cRotatorTrgtPos_degs	=	newPosition_degs;

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
			}
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
TYPE_ASCOM_STATUS	RotatorDriver::Put_Moveabsolute(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
bool	foundKeyWord;
char	argumentString[32];
double	newPosition_degs;

	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		foundKeyWord	=	GetKeyWordArgument(reqData->contentData, "Position", argumentString, 31);
		if (foundKeyWord)
		{
			newPosition_degs		=	atof(argumentString);
			if ((newPosition_degs >= 0.0) && (newPosition_degs < 360.0))
			{
				alpacaErrCode			=	SetCurrentPoisiton_degs(newPosition_degs);
				cRotatorTrgtPos_degs	=	newPosition_degs;

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
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
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

	if (reqData != NULL)
	{
		foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
												"Position",
												argumentString,
												31);
		if (foundKeyWord)
		{
			//*	its a relative move so get the current position and add
			cRotatorPos_step	=	ReadCurrentPoisiton_steps();
			cRotatorPos_degs	=	ReadCurrentPoisiton_degs();

			newPositionOffset	=	atoi(argumentString);
			newPosition			=	cRotatorPos_step + newPositionOffset;
			alpacaErrCode		=	SetCurrentPoisiton_steps(newPosition);
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
TYPE_ASCOM_STATUS	RotatorDriver::Put_Stepabsolute(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
bool				foundKeyWord;
char				argumentString[32];
int32_t				newPosition;

	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		foundKeyWord	=	GetKeyWordArgument(reqData->contentData, "Position", argumentString, 31);
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
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver::Get_Readall(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		//*	do the common ones first
		Get_Readall_Common(	reqData, alpacaErrMsg);

		Get_Canreverse(		reqData, alpacaErrMsg, "canreverse");
		Get_Ismoving(		reqData, alpacaErrMsg, "ismoving");
		Get_Position(		reqData, alpacaErrMsg, "position");
		Get_Reverse(		reqData, alpacaErrMsg, "reverse");
		Get_Stepsize(		reqData, alpacaErrMsg, "stepsize");
		Get_Targetposition(	reqData, alpacaErrMsg, "targetposition");

		strcpy(alpacaErrMsg, "");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
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
void	RotatorDriver::OutputHTML(TYPE_GetPutRequestData *reqData)
{
int			mySocketFD;
char		lineBuffer[128];

	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		mySocketFD		=	reqData->socket;
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");



		OutputHTMLrowData(mySocketFD,	"Rotator",		cDeviceName);
		OutputHTMLrowData(mySocketFD,	"Model",		cRotatorModel);

		//*	rotator position
		cRotatorPos_step	=	ReadCurrentPoisiton_steps();
		cRotatorPos_degs	=	ReadCurrentPoisiton_degs();
		sprintf(lineBuffer, "%d steps", cRotatorPos_step);
		OutputHTMLrowData(mySocketFD,	"Rotator position",	lineBuffer);

		sprintf(lineBuffer, "%2.1f&deg;", cRotatorPos_degs);
		OutputHTMLrowData(mySocketFD,	"Rotator position",	lineBuffer);

		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<P>\r\n");

		//*	now generate links to all of the commands
		GenerateHTMLcmdLinkTable(mySocketFD, "rotator", 0, gRotatorCmdTable);
	}
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

	//*	this should be overloaded by the hardware implementation
	alpacaErrCode	=	kASCOM_Err_NotImplemented;
//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented at this layer");
	return(alpacaErrCode);
}

//*****************************************************************************
bool	RotatorDriver::IsRotatorMoving(void)
{
	//*	this should be overloaded by the hardware implementation
	return(false);
}


#endif // _ENABLE_ROTATOR_
