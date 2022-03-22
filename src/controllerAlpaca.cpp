//*****************************************************************************
//*		controllerAlpaca.cpp		(c) 2020 by Mark Sproul
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
//*	Mar 17,	2020	<MLS> Created controllerAlpaca.cpp
//*	Mar 17,	2020	<MLS> Added AlpacaGetSupportedActions()
//*	Mar 17,	2020	<MLS> Added AlpacaProcessSupportedActions()
//*	Mar 17,	2020	<MLS> Added AlpacaCheckForErrors()
//*	Mar 19,	2020	<MLS> Added ClientID and ClientTransactionID to data request
//*	Apr 15,	2020	<MLS> Fixed dataString bug in AlpacaGet... routines
//*	Jun 22,	2020	<MLS> Added AlpacaGetIntegerArray()
//*	Jun 24,	2020	<MLS> Added UpdateDownloadProgress()
//*	Jan  9,	2021	<MLS> Added new version of AlpacaGetStatus_ReadAll()
//*	Jan  9,	2021	<MLS> Added new version of AlpacaGetSupportedActions()
//*	Jan 10,	2021	<MLS> Added new version of AlpacaSendPutCmdwResponse()
//*	Jan 12,	2021	<MLS> Added AlpacaGetStringValue()
//*	Jan 30,	2021	<MLS> Added AlpacaGetImageArray()
//*	Jan 31,	2021	<MLS> Finished debugging AlpacaGetImageArray()
//*	Feb  9,	2021	<MLS> Added AlpacaGetCommonProperties_OneAAT()
//*	Feb 13,	2021	<MLS> Added UpdateCommonProperties()
//*	Sep  8,	2021	<MLS> Added AlpacaSetConnected()
//*	Dec 14,	2021	<MLS> Started working on support for imageBinary
//*	Dec 15,	2021	<MLS> AlpacaGetImageArray() binary working with 16 bit monochrome
//*	Dec 16,	2021	<MLS> AlpacaGetImageArray() binary working with 16 bit Color (RGB)
//*****************************************************************************

#ifdef _CONTROLLER_USES_ALPACA_

#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>
#include	<errno.h>



#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#define	_DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"widget.h"
#include	"controller.h"

static int	gClientID				=	1;
static int	gClientTransactionID	=	1;


//*****************************************************************************
//*	this should be over-ridden
//*****************************************************************************
bool	Controller::AlpacaGetStartupData(void)
{
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
//curl -X PUT "https://virtserver.swaggerhub.com/ASCOMInitiative/api/v1/telescope/0/connected"
//		-H  "accept: application/json"
//		-H  "Content-Type: application/x-www-form-urlencoded"
//		-d "Connected=true&ClientID=11&ClientTransactionID=22"
//*****************************************************************************
bool	Controller::AlpacaSetConnected(const char *deviceTypeStr, const bool newConnectedState)
{
bool			validData;
char			dataString[128];
SJP_Parser_t	jsonParser;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);


	CONSOLE_DEBUG(__FUNCTION__);
	sprintf(dataString,		"Connected=%s", (newConnectedState ? "true" : "false"));
	CONSOLE_DEBUG_W_STR("deviceTypeStr\t=", deviceTypeStr);
	CONSOLE_DEBUG_W_STR("dataString\t=", dataString);
	validData	=	AlpacaSendPutCmdwResponse(	deviceTypeStr,
												"connected",
												dataString,
												&jsonParser);

	if (validData == false)
	{
		CONSOLE_DEBUG("CONNECT failed");
	}
//	SJP_DumpJsonData(&jsonParser);
//	CONSOLE_ABORT(__FUNCTION__);

	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaGetCommonConnectedState(const char *deviceTypeStr)
{
bool	validData;
bool	myConnectedFlag;
bool	prevConnectedState;
bool	returnedValid;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceTypeStr);
	validData		=	false;

	//-----------------------------------------------------------------------------------------
//	CONSOLE_DEBUG_W_STR("deviceTypeStr=", deviceTypeStr);
	validData	=	AlpacaGetBooleanValue(	deviceTypeStr,
											"connected",
											NULL,
											&myConnectedFlag,
											&returnedValid,
											false);
	if (validData)
	{
	//	CONSOLE_DEBUG(__FUNCTION__);
		//*	keep track of the previous state
		prevConnectedState		=	cCommonProp.Connected;
		cCommonProp.Connected	=	myConnectedFlag;
//		CONSOLE_DEBUG_W_NUM("cCommonProp.Connected\t=", cCommonProp.Connected);
		if (cCommonProp.Connected && (prevConnectedState == false))
		{
			//*	if we are going from un-connected to connected, read the start up data
			cReadStartup	=	true;
		}
	}
	else
	{
		CONSOLE_DEBUG("Failed to get connected status");
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	return(validData);
}

//*****************************************************************************
void	Controller::UpdateConnectedIndicator(const int tabNum, const int widgetNum)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cCommonProp.Connected)
	{
		SetWidgetText(		tabNum,	widgetNum,	"C");
		SetWidgetTextColor(	tabNum,	widgetNum,	CV_RGB(0,	255,	0));
	}
	else
	{
		SetWidgetText(		tabNum,	widgetNum,	"NC");
		SetWidgetTextColor(	tabNum,	widgetNum,	CV_RGB(255,	0,	0));
		SetWidgetTextColor(	tabNum,	widgetNum,	CV_RGB(255,	255,	255));
	}
}

//*****************************************************************************
bool	Controller::AlpacaGetCommonProperties_OneAAT(const char *deviceTypeStr)
{
char	returnString[256];
bool	validData;
bool	myOnLineFlag;
bool	myConnectedFlag;
int		validCnt;

	CONSOLE_DEBUG(__FUNCTION__);
	validCnt		=	0;
	myOnLineFlag	=	true;	//*	assume it is on line
	validData		=	false;

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		CONSOLE_DEBUG_W_STR("deviceTypeStr=", deviceTypeStr);
		validData	=	AlpacaGetBooleanValue(	deviceTypeStr, "connected",	NULL,	&myConnectedFlag);
		if (validData)
		{
			cCommonProp.Connected	=	myConnectedFlag;
			validCnt++;
			CONSOLE_DEBUG_W_NUM("cCommonProp.Connected\t=", cCommonProp.Connected);
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		validData	=	AlpacaGetStringValue(	deviceTypeStr, "description",	NULL,	returnString);
		if (validData)
		{
			strcpy(cCommonProp.Description,	returnString);
			validCnt++;
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		validData	=	AlpacaGetStringValue(	deviceTypeStr, "driverinfo",	NULL,	returnString);
		if (validData)
		{
			strcpy(cCommonProp.DriverInfo,	returnString);
			validCnt++;
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		validData	=	AlpacaGetStringValue(	deviceTypeStr, "driverinfo",	NULL,	returnString);
		if (validData)
		{
			strcpy(cCommonProp.DriverInfo,	returnString);
			validCnt++;
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		validData	=	AlpacaGetStringValue(	deviceTypeStr, "interfaceversion",	NULL,	returnString);
		if (validData)
		{
			cCommonProp.InterfaceVersion	=	atoi(returnString);
			validCnt++;
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		validData	=	AlpacaGetStringValue(	deviceTypeStr, "driverversion",	NULL,	returnString);
		if (validData)
		{
			strcpy(cCommonProp.DriverVersion,	returnString);
			validCnt++;
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}

	//-----------------------------------------------------------------------------------------
	if (myOnLineFlag)
	{
		validData	=	AlpacaGetStringValue(	deviceTypeStr, "name",	NULL,	returnString);
		if (validData)
		{
			strcpy(cCommonProp.Name,	returnString);
			validCnt++;
		}
		else
		{
			myOnLineFlag	=	false;
		}
	}
	CONSOLE_DEBUG_W_NUM("validCnt\t=", validCnt);

	UpdateCommonProperties();
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
	return(validData);
}


//*****************************************************************************
void	Controller::UpdateCommonProperties(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	//*	needs to be overloaded
}


//*****************************************************************************
bool	Controller::AlpacaGetSupportedActions(	sockaddr_in	*deviceAddress,
												int			devicePort,
												const char	*deviceTypeStr,
												const int	deviceNum)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;

	if (gVerbose)
	{
	char			ipAddrStr[32];
		CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
		CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceTypeStr);

		inet_ntop(AF_INET, &(deviceAddress->sin_addr), ipAddrStr, INET_ADDRSTRLEN);
		CONSOLE_DEBUG_W_STR("IP address=", ipAddrStr);
		CONSOLE_DEBUG_W_NUM("devicePort=", devicePort);
		CONSOLE_DEBUG_W_NUM("deviceNum=", deviceNum);
	}

	//===============================================================
	//*	get supportedactions
	SJP_Init(&jsonParser);
//	CONSOLE_DEBUG("Dumping EMPTY Json parser");
//	SJP_DumpJsonData(&jsonParser);
//	CONSOLE_DEBUG_W_NUM("tokenCount_Data\t=", jsonParser.tokenCount_Data);
	sprintf(alpacaString,	"/api/v1/%s/%d/supportedactions", deviceTypeStr, deviceNum);
//	CONSOLE_DEBUG_W_STR("Calling GetJsonResponse() with:", alpacaString);
	validData	=	GetJsonResponse(	deviceAddress,
										devicePort,
										alpacaString,
										NULL,
										&jsonParser);
//	CONSOLE_DEBUG("Returned from GetJsonResponse");
	if (validData)
	{
//		CONSOLE_DEBUG("We have valid data");
//		CONSOLE_DEBUG_W_NUM("tokenCount_Data\t=", jsonParser.tokenCount_Data);
//		if (jsonParser.tokenCount_Data > 90)
//		{
//			CONSOLE_DEBUG("Dumping FULL Json parser");
//		}
//		CONSOLE_DEBUG("Dumping FULL Json parser");
//		SJP_DumpJsonData(&jsonParser);
		cHas_readall	=	false;
		jjj	=	0;
		while (jjj < jsonParser.tokenCount_Data)
		{
//			if (strcmp(deviceTypeStr, "dome") == 0)
//			{
//				CONSOLE_DEBUG_W_NUM("jjj\t=", jjj);
//				CONSOLE_DEBUG_W_2STR("kw:val=", jsonParser.dataList[jjj].keyword,
//												jsonParser.dataList[jjj].valueString);
//			}
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "ARRAY") == 0)
			{
				jjj++;	//*	skip over the ARRAY entry

//				CONSOLE_DEBUG_W_NUM("ARRAY found, jjj\t=", jjj);
//				CONSOLE_DEBUG_W_STR("keyword\t=", jsonParser.dataList[jjj].keyword);
				while ((jjj < jsonParser.tokenCount_Data) &&
						(jsonParser.dataList[jjj].keyword[0] != ']'))
				{
//					CONSOLE_DEBUG_W_STR("Calling AlpacaProcessSupportedActions", deviceTypeStr);
					AlpacaProcessSupportedActions(	deviceTypeStr,
													deviceNum,
													jsonParser.dataList[jjj].valueString);
					jjj++;
				}
			}
			jjj++;
		}
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	return(validData);
}


//*****************************************************************************
bool	Controller::AlpacaGetSupportedActions(const char *deviceTypeStr, const int deviceNum)
{
bool			validData;

	CONSOLE_DEBUG(__FUNCTION__);
	validData	=	AlpacaGetSupportedActions(	&cDeviceAddress,
												cPort,
												deviceTypeStr,
												deviceNum);
	return(validData);
}

//*****************************************************************************
//*	if this routine gets overloaded, the first part, checking for "readall" must be preserved
//*****************************************************************************
void	Controller::AlpacaProcessSupportedActions(const char *deviceTypeStr, const int deviveNum, const char *valueString)
{

	if (strcasecmp(valueString, "readall") == 0)
	{
		cHas_readall	=	true;
	}
	else if (strcasecmp(valueString, "foo") == 0)
	{
		//*	you get the idea
	}
//	CONSOLE_DEBUG(cWindowName);
//	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
//*	added new version of this 1/9/2021 to allow multiple devices
//*****************************************************************************
bool	Controller::AlpacaGetStatus_ReadAll(	sockaddr_in	*deviceAddress,
												int			devicePort,
												const char	*deviceTypeStr,
												const int	deviceNum)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;

//	CONSOLE_DEBUG(cWindowName);
//	CONSOLE_DEBUG_W_STR("Requesting 'readall' for", deviceTypeStr);

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", deviceTypeStr, deviceNum, "readall");

	validData	=	GetJsonResponse(	deviceAddress,
										devicePort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
		cLastAlpacaErrNum	=	kASCOM_Err_Success;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
			AlpacaProcessReadAll(	deviceTypeStr,
									deviceNum,
									jsonParser.dataList[jjj].keyword,
									jsonParser.dataList[jjj].valueString);
		}
	}
	return(validData);
}


//*****************************************************************************
bool	Controller::AlpacaGetStatus_ReadAll(const char *deviceTypeStr, const int deviceNum)
{
bool			validData;

//	CONSOLE_DEBUG(cWindowName);

	validData	=	AlpacaGetStatus_ReadAll(&cDeviceAddress,
											cPort,
											deviceTypeStr,
											deviceNum);
	return(validData);
}

//*****************************************************************************
void	Controller::AlpacaProcessReadAll(	const char	*deviceTypeStr,
											const int	deviceNum,
											const char	*keywordString,
											const char	*valueString)
{
	//*	this function should be overloaded
	CONSOLE_DEBUG(cWindowName);
	CONSOLE_DEBUG_W_2STR("json=",	keywordString, valueString);
//	CONSOLE_ABORT(cWindowName)
}

//*****************************************************************************
void	Controller::AlpacaProcessReadAll_Common(const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString)
{
//	CONSOLE_DEBUG(cWindowName);
//	CONSOLE_DEBUG_W_2STR("json=",	keywordString, valueString);
	//=================================================================================
	if (strcasecmp(keywordString, "connected") == 0)
	{
		cCommonProp.Connected	=	IsTrueFalse(valueString);
	}
	//=================================================================================
	else if (strcasecmp(keywordString, "description") == 0)
	{
		strcpy(cCommonProp.Description,	valueString);
	}
	//=================================================================================
	else if (strcasecmp(keywordString, "driverinfo") == 0)
	{
		strcpy(cCommonProp.DriverInfo,	valueString);
	}
	//=================================================================================
	else if (strcasecmp(keywordString, "driverversion") == 0)
	{
		strcpy(cCommonProp.DriverVersion,	valueString);
	}
	//=================================================================================
	else if (strcasecmp(keywordString, "interfaceversion") == 0)
	{
		cCommonProp.InterfaceVersion	=	atoi(valueString);
	}
	//=================================================================================
	else if (strcasecmp(keywordString, "name") == 0)
	{
		strcpy(cCommonProp.Name,	valueString);
	}
}


//*****************************************************************************
bool	Controller::AlpacaSendPutCmdwResponse(	sockaddr_in		*deviceAddress,
												int				devicePort,
												const char		*alpacaDevice,
												const int		alpacaDevNum,
												const char		*alpacaCmd,
												const char		*dataString,
												SJP_Parser_t	*jsonParser)
{
char		alpacaString[128];
bool		sucessFlag;
char		myDataString[512]	=	"";
int			dataStrLen;


//	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);

	if (jsonParser  != NULL)
	{
		SJP_Init(jsonParser);

		sprintf(alpacaString, "/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);

		dataStrLen	=	0;
		if (dataString != NULL)
		{
			dataStrLen	=	strlen(dataString);
		}

		if (dataStrLen > 0)
		{
			sprintf(myDataString, "%s&ClientID=%d&ClientTransactionID=%d",
													dataString,
													gClientID,
													gClientTransactionID);
		}
		else
		{
			sprintf(myDataString, "ClientID=%d&ClientTransactionID=%d",
													gClientID,
													gClientTransactionID);
		}
//		CONSOLE_DEBUG_W_STR("Sending:", myDataString);
		sucessFlag	=	SendPutCommand(	deviceAddress,
										devicePort,
										alpacaString,
										myDataString,
										jsonParser);
		if (sucessFlag == false)
		{
			CONSOLE_DEBUG_W_STR("SendPutCommand Failed!!!", alpacaString);
			CONSOLE_DEBUG_W_STR("cLastAlpacaErrStr\t=", cLastAlpacaErrStr);
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(jsonParser, cLastAlpacaErrStr, true);


		cForceAlpacaUpdate	=	true;
		gClientTransactionID++;

		strcpy(cLastAlpacaCmdString, alpacaString);
	}
	else
	{
		CONSOLE_ABORT("Internal error, jsonParser is NULL");
	}

	return(sucessFlag);
}

//*****************************************************************************
//*
//*		curl -X PUT "https://virtserver.swaggerhub.com/ASCOMInitiative/api/v1/camera/0/gain"
//*			-H  "accept: application/json"
//*			-H  "Content-Type: application/x-www-form-urlencoded"
//*			-d "Gain=77&ClientID=34&ClientTransactionID=12"
//*****************************************************************************
bool	Controller::AlpacaSendPutCmdwResponse(	const char		*alpacaDevice,
												const char		*alpacaCmd,
												const char		*dataString,
												SJP_Parser_t	*jsonParser)
{
bool			sucessFlag;

//	CONSOLE_DEBUG(__FUNCTION__);

	sucessFlag	=	AlpacaSendPutCmdwResponse(	&cDeviceAddress,
												cPort,
												alpacaDevice,
												cAlpacaDevNum,
												alpacaCmd,
												dataString,
												jsonParser);
	return(sucessFlag);
}

//*****************************************************************************
bool	Controller::AlpacaSendPutCmd(	const char	*alpacaDevice,
										const char	*alpacaCmd,
										const char	*dataString)
{
SJP_Parser_t	jsonParser;
bool			sucessFlag;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);

	sucessFlag	=	AlpacaSendPutCmdwResponse(alpacaDevice, alpacaCmd, dataString, &jsonParser);

	return(sucessFlag);
}

//*****************************************************************************
bool	Controller::AlpacaGetIntegerValue(	struct sockaddr_in	deviceAddress,
											int					port,
											int					alpacaDevNum,
											const char			*alpacaDevice,
											const char			*alpacaCmd,
											const char			*dataString,
											int					*returnValue,
											bool				*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
int				myIntgerValue;

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);
//	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);

	validData	=	GetJsonResponse(	&deviceAddress,
										port,
										alpacaString,
										dataString,
										&jsonParser);
//	CONSOLE_DEBUG(__FUNCTION__);
	if (validData)
	{
		cLastAlpacaErrNum	=	kASCOM_Err_Success;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				myIntgerValue	=	atoi(jsonParser.dataList[jjj].valueString);
				if (returnValue != NULL)
				{
					*returnValue	=	myIntgerValue;
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
			//*	does the calling routine want to know if the data was good
			if (rtnValidData != NULL)
			{
				*rtnValidData	=	false;
			}
		}
	}
	else
	{
		cReadFailureCnt++;
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	return(validData);
}


//*****************************************************************************
bool	Controller::AlpacaGetIntegerValue(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString,
											int			*returnValue,
											bool		*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				myIntgerValue;
int				jjj;

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, cAlpacaDevNum, alpacaCmd);
//	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);
	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										alpacaString,
										dataString,
										&jsonParser);
	if (validData)
	{
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
//			CONSOLE_DEBUG_W_2STR("json=",	jsonParser.dataList[jjj].keyword,
//											jsonParser.dataList[jjj].valueString);

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				myIntgerValue	=	atoi(jsonParser.dataList[jjj].valueString);
				if (returnValue != NULL)
				{
					*returnValue	=	myIntgerValue;
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
	}
	else
	{
		cReadFailureCnt++;
	}
	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaGetStringValue(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString,
											char		*returnString,
											bool		*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, cAlpacaDevNum, alpacaCmd);
//	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);
	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										alpacaString,
										dataString,
										&jsonParser);
	if (validData)
	{
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
//			CONSOLE_DEBUG_W_2STR("json=",	jsonParser.dataList[jjj].keyword,
//											jsonParser.dataList[jjj].valueString);

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				if (returnString != NULL)
				{
					strcpy(returnString, jsonParser.dataList[jjj].valueString);
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
	}
	else
	{
		cReadFailureCnt++;
	}
	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaGetDoubleValue(	struct sockaddr_in	deviceAddress,
											int					port,
											int					alpacaDevNum,
											const char			*alpacaDevice,
											const char			*alpacaCmd,
											const char			*dataString,
											double				*returnValue,
											bool				*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
double			myDoubleValue;

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);
//	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);

	validData	=	GetJsonResponse(	&deviceAddress,
										port,
										alpacaString,
										dataString,
										&jsonParser);
//	CONSOLE_DEBUG(__FUNCTION__);
	if (validData)
	{
		cLastAlpacaErrNum	=	kASCOM_Err_Success;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				myDoubleValue	=	atof(jsonParser.dataList[jjj].valueString);
				if (returnValue != NULL)
				{
					*returnValue	=	myDoubleValue;
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
			//*	does the calling routine want to know if the data was good
			if (rtnValidData != NULL)
			{
				*rtnValidData	=	false;
			}
		}
	}
	else
	{
		cReadFailureCnt++;
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	return(validData);
}



//*****************************************************************************
//*	return value is true if the message was sent and a response was received
//*	The *validData flag indicates if data was found
//*****************************************************************************
bool	Controller::AlpacaGetDoubleValue(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString,
											double		*returnValue,
											bool		*rtnValidData)
{
bool			validData;
#if 1
	validData	=	AlpacaGetDoubleValue(	cDeviceAddress,
											cPort,
											cAlpacaDevNum,
											alpacaDevice,
											alpacaCmd,
											dataString,
											returnValue,
											rtnValidData);
#else

SJP_Parser_t	jsonParser;
char			alpacaString[128];
int				jjj;
double			myDoubleValue;

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, cAlpacaDevNum, alpacaCmd);
//	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);

	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										alpacaString,
										dataString,
										&jsonParser);
//	CONSOLE_DEBUG(__FUNCTION__);
	if (validData)
	{
		cLastAlpacaErrNum	=	kASCOM_Err_Success;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				myDoubleValue	=	atof(jsonParser.dataList[jjj].valueString);
				if (returnValue != NULL)
				{
					*returnValue	=	myDoubleValue;
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
			//*	does the calling routine want to know if the data was good
			if (rtnValidData != NULL)
			{
				*rtnValidData	=	false;
			}
		}
	}
	else
	{
		cReadFailureCnt++;
	}
//	CONSOLE_DEBUG(__FUNCTION__);
#endif

	return(validData);
}

//*****************************************************************************
//*	this is a very special case, but it gets used at least twice by the telescope controller
//*****************************************************************************
bool	Controller::AlpacaGetMinMax(	const char	*alpacaDevice,
										const char	*alpacaCmd,
										const char	*dataString,
										double		*returnMinValue,
										double		*returnMaxValue,
										bool		*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
double			myDoubleValue;

//	CONSOLE_DEBUG(__FUNCTION__);
	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, cAlpacaDevNum, alpacaCmd);

	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										alpacaString,
										dataString,
										&jsonParser);
	if (validData)
	{
		cLastAlpacaErrNum	=	kASCOM_Err_Success;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "MINIMUM") == 0)
			{
				myDoubleValue	=	atof(jsonParser.dataList[jjj].valueString);
				if (returnMinValue != NULL)
				{
					*returnMinValue	=	myDoubleValue;
				}
			}
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "MAXIMUM") == 0)
			{
				myDoubleValue	=	atof(jsonParser.dataList[jjj].valueString);
				if (returnMaxValue != NULL)
				{
					*returnMaxValue	=	myDoubleValue;
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
		if (cLastAlpacaErrNum != kASCOM_Err_Success)
		{
			//*	does the calling routine want to know if the data was good
			if (rtnValidData != NULL)
			{
				*rtnValidData	=	false;
			}
		}
	}
	else
	{
		cReadFailureCnt++;
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	return(validData);
}



//*****************************************************************************
bool	Controller::AlpacaGetBooleanValue(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString,
											bool		*returnValue,
											bool		*rtnValidData,
											bool		printDebug)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
bool			myBooleanValue;

	if (printDebug)
	{
		CONSOLE_DEBUG_W_2STR(__FUNCTION__, alpacaDevice, alpacaCmd);
	}

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, cAlpacaDevNum, alpacaCmd);

//	if (strcmp(alpacaDevice, "switch") == 0)
//	{
//		CONSOLE_DEBUG_W_2STR("alpacaString\t=", alpacaString, dataString);
//	}

	validData	=	GetJsonResponse(	&cDeviceAddress,
										cPort,
										alpacaString,
										dataString,
										&jsonParser);
	if (validData)
	{
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
			//*	is debugging enabled
			if (printDebug)
			{
				CONSOLE_DEBUG_W_2STR("json=",	jsonParser.dataList[jjj].keyword,
												jsonParser.dataList[jjj].valueString);
			}

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				myBooleanValue	=	IsTrueFalse(jsonParser.dataList[jjj].valueString);

				if (returnValue != NULL)
				{
					*returnValue	=	myBooleanValue;
				}
			}
		}
		cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
	}
	else
	{
		CONSOLE_DEBUG_W_2STR("Failed", alpacaString, dataString);
		cReadFailureCnt++;
	}
	return(validData);
}


//*****************************************************************************
void	Controller::UpdateDownloadProgress(const int unitsRead, const int totalUnits)
{
	//*	this is to be over loaded if needed
	CONSOLE_ABORT(__FUNCTION__);
}


#if 0
//*****************************************************************************
static int	CountChars(const char *theString, const char theChar)
{
int		charCnt;
int		iii;

	charCnt	=	0;
	iii		=	0;
	while (theString[iii] != 0)
	{
		if (theString[iii] == theChar)
		{
			charCnt++;
		}
		iii++;
	}
//	CONSOLE_DEBUG_W_NUM("charCnt\t=", charCnt);
	return(charCnt);
}
#endif // 0


//*****************************************************************************
TYPE_ASCOM_STATUS	Controller::AlpacaCheckForErrors(	SJP_Parser_t	*jsonParser,
										char			*errorMsg,
										bool 			reportError)
{
int					jjj;
TYPE_ASCOM_STATUS	alpacaErrorCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrorCode	=	kASCOM_Err_Success;
	strcpy(errorMsg, "");
	if (jsonParser != NULL)
	{
		for (jjj=0; jjj<jsonParser->tokenCount_Data; jjj++)
		{
			if (strcasecmp(jsonParser->dataList[jjj].keyword, "ErrorNumber") == 0)
			{
				alpacaErrorCode	=	(TYPE_ASCOM_STATUS)atoi(jsonParser->dataList[jjj].valueString);
			}
			else if (strcasecmp(jsonParser->dataList[jjj].keyword, "ErrorMessage") == 0)
			{
				if (strlen(jsonParser->dataList[jjj].valueString) > 0)
				{
					strcpy(errorMsg, jsonParser->dataList[jjj].valueString);
					if (reportError)
					{
						AlpacaDisplayErrorMessage(errorMsg);
					}
				}
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("jsonParser is NULL");
	}
	return(alpacaErrorCode);
}

//*****************************************************************************
void	Controller::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
	//*	this should be overloaded
}
#endif // _CONTROLLER_USES_ALPACA_

