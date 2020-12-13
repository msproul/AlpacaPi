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
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Oct 14,	2019	<MLS> Added subclass for Raspberry Pi dome driver
//*	Oct 21,	2019	<MLS> R_Pi C++ version tested for first time on dome, working
//*	Oct 25,	2019	<MLS> Dome Raspberry Pi C++ version fully working
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

#ifdef _ENABLE_DOME_

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
#include	"domedriver_rpi.h"


#if defined(__arm__) && !defined(_ENABLE_PI_HAT_SESNSOR_BOARD_)
	#define	_ENABLE_DOME_HARDWARE_
	#include <wiringPi.h>
#else
	#define	LOW		0
	#define	HIGH	1
#endif

//*	Using BCM pin numbering scheme
#define	_USE_BCM_PIN_NUMBERS_
	//*	inputs
	#define	kHWpin_ButtonCW		23
	#define	kHWpin_ButtonCCW	24
	#define	kHWpin_Stop			25
	//*	outputs
	#define	kHWpin_PowerOnOff	17
	#define	kHWpin_Direction	27
	#define	kHWpin_PowerPWM		18

	#define	kHWpin_HomeSensor	5
	#define	kHWpin_ParkSensor	6




#ifndef _ENABLE_DOME_HARDWARE_

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
	{	kHWpin_ButtonCW,	0,	kButtonReleased,	kButtonReleased},
	{	kHWpin_ButtonCCW,	0,	kButtonReleased,	kButtonReleased},
	{	kHWpin_Stop,		0,	kButtonReleased,	kButtonReleased},

	{	-1,					0,	0,	0},

};

//**************************************************************************************
DomeDriverRPi::DomeDriverRPi(const int argDevNum)
	:DomeDriver(argDevNum)
{
	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cDeviceName, "Dome-Raspberry-Pi");
	strcpy(gWebTitle, "Dome-Raspberry-Pi");

	cDomeConfig				=	kIsDome;
	cCanFindHome			=	true;
	cCanPark				=	true;
	cCanSetAzimuth			=	true;

	Init_Hardware();
	LogEvent(	"dome",
				"R-Pi Dome created",
				NULL,
				kASCOM_Err_Success,
				"");

	strcpy(cDeviceName,			"AlpacaPi-Dome");
	strcpy(cDeviceDescription,	"Dome controlled by Raspberry Pi");
}

//**************************************************************************************
// Destructor
//**************************************************************************************
DomeDriverRPi::~DomeDriverRPi( void )
{
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

	pinMode(kHWpin_PowerOnOff,	OUTPUT);
	pinMode(kHWpin_Direction,	OUTPUT);

	pinMode(kHWpin_PowerPWM,	PWM_OUTPUT);

	pinMode(kHWpin_ButtonCW,	INPUT);
	pinMode(kHWpin_ButtonCCW,	INPUT);
	pinMode(kHWpin_Stop,		INPUT);
	pinMode(kHWpin_HomeSensor,	INPUT);
	pinMode(kHWpin_ParkSensor,	INPUT);

	pullUpDnControl(kHWpin_ButtonCW,	PUD_UP);
	pullUpDnControl(kHWpin_ButtonCCW,	PUD_UP);
	pullUpDnControl(kHWpin_Stop,		PUD_UP);
	pullUpDnControl(kHWpin_HomeSensor,	PUD_UP);
	pullUpDnControl(kHWpin_ParkSensor,	PUD_UP);


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
	if (cCurrentPWM > 1023)
	{
		returnState				=	true;
		cCurrentPWM				=	1023;
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
		cSlewing			=	true;
	}
	else
	{
		cSlewing			=	false;
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
			if (cSlewing)
			{
				strcpy(alpacaErrMsg, "Dome already in motion, command ignored");
			}
			else
			{
				StartDomeMoving(kRotateDome_CW);
				cManualMove	=	true;
			}
			break;

		case kHWpin_ButtonCCW:
			strcpy(command, "Button-CCW");
			if (cSlewing)
			{
				strcpy(alpacaErrMsg, "Dome already in motion, command ignored");
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
				strcpy(alpacaErrMsg, "Panic stop");
			}
			else
			{
				StopDomeMoving(kStopNormal);
			}
			break;
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
	while(currentlTics == initialTics)
	{
		currentlTics	=	millis();
	}
	cCurrentPWM	=	500;
	pwmWrite(kHWpin_PowerPWM, cCurrentPWM);

	cCurrentDirection		=	direction;
	cDomeState				=	kDomeState_SpeedingUp;
	cTimeOfLastSpeedChange	=	millis();
	cTimeOfMovingStart		=	millis();
	cSlewing				=	true;
}


//*****************************************************************************
void	DomeDriverRPi::StopDomeMoving(bool rightNow)
{
	CONSOLE_DEBUG(__FUNCTION__);

	if (rightNow)
	{
		BumpDomeSpeed(-2000);	//*	this way we get the logging information as well

		//*	do this anyway, just as a backup
		pwmWrite(kHWpin_PowerPWM, 0);
		cCurrentPWM		=	0;
		cDomeState		=	kDomeState_Stopped;
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
		if (cAtHome == false)
		{
			CONSOLE_DEBUG("cAtHome state changed");
		}
		cAtHome	=	true;
	}
	else
	{
		cAtHome	=	false;
	}

	sensorState	=	digitalRead(kHWpin_ParkSensor);
	if (sensorState == 0)
	{
		if (cAtPark == false)
		{
			CONSOLE_DEBUG("atPark state changed");
		}
		cAtPark	=	true;
	}
	else
	{
		cAtPark	=	false;
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

#endif // _ENABLE_DOME_
