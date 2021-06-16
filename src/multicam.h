//**************************************************************************
//*	Name:			multicam.h
//*
//*	Author:			Mark Sproul (C) 2019
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*		https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*		https://github.com/OpenPHDGuiding/phd2/tree/master/cameras/zwolibs
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*****************************************************************************
//#include	"multicam.h"

#ifndef _DOME_DRIVER_H_
#define	_DOME_DRIVER_H_

#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif



//**************************************************************************************
class MultiCam: public AlpacaDriver
{
	public:

		//
		// Construction
		//
						MultiCam(const int argDevNum);
		virtual			~MultiCam(void);

		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	int32_t				RunStateMachine(void);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);

			TYPE_ASCOM_STATUS		StartExposure(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
			TYPE_ASCOM_STATUS		SetExposureTime(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

	protected:
				int		cCameraCnt;
				int		cMultiCamState;
};





void	CreateMultiCamObject(void);


#endif	//	_DOME_DRIVER_H_
