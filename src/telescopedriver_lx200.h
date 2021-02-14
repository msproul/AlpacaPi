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
//-		virtual	TYPE_ASCOM_STATUS	Telescope_AbortSlew(char *alpacaErrMsg);
//-		virtual	TYPE_ASCOM_STATUS	Telescope_SlewToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg);
//-		virtual	TYPE_ASCOM_STATUS	Telescope_SyncToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg);

				bool			Process_GR_RtAsc(char *dataBuffer);
				bool			Process_GD(char *dataBuffer);
				bool			Process_GT(char *dataBuffer);

				bool			cTelescopeInfoValid;
				int				cLX200_SocketErrCnt;
				int				cLX200_OutOfBoundsCnt;
				char			cTelescopeRA_String[32];
				char			cTelescopeDecl_String[32];

#ifdef _USE_TELESCOPE_COM_OBJECT_
		TelescopeComm				*cTelescopeComm;
#endif
};
