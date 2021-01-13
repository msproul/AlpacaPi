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
//*****************************************************************************

#ifdef _ENABLE_SWITCH_

#include	<string.h>
#include	<unistd.h>


#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"switchdriver.h"
#include	"switchdriver_rpi.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#ifdef _ENABLE_PWM_SWITCH_
	#define	kAnalogSwitch1		8
	#define	kHWpin_PowerPWM		18
#endif // _ENABLE_PWM_SWITCH_


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
		kHWpin_Channel8

};


#if defined(__arm__)
	#define	_ENABLE_WIRING_PI_
	#include <wiringPi.h>
#else
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




//*****************************************************************************
void	CreateSwitchObjectsRPi(void)
{
	new SwitchDriverRPi();
}

//**************************************************************************************
SwitchDriverRPi::SwitchDriverRPi(void)
	:SwitchDriver()
{
	strcpy(cDeviceName, "Switch-Raspberry-Pi");

	cNumSwitches	=	kR_Pi_SwitchCount;

#ifdef _ENABLE_PWM_SWITCH_
	cNumSwitches	+=	1;
#endif // _ENABLE_PWM_SWITCH_

	Init_Hardware();
}

//**************************************************************************************
SwitchDriverRPi::~SwitchDriverRPi( void )
{
}

//*****************************************************************************
void	SwitchDriverRPi::Init_Hardware(void)
{
int		ii;
char	debugString[64];
int		pinNumber;
int		pinState;
#ifdef _ENABLE_WIRING_PI_
int		wiringPi_rc;
int		wiringPi_verMajor;
int		wiringPi_verMinor;
char	wiringPi_VerString[32];
int		pinValue;

	wiringPiVersion(&wiringPi_verMajor, &wiringPi_verMinor);
	sprintf(wiringPi_VerString, "%d.%d", wiringPi_verMajor, wiringPi_verMinor);
	AddLibraryVersion("software", "wiringPi", wiringPi_VerString);

	wiringPi_rc	=	wiringPiSetupGpio();

	CONSOLE_DEBUG_W_NUM("wiringPi_rc\t=", wiringPi_rc);
#endif	//	_ENABLE_WIRING_PI_

	pinState	=	0;
	CONSOLE_DEBUG("Setting up hardware io pins");
	//*	step through the pin list and set them all to outputs
	for (ii=0; ii<kR_Pi_SwitchCount; ii++)
	{
		pinNumber		=	gRelayControlPinNumbers[ii];
		if ((pinNumber >= 0) && (pinNumber < 50))
		{
			//*	set to output
			pinMode(pinNumber,		OUTPUT);

			//*	on some boards, HIGH is off
			digitalWrite(pinNumber,	TURN_PIN_OFF);

			pinState	=	digitalRead(pinNumber);
			sprintf(debugString, "Switch#%d is pin#%2d state=%d", ii+1, pinNumber, pinState);
			CONSOLE_DEBUG(debugString);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Invalid pin number\t=", pinNumber);
		}
	}



#ifdef _ENABLE_PWM_SWITCH_
	cSwitchType[kAnalogSwitch1]		=	kSwitchType_Analog;
	cMinSwitchValue[kAnalogSwitch1]	=	0;
	cMaxSwitchValue[kAnalogSwitch1]	=	1023;
	cCurSwitchValue[kAnalogSwitch1]	=	0;


	pinMode(kHWpin_PowerPWM,	PWM_OUTPUT);
	pwmWrite(kHWpin_PowerPWM,	0);

//	pwmWrite(kHWpin_PowerPWM,	512);

#endif // _ENABLE_PWM_SWITCH_

}

//*****************************************************************************
int		SwitchDriverRPi::TranslateSwitchToPin(const int switchNumber)
{
int		pinNumber;

	if (switchNumber < kR_Pi_SwitchCount)
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
//	CONSOLE_DEBUG_W_NUM("switchNumber\t=", switchNumber);

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
	if ((switchNumber >= 0) && (switchNumber < cNumSwitches))
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
#endif // _ENABLE_PWM_SWITCH_
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Switch number out of bounds:", switchNumber);
	}
}


#endif // _ENABLE_SWITCH_
