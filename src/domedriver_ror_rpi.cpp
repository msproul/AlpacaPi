//**************************************************************************
//*	Name:			domedriver_ror_rpi.cpp
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Dec  2,	2020	<MLS> Created domedriver_ror_rpi.cpp
//*	Dec  2,	2020	<MLS> Started on Roll Off Roof implementation on R-Pi
//*	Jan 12,	2021	<MLS> Started working on implementation for Chris A.
//*	Jan 12,	2021	<MLS> Relay board working for open/close ROR
//*	Jan 12,	2021	<MLS> CONFORM-dome/ror -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	Jun 15,	2021	<MLS> Added GetPower() & GetAuxiliary()
//*****************************************************************************

#ifdef _ENABLE_ROR_

#define	_CHRIS_A_ROLL_OFF_ROOF_

#ifdef _CHRIS_A_ROLL_OFF_ROOF_

	//*	this is a specific implementation for Chris A of the Netherlands

	#define		kRelay_RoofPower	1
	#define		kRelay_RoofOpen		2
	#define		kRelay_RoofClose	3
	#define		kRelay_FlatScren	4


	#define		kSwitchDelaySeconds	20

	#include	"raspberrypi_relaylib.h"

	#ifndef _ENABLE_4REALY_BOARD
//		#error	"_ENABLE_4REALY_BOARD should be enabled"
	#endif // _ENABLE_4REALY_BOARD

#endif // _CHRIS_A_ROLL_OFF_ROOF_


#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"RequestData.h"
#include	"JsonResponse.h"
#include	"eventlogging.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"domedriver.h"
#include	"domedriver_ror_rpi.h"



#if defined(__arm__)
	#define	_ENABLE_DOME_HARDWARE_
	#include <wiringPi.h>
#else
	#define	LOW		0
	#define	HIGH	1
#endif


//*****************************************************************************
void	CreateDomeObjectsROR(void)
{
	new DomeDriverROR(0);
}



//**************************************************************************************
DomeDriverROR::DomeDriverROR(const int argDevNum)
	:DomeDriver(argDevNum)
{
	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name, "Dome-Roll-Off-Roof");
	strcpy(gWebTitle, "Dome-Roll-Off-Roof");

	cDomeConfig					=	kIsRollOffRoof;
	cDomeProp.AtPark			=	true;	//*	for testing
	cDomeProp.Azimuth			=	0.0;	//*	Azimuth is meaningless for a ROR
	cDomeProp.CanSyncAzimuth	=	false;
	cDomeProp.CanSetShutter		=	true;

	//*	local stuff
	cRORisOpening			=	false;
	cRORisClosing			=	false;

	Init_Hardware();

	CONSOLE_DEBUG("Returned from Init_Hardware()");

	strcpy(cCommonProp.Name,		"AlpacaPi-ROR");
	strcpy(cCommonProp.Description,	"Roll Off Roof");

	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
DomeDriverROR::~DomeDriverROR( void )
{
}

//*****************************************************************************
void	DomeDriverROR::Init_Hardware(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _CHRIS_A_ROLL_OFF_ROOF_
	cRelayCount	=	RpiRelay_Init();

	CONSOLE_DEBUG_W_NUM("cRelayCount\t=", cRelayCount);

#endif // _CHRIS_A_ROLL_OFF_ROOF_

}

//*****************************************************************************
int32_t	DomeDriverROR::RunStateMachine_ROR(void)
{
uint32_t	deltaMilliSecs;
uint32_t	currentMilliSecs;
int32_t		minDealy_microSecs;

	minDealy_microSecs		=	1000;		//*	default to 1 millisecond


//	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _CHRIS_A_ROLL_OFF_ROOF_
bool		relayOK;

	currentMilliSecs	=	millis();
	deltaMilliSecs		=	currentMilliSecs - cTimeOfLastOpenClose;
	if (cRORisOpening && (deltaMilliSecs > (kSwitchDelaySeconds * 1000)))
	{
		CONSOLE_DEBUG("Done with OPEN switch contact");
		relayOK		=	RpiRelay_SetRelay(kRelay_RoofOpen, false);
		if (relayOK == false)
		{
			CONSOLE_DEBUG("RpiRelay_SetRelay returned false");
		}
		cDomeProp.ShutterStatus	=	kShutterStatus_Open;
		cDomeProp.Slewing		=	false;
		cRORisOpening			=	false;
	}

	if (cRORisClosing && (deltaMilliSecs > (kSwitchDelaySeconds * 1000)))
	{
		CONSOLE_DEBUG("Done with CLOSE switch contact");
		relayOK		=	RpiRelay_SetRelay(kRelay_RoofClose, false);
		if (relayOK == false)
		{
			CONSOLE_DEBUG("RpiRelay_SetRelay returned false");
		}
		cDomeProp.ShutterStatus	=	kShutterStatus_Closed;
		cDomeProp.Slewing		=	false;
		cRORisClosing			=	false;

	}
#endif // _CHRIS_A_ROLL_OFF_ROOF_
	return(minDealy_microSecs);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverROR::SetPower(bool onOffFlag)
{
bool				relayOK;

	relayOK		=	RpiRelay_SetRelay(kRelay_RoofPower, onOffFlag);
	if (relayOK == false)
	{
		CONSOLE_DEBUG("RpiRelay_SetRelay returned false");
	}
	return(kASCOM_Err_Success);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverROR::GetPower(bool *onOffFlag)
{

	*onOffFlag	=	RpiRelay_GetRelay(kRelay_RoofPower);
	return(kASCOM_Err_Success);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverROR::SetAuxiliary(bool onOffFlag)
{
bool				relayOK;

	relayOK		=	RpiRelay_SetRelay(kRelay_FlatScren, onOffFlag);
	if (relayOK == false)
	{
		CONSOLE_DEBUG("RpiRelay_SetRelay returned false");
	}
	return(kASCOM_Err_Success);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverROR::GetAuxiliary(bool *onOffFlag)
{

	*onOffFlag	=	RpiRelay_GetRelay(kRelay_FlatScren);
	return(kASCOM_Err_Success);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverROR::OpenShutter(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
bool				relayOK;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _CHRIS_A_ROLL_OFF_ROOF_

	//*	turn the power on
//	relayOK					=	RpiRelay_SetRelay(kRelay_RoofPower,	true);
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofOpen,	true);
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofClose, false);
	if (relayOK == false)
	{
		CONSOLE_DEBUG("RpiRelay_SetRelay returned false");
	}
	alpacaErrCode			=	kASCOM_Err_Success;
	cTimeOfLastOpenClose	=	millis();
	cRORisOpening			=	true;
	cDomeProp.Slewing		=	true;
	cDomeProp.ShutterStatus	=	kShutterStatus_Opening;


#else
	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Open shutter not implemented");
#endif
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverROR::CloseShutter(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
bool				relayOK;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _CHRIS_A_ROLL_OFF_ROOF_

	//*	turn the power on
//	relayOK					=	RpiRelay_SetRelay(kRelay_RoofPower,	true);
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofClose,	true);
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofOpen,	false);
	if (relayOK == false)
	{
		CONSOLE_DEBUG("RpiRelay_SetRelay returned false");
	}
	alpacaErrCode			=	kASCOM_Err_Success;
	cTimeOfLastOpenClose	=	millis();
	cRORisClosing			=	true;
	cDomeProp.Slewing		=	true;
	cDomeProp.ShutterStatus	=	kShutterStatus_Closing;

#else
	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Open shutter not implemented");
#endif
	return(alpacaErrCode);
}


//*****************************************************************************
void	DomeDriverROR::StopDomeMoving(bool rightNow)
{
bool				relayOK;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _CHRIS_A_ROLL_OFF_ROOF_

	//*	turn it all off
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofOpen, false);
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofClose, false);
//	relayOK					=	RpiRelay_SetRelay(kRelay_RoofPower, false);
	if (relayOK == false)
	{
		CONSOLE_DEBUG("RpiRelay_SetRelay returned false");
	}
	cRORisOpening			=	false;
	cRORisClosing			=	false;
	cDomeProp.Slewing		=	false;

#endif
}



#endif // _ENABLE_ROR_
