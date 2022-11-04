//**************************************************************************
//*	Name:			alpacadriver_templog.cpp
//*
//*	Author:			Mark Sproul (C) 2022
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
//*	Oct 16,	2022	<MLS> Created alpacadriver_templog.cpp
//*	Oct 16,	2022	<MLS> Added TemperatureLog_Init()
//*	Oct 16,	2022	<MLS> Added TemperatureLog_AddEntry()
//*	Oct 16,	2022	<MLS> Added Get_TemperatureLog()
//*****************************************************************************

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"JsonResponse.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



//*****************************************************************************
void	AlpacaDriver::TemperatureLog_Init(void)
{

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_LONG("sizeof(cTemperatureLog)\t=", sizeof(cTemperatureLog));
	CONSOLE_DEBUG_W_NUM("kTemperatureLogEntries  \t=", kTemperatureLogEntries);
	CONSOLE_DEBUG_W_LONG("sizeof(AlpacaDriver)   \t=", sizeof(AlpacaDriver));

	memset((void *)cTemperatureLog, 0, sizeof(cTemperatureLog));
	strcpy(cTempLogDescription, "unknown");

	cLastTempUpdate_Secs	=	GetSecondsSinceEpoch();

}

//*****************************************************************************
void	AlpacaDriver::TemperatureLog_SetDescription(const char *description)
{
	strcpy(cTempLogDescription, description);
}


//*****************************************************************************
//*	this routine must be called at least once a minute
//*****************************************************************************
void	AlpacaDriver::TemperatureLog_AddEntry(const double temperatureEntry)
{
int		minutesSinceMidnight;

	minutesSinceMidnight	=	GetMinutesSinceMidnight();
	if (minutesSinceMidnight < kTemperatureLogEntries)
	{
		cTemperatureLog[minutesSinceMidnight]	=	temperatureEntry;
	}
}

//*****************************************************************************
TYPE_ASCOM_STATUS	AlpacaDriver::Get_TemperatureLog(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					iii;
int					mySocket;
char				longBuffer[2048];
char				lineBuff[128];
int					bytesWritten;
int					bufLen;
int					dataElementCnt;
char				httpHeader[500];

	mySocket		=	reqData->socket;


	JsonResponse_FinishHeader(httpHeader, "");
	JsonResponse_SendTextBuffer(mySocket, httpHeader);

	//*	add the description of what the temperature log is logging
	sprintf(lineBuff, "\t\t\"Description\":\"%s\",\r\n", cTempLogDescription);
	strcat(reqData->jsonTextBuffer, lineBuff);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_ArrayStart(	mySocket,
																reqData->jsonTextBuffer,
																kMaxJsonBuffLen,
																gValueString);

	//*	Flush the json buffer
	JsonResponse_SendTextBuffer(mySocket, reqData->jsonTextBuffer);

	strcpy(longBuffer, "\n");

	//*	stop at n-1 so we can do the last one without a comma
	dataElementCnt	=	0;
	for (iii =0; iii< (kTemperatureLogEntries - 1); iii++)
	{
		sprintf(lineBuff, "%3.2f,", cTemperatureLog[iii]);
		strcat(longBuffer, lineBuff);
		dataElementCnt++;
		if (dataElementCnt > 25)
		{
			strcat(longBuffer, "\n");
			dataElementCnt	=	0;
		}
		bufLen	=	strlen(longBuffer);

		if (bufLen > 1800)
		{
			bufLen			=	strlen(longBuffer);
			bytesWritten	=	write(mySocket, longBuffer, bufLen);
			longBuffer[0]	=	0;
		}
	}
	//*	now do the last one WITHOUT the comma
	sprintf(lineBuff, "%3.2f", cTemperatureLog[iii]);
	strcat(lineBuff, "\n");
	strcat(longBuffer, lineBuff);
	bufLen			=	strlen(longBuffer);
	bytesWritten	=	write(mySocket, longBuffer, bufLen);
	if (bytesWritten <= 0)
	{
		CONSOLE_DEBUG("Error writing temperature data");
	}
	cBytesWrittenForThisCmd	+=	JsonResponse_Add_ArrayEnd(	mySocket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															INCLUDE_COMMA);
	return(alpacaErrCode);
}
