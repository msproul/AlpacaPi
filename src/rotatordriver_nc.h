//**************************************************************************
//*	Name:			rotatordriver_nc.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Nov 28,	2020	<MLS> Updated return values to TYPE_ASCOM_STATUS
//*****************************************************************************
//#include	"rotatordriver_nc.h"

#ifndef _ROTOR_DRIVER__NC_H_
#define	_ROTOR_DRIVER__NC_H_

#ifndef	_ROTOR_DRIVER_H_
	#include	"rotatordriver.h"
#endif

#ifndef	_FOCUSER_MOONLITE_H_
	#include	"focuserdriver_nc.h"
#endif

//**************************************************************************************
class RotatorDriver_NiteCrawler: public RotatorDriver
{
	public:

		//
		// Construction
		//
						RotatorDriver_NiteCrawler(const int argDevNum, FocuserMoonLite *focuserObject = NULL);
		virtual			~RotatorDriver_NiteCrawler(void);
//		virtual	void	OutputHTML(TYPE_GetPutRequestData *reqData);
//		virtual	void	OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
//		virtual	int32_t	RunStateMachine(void);


		virtual	int32_t				ReadCurrentPoisiton_steps(void);
		virtual	double				ReadCurrentPoisiton_degs(void);
		virtual	TYPE_ASCOM_STATUS	SetCurrentPoisiton_steps(const int32_t newPosition);
		virtual	TYPE_ASCOM_STATUS	SetCurrentPoisiton_degs(const double newPosition);

		virtual	TYPE_ASCOM_STATUS	HaltMovement(void);
		virtual	bool				IsRotatorMoving(void);


	protected:
		FocuserMoonLite		*cFocuserObject;

};



#endif // _ROTOR_DRIVER__NC_H_
