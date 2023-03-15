//*****************************************************************************
//*
//*	Name:			domeshutter.cpp
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:	Code to talk to shutter from dome driver
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
//*	Apr 30,	2020	<MLS> Created domeshutter.cpp
//*	Apr 30,	2020	<MLS> Added GetShutterStatus()
//*	May  8,	2020	<MLS> Added altitude processing
//*	Dec  8,	2020	<MLS> Added OpenRemoteShutter() & CloseRemoteShutter()
//*	Dec  8,	2020	<MLS> Remote Shutter open/close working
//*	Feb 25,	2021	<MLS> Added StopRemoteShutter()
//*	Mar  2,	2023	<MLS> Added _ENABLE_DOME_SHUTTER_
//*****************************************************************************



#ifdef _ENABLE_DOME_SHUTTER_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<netinet/in.h>
#include	<errno.h>
#include	<pthread.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"

#include	"RequestData.h"
#include	"json_parse.h"
#include	"discoverythread.h"
#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"domedriver.h"

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::OpenRemoteShutter(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _ENABLE_REMOTE_SHUTTER_
	if (cShutterInfoValid)
	{
	SJP_Parser_t		jsonParser;
	bool				validData;
	char				alpacaString[128];
	int					jjj;
	int					alpacaReturnCode;

		CONSOLE_DEBUG("Using remote shutter info");
		SJP_Init(&jsonParser);
		sprintf(alpacaString,	"/api/v1/%s/%d/openshutter", "shutter", 0);
		validData	=	SendPutCommand(	&cShutterDeviceAddress,
											cShutterPort,
											alpacaString,
											NULL,
											&jsonParser);
		if (validData)
		{
			jjj	=	0;
			while (jjj<jsonParser.tokenCount_Data)
			{
				CONSOLE_DEBUG_W_STR(jsonParser.dataList[jjj].keyword, jsonParser.dataList[jjj].valueString);
				if (strcasecmp(jsonParser.dataList[jjj].keyword, "ERRORNUMBER") == 0)
				{
					alpacaReturnCode	=	atoi(jsonParser.dataList[jjj].valueString);
					if (alpacaReturnCode == 0)
					{
						alpacaErrCode	=	kASCOM_Err_Success;
					}
					else
					{
						alpacaErrCode	=	(TYPE_ASCOM_STATUS)alpacaReturnCode;
					}
				}
				jjj++;
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Remote shutter not detected");
	}
#else

	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Remote shutter enabled");
	}
#endif	// _ENABLE_REMOTE_SHUTTER_

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::CloseRemoteShutter(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _ENABLE_REMOTE_SHUTTER_
	if (cShutterInfoValid)
	{
	SJP_Parser_t		jsonParser;
	bool				validData;
	char				alpacaString[128];
	int					jjj;
	int					alpacaReturnCode;

		CONSOLE_DEBUG("Using remote shutter info");
		SJP_Init(&jsonParser);
		sprintf(alpacaString,	"/api/v1/%s/%d/closeshutter", "shutter", 0);
		validData	=	SendPutCommand(	&cShutterDeviceAddress,
											cShutterPort,
											alpacaString,
											NULL,
											&jsonParser);
		if (validData)
		{
			jjj	=	0;
			while (jjj<jsonParser.tokenCount_Data)
			{
				CONSOLE_DEBUG_W_STR(jsonParser.dataList[jjj].keyword, jsonParser.dataList[jjj].valueString);
				if (strcasecmp(jsonParser.dataList[jjj].keyword, "ERRORNUMBER") == 0)
				{
					alpacaReturnCode	=	atoi(jsonParser.dataList[jjj].valueString);
					if (alpacaReturnCode == 0)
					{
						alpacaErrCode	=	kASCOM_Err_Success;
					}
					else
					{
						alpacaErrCode	=	(TYPE_ASCOM_STATUS)alpacaReturnCode;
					}
				}
				jjj++;
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Remote shutter not detected");
	}
#else

	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Remote shutter enabled");
	}
#endif	// _ENABLE_REMOTE_SHUTTER_

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::StopRemoteShutter(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _ENABLE_REMOTE_SHUTTER_
	if (cShutterInfoValid)
	{
	SJP_Parser_t		jsonParser;
	bool				validData;
	char				alpacaString[128];
	int					jjj;
	int					alpacaReturnCode;

		CONSOLE_DEBUG("Using remote shutter info");
		SJP_Init(&jsonParser);
		sprintf(alpacaString,	"/api/v1/%s/%d/abortslew", "shutter", 0);
		validData	=	SendPutCommand(	&cShutterDeviceAddress,
											cShutterPort,
											alpacaString,
											NULL,
											&jsonParser);
		if (validData)
		{
			jjj	=	0;
			while (jjj<jsonParser.tokenCount_Data)
			{
				CONSOLE_DEBUG_W_STR(jsonParser.dataList[jjj].keyword, jsonParser.dataList[jjj].valueString);
				if (strcasecmp(jsonParser.dataList[jjj].keyword, "ERRORNUMBER") == 0)
				{
					alpacaReturnCode	=	atoi(jsonParser.dataList[jjj].valueString);
					if (alpacaReturnCode == 0)
					{
						alpacaErrCode	=	kASCOM_Err_Success;
					}
					else
					{
						alpacaErrCode	=	(TYPE_ASCOM_STATUS)alpacaReturnCode;
					}
				}
				jjj++;
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Remote shutter not detected");
	}
#else

	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Remote shutter enabled");
	}
#endif	// _ENABLE_REMOTE_SHUTTER_

	return(alpacaErrCode);
}


//*****************************************************************************
void	DomeDriver::GetRemoteShutterStatus(void)
{
struct sockaddr_in	shutterAddr;
int					shutterPort;
SJP_Parser_t		jsonParser;
bool				validData;
char				alpacaString[128];
int					jjj;

//	CONSOLE_DEBUG(__FUNCTION__);
	memset(&shutterAddr, 0, sizeof(struct sockaddr_in));
#ifdef _ENABLE_REMOTE_SHUTTER_
	if (cShutterInfoValid)
	{
//		CONSOLE_DEBUG("Using remote shutter info");
		shutterAddr	=	cShutterDeviceAddress;
		shutterPort	=	cShutterPort;

		//===============================================================
		//*	get readall
		SJP_Init(&jsonParser);
		sprintf(alpacaString,	"/api/v1/%s/%d/readall", "shutter", 0);
		validData	=	GetJsonResponse(	&shutterAddr,
											shutterPort,
											alpacaString,
											NULL,
											&jsonParser);
		if (validData)
		{
			jjj	=	0;
			while (jjj<jsonParser.tokenCount_Data)
			{
				if (strcasecmp(jsonParser.dataList[jjj].keyword, "SHUTTERSTATUS") == 0)
				{
					cDomeProp.ShutterStatus	=	atoi(jsonParser.dataList[jjj].valueString);
				}
				else if (strcasecmp(jsonParser.dataList[jjj].keyword, "altitude") == 0)
				{
					cDomeProp.Altitude	=	atof(jsonParser.dataList[jjj].valueString);
				}
				jjj++;
			}
		}
		else
		{
			CONSOLE_DEBUG("Read failure - readall");
		}
	}
	else
	{
		CONSOLE_DEBUG("No shutter to talk to");
	}
#endif

}


#endif // _ENABLE_DOME_SHUTTER_
