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
//*****************************************************************************

#ifdef _ENABLE_ROR_

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

	Init_Hardware();
	LogEvent(	"dome",
				"ROR created",
				NULL,
				kASCOM_Err_Success,
				"");

	strcpy(cDeviceName,			"AlpacaPi-Dome");
	strcpy(cDeviceDescription,	"ROR");
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

//	pinMode(kHWpin_PowerOnOff,	OUTPUT);
//	pinMode(kHWpin_Direction,	OUTPUT);

//	pinMode(kHWpin_PowerPWM,	PWM_OUTPUT);

//	pinMode(kHWpin_ButtonCW,	INPUT);
//	pinMode(kHWpin_ButtonCCW,	INPUT);
//	pinMode(kHWpin_Stop,		INPUT);
//	pinMode(kHWpin_HomeSensor,	INPUT);
//	pinMode(kHWpin_ParkSensor,	INPUT);

//	pullUpDnControl(kHWpin_ButtonCW,	PUD_UP);
//	pullUpDnControl(kHWpin_ButtonCCW,	PUD_UP);
//	pullUpDnControl(kHWpin_Stop,		PUD_UP);
//	pullUpDnControl(kHWpin_HomeSensor,	PUD_UP);
//	pullUpDnControl(kHWpin_ParkSensor,	PUD_UP);


#endif	//	_ENABLE_DOME_HARDWARE_

}

//*****************************************************************************
//*	this should be over ridden
TYPE_ASCOM_STATUS	DomeDriverROR::OpenShutter(void)
{
	return(kASCOM_Err_ActionNotImplemented);
}

//*****************************************************************************
//*	this should be over ridden
TYPE_ASCOM_STATUS	DomeDriverROR::CloseShutter(void)
{
	return(kASCOM_Err_ActionNotImplemented);
}





#endif // _ENABLE_ROR_
