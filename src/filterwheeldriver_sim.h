//**************************************************************************
//*	Name:			filterwheeldriver_sim.h
//*
//**************************************************************************
//*	Mar  1,	2023	<MLS> Created filterwheeldriver_SIM.h
//**************************************************************************
//#include	"filterwheeldriver_sim.h"

#ifndef _FILTERWHEELDRIVER_SIM_H_
#define	_FILTERWHEELDRIVER_SIM_H_


#ifndef _FILTERWHEELDRIVER_H_
	#include	"filterwheeldriver.h"
#endif

//**************************************************************************************
class FilterwheelSIM: public FilterwheelDriver
{
	public:

		//
		// Construction
		//
						FilterwheelSIM(const int deviceNumber);
		virtual			~FilterwheelSIM(void);
		virtual	bool	AlpacaConnect(void);

	protected:
		void						ReadFilterWheelInfo(void);

		virtual	int					Read_CurrentFWstate(void);
		virtual	TYPE_ASCOM_STATUS	Read_CurrentFilterPositon(int *rtnCurrentPosition = NULL);
		virtual	TYPE_ASCOM_STATUS	Set_CurrentFilterPositon(const int newPosition);

		bool						cForceReadPosition;
		int							cActualPosReadCout;

};

void	CreateFilterWheelObjects_SIM(void);

#endif // _FILTERWHEELDRIVER_SIM_H_

