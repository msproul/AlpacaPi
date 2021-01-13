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

#ifdef _ENABLE_4REALY_BOARD
//	https://smile.amazon.com/gp/product/B077LV4F1B/ref=ppx_yo_dt_b_asin_title_o01_s00?ie=UTF8&psc=1
//	https://shop.sb-components.co.uk/products/pirelay-relay-board-shield-for-raspberry-pi

//	https://github.com/sbcshop/PiRelay-V2

//	40-Pin Stacking Header for accessing GPIO of Raspberry Pi.
//	Connectivity of the Relay with Raspberry Pi without GPIO
//	Pin 2-> 5V
//	Pin 6-> GND
//	Pin 7-> Relay 4
//	Pin 11-> Relay 3
//	Pin 13-> Relay 2
//	Pin 15-> Relay 1
//	Connectivity using the stack method:
//	GPIO 4, GPIO 17, GPIO 27 and GPIO 22 are used for controlling Relay. But the user can remove Relay Jumper and select custom GPIO pin.
//	Relay pins - COM, NO (Normally Open), and NC (Normally Closed) are available for each relay.
//	Comes with development resources, including examples in python.
	#define	kHWpin_Channel1	22
	#define	kHWpin_Channel2	27
	#define	kHWpin_Channel3	17
	#define	kHWpin_Channel4	4

	#define	kHWpin_Channel5	99
	#define	kHWpin_Channel6	99
	#define	kHWpin_Channel7	99
	#define	kHWpin_Channel8	99

#define	kR_Pi_SwitchCount	4

	//*	some boards are reverse logic so we have to do it this way
	#define		TURN_PIN_ON		1
	#define		TURN_PIN_OFF	0

#else

	//*	using BCM pin numbers
	#define	kHWpin_Channel1	5
	#define	kHWpin_Channel2	6
	#define	kHWpin_Channel3	13
	#define	kHWpin_Channel4	16
	#define	kHWpin_Channel5	19
	#define	kHWpin_Channel6	20
	#define	kHWpin_Channel7	21
	#define	kHWpin_Channel8	26

#define	kR_Pi_SwitchCount	8

	//*	some boards are reverse logic so we have to do it this way
	#define		TURN_PIN_ON		0
	#define		TURN_PIN_OFF	1


#endif // _ENABLE_4REALY_BOARD


