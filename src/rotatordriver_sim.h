//**************************************************************************
//*	Name:			rotatordriver_sim.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar  1,	2023	<MLS> Created rotatordriver_sim.h
//*****************************************************************************
//#include	"rotatordriver_sim.h"

#ifndef _ROTOR_DRIVER__SIM_H_
#define	_ROTOR_DRIVER__SIM_H_


#ifndef	_ROTOR_DRIVER_H_
	#include	"rotatordriver.h"
#endif // _ROTOR_DRIVER_H_


//**************************************************************************************
class RotatorDriver_Sim: public RotatorDriver
{
	public:

		//
		// Construction
		//
						RotatorDriver_Sim(const int argDevNum);
		virtual			~RotatorDriver_Sim(void);
//		virtual	void	OutputHTML(TYPE_GetPutRequestData *reqData);
//		virtual	void	OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
//		virtual	int32_t	RunStateMachine(void);


		virtual	int32_t				ReadCurrentPoisiton_steps(void);
		virtual	double				ReadCurrentPoisiton_degs(void);
		virtual	TYPE_ASCOM_STATUS	SetCurrentPoisiton_steps(const int32_t newPosition);
		virtual	TYPE_ASCOM_STATUS	SetCurrentPoisiton_degs(const double newPosition_Degs);

		virtual	TYPE_ASCOM_STATUS	HaltMovement(void);
		virtual	bool				IsRotatorMoving(void);


	protected:

};

void	CreateRotatorObjects_SIM(void);


#endif // _ROTOR_DRIVER__SIM_H_
