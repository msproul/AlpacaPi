//**************************************************************************
//*	Name:			domedriver_sim.h
//*
//*	Author:			Mark Sproul (C) 2023
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
//*	Mar  2,	2023	<MLS> Created domedriver_sim.h
//*****************************************************************************
//#include	"domedriver_sim.h"

#ifndef _DOMEDRIVER_SIM_H_
#define _DOMEDRIVER_SIM_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif

#ifndef _DOME_DRIVER_H_
	#include	"domedriver.h"
#endif

//**************************************************************************************
class DomeDriverSIM: public DomeDriver
{
	public:

		//
		// Construction
		//
						DomeDriverSIM(const int argDevNum);
		virtual			~DomeDriverSIM(void);
//		virtual	void	OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
		virtual	void	Init_Hardware(void);

	protected:


//		virtual	bool	BumpDomeSpeed(const int howMuch);
//		virtual	void	CheckDomeButtons(void);
//		virtual	void	CheckSensors(void);
//		virtual	void	UpdateDomePosition(void);
//		virtual	void 	ProcessButtonPressed(const int pressedButton);
		virtual	void	StartDomeMoving(const int direction);
//		virtual	void	StopDomeMoving(bool rightNow);

//		virtual	TYPE_ASCOM_STATUS 	OpenShutter(void);
//		virtual	TYPE_ASCOM_STATUS 	CloseShutter(void);

};


void	CreateDomeObjectsSIM(void);

#endif // _DOMEDRIVER_SIM_H_
