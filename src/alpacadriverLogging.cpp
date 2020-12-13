//**************************************************************************
//*	Name:			alpacadriverLogging.cpp
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr  5,	2020	<MLS> Created alpacadriverLogging.cpp
//*	Apr  5,	2020	<MLS> Started working on error and conform logging
//*****************************************************************************

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"RequestData.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"



//*****************************************************************************
//*	log data to disk in TXT format with tabs.
//*	There are different log files, the format is the same
//*****************************************************************************
void	LogToDisk(const int whichLogFile, TYPE_GetPutRequestData *reqData)
{

	switch(whichLogFile)
	{
		case kLog_Error:
		case kLog_Conform:
			break;

		default:
			break;

	}
}


