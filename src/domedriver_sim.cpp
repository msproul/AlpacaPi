//**************************************************************************
//*	Name:			domedriver_sim.cpp
//*
//*	Author:			Mark Sproul (C) 2023
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar  2,	2023	<MLS> Created domedriver_sim.cpp
//*****************************************************************************

#ifdef _ENABLE_DOME_SIMULATOR_

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<unistd.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"RequestData.h"
#include	"JsonResponse.h"
#include	"eventlogging.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"domedriver.h"
#include	"domedriver_sim.h"


//*****************************************************************************
void	CreateDomeObjectsSIM(void)
{
	new DomeDriverSIM(0);
}

//**************************************************************************************
DomeDriverSIM::DomeDriverSIM(const int argDevNum)
	:DomeDriver(argDevNum)
{
	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name,		"AlpacaPi Dome Simulator");
	strcpy(cCommonProp.Description,	"AlpacaPi Dome Simulator");
	strcpy(gWebTitle, "AlpacaPi Dome Simulator");

	cDomeConfig					=	kIsDome;
	cDomeProp.CanFindHome		=	true;
	cDomeProp.CanPark			=	true;
	cDomeProp.CanSetAzimuth		=	true;
	cDomeProp.CanSyncAzimuth	=	true;
	cDomeProp.CanSetShutter		=	true;
	cParkAzimuth				=	170.0;		//*	these are approximate for my dome
	cHomeAzimuth				=	230.0;
	Init_Hardware();
	LogEvent(	"dome",
				"R-Pi Dome created",
				NULL,
				kASCOM_Err_Success,
				"");

}

//**************************************************************************************
// Destructor
//**************************************************************************************
DomeDriverSIM::~DomeDriverSIM( void )
{
}


//*****************************************************************************
void	DomeDriverSIM::Init_Hardware(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}




#endif // _ENABLE_DOME_SIMULATOR_
