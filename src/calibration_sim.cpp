//**************************************************************************
//*	Name:			calibration_sim.cpp
//*
//*	Author:			Mark Sproul (C) 2023
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
//*	Mar  3,	2023	<MLS> Created calibration_sim.cpp
//*	Mar  4,	2023	<MLS> CONFORMU-covercalibration/simulator -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*****************************************************************************


#ifdef _ENABLE_CALIBRATION_SIMULATOR_

#include	<errno.h>
#include	<stdlib.h>
#include	<string.h>
#include	<termios.h>
#include	<fcntl.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"calibrationdriver.h"
#include	"calibration_sim.h"


//*****************************************************************************
void	CreateCalibrationObjects_SIM(void)
{
	new CalibrationDriverSIM();
}

//**************************************************************************************
CalibrationDriverSIM::CalibrationDriverSIM(void)
	:CalibrationDriver()
{
	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name,		"AlpacaPi Calibrarion Simulator");
	strcpy(cCommonProp.Description,	"AlpacaPi Calibrarion Simulator");

	cCoverCalibrationProp.Brightness		=	0;
	cCoverCalibrationProp.CalibratorState	=	kCalibrator_Ready;
	cCoverCalibrationProp.CoverState		=	kCover_Closed;
	cCoverCalibrationProp.MaxBrightness		=	255;
	cCoverCalibrationProp.CanSetAperture	=	true;

	Init_Hardware();
}

//**************************************************************************************
CalibrationDriverSIM::~CalibrationDriverSIM( void )
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
void	CalibrationDriverSIM::Init_Hardware(void)
{

	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
int32_t	CalibrationDriverSIM::RunStateMachine(void)
{
uint32_t	deltaTime;

	switch(cCoverCalibrationProp.CoverState)
	{
		case kCover_NotPresent:		//*	This device does not have a cover that can be closed independently
		case kCover_Closed:			//*	The cover is closed
			break;

		//*	this is so there is an actual time delay
		case kCover_Moving:
			deltaTime	=	millis() - cCoverMovementStartTime;
			if (deltaTime > 5000)
			{
				cCoverCalibrationProp.CoverState	=	cCoverDesiredPostion;
			}
			break;

		case kCover_Open:		//*	The cover is open
		case kCover_Unknown:	//*	The state of the cover is unknown
		case kCover_Error:		//*	The device encountered an error when changing state
			break;
	}

	return(2 * 1000 * 1000);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverSIM::Calibrator_TurnOn(const int brightnessValue, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);

	cCoverCalibrationProp.Brightness		=	brightnessValue;
	cCoverCalibrationProp.CalibratorState	=	kCalibrator_Ready;

	alpacaErrCode	=	kASCOM_Err_Success;

	return(alpacaErrCode);

}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverSIM::Calibrator_TurnOff(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);

	cCoverCalibrationProp.Brightness		=	0;
	cCoverCalibrationProp.CalibratorState	=	kCalibrator_Off;

	alpacaErrCode	=	kASCOM_Err_Success;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverSIM::Cover_Open(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	cCoverCalibrationProp.CoverState	=	kCover_Moving;
	cCoverDesiredPostion				=   kCover_Open;
	cCoverMovementStartTime				=	millis();

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverSIM::Cover_Close(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	cCoverCalibrationProp.CoverState	=	kCover_Moving;
	cCoverDesiredPostion				=   kCover_Closed;
	cCoverMovementStartTime				=	millis();

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverSIM::Cover_Halt(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	cCoverCalibrationProp.CoverState	=	kCover_Unknown;

	alpacaErrCode	=	kASCOM_Err_Success;
	return(alpacaErrCode);
}


#endif	//	_ENABLE_CALIBRATION_SIMULATOR_

