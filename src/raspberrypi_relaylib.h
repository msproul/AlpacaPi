//*****************************************************************************
//#include	"raspberrypi_relaylib.h"

#ifndef _RPI_RELAYLIB_H_
#define _RPI_RELAYLIB_H_

#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif

#if defined(__ARM_ARCH) && !defined(__arm__)
	#define __arm__
#endif


#ifdef _ENABLE_4REALY_BOARD
	//==============================================================
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
	//	GPIO 4, GPIO 17, GPIO 27 and GPIO 22 are used for controlling Relay.
	//	But the user can remove Relay Jumper and select custom GPIO pin.
	//	Relay pins - COM, NO (Normally Open), and NC (Normally Closed) are available for each relay.
	//	Comes with development resources, including examples in python.


	#define	kR_Pi_RelayCount	4


	#define	kHWpin_Channel1	22
	#define	kHWpin_Channel2	27
	#define	kHWpin_Channel3	17
	#define	kHWpin_Channel4	4

	#define	kHWpin_Channel5	99
	#define	kHWpin_Channel6	99
	#define	kHWpin_Channel7	99
	#define	kHWpin_Channel8	99


	//*	some boards are reverse logic so we have to do it this way
	#define		TURN_PIN_ON		1
	#define		TURN_PIN_OFF	0

#else	// _ENABLE_4REALY_BOARD

	//	8 port relay on DIN rail 	5,	6,	13,	16,	19,	20,	21,	26
	//==============================================================
	#define	kR_Pi_RelayCount	8

	//*	using BCM pin numbers
	#define	kHWpin_Channel1	5
	#define	kHWpin_Channel2	6
	#define	kHWpin_Channel3	13
	#define	kHWpin_Channel4	16
	#define	kHWpin_Channel5	19
	#define	kHWpin_Channel6	20
	#define	kHWpin_Channel7	21
	#define	kHWpin_Channel8	26


	//*	some boards are reverse logic so we have to do it this way
	#define		TURN_PIN_ON		0
	#define		TURN_PIN_OFF	1


#endif


#ifdef __cplusplus
	extern "C" {
#endif



int		RpiRelay_Init(void);	//*	returns the number of configured relays
bool	RpiRelay_SetRelay(const int relayNum, bool newState);	//*	returns true if OK
bool	RpiRelay_GetRelay(const int relayNumber);

#ifdef __cplusplus
}
#endif



#endif // _RPI_RELAYLIB_H_
