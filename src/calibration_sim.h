//**************************************************************************************
//#include	"calibration_sim.h"
//**************************************************************************************

#ifndef	_CALIBRATION_DRIVER_H_
	#include	"calibrationdriver.h"
#endif


void	CreateCalibrationObjects_SIM(void);

//**************************************************************************************
class CalibrationDriverSIM: public CalibrationDriver
{
	public:

		//
		// Construction
		//
						CalibrationDriverSIM(void);
		virtual			~CalibrationDriverSIM(void);
		virtual	void	Init_Hardware(void);
		virtual	int32_t	RunStateMachine(void);

	protected:

		virtual	TYPE_ASCOM_STATUS	Calibrator_TurnOn(const int brightnessValue, char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Calibrator_TurnOff(char *alpacaErrMsg);

		virtual	TYPE_ASCOM_STATUS	Cover_Open(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Cover_Close(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Cover_Halt(char *alpacaErrMsg);

};

