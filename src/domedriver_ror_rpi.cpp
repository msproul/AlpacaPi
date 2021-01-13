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
//*****************************************************************************

#ifdef _ENABLE_ROR_

#define	_CHRIS_A_ROOL_OFF_ROOF_

#ifdef _CHRIS_A_ROOL_OFF_ROOF_

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

#endif // _CHRIS_A_ROOL_OFF_ROOF_


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
	strcpy(cDeviceName, "Dome-Roll-Off-Roof");
	strcpy(gWebTitle, "Dome-Roll-Off-Roof");

	cDomeConfig				=	kIsRollOffRoof;
	cAtPark					=	true;	//*	for testing
	cAzimuth_Degrees		=	123.0;	//*	for testing
	cCanSyncAzimuth			=	false;
	cCanSetShutter			=	true;

	//*	local stuff
	cRORisOpening			=	false;
	cRORisClosing			=	false;

	Init_Hardware();

	strcpy(cDeviceName,			"AlpacaPi-ROR");
	strcpy(cDeviceDescription,	"Rool Off Roof");


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

#ifdef _CHRIS_A_ROOL_OFF_ROOF_
	cRelayCount	=	RpiRelay_Init();

	CONSOLE_DEBUG_W_NUM("cRelayCount\t=", cRelayCount);

#endif // _CHRIS_A_ROOL_OFF_ROOF_

}

//*****************************************************************************
int32_t	DomeDriverROR::RunStateMachine_ROR(void)
{
uint32_t	deltaMilliSecs;
uint32_t	currentMilliSecs;
bool		relayOK;
int32_t		minDealy_microSecs;

	minDealy_microSecs		=	1000;		//*	default to 1 millisecond


//	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _CHRIS_A_ROOL_OFF_ROOF_

	currentMilliSecs	=	millis();
	deltaMilliSecs		=	currentMilliSecs - cTimeOfLastOpenClose;
	if (cRORisOpening && (deltaMilliSecs > (kSwitchDelaySeconds * 1000)))
	{
		CONSOLE_DEBUG("Done with OPEN switch contact");
		relayOK		=	RpiRelay_SetRelay(kRelay_RoofOpen, false);

		//*	if you want the power turned off, do it here
		relayOK		=	RpiRelay_SetRelay(kRelay_RoofPower, false);

		cShutterstatus	=	kShutterStatus_Open;
		cSlewing		=	false;
		cRORisOpening	=	false;
	}

	if (cRORisClosing && (deltaMilliSecs > (kSwitchDelaySeconds * 1000)))
	{
		CONSOLE_DEBUG("Done with CLOSE switch contact");
		relayOK		=	RpiRelay_SetRelay(kRelay_RoofClose, false);
		//*	if you want the power turned off, do it here
		relayOK		=	RpiRelay_SetRelay(kRelay_RoofPower, false);

		cShutterstatus	=	kShutterStatus_Closed;
		cSlewing		=	false;
		cRORisClosing	=	false;
	}
#endif // _CHRIS_A_ROOL_OFF_ROOF_
	return(minDealy_microSecs);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverROR::OpenShutter(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
bool				relayOK;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _CHRIS_A_ROOL_OFF_ROOF_

	//*	turn the power on
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofPower, true);
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofOpen, true);
	alpacaErrCode			=	kASCOM_Err_Success;
	cTimeOfLastOpenClose	=	millis();
	cRORisOpening			=	true;
	cSlewing				=	true;

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

#ifdef _CHRIS_A_ROOL_OFF_ROOF_

	//*	turn the power on
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofPower, true);
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofClose, true);
	alpacaErrCode			=	kASCOM_Err_Success;
	cTimeOfLastOpenClose	=	millis();
	cRORisClosing			=	true;
	cSlewing				=	true;

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

#ifdef _CHRIS_A_ROOL_OFF_ROOF_

	//*	turn it all off
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofOpen, false);
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofClose, false);
	relayOK					=	RpiRelay_SetRelay(kRelay_RoofPower, false);
	cRORisOpening			=	false;
	cRORisClosing			=	false;
	cSlewing				=	false;

#endif
}



#endif // _ENABLE_ROR_
