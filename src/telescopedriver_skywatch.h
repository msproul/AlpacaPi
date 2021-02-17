//**************************************************************************
//*	Name:			telescopedriver_skywatch.h
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
//*	Jan 30,	2021	<MLS> Created telescopedriver_skywatch.h
//*****************************************************************************
//#include	"telescopedriver_skywatch.h"



#ifndef _TELESCOPE_DRIVER_H_
	#include	"telescopedriver.h"
#endif

#ifndef _TELESCOPE_DRIVER_COMM_H_
	#include	"telescopedriver_comm.h"
#endif

void	CreateTelescope_SkyWatchObjects(void);


//**************************************************************************************
class TelescopeDriverSkyWatch: public TelescopeDriverComm
{
	public:

		//
		// Construction
		//
						TelescopeDriverSkyWatch(	DeviceConnectionType	connectionType,
													const char				*devicePath);
		virtual			~TelescopeDriverSkyWatch(void);
//		virtual	int32_t	RunStateMachine(void);
		virtual	bool	SendCmdsFromQueue(void);
		virtual	bool	SendCmdsPeriodic(void);


		//--------------------------------------------------------------------------------------------------
		//*	these routines should be implemented by the sub-classes
		//*	all have to return an Alpaca Error code
		virtual	TYPE_ASCOM_STATUS	Telescope_AbortSlew(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_MoveAxis(		const int axisNum,
															const double moveRate_degPerSec,
															char *alpacaErrMsg);

		virtual	TYPE_ASCOM_STATUS	Telescope_SlewToRA_DEC(	const double	newRtAscen_Hours,
															const double	newDeclination_Degrees,
															char *alpacaErrMsg);

		virtual	TYPE_ASCOM_STATUS	Telescope_SyncToRA_DEC(	const double	newRtAscen_Hours,
															const double	newDeclination_Degrees,
															char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_TrackingOnOff(const bool newTrackingState,
															char *alpacaErrMsg);



};
