//**************************************************************************
//*	Name:			calibrationdriver_rpi.cpp
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:	This driver does not implement any actual device,
//*					you must create a sub-class that does the actual control
//*
//*	References:		https://ascom-standards.org/api/
//*					https://ascom-standards.org/Help/Developer/html/N_ASCOM_DeviceInterface.htm
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Sep  1,	2020	<MLS> Created calibrationdriver.cpp
//*****************************************************************************


#ifdef _ENABLE_CALIBRATION_

#include	<string.h>
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpacadriver_helper.h"
#include	"calibrationdriver.h"
#include	"calibrationdriver_rpi.h"

#ifdef _ENABLE_WIRING_PI_
	#include <wiringPi.h>
	#define	kHWpin_PowerPWM		18
#endif // _ENABLE_WIRING_PI_


//*****************************************************************************
void	CreateCalibrationObjectsRPi(void)
{
	new CalibrationDriverRPI();
}

//**************************************************************************************
CalibrationDriverRPI::CalibrationDriverRPI(void)
	:CalibrationDriver()
{
	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cDeviceName, "CoverCalibration-Raspberry-Pi");


	Init_Hardware();
}

//**************************************************************************************
CalibrationDriverRPI::~CalibrationDriverRPI( void )
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
void	CalibrationDriverRPI::Init_Hardware(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

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


	pinMode(kHWpin_PowerPWM,	PWM_OUTPUT);
	pwmWrite(kHWpin_PowerPWM,	0);


#endif	//	_ENABLE_WIRING_PI_

}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverRPI::Calibrator_TurnOn(const int brightnessValue, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _ENABLE_WIRING_PI_
	//*	check to make sure the value is within the range of the R-Pi PWM
	if ((brightnessValue >= 0) && (brightnessValue <= 1023))
	{
		cCalibratorBrightness	=	brightnessValue;
		pwmWrite(kHWpin_PowerPWM,	cCalibratorBrightness);
		alpacaErrCode			=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Brightness level out of range (0 <-> 1023)");
	}
#else

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");

#endif // _ENABLE_WIRING_PI_

	return(alpacaErrCode);

}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverRPI::Calibrator_TurnOff(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _ENABLE_WIRING_PI_

	cCalibratorBrightness	=	0;
	pwmWrite(kHWpin_PowerPWM,	0);
	alpacaErrCode	=	kASCOM_Err_Success;

#else

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
#endif // _ENABLE_WIRING_PI_

//	CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=", alpacaErrCode);

	return(alpacaErrCode);
}



#endif	//	_ENABLE_CALIBRATION_
