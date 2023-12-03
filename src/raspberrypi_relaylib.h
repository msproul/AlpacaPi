//*****************************************************************************
//#include	"raspberrypi_relaylib.h"
//*****************************************************************************
//*	Dec  2,	2023	<MLS> Added support for WaveShare RPI RELAY BOARD (3 relays)
//*****************************************************************************

#ifndef _RPI_RELAYLIB_H_
#define _RPI_RELAYLIB_H_

#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif

#if defined(__ARM_ARCH) && !defined(__arm__)
	#define __arm__
#endif

#ifdef _ENABLE_WAVESHARE_3RELAY_
	//==============================================================
	//*	https://www.waveshare.com/rpi-relay-board.htm
	//*	https://www.amazon.com/dp/B0CDH1L58X?psc=1&ref=ppx_yo2ov_dt_b_product_details
	//*	https://www.waveshare.com/wiki/RPi_Relay_Board

	#define	kR_Pi_RelayCount	3

	//*	using BCM pin numbers			//*	these are the pins from the documentation
	#define	kHWpin_Channel1		26		//*	wPi	=	25
	#define	kHWpin_Channel2		20		//*	wPi	=	28
	#define	kHWpin_Channel3		21		//*	wPi	=	29

	#define	kHWpin_Channel4		99
	#define	kHWpin_Channel5		99
	#define	kHWpin_Channel6		99
	#define	kHWpin_Channel7		99
	#define	kHWpin_Channel8		99


	#define		TURN_PIN_ON		0
	#define		TURN_PIN_OFF	1

#elif defined(_ENABLE_4REALY_BOARD_)
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

#elif defined(_ENABLE_4REALY_KEYSTUDIO_)
//*	https://wiki.keyestudio.com/KS0212_keyestudio_RPI_4-channel_Relay_Shield#Specification	#define	kR_Pi_RelayCount	4

	#define	kHWpin_Channel1	4		//*	wPi	=	7
	#define	kHWpin_Channel2	22		//*	wPi	=	3
	#define	kHWpin_Channel3	6		//*	wPi	=	22
	#define	kHWpin_Channel4	26		//*	wPi	=	25

	#define	kHWpin_Channel5	99
	#define	kHWpin_Channel6	99
	#define	kHWpin_Channel7	99
	#define	kHWpin_Channel8	99


	//*	some boards are reverse logic so we have to do it this way
	#define		TURN_PIN_ON		1
	#define		TURN_PIN_OFF	0

#else	// _ENABLE_4REALY_BOARD_

	//==============================================================
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



int		RpiRelay_Init(const int intialState);	//*	returns the number of configured relays
bool	RpiRelay_SetRelay(const int relayNum, bool newState);	//*	returns true if OK
bool	RpiRelay_GetRelay(const int relayNumber);

#ifdef __cplusplus
}
#endif



#endif // _RPI_RELAYLIB_H_
