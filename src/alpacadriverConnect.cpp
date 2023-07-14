//**************************************************************************
//*	Name:			alpacadriverConnect.cpp
//*
//*	Author:			Mark Sproul (C) 2023
//*					msproul@skychariot.com
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
//*	Re-distribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul msproul@skychariot.com
//*****************************************************************************
//*	Jun 18,	2023	<MLS> Created alpacadriverConnect.cpp
//*	Jun 18,	2023	<MLS> Added Put_Connect(), Put_Disconnect(), Get_Connecting()
//*****************************************************************************

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"JsonResponse.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"eventlogging.h"


//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_Connected(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cCommonProp.Connected,
							INCLUDE_COMMA);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Put_Connected(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				foundKeyWord;
bool				validTruFalseArg;
bool				connectFlag;
char				argumentString[32];

//	CONSOLE_DEBUG(__FUNCTION__);
	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Connected",
											argumentString,
											(sizeof(argumentString) -1));
	if (foundKeyWord)
	{
		CONSOLE_DEBUG_W_STR(__FUNCTION__, argumentString);
		validTruFalseArg	=	IsTrueFalseArgValid(argumentString);
		if (validTruFalseArg)
		{
			connectFlag			=	IsTrueFalse(argumentString);
			if (connectFlag)
			{
				AlpacaConnect();
				LogEvent(	reqData->deviceType,
							"Connect",
							NULL,
							kASCOM_Err_Success,
							"");
				cCommonProp.Connected		=	true;
			}
			else
			{
				AlpacaDisConnect();
				LogEvent(	reqData->deviceType,
							"Dis-Connect",
							NULL,
							kASCOM_Err_Success,
							"");
				cCommonProp.Connected		=	false;
			}
		}
		else
		{
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "true/false not specified");
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			CONSOLE_DEBUG(alpacaErrMsg);
			DumpRequestStructure(__FUNCTION__, reqData);
		}
	}
	else
	{
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Keyword 'Connected' not found");
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		CONSOLE_DEBUG(alpacaErrMsg);
		DumpRequestStructure(__FUNCTION__, reqData);
	}

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							gValueString,
							true,
							INCLUDE_COMMA);

	return(alpacaErrCode);
}

//*****************************************************************************
bool	AlpacaDriver::AlpacaConnect(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	return(true);
}

//*****************************************************************************
bool	AlpacaDriver::AlpacaDisConnect(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	return(false);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_Connecting(	TYPE_GetPutRequestData	*reqData,
													char					*alpacaErrMsg,
													const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	JsonResponse_Add_Bool(	reqData->socket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							responseString,
							cCommonProp.Connecting,
							INCLUDE_COMMA);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Put_Connect(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
bool				isConnected;

	isConnected				=	AlpacaConnect();
	if (isConnected)
	{
		cCommonProp.Connected	=	true;
		alpacaErrCode			=	kASCOM_Err_Success;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Put_Disconnect(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
bool				disconnectOK;

	disconnectOK				=	AlpacaDisConnect();
	if (disconnectOK)
	{
		cCommonProp.Connected	=	false;
		alpacaErrCode			=	kASCOM_Err_Success;
	}
	return(alpacaErrCode);
}


