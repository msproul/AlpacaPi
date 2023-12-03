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
//*	Dec 23,	2021	<MLS> Added OutputHTML_Part2() to output hardware configuration
//*	Mar  2,	2023	<MLS> Added _ENABLE_DOME_ROR_
//*	Sep 24,	2023	<MLS> Adding support for Topens door opener
//*	Sep 24,	2023	<MLS> Added _TOPENS_ROLL_OFF_ROOF_
//*	Sep 24,	2023	<MLS> Started migrating to using background thread
//*	Oct  2,	2023	<MLS> Updated background thread open/close logic
//*	Nov 24,	2023	<MLS> Fixed bug in RunThread_Loop() as per pull request #28
//*	Nov 26,	2023	<SCV> Fixed Closing status bug RunThread_Loop() as per pull request #32
//*	Nov 26,	2023	<MLS> Support for topens ROR driver appears to be complete
//*****************************************************************************
//*****************************************************************************
//	After doing some experimenting with AlpacaPi,
//	I tried to operate the ROR Driver with a 4-relay module I had around.
//	This is where I discovered my one line to open and Open/Stop/Close the roof.
//	I think there is no sensor inputs (switch I mention above) in the code to tell that the roof is closed.
//	Think this code works with timing for opening and closing.
//
//	To start of, is there a driver embedded in AlpacaPi to perform the following:
//
//	Inputs from Mag Switch.
//
//		Mag Switch 1:
//			HIGH when Roof is closed
//			LOW when Roof is starting to open
//
//		Mag Switch 2:
//			HIGH when Roof is open
//			LOW when Roof is starting to close
//
//	Output to Relay
//
//		Trigger O/S/C 3 seconds receive API from client (PUT / 192.168.4.204:6800/api/v1/dome/0/openshutter
//		Trigger O/S/C 3 seconds receive API from client (PUT / 192.168.4.204:6800/api/v1/dome/0/closeshutter
//
//	Roof Status
//
//		Depending on the Mag switches the client will request for status on state of the roof.
//		Open/Closing/Close and Close/Opening/Open
//
//	192.168.4.204:6800/api/v1/dome/0/shutterstatus
//
//	If there is such driver, what driver is closest and the location in the code to modify it?
//
//	I deeply appreciate if you can help me out and point me in the right direction.
//
//	Thanks
//*****************************************************************************


#ifdef _ENABLE_DOME_ROR_
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>

//*	these are defined in the makefile
//#define	_CHRIS_A_ROLL_OFF_ROOF_
//#define	_TOPENS_ROLL_OFF_ROOF_


#if defined(_CHRIS_A_ROLL_OFF_ROOF_) && defined(_TOPENS_ROLL_OFF_ROOF_)
	#error "_CHRIS_A_ROLL_OFF_ROOF_ & _TOPENS_ROLL_OFF_ROOF_ cannot both enabled at the same time"
#endif

//*****************************************************************************
#ifdef _CHRIS_A_ROLL_OFF_ROOF_

	//*	this is a specific implementation for Chris A of the Netherlands

	#define		kRelay_RoofPower	1
	#define		kRelay_RoofOpen		2
	#define		kRelay_RoofClose	3

	//*	this is the default value, it can be changed from the web setup interface
	#define		kSwitchDelaySeconds	20

	#include	"raspberrypi_relaylib.h"

	#ifndef _ENABLE_4REALY_BOARD_
//		#error	"_ENABLE_4REALY_BOARD_ should be enabled"
	#endif // _ENABLE_4REALY_BOARD_

#endif // _CHRIS_A_ROLL_OFF_ROOF_

//*****************************************************************************
#ifdef _TOPENS_ROLL_OFF_ROOF_
	#define		kRelay_OpenStopClose	1

	//*	from Steven <SCV>
	//*	You can use Pin 29 (GPIO 5) & pin 31 (GPIO 6)
	#define		kRelay_RoofOpenSensor	5
	#define		kRelay_RoofCloseSensor	6

	//*	this is the default value, it can be changed from the web setup interface
	#define		kSwitchDelaySeconds		3

	#include	"raspberrypi_relaylib.h"

#endif // _TOPENS_ROLL_OFF_ROOF_


//#define _DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"RequestData.h"
#include	"JsonResponse.h"
#include	"eventlogging.h"

#include	"alpaca_defs.h"
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


#ifndef __arm__
#define	PUD_UP	1
#define	INPUT	0
#define	OUTPUT	1
//*****************************************************************************
void pinMode(int pinNumber, int inputoutput)
{
}
//*****************************************************************************
int	digitalRead(int pinNumber)
{
	return(0);
}
//*****************************************************************************
void pullUpDnControl(int pinNumber, int pullupState)
{
}
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
#ifdef _TOPENS_ROLL_OFF_ROOF_
	CONSOLE_DEBUG("_TOPENS_ROLL_OFF_ROOF_");
	cClosedSensorState	=	0;
	cOpenSensorState	=	0;
#elif defined(_CHRIS_A_ROLL_OFF_ROOF_)
	CONSOLE_DEBUG("_CHRIS_A_ROLL_OFF_ROOF_");
#else
	CONSOLE_DEBUG("Noting defined, ERROR!!!!!!!!");
#endif
	strcpy(cCommonProp.Name,	"Dome-Roll-Off-Roof");
	strcpy(gWebTitle,			"Dome-Roll-Off-Roof");

	cDomeConfig					=	kIsRollOffRoof;
	cDomeProp.AtPark			=	false;
	cDomeProp.Azimuth			=	0.0;	//*	Azimuth is meaningless for a ROR
	cDomeProp.CanSyncAzimuth	=	false;
	cDomeProp.CanSetShutter		=	true;

	cEnableIdleMoveTimeout		=	false;
	cWatchDogEnabled			=	false;
	cRORrelayDelay_secs			=	kSwitchDelaySeconds;	//*	used by Roll Off Roof ONLY

	//*	local stuff
	cRORisOpening				=	false;
	cRORisClosing				=	false;
	cCmdRcvd_OpenRoof			=	false;
	cCmdRcvd_CloseRoof			=	false;

	Init_Hardware();

	CONSOLE_DEBUG("Returned from Init_Hardware()");

	strcpy(cCommonProp.Name,		"AlpacaPi-ROR");
	strcpy(cCommonProp.Description,	"Roll Off Roof");
#ifdef _TOPENS_ROLL_OFF_ROOF_
	strcpy(cCommonProp.Description,	"Roll Off Roof - TOPENS controller");
#endif
	StartDriverThread();
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

#if defined(_CHRIS_A_ROLL_OFF_ROOF_)
	cRelayCount	=	RpiRelay_Init(0);

	CONSOLE_DEBUG_W_NUM("cRelayCount\t=", cRelayCount);

#endif // _CHRIS_A_ROLL_OFF_ROOF_

#ifdef _TOPENS_ROLL_OFF_ROOF_
bool		relayOK;
	cRelayCount	=	RpiRelay_Init(1);
	relayOK		=	RpiRelay_SetRelay(kRelay_OpenStopClose, true);
	relayOK		=	RpiRelay_SetRelay(2, true);
	relayOK		=	RpiRelay_SetRelay(3, true);
	relayOK		=	RpiRelay_SetRelay(4, true);
	CONSOLE_DEBUG("all relays set to TRUE");

	//*	set the I/O pins
	pinMode(kRelay_RoofOpenSensor,		INPUT);
	pinMode(kRelay_RoofCloseSensor,		INPUT);
	CONSOLE_DEBUG_W_NUM("kRelay_RoofOpenSensor  (INPUT)\t=", kRelay_RoofOpenSensor);
	CONSOLE_DEBUG_W_NUM("kRelay_RoofCloseSensor (INPUT)\t=", kRelay_RoofCloseSensor);

	//*	set the internal pullup resisters
	pullUpDnControl(kRelay_RoofOpenSensor,	PUD_UP);
	pullUpDnControl(kRelay_RoofCloseSensor,	PUD_UP);

	CONSOLE_DEBUG_W_NUM("Open/Stop/Close uses relay#", kRelay_OpenStopClose);

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

	currentMilliSecs	=	Millis();
	deltaMilliSecs		=	currentMilliSecs - cTimeOfLastOpenClose;
	if (cRORisOpening && (deltaMilliSecs > (cRORrelayDelay_secs * 1000)))
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

	if (cRORisClosing && (deltaMilliSecs > (cRORrelayDelay_secs * 1000)))
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
#ifdef _CHRIS_A_ROLL_OFF_ROOF_
bool				relayOK;

	relayOK		=	RpiRelay_SetRelay(kRelay_RoofPower, onOffFlag);
	if (relayOK == false)
	{
		CONSOLE_DEBUG("RpiRelay_SetRelay returned false");
	}
#endif // _CHRIS_A_ROLL_OFF_ROOF_
	return(kASCOM_Err_Success);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverROR::GetPower(bool *onOffFlag)
{
#ifdef _CHRIS_A_ROLL_OFF_ROOF_

	*onOffFlag	=	RpiRelay_GetRelay(kRelay_RoofPower);
#endif // _CHRIS_A_ROLL_OFF_ROOF_
	return(kASCOM_Err_Success);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverROR::SetAuxiliary(bool onOffFlag)
{
#ifdef _CHRIS_A_ROLL_OFF_ROOF_
bool				relayOK;

	relayOK		=	RpiRelay_SetRelay(kRelay_FlatScren, onOffFlag);
	if (relayOK == false)
	{
		CONSOLE_DEBUG("RpiRelay_SetRelay returned false");
	}
#endif // _CHRIS_A_ROLL_OFF_ROOF_
	return(kASCOM_Err_Success);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverROR::GetAuxiliary(bool *onOffFlag)
{
#ifdef _CHRIS_A_ROLL_OFF_ROOF_
	*onOffFlag	=	RpiRelay_GetRelay(kRelay_FlatScren);
#endif // _CHRIS_A_ROLL_OFF_ROOF_
	return(kASCOM_Err_Success);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverROR::OpenShutter(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
bool				relayOK;

	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;

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
	cTimeOfLastOpenClose	=	Millis();
	cRORisOpening			=	true;
	cDomeProp.Slewing		=	true;
	cDomeProp.ShutterStatus	=	kShutterStatus_Opening;
#elif defined(_TOPENS_ROLL_OFF_ROOF_)
	CONSOLE_DEBUG("_TOPENS_ROLL_OFF_ROOF_");
	//*	let the background thread do the real work
	cCmdRcvd_OpenRoof		=	true;
	cCmdRcvd_CloseRoof		=	false;
	alpacaErrCode			=	kASCOM_Err_Success;
	cTimeOfLastOpenClose	=	Millis();
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
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
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
	cTimeOfLastOpenClose	=	Millis();
	cRORisClosing			=	true;
	cDomeProp.Slewing		=	true;
	cDomeProp.ShutterStatus	=	kShutterStatus_Closing;
#elif defined(_TOPENS_ROLL_OFF_ROOF_)
	CONSOLE_DEBUG("_TOPENS_ROLL_OFF_ROOF_");
	cCmdRcvd_CloseRoof		=	true;
	cCmdRcvd_OpenRoof		=	false;
	alpacaErrCode			=	kASCOM_Err_Success;
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

//*****************************************************************************
void	DomeDriverROR::RunThread_Startup(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
void	DomeDriverROR::RunThread_Loop(void)
{
//uint32_t			currentTime_ms;
//uint32_t			deltaTime_ms;

//	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _TOPENS_ROLL_OFF_ROOF_
bool	relayOK;
uint32_t	relayStartMilliSecs;
uint32_t	currentStartMilliSecs;
uint32_t	deltaMilliSecs;

	//--------------------------------------------
	//*	read the current ror state
	//-----------------------------------------------------------
	//		Mag Switch 1:
	//			HIGH when Roof is closed
	//			LOW when Roof is starting to open
	//-----------------------------------------------------------
	//		Mag Switch 2:
	//			HIGH when Roof is open
	//			LOW when Roof is starting to close


	cOpenSensorState	=	digitalRead(kRelay_RoofOpenSensor);
	cClosedSensorState	=	digitalRead(kRelay_RoofCloseSensor);

//	CONSOLE_DEBUG_W_BOOL("cClosedSensorState\t=",	cClosedSensorState)
//	CONSOLE_DEBUG_W_BOOL("cOpenSensorState  \t=",	cOpenSensorState)


	if ((cClosedSensorState != 0) && (cOpenSensorState != 0))
	{
		cDomeProp.Slewing		=	true;
	}
	else if (cClosedSensorState == 0)
	{
		//*	cClosedSensorState == 0 means the shutter is closed
		cDomeProp.ShutterStatus	=	kShutterStatus_Closed;
		cDomeProp.Slewing		=	false;
		cRORisOpening			=	false;
		cRORisClosing			=	false;
	}
	else if (cOpenSensorState == 0)
	{
		//*	cOpenSensorState == 0 means the shutter is open
		cDomeProp.ShutterStatus	=	kShutterStatus_Open;
		cDomeProp.Slewing		=	false;
		cRORisOpening			=	false;
		cRORisClosing			=	false;
	}
	else if ((cClosedSensorState == 0) && (cOpenSensorState == 0))
	{
		CONSOLE_DEBUG("HARDWARE ERROR!!!!! Open and Close sensors both are active")
	}
	else
	{
		CONSOLE_DEBUG("Have no idea what state we are in")
	}
	//----------------------------------------------------------
	//*	check for an open command
	if (cCmdRcvd_OpenRoof)
	{
		CONSOLE_DEBUG("command to open roof has been received");
		if (cDomeProp.ShutterStatus == kShutterStatus_Open)
		{
			CONSOLE_DEBUG("ROR is already open")
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Turning on relay # (setting value to 0)", kRelay_OpenStopClose);
			cDomeProp.ShutterStatus	=	kShutterStatus_Opening;
			cDomeProp.Slewing		=	true;
			cRORisOpening			=	true;
			cRORisClosing			=	false;
			relayStartMilliSecs		=	Millis();
			//*	set the line HIGH to turn the relay on and connect the signal to ground
			relayOK		=	RpiRelay_SetRelay(kRelay_OpenStopClose, false);
			//*	now wait 3 seconds or until the open sensor goes false;
			CONSOLE_DEBUG("Waiting 3 seconds");
			deltaMilliSecs	=	0;
			while (deltaMilliSecs < 3000)
			{
				//*	we want the switch to activate for at least 1 second
				if (deltaMilliSecs > 1000)
				{
					//*	check the open sensor
					cOpenSensorState	=	digitalRead(kRelay_RoofOpenSensor);
					if (cOpenSensorState != 0)
					{
						break;
					}
				}
				currentStartMilliSecs	=	Millis();
				deltaMilliSecs			=	currentStartMilliSecs - relayStartMilliSecs;
				//*	wait 50 milliseconds
				usleep(50 * 1000);
			}

			CONSOLE_DEBUG_W_NUM("Turning off relay # (setting value to 1)", kRelay_OpenStopClose);
			relayOK		=	RpiRelay_SetRelay(kRelay_OpenStopClose, true);
			CONSOLE_DEBUG_W_NUM("elapsed time (milliseconds)", deltaMilliSecs);

		}
		cCmdRcvd_OpenRoof	=	false;
	}

	//----------------------------------------------------------
	//*	check for an close command
	if (cCmdRcvd_CloseRoof)
	{
		CONSOLE_DEBUG("command to close roof has been received");
		if (cDomeProp.ShutterStatus == kShutterStatus_Closed)
		{
			CONSOLE_DEBUG("ROR is already closed")
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Turning on relay #", kRelay_OpenStopClose);
			cDomeProp.ShutterStatus	=	kShutterStatus_Closing;
			cDomeProp.Slewing		=	true;
			cRORisClosing			=	true;
			currentStartMilliSecs	=	Millis();
			cRORisOpening			=	false;
			currentStartMilliSecs	=	Millis();
			//*	set the line HIGH to turn the relay on and connect the signal to ground
			relayOK		=	RpiRelay_SetRelay(kRelay_OpenStopClose, false);
			CONSOLE_DEBUG("Waiting 3 seconds");
			deltaMilliSecs		=	0;
			cClosedSensorState	=	0;
			while (deltaMilliSecs < 3000)
			{
				//*	we want the switch to activate for at least 1 second
				if (deltaMilliSecs > 1000)
				{
					//*	check the close sensor
					cClosedSensorState	=	digitalRead(kRelay_RoofCloseSensor);
					if (cClosedSensorState != 0)
					{
						break;
					}
				}
				currentStartMilliSecs	=	Millis();
				deltaMilliSecs			=	currentStartMilliSecs - relayStartMilliSecs;
				//*	wait 50 milliseconds
				usleep(50 * 1000);
			}
			CONSOLE_DEBUG_W_NUM("Turning off relay #", kRelay_OpenStopClose);
			relayOK		=	RpiRelay_SetRelay(kRelay_OpenStopClose, true);
			CONSOLE_DEBUG_W_NUM("elapsed time (milliseconds)", deltaMilliSecs);
		}

		cCmdRcvd_CloseRoof	=	false;
	}

#endif // _TOPENS_ROLL_OFF_ROOF_

//	//*	check if time to update
//	currentTime_ms	=	Millis();
//	deltaTime_ms	=	currentTime_ms - cLastUpdate_ms;
//	if (deltaTime_ms > 2000)
//	{
//
//
//	}

	usleep(500 * 1000);
}


//*****************************************************************************
void	DomeDriverROR::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
char				lineBuffer[256];
int					mySocketFD;

//	CONSOLE_DEBUG(__FUNCTION__);

	mySocketFD	=	reqData->socket;

	SocketWriteData(mySocketFD,	"<P>\r\n");

	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H2>Raspberry-Pi Roll Off Roof Driver</H2>\r\n");
	//===============================================================
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TH COLSPAN=3>Raspberry-Pi Roll Off Roof Driver</TH>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TH COLSPAN=3>Hardware configuration</TH>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

#ifdef _USE_BCM_PIN_NUMBERS_
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TD COLSPAN=3><CENTER>Using BCM Pin numbering</TH>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

#endif // _USE_BCM_PIN_NUMBERS_

#ifdef _ENABLE_4REALY_BOARD_
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TD COLSPAN=3><CENTER>Using Raspberry Pi 4 Relay board</TH>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

#endif // _ENABLE_4REALY_BOARD_


	SocketWriteData(mySocketFD,	"<TR>\r\n");
	sprintf(lineBuffer,	"\t<TD>Relay #1 pin</TD><TD>%d</TD><TD>Output</TD>\r\n", kHWpin_Channel1);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	SocketWriteData(mySocketFD,	"<TR>\r\n");
	sprintf(lineBuffer,	"\t<TD>Relay #2 pin</TD><TD>%d</TD><TD>Output</TD>\r\n", kHWpin_Channel2);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	SocketWriteData(mySocketFD,	"<TR>\r\n");
	sprintf(lineBuffer,	"\t<TD>Relay #3 pin</TD><TD>%d</TD><TD>Output</TD>\r\n", kHWpin_Channel3);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	SocketWriteData(mySocketFD,	"<TR>\r\n");
	sprintf(lineBuffer,	"\t<TD>Relay #4 pin</TD><TD>%d</TD><TD>Output</TD>\r\n", kHWpin_Channel4);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n");
#ifdef _TOPENS_ROLL_OFF_ROOF_
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	sprintf(lineBuffer,	"\t<TD>Open Sensor</TD><TD>%d</TD><TD>Input</TD><TD>%d</TD>\r\n",
												kRelay_RoofOpenSensor,
												cOpenSensorState);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	SocketWriteData(mySocketFD,	"<TR>\r\n");
	sprintf(lineBuffer,	"\t<TD>Close Sensor</TD><TD>%d</TD><TD>Input</TD><TD>%d</TD>\r\n",
												kRelay_RoofCloseSensor,
												cClosedSensorState);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n");
#endif // _TOPENS_ROLL_OFF_ROOF_

	SocketWriteData(reqData->socket,	"</TABLE>\r\n");
	SocketWriteData(reqData->socket,	"</CENTER>\r\n");
	SocketWriteData(reqData->socket,	"<P>\r\n");

}

#endif // _ENABLE_DOME_ROR_
