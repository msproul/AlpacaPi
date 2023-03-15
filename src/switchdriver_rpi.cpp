//**************************************************************************
//*	Name:			switchdriver_rpi.cpp
//*
//*	Author:			Mark Sproul
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
//*	Dec 26,	2019	<MLS> Created switchdriver_rpi.cpp
//*	Mar 13,	2020	<MLS> No longer reset switches to off on startup
//*	Apr 13,	2020	<MLS> Turn them off at startup because they default to on when rebooted
//*	Apr 14,	2020	<MLS> Working on analog switch (Pin 18, PWM)
//*	Apr 16,	2020	<MLS> Added ability to disable PWM analog switch via _ENABLE_PWM_SWITCH_
//*	Dec  1,	2020	<MLS> Pins are reset to OFF on startup
//*	Jan 11,	2021	<MLS> Added support for alternate high/low for relay on/off
//*	Jun 24,	2021	<MLS> Updated SetSwitchValue() to handle on/off values
//*	Jan  1,	2022	<MLS> Upgraded Raspberry-Pi 4 to wiringPi 2.5.2
//*	Jan  1,	2022	<MLS> Added _ENABLE_STATUS_SWITCH_, specified in Makefile
//*	Jan  1,	2022	<MLS> Status Read Only switches now supported.
//*	Jan  1,	2022	<MLS> Able to use Commutator power relay to read AC power status on dome
//*****************************************************************************
//*	WiringPi for raspberry pi 4
//	wget https://project-downloads.drogon.net/wiringpi-latest.deb
//	sudo dpkg -i wiringpi-latest.deb
//*****************************************************************************

#ifdef _ENABLE_SWITCH_RPI_

#include	<string.h>
#include	<unistd.h>


#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"switchdriver.h"
#include	"switchdriver_rpi.h"
#include	"eventlogging.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#ifdef _ENABLE_PWM_SWITCH_
	#define	kAnalogSwitch1		8
	#define	kHWpin_PowerPWM		18
#endif // _ENABLE_PWM_SWITCH_

#ifdef _ENABLE_STATUS_SWITCH_
	//*	using BCM pin numbers
	#define		kStatusPin1	23
	#define		kStatusPin2	24
	#define		kStatusPin3	25
#endif // _ENABLE_STATUS_SWITCH_


//*****************************************************************************
void	CreateSwitchObjects_RPi(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	new SwitchDriverRPi();
}

//*****************************************************************************
const int	gRelayControlPinNumbers[]	=
{
//	8 port relay on DIN rail 	5,	6,	13,	16,	19,	20,	21,	26
//	4 port relay HAT			22, 27, 17, 4


		kHWpin_Channel1,
		kHWpin_Channel2,
		kHWpin_Channel3,
		kHWpin_Channel4,
		kHWpin_Channel5,
		kHWpin_Channel6,
		kHWpin_Channel7,
		kHWpin_Channel8,
	#ifdef _ENABLE_PWM_SWITCH_
		kHWpin_PowerPWM,
	#endif

	#ifdef _ENABLE_STATUS_SWITCH_
		kStatusPin1,
		kStatusPin2,
		kStatusPin3,
	#endif
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1
};


#if defined(__arm__)
	#ifndef _ENABLE_WIRING_PI_
		#define	_ENABLE_WIRING_PI_
	#endif
	#include <wiringPi.h>
#else
	//*****************************************************************************
	//*	this is for simulation on non raspberry Pi
	#define	LOW		0
	#define	HIGH	1
	#define	OUTPUT	1

	#define	PWM_OUTPUT	0
//*****************************************************************************
static void	pinMode(int pinNum, int inPut_Output)
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
//static void	pwmWrite(int pinNum, int pwmValue)
//{
//}
#endif





//**************************************************************************************
SwitchDriverRPi::SwitchDriverRPi(void)
	:SwitchDriver()
{
	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name, "Switch-Raspberry-Pi");
#ifdef _ENABLE_4REALY_BOARD
	strcpy(cCommonProp.Description,	"Switch utilizing R-Pi 4 channel relay board");
#else
	strcpy(cCommonProp.Description,	"Switch utilizing R-Pi 8 channel relay board");
#endif

	cSwitchProp.MaxSwitch	=	kR_Pi_RelayCount;

#ifdef _ENABLE_PWM_SWITCH_
	cSwitchProp.MaxSwitch	+=	1;
#endif // _ENABLE_PWM_SWITCH_



	Init_Hardware();
	CONSOLE_DEBUG("exit");
}

//**************************************************************************************
SwitchDriverRPi::~SwitchDriverRPi( void )
{
}

//*****************************************************************************
void	SwitchDriverRPi::Init_Hardware(void)
{
int		iii;
char	debugString[64];
int		pinNumber;
int		pinState;
#ifdef _ENABLE_WIRING_PI_
int		wiringPi_rc;
int		wiringPi_verMajor;
int		wiringPi_verMinor;
char	wiringPi_VerString[32];


	wiringPiVersion(&wiringPi_verMajor, &wiringPi_verMinor);
	sprintf(wiringPi_VerString, "%d.%d", wiringPi_verMajor, wiringPi_verMinor);
	AddLibraryVersion("software", "wiringPi", wiringPi_VerString);

	wiringPi_rc	=	wiringPiSetupGpio();

	CONSOLE_DEBUG_W_NUM("wiringPi_rc\t=", wiringPi_rc);
	LogEvent(	cAlpacaName,
				"WiringPi",
				NULL,
				kASCOM_Err_Success,
				wiringPi_VerString);
#endif	//	_ENABLE_WIRING_PI_

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG("Setting up hardware io pins");
	//*	step through the pin list and set them all to outputs
	for (iii=0; iii<kR_Pi_RelayCount; iii++)
	{
		pinNumber		=	gRelayControlPinNumbers[iii];
		if ((pinNumber >= 0) && (pinNumber < 50))
		{
			ConfigureSwitch(iii, kSwitchType_Relay, pinNumber, TURN_PIN_ON);
			//*	set to output
			pinMode(pinNumber,		OUTPUT);

			//*	on some boards, HIGH is off
			digitalWrite(pinNumber,	TURN_PIN_OFF);

			pinState	=	digitalRead(pinNumber);
			sprintf(debugString, "Switch#%d is pin#%2d state=%d", iii+1, pinNumber, pinState);
			CONSOLE_DEBUG(debugString);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Invalid pin number\t=", pinNumber);
		}
	}

#ifdef _ENABLE_PWM_SWITCH_
	CONSOLE_DEBUG("_ENABLE_PWM_SWITCH_");

	ConfigureSwitch(kAnalogSwitch1, kSwitchType_Analog, kHWpin_PowerPWM);

	cMinSwitchValue[kAnalogSwitch1]		=	0;
	cMaxSwitchValue[kAnalogSwitch1]		=	1023;
	cCurSwitchValue[kAnalogSwitch1]		=	0;


	pinMode(kHWpin_PowerPWM,	PWM_OUTPUT);
	pwmWrite(kHWpin_PowerPWM,	0);
#endif // _ENABLE_PWM_SWITCH_

#ifdef _ENABLE_STATUS_SWITCH_
	CONSOLE_DEBUG("_ENABLE_STATUS_SWITCH_");

	//*	NOTE: cSwitchProp.MaxSwitch gets incremented in ConfigureSwitch()
	ConfigureSwitch(cSwitchProp.MaxSwitch, kSwitchType_Status, kStatusPin1);
	ConfigureSwitch(cSwitchProp.MaxSwitch, kSwitchType_Status, kStatusPin2);
	ConfigureSwitch(cSwitchProp.MaxSwitch, kSwitchType_Status, kStatusPin3);

	for (iii=kR_Pi_RelayCount; iii<kMaxSwitchCnt; iii++)
	{
		pinNumber		=	gRelayControlPinNumbers[iii];
		if ((pinNumber >= 0) && (pinNumber < 50))
		{
			if (cSwitchTable[iii].switchType == kSwitchType_Status)
			{
				//*	set to output
				pinMode(pinNumber,		INPUT);

				pinState	=	digitalRead(pinNumber);
				sprintf(debugString, "Switch#%d is pin#%2d state=%d", iii+1, pinNumber, pinState);
				CONSOLE_DEBUG(debugString);
			}
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Invalid pin number\t=", pinNumber);
		}
	}
#endif // _ENABLE_STATUS_SWITCH_

	CONSOLE_DEBUG("exit");

}

//*****************************************************************************
int		SwitchDriverRPi::TranslateSwitchToPin(const int switchNumber)
{
int		pinNumber;

	if (switchNumber < kMaxSwitchCnt)
	{
		pinNumber	=	gRelayControlPinNumbers[switchNumber];
	}
	else
	{
		pinNumber	=	-1;
	}
	return(pinNumber);
}

//*****************************************************************************
bool	SwitchDriverRPi::GetSwitchState(const int switchNumber)
{
int		pinNumber;
int		pinValue;
bool	switchState;

//	CONSOLE_DEBUG(__FUNCTION__);
	switchState	=	false;
	pinNumber	=	TranslateSwitchToPin(switchNumber);
//	CONSOLE_DEBUG_W_NUM("switchNumber\t=",	switchNumber);
//	CONSOLE_DEBUG_W_NUM("pinNumber\t=",		pinNumber);

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
//		CONSOLE_DEBUG_W_NUM("switchState\t=", switchState);
	}
#ifdef _ENABLE_PWM_SWITCH_
	else if (switchNumber == kAnalogSwitch1)
	{
		CONSOLE_DEBUG("kAnalogSwitch1");
		switchState	=	(cCurSwitchValue[switchNumber] > 0);
	}
#endif // _ENABLE_PWM_SWITCH_
	else
	{
		CONSOLE_DEBUG_W_NUM("Switch number out of bounds:", switchNumber);
	}
	return(switchState);
}

//*****************************************************************************
void	SwitchDriverRPi::SetSwitchState(const int switchNumber, bool on_off)
{
int		pinNumber;
//int		pinValue;

//	CONSOLE_DEBUG(__FUNCTION__);

	pinNumber	=	TranslateSwitchToPin(switchNumber);
//	CONSOLE_DEBUG_W_NUM("switchNumber\t=", switchNumber);
	if (pinNumber > 0)
	{
		if (on_off)
		{
			//*	some boards have reverse logic
			digitalWrite(pinNumber, TURN_PIN_ON);
		//	digitalWrite(pinNumber, LOW);
		}
		else
		{
			//*	HIGH turns the relay OFF
			digitalWrite(pinNumber, TURN_PIN_OFF);
		//	digitalWrite(pinNumber, HIGH);
		}
//		CONSOLE_DEBUG_W_NUM("on_off\t\t=", on_off);
//		pinValue	=	digitalRead(pinNumber);
//		CONSOLE_DEBUG_W_NUM("pinValue\t\t=", pinValue);
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Switch number out of bounds:", switchNumber);
	}
}

//*****************************************************************************
void	SwitchDriverRPi::SetSwitchValue(const int switchNumber, double switchValue)
{
	CONSOLE_DEBUG(__FUNCTION__);
	if ((switchNumber >= 0) && (switchNumber < cSwitchProp.MaxSwitch))
	{
#ifdef _ENABLE_PWM_SWITCH_
		if (switchNumber == kAnalogSwitch1)
		{
		int	pwmValueInt;

			//*	set the PWM value
			pwmValueInt	=	switchValue;
			CONSOLE_DEBUG_W_NUM("Setting PWM value to ", pwmValueInt);
			pwmWrite(kHWpin_PowerPWM,	pwmValueInt);
		}
		else
#endif // _ENABLE_PWM_SWITCH_
		{
			//*	if its not an anlog switch, turn it on or off. 0=off anything else = on
			SetSwitchState(switchNumber, (switchValue > 0.0));
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Switch number out of bounds:", switchNumber);
	}
}



#endif // _ENABLE_SWITCH_RPI_
