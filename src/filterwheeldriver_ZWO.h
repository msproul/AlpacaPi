//**************************************************************************
//*	Name:			filterwheeldriver_ZWO.h
//*
//**************************************************************************
//#include	"filterwheeldriver_ZWO.h"


#ifndef _FILTERWHEELDRIVER_ZWO_H_
	#define	_FILTERWHEELDRIVER_ZWO_H_



#ifndef EFW_FILTER_H
	#include	"include/EFW_filter.h"
#endif

#ifndef _FILTERWHEELDRIVER_H_
	#include	"filterwheeldriver.h"
#endif

//**************************************************************************************
class FilterwheelZWO: public FilterwheelDriver
{
	public:

		//
		// Construction
		//
						FilterwheelZWO(const int argDevNum);
		virtual			~FilterwheelZWO(void);

	protected:
		void						ReadZWOfilterWheelInfo(void);

		virtual	int					Read_CurrentFWstate(void);
		virtual	TYPE_ASCOM_STATUS	Read_CurrentFilterPositon(int *rtnCurrentPosition = NULL);
		virtual	TYPE_ASCOM_STATUS	Set_CurrentFilterPositon(const int newPosition);

		bool						cForceReadPosition;
		int							cActualPosReadCout;
		EFW_INFO					cFilterwheelInfo;


};

void	CreateFilterWheelObjects_ZWO(void);
#endif // _FILTERWHEELDRIVER_ZWO_H_
