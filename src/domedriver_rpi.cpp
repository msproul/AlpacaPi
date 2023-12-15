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
	#define	kHWpin_ButtonCW		23
	#define	kHWpin_ButtonCCW	24
	#define	kHWpin_Stop			25
	//*	outputs
	#define	kHWpin_Direction	27
	#define	kHWpin_PowerPWM		18

	#define	kHWpin_HomeSensor	5
	#define	kHWpin_ParkSensor	6

//	#define	kHWpin_PowerOnOff		17
	#define	kHWpin_CommutatorPwr	17


#define	kMaxPWMvalue	1023


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
void	DomeDriverRPi::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
char				lineBuffer[256];
int					mySocketFD;

//	CONSOLE_DEBUG(__FUNCTION__);

	mySocketFD	=	reqData->socket;

	SocketWriteData(mySocketFD,	"<P>\r\n");

	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H2>Raspberry-Pi Dome Driver</H2>\r\n");
	//===============================================================
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TH COLSPAN=3>Raspberry-Pi Dome Driver</TH>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TH COLSPAN=3>Hardware configuration</TH>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n<TR>\r\n");

#ifdef _USE_BCM_PIN_NUMBERS_
	SocketWriteData(mySocketFD,	"<TD COLSPAN=3><CENTER>Using BCM Pin numbering</TH>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n<TR>\r\n");

#endif // _USE_BCM_PIN_NUMBERS_
	sprintf(lineBuffer,	"\t<TD>Clockwise button pin</TD><TD>%d</TD><TD>Input</TD>\r\n", kHWpin_ButtonCW);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n<TR>\r\n");

	sprintf(lineBuffer,	"\t<TD>Counter Clockwise button pin</TD><TD>%d</TD><TD>Input</TD>\r\n", kHWpin_ButtonCCW);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n<TR>\r\n");

	sprintf(lineBuffer,	"\t<TD>Stop button pin</TD><TD>%d</TD><TD>Input</TD>\r\n",	kHWpin_Stop);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n<TR>\r\n");

	sprintf(lineBuffer,	"\t<TD>Direction Control pin</TD><TD>%d</TD><TD>Output</TD>\r\n", kHWpin_Direction);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n<TR>\r\n");

	sprintf(lineBuffer,	"\t<TD>Power PWM pin</TD><TD>%d</TD><TD>Output</TD>\r\n",		kHWpin_PowerPWM);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n<TR>\r\n");

	sprintf(lineBuffer,	"\t<TD>Home Sensor pin</TD><TD>%d</TD><TD>Input</TD>\r\n",	kHWpin_HomeSensor);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n<TR>\r\n");

	sprintf(lineBuffer,	"\t<TD>Park Sensor pin</TD><TD>%d</TD><TD>Input</TD>\r\n",	kHWpin_ParkSensor);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TR>\r\n<TR>\r\n");


	SocketWriteData(reqData->socket,	"</TABLE>\r\n");
	SocketWriteData(reqData->socket,	"</CENTER>\r\n");
	SocketWriteData(reqData->socket,	"<P>\r\n");

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

	pinMode(kHWpin_CommutatorPwr,	OUTPUT);
	pinMode(kHWpin_Direction,	OUTPUT);

	pinMode(kHWpin_PowerPWM,	PWM_OUTPUT);
	pwmWrite(kHWpin_PowerPWM,	0);				//*	make sure its zero.

	pinMode(kHWpin_ButtonCW,	INPUT);
	pinMode(kHWpin_ButtonCCW,	INPUT);
	pinMode(kHWpin_Stop,		INPUT);
	pinMode(kHWpin_HomeSensor,	INPUT);
	pinMode(kHWpin_ParkSensor,	INPUT);

	//*	set the internal pullup resisters
	pullUpDnControl(kHWpin_ButtonCW,	PUD_UP);
	pullUpDnControl(kHWpin_ButtonCCW,	PUD_UP);
	pullUpDnControl(kHWpin_Stop,		PUD_UP);
	pullUpDnControl(kHWpin_HomeSensor,	PUD_UP);
	pullUpDnControl(kHWpin_ParkSensor,	PUD_UP);

#ifdef _ENABLE_COMMUTATOR_POWER_
	pinMode(kHWpin_CommutatorPwr,	OUTPUT);
	digitalWrite(kHWpin_CommutatorPwr, LOW);
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

#endif // _ENABLE_DOME_RPI_
