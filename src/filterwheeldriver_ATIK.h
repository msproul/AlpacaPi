//**************************************************************************
//*	Name:			filterwheeldriver_ATIK.h
//*
//**************************************************************************
//#include	"filterwheeldriver_ATIK.h"

#ifndef _FILTERWHEELDRIVER_ATIK_H_
	#define _FILTERWHEELDRIVER_ATIK_H_

#ifndef _FILTERWHEELDRIVER_H_
	#include	"filterwheeldriver.h"
#endif

#include	"AtikCameras.h"

//**************************************************************************************
class FilterwheelATIK: public FilterwheelDriver
{
	public:

		//
		// Construction
		//
						FilterwheelATIK(const int argAtikDeviceNumber);
		virtual			~FilterwheelATIK(void);
		virtual	bool	AlpacaConnect(void);

	protected:
		void						ReadFilterWheelInfo(void);

		virtual	int					Read_CurrentFWstate(void);
		virtual	TYPE_ASCOM_STATUS	Read_CurrentFilterPositon(int *rtnCurrentPosition = NULL);
		virtual	TYPE_ASCOM_STATUS	Set_CurrentFilterPositon(const int newPosition);

		bool						cForceReadPosition;
		int							cActualPosReadCout;
//-		EFW_INFO					cFilterwheelInfo;

		ArtemisHandle				cAtikEFWHandle;
		int							cArtimisErrorCnt;
};

void	CreateFilterWheelObjects_ATIK(void);

#endif // _FILTERWHEELDRIVER_ATIK_H_
