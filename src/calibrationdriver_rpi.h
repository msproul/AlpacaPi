//**************************************************************************************
//#include	"calibrationdriver_rpi.h"

void	CreateCalibrationObjects_RPi(void);

//**************************************************************************************
class CalibrationDriverRPI: public CalibrationDriver
{
	public:

		//
		// Construction
		//
						CalibrationDriverRPI(void);
		virtual			~CalibrationDriverRPI(void);
		virtual	void	Init_Hardware(void);

	protected:

		virtual	TYPE_ASCOM_STATUS	Calibrator_TurnOn(const int brightnessValue, char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Calibrator_TurnOff(char *alpacaErrMsg);

};
