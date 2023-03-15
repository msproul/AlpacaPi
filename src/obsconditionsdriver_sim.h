//**************************************************************************
//*	Name:			obsconditionsdriver_sim.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar  2,	2023	<MLS> Created obsconditionsdriver_sim.h
//*****************************************************************************
//#include	"obsconditionsdriver_sim.h"


#ifndef _OBSERVINGCONDITIONSDRIVER_SIM_H_
#define	_OBSERVINGCONDITIONSDRIVER_SIM_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif
#ifndef _OBSERVINGCONDITIONSDRIVER_H_
	#include	"obsconditionsdriver.h"
#endif


void	CreateObsConditionObjects_SIM(void);

//**************************************************************************************
class ObsConditionsDriverSIM: public ObsConditionsDriver
{
	public:

		//
		// Construction
		//
						ObsConditionsDriverSIM(const int argDevNum);
		virtual			~ObsConditionsDriverSIM(void);



	protected:
//		virtual	double	ReadPressure_kPa(void);
//		virtual	double	ReadTemperature(void);
//		virtual	double	ReadHumidity(void);

virtual	TYPE_ASCOM_STATUS	GetSensorInfo(			TYPE_ObsConSensorType sensorType,
													char	*description,
													double	*timeSinceLastUpdate);


};

#endif	//	_OBSERVINGCONDITIONSDRIVER_SIM_H_
