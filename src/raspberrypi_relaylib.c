//*****************************************************************************
//*	Name:			raspberrypi_relaylib.c
//*
//*	Author:			Mark Sproul (C) 2021
//*
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
//*	Jan 12,	2021	<MLS> Created raspberrypi_relaylib.c
//*	Jan 12,	2021	<MLS> Relay library working on 4 relay R-Pi hat
//*	Jan 15,	2021	<MLS> Raspberry Pi 64 bit gcc/g++ does not define __arm__
//*	Oct  3,	2023	<MLS> Added initial state to RpiRelay_Init()
//*****************************************************************************
//*	for raspberry pi 4
//wget https://project-downloads.drogon.net/wiringpi-latest.deb
//sudo dpkg -i wiringpi-latest.deb
//*****************************************************************************

#if defined(_ENABLE_DOME_ROR_) || defined(_ENABLE_SWITCH_RPI_)


#include	<stdio.h>
#include	<stdbool.h>
#include	<stdlib.h>
#include	<unistd.h>

//*	this should be defined in the Makefile
//#define	_ENABLE_4REALY_BOARD

#include	"raspberrypi_relaylib.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

//*****************************************************************************
static const int	gRelayControlPinNumbers[]	=
{
//*	these pin number are defined in .h file
//*	using BCM pin numbers
//	8 port relay on DIN rail 	5,	6,	13,	16,	19,	20,	21,	26
//	4 port relay HAT			22, 27, 17, 4


		kHWpin_Channel1,
		kHWpin_Channel2,
		kHWpin_Channel3,
		kHWpin_Channel4,
		kHWpin_Channel5,
		kHWpin_Channel6,
		kHWpin_Channel7,
		kHWpin_Channel8

};


#if defined(__arm__)
	#ifndef _ENABLE_WIRING_PI_
		#define	_ENABLE_WIRING_PI_
	#endif
	#include <wiringPi.h>
#else
	#define	LOW		0
	#define	HIGH	1
	#define	OUTPUT	1

	#define	PWM_OUTPUT	0
//*****************************************************************************
static void	pinMode(int pinNum, int inPut_Output)
{
	CONSOLE_DEBUG(__FUNCTION__);
}
//*****************************************************************************
static int	digitalRead(int pinNum)
{
	CONSOLE_DEBUG(__FUNCTION__);
	return(false);
}
//*****************************************************************************
static void	digitalWrite(int pinNum, int digtialStte)
{
	CONSOLE_DEBUG(__FUNCTION__);
}
#endif





//*****************************************************************************
//*	returns the number of configured relays
//*	currently this is either 4 or 8, depending on which relay board is selected
//*****************************************************************************
int	RpiRelay_Init(const int intialState)
{
int		ii;
char	debugString[64];
int		pinNumber;
int		pinState;
#if defined(_ENABLE_WIRING_PI_) && defined(__arm__)
int		wiringPi_rc;
int		wiringPi_verMajor;
int		wiringPi_verMinor;
char	wiringPi_VerString[32];

	wiringPiVersion(&wiringPi_verMajor, &wiringPi_verMinor);
	sprintf(wiringPi_VerString, "%d.%d", wiringPi_verMajor, wiringPi_verMinor);

	CONSOLE_DEBUG_W_STR("Wiring Pi version:", wiringPi_VerString);


	wiringPi_rc	=	wiringPiSetupGpio();

	CONSOLE_DEBUG_W_NUM("wiringPi_rc\t=", wiringPi_rc);
#endif	//	_ENABLE_WIRING_PI_

	pinState	=	0;
	CONSOLE_DEBUG("Setting up relay hardware io pins");
	//*	step through the pin list and set them all to outputs
	for (ii=0; ii < kR_Pi_RelayCount; ii++)
	{
		pinNumber		=	gRelayControlPinNumbers[ii];
		if ((pinNumber >= 0) && (pinNumber < 50))
		{
			//*	First set the state so it doesnt send a pusle
			digitalWrite(pinNumber,	intialState);
			pinMode(pinNumber,		OUTPUT);

			//*	Now set it again, just to make sure
			digitalWrite(pinNumber,	intialState);

			pinState	=	digitalRead(pinNumber);
			sprintf(debugString, "Switch#%d is pin#%2d state=%d", ii+1, pinNumber, pinState);
			CONSOLE_DEBUG(debugString);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Invalid pin number\t=", pinNumber);
		}
	}
	return(kR_Pi_RelayCount);
}

//*****************************************************************************
//*	Relay numbers start at 1
//*****************************************************************************
static int TranslateSwitchToPin(const int relayNumber)
{
int		pinNumber;
int		myRelayIndex;

	myRelayIndex	=	relayNumber - 1;

	if ((myRelayIndex >= 0) &&  (myRelayIndex < kR_Pi_RelayCount))
	{
		pinNumber	=	gRelayControlPinNumbers[myRelayIndex];
	}
	else
	{
		pinNumber	=	-1;
	}
	return(pinNumber);
}

//*****************************************************************************
bool	RpiRelay_SetRelay(const int relayNumber, bool newState)	//*	returns true if OK
{
int		pinNumber;
bool	okFlag;

//	CONSOLE_DEBUG_W_NUM("relayNumber\t=", relayNumber);

	pinNumber	=	TranslateSwitchToPin(relayNumber);
//	CONSOLE_DEBUG_W_NUM("pinNumber\t=", pinNumber);
	if (pinNumber > 0)
	{
		okFlag	=	true;
		if (newState)
		{
			//*	some boards have reverse logic
			digitalWrite(pinNumber, TURN_PIN_ON);
		}
		else
		{
			digitalWrite(pinNumber, TURN_PIN_OFF);
		}
	}
	else
	{
		okFlag	=	false;
	}
	return(okFlag);
}

//*****************************************************************************
bool	RpiRelay_GetRelay(const int relayNumber)
{
int		pinNumber;
int		pinValue;
bool	switchState;

//	CONSOLE_DEBUG(__FUNCTION__);
	switchState	=	false;
	pinNumber	=	TranslateSwitchToPin(relayNumber);

	if (pinNumber > 0)
	{
		pinValue	=	digitalRead(pinNumber);
		if (pinValue == TURN_PIN_ON)
		{
			switchState	=	true;
		}
		else
		{
			switchState	=	false;
		}
	}
	return(switchState);
}
#endif // defined(_ENABLE_DOME_ROR_) || defined(_ENABLE_DOME_) || defineED(_ENABLE_SWITCH_)


#ifdef _ENABLE_RELAY_TESTING_
//*****************************************************************************
//*	for testing
//*****************************************************************************
int	main(int argc, char **argv)
{
int		relayCount;
int		iii;
int		relayNum;

	relayCount	=	RpiRelay_Init();
	CONSOLE_DEBUG_W_NUM("relayCount\t=", relayCount);

	for (iii=0; iii<3; iii++)
	{
		for (relayNum=1; relayNum<=relayCount; relayNum++)
		{
			CONSOLE_DEBUG_W_NUM("relayNum\t=", relayNum);
			RpiRelay_SetRelay(relayNum, true);
			sleep(1);

			RpiRelay_SetRelay(relayNum, false);
			sleep(1);
		}
	}
}

#endif	//	_ENABLE_RELAY_TESTING_


