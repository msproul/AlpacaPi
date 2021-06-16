//**************************************************************************
//*	Name:			obsconditionsdriver.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*****************************************************************************
//#include	"obsconditionsdriver.h"


#ifndef _OBSERVINGCONDITIONSDRIVER_H_
#define	_OBSERVINGCONDITIONSDRIVER_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif

void	CreateObsConditionObjects(void);

//*****************************************************************************
//*	ObservCond states
typedef enum ObservCond_STATE_TYPE
{
	kObservCondState_Startup	=	0,
	kObservCondState_Idle,

	kObservCondState_last

} ObservCond_STATE_TYPE;



//**************************************************************************************
enum TYPE_ObsConSensorType
{
	kSensor_Invalid		=	-1,
	kSensor_CloudCover	=	0,
	kSensor_DewPoint,
	kSensor_Humidity,
	kSensor_Pressure,
	kSensor_RainRate,
	kSensor_SkyBrightness,
	kSensor_SkyQuality,
	kSensor_StarFWHM,
	kSensor_SkyTemperature,
	kSensor_Temperature,
	kSensor_WindDirection,
	kSensor_WindGust,
	kSensor_WindSpeed
};


#define	kAvgSampleCount		20
#define	kSampleDetlaSecs	10
//**************************************************************************************
class ObsConditionsDriver: public AlpacaDriver
{
	public:

		//
		// Construction
		//
									ObsConditionsDriver(const int argDevNum);
		virtual						~ObsConditionsDriver(void);
		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
//		virtual	void				OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
		virtual	int32_t				RunStateMachine(void);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);

	protected:
		TYPE_ASCOM_STATUS	Get_AveragePeriod(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_AveragePeriod(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Cloudcover(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Dewpoint(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Humidity(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Pressure(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Rainrate(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Skybrightness(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Skyquality(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Skytemperature(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Starfwhm(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Temperature(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_TimeSinceLastUpdate(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);


		TYPE_ASCOM_STATUS	Get_Winddirection(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Windgust(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Windspeed(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Refresh(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_SensorDescription(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);



		TYPE_ASCOM_STATUS	Get_Readall(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
virtual	TYPE_ASCOM_STATUS	GetSensorInfo(			TYPE_ObsConSensorType sensorType,
													char	*description,
													double	*timeSinceLastUpdate);

		TYPE_ObsConSensorType		GetSensorEnum(const char *sensorName);

				void	UpdateSensorsReadings(void);
		virtual	double	ReadPressure_kPa(void);
		virtual	double	ReadTemperature(void);
		virtual	double	ReadHumidity(void);


		ObservCond_STATE_TYPE		cObservCondState;
		TYPE_ObsConditionProperties	cObsConditionProp;

		bool		cHasTempSensor;
		bool		cHasPresSensor;
		bool		cHasHumidSensor;


		//*	these arrays are for averaging
		double		cPressureArray[kAvgSampleCount];
		double		cTemperatureArray[kAvgSampleCount];
		double		cHumidityArray[kAvgSampleCount];


		double		cCurrentPressure_kPa;		//*	kilo Pascals
		int			cSuccesfullReadCnt;			//*	used to know if we have active sensors
		time_t		cTimeOfLastUpdate_secs;		//*	time in seconds

};

#endif	//	_OBSERVINGCONDITIONSDRIVER_H_

