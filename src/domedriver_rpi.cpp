//**************************************************************************
//*	Name:			domedriver_rpi.cpp
//*
//*	Author:			Mark Sproul (C) 2019
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
//*	Oct 14,	2019	<MLS> Added subclass for Raspberry Pi dome driver
//*	Oct 21,	2019	<MLS> R_Pi C++ version tested for first time on dome, working
//*	Oct 25,	2019	<MLS> Dome Raspberry Pi C++ version fully working
//*	Jan 10,	2021	<MLS> Added UpdateDomePosition() using time integration
//*	Mar  5,	2021	<MLS> Started working on commutator power on/off
//*	Mar  5,	2021	<MLS> Added _ENABLE_COMMUTATOR_POWER_
//*	Dec 23,	2021	<MLS> Added OutputHTML_Part2() to output hardware configuration
//*	Mar  2,	2023	<MLS> Added _ENABLE_DOME_RPI_
//*	Sep 12,	2023	<MLS> Installed Arduino dev platform on Dome R-Pi
//*	Aug 17,	2024	<MLS> Added _ENABLE_EXPLORADOME_
//*	Aug 17,	2024	<MLS> Working on exlporadome option for Larry
//*	Aug 17,	2024	<MLS> Added OutputHTMLsensorPin()
//*****************************************************************************
//*	cd /home/pi/dev-mark/alpaca
//*	LOGFILE=logfile.txt
//*	DATE=`date`
//*
//*	if [ -f domecontroller ]
//*	then
//*		./domecontroller >/dev/null &
//*		echo "$DATE - Dome controller started" >> $LOGFILE
//*	else
//*		pwd
//*		echo "Dome controller executable not found"
//*		echo "$DATE - Dome controller executable not found" >> $LOGFILE
//*	fi
//*****************************************************************************
//*		https://www.amazon.com/gp/product/B07L6HGFWY/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1
//*****************************************************************************


#ifdef _ENABLE_DOME_RPI_

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
#include	"domedriver_rpi.h"


#if defined(__arm__) && !defined(_ENABLE_PI_HAT_SESNSOR_BOARD_)
	#define	_ENABLE_DOME_HARDWARE_
	#define	_ENABLE_COMMUTATOR_POWER_

	#include <wiringPi.h>
#else
	#define	LOW		0
	#define	HIGH	1
#endif

//*	Using BCM pin numbering scheme
#define	_USE_BCM_PIN_NUMBERS_
	//*	inputs
	#define	kHWpin_ButtonCW			23
	#define	kHWpin_ButtonCCW		24
	#define	kHWpin_Stop				25
	//*	outputs
	#define	kHWpin_Direction		27
	#define	kHWpin_PowerPWM			18

	#define	kHWpin_HomeSensor		5
	#define	kHWpin_ParkSensor		6

//	#define	kHWpin_PowerOnOff		17
	#define	kHWpin_CommutatorPwr	17


	#define	kMaxPWMvalue	1023

#ifdef _ENABLE_EXPLORADOME_
	#define	kHWpin_OpenCompleteSensor	20
	#define	kHWpin_CloseCompleteSensor	21

	#define	kHWpin_ButtonShutterOpen	13
	#define	kHWpin_ButtonShutterClose	19
	#define	kHWpin_ButtonShutterStop	26


	#define	kHWpin_ShutterDirectionCtrl	12
	#define	kHWpin_ShutterMotorCtrl		16

#endif // _ENABLE_EXPLORADOME_
// +-----+-----+---------+------+---+---Pi 4B--+---+------+---------+-----+-----+
// | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
// +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
// |     |     |    3.3v |      |   |  1 || 2  |   |      | 5v      |     |     |
// |   2 |   8 |   SDA.1 |   IN | 1 |  3 || 4  |   |      | 5v      |     |     |
// |   3 |   9 |   SCL.1 |   IN | 1 |  5 || 6  |   |      | 0v      |     |     |
// |   4 |   7 | GPIO. 7 |   IN | 1 |  7 || 8  | 1 | IN   | TxD     | 15  | 14  |
// |     |     |      0v |      |   |  9 || 10 | 1 | IN   | RxD     | 16  | 15  |
// |  17 |   0 | GPIO. 0 |   IN | 0 | 11 || 12 | 0 | IN   | GPIO. 1 | 1   | 18  |
// |  27 |   2 | GPIO. 2 |   IN | 0 | 13 || 14 |   |      | 0v      |     |     |
// |  22 |   3 | GPIO. 3 |   IN | 0 | 15 || 16 | 0 | IN   | GPIO. 4 | 4   | 23  |
// |     |     |    3.3v |      |   | 17 || 18 | 0 | IN   | GPIO. 5 | 5   | 24  |
// |  10 |  12 |    MOSI |   IN | 0 | 19 || 20 |   |      | 0v      |     |     |
// |   9 |  13 |    MISO |   IN | 0 | 21 || 22 | 0 | IN   | GPIO. 6 | 6   | 25  |
// |  11 |  14 |    SCLK |   IN | 0 | 23 || 24 | 1 | IN   | CE0     | 10  | 8   |
// |     |     |      0v |      |   | 25 || 26 | 1 | IN   | CE1     | 11  | 7   |
// |   0 |  30 |   SDA.0 |   IN | 1 | 27 || 28 | 1 | IN   | SCL.0   | 31  | 1   |
// |   5 |  21 | GPIO.21 |   IN | 1 | 29 || 30 |   |      | 0v      |     |     |
// |   6 |  22 | GPIO.22 |   IN | 1 | 31 || 32 | 0 | IN   | GPIO.26 | 26  | 12  |
// |  13 |  23 | GPIO.23 |   IN | 0 | 33 || 34 |   |      | 0v      |     |     |
// |  19 |  24 | GPIO.24 |   IN | 0 | 35 || 36 | 0 | IN   | GPIO.27 | 27  | 16  |
// |  26 |  25 | GPIO.25 |   IN | 0 | 37 || 38 | 0 | IN   | GPIO.28 | 28  | 20  |
// |     |     |      0v |      |   | 39 || 40 | 0 | IN   | GPIO.29 | 29  | 21  |
// +-----+-----+---------+------+---+----++----+---+------+---------+-----+-----+
// | BCM | wPi |   Name  | Mode | V | Physical | V | Mode | Name    | wPi | BCM |
// +-----+-----+---------+------+---+---Pi 4B--+---+------+---------+-----+-----+

#ifndef _ENABLE_DOME_HARDWARE_
#define INPUT 0
#define OUTPUT 1
#define PUD_UP 2
//*****************************************************************************
static void	pinMode(int pinNum, int mode)
{
}

//*****************************************************************************
static void	pullUpDnControl(int pinNum, int mode)
{
}

//*****************************************************************************
static void	delayMicroseconds(long microsecs)
{
}


//*****************************************************************************
static int	digitalRead(int pinNum)
{
	return(true);
}

//*****************************************************************************
static void	digitalWrite(int pinNum, int digtialStte)
{
}


//*****************************************************************************
static void	pwmWrite(int pinNum, int pwmValue)
{
}
#endif	//	! _ENABLE_DOME_HARDWARE_

static uint32_t	DebounceSwitch(int pinNumber, uint32_t previousSwitchState);

//*****************************************************************************
void	CreateDomeObjectsRPi(void)
{
	new DomeDriverRPi(0);
}



//*****************************************************************************
BUTTON_ENTRY	gDomeButtons[]	=
{
	{	kHWpin_ButtonCW,			0,	kButtonReleased,	kButtonReleased},
	{	kHWpin_ButtonCCW,			0,	kButtonReleased,	kButtonReleased},
	{	kHWpin_Stop,				0,	kButtonReleased,	kButtonReleased},
#ifdef _ENABLE_EXPLORADOME_
	{	kHWpin_ButtonShutterOpen,	0,	kButtonReleased,	kButtonReleased},
	{	kHWpin_ButtonShutterClose,	0,	kButtonReleased,	kButtonReleased},
	{	kHWpin_ButtonShutterStop,	0,	kButtonReleased,	kButtonReleased},
#endif // _ENABLE_EXPLORADOME_
	{	-1,					0,	0,	0},

};

//**************************************************************************************
DomeDriverRPi::DomeDriverRPi(const int argDevNum)
	:DomeDriver(argDevNum)
{
	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name, "Dome-Raspberry-Pi");
	strcpy(gWebTitle, "Dome-Raspberry-Pi");

	cDomeConfig					=	kIsDome;
	cDomeProp.CanFindHome		=	true;
	cDomeProp.CanPark			=	true;
	cDomeProp.CanSetAzimuth		=	true;
	cDomeProp.CanSyncAzimuth	=	false;
	cDomeProp.CanSetShutter		=	true;
	cParkAzimuth				=	170.0;		//*	these are approximate for my dome
	cHomeAzimuth				=	230.0;
	cWatchDogEnabled			=	true;
	strcpy(cWatchDogTimeOutAction, "Close shutter");

	Init_Hardware();
	LogEvent(	"dome",
				"R-Pi Dome created",
				NULL,
				kASCOM_Err_Success,
				"");

	strcpy(cCommonProp.Name,		"AlpacaPi-Dome");
	strcpy(cCommonProp.Description,	"Dome controlled by Raspberry Pi");
}

//**************************************************************************************
// Destructor
//**************************************************************************************
DomeDriverRPi::~DomeDriverRPi( void )
{
}

//*****************************************************************************
static void	OutputHTMLsensorPin(int socketFD, int pinNumber, const char *descripton)
{
char		lineBuffer[256];
int			sensorState;

	SocketWriteData(socketFD,	"<TR>\r\n");
	sprintf(lineBuffer,	"\t<TD>%s</TD><TD>%d</TD><TD>Input</TD>\r\n",
							descripton,
							pinNumber);
	SocketWriteData(socketFD,	lineBuffer);
	sensorState	=	digitalRead(pinNumber);
	sprintf(lineBuffer,	"\t<TD>%d</TD>\r\n",	sensorState);
	SocketWriteData(socketFD,	lineBuffer);
	SocketWriteData(socketFD,	"</TR>\r\n");

}

//*****************************************************************************
static void	OutputHTMLcontrolPin(int socketFD, int pinNumber, const char *descripton)
{
char		lineBuffer[256];

	SocketWriteData(socketFD,	"<TR>\r\n");
	sprintf(lineBuffer,	"\t<TD>%s</TD><TD>%d</TD><TD>Output</TD>\r\n",
							descripton,
							pinNumber);
	SocketWriteData(socketFD,	lineBuffer);
	SocketWriteData(socketFD,	"</TR>\r\n");

}

//*****************************************************************************
void	DomeDriverRPi::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
int			mySocketFD;

//	CONSOLE_DEBUG(__FUNCTION__);

	mySocketFD	=	reqData->socket;

	SocketWriteData(mySocketFD,	"<P>\r\n");

	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H2>Raspberry-Pi Dome Driver</H2>\r\n");
#ifdef _ENABLE_EXPLORADOME_
	SocketWriteData(mySocketFD,	"<H3>ExploraDome implementation</H3>\r\n");
#endif // _ENABLE_EXPLORADOME_
	//===============================================================
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TH COLSPAN=4>Raspberry-Pi Dome Driver</TH>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TH COLSPAN=4>Hardware configuration</TH>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

#ifdef _USE_BCM_PIN_NUMBERS_
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TD COLSPAN=4><CENTER>Using BCM Pin numbering</TH>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");
#endif // _USE_BCM_PIN_NUMBERS_

	OutputHTMLsensorPin(mySocketFD,		kHWpin_ButtonCW,		"Clockwise button pin");
	OutputHTMLsensorPin(mySocketFD,		kHWpin_ButtonCCW,		"Counter Clockwise button pin");
	OutputHTMLsensorPin(mySocketFD,		kHWpin_Stop,			"Stop button pin");

	OutputHTMLcontrolPin(mySocketFD,	kHWpin_Direction,		"Direction Control pin");
	OutputHTMLcontrolPin(mySocketFD,	kHWpin_PowerPWM,		"Power PWM pin");

	OutputHTMLsensorPin(mySocketFD,		kHWpin_HomeSensor,		"Home Sensor pin");
	OutputHTMLsensorPin(mySocketFD,		kHWpin_ParkSensor,		"Park Sensor pin");

#ifdef _ENABLE_EXPLORADOME_
	SocketWriteData(mySocketFD,	"<TH COLSPAN=4>Specific to ExlporaDome implementation</TH>\r\n");

	OutputHTMLsensorPin(mySocketFD,		kHWpin_OpenCompleteSensor,	"<FONT COLOR=RED>Open Complete Sensor pin");
	OutputHTMLsensorPin(mySocketFD,		kHWpin_CloseCompleteSensor,	"<FONT COLOR=RED>Close Complete Sensor pin");


	OutputHTMLsensorPin(mySocketFD,		kHWpin_ButtonShutterOpen,	"<FONT COLOR=RED>Shutter Open Button pin");
	OutputHTMLsensorPin(mySocketFD,		kHWpin_ButtonShutterClose,	"<FONT COLOR=RED>Shutter Close Button pin");
	OutputHTMLsensorPin(mySocketFD,		kHWpin_ButtonShutterStop,	"<FONT COLOR=RED>Shutter Stop Button pin");

	OutputHTMLcontrolPin(mySocketFD,	kHWpin_ShutterDirectionCtrl,	"<FONT COLOR=RED>Shutter Direction Control pin");
	OutputHTMLcontrolPin(mySocketFD,	kHWpin_ShutterMotorCtrl,		"<FONT COLOR=RED>Shutter Motor Control pin");

#endif // _ENABLE_EXPLORADOME_


	SocketWriteData(reqData->socket,	"</TABLE>\r\n");
	SocketWriteData(reqData->socket,	"</CENTER>\r\n");
	SocketWriteData(reqData->socket,	"<P>\r\n");

}

#define	kRpiPinCount	30
static void		SetupPin(int pinNumber, int pinMode, int initialState=0);
static short	gPinAllocation[kRpiPinCount];
static bool		gPinAllocationInit	=	true;
//*****************************************************************************
static void	SetupPin(int pinNumber, int pinModeValue, int initialState)
{
int	iii;

	if ((pinNumber>= 0) && (pinNumber < kRpiPinCount))
	{
		CONSOLE_DEBUG_W_NUM("Pin #", pinNumber);
		if (gPinAllocationInit)
		{
			for (iii=0; iii<kRpiPinCount; iii++)
			{
				gPinAllocation[iii]	=	0;
			}
			gPinAllocationInit	=	false;
		}
		if (gPinAllocation[pinNumber] == true)
		{
			CONSOLE_DEBUG_W_NUM("Pin is already allocated", pinNumber);
			CONSOLE_ABORT(__FUNCTION__);
		}
		gPinAllocation[pinNumber]	=	true;
		pinMode(pinNumber,			pinModeValue);

		if (pinModeValue == INPUT)
		{
			pullUpDnControl(pinNumber,	PUD_UP);
		}
		else if (pinModeValue == OUTPUT)
		{
			digitalWrite(pinNumber,		initialState);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Pin # is invalid, (out of bounds)", pinNumber);
		CONSOLE_ABORT(__FUNCTION__);
	}
}


//*****************************************************************************
void	DomeDriverRPi::Init_Hardware(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _ENABLE_DOME_HARDWARE_
int		wiringPi_rc;

int		wiringPi_verMajor;
int		wiringPi_verMinor;
char	wiringPi_VerString[32];


	wiringPiVersion(&wiringPi_verMajor, &wiringPi_verMinor);
	sprintf(wiringPi_VerString, "%d.%d", wiringPi_verMajor, wiringPi_verMinor);
	AddLibraryVersion("software", "wiringPi", wiringPi_VerString);


	CONSOLE_DEBUG("Setting up hardware io pins");

	wiringPi_rc	=	wiringPiSetupGpio();

	CONSOLE_DEBUG_W_NUM("wiringPi_rc", wiringPi_rc);

	SetupPin(kHWpin_CommutatorPwr,		OUTPUT);
	SetupPin(kHWpin_Direction,			OUTPUT);

	SetupPin(kHWpin_PowerPWM,			PWM_OUTPUT);
	pwmWrite(kHWpin_PowerPWM,			0);				//*	make sure its zero.

	SetupPin(kHWpin_ButtonCW,			INPUT);
	SetupPin(kHWpin_ButtonCCW,			INPUT);
	SetupPin(kHWpin_Stop,				INPUT);
	SetupPin(kHWpin_HomeSensor,			INPUT);
	SetupPin(kHWpin_ParkSensor,			INPUT);

	//*	set the internal pullup resisters
//	pullUpDnControl(kHWpin_ButtonCW,	PUD_UP);
//	pullUpDnControl(kHWpin_ButtonCCW,	PUD_UP);
//	pullUpDnControl(kHWpin_Stop,		PUD_UP);
//	pullUpDnControl(kHWpin_HomeSensor,	PUD_UP);
//	pullUpDnControl(kHWpin_ParkSensor,	PUD_UP);


#ifdef _ENABLE_EXPLORADOME_
	//*	this option created for Larry, August 2024
	SetupPin(kHWpin_ShutterDirectionCtrl,		OUTPUT, LOW);
	SetupPin(kHWpin_ShutterMotorCtrl,			OUTPUT, LOW);

	SetupPin(kHWpin_OpenCompleteSensor,			INPUT);
	SetupPin(kHWpin_CloseCompleteSensor,		INPUT);
	SetupPin(kHWpin_ButtonShutterOpen,			INPUT);
	SetupPin(kHWpin_ButtonShutterClose,			INPUT);
	SetupPin(kHWpin_ButtonShutterStop,			INPUT);
#endif

#endif	//	_ENABLE_DOME_HARDWARE_
}



//*****************************************************************************
//*	returns true if at max or min
//*	returns false otherwise
//*****************************************************************************
bool		DomeDriverRPi::BumpDomeSpeed(const int howMuch)
{
bool		returnState;
uint32_t	currentMilliSecs;
uint32_t	movingTime_millisecs;
char		msgBuffer[64];

	returnState		=	0;
	cCurrentPWM		+=	howMuch;
	if (cCurrentPWM > kMaxPWMvalue)
	{
		returnState				=	true;
		cCurrentPWM				=	kMaxPWMvalue;
		currentMilliSecs		=	millis();
		movingTime_millisecs	=	currentMilliSecs - cTimeOfMovingStart;
//		CONSOLE_DEBUG_W_INT32("Ramp up time\t=", movingTime_millisecs);
	}
	else if (cCurrentPWM < 200)
	{
		returnState			=	true;
		cCurrentPWM			=	0;

		currentMilliSecs		=	millis();
		movingTime_millisecs	=	currentMilliSecs - cTimeOfMovingStart;
//		CONSOLE_DEBUG_W_INT32("Total time moving\t=", movingTime_millisecs);
		sprintf(msgBuffer, "Total time moving %d millisecs", movingTime_millisecs);
		LogEvent(	"dome",
					"Movement stopped",
					NULL,
					kASCOM_Err_Success,
					msgBuffer);

	}

	pwmWrite(kHWpin_PowerPWM, cCurrentPWM);

	if (cCurrentPWM > 0)
	{
		cDomeProp.Slewing			=	true;
	}
	else
	{
		cDomeProp.Slewing			=	false;
	}

	cTimeOfLastSpeedChange	=	millis();

	return(returnState);
}

//*****************************************************************************
void	DomeDriverRPi::CheckDomeButtons(void)
{
int			ii;
uint32_t	newButtonBits;

//	CONSOLE_DEBUG(__FUNCTION__);

	ii	=	0;
	while (gDomeButtons[ii].pinNumber >= 0)
	{
		newButtonBits	=	DebounceSwitch(	gDomeButtons[ii].pinNumber,
											gDomeButtons[ii].debounceBits);

		gDomeButtons[ii].debounceBits	=	newButtonBits;
		if (newButtonBits == 0)
		{
			//*	the button is pressed
			gDomeButtons[ii].curentState	=	kButtonPressed;
//			CONSOLE_DEBUG_W_NUM("Button is pressed, pin # ", gDomeButtons[ii].pinNumber);
		}
		else if (newButtonBits == 0xffffffff)
		{
			gDomeButtons[ii].curentState	=	kButtonReleased;
		}
		if (gDomeButtons[ii].curentState != gDomeButtons[ii].previousState)
		{
			//*	we have a change of state, if it is pressed, we want to do something
			if (gDomeButtons[ii].curentState)
			{
				CONSOLE_DEBUG_W_NUM("Button is pressed, pin # ", gDomeButtons[ii].pinNumber);
				ProcessButtonPressed(gDomeButtons[ii].pinNumber);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("Button is released, pin # ", gDomeButtons[ii].pinNumber);
			}
			gDomeButtons[ii].previousState	=	gDomeButtons[ii].curentState;
		}
		ii++;
	}
}

//*****************************************************************************
void DomeDriverRPi::ProcessButtonPressed(const int pressedButton)
{
char		alpacaErrMsg[256];
char		command[64];

	strcpy(alpacaErrMsg, "");
	strcpy(command, "");

	switch(pressedButton)
	{
		case kHWpin_ButtonCW:
			strcpy(command, "Button-CW");
			if (cDomeProp.Slewing)
			{
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Dome already in motion, command ignored");
			}
			else
			{
				StartDomeMoving(kRotateDome_CW);
				cManualMove	=	true;
			}
			break;

		case kHWpin_ButtonCCW:
			strcpy(command, "Button-CCW");
			if (cDomeProp.Slewing)
			{
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Dome already in motion, command ignored");
			}
			else
			{
				StartDomeMoving(kRotateDome_CCW);
				cManualMove	=	true;
			}
			break;

		case kHWpin_Stop:
			strcpy(command, "Button-Stop");
			if (cDomeState == kDomeState_SlowingDown)
			{
				//*	if we are already slowing down and the button is pressed a 2nd time
				//*	stop right now
				StopDomeMoving(kStopRightNow);
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Panic stop");
			}
			else
			{
				StopDomeMoving(kStopNormal);
			}
			break;

#ifdef _ENABLE_EXPLORADOME_
		case kHWpin_ButtonShutterOpen:
			strcpy(command, "Shutter Open");
			switch(cDomeProp.ShutterStatus)
			{
				case kShutterStatus_Unknown:		//*	not part of the alpaca standard
				case kShutterStatus_Closed:
				case kShutterStatus_Error:
					OpenShutter(alpacaErrMsg);
					break;

				case kShutterStatus_Closing:
					StopShutter(alpacaErrMsg);
					break;

				case kShutterStatus_Open:
				case kShutterStatus_Opening:
					//*	do nothing
					break;
			}
			break;

		case kHWpin_ButtonShutterClose:
			strcpy(command, "Shutter Close");
			switch(cDomeProp.ShutterStatus)
			{
				case kShutterStatus_Unknown:		//*	not part of the alpaca standard
				case kShutterStatus_Open:
				case kShutterStatus_Error:
					CloseShutter(alpacaErrMsg);
					break;

				case kShutterStatus_Opening:
					StopShutter(alpacaErrMsg);
					break;

				case kShutterStatus_Closed:
				case kShutterStatus_Closing:
					//*	do nothing
					break;
			}
			break;

		case kHWpin_ButtonShutterStop:
			strcpy(command, "Shutter Stop");
			StopShutter(alpacaErrMsg);
			break;

#endif // _ENABLE_EXPLORADOME_
	}
	LogEvent(	"dome",
				command,
				NULL,
				kASCOM_Err_Success,
				alpacaErrMsg);
}



//*****************************************************************************
void	DomeDriverRPi::StartDomeMoving(const int direction)
{
uint32_t	initialTics;
uint32_t	currentlTics;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _ENABLE_COMMUTATOR_POWER_
	//*	make sure the commutator power is off
	digitalWrite(kHWpin_CommutatorPwr, LOW);
#endif


	//*	first set the direction
	if (direction)
	{
		CONSOLE_DEBUG("Starting CCW");
		digitalWrite(kHWpin_Direction, HIGH);
	}
	else
	{
		CONSOLE_DEBUG("Starting CW");
		digitalWrite(kHWpin_Direction, LOW);
	}

	delayMicroseconds(5000);

	initialTics		=	millis();
	currentlTics	=	initialTics;
	//*	I know this looks a little weird,
	//*	this is an attempt to get consistent times
	while (currentlTics == initialTics)
	{
		currentlTics	=	millis();
	}
	cCurrentPWM	=	500;
	pwmWrite(kHWpin_PowerPWM, cCurrentPWM);

	cCurrentDirection			=	direction;
	cDomeState					=	kDomeState_SpeedingUp;
	cTimeOfLastSpeedChange		=	millis();
	cTimeOfMovingStart			=	millis();
	cTimeOfLastAzimuthUpdate	=	millis();
	cDomeProp.Slewing			=	true;
}


//*****************************************************************************
void	DomeDriverRPi::StopDomeMoving(bool rightNow)
{
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	cAzimuth_Destination	=	-1;

	if (rightNow)
	{
		CONSOLE_DEBUG("rightNow");
		BumpDomeSpeed(-2000);	//*	this way we get the logging information as well

		//*	do this anyway, just as a backup
		for (iii=0; iii<5; iii++)
		{
			pwmWrite(kHWpin_PowerPWM, 0);
			usleep(1000);
		}
		cCurrentPWM			=	0;
		cDomeState			=	kDomeState_Stopped;
		cDomeProp.Slewing	=	false;
	}
	else
	{
		cDomeState			=	kDomeState_SlowingDown;
	}
	cGoingBump			=	false;
	cGoingPark			=	false;
	cGoingHome			=	false;
	cManualMove			=	false;
}

//*****************************************************************************
void	DomeDriverRPi::CheckSensors(void)
{
int			sensorState;

	//*	read the home switch
	sensorState	=	digitalRead(kHWpin_HomeSensor);
	if (sensorState == 0)
	{
		if (cDomeProp.AtHome == false)
		{
			CONSOLE_DEBUG("cAtHome state changed");
		}
		cDomeProp.AtHome	=	true;
	}
	else
	{
		cDomeProp.AtHome	=	false;
	}

	sensorState	=	digitalRead(kHWpin_ParkSensor);
	if (sensorState == 0)
	{
		if (cDomeProp.AtPark == false)
		{
			CONSOLE_DEBUG("atPark state changed");
		}
		cDomeProp.AtPark	=	true;
		cDomeProp.Azimuth	=	170.0;
	}
	else
	{
		cDomeProp.AtPark	=	false;
	}
#ifdef _ENABLE_COMMUTATOR_POWER_
	//*	check to make sure we are parked and not moving
	if ((cDomeProp.AtPark == true) && (cDomeProp.Slewing == false))
	{
		//*	turn on the relay to supply power to the commutator
		digitalWrite(kHWpin_CommutatorPwr, HIGH);
	}
	else
	{
		digitalWrite(kHWpin_CommutatorPwr, LOW);
	}
#endif // _ENABLE_COMMUTATOR_POWER_


#ifdef _ENABLE_EXPLORADOME_
	sensorState	=	digitalRead(kHWpin_OpenCompleteSensor);
	if (sensorState == 0)
	{
		if (cDomeProp.ShutterStatus != kShutterStatus_Open)
		{
			CONSOLE_DEBUG("ShutterStatus state changed");
		}
		cDomeProp.ShutterStatus	=	kShutterStatus_Open;
	}

	sensorState	=	digitalRead(kHWpin_CloseCompleteSensor);
	if (sensorState == 0)
	{
		if (cDomeProp.ShutterStatus != kShutterStatus_Closed)
		{
			CONSOLE_DEBUG("ShutterStatus state changed");
		}
		cDomeProp.ShutterStatus	=	kShutterStatus_Closed;
	}

#endif // _ENABLE_EXPLORADOME_

}

//*****************************************************************************
//	The dome azimuth, increasing clockwise,
//	i.e.,	North	=	0
//			East	=	90
//			South	=	180
//			West	=	270
//*	North is true north and not magnetic north.
//*****************************************************************************
void	DomeDriverRPi::UpdateDomePosition(void)
{
int32_t	currentMilliSecs;
int32_t	deltaMilliSecs;
double	speedPercent;
double	degreesMoved;
double	degreesPerMilliSec;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cDomeProp.Slewing)
	{
		currentMilliSecs	=	millis();
		deltaMilliSecs		=	currentMilliSecs - cTimeOfLastAzimuthUpdate;
		if (deltaMilliSecs > 0)
		{
		//	CONSOLE_DEBUG_W_NUM("deltaMilliSecs\t\t=", deltaMilliSecs);

			speedPercent	=	(1.0 * cCurrentPWM) / kMaxPWMvalue;
			if (speedPercent > 0.0)
			{
//				CONSOLE_DEBUG_W_DBL("speedPercent\t=", speedPercent);

				//*	At full speed, it takes about 2.4 minutes to go one full revolution
				degreesPerMilliSec	=	360.0 / (2.4 * 60.0 * 1000.0);
				degreesMoved		=	speedPercent * deltaMilliSecs * degreesPerMilliSec;
				if (cCurrentDirection == kRotateDome_CW)
				{
					cDomeProp.Azimuth	+=	degreesMoved;
				}
				else
				{
					cDomeProp.Azimuth	-=	degreesMoved;
				}
				if (cDomeProp.Azimuth < 0.0)
				{
					cDomeProp.Azimuth	+=	360.0;
				}
				if (cDomeProp.Azimuth > 360.0)
				{
					cDomeProp.Azimuth	-=	360.0;
				}
			}
			cTimeOfLastAzimuthUpdate	=	millis();
		}
	}
}

#pragma mark -



//*****************************************************************************
//*	returns a 32 bit field showing the last 32 states of the switch
static uint32_t	DebounceSwitch(int pinNumber, uint32_t previousSwitchState)
{
unsigned long	newSwitchState;
int				switchUpDown;

	newSwitchState	=	previousSwitchState << 1;

	switchUpDown	=	digitalRead(pinNumber);

//	printf("Pin %2d is %d\r\n", pinNumber, switchUpDown);
	if (switchUpDown != 0)
	{
		newSwitchState	|=	0x01;
	}
	return(newSwitchState);
}


#ifdef _ENABLE_EXPLORADOME_
//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverRPi::OpenShutter(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	digitalWrite(kHWpin_ShutterDirectionCtrl,	HIGH);
	usleep(5000);
	digitalWrite(kHWpin_ShutterMotorCtrl,		HIGH);

	cDomeProp.ShutterStatus	=	kShutterStatus_Opening;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverRPi::CloseShutter(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	digitalWrite(kHWpin_ShutterDirectionCtrl,	LOW);
	usleep(5000);
	digitalWrite(kHWpin_ShutterMotorCtrl,		HIGH);

	cDomeProp.ShutterStatus	=	kShutterStatus_Opening;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriverRPi::StopShutter(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;

	return(alpacaErrCode);
}

#endif // _ENABLE_EXPLORADOME_

#endif // _ENABLE_DOME_RPI_
