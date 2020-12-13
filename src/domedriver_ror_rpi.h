//**************************************************************************
//*	Name:			domedriver_ror.h
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
//*	Dec  2,	2020	<MLS> Created domedriver_ror.h
//*****************************************************************************
//#include	"domedriver_rpi.h"

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif

#ifndef _DOME_DRIVER_H_
	#include	"domedriver.h"
#endif

//**************************************************************************************
class DomeDriverROR: public DomeDriver
{
	public:

		//
		// Construction
		//
						DomeDriverROR(const int argDevNum);
		virtual			~DomeDriverROR(void);
		virtual	void	Init_Hardware(void);

	protected:


//		virtual	bool	BumpDomeSpeed(const int howMuch);
//		virtual	void	CheckDomeButtons(void);
//		virtual	void	CheckSensors(void);
//		virtual	void 	ProcessButtonPressed(const int pressedButton);
//		virtual	void	StartDomeMoving(const int direction);
//		virtual	void	StopDomeMoving(bool rightNow);

		virtual	TYPE_ASCOM_STATUS 	OpenShutter(void);
		virtual	TYPE_ASCOM_STATUS 	CloseShutter(void);

};


void	CreateDomeObjectsROR(void);

