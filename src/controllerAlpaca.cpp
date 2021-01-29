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
//*****************************************************************************


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>
#include	<errno.h>


//-#include "opencv/highgui.h"
//-#include "opencv2/highgui/highgui_c.h"
//-#include "opencv2/imgproc/imgproc_c.h"

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#define	_DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"widget.h"
#include	"controller.h"

static int	gClientID				=	1;
static int	gClientTransactionID	=	1;

//*****************************************************************************
//*	this should be over-ridden
bool	Controller::AlpacaGetStartupData(void)
{
	CONSOLE_ABORT(__FUNCTION__);
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
char			ipAddrStr[32];
int				jjj;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceTypeStr);

//	inet_ntop(AF_INET, &(deviceAddress->sin_addr), ipAddrStr, INET_ADDRSTRLEN);
//	CONSOLE_DEBUG_W_STR("IP address=", ipAddrStr);

	//===============================================================
	//*	get supportedactions
	SJP_Init(&jsonParser);
//	CONSOLE_DEBUG_W_NUM("tokenCount_Data\t=", jsonParser.tokenCount_Data);
	sprintf(alpacaString,	"/api/v1/%s/%d/supportedactions", deviceTypeStr, deviceNum);
//	CONSOLE_DEBUG_W_STR("alpacaString\t=", alpacaString);
	validData	=	GetJsonResponse(	deviceAddress,
										devicePort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
		cHas_readall	=	false;
		jjj	=	0;
		while (jjj<jsonParser.tokenCount_Data)
		{
//			CONSOLE_DEBUG_W_NUM("jjj\t=", jjj);
//			CONSOLE_DEBUG_W_2STR("kw:val=", jsonParser.dataList[jjj].keyword,
//											jsonParser.dataList[jjj].valueString);
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
	CONSOLE_DEBUG(cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
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

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", deviceTypeStr, deviceNum, "readall");

	validData	=	GetJsonResponse(	deviceAddress,
										devicePort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
		cLastAlpacaErrNum	=	0;
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
		CONSOLE_DEBUG_W_STR("Sending:", myDataString);
		sucessFlag	=	SendPutCommand(	deviceAddress,
										devicePort,
										alpacaString,
										myDataString,
										jsonParser);

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

#if 1
	sucessFlag	=	AlpacaSendPutCmdwResponse(	&cDeviceAddress,
												cPort,
												alpacaDevice,
												cAlpacaDevNum,
												alpacaCmd,
												dataString,
												jsonParser);
#else
char			alpacaString[128];
char			myDataString[512];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);

	SJP_Init(jsonParser);

	sprintf(alpacaString, "/api/v1/%s/%d/%s", alpacaDevice, cAlpacaDevNum, alpacaCmd);
	sprintf(myDataString, "%s&ClientID=%d&ClientTransactionID=%d",
											dataString,
											gClientID,
											gClientTransactionID);
	sucessFlag	=	SendPutCommand(	&cDeviceAddress,
									cPort,
									alpacaString,
									myDataString,
									jsonParser);

	cLastAlpacaErrNum	=	AlpacaCheckForErrors(jsonParser, cLastAlpacaErrStr, true);


	cForceAlpacaUpdate	=	true;
	gClientTransactionID++;

	strcpy(cLastAlpacaCmdString, alpacaString);
#endif
	return(sucessFlag);
}

//*****************************************************************************
bool	Controller::AlpacaSendPutCmd(	const char	*alpacaDevice,
										const char	*alpacaCmd,
										const char	*dataString)
{
SJP_Parser_t	jsonParser;
bool			sucessFlag;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);

	sucessFlag	=	AlpacaSendPutCmdwResponse(alpacaDevice, alpacaCmd, dataString, &jsonParser);

	return(sucessFlag);
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
//*	return value is true if the message was sent and a response was received
//*	The *validData flag indicates if data was found
//*****************************************************************************
bool	Controller::AlpacaGetDoubleValue(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString,
											double		*returnValue,
											bool		*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
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
		cLastAlpacaErrNum	=	0;
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
		if (cLastAlpacaErrNum != 0)
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
		cLastAlpacaErrNum	=	0;
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
		if (cLastAlpacaErrNum != 0)
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
											bool		*rtnValidData)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
bool			myBooleanValue;

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
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
//			CONSOLE_DEBUG_W_2STR("json=",	jsonParser.dataList[jjj].keyword,
//											jsonParser.dataList[jjj].valueString);

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
			{
				if (strcasecmp(jsonParser.dataList[jjj].valueString, "true") == 0)
				{
					myBooleanValue	=	true;
				}
				else
				{
					myBooleanValue	=	false;
				}

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
		cReadFailureCnt++;
	}
	return(validData);
}

#define	kLineBufSize	512

//*****************************************************************************
void	JSON_ExtractKeyword_Value(const char *linebuf, char *keywordStr, char *valueStr)
{
char	myLineBuf[kLineBufSize];
int		ccc;
int		sLen;
char	*colonPtr;

	keywordStr[0]	=	0;
	valueStr[0]		=	0;

	sLen	=	strlen(linebuf);
	//*	get rid of leading spaces
	ccc	=	0;
	while (((linebuf[ccc] == 0x20) || (linebuf[ccc] == 0x09)) && (ccc < sLen))
	{
		ccc++;
	}
	strcpy(myLineBuf, &linebuf[ccc]);


	//*	separate out the keyword
	if (myLineBuf[0] == '"')
	{
		strcpy(keywordStr, &myLineBuf[1]);
	}
	else
	{
		strcpy(keywordStr, myLineBuf);
	}
	colonPtr	=	strchr(keywordStr, ':');
	if (colonPtr != NULL)
	{
		*colonPtr	=	0;
	}
	//*	check for trailing quote
	sLen	=	strlen(keywordStr);
	if (keywordStr[sLen - 1] == '"')
	{
		keywordStr[sLen - 1]	=	0;
	}
	//*	now go back to the original and look for the value
	colonPtr	=	strchr(myLineBuf, ':');
	if (colonPtr != NULL)
	{
		//*	we have a value string
		colonPtr++;	//*	skip the colon
		if (*colonPtr == '"')
		{
			colonPtr++;
		}
		strcpy(valueStr, colonPtr);

		//*	check for trailing coma
		sLen	=	strlen(valueStr);
		if (valueStr[sLen - 1] == ',')
		{
			valueStr[sLen - 1]	=	0;
		}

		//*	check for trailing quote
		sLen	=	strlen(valueStr);
		if (valueStr[sLen - 1] == '"')
		{
			valueStr[sLen - 1]	=	0;
		}

		printf("%-20s\t%s\r\n", keywordStr, valueStr);
	}
}

//*****************************************************************************
void	Controller::UpdateDownloadProgress(const int unitsRead, const int totalUnits)
{
	//*	this is to be over loaded if needed
}

#define		kImageArrayBuffSize	15000

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

//*****************************************************************************
void	ParseJsonKeyWordValuePair(const char *jsonData, char *keywordStr, char *valueStr)
{
int		iii;
int		ccc;
int		sLen;
char	theChar;

	keywordStr[0]	=	0;
	valueStr[0]	=	0;
	sLen	=	strlen(jsonData);
	ccc		=	0;
	iii		=	0;
//{"Type":2

	while ((jsonData[iii] != ':') && (iii < sLen))
	{
		theChar	=	jsonData[iii];
		switch(theChar)
		{
			case '{':
			case '\"':
				break;

			default:
				keywordStr[ccc++]	=	theChar;
				keywordStr[ccc]		=	0;
				break;

		}
		iii++;
	}
	if (jsonData[iii] == ':')
	{
		iii++;
		strcpy(valueStr, &jsonData[iii]);
	}
}


//*****************************************************************************
//*	AlpacaGetIntegerArray
//*		In this routine, we are going to send the request and get the answer directly
//*		We cannot buffer the entire JSON response because it is going to be too big
//*		which prevents us from using the already developed routines
//*
//*		Open the socket
//*		Send the request
//*		Start reading the response,
//*		We have to parse the data here and not use the JSON parser
//*****************************************************************************
bool	Controller::AlpacaGetIntegerArray(	const char	*alpacaDevice,
											const int	alpacaDevNum,
											const char	*alpacaCmd,
											const char	*dataString,
											int			*uint32array,
											int			arrayLength,
											int			*actualValueCnt)
{
bool			validData;
int				socket_desc;
char			alpacaString[128];
int				shutDownRetCode;
int				closeRetCode;
bool			keepReading;
int				recvByteCnt;
int				iii;
int				ccc;
char			theChar;
int				linesProcessed;
int				imgArrayType;
int				imgRank;
bool			valueFoundFlag;
int				myIntegerValue;
int				arrayIndex;
int				totalBytesRead;
double			downLoadSeconds;
int				braceCnt;	//*	{}
int				bracktCnt;	//*	[]
int				crCnt;
int				lfCnt;
//-char			dataLine[500];
int				startIdx;
int				prefix400Cnt;
int				socketReadCnt;
int				recv_flags;
char			keywordStr[kLineBufSize];
char			valueStr[kLineBufSize];
//char			linebuf[kImageArrayBuffSize];
char			linebuf[kReadBuffLen];
char			returnedData[kReadBuffLen + 10];
char			errorBuff[kImageArrayBuffSize + 10];
uint32_t		tStartMillisecs;
uint32_t		tCurrentMillisecs;
uint32_t		tLastUpdateMillisecs;
uint32_t		tDeltaMillisecs;
uint32_t		tStopMillisecs;


	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("kReadBuffLen\t=", kReadBuffLen);
	CONSOLE_DEBUG_W_NUM("arrayLength\t=", arrayLength);

	imgArrayType	=	-1;
	imgRank			=	-1;
	arrayIndex		=	0;
	tStartMillisecs			=	millis();
	tLastUpdateMillisecs	=	tStartMillisecs;

	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);
	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);
	socket_desc	=	OpenSocketAndSendRequest(	&cDeviceAddress,
												cPort,
												"GET",	//*	must be either GET or PUT
												alpacaString,
												dataString);
	strcpy(cLastAlpacaCmdString, alpacaString);
	if (socket_desc >= 0)
	{
		CONSOLE_DEBUG("Success: Connection open and data sent");
		START_TIMING();
		valueFoundFlag	=	false;
		keepReading		=	true;
		ccc				=	0;
		linesProcessed	=	0;
		totalBytesRead	=	0;
		braceCnt		=	0;
		bracktCnt		=	0;
		crCnt			=	0;
		lfCnt			=	0;
		prefix400Cnt	=	0;
		socketReadCnt	=	0;
		while (keepReading)
		{
		int			getOptRetCode;
		int			socketErrCode;
		socklen_t	sockOptLen;

			socketErrCode	=	0;
			sockOptLen		=	sizeof(int);
			getOptRetCode	=	getsockopt(	socket_desc,
											SOL_SOCKET,
											SO_ERROR,
											&socketErrCode,
											&sockOptLen);

			if (socketErrCode!= 0)
			{
				CONSOLE_DEBUG_W_NUM("socketErrCode\t=", socketErrCode);
			}

		//	CONSOLE_DEBUG("Calling recv with MSG_OOB");
			recv_flags	=	MSG_ERRQUEUE;
			recvByteCnt	=	recv(socket_desc, errorBuff , kImageArrayBuffSize , recv_flags);
			if (recvByteCnt >= 0)
			{
				CONSOLE_DEBUG_W_NUM("MSG_ERRQUEUE recvByteCnt\t=", recvByteCnt);
				CONSOLE_DEBUG_W_STR("errorBuff\t=", errorBuff);
				CONSOLE_ABORT(__FUNCTION__)
			}


		//	CONSOLE_DEBUG(__FUNCTION__);
			recvByteCnt	=	recv(socket_desc, returnedData , kReadBuffLen , 0);
		//	CONSOLE_DEBUG_W_NUM("recvByteCnt\t=", recvByteCnt);
			if (recvByteCnt > 0)
			{
				socketReadCnt++;
				if (socketReadCnt < 6)
				{
					CONSOLE_DEBUG("--------------------------------------------------");
					CONSOLE_DEBUG_W_NUM("socketReadCnt\t=",	socketReadCnt);
					CONSOLE_DEBUG_W_NUM("recvByteCnt\t=",	recvByteCnt);
					CONSOLE_DEBUG_W_STR("returnedData\t=",	returnedData);
				}
		//		CONSOLE_DEBUG("-----");
		//		CONSOLE_DEBUG_W_HEX("returnedData\t=", returnedData[0]);
		//		CONSOLE_DEBUG_W_HEX("returnedData\t=", returnedData[1]);
		//		CONSOLE_DEBUG_W_HEX("returnedData\t=", returnedData[2]);
		//		CONSOLE_DEBUG_W_HEX("returnedData\t=", returnedData[3]);
		//		CONSOLE_DEBUG_W_HEX("returnedData\t=", returnedData[4]);
		//		CONSOLE_DEBUG_W_HEX("returnedData\t=", returnedData[5]);

				validData					=	true;
				totalBytesRead				+=	recvByteCnt;
				returnedData[recvByteCnt]	=	0;

				//*	debugging
//				fprintf(stderr, "%s", returnedData);

//				strncpy(dataLine, returnedData, 90);
//				dataLine[89]	=	0;
//				printf(">>>%s\r\n", dataLine);

				startIdx	=	0;
				if ((strncmp(returnedData, "400", 3) == 0) && (returnedData[3] == 0x0d))
				{
					startIdx	=	5;
					prefix400Cnt++;
				}



		//		CountChars(returnedData, 0x0d);
		//		CountChars(returnedData, 0x0a);
//				CONSOLE_DEBUG_W_NUM("strlen(returnedData)\t=", strlen(returnedData));

				//{"Type":2,"Rank":2,"Value":[[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

				for (iii=startIdx; iii<recvByteCnt; iii++)
				{
					theChar	=	returnedData[iii];
					if (theChar == '{')
					{
						braceCnt++;
					}
					else if (theChar == '}')
					{
						braceCnt--;
					}
					else if (theChar == '[')
					{
						bracktCnt++;
					}
					else if (theChar == ']')
					{
						bracktCnt--;
					}
					else if (theChar == 0x0d)
					{
						crCnt++;
					}
					else if (theChar == 0x0a)
					{
						lfCnt++;
					}

					if (valueFoundFlag && ((theChar == ',') || (theChar == ']')))
					{
						linebuf[ccc]	=	0;
						if (isdigit(linebuf[0]))
						{
							myIntegerValue	=	atoi(linebuf);
							if (arrayIndex < arrayLength)
							{
								uint32array[arrayIndex]	=	myIntegerValue;
								arrayIndex++;
							}
							else
							{
								CONSOLE_DEBUG("Ran out of room");
							}
						}
						ccc				=	0;
						linebuf[ccc]	=	0;

						if ((theChar == ']') || (returnedData[iii+1] == ','))
						{
//							CONSOLE_DEBUG_W_NUM("Skipping at \t=", iii);
							iii++;
						}
					}
					else if (theChar == ',')
					{
						linebuf[ccc]	=	0;
						if (socketReadCnt < 3)
						{
							CONSOLE_DEBUG_W_STR("linebuf\t=", linebuf);
						}
						if ((strlen(linebuf) > 3) && (linebuf[0] != 0x30))
						{
							ParseJsonKeyWordValuePair(linebuf, keywordStr, valueStr);
							if (socketReadCnt < 3)
							{
								CONSOLE_DEBUG_W_2STR("kw:val\t=", keywordStr, valueStr);
							}
							if (strcasecmp(keywordStr, "ArrayType") == 0)
							{
								imgArrayType	=	atoi(valueStr);
							}
							else if (strcasecmp(keywordStr, "Type") == 0)
							{
								imgArrayType	=	atoi(valueStr);
							}
							else if (strcasecmp(keywordStr, "rank") == 0)
							{
								imgRank	=	atoi(valueStr);
							}
							else if (strcasecmp(keywordStr, "value") == 0)
							{
							//	CONSOLE_DEBUG("value found!!!!");
								valueFoundFlag	=	true;
//								CONSOLE_DEBUG_W_NUM("braceCnt\t=", braceCnt);
//								CONSOLE_DEBUG_W_NUM("bracktCnt\t=", bracktCnt);
							}
						}
						ccc	=	0;
					}
					else if ((theChar == 0x0d) || (theChar == 0x0a))
					{
						linesProcessed++;

						//*	ignore
						//*	check to see if there is a string
						if (strcmp(linebuf, "400") == 0)
						{
						//	CONSOLE_DEBUG("Found 400");
							ccc	=	0;
							linebuf[ccc]	=	0;
						}
						else
						{
						int	mySlen;

							mySlen	=	strlen(linebuf);
							if ((mySlen > 1) && (linebuf[mySlen - 1] != ','))
							{
							//	CONSOLE_DEBUG("---------------------------------------");
							//	CONSOLE_DEBUG(linebuf);
							//	CONSOLE_DEBUG_W_NUM("socketReadCnt\t=", socketReadCnt);
							//	CONSOLE_DEBUG_W_STR("returnedData\t=", returnedData);
							}
						}

					}
					else if (braceCnt > 0)
					{
						if (ccc < kImageArrayBuffSize)
						{
							if ((theChar != '[') && (theChar > 0x20))
							{
								linebuf[ccc++]	=	theChar;
								linebuf[ccc]	=	0;
							}
						}
						else
						{
							CONSOLE_DEBUG("Line to long");
						}
					}
				}
			}
			else
			{
				keepReading		=	false;
			}
			//=================================================
			//*	deal with the progress bar
			tCurrentMillisecs	=	millis();
			tDeltaMillisecs		=	tCurrentMillisecs - tStartMillisecs;
			//*	dont do anything at first
			if (tDeltaMillisecs > 1500)
			{
				tDeltaMillisecs	=	tCurrentMillisecs - tLastUpdateMillisecs;
				if (tDeltaMillisecs > 400)
				{
				//	CONSOLE_DEBUG_W_DBL("Percent done=", ((100.0 * arrayIndex) / arrayLength))
					UpdateDownloadProgress(arrayIndex, arrayLength);
					tLastUpdateMillisecs	=	tCurrentMillisecs;
				}
			}
		}
		//*	one last time to show we are done
		UpdateDownloadProgress(arrayIndex, arrayLength);

		*actualValueCnt	=	arrayIndex;

		DEBUG_TIMING("Time to download image (ms)");

		cLastDownload_Bytes			=	totalBytesRead;
		cLastDownload_Millisecs		=	tDeltaMillisecs;
		downLoadSeconds				=	tDeltaMillisecs / 1000.0;
		if (downLoadSeconds > 0)
		{
			cLastDownload_MegaBytesPerSec	=	1.0 * totalBytesRead / downLoadSeconds;

		}
		else
		{
			CONSOLE_DEBUG("tDeltaMillisecs invalid");
			cLastDownload_MegaBytesPerSec	=	0.0;
		}
		CONSOLE_DEBUG_W_NUM("crCnt\t=", crCnt);
		CONSOLE_DEBUG_W_NUM("lfCnt\t=", lfCnt);

		CONSOLE_DEBUG_W_NUM("imgArrayType\t=", imgArrayType);
		CONSOLE_DEBUG_W_NUM("imgRank\t\t=", imgRank);
		CONSOLE_DEBUG_W_NUM("totalBytesRead\t=", totalBytesRead);

		CONSOLE_DEBUG_W_NUM("linesProcessed\t=", linesProcessed);
		CONSOLE_DEBUG_W_NUM("prefix400Cnt\t=", prefix400Cnt);

		shutDownRetCode	=	shutdown(socket_desc, SHUT_RDWR);
		if (shutDownRetCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("shutDownRetCode\t=", shutDownRetCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}

		closeRetCode	=	close(socket_desc);
		if (closeRetCode != 0)
		{
			CONSOLE_DEBUG("Close error");
		}
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
	}
	return(validData);
}

//*****************************************************************************
bool	Controller::AlpacaGetIntegerArrayShortLines(const char	*alpacaDevice,
													const int	alpacaDevNum,
													const char	*alpacaCmd,
													const char	*dataString,
													int			*uint32array,
													int			arrayLength,
													int			*actualValueCnt)
{
bool			validData;
int				socket_desc;
char			alpacaString[128];
int				shutDownRetCode;
int				closeRetCode;
bool			keepReading;
char			returnedData[kReadBuffLen + 10];
int				recvByteCnt;
char			linebuf[kImageArrayBuffSize];
int				iii;
int				ccc;
char			theChar;
int				linesProcessed;
char			keywordStr[kLineBufSize];
char			valueStr[kLineBufSize];
int				imgArrayType;
int				imgRank;
bool			valueFoundFlag;
int				myIntegerValue;
int				arrayIndex;
int				totalBytesRead;
double			downLoadSeconds;
int				braceCnt;	//*	()
int				bracktCnt;	//*	[]
int				socketReadCnt;
int				progressUpdateCnt;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("kReadBuffLen\t=", kReadBuffLen);
	CONSOLE_DEBUG_W_NUM("arrayLength\t=", arrayLength);

	imgArrayType	=	-1;
	imgRank			=	-1;
	arrayIndex		=	0;

	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);
	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);
	socket_desc	=	OpenSocketAndSendRequest(	&cDeviceAddress,
												cPort,
												"GET",	//*	must be either GET or PUT
												alpacaString,
												dataString);
	strcpy(cLastAlpacaCmdString, alpacaString);
	if (socket_desc >= 0)
	{
		CONSOLE_DEBUG("Success: Connection open and data sent");
		SETUP_TIMING();
		START_TIMING();
		valueFoundFlag		=	false;
		keepReading			=	true;
		ccc					=	0;
		linesProcessed		=	0;
		totalBytesRead		=	0;
		braceCnt			=	0;
		bracktCnt			=	0;
		progressUpdateCnt	=	0;
		socketReadCnt		=	0;
		while (keepReading)
		{
			recvByteCnt	=	recv(socket_desc, returnedData , kReadBuffLen , 0);
//			CONSOLE_DEBUG_W_NUM("recvByteCnt\t=", recvByteCnt);
			if (recvByteCnt > 0)
			{
				validData					=	true;
				totalBytesRead				+=	recvByteCnt;
				returnedData[recvByteCnt]	=	0;


				for (iii=0; iii<recvByteCnt; iii++)
				{
					theChar	=	returnedData[iii];
					if ((theChar == 0x0d) || (theChar == 0x0a))
					{
						linebuf[ccc]	=	0;
						if (strlen(linebuf) > 0)
						{
							JSON_ExtractKeyword_Value(linebuf, keywordStr, valueStr);

							if (strcasecmp(keywordStr, "ArrayType") == 0)
							{
								imgArrayType	=	atoi(valueStr);
							}
							if (strcasecmp(keywordStr, "Type") == 0)
							{
								imgArrayType	=	atoi(valueStr);
							}
							else if (strcasecmp(keywordStr, "rank") == 0)
							{
								imgRank	=	atoi(valueStr);
							}
							else if (strcasecmp(keywordStr, "value") == 0)
							{
							//	CONSOLE_DEBUG("value found!!!!");
								valueFoundFlag	=	true;
							}
							else if (valueFoundFlag)
							{
								if (isdigit(keywordStr[0]))
								{
									myIntegerValue	=	atoi(keywordStr);
									if (arrayIndex < arrayLength)
									{
										uint32array[arrayIndex]	=	myIntegerValue;
										arrayIndex++;
									}
									else
									{
										CONSOLE_DEBUG("Ran out of room");
									}
								}
								else
								{
									printf("%-20s\t%s\r\n", keywordStr, valueStr);
								}
							}
							linesProcessed++;
						}
						ccc				=	0;
						linebuf[ccc]	=	0;
					}
					else
					{
						if (ccc < kImageArrayBuffSize)
						{
							linebuf[ccc]	=	theChar;
							ccc++;
						}
						else
						{
							CONSOLE_DEBUG("Line to long");
						}
					}
				}
				socketReadCnt++;
			}
			else
			{
				keepReading		=	false;
			}
			//=================================================
			//*	deal with the progress bar
			if ((socketReadCnt % 2000) == 0)
			{
				UpdateDownloadProgress(arrayIndex, arrayLength);
				progressUpdateCnt++;
			}
		}
		//*	one last time to show we are done
		UpdateDownloadProgress(arrayIndex, arrayLength);


		*actualValueCnt	=	arrayIndex;

		DEBUG_TIMING("Time to download image (ms)");

		cLastDownload_Bytes			=	totalBytesRead;
		cLastDownload_Millisecs		=	tDeltaMillisecs;
		downLoadSeconds				=	tDeltaMillisecs / 1000.0;
		if (downLoadSeconds > 0)
		{
			cLastDownload_MegaBytesPerSec	=	1.0 * totalBytesRead / downLoadSeconds;

		}
		else
		{
			CONSOLE_DEBUG("tDeltaMillisecs invalid");
			cLastDownload_MegaBytesPerSec	=	0.0;
		}

		CONSOLE_DEBUG_W_NUM("imgArrayType\t=", imgArrayType);
		CONSOLE_DEBUG_W_NUM("imgRank\t\t=", imgRank);
		CONSOLE_DEBUG_W_NUM("totalBytesRead\t=", totalBytesRead);

		CONSOLE_DEBUG_W_NUM("linesProcessed\t=", linesProcessed);
		CONSOLE_DEBUG_W_NUM("progressUpdateCnt\t=", progressUpdateCnt);

		shutDownRetCode	=	shutdown(socket_desc, SHUT_RDWR);
		if (shutDownRetCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("shutDownRetCode\t=", shutDownRetCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}

		closeRetCode	=	close(socket_desc);
		if (closeRetCode != 0)
		{
			CONSOLE_DEBUG("Close error");
		}
	}
	else
	{
		CONSOLE_DEBUG("Failed");
		cReadFailureCnt++;
	}
	return(validData);
}

//*****************************************************************************
int	Controller::AlpacaCheckForErrors(	SJP_Parser_t	*jsonParser,
										char			*errorMsg,
										bool 			reportError)
{
int		jjj;
int		alpacaErrorCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrorCode	=	0;
	strcpy(errorMsg, "");
	if (jsonParser != NULL)
	{
		for (jjj=0; jjj<jsonParser->tokenCount_Data; jjj++)
		{
			if (strcasecmp(jsonParser->dataList[jjj].keyword, "ErrorNumber") == 0)
			{
				alpacaErrorCode	=	atoi(jsonParser->dataList[jjj].valueString);
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

