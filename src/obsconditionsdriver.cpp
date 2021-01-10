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
//*	Redistributions of this source code must retain this copyright notice.
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
//*	Jun 27,	2020	<MLS> Fixed bug in Get_Timesincelastupdate()
//*	Jun 27,	2020	<MLS> Updated routine names to be more consistent
//*	Sep  8,	2020	<MLS> Updated routines to return TYPE_ASCOM_STATUS
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
	#define	_ENABLE_PI_HAT_SESNSOR_BOARD_	//*	moved to Makefile
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

//*****************************************************************************
void	CreateObsConditionObjects(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	new ObsConditionsDriver(0);
}


//*****************************************************************************
//*	Observingconditions
enum
{

	kCmd_ObservCond_averageperiod	=	0,	//*	Returns the time period over which observations will be averaged
											//*	Sets the time period over which observations will be averaged
	kCmd_ObservCond_cloudcover,				//*	Returns the amount of sky obscured by cloud
	kCmd_ObservCond_dewpoint,				//*	Returns the atmospheric dew point at the observatory
	kCmd_ObservCond_humidity,				//*	Returns the atmospheric humidity at the observatory
	kCmd_ObservCond_pressure,				//*	Returns the atmospheric pressure at the observatory.
	kCmd_ObservCond_rainrate,				//*	Returns the rain rate at the observatory.
	kCmd_ObservCond_skybrightness,			//*	Returns the sky brightness at the observatory
	kCmd_ObservCond_skyquality,				//*	Returns the sky quality at the observatory
	kCmd_ObservCond_skytemperature,			//*	Returns the sky temperature at the observatory
	kCmd_ObservCond_starfwhm,				//*	Returns the seeing at the observatory
	kCmd_ObservCond_temperature,			//*	Returns the temperature at the observatory
	kCmd_ObservCond_winddirection,			//*	Returns the wind direction at the observatory
	kCmd_ObservCond_windgust,				//*	Returns the peak 3 second wind gust at the observatory over the last 2 minutes
	kCmd_ObservCond_windspeed,				//*	Returns the wind speed at the observatory.
	kCmd_ObservCond_refresh,				//*	Refreshes sensor values from hardware.
	kCmd_ObservCond_sensordescription,		//*	Return a sensor description
	kCmd_ObservCond_timesincelastupdate,	//*	Return the time since the sensor value was last updated

	//=================================================================
	//*	commands added that are not part of Alpaca
	//*	added by MLS
	kCmd_ObservCond_Extras,
	kCmd_ObservCond_readall
};


//*****************************************************************************
static TYPE_CmdEntry	gObsCondCmdTable[]	=
{
	{	"averageperiod",		kCmd_ObservCond_averageperiod,			kCmdType_BOTH	},
	{	"cloudcover",			kCmd_ObservCond_cloudcover,				kCmdType_GET	},
	{	"dewpoint",				kCmd_ObservCond_dewpoint,				kCmdType_GET	},
	{	"humidity",				kCmd_ObservCond_humidity,				kCmdType_GET	},
	{	"pressure",				kCmd_ObservCond_pressure,				kCmdType_GET	},
	{	"rainrate",				kCmd_ObservCond_rainrate,				kCmdType_GET	},
	{	"skybrightness",		kCmd_ObservCond_skybrightness,			kCmdType_GET	},
	{	"skyquality",			kCmd_ObservCond_skyquality,				kCmdType_GET	},
	{	"skytemperature",		kCmd_ObservCond_skytemperature,			kCmdType_GET	},
	{	"starfwhm",				kCmd_ObservCond_starfwhm,				kCmdType_GET	},
	{	"temperature",			kCmd_ObservCond_temperature,			kCmdType_GET	},
	{	"winddirection",		kCmd_ObservCond_winddirection,			kCmdType_GET	},
	{	"windgust",				kCmd_ObservCond_windgust,				kCmdType_GET	},
	{	"windspeed",			kCmd_ObservCond_windspeed,				kCmdType_GET	},
	{	"refresh",				kCmd_ObservCond_refresh,				kCmdType_PUT	},
	{	"sensordescription",	kCmd_ObservCond_sensordescription,		kCmdType_GET	},
	{	"timesincelastupdate",	kCmd_ObservCond_timesincelastupdate,	kCmdType_GET	},

	//*	added by MLS
	{	"--extras",				kCmd_ObservCond_Extras,					kCmdType_GET	},
	{	"readall",				kCmd_ObservCond_readall,				kCmdType_GET	},


	{	"",						-1,	0x00	}
};


//**************************************************************************************
ObsConditionsDriver::ObsConditionsDriver(const int argDevNum)
	:AlpacaDriver(kDeviceType_Observingconditions)
{
int	ii;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cDeviceName, "ObsConditionsDriver");
	strcpy(cObsCondDescription, "outdoor conditions");
	cCurrentPressure_kPa	=	0;		//*	kPa
	cCurrentPressure_hPa	=	0;		//*	hPa
	cCurrentTemp_DegC		=	0;		//*	degrees Centigrade
	cCurrentHumidity		=	0;		//*	Percent (0 -> 100)
	cSuccesfullReadCnt		=	0;		//*	used to know if we have active sensors
	cTimeOfLastUpdate_secs	=	0;		//*	time in seconds
	cObservCondState		=	kObservCondState_Startup;

	//*	the sub classes have to set this appropriately
	cHasTempSensor			=	false;
	cHasPresSensor			=	false;
	cHasHumidSensor			=	false;

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
	cCurrentPressure_kPa	=	pressureTotal / kAvgSampleCount;
	cCurrentPressure_hPa	=	cCurrentPressure_kPa * 10;
	cCurrentTemp_DegC		=	temperatureTotal / kAvgSampleCount;
	cCurrentHumidity		=	humidityTotal / kAvgSampleCount;

	//*	update the global copy

	gEnvData.siteTemperature_degC	=	cCurrentTemp_DegC;
	gEnvData.sitePressure_kPa		=	cCurrentPressure_kPa;
	gEnvData.siteHumidity			=	cCurrentHumidity;
	gEnvData.siteDataValid			=	true;
	gettimeofday(&gEnvData.siteLastUpdate, NULL);


	gEnvData.domeTemperature_degC	=	cCurrentTemp_DegC;
	gEnvData.domePressure_kPa		=	cCurrentPressure_kPa;
	gEnvData.domeHumidity			=	cCurrentHumidity;
	gEnvData.domeDataValid			=	true;
	gettimeofday(&gEnvData.domeLastUpdate, NULL);
}

//*****************************************************************************
double	ObsConditionsDriver::ReadPressure_kPa(void)
{
	CONSOLE_DEBUG("Should not be here");
	//*	this routine should be overloaded by sub class
	return(0.0);
}

//*****************************************************************************
double	ObsConditionsDriver::ReadTemperature(void)
{
	CONSOLE_DEBUG("Should not be here");
	//*	this routine should be overloaded by sub class
	return(0.0);
}

//*****************************************************************************
double	ObsConditionsDriver::ReadHumidity(void)
{
	CONSOLE_DEBUG("Should not be here");
	//*	this routine should be overloaded by sub class
	return(0.0);
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
	JsonResponse_CreateHeader(reqData->jsonTextBuffer, kMaxJsonBuffLen);

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
				alpacaErrCode	=	Get_Averageperiod(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
//+				alpacaErrCode	=	Put_averageperiod(reqData, alpacaErrMsg);
				alpacaErrCode	=	kASCOM_Err_NotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
			}
			break;

		case kCmd_ObservCond_cloudcover:	//*	Returns the amount of sky obscured by cloud
			alpacaErrCode	=	Get_Cloudcover(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_dewpoint:		//*	Returns the atmospheric dew point at the observatory
			alpacaErrCode	=	Get_Dewpoint(reqData, alpacaErrMsg, gValueString);
			break;


		case kCmd_ObservCond_humidity:		//*	Returns the atmospheric humidity at the observatory
			alpacaErrCode	=	Get_Humidity(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_pressure:		//*	Returns the atmospheric pressure at the observatory.
											//*	units = hectoPascals
			alpacaErrCode	=	Get_Pressure(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_rainrate:				//*	Returns the rain rate at the observatory.
			alpacaErrCode	=	Get_Rainrate(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_skybrightness:			//*	Returns the sky brightness at the observatory
			alpacaErrCode	=	Get_Skybrightness(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_skyquality:				//*	Returns the sky quality at the observatory
			alpacaErrCode	=	Get_Skyquality(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_skytemperature:			//*	Returns the sky temperature at the observatory
			alpacaErrCode	=	Get_Skytemperature(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_starfwhm:				//*	Returns the seeing at the observatory
			alpacaErrCode	=	Get_Starfwhm(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_temperature:			//*	Returns the temperature at the observatory
			alpacaErrCode	=	Get_Temperature(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_ObservCond_winddirection:			//*	Returns the wind direction at the observatory
		case kCmd_ObservCond_windgust:				//*	Returns the peak 3 second wind gust at the observatory over the last 2 minutes
		case kCmd_ObservCond_windspeed:				//*	Returns the wind speed at the observatory.
		case kCmd_ObservCond_refresh:				//*	Refreshes sensor values from hardware.
		case kCmd_ObservCond_sensordescription:		//*	Return a sensor description
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Command not implemented");
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			break;

		case kCmd_ObservCond_timesincelastupdate:	//*	Return the time since the sensor value was last updated
			alpacaErrCode	=	Get_Timesincelastupdate(reqData, alpacaErrMsg, gValueString);
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

	//*	send the response information
	JsonResponse_Add_Int32(		mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ClientTransactionID",
								gClientTransactionID,
								INCLUDE_COMMA);

	JsonResponse_Add_Int32(		mySocket,
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
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								kInclude_HTTP_Header);

	//*	this is for the logging function
	strcpy(reqData->alpacaErrMsg, alpacaErrMsg);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Averageperiod(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				averagePeriod;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		averagePeriod	=	(kAvgSampleCount * kSampleDetlaSecs * 1.0) / 60.0;
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								averagePeriod,
								INCLUDE_COMMA);

		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Units",
								"Minutes",
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Cloudcover(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Dewpoint(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Humidity(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCurrentHumidity,
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
		alpacaErrCode	=	kASCOM_Err_InternalError;
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
	if (reqData != NULL)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCurrentPressure_hPa,
								INCLUDE_COMMA);

		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Units",
								"hectoPascals",
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Rainrate(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Skybrightness(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Skyquality(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Skytemperature(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Starfwhm(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not Implemented");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Temperature(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		if (cHasTempSensor)
		{
			JsonResponse_Add_Double(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									responseString,
									cCurrentTemp_DegC,
									INCLUDE_COMMA);

			JsonResponse_Add_String(reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"Units",
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
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	ObsConditionsDriver::Get_Timesincelastupdate(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
uint32_t			deltaTime_secs;
time_t				currentTime_secs;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
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
								"Units",
								"Minutes",
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
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


		Get_Averageperiod(		reqData, alpacaErrMsg, "averageperiod");
		Get_Cloudcover(			reqData, alpacaErrMsg, "cloudcover");
		Get_Dewpoint(			reqData, alpacaErrMsg, "dewpoint");
		Get_Humidity(			reqData, alpacaErrMsg, "humidity");
		Get_Pressure(			reqData, alpacaErrMsg, "pressure");
		Get_Rainrate(			reqData, alpacaErrMsg, "rainrate");
		Get_Skybrightness(		reqData, alpacaErrMsg, "skybrightness");
		Get_Skyquality(			reqData, alpacaErrMsg, "skyquality");
		Get_Skytemperature(		reqData, alpacaErrMsg, "skytemperature");
		Get_Starfwhm(			reqData, alpacaErrMsg, "starfwhm");
		Get_Temperature(		reqData, alpacaErrMsg, "temperature");
		Get_Timesincelastupdate(reqData, alpacaErrMsg, "timesincelastupdate");


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
			if (cHasTempSensor)
			{
				SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"\t<TD>Temperature:</TD>");
				sprintf(lineBuffer,	"\t<TD>%1.2f&deg;C</TD>",	cCurrentTemp_DegC);
				SocketWriteData(mySocketFD,	lineBuffer);

				degreesF	=	(cCurrentTemp_DegC * 1.8) + 32.0;
				sprintf(lineBuffer,	"\t<TD>%1.2f&deg;F</TD>",	degreesF);
				SocketWriteData(mySocketFD,	lineBuffer);

				SocketWriteData(mySocketFD,	"</TR>\r\n");
			}

			//*-----------------------------------------------------------
			if (cHasPresSensor)
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
			if (cHasHumidSensor)
			{
				SocketWriteData(mySocketFD,	"<TR>\r\n");
				SocketWriteData(mySocketFD,	"\t<TD>Humidity:</TD>");
				sprintf(lineBuffer,	"\t<TD>%1.1f %%</TD>",	cCurrentHumidity);
				SocketWriteData(mySocketFD,	lineBuffer);
				SocketWriteData(mySocketFD,	"</TR>\r\n");

				SocketWriteData(mySocketFD,	"</TABLE>\r\n");
				SocketWriteData(mySocketFD,	"<P>\r\n");
				SocketWriteData(mySocketFD,	"</CENTER>\r\n");
			}
		}
		//*	now generate links to all of the commands
		GenerateHTMLcmdLinkTable(mySocketFD, "observingconditions", 0, gObsCondCmdTable);
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

