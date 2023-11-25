//**************************************************************************
//*	Name:			switchdriver_rpi.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Dec 26,	2019	<MLS> Created switchdriver_rpi.h
//*	Jan 11,	2021	<MLS> Got 4 relay board working
//*	Jan 11,	2021	<MLS> Added TURN_PIN_ON/TURN_PIN_OFF
//*	Jan 11,	2021	<MLS> Added kR_Pi_SwitchCount
//*****************************************************************************
//#include	"switchdriver_rpi.h"

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif


#ifndef _SWITCHDRIVER_H_
	#include	"switchdriver.h"
#endif

#ifndef _RPI_RELAYLIB_H_
	#include	"raspberrypi_relaylib.h"
#endif


void	CreateSwitchObjects_RPi(void);

//**************************************************************************************
class SwitchDriverRPi: public SwitchDriver
{
	public:

		//
		// Construction
		//
						SwitchDriverRPi(void);
		virtual			~SwitchDriverRPi(void);
		virtual	void	Init_Hardware(void);

	protected:

				int		TranslateSwitchToPin(const int switchNumber);

		virtual	bool	GetSwitchState(const int switchNumber);
		virtual	void	SetSwitchState(const int switchNumber, bool on_off);

		virtual	void	SetSwitchValue(const int switchNumber, double switchValue);



};



