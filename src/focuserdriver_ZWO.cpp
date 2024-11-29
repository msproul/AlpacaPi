//**************************************************************************
//*	Name:			focuserdriver_ZWO.cpp
//*
//*	Author:			Mark Sproul (C) 2024
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
//*	Jul 25,	2024	<MLS> Created focuserdriver_ZWO.cpp
//*	Nov 28,	2024	<MLS> Working on ZWO focuers driver (EAF)
//*	Nov 28,	2024	<MLS> ZWO EAF Focus movement working
//*	Nov 28,	2024	<MLS> ZWO EAF Temperature working
//*	Nov 29,	2024	<MLS> Added ProcessEAFerror()
//*	Nov 29,	2024	<MLS> Hot swapping works, must be plugged in at startup
//*****************************************************************************

#ifdef _ENABLE_FOCUSER_ZWO_

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

#include	"EAF_focuser.h"		//*	ZWO header file for EAF

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"focuserdriver.h"
#include	"focuserdriver_ZWO.h"
#include	"eventlogging.h"



//*****************************************************************************
int		CreateFocuserObjects_ZWO(void)
{
int		iii;
int		eaf_count;
char	rulesFileName[]	=	"eaf.rules";
bool	rulesFileOK;
char	driverVersionString[64];

	CONSOLE_DEBUG(__FUNCTION__);
	rulesFileOK	=	Check_udev_rulesFile(rulesFileName);
	if (rulesFileOK == false)
	{
		LogEvent(	"focuser",
					"Problem with ZWO focuser rules file",
					NULL,
					kASCOM_Err_Success,
					rulesFileName);
	}

	strcpy(driverVersionString,	EAFGetSDKVersion());
	LogEvent(	"focuser",
				"Library version (ZWO-EAF)",
				NULL,
				kASCOM_Err_Success,
				driverVersionString);
	AddLibraryVersion("focuser", "ZWO-EAF", driverVersionString);

	eaf_count	=	EAFGetNum();
	CONSOLE_DEBUG_W_NUM("eaf_count\t=", eaf_count);
	for (iii=0; iii < eaf_count; iii++)
	{
		new FocuserDriverZWO(iii);
	}

	return(1);
}

//**************************************************************************************
FocuserDriverZWO::FocuserDriverZWO(const int eaf_ID_num)
	:FocuserDriver()
{
	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name,		"AlpacaPi Focuser ZWO");
	strcpy(cCommonProp.Description,	"AlpacaPi Focuser ZWO");
	cFocuserHasTemperature		=	true;
	cFocuserProp.StepSize		=	((0.00016 * 25.4) * 1000);	//	Step size (microns) for the focuser.
	strcpy(cDeviceManufacturer,	"AlpacaPi");

	cFocuserProp.Position			=	0;
	cFocuserProp.Temperature_DegC	=	22.3;
	cFocuserProp.Absolute			=	true;
	cFocuserProp.MaxIncrement		=	10000;
	cFocuserProp.MaxStep			=	10000;
	cFocuserProp.StepSize			=	0.2667;	//	Step size (microns) for the focuser.

	cUUID.part1						=	'ZWO_';					//*	4 byte manufacturer code

	cEAFconnectionIsOpen			=	false;
	cEAF_ID_num						=	eaf_ID_num;
	OpenFocuserConnection();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
FocuserDriverZWO::~FocuserDriverZWO(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
//*	returns true if open succeeded.
bool	FocuserDriverZWO::OpenFocuserConnection(void)
{
int		eaf_RetCode;
int		currentPos;

	CONSOLE_DEBUG(__FUNCTION__);
	eaf_RetCode	=	EAFGetID(cEAF_ID_num, &cEAFInfo.ID);
	eaf_RetCode	=	EAFOpen(cEAFInfo.ID);
	if (eaf_RetCode == EAF_SUCCESS)
	{
		cEAFconnectionIsOpen			=	true;
		EAFGetProperty(cEAFInfo.ID, &cEAFInfo);

		CONSOLE_DEBUG_W_NUM("cEAF_ID_num     \t=",	cEAF_ID_num);
		CONSOLE_DEBUG_W_NUM("cEAFInfo.ID     \t=",	cEAFInfo.ID);
		CONSOLE_DEBUG_W_NUM("cEAFInfo.MaxStep\t=",	cEAFInfo.MaxStep);
		CONSOLE_DEBUG_W_STR("cEAFInfo.Name   \t=",	cEAFInfo.Name);
		cFocuserProp.MaxStep			=	cEAFInfo.MaxStep;
		eaf_RetCode	=	EAFGetPosition(cEAFInfo.ID, &currentPos);
		if (eaf_RetCode == EAF_SUCCESS)
		{
			cFocuserProp.Position	=	currentPos;
		}
	}
	else
	{
		cEAFconnectionIsOpen			=	false;
		CONSOLE_DEBUG_W_NUM("eaf_RetCode\t=",	eaf_RetCode);
	}
	return(cEAFconnectionIsOpen);
}

//*****************************************************************************
int32_t	FocuserDriverZWO::RunStateMachine(void)
{
bool		bMoving			=	false;
bool		pbHandControl	=	false;
int			eaf_RetCode;
int			currentPos;
float		fTemp;
uint32_t	currentMillis;
uint32_t	currentSeconds;
int			eaf_count;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cEAFconnectionIsOpen == false)
	{
		//*	something has happened and we lost connection
		eaf_count	=	EAFGetNum();
		if (eaf_count > 0)
		{
			OpenFocuserConnection();
		}

	}
	currentMillis	=	millis();
	currentSeconds	=	currentMillis / 1000;

	//---------------------------------------------------------------------
	eaf_RetCode	=	EAFIsMoving(cEAFInfo.ID, &bMoving, &pbHandControl);
	if (eaf_RetCode == EAF_SUCCESS)
	{
		cFocuserProp.IsMoving	=	bMoving;
	}
	else
	{
		ProcessEAFerror(eaf_RetCode, __FUNCTION__, "EAFIsMoving() failed:");
	}

	//---------------------------------------------------------------------
	eaf_RetCode	=	EAFGetPosition(cEAFInfo.ID, &currentPos);
	if (eaf_RetCode == EAF_SUCCESS)
	{
		cFocuserProp.Position	=	currentPos;
	}
	else
	{
		ProcessEAFerror(eaf_RetCode, __FUNCTION__, "EAFGetPosition() failed:");
	}

	//---------------------------------------------------------------------
	//*	check the temperature every 15 seconds
	if ((currentSeconds - cLastTimeSecs_Temperature) > 15)
	{
		eaf_RetCode	=	EAFGetTemp(cEAFInfo.ID, &fTemp);
		if (eaf_RetCode == EAF_SUCCESS)
		{
			cFocuserProp.Temperature_DegC	=	fTemp;
			TemperatureLog_AddEntry(cFocuserProp.Temperature_DegC);
//			CONSOLE_DEBUG_W_DBL("cFocuserProp.Temperature_DegC\t=",	cFocuserProp.Temperature_DegC);
		}
		else
		{
			ProcessEAFerror(eaf_RetCode, __FUNCTION__, "EAFGetTemp() failed:");
		}
		cLastTimeSecs_Temperature	=	currentSeconds;
	}

	return(1000 * 1000);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserDriverZWO::SetFocuserPosition(const int32_t newPosition, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					eaf_RetCode;

	CONSOLE_DEBUG_W_NUM("newPosition          \t=",	newPosition);
	CONSOLE_DEBUG_W_NUM("cFocuserProp.Position\t=",	cFocuserProp.Position);
	if (newPosition != cFocuserProp.Position)
	{
		CONSOLE_DEBUG("Calling EAFMove()");
		eaf_RetCode	=	EAFMove(cEAFInfo.ID, newPosition);
		if (eaf_RetCode == EAF_SUCCESS)
		{
			alpacaErrCode			=   kASCOM_Err_Success;
			cFocuserProp.IsMoving	=	true;
		}
		else
		{
			alpacaErrCode	=   kASCOM_Err_UnspecifiedError;
			CONSOLE_DEBUG_W_NUM("eaf_RetCode\t=",	eaf_RetCode);
//			CONSOLE_ABORT("EAFMove failed");
		}
	}
	else
	{
		CONSOLE_DEBUG("NO move required");
		alpacaErrCode			=   kASCOM_Err_Success;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
const char	*gZWO_EAF_errorMsgs[]	=
{
	"EAF_SUCCESS",
	"EAF_ERROR_INVALID_INDEX",
	"EAF_ERROR_INVALID_ID",
	"EAF_ERROR_INVALID_VALUE",
	"EAF_ERROR_REMOVED",		//failed to find the focuser, maybe the focuser has been removed
	"EAF_ERROR_MOVING",			//focuser is moving
	"EAF_ERROR_ERROR_STATE",	//focuser is in error state
	"EAF_ERROR_GENERAL_ERROR",	//other error
	"EAF_ERROR_NOT_SUPPORTED",
	"EAF_ERROR_CLOSED",
	"",
	"",
	"",
	"",

};

//*****************************************************************************
void	FocuserDriverZWO::ProcessEAFerror(const int eaf_ErrorCode, const char *functionName, const char *errorMssg)
{

	switch(eaf_ErrorCode)
	{
		case EAF_SUCCESS:
			break;

		case EAF_ERROR_REMOVED:			//failed to find the focuser, maybe the focuser has been removed
		case EAF_ERROR_INVALID_ID:
			cEAFconnectionIsOpen	=	false;
			break;


		case EAF_ERROR_INVALID_INDEX:
		case EAF_ERROR_INVALID_VALUE:
		case EAF_ERROR_MOVING:			//focuser is moving
		case EAF_ERROR_ERROR_STATE:		//focuser is in error state
		case EAF_ERROR_GENERAL_ERROR:	//other error
		case EAF_ERROR_NOT_SUPPORTED:
		case EAF_ERROR_CLOSED:
		case EAF_ERROR_END:
		default:
			CONSOLE_DEBUG_W_NUM("EAF error code\t=", eaf_ErrorCode);
			if ((eaf_ErrorCode >= 0) && (eaf_ErrorCode <= EAF_ERROR_CLOSED))
			{
				CONSOLE_DEBUG_W_STR("EAF error code\t=", gZWO_EAF_errorMsgs[eaf_ErrorCode]);
			}
			break;
	}
}


#endif	//	_ENABLE_FOCUSER_ZWO_


