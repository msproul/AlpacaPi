//**************************************************************************
//*	Name:			calibrationdriver.h
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Sep  1,	2020	<MLS> Created calibrationdriver.h
//*****************************************************************************
//#include	"calibrationdriver.h"

#ifndef _CALIBRATION_DRIVER_H_
#define	_CALIBRATION_DRIVER_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif

void	CreateCalibrationObjects(void);

//**************************************************************************************
class CalibrationDriver: public AlpacaDriver
{
	public:

		//
		// Construction
		//
									CalibrationDriver(void);
		virtual						~CalibrationDriver(void);
		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
//		virtual	void				OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
		virtual	int32_t				RunStateMachine(void);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);

	protected:

		TYPE_ASCOM_STATUS	Get_Brightness(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Calibratorstate(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Coverstate(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Maxbrightness(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Put_CalibratorOff(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_CalibratorOn(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_CloseCover(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_HaltCover(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_OpenCover(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_CalibratorReady(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CoverMoving(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		//*	extras defined by MLS
		TYPE_ASCOM_STATUS	Get_Aperture(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Aperture(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_CanAdjustAperture(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

virtual	TYPE_ASCOM_STATUS	Get_Readall(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

virtual	bool	DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen);

		//-------------------------------------------------------------------------
		//*	Watchdog timer stuff
		virtual	void				WatchDog_TimeOut(void);

		//*	these routines should be implemented by the sub-classes
		//*	all have to return an Alpaca Error code
		virtual	TYPE_ASCOM_STATUS	Calibrator_TurnOn(const int brightnessValue, char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Calibrator_TurnOff(char *alpacaErrMsg);

		virtual	TYPE_ASCOM_STATUS	Cover_Open(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Cover_Close(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Cover_Halt(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Cover_GetStatus(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	GetBrightness(char *alpacaErrMsg);

				TYPE_CoverCalibrationProperties	cCoverCalibrationProp;
				CoverStatus						cCoverDesiredPostion;
				uint32_t						cCoverMovementStartTime;	//*	milliseconds
				uint32_t						cCoverLastUpdateTime;		//*	milliseconds

};


#endif // _CALIBRATION_DRIVER_H_
