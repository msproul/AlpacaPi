//**************************************************************************
//*	Name:			shutterdriver.h
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*		https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*		https://github.com/OpenPHDGuiding/phd2/tree/master/cameras/zwolibs
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Feb 10,	2020	<MLS> Started on Shutter control
//*	Nov 28,	2020	<MLS> Updated return values to TYPE_ASCOM_STATUS
//*****************************************************************************
//#include	"shutterdriver.h"

#ifndef _SHUTTER_DRIVER_H_
#define	_SHUTTER_DRIVER_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif

void	CreateShutterObjects(void);


//**************************************************************************************
class ShutterDriver: public AlpacaDriver
{
	public:

		//
		// Construction
		//
									ShutterDriver(const int argDevNum);
		virtual						~ShutterDriver(void);
		virtual	void				Init_Hardware(void);

		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	int32_t				RunStateMachine(void);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);

	protected:

				TYPE_ASCOM_STATUS	Get_Altitude(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Cansetaltitude(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Cansetshutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Get_Shutterstatus(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Put_OpenShutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Put_CloseShutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_Slewing(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
				TYPE_ASCOM_STATUS	Put_AbortSlew(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

				//*	commands added by MLS
				TYPE_ASCOM_STATUS	Put_StopShutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Get_Readall(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		virtual	void	CheckSensors(void);

		virtual	bool	OpenShutter(void);
		virtual	bool	CloseShutter(void);
		virtual	bool	StopShutter(void);
		virtual	void	WatchDog_TimeOut(void);

				int32_t		cShutterStatus;
				int32_t		cPreviousShutterStatus;
				double		cAltitude_Dbl;
				int32_t		cIdleStateCnt;

};


#endif	//	_SHUTTER_DRIVER_H_
