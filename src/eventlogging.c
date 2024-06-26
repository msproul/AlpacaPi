//**************************************************************************
//*	Name:			eventlogging.c
//*
//*	Author:			Mark Sproul (C) 2019
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*		https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*		https://github.com/OpenPHDGuiding/phd2/tree/master/cameras/zwolibs
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	May 21,	2019	<MLS> Created eventlogging.c
//*	May 22,	2019	<MLS> Added SendHtmlLog()
//*****************************************************************************


//#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
//#include	<ctype.h>
//#include	<stdint.h>
#include	<time.h>
//#include	<unistd.h>



#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif // _ALPACA_DEFS_H_


#include	"eventlogging.h"
#include	"html_common.h"

#include	"alpacadriver_helper.h"


#define	kDescriptionLen	96
#define	kResultStrLen	96
#define	kErrorStrLen	96
//**************************************************************************
typedef struct
{
	time_t				eventTime;
	char				eventName[64];
	char				eventDescription[kDescriptionLen];
	char				resultString[kResultStrLen];
	TYPE_ASCOM_STATUS	alpacaErrCode;
	char				errorString[kErrorStrLen];
} TYPE_EVENTLOG;

#define	kMaxLogEntries	300

TYPE_EVENTLOG	gEventLog[kMaxLogEntries];
int				gLogIndex	=	0;

//**************************************************************************
//*	if the log files up, we will dump the first half and continue
static void	FlushHalfLog(void)
{
int		iii;
int		halfSize;

	halfSize	=	kMaxLogEntries / 2;
	for (iii=0; iii<halfSize; iii++)
	{
		gEventLog[iii]	=	gEventLog[iii + halfSize];
	}
	gLogIndex	=	halfSize;

}

//**************************************************************************
void	LogEvent(	const char				*eventName,
					const char				*eventDescription,
					const char				*resultString,
					const TYPE_ASCOM_STATUS	alpacaErrCode,
					const char				*errorString)
{
	if (gLogIndex < kMaxLogEntries)
	{
		memset(&gEventLog[gLogIndex], 0, sizeof(TYPE_EVENTLOG));
		gEventLog[gLogIndex].eventTime		=	time(NULL);
		gEventLog[gLogIndex].alpacaErrCode	=	alpacaErrCode;

		if (eventName != NULL)
		{
			strcpy(gEventLog[gLogIndex].eventName,	eventName);
		}
		if (eventDescription != NULL)
		{
			strncpy(gEventLog[gLogIndex].eventDescription,	eventDescription, (kDescriptionLen -1));
			gEventLog[gLogIndex].eventDescription[kDescriptionLen -1]	=	0;
		}
		if (resultString != NULL)
		{
			strcpy(gEventLog[gLogIndex].resultString,	resultString);
		}
		if (errorString != NULL)
		{
			strcpy(gEventLog[gLogIndex].errorString,	errorString);
		}
		gLogIndex++;

		//*	check to see if the log is full
		if (gLogIndex >= kMaxLogEntries)
		{
			FlushHalfLog();
		}
	}
}

//**************************************************************************
void	PrintLog(void)
{
int			ii;
struct tm	*linuxTime;

	for (ii=0; ii<gLogIndex; ii++)
	{
		linuxTime		=	localtime(&gEventLog[ii].eventTime);
		printf("%d/%d/%d %02d:%02d:%02d\t",
								(1 + linuxTime->tm_mon),
								linuxTime->tm_mday,
								(1900 + linuxTime->tm_year),
								linuxTime->tm_hour,
								linuxTime->tm_min,
								linuxTime->tm_sec);
		printf("%-20s\t",	gEventLog[ii].eventName);
		printf("%-20s\t",	gEventLog[ii].eventDescription);
		printf("%-20s\t",	gEventLog[ii].resultString);
		printf("%-20s\t",	gEventLog[ii].errorString);
		printf("\r\n");

	}
}


#pragma mark -

//*****************************************************************************
const char	gHtmlHeaderLog[]	=
{
	"HTTP/1.0 200 \r\n"
//	"Server: alpaca\r\n"
//	"Mime-Version: 1.0\r\n"
	"Content-Type: text/html\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<!DOCTYPE html>\r\n"
	"<HTML lang=\"en\">\r\n"
	"<HEAD>\r\n"

};

//*****************************************************************************
const char	gHtmlTitleLog[]	=
{
	"<TITLE>Alpaca command log</TITLE>\r\n"
	"</HEAD><BODY>\r\n"
	"<CENTER>\r\n"
	"<H1>Alpaca command log</H1>\r\n"
	"</CENTER>\r\n"

};


#define	kMaxErrors	256
//*****************************************************************************
void	SendHtmlLog(int mySocketFD)
{
char		lineBuff[256];
int			iii;
struct tm	*linuxTime;
int			errorTotal;
int			errorCounts[kMaxErrors];
int			errIndx;

	for (errIndx=0; errIndx<kMaxErrors; errIndx++)
	{
		errorCounts[errIndx]	=	0;
	}
	errorTotal	=	0;

	SocketWriteData(mySocketFD,	gHtmlHeaderLog);
	SocketWriteData(mySocketFD,	gHtmlNightMode);
	SocketWriteData(mySocketFD,	gHtmlTitleLog);

	//-------------------------------------------------------------------
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TH>Date/Time</TH>\r\n");
	SocketWriteData(mySocketFD,	"<TH>Device</TH>\r\n");
	SocketWriteData(mySocketFD,	"<TH>Command</TH>\r\n");
	SocketWriteData(mySocketFD,	"<TH>Err #</TH>\r\n");
	SocketWriteData(mySocketFD,	"<TH>Error/Comment</TH>\r\n");

	SocketWriteData(mySocketFD,	"</TR>\r\n");
	for (iii=0; iii<gLogIndex; iii++)
	{
		SocketWriteData(mySocketFD,	"<TR>\r\n");
		linuxTime		=	localtime(&gEventLog[iii].eventTime);
		sprintf(lineBuff, "\t<TD>%d/%d/%d %02d:%02d:%02d</TD>",
								(1 + linuxTime->tm_mon),
								linuxTime->tm_mday,
								(1900 + linuxTime->tm_year),
								linuxTime->tm_hour,
								linuxTime->tm_min,
								linuxTime->tm_sec);
		SocketWriteData(mySocketFD,	lineBuff);

		sprintf(lineBuff, "<TD>%s</TD>",	gEventLog[iii].eventName);
		SocketWriteData(mySocketFD,	lineBuff);


		sprintf(lineBuff, "<TD>%s</TD>",	gEventLog[iii].eventDescription);
		SocketWriteData(mySocketFD,	lineBuff);

		if (gEventLog[iii].alpacaErrCode != 0)
		{
			sprintf(lineBuff, "<TD>0x%03X/%d</TD>",	gEventLog[iii].alpacaErrCode, gEventLog[iii].alpacaErrCode);

			errorTotal++;
			errIndx	=	gEventLog[iii].alpacaErrCode - kASCOM_Err_NotImplemented;
			if ((errIndx >= 0) && (errIndx < kMaxErrors))
			{
				errorCounts[errIndx]++;
			}
		}
		else
		{
			strcpy(lineBuff, "<TD><CENTER>-</TD>");
		}
		SocketWriteData(mySocketFD,	lineBuff);

		sprintf(lineBuff, "<TD>%s</TD>",	gEventLog[iii].errorString);
		SocketWriteData(mySocketFD,	lineBuff);


		SocketWriteData(mySocketFD,	"</TR>\r\n");
	}

	SocketWriteData(mySocketFD,	"<TR>\r\n");
	sprintf(lineBuff, "<TD COLSPAN=5>Total entries %d, max=%d</TD>",	gLogIndex, kMaxLogEntries);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	SocketWriteData(mySocketFD,	"<P>\r\n");

	//--------------------------------------------------------------------
	//*	print out the error code meanings
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
		SocketWriteData(mySocketFD,	"<TR><TH COLSPAN=3>Error Counts</TH></TR>\r\n");

		SocketWriteData(mySocketFD,	"<TR>\r\n");
		sprintf(lineBuff, "<TD>0x%03X/%d</TD>",	kASCOM_Err_NotImplemented, kASCOM_Err_NotImplemented);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<TD>NotImplemented</TD>");

		errIndx	=	kASCOM_Err_NotImplemented - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<TD>%d</TD>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</TR>\r\n");

		SocketWriteData(mySocketFD,	"<TR>\r\n");
		sprintf(lineBuff, "<TD>0x%03X/%d</TD>",	kASCOM_Err_InvalidValue, kASCOM_Err_InvalidValue);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<TD>InvalidValue</TD>");
		errIndx	=	kASCOM_Err_InvalidValue - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<TD>%d</TD>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</TR>\r\n");


		SocketWriteData(mySocketFD,	"<TR>\r\n");
		sprintf(lineBuff, "<TD>0x%03X/%d</TD>",	kASCOM_Err_ValueNotSet, kASCOM_Err_ValueNotSet);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<TD>ValueNotSet</TD>");
		errIndx	=	kASCOM_Err_ValueNotSet - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<TD>%d</TD>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</TR>\r\n");


		SocketWriteData(mySocketFD,	"<TR>\r\n");
		sprintf(lineBuff, "<TD>0x%03X/%d</TD>",	kASCOM_Err_NotConnected, kASCOM_Err_NotConnected);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<TD>NotConnected</TD>");
		errIndx	=	kASCOM_Err_NotConnected - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<TD>%d</TD>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</TR>\r\n");


		SocketWriteData(mySocketFD,	"<TR>\r\n");
		sprintf(lineBuff, "<TD>0x%03X/%d</TD>",	kASCOM_Err_InvalidWhileParked, kASCOM_Err_InvalidWhileParked);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<TD>InvalidWhileParked</TD>");
		errIndx	=	kASCOM_Err_InvalidWhileParked - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<TD>%d</TD>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</TR>\r\n");


		SocketWriteData(mySocketFD,	"<TR>\r\n");
		sprintf(lineBuff, "<TD>0x%03X/%d</TD>",	kASCOM_Err_InvalidWhileSlaved, kASCOM_Err_InvalidWhileSlaved);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<TD>InvalidWhileSlaved</TD>");
		errIndx	=	kASCOM_Err_InvalidWhileSlaved - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<TD>%d</TD>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</TR>\r\n");


		SocketWriteData(mySocketFD,	"<TR>\r\n");
		sprintf(lineBuff, "<TD>0x%03X/%d</TD>",	kASCOM_Err_InvalidOperation, kASCOM_Err_InvalidOperation);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<TD>InvalidOperation</TD>");
		errIndx	=	kASCOM_Err_InvalidOperation - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<TD>%d</TD>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</TR>\r\n");

		SocketWriteData(mySocketFD,	"<TR>\r\n");
		sprintf(lineBuff, "<TD>0x%03X/%d</TD>",	kASCOM_Err_ActionNotImplemented, kASCOM_Err_ActionNotImplemented);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<TD>ActionNotImplemented</TD>");
		errIndx	=	kASCOM_Err_ActionNotImplemented - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<TD>%d</TD>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</TR>\r\n");

		SocketWriteData(mySocketFD,	"<TR>\r\n");
		sprintf(lineBuff, "<TD>0x%03X/%d</TD>",	kASCOM_Err_NotInCacheException, kASCOM_Err_NotInCacheException);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<TD>NotInCacheException</TD>");
		errIndx	=	kASCOM_Err_NotInCacheException - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<TD>%d</TD>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</TR>\r\n");

		SocketWriteData(mySocketFD,	"<TR>\r\n");
		sprintf(lineBuff, "<TD>0x%03X/%d</TD>",	kASCOM_Err_UnspecifiedError, kASCOM_Err_UnspecifiedError);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"<TD>UnspecifiedError</TD>");
		errIndx	=	kASCOM_Err_UnspecifiedError - kASCOM_Err_NotImplemented;
		sprintf(lineBuff, "<TD>%d</TD>",	errorCounts[errIndx]);
		SocketWriteData(mySocketFD,	lineBuff);
		SocketWriteData(mySocketFD,	"</TR>\r\n");

	SocketWriteData(mySocketFD,	"</TABLE>\r\n");


	SocketWriteData(mySocketFD,	"</CENTER>\r\n");

}
