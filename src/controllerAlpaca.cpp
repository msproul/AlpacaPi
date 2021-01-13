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
//*	Mar 17,	2020	<MLS> Added AlpacaProcessSupportedAction()
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
bool	Controller::AlpacaGetSupportedActions(	sockaddr_in	*deviceAddress,
												int			devicePort,
												const char	*deviceType,
												const int	deviceNum)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	//===============================================================
	//*	get supportedactions
	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/supportedactions", deviceType, deviceNum);
	validData	=	GetJsonResponse(	deviceAddress,
										devicePort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
		cHasReadAll	=	false;
		jjj	=	0;
		while (jjj<jsonParser.tokenCount_Data)
		{
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "ARRAY") == 0)
			{
				jjj++;
				while ((jjj<jsonParser.tokenCount_Data) &&
						(jsonParser.dataList[jjj].keyword[0] != ']'))
				{
					AlpacaProcessSupportedAction(deviceType, deviceNum, jsonParser.dataList[jjj].valueString);
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
	return(validData);
}


//*****************************************************************************
bool	Controller::AlpacaGetSupportedActions(const char *deviceType, const int deviceNum)
{
bool			validData;

	validData	=	AlpacaGetSupportedActions(	&cDeviceAddress,
												cPort,
												deviceType,
												deviceNum);
	return(validData);
}

//*****************************************************************************
//*	if this routine gets overloaded, the first part, checking for "readall" must be preserved
//*****************************************************************************
void	Controller::AlpacaProcessSupportedAction(const char *deviceType, const int deviveNum, const char *valueString)
{

	if (strcasecmp(valueString, "readall") == 0)
	{
		cHasReadAll	=	true;
	}
	else if (strcasecmp(valueString, "foo") == 0)
	{
		//*	you get the idea
	}
}

//*****************************************************************************
//*	added new version of this 1/9/2021 to allow multiple devices
//*****************************************************************************
bool	Controller::AlpacaGetStatus_ReadAll(	sockaddr_in	*deviceAddress,
												int			devicePort,
												const char	*deviceType,
												const int	deviceNum)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;

//	CONSOLE_DEBUG(cWindowName);

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", deviceType, deviceNum, "readall");

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
			AlpacaProcessReadAll(	deviceType,
									deviceNum,
									jsonParser.dataList[jjj].keyword,
									jsonParser.dataList[jjj].valueString);
		}
	}
	return(validData);
}


//*****************************************************************************
bool	Controller::AlpacaGetStatus_ReadAll(const char *deviceType, const int deviceNum)
{
bool			validData;

//	CONSOLE_DEBUG(cWindowName);

	validData	=	AlpacaGetStatus_ReadAll(&cDeviceAddress,
											cPort,
											deviceType,
											deviceNum);
	return(validData);
}

//*****************************************************************************
void	Controller::AlpacaProcessReadAll(	const char	*deviceType,
											const int	deviceNum,
											const char	*keywordString,
											const char	*valueString)
{
	//*	this function should be overloaded
	CONSOLE_DEBUG(cWindowName);
	CONSOLE_DEBUG_W_2STR("json=",	keywordString, valueString);
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
char			alpacaString[128];
bool			sucessFlag;
char			myDataString[512];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);

	if (jsonParser  != NULL)
	{
		SJP_Init(jsonParser);

		sprintf(alpacaString, "/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);
		sprintf(myDataString, "%s&ClientID=%d&ClientTransactionID=%d",
												dataString,
												gClientID,
												gClientTransactionID);
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

	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);

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
char			returnedData[kReadBuffLen + 10];
int				recvByteCnt;
char			linebuf[kLineBufSize];
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
long			totalBytesRead;
double			downLoadSeconds;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("kReadBuffLen\t=", kReadBuffLen);
	CONSOLE_DEBUG_W_NUM("arrayLength\t=", arrayLength);

	imgArrayType	=	-1;
	imgRank			=	-1;
	arrayIndex		=	1;		//*	change to 0 later

	sprintf(alpacaString,	"/api/v1/%s/%d/%s", alpacaDevice, alpacaDevNum, alpacaCmd);
	CONSOLE_DEBUG_W_STR("alpacaString\t=",	alpacaString);
	socket_desc	=	OpenSocketAndSendRequest(	&cDeviceAddress,
												cPort,
												"GET",	//*	must be either GET or PUT
												alpacaString,
												dataString);
	if (socket_desc >= 0)
	{
		CONSOLE_DEBUG("Success: Connection open and data sent");
		SETUP_TIMING();
		START_TIMING();
		valueFoundFlag	=	false;
		keepReading		=	true;
		ccc				=	0;
		linesProcessed	=	0;
		totalBytesRead	=	0;
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
						if (ccc < kLineBufSize)
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
				UpdateDownloadProgress(arrayIndex, arrayLength);
			}
			else
			{
				keepReading		=	false;
			}
		}
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
		CONSOLE_DEBUG_W_NUM("linesProcessed\t=", linesProcessed);

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

