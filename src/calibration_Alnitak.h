//**************************************************************************************
//#include	"calibration_Alnitak.h"


#ifndef _PTHREAD_H
	#include	<pthread.h>
#endif
#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif

#ifndef _CALIBRATION_DRIVER_H_
	#include	"calibrationdriver.h"
#endif


void	CreateCalibrationObjects_Alnitak(void);

//**************************************************************************************
class CalibrationDriverAlnitak: public CalibrationDriver
{
	public:

		//
		// Construction
		//
						CalibrationDriverAlnitak(const char *argUSBpath=NULL);
		virtual			~CalibrationDriverAlnitak(void);
		virtual	bool	Init_Hardware(void);

	protected:

		virtual	TYPE_ASCOM_STATUS	Calibrator_TurnOn(const int brightnessValue, char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Calibrator_TurnOff(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Cover_Open(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Cover_Close(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Cover_Halt(char *alpacaErrMsg);
//		virtual	TYPE_ASCOM_STATUS	Cover_GetStatus(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	GetBrightness(char *alpacaErrMsg);


				char				cUSBportPath[32];
				int					cFileDesc;				//*	port file descriptor
				bool				cCalibrationIsOn;
	protected:
		virtual	void				RunThread_Startup(void);
		virtual	void				RunThread_Loop(void);
	private:

				//*	these are used to tell the thread what to do
				bool				cOpenCover;
				bool				cCloseCover;
				bool				cTurnOn;
				bool				cTurnOff;
				int					cNewBrightnessValue;
				uint32_t			cLastUpdate_ms;
				bool				cForceUpdate;

				//*	these are the routines that actually talk to the hardware
				int					SendFlipFlatCmd(const char cmdChr, int commandArgValue);
				int					SendCmd_GetResponse(const char cmdChr, int commandArgValue, char *returnedString);
				TYPE_ASCOM_STATUS	Alnitak_Cover_Open(char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Alnitak_Cover_Close(char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Alnitak_Cover_Halt(char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Alnitak_Cover_GetStatus(char *alpacaErrMsg);

				TYPE_ASCOM_STATUS	Alnitak_TurnOn(const int brightnessValue, char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Alnitak_TurnOff(char *alpacaErrMsg);
				TYPE_ASCOM_STATUS	Alnitak_GetBrightness(char *alpacaErrMsg);

};

