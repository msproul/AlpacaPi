//**************************************************************************
//*	Name:			obsconditionsdriver.c
//*
//*	Author:			Mark Sproul (C) 2019, 2020
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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	May  7,	2019	<MLS> Started on observingconditions
//*	May  7,	2019	<MLS> Pi hat sensor pressure/temp working
//*	May  7,	2019	<MLS> Pi hat sensor humidity working
//*	May  9,	2019	<MLS> Added ObservCond_OutputHTML()
//*	Dec 30,	2019	<MLS> Observingconditions converted to c++
//*	Dec 31,	2019	<MLS> Added averaging to temp/pres/humid readings
//*	Jan  1,	2020	<MLS> Confirmed that atmosphere is in Hectopascals
//*	Jan 24,	2020	<MLS> Fixed kiloPascals vs hectoPascals conversion issues
//*	Apr 20,	2020	<MLS> Added Get_Readall()
//*	Jun 27,	2020	<MLS> Fixed bug in Get_TimeSinceLastUpdate()
//*	Jun 27,	2020	<MLS> Updated routine names to be more consistent
//*	Sep  8,	2020	<MLS> Updated routines to return TYPE_ASCOM_STATUS
//*	Mar  1,	2021	<MLS> Added GetSensorEnum() & GetSensorInfo()
//*	Mar  1,	2021	<MLS> Added Put_AveragePeriod()
//*	Mar  1,	2021	<MLS> CONFORM-observingconditions -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	Sep 26,	2022	<MLS> Major re-org of how sensors are enabled
//*	Nov 27,	2022	<MLS> CONFORMU-observingconditions -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	Jun  4,	2023	<MLS> Updated compile time ifdefs
//*	Jun 18,	2023	<MLS> Added DeviceState_Add_Content() to obsConditions driver
//*	May 17,	2024	<MLS> Added http error 400 processing to obsConditions driver
//*****************************************************************************



#ifdef _ENABLE_OBSERVINGCONDITIONS_

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<time.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#ifdef __arm__
	#include <wiringPi.h>
//	#define	_ENABLE_PI_HAT_SESNSOR_BOARD_	//*	moved to Makefile
#endif

#ifdef _ENABLE_PI_HAT_SESNSOR_BOARD_
	#include	<unistd.h>
	#include	<linux/i2c-dev.h>
//	#include	<i2c/smbus.h>
	#include	<fcntl.h>
	#include	<sys/ioctl.h>
#endif

#include	"JsonResponse.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"obsconditionsdriver.h"
#include	"obsconditions_globals.h"

#ifndef _OBSERVINGCONDITIONSDRIVER_RPI_H_
	#include	"obsconditionsdriver_rpi.h"
#endif

#ifdef _ENABLE_OBSERVINGCONDITIONS_SIMULATOR_
	#include	"obsconditionsdriver_sim.h"
#endif

#include	"obscond_AlpacaCmds.h"
#include	"obscond_AlpacaCmds.cpp"

//*****************************************************************************
void	CreateObsConditionObjects(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

#if defined(__arm__) && defined(_ENABLE_OBSERVINGCONDITIONS_RPI_)
	CreateObsConditionObjects_RPi();
#endif

#ifdef _ENABLE_OBSERVINGCONDITIONS_SIMULATOR_
	CreateObsConditionObjects_SIM();
#endif
}


//**************************************************************************************
ObsConditionsDriver::ObsConditionsDriver(const int argDevNum)
	:AlpacaDriver(kDeviceType_Observingconditions)
{
int	ii;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name,		"ObsConditionsDriver");
	strcpy(cCommonProp.Description,	"AlpacaPi Observing Condition");

	cDriverCmdTablePtr			=	gObsCondCmdTable;

	//--------------------------------------------------------
	//*	clear the entire properties list data structure
	//*	each subclass MUST set the
	//*			cObsConditionProp.xxx.IsSupported to true
	//*	for each device that it supports
	memset(&cObsConditionProp, 0, sizeof(TYPE_ObsConditionProperties));

	//*	all drivers must support AveragePeriod, so set to true here.
	//*	however, since it is required, the IsSupported is ignored most places
	cObsConditionProp.Averageperiod.IsSupported	=	true;
	cObsConditionProp.Averageperiod.Value		=	0.0;

	cObsConditionProp.Averageperiod.Value	=	(kAvgSampleCount * kSampleDetlaSecs * 1.0) / 3600.0;

	cCurrentPressure_kPa					=	0;		//*	kPa
	cSuccesfullReadCnt						=	0;		//*	used to know if we have active sensors
	cTimeOfLastUpdate_secs					=	0;		//*	time in seconds
	cObservCondState						=	kObservCondState_Startup;


	//*	zero out the averaging arrays
	for (ii=0; ii<kAvgSampleCount; ii++)
	{
		cPressureArray[ii]		=	0.0;
		cTemperatureArray[ii]	=	0.0;
		cHumidityArray[ii]		=	0.0;
	}
}


//**************************************************************************************
// Destructor
//**************************************************************************************
ObsConditionsDriver::~ObsConditionsDriver(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
char				alpacaErrMsg[256];
int					cmdEnumValue;
int					cmdType;
int					mySocket;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, reqData->deviceCommand);


	//*	make local copies of the data structure to make the code easier to read
	mySocket	=	reqData->socket;
//	myDeviceNum	=	reqData->deviceNumber;

	strcpy(alpacaErrMsg, "");
	alpacaErrCode	=	kASCOM_Err_Success;

	//*	set up the json response
	JsonResponse_CreateHeader(reqData->jsonTextBuffer);

	//*	this is not part of the protocol, I am using it for testing
	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Device",
								cCommonProp.Name,
								INCLUDE_COMMA);

	//*	this is not part of the protocol, I am using it for testing
	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Command",
								reqData->deviceCommand,
								INCLUDE_COMMA);

	//*	look up the command
	cmdEnumValue	=	FindCmdFromTable(reqData->deviceCommand, gObsCondCmdTable, &cmdType);
	switch(cmdEnumValue)
	{
		//----------------------------------------------------------------------------------------
		//*	Common commands that we want to over ride
		//----------------------------------------------------------------------------------------
		case kCmd_Common_supportedactions:	//*	Returns the list of action names supported by this driver.
			alpacaErrCode	=	Get_SupportedActions(reqData, gObsCondCmdTable);
			break;

		//----------------------------------------------------------------------------------------
		//*	Device specific commands
		//----------------------------------------------------------------------------------------
		case kCmd_ObservCond_averageperiod:	//*	Returns the time period over which observations will be averaged
											//*	Sets the time period over which observations will be averaged
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_AveragePeriod(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_AveragePeriod(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_ObservCond_cloudcover:	//*	Returns the amount of sky obscured by cloud
			alpacaErrCode	=	Get_CloudCover(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_dewpoint:		//*	Returns the atmospheric dew point at the observatory
			alpacaErrCode	=	Get_DewPoint(reqData, alpacaErrMsg, gValueString);
			break;


		case kCmd_ObservCond_humidity:		//*	Returns the atmospheric humidity at the observatory
			alpacaErrCode	=	Get_Humidity(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_pressure:		//*	Returns the atmospheric pressure at the observatory.
											//*	units = hectoPascals
			alpacaErrCode	=	Get_Pressure(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_rainrate:				//*	Returns the rain rate at the observatory.
			alpacaErrCode	=	Get_RainRate(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_skybrightness:			//*	Returns the sky brightness at the observatory
			alpacaErrCode	=	Get_SkyBrightness(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_skyquality:				//*	Returns the sky quality at the observatory
			alpacaErrCode	=	Get_SkyQuality(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_skytemperature:			//*	Returns the sky temperature at the observatory
			alpacaErrCode	=	Get_SkyTemperature(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_starfwhm:				//*	Returns the seeing at the observatory
			alpacaErrCode	=	Get_StarFWHM(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_temperature:			//*	Returns the temperature at the observatory
			alpacaErrCode	=	Get_Temperature(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_winddirection:			//*	Returns the wind direction at the observatory
			alpacaErrCode	=	Get_WindDirection(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_windgust:				//*	Returns the peak 3 second wind gust at the observatory over the last 2 minutes
			alpacaErrCode	=	Get_WindGust(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_windspeed:				//*	Returns the wind speed at the observatory.
			alpacaErrCode	=	Get_WindSpeed(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_refresh:				//*	Refreshes sensor values from hardware.
			alpacaErrCode	=	Put_Refresh(reqData, alpacaErrMsg);
			break;

		case kCmd_ObservCond_sensordescription:		//*	Return a sensor description
			alpacaErrCode	=	Get_SensorDescription(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_timesincelastupdate:	//*	Return the time since the sensor value was last updated
			alpacaErrCode	=	Get_TimeSinceLastUpdate(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_readall:
			alpacaErrCode	=	Get_Readall(reqData, alpacaErrMsg);
			break;

		//----------------------------------------------------------------------------------------
		//*	let anything undefined go to the common command processor
		//----------------------------------------------------------------------------------------
		default:
			alpacaErrCode	=	ProcessCommand_Common(reqData, cmdEnumValue, alpacaErrMsg);
			break;
	}
	RecordCmdStats(cmdEnumValue, reqData->get_putIndicator, alpacaErrCode);

	if (cSendJSONresponse)	//*	False for setupdialog and camera binary data
	{
		//*	send the response information
		JsonResponse_Add_Uint32(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"ClientTransactionID",
									reqData->ClientTransactionID,
									INCLUDE_COMMA);

		JsonResponse_Add_Uint32(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"ServerTransactionID",
									gServerTransactionID,
									INCLUDE_COMMA);

		JsonResponse_Add_Int32(		mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"ErrorNumber",
									alpacaErrCode,
									INCLUDE_COMMA);

		JsonResponse_Add_String(	mySocket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"ErrorMessage",
									alpacaErrMsg,
									NO_COMMA);

		JsonResponse_Add_Finish(	mySocket,
									reqData->httpRetCode,
									reqData->jsonTextBuffer,
									(cHttpHeaderSent == false));
	}
	//*	this is for the logging function
	strcpy(reqData->alpacaErrMsg, alpacaErrMsg);
	return(alpacaErrCode);
}




//*****************************************************************************
//*	return number of microseconds allowed for delay
//*****************************************************************************
int32_t	ObsConditionsDriver::RunStateMachine(void)
{
time_t	currentTimeSecs;
int		ii;

//	CONSOLE_DEBUG(__FUNCTION__);

	currentTimeSecs	=	time(NULL);

	switch(cObservCondState)
	{
		//*	the first time thru we are going to read multiple times to fill the buffers
		case kObservCondState_Startup:
			for (ii=0; ii<kAvgSampleCount; ii++)
			{
				UpdateSensorsReadings();
			}
			cTimeOfLastUpdate_secs	=	time(NULL);
			cObservCondState		=	kObservCondState_Idle;
			break;

		case kObservCondState_Idle:
			if ((currentTimeSecs - cTimeOfLastUpdate_secs) >= kSampleDetlaSecs)
			{
				UpdateSensorsReadings();

				cTimeOfLastUpdate_secs	=	time(NULL);
			}
			break;

		default:
			break;
	}

	return(10 * 1000 * 1000);	//*	10 seconds
}

//*****************************************************************************
void	ObsConditionsDriver::UpdateSensorsReadings(void)
{
int		ii;
double	pressureTotal;
double	temperatureTotal;
double	humidityTotal;

//	CONSOLE_DEBUG(__FUNCTION__);
	//*	move everything up in the array
	for (ii=0; ii< (kAvgSampleCount - 1); ii++)
	{
		cPressureArray[ii]		=	cPressureArray[ii + 1];
		cTemperatureArray[ii]	=	cTemperatureArray[ii + 1];
		cHumidityArray[ii]		=	cHumidityArray[ii + 1];
	}
	//*	read the new values
	cPressureArray[kAvgSampleCount - 1]		=	ReadPressure_kPa();
	cTemperatureArray[kAvgSampleCount - 1]	=	ReadTemperature();
	cHumidityArray[kAvgSampleCount - 1]		=	ReadHumidity();

	//*	now average them
	pressureTotal		=	0.0;
	temperatureTotal	=	0.0;
	humidityTotal		=	0.0;
	for (ii=0; ii< kAvgSampleCount; ii++)
	{
		pressureTotal		+=	cPressureArray[ii];
		temperatureTotal	+=	cTemperatureArray[ii];
		humidityTotal		+=	cHumidityArray[ii];
	}
	cCurrentPressure_kPa				=	pressureTotal / kAvgSampleCount;
	cObsConditionProp.Pressure.Value	=	cCurrentPressure_kPa * 10;
	cObsConditionProp.Temperature.Value	=	temperatureTotal / kAvgSampleCount;
	cObsConditionProp.Humidity.Value	=	humidityTotal / kAvgSampleCount;


	//*	update the global copy

	gEnvData.siteTemperature_degC	=	cObsConditionProp.Temperature.Value;
	gEnvData.sitePressure_kPa		=	cCurrentPressure_kPa;
	gEnvData.siteHumidity			=	cObsConditionProp.Humidity.Value;
	gEnvData.siteDataValid			=	true;
	gettimeofday(&gEnvData.siteLastUpdate, NULL);


	gEnvData.domeTemperature_degC	=	cObsConditionProp.Temperature.Value;
	gEnvData.domePressure_kPa		=	cCurrentPressure_kPa;
	gEnvData.domeHumidity			=	cObsConditionProp.Humidity.Value;
	gEnvData.domeDataValid			=	true;
	gettimeofday(&gEnvData.domeLastUpdate, NULL);
}

//*****************************************************************************
double	ObsConditionsDriver::ReadPressure_kPa(void)
{
//	CONSOLE_DEBUG("Should not be here");
	//*	this routine should be overloaded by sub class
	return(0.0);
}

//*****************************************************************************
double	ObsConditionsDriver::ReadTemperature(void)
{
//	CONSOLE_DEBUG("Should not be here");
	//*	this routine should be overloaded by sub class
	return(0.0);
}

//*****************************************************************************
double	ObsConditionsDriver::ReadHumidity(void)
{
//	CONSOLE_DEBUG("Should not be here");
	//*	this routine should be overloaded by sub class
	return(0.0);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_AveragePeriod(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		//*	this MUST be supported so there is no check
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.Averageperiod.Value,
								INCLUDE_COMMA);

		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Units-period",
								"Hours",
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	"AveragePeriod=0.15&ClientID=22&ClientTransactionID=33"
//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Put_AveragePeriod(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool					avgPeriodFound;
char					avgPeriodString[32];
double					avgPeriodValue;
//	CONSOLE_DEBUG(__FUNCTION__);
	avgPeriodFound	=	GetKeyWordArgument(	reqData->contentData,
											"AveragePeriod",
											avgPeriodString,
											(sizeof(avgPeriodString) -1),
											kArgumentIsNumeric);
	if (avgPeriodFound)
	{
		avgPeriodValue	=	atof(avgPeriodString);
		if (avgPeriodValue >= 0.0)
		{
			cObsConditionProp.Averageperiod.Value	=	avgPeriodValue;
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Average period out of range");
		}
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "argument AveragePeriod not found");
	}
	CONSOLE_DEBUG(__FUNCTION__);


	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_CloudCover(	TYPE_GetPutRequestData	*reqData,
															char					*alpacaErrMsg,
															const char				*responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.CloudCover.IsSupported)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.CloudCover.Value,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_DewPoint(	TYPE_GetPutRequestData	*reqData,
														char					*alpacaErrMsg,
														const char				*responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				dewPointValue;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.Temperature.IsSupported && cObsConditionProp.Humidity.IsSupported)
	{
		dewPointValue	=	cObsConditionProp.Temperature.Value -
							((100.0 - cObsConditionProp.Humidity.Value) / 5);

		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								dewPointValue,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Humidity(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.Humidity.IsSupported)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.Humidity.Value,
								INCLUDE_COMMA);

	#ifdef _ENABLE_PI_HAT_SESNSOR_BOARD_
		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Comment-humidity",
								"This humidity sensor is on the Raspberry pi sense hat and does not represent outside humidity",
								INCLUDE_COMMA);
	#endif
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	return(alpacaErrCode);
}


//*****************************************************************************
//*	Returns the atmospheric pressure at the observatory.
//*	units = hectoPascals
//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Pressure(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.Pressure.IsSupported)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.Pressure.Value,
								INCLUDE_COMMA);

		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Units-pressure",
								"hectoPascals",
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_RainRate(	TYPE_GetPutRequestData	*reqData,
														char					*alpacaErrMsg,
														const char				*responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.RainRate.IsSupported)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.RainRate.Value,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_SkyBrightness(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.SkyBrightness.IsSupported)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.SkyBrightness.Value,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_SkyQuality(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.SkyQuality.IsSupported)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.SkyQuality.Value,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_SkyTemperature(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.SkyTemperature.IsSupported)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.SkyTemperature.Value,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_StarFWHM(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.StarFWHM.IsSupported)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.StarFWHM.Value,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Temperature(	TYPE_GetPutRequestData	*reqData,
															char					*alpacaErrMsg,
															const char				*responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.Temperature.IsSupported)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.Temperature.Value,
								INCLUDE_COMMA);

		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Units-temperature",
								"degrees C",
								INCLUDE_COMMA);

	#ifdef _ENABLE_PI_HAT_SESNSOR_BOARD_
		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Comment-temperature",
								"This temperature sensor is on the Raspberry pi sense hat and does not represent outside temperature",
								INCLUDE_COMMA);
	#endif
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Temperature sensor not available");
	}
	return(alpacaErrCode);
}



//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_WindDirection(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.WindDirection.IsSupported)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.WindDirection.Value,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_WindGust(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.WindGust.IsSupported)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.WindGust.Value,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_WindSpeed(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cObsConditionProp.WindSpeed.IsSupported)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cObsConditionProp.WindSpeed.Value,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Put_Refresh(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	kASCOM_Err_NotImplemented;
	return(alpacaErrCode);
}

//*****************************************************************************
typedef struct
{
	char					sensorName[16];
	TYPE_ObsConSensorType	senrsorEnum;

} TYPE_SENSOR_NAME;


//*****************************************************************************
const TYPE_SENSOR_NAME	gSensorNames[]	=
{

	{	"CloudCover",		kSensor_CloudCover		},
	{	"DewPoint",			kSensor_DewPoint		},
	{	"Humidity",			kSensor_Humidity		},
	{	"Pressure",			kSensor_Pressure		},
	{	"RainRate",			kSensor_RainRate		},
	{	"SkyBrightness",	kSensor_SkyBrightness	},
	{	"SkyQuality",		kSensor_SkyQuality		},
	{	"StarFWHM",			kSensor_StarFWHM		},
	{	"SkyTemperature",	kSensor_SkyTemperature	},
	{	"Temperature",		kSensor_Temperature		},
	{	"WindDirection",	kSensor_WindDirection	},
	{	"WindGust",			kSensor_WindGust		},
	{	"WindSpeed",		kSensor_WindSpeed		},
	{	"",					kSensor_Invalid			}
};


//*****************************************************************************
//*	returns the enum of the sensor name
TYPE_ObsConSensorType		ObsConditionsDriver::GetSensorEnum(const char *sensorName)
{
TYPE_ObsConSensorType	mySensorType;
int						iii;

	mySensorType	=	kSensor_Invalid;
	iii	=	0;
	while ((mySensorType == kSensor_Invalid) && (gSensorNames[iii].senrsorEnum > kSensor_Invalid))
	{
		if (strcasecmp(sensorName, gSensorNames[iii].sensorName) == 0)
		{
			mySensorType	=	gSensorNames[iii].senrsorEnum;
		}
		iii++;
	}

	if (mySensorType == kSensor_Invalid)
	{
		CONSOLE_DEBUG_W_STR("Unknown sensor type=", sensorName);
	}
	return(mySensorType);
}

//*****************************************************************************
//*	curl -X GET "https://virtserver.swaggerhub.com/ASCOMInitiative/api/v1/observingconditions/0/sensordescription?
//*		SensorName=Pressure&ClientID=1&ClientTransactionID=1234"
//*		-H "accept: application/json"
//*
//*****************************************************************************
//*	CloudCover
//*	DewPoint
//*	Humidity
//*	Pressure
//*	RainRate
//*	SkyBrightness
//*	SkyQuality
//*	StarFWHM
//*	SkyTemperature
//*	Temperature
//*	WindDirection
//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_SensorDescription(	TYPE_GetPutRequestData	*reqData,
																char					*alpacaErrMsg,
																const char				*responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
bool					sensorNameFound;
char					sensorNameString[64];
char					*spacePtr;
char					theDescription[128];
TYPE_ObsConSensorType	mySensorType;
double					lastUpdateTime;

//	CONSOLE_DEBUG(__FUNCTION__);
	sensorNameFound	=	GetKeyWordArgument(	reqData->contentData,
											"SensorName",
											sensorNameString,
											(sizeof(sensorNameString) -1));
	if (sensorNameFound)
	{
		strcpy(theDescription,	"AlpacaPi - not implemented");

		spacePtr	=	strchr(sensorNameString, 0x20);
		if (spacePtr != NULL)
		{
			*spacePtr	=	0;
		}
		CONSOLE_DEBUG_W_STR("sensorNameString\t=", sensorNameString);
		//*	translate sensor name to an enum
		mySensorType	=	GetSensorEnum(sensorNameString);

		alpacaErrCode	=	GetSensorInfo(mySensorType, theDescription, &lastUpdateTime);

			JsonResponse_Add_String(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									responseString,
									theDescription,
									INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "argument SensorName not found");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_TimeSinceLastUpdate(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;
uint32_t				deltaTime_secs;
time_t					currentTime_secs;
bool					sensorNameFound;
char					sensorNameString[64];
char					*spacePtr;
char					theDescription[128];
TYPE_ObsConSensorType	mySensorType;
double					lastUpdateTime;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(reqData->contentData);
	sensorNameFound	=	GetKeyWordArgument(	reqData->contentData,
											"SensorName",
											sensorNameString,
											(sizeof(sensorNameString) -1));
	if (sensorNameFound)
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		strcpy(theDescription,	"AlpacaPi - not implemented");

		spacePtr	=	strchr(sensorNameString, 0x20);
		if (spacePtr != NULL)
		{
			*spacePtr	=	0;
		}
		CONSOLE_DEBUG_W_STR("sensorNameString\t=", sensorNameString);
		//*	translate sensor name to an enum
		mySensorType	=	GetSensorEnum(sensorNameString);

		if (mySensorType > kSensor_Invalid)
		{
			alpacaErrCode	=	GetSensorInfo(mySensorType, theDescription, &lastUpdateTime);
		}
		else if (strlen(sensorNameString) == 0)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
		}

		currentTime_secs	=	time(NULL);
		deltaTime_secs		=	currentTime_secs - cTimeOfLastUpdate_secs;
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								(deltaTime_secs / 60.0),
								INCLUDE_COMMA);

		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Units-time",
								"Minutes",
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode			=	kASCOM_Err_InvalidValue;
		reqData->httpRetCode	=	400;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "argument SensorName not found");
	}
	return(alpacaErrCode);
}


//*****************************************************************************
//*	this MUST be implemented by the sub class
//*	change the ones that the sub class supports
//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::GetSensorInfo(	TYPE_ObsConSensorType sensorType,
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
			if (cObsConditionProp.Temperature.IsSupported && cObsConditionProp.Humidity.IsSupported)
			{
				alpacaErrCode	=	kASCOM_Err_Success;
				strcpy(description, "AlpacaPi: Calculated from temp/humidity");
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			}
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

//*****************************************************************************
//CloudCover
//DewPoint
//Humidity
//Pressure
//RainRate
//SkyBrightness
//SkyQuality
//SkyTemperature
//StarFWHM
//Temperature
//WindDirection
//WindGust
//WindSpeed
//*****************************************************************************
bool	ObsConditionsDriver::DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen)
{
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"CloudCover",		cObsConditionProp.CloudCover.Value);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"DewPoint",			cObsConditionProp.DewPoint.Value);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"Humidity",			cObsConditionProp.Humidity.Value);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"Pressure",			cObsConditionProp.Pressure.Value);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"RainRate",			cObsConditionProp.RainRate.Value);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"SkyBrightness",	cObsConditionProp.SkyBrightness.Value);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"SkyQuality",		cObsConditionProp.SkyQuality.Value);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"StarFWHM",			cObsConditionProp.StarFWHM.Value);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"Temperature",		cObsConditionProp.Temperature.Value);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"WindDirection",	cObsConditionProp.WindDirection.Value);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"WindGust",			cObsConditionProp.WindGust.Value);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"WindSpeed",		cObsConditionProp.WindSpeed.Value);

	return(true);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Readall(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		//*	do the common ones first
		Get_Readall_Common(	reqData, alpacaErrMsg);


		Get_AveragePeriod(		reqData, alpacaErrMsg, "averageperiod");
		Get_CloudCover(			reqData, alpacaErrMsg, "cloudcover");
		Get_DewPoint(			reqData, alpacaErrMsg, "dewpoint");
		Get_Humidity(			reqData, alpacaErrMsg, "humidity");
		Get_Pressure(			reqData, alpacaErrMsg, "pressure");
		Get_RainRate(			reqData, alpacaErrMsg, "rainrate");
		Get_SkyBrightness(		reqData, alpacaErrMsg, "skybrightness");
		Get_SkyQuality(			reqData, alpacaErrMsg, "skyquality");
		Get_SkyTemperature(		reqData, alpacaErrMsg, "skytemperature");
		Get_StarFWHM(			reqData, alpacaErrMsg, "starfwhm");
		Get_Temperature(		reqData, alpacaErrMsg, "temperature");
//		Get_TimeSinceLastUpdate(reqData, alpacaErrMsg, "timesincelastupdate");


		strcpy(alpacaErrMsg, "");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//#define	kPascals_to_lbf_in2_Constant		(1290320000.0 / 8896443230521.0)		//*	http://www.calualteme.com/Pressure

#define	kPascals_to_lbf_in2_Constant		(0.0001450377377969)
//*****************************************************************************
void	ObsConditionsDriver::OutputHTML(TYPE_GetPutRequestData *reqData)
{
int			mySocketFD;
char		lineBuffer[128];
double		degreesF;
double		pressure_PSI;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		mySocketFD	=	reqData->socket;
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<H2>Observing Conditions</H2>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		if (cSuccesfullReadCnt > 0)
		{
			SocketWriteData(mySocketFD,	"<CENTER>\r\n");

			SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

			//*-----------------------------------------------------------
			if (cObsConditionProp.Temperature.IsSupported)
			{
				SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"\t<TD>Temperature:</TD>");
				sprintf(lineBuffer,	"\t<TD>%1.2f&deg;C</TD>",	cObsConditionProp.Temperature.Value);
				SocketWriteData(mySocketFD,	lineBuffer);

				degreesF	=	(cObsConditionProp.Temperature.Value * 1.8) + 32.0;
				sprintf(lineBuffer,	"\t<TD>%1.2f&deg;F</TD>",	degreesF);
				SocketWriteData(mySocketFD,	lineBuffer);

				SocketWriteData(mySocketFD,	"</TR>\r\n");
			}

			//*-----------------------------------------------------------
			if (cObsConditionProp.Pressure.IsSupported)
			{
				SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"\t<TD>Pressure:</TD>");
				sprintf(lineBuffer,	"\t<TD>%1.2f kPa</TD>",	cCurrentPressure_kPa);
				SocketWriteData(mySocketFD,	lineBuffer);

				pressure_PSI	=	(cCurrentPressure_kPa * 1000)  * kPascals_to_lbf_in2_Constant;
				sprintf(lineBuffer,	"\t<TD>%1.2f PSI</TD>",	pressure_PSI);
				SocketWriteData(mySocketFD,	lineBuffer);
				SocketWriteData(mySocketFD,	"</TR>\r\n");
			}

			//*-----------------------------------------------------------
			if (cObsConditionProp.Humidity.IsSupported)
			{
				SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"\t<TD>Humidity:</TD>");
				sprintf(lineBuffer,	"\t<TD>%1.1f %%</TD>",	cObsConditionProp.Humidity.Value);
				SocketWriteData(mySocketFD,	lineBuffer);
				SocketWriteData(mySocketFD,	"</TR>\r\n");

				SocketWriteData(mySocketFD,	"</TABLE>\r\n");
				SocketWriteData(mySocketFD,	"<P>\r\n");
				SocketWriteData(mySocketFD,	"</CENTER>\r\n");
			}
		}
	}
}

//*****************************************************************************
bool	ObsConditionsDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gObsCondCmdTable, getPut);
	return(foundIt);
}



#endif	//	_ENABLE_OBSERVINGCONDITIONS_

