//**************************************************************************
//*	Name:			alpacadriverSetup.cpp
//*
//*	Author:			Mark Sproul (C) 2022-2023
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
//*	Dec  1,	2022	<MLS> Created alpacadriverSetup.c
//*	Dec  2,	2022	<MLS> Added Setup_OutputForm(), Setup_Save(), Setup_ProcessCommand()
//*	Dec  2,	2022	<MLS> HTML form generation and processing working
//*	Dec  4,	2022	<MLS> Added Setup_SaveInit()
//*	Dec  5,	2022	<MLS> Added Setup_OutputCheckBox() & Setup_OutputRadioBtn()
//*	Dec  5,	2022	<MLS> Added Setup_SaveFinish()
//*****************************************************************************


#include	<stdio.h>
#include	<stdlib.h>
//#include	<ctype.h>
#include	<string.h>
//#include	<sys/time.h>
//#include	<sys/resource.h>
//#include	<time.h>
//#include	<gnu/libc-version.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"html_common.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"

//*****************************************************************************
const char	gHtmlHeader[]	=
{
	"HTTP/1.0 200 \r\n"
//	"Server: alpaca\r\n"
//	"Mime-Version: 1.0\r\n"
	"User-Agent: AlpacaPi\r\n"
	"Content-Type: text/html\r\n"
	"Connection: close\r\n"
	"\r\n"
	"<!DOCTYPE html>\r\n"
	"<HTML><HEAD>\r\n"



};

//*****************************************************************************
//*	https://www.w3schools.com/html/html_forms.asp
//*****************************************************************************
bool	AlpacaDriver::Setup_OutputForm(TYPE_GetPutRequestData *reqData, const char *formActionString)
{
int		mySocketFD;
char	lineBuff[256];

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(formActionString);

	mySocketFD	=	reqData->socket;

	SocketWriteData(mySocketFD,	gHtmlHeader);
	SocketWriteData(mySocketFD,	"<!DOCTYPE html>\r\n");
	SocketWriteData(mySocketFD,	"<HTML>\r\n");
	SocketWriteData(mySocketFD,	"<TITLE>Form Testing</TITLE>\r\n");
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H1>Form Testing</H1>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");
//	SocketWriteData(mySocketFD,	"<form action=\"/setup/common\" target=\"_blank\">\r\n");
	sprintf(lineBuff, "<form action=\"%s\" target=\"_blank\">\r\n", formActionString);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"  <label for=\"fname\">First name:</label><br>\r\n");
	SocketWriteData(mySocketFD,	"  <input type=\"text\" id=\"fname\" name=\"fname\" value=\"John\"><br>\r\n");
	SocketWriteData(mySocketFD,	"  <label for=\"lname\">Last name:</label><br>\r\n");
	SocketWriteData(mySocketFD,	"  <input type=\"text\" id=\"lname\" name=\"lname\" value=\"Doe\"><br><br>\r\n");
	SocketWriteData(mySocketFD,	"  <input type=\"submit\" value=\"Submit\">\r\n");

	SocketWriteData(mySocketFD,	"</form>\r\n");
	return(true);
}

//*****************************************************************************
int		AlpacaDriver::Setup_Save(TYPE_GetPutRequestData *reqData)
{
int		iii;
int		ccc;
char	keyword[128];
char	valueString[128];
int		sLen;
char	theChar;
bool	workingOnKeyword;
int		valuePairCount;
int		mySocketFD;

	CONSOLE_DEBUG(__FUNCTION__);

	Setup_SaveInit();

	valuePairCount		=	0;
	sLen				=	strlen(reqData->contentData);
	workingOnKeyword	=	true;
	keyword[0]			=	0;
	valueString[0]		=	0;
	iii					=	0;
	ccc					=	0;
	while (iii <= sLen)
	{
		theChar	=	reqData->contentData[iii++];
		if ((theChar == '&') || (theChar == 0))
		{
			Setup_ProcessKeyword(keyword, valueString);
			valuePairCount++;
			//*	reset for the next round
			ccc					=	0;
			workingOnKeyword	=	true;
			keyword[0]			=	0;
			valueString[0]		=	0;
		}
		else if (theChar == '=')
		{
			workingOnKeyword	=	false;
			ccc					=	0;
		}
		else if (workingOnKeyword)
		{
			keyword[ccc++]	=	theChar;
			keyword[ccc]	=	0;
		}
		else
		{
			valueString[ccc++]	=	theChar;
			valueString[ccc]	=	0;
		}
	}

	mySocketFD	=	reqData->socket;


	SocketWriteData(mySocketFD,	gHtmlHeader);

	SocketWriteData(mySocketFD,	"<!DOCTYPE html>\r\n");
	SocketWriteData(mySocketFD,	"<HTML>\r\n");
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H1>Saved succesfully</H1>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");

	Setup_SaveFinish();

	return(valuePairCount);
}

//*****************************************************************************
bool	AlpacaDriver::Setup_ProcessCommand(TYPE_GetPutRequestData *reqData)
{
char				formActionString[128];

	CONSOLE_DEBUG(__FUNCTION__);

	//*	build the form action string,
	//*	this MUST be used by each form so that the processing can be handled properly

	//*******************************************
	if (strcmp(reqData->deviceCommand, "save") == 0)
	{
		Setup_Save(reqData);
	}
	else if (strcmp(reqData->deviceCommand, "setup") == 0)
	{
		//*		/setup/v1/{device_type}/{device_number}/setup
		sprintf(formActionString,	"/setup/v1/%s/%d/save",
									cAlpacaName,
									cDeviceNum);
		tolowerStr(formActionString);
		Setup_OutputForm(reqData, formActionString);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Unrecognized setup command", reqData->deviceCommand);
	}

	return(false);
}

//*****************************************************************************
//*	this routine gets called on the START of a SAVE command so that
//*	the class can set any defaults
//*****************************************************************************
//*	this should be overridden if setup is supported
//*****************************************************************************
void	AlpacaDriver::Setup_SaveInit(void)
{
//*	this should be overridden if setup is supported
	CONSOLE_DEBUG(__FUNCTION__);

}

//*****************************************************************************
//*	this routine gets called on the END of a SAVE command so that
//*	the class process the results
//*****************************************************************************
//*	this should be overridden if setup is supported
//*****************************************************************************
void	AlpacaDriver::Setup_SaveFinish(void)
{
//*	this should be overridden if setup is supported
	CONSOLE_DEBUG(__FUNCTION__);

}


//*****************************************************************************
//*	this should be overridden if setup is supported
//*****************************************************************************
bool	AlpacaDriver::Setup_ProcessKeyword(const char *keyword, const char *valueString)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_2STR("kw:value", keyword, valueString);


	return(false);
}

//*****************************************************************************
void	AlpacaDriver::Setup_OutputCheckBox(	const int	socketFD,
											const char	*name,
											const char	*displayedName,
											const bool	checked)
{
char	htmlOutputString[128];

	strcpy(htmlOutputString,	"<input type=\"checkbox\" name=\"");
	strcat(htmlOutputString,	name);
	strcat(htmlOutputString,	"\" value=\"true\" ");
	if (checked)
	{
		strcat(htmlOutputString,	" checked=\"checked\"");
	}
	strcat(htmlOutputString,	">");
	strcat(htmlOutputString,	displayedName);
	strcat(htmlOutputString,	"<BR>\r\n");
	SocketWriteData(socketFD,	htmlOutputString);
}

//*****************************************************************************
void	AlpacaDriver::Setup_OutputRadioBtn(	const int	socketFD,
											const char	*groupName,
											const char	*name,
											const char	*displayedName,
											const bool	checked)
{
char	htmlOutputString[128];

//sprintf(lineBuff,	"<input type=\"radio\" id=\"%s\" name=\"target\" value=\"%s\">%s<BR>\r\n",
//										gTargetNames[iii].name,
//										gTargetNames[iii].prefix,
//										gTargetNames[iii].name);
//
	strcpy(htmlOutputString,	"<input type=\"radio\" name=\"");
	strcat(htmlOutputString,	groupName);
	strcat(htmlOutputString,	"\" value=\"");
	strcat(htmlOutputString,	name);
	strcat(htmlOutputString,	"\"");
	if (checked)
	{
		strcat(htmlOutputString,	" checked=\"checked\"");
	}
	strcat(htmlOutputString,	">");
	strcat(htmlOutputString,	displayedName);
	strcat(htmlOutputString,	"<BR>\r\n");
	SocketWriteData(socketFD,	htmlOutputString);
}

