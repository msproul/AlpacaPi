//**************************************************************************
//*	Name:			filterwheeldriver_PlayerOne.h
//*
//**************************************************************************
//#include	"filterwheeldriver_PlayerOne.h"

#ifndef _FILTERWHEELDRIVER_PLAYERONE_H_
	#define _FILTERWHEELDRIVER_PLAYERONE_H_

#ifndef _FILTERWHEELDRIVER_H_
	#include	"filterwheeldriver.h"
#endif

#ifndef PLAYERONEPW_H
	#include	"PlayerOnePW.h"
#endif


//**************************************************************************************
class FilterwheelPlayerOne: public FilterwheelDriver
{
	public:

		//CreateFilterWheelObjects_PlayerOne
		// Construction
		//
						FilterwheelPlayerOne(const int argPlayerDeviceNumber);
		virtual			~FilterwheelPlayerOne(void);
		virtual	bool	AlpacaConnect(void);

	protected:
		void						ReadFilterWheelInfo(void);

		virtual	int					Read_CurrentFWstate(void);
		virtual	TYPE_ASCOM_STATUS	Read_CurrentFilterPositon(int *rtnCurrentPosition = NULL);
		virtual	TYPE_ASCOM_STATUS	Set_CurrentFilterPositon(const int newPosition);

		bool						cForceReadPosition;
		int							cActualPosReadCout;

		int							cPWindex;
		PWProperties				cPWProperties;
		PWState						cPWstate;

};

void	CreateFilterWheelObjects_PlayerOne(void);

#endif // _FILTERWHEELDRIVER_PLAYERONE_H_
