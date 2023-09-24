//**************************************************************************
//*	Name:			filterwheeldriver_QHY.h
//*
//**************************************************************************
//#include	"filterwheeldriver_QHY.h"


#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef _FILTERWHEELDRIVER_QHY_H_
	#define _FILTERWHEELDRIVER_QHY_H_

#ifndef _FILTERWHEELDRIVER_H_
	#include	"filterwheeldriver.h"
#endif



//**************************************************************************************
class FilterwheelQHY: public FilterwheelDriver
{
	public:

		//
		// Construction
		//
						FilterwheelQHY(const char *usbPath);
		virtual			~FilterwheelQHY(void);
		virtual	bool	AlpacaConnect(void);

	protected:
		virtual	int					Read_CurrentFWstate(void);
		virtual	TYPE_ASCOM_STATUS	Read_CurrentFilterPositon(int *rtnCurrentPosition = NULL);
		virtual	TYPE_ASCOM_STATUS	Set_CurrentFilterPositon(const int newPosition);

		bool						cForceReadPosition;
		int							cActualPosReadCout;
		char						cUSBportPath[64];
		int							cQHYusbPort_fileDesc;
		virtual	void				RunThread_Startup(void);
		virtual	void				RunThread_Loop(void);

		//*	commands to the FilterWheel are done in a separate thread
		//*	this is the data to tell the thread what to do
		bool						cMoveNewPosition;
		int							cNewPosition;
		bool						cReadPosition;

};

void	CreateFilterWheelObjects_QHY(void);

#endif // _FILTERWHEELDRIVER_QHY_H_
