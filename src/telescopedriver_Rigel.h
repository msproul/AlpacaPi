//**************************************************************************
//*	Name:			telescopedriver_Rigel.h
//*
//*	Author:			Mark Sproul (C) 2021
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 20,	2021	<MLS> Created telescopedriver_Rigel.h
//*****************************************************************************
//#include	"telescopedriver_Rigel.h"



#ifdef _USE_TELESCOPE_COM_OBJECT_
	#ifndef _TELESCOPE_COMM_H_
		#include	"telescope_comm.h"
	#endif
#endif

#ifndef _TELESCOPE_DRIVER_H_
	#include	"telescopedriver.h"
#endif



void	CreateTelescopeRigelObjects(void);

//**************************************************************************************
class TelescopeDriverRigel: public TelescopeDriver
{
	public:

		//
		// Construction
		//
								TelescopeDriverRigel(void);
		virtual					~TelescopeDriverRigel(void);
		virtual	int32_t				RunStateMachine(void);


		//*************************************************************************
		//*	DO NOT IMPLEMENT THE SYNCHRONOUS METHODS
		//*		Use the ASYNC methods instead
		//*		Alpaca cannot do synchronous and ASCOM/ALPACA are trying to eliminate all SYNC commands
		//*************************************************************************


		//--------------------------------------------------------------------------------------------------
		//*	these routines should be implemented by the sub-classes
		//*	all have to return an Alpaca Error code
		virtual	TYPE_ASCOM_STATUS	Telescope_AbortSlew(	char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_FindHome(		char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_MoveAxis(		const int axisNum,
															const double moveRate_degPerSec,
															char *alpacaErrMsg);

		virtual	TYPE_ASCOM_STATUS	Telescope_Park(			char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SetPark(		char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SlewToAltAz(	const double	newAlt_Degrees,
															const double	newAz_Degrees,
															char *alpacaErrMsg);

		virtual	TYPE_ASCOM_STATUS	Telescope_SlewToRA_DEC(	const double	newRtAscen_Hours,
															const double	newDeclination_Degrees,
															char *alpacaErrMsg);



		virtual	TYPE_ASCOM_STATUS	Telescope_SyncToRA_DEC(	const double	newRtAscen_Hours,
															const double	newDeclination_Degrees,
															char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_TrackingOnOff(const bool newTrackingState,
															char *alpacaErrMsg);


		virtual	TYPE_ASCOM_STATUS	Telescope_UnPark(		char *alpacaErrMsg);


};
