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
//*****************************************************************************
//#include	"switchdriver_rpi.h"

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif


#ifndef _SWITCHDRIVER_H_
	#include	"switchdriver.h"
#endif

void	CreateSwitchObjectsRPi(void);

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


//*	using BCM pin numbers
#define	kHWpin_Channel1	5
#define	kHWpin_Channel2	6
#define	kHWpin_Channel3	13
#define	kHWpin_Channel4	16
#define	kHWpin_Channel5	19
#define	kHWpin_Channel6	20
#define	kHWpin_Channel7	21
#define	kHWpin_Channel8	26



