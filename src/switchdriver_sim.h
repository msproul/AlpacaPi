//**************************************************************************
//*	Name:			switchdriver_sim.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar  2,	2023	<MLS> Created switchdriver_sim.h
//*****************************************************************************
//#include	"switchdriver_sim.h"

#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif


#ifndef _SWITCHDRIVER_H_
	#include	"switchdriver.h"
#endif



void	CreateSwitchObjects_SIM(void);

//**************************************************************************************
class SwitchDriverSIM: public SwitchDriver
{
	public:

		//
		// Construction
		//
						SwitchDriverSIM(void);
		virtual			~SwitchDriverSIM(void);
		virtual	void	Init_Hardware(void);

	protected:

		virtual	bool	GetSwitchState(const int switchNumber);
		virtual	void	SetSwitchState(const int switchNumber, bool on_off);

		virtual	void	SetSwitchValue(const int switchNumber, double switchValue);

		bool			cSwitchValues[kMaxSwitchCnt];
};




