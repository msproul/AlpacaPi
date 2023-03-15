//**************************************************************************************
//#include	"calibration_Alnitak.h"


void	CreateCalibrationObjects_Alnitak(void);

//**************************************************************************************
class CalibrationDriverAlnitak: public CalibrationDriver
{
	public:

		//
		// Construction
		//
						CalibrationDriverAlnitak(void);
		virtual			~CalibrationDriverAlnitak(void);
		virtual	void	Init_Hardware(void);

	protected:

		virtual	TYPE_ASCOM_STATUS	Calibrator_TurnOn(const int brightnessValue, char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Calibrator_TurnOff(char *alpacaErrMsg);

				char				cUSBportPath[32];
				int					cFileDesc;				//*	port file descriptor

};

