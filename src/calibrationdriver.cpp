//**************************************************************************
//*	Name:			calibrationdriver.cpp
//*
//*	Author:			Mark Sproul (C) 2020
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*	Usage notes:	This driver does not implement any actual device,
//*					you must create a sub-class that does the actual control
//*
//*	References:		https://ascom-standards.org/api/
//*					https://ascom-standards.org/Help/Developer/html/N_ASCOM_DeviceInterface.htm
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Sep  1,	2020	<MLS> Created calibrationdriver.cpp
//*	Oct 21,	2020	<MLS> Finished GetCmdNameFromMyCmdTable() for calibrationdriver
//*	Feb 12,	2021	<MLS> Added GetCalibrationStateString()
//*	Apr  6,	2021	<MLS> CONFORM-covercalibrator -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	Dec 16,	2021	<MLS> Added WatchDog_TimeOut() to calibrationdriver
//*****************************************************************************


#ifdef _ENABLE_CALIBRATION_

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
#include	"eventlogging.h"

#include	"alpacadriver.h"
#include	"calibrationdriver.h"


//*****************************************************************************
//calibration
//*****************************************************************************
enum
{
	kCmd_Calibration_brightness,			//*	Returns the current calibrator brightness
	kCmd_Calibration_calibratorstate,		//*	Returns the state of the calibration device
	kCmd_Calibration_coverstate,			//*	Returns the state of the device cover"
	kCmd_Calibration_maxbrightness,			//*	Returns the calibrator's maximum Brightness value.
	kCmd_Calibration_calibratoroff,			//*	Turns the calibrator off
	kCmd_Calibration_calibratoron,			//*	Turns the calibrator on at the specified brightness
	kCmd_Calibration_closecover,			//*	Initiates cover closing
	kCmd_Calibration_haltcover,				//*	Stops any cover movement that may be in progress
	kCmd_Calibration_opencover,				//*	Initiates cover opening


	//*	added by MLS
	kCmd_Calibration_Extras,
	kCmd_Calibration_readall,

	kCmd_Calibration_last
};

//*****************************************************************************
const TYPE_CmdEntry	gCalibrationCmdTable[]	=
{


	{	"brightness",		kCmd_Calibration_brightness,		kCmdType_GET	},	//*	Returns the current calibrator brightness
	{	"calibratorstate",	kCmd_Calibration_calibratorstate,	kCmdType_GET	},	//*	Returns the state of the calibration device
	{	"coverstate",		kCmd_Calibration_coverstate,		kCmdType_GET	},	//*	Returns the state of the device cover"
	{	"maxbrightness",	kCmd_Calibration_maxbrightness,		kCmdType_GET	},	//*	Returns the calibrator's maximum Brightness value.
	{	"calibratoroff",	kCmd_Calibration_calibratoroff,		kCmdType_PUT	},	//*	Turns the calibrator off
	{	"calibratoron",		kCmd_Calibration_calibratoron,		kCmdType_PUT	},	//*	Turns the calibrator on at the specified brightness
	{	"closecover",		kCmd_Calibration_closecover,		kCmdType_PUT	},	//*	Initiates cover closing
	{	"haltcover",		kCmd_Calibration_haltcover,			kCmdType_PUT	},	//*	Stops any cover movement that may be in progress
	{	"opencover",		kCmd_Calibration_opencover,			kCmdType_PUT	},	//*	Initiates cover opening

	//*	added by MLS
	{	"--extras",			kCmd_Calibration_Extras,			kCmdType_GET	},
	{	"readall",			kCmd_Calibration_readall,			kCmdType_GET	},

	{	"",						-1,	0x00	}
};

//**************************************************************************************
void	CreateCalibrationObjects(void)
{
	new CalibrationDriver();
}


//**************************************************************************************
CalibrationDriver::CalibrationDriver(void)
	:AlpacaDriver(kDeviceType_CoverCalibrator)
{

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name, "CoverCalibrator");

	memset(&cCoverCalibrationProp, 0, sizeof(TYPE_CoverCalibrationProperties));

	cCoverCalibrationProp.Brightness		=	0;
	cCoverCalibrationProp.CalibratorState	=	kCalibrator_NotPresent;
	cCoverCalibrationProp.CoverState		=	kCover_NotPresent;
	cCoverCalibrationProp.MaxBrightness		=	1;
}

//**************************************************************************************
// Destructor
//**************************************************************************************
CalibrationDriver::~CalibrationDriver(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
char				alpacaErrMsg[256];
int					cmdEnumValue;
int					cmdType;
int					mySocket;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, reqData->deviceCommand);
#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
#endif // _DEBUG_CONFORM_

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
	cmdEnumValue	=	FindCmdFromTable(reqData->deviceCommand, gCalibrationCmdTable, &cmdType);
	switch(cmdEnumValue)
	{
		//----------------------------------------------------------------------------------------
		//*	Common commands that we want to over ride
		//----------------------------------------------------------------------------------------
		case kCmd_Common_supportedactions:	//*	Returns the list of action names supported by this driver.
			alpacaErrCode	=	Get_SupportedActions(reqData, gCalibrationCmdTable);
			break;

		//----------------------------------------------------------------------------------------
		//*	Device specific commands
		//----------------------------------------------------------------------------------------
		case kCmd_Calibration_brightness:
			alpacaErrCode	=	Get_Brightness(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Calibration_calibratorstate:
			alpacaErrCode	=	Get_Calibratorstate(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Calibration_coverstate:
			alpacaErrCode	=	Get_Coverstate(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Calibration_maxbrightness:
			alpacaErrCode	=	Get_Maxbrightness(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Calibration_calibratoroff:	//*	Turns the calibrator off
			alpacaErrCode	=	Put_CalibratorOff(reqData, alpacaErrMsg);
			break;

		case kCmd_Calibration_calibratoron:		//*	Turns the calibrator on at the specified brightness
			alpacaErrCode	=	Put_CalibratorOn(reqData, alpacaErrMsg);
			break;

		case kCmd_Calibration_closecover:		//*	Initiates cover closing
			alpacaErrCode	=	Put_CloseCover(reqData, alpacaErrMsg);
			break;

		case kCmd_Calibration_haltcover:		//*	Stops any cover movement that may be in progress
			alpacaErrCode	=	Put_HaltCover(reqData, alpacaErrMsg);
			break;

		case kCmd_Calibration_opencover:		//*	Initiates cover opening
			alpacaErrCode	=	Put_OpenCover(reqData, alpacaErrMsg);
			break;

		case kCmd_Calibration_readall:
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
								kInclude_HTTP_Header);

	//*	this is for the logging function
	strcpy(reqData->alpacaErrMsg, alpacaErrMsg);
	return(alpacaErrCode);
}

//*****************************************************************************
bool	CalibrationDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gCalibrationCmdTable, getPut);
	return(foundIt);
}

//*****************************************************************************
void	CalibrationDriver::WatchDog_TimeOut(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				alpacaErrMsg[64];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cCommonProp.Name);

	if (cCoverCalibrationProp.Brightness > 0)
	{
		LogEvent("CalibratorStatus",	"Turning off due to Timeout",	NULL,	kASCOM_Err_Success,	"");
		alpacaErrCode	=	Calibrator_TurnOff(alpacaErrMsg);
	}
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::Get_Brightness(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
//		CONSOLE_DEBUG_W_NUM("cCoverCalibrationProp.Brightness\t=",	cCoverCalibrationProp.Brightness);
		JsonResponse_Add_Int32(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCoverCalibrationProp.Brightness,
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
static void	GetCalibrationStateString(CalibratorStatus calibState, char *stateString)
{
	switch(calibState)
	{
		case kCalibrator_NotPresent:	strcpy(stateString, "This device does not have a calibration capability");	break;
		case kCalibrator_Off:			strcpy(stateString, "The calibrator is off");	break;
		case kCalibrator_NotReady:		strcpy(stateString, "The calibrator is stabilizing or is not yet in the commanded state");	break;
		case kCalibrator_Ready:			strcpy(stateString, "The calibrator is ready for use");	break;
		case kCalibrator_Unknown:		strcpy(stateString, "The calibrator state is unknown");	break;
		case kCalibrator_Error:			strcpy(stateString, "The calibrator encountered an error when changing state");	break;
	}
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::Get_Calibratorstate(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode;
char					calibrationStateStr[128];

//	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		JsonResponse_Add_Int32(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCoverCalibrationProp.CalibratorState,
								INCLUDE_COMMA);

		GetCalibrationStateString(cCoverCalibrationProp.CalibratorState, calibrationStateStr);
		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"calibratorstate-str",
								calibrationStateStr,
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
TYPE_ASCOM_STATUS	CalibrationDriver::Get_Coverstate(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		JsonResponse_Add_Int32(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCoverCalibrationProp.CoverState,
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
TYPE_ASCOM_STATUS	CalibrationDriver::Get_Maxbrightness(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		JsonResponse_Add_Int32(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCoverCalibrationProp.MaxBrightness,
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
TYPE_ASCOM_STATUS	CalibrationDriver::Put_CalibratorOn(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
bool				brightnessFound;
char				brightnessString[32];
int					brightnessValue;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCoverCalibrationProp.CalibratorState == kCalibrator_NotPresent)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Calibrator not present");
	}
	else
	{
		//*	we have to find the "Brightness" string
		brightnessFound		=	GetKeyWordArgument(	reqData->contentData,
													"Brightness",
													brightnessString,
													(sizeof(brightnessString) -1),
													kArgumentIsNumeric);

		if (brightnessFound)
		{
			brightnessValue	=	atoi(brightnessString);
//			CONSOLE_DEBUG_W_NUM("new brightnessValue\t=", brightnessValue);

			alpacaErrCode	=	Calibrator_TurnOn(brightnessValue, alpacaErrMsg);
			cCoverCalibrationProp.CalibratorState	=	kCalibrator_Ready;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_ValueNotSet;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Brightness not specified");
		}
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::Put_CalibratorOff(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCoverCalibrationProp.CalibratorState == kCalibrator_NotPresent)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Calibrator not present");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else
	{
		cCoverCalibrationProp.CalibratorState	=	kCalibrator_Off;
		alpacaErrCode	=	Calibrator_TurnOff(alpacaErrMsg);
	}
	return(alpacaErrCode);
}



//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::Put_CloseCover(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCoverCalibrationProp.CoverState == kCover_NotPresent)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Cover not present");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	Cover_Close(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::Put_HaltCover(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cCoverCalibrationProp.CoverState == kCover_NotPresent)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Cover not present");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	Cover_Halt(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::Put_OpenCover(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cCoverCalibrationProp.CoverState == kCover_NotPresent)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Cover not present");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	Cover_Open(alpacaErrMsg);
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::Get_Readall(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;
int		mySocket;

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


		alpacaErrCode	=	Get_Brightness(		reqData, alpacaErrMsg, "brightness");
		alpacaErrCode	=	Get_Calibratorstate(reqData, alpacaErrMsg, "calibratorstate");
		alpacaErrCode	=	Get_Coverstate(		reqData, alpacaErrMsg, "coverstate");
		alpacaErrCode	=	Get_Maxbrightness(	reqData, alpacaErrMsg, "maxbrightness");


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
void	CalibrationDriver::OutputHTML(TYPE_GetPutRequestData *reqData)
{
int		mySocketFD;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		mySocketFD	=	reqData->socket;

		mySocketFD		=	reqData->socket;
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");

		SocketWriteData(mySocketFD,	"<H2>Calibration/Cover</H2>\r\n");


		SocketWriteData(mySocketFD,	"</CENTER>\r\n");


		GenerateHTMLcmdLinkTable(mySocketFD, "covercalibrator", cDeviceNum, gCalibrationCmdTable);
	}
}

//*****************************************************************************
//*	returns delay time in micro-seconds
//*****************************************************************************
int32_t	CalibrationDriver::RunStateMachine(void)
{
	return(5 * 1000 * 1000);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::Calibrator_TurnOn(const int brightnessValue, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be over-ridden");

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	CONSOLE_DEBUG(alpacaErrMsg);
	return(alpacaErrCode);

}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::Calibrator_TurnOff(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be over-ridden");
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	CONSOLE_DEBUG(alpacaErrMsg);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::Cover_Open(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be over-ridden");
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	CONSOLE_DEBUG(alpacaErrMsg);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::Cover_Close(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be over-ridden");
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	CONSOLE_DEBUG(alpacaErrMsg);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriver::Cover_Halt(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be over-ridden");
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	CONSOLE_DEBUG(alpacaErrMsg);
	return(alpacaErrCode);
}


#endif	//	_ENABLE_CALIBRATION_
