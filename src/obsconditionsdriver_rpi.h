//**************************************************************************
//*	Name:			obsconditionsdriver_rpi.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*****************************************************************************
//#include	"obsconditionsdriver_rpi.h"


#ifndef _OBSERVINGCONDITIONSDRIVER_RPI_H_
#define	_OBSERVINGCONDITIONSDRIVER_RPI_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif
#ifndef _OBSERVINGCONDITIONSDRIVER_H_
	#include	"obsconditionsdriver.h"
#endif

#ifdef __arm__
//	#include <wiringPi.h>
//	#define	_ENABLE_PI_HAT_SESNSOR_BOARD_
	#define	_ENABLE_RTIMULib_
#endif

#ifdef _ENABLE_RTIMULib_
	#include "RTIMULib.h"
#endif

void	CreateObsConditionRpiObjects(void);

//**************************************************************************************
class ObsConditionsDriverRpi: public ObsConditionsDriver
{
	public:

		//
		// Construction
		//
						ObsConditionsDriverRpi(const int argDevNum);
		virtual			~ObsConditionsDriverRpi(void);



	protected:
		virtual	double	ReadPressure_kPa(void);
		virtual	double	ReadTemperature(void);
		virtual	double	ReadHumidity(void);

virtual	TYPE_ASCOM_STATUS	GetSensorInfo(			TYPE_ObsConSensorType sensorType,
													char	*description,
													double	*timeSinceLastUpdate);

				void	ReadSenseHat_Pressure(void);
				void	ReadSenseHat_Humidity(void);

	#ifdef _ENABLE_RTIMULib_
		RTIMUSettings	*rt_settings;
		RTIMU			*rt_imu;
		RTPressure		*rt_pressure;
		RTHumidity		*rt_humidity;
	#endif	//	_ENABLE_RTIMULib_
		int				foo;	//*	this can be deleted, put here to fix pmcabe error
};

#endif	//	_OBSERVINGCONDITIONSDRIVER_RPI_H_
