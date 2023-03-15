//**************************************************************************
//*	Name:			obsconditionsdriver_sim.c
//*
//*	Author:			Mark Sproul (C) 2023
//*
//*	Description:
//*
//*	Limitations:
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar  2,	2023	<MLS> Created obsconditionsdriver_sim.cpp
//*****************************************************************************

#ifdef _ENABLE_OBSERVINGCONDITIONS_SIMULATOR_


#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<time.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"obsconditionsdriver.h"
#include	"obsconditionsdriver_sim.h"
#include	"obsconditions_globals.h"

//*****************************************************************************
void	CreateObsConditionObjects_SIM(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	new ObsConditionsDriverSIM(0);
}



//**************************************************************************************
ObsConditionsDriverSIM::ObsConditionsDriverSIM(const int argDevNum)
	:ObsConditionsDriver(argDevNum)
{

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name,		"AlpacaPi ObsCond Simulator");
	strcpy(cCommonProp.Description,	"AlpacaPi ObsCond Simulator");
	strcpy(gEnvData.siteDataSource,	"AlpacaPi ObsCond Simulator");
	strcpy(gEnvData.domeDataSource,	"AlpacaPi ObsCond Simulator");

}

//**************************************************************************************
// Destructor
//**************************************************************************************
ObsConditionsDriverSIM::~ObsConditionsDriverSIM(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
//*	this MUST be implemented by the sub class
//*	change the ones that the sub class supports
//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriverSIM::GetSensorInfo(	TYPE_ObsConSensorType sensorType,
															char	*description,
															double	*timeSinceLastUpdate)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	strcpy(description, "AlpacaPi: Not implemented");

	switch(sensorType)
	{
		case kSensor_CloudCover:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_DewPoint:
			alpacaErrCode	=	kASCOM_Err_Success;
			strcpy(description, "AlpacaPi: Calculated from temp/humidity");
			break;

		case kSensor_Humidity:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_Pressure:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_RainRate:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_SkyBrightness:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_SkyQuality:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_StarFWHM:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_SkyTemperature:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_Temperature:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_WindDirection:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_WindGust:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		case kSensor_WindSpeed:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;

		default:
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			break;
	}
	return(alpacaErrCode);

}


#endif	//	_ENABLE_OBSERVINGCONDITIONS_SIMULATOR_

