//**************************************************************************
//*	Name:			telescopedriver_lx200.h
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
//*	Jan 13,	2021	<MLS> Created telescopedriver_lx200.h
//*****************************************************************************
//#include	"telescopedriver_lx200.h"



#ifdef _USE_TELESCOPE_COM_OBJECT_
	#ifndef _TELESCOPE_COMM_H_
		#include	"telescope_comm.h"
	#endif
#endif

#ifndef _TELESCOPE_DRIVER_H_
	#include	"telescopedriver.h"
#endif

#ifndef _TELESCOPE_DRIVER_COMM_H_
	#include	"telescopedriver_comm.h"
#endif

void	CreateTelescopeLX200Objects(void);


//**************************************************************************************
class TelescopeDriverLX200: public TelescopeDriverComm
{
	public:

		//
		// Construction
		//
								TelescopeDriverLX200(	DeviceConnectionType	connectionType,
														const char				*devicePath);
		virtual					~TelescopeDriverLX200(void);
//-		virtual	int32_t			RunStateMachine(void);

		virtual	bool			SendCmdsFromQueue(void);
		virtual	bool			SendCmdsPeriodic(void);


//-		virtual	bool				AlpacaConnect(void);
//-		virtual	bool				AlpacaDisConnect(void);
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


//-				void			AddCmdToLX200queue(const char *cmdString);
				bool			Process_GR_RtAsc(char *dataBuffer);
				bool			Process_GD(char *dataBuffer);
				bool			Process_GT(char *dataBuffer);

				bool			cTelescopeInfoValid;
				int				cLX200_SocketErrCnt;
				int				cLX200_OutOfBoundsCnt;
				char			cTelescopeRA_String[32];
				char			cTelescopeDecl_String[32];



};
