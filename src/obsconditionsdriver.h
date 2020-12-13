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
		TYPE_ASCOM_STATUS	Get_Averageperiod(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
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
		TYPE_ASCOM_STATUS	Get_Timesincelastupdate(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_Readall(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

				void	UpdateSensorsReadings(void);
		virtual	double	ReadPressure_kPa(void);
		virtual	double	ReadTemperature(void);
		virtual	double	ReadHumidity(void);

		ObservCond_STATE_TYPE	cObservCondState;

		char		cObsCondDescription[64];
		bool		cHasTempSensor;
		bool		cHasPresSensor;
		bool		cHasHumidSensor;


		//*	these arrays are for averaging
		double		cPressureArray[kAvgSampleCount];
		double		cTemperatureArray[kAvgSampleCount];
		double		cHumidityArray[kAvgSampleCount];

		double		cCurrentPressure_kPa;		//*	kilo Pascals
		double		cCurrentPressure_hPa;		//*	hectoPascals
		double		cCurrentTemp_DegC;			//*	degrees Centigrade
		double		cCurrentHumidity;			//*	Percent (0 -> 100)
		int			cSuccesfullReadCnt;			//*	used to know if we have active sensors
		time_t		cTimeOfLastUpdate_secs;		//*	time in seconds

};

#endif	//	_OBSERVINGCONDITIONSDRIVER_H_

