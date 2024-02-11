//**************************************************************************
//*	Name:			alpacadriverThread.cpp
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
//*	Sep 20,	2023	<MLS> Created alpacadriverThread.cpp
//*	Sep 20,	2023	<MLS> Added StartDriverThread()
//*	Sep 21,	2023	<MLS> Added StopDriverThread()
//*****************************************************************************


#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"helper_functions.h"
#include	"html_common.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"


//*****************************************************************************
static void	*AplacaDriverThread(void *arg)
{
AlpacaDriver	*alpacaDriverPtr;

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaDriverPtr	=	(AlpacaDriver *)arg;
	if (alpacaDriverPtr != NULL)
	{
		if (alpacaDriverPtr->cMagicCookie == kMagicCookieValue)
		{
			alpacaDriverPtr->RunThread();
		}
		else
		{
			CONSOLE_DEBUG("cMagicCookie is invalid  !!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		}
	}
	else
	{
		CONSOLE_DEBUG("alpacaDriverPtr is NULL  !!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	}
	CONSOLE_DEBUG("We should NOT be here!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

	return(NULL);
}

//*****************************************************************************
void	AlpacaDriver::RunThread(void)
{

	CONSOLE_DEBUG(__FUNCTION__);

	cDriverThreadIsActive	=	true;
	cDriverThreadLoopCnt	=	0;
	RunThread_Startup();
	while (cDriverThreadKeepRunning)
	{
		RunThread_Loop();
		usleep(1000);

		cDriverThreadLoopCnt++;
	}
	//*	normally we dont ever get here,
	//*	however, the telescopedriver_comm will exit the thread and let it restart
	cDriverThreadIsActive	=	false;
}


//*****************************************************************************
void	AlpacaDriver::StartDriverThread(void)
{
int	threadErr;

	cDriverThreadKeepRunning	=	true;
	threadErr					=	pthread_create(&cDriverThreadID, NULL, &AplacaDriverThread, this);
	if (threadErr != 0)
	{
		CONSOLE_DEBUG_W_NUM("ERROR: pthread_create() returned\t=", threadErr);
	}
}

//*****************************************************************************
void	AlpacaDriver::StopDriverThread(void)
{
	cDriverThreadKeepRunning	=	false;
}

//*****************************************************************************
void	AlpacaDriver::RunThread_Startup(void)
{
//	CONSOLE_DEBUG("this should be over-ridden");
}

//*****************************************************************************
void	AlpacaDriver::RunThread_Loop(void)
{
	CONSOLE_DEBUG("this should be over-ridden");
}
