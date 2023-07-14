//**************************************************************************
//*	Name:			focuserdriver_sim.cpp
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
//*	Re-distribution of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar  3,	2023	<MLS> Created focuserdriver_sim.cpp
//*	Jul  8,	2023	<MLS> Added SetFocuserPosition() to simulator
//*****************************************************************************

#ifdef _ENABLE_FOCUSER_SIMULATOR_

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<errno.h>
#include	<dirent.h>


#define _DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"focuserdriver.h"
#include	"focuserdriver_sim.h"



//*****************************************************************************
int		CreateFocuserObjects_SIM(void)
{
	new FocuserDriverSIM();

	return(1);
}

//**************************************************************************************
FocuserDriverSIM::FocuserDriverSIM(void)
	:FocuserDriver()
{

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name,		"AlpacaPi Focuser Simulator");
	strcpy(cCommonProp.Description,	"AlpacaPi Focuser Simulator");
	cFocuserHasTemperature		=	false;
	cFocuserProp.StepSize		=	((0.00016 * 25.4) * 1000);	//	Step size (microns) for the focuser.
	strcpy(cDeviceManufacturer,	"AlpacaPi");

	cFocuserProp.Position			=	4570;
	cFocuserProp.Temperature_DegC	=	22.3;
	cFocuserProp.Absolute			=	true;
	cFocuserProp.MaxIncrement		=	10000;
	cFocuserProp.MaxStep			=	87000;
	cFocuserProp.StepSize			=	0.2667;	//	Step size (microns) for the focuser.


	cUUID.part1				=	'ALPI';					//*	4 byte manufacturer code
}

//**************************************************************************************
// Destructor
//**************************************************************************************
FocuserDriverSIM::~FocuserDriverSIM(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
int32_t	FocuserDriverSIM::RunStateMachine(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

//	if (cSendHaltCmd || cSendMoveCmd)
//	{
//		ProcessQueuedCommands();
//	}
//	else
//	{
//		ProcessPeriodicRequests();
//	}

	return(1000 * 1000);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriverSIM::SetFocuserPosition(const int32_t newPosition, char *alpacaErrMsg)
{
	cFocuserProp.Position	=	newPosition;

	return(kASCOM_Err_Success);
}


#endif	//	_ENABLE_FOCUSER_SIMULATOR_


