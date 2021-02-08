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



#ifndef _TELESCOPE_COMM_H_
	#include	"telescope_comm.h"
#endif

#ifndef _TELESCOPE_DRIVER_H_
	#include	"telescopedriver.h"
#endif

void	CreateTelescopeLX200Objects(void);


//**************************************************************************************
class TelescopeDriverLX200: public TelescopeDriver
{
	public:

		//
		// Construction
		//
								TelescopeDriverLX200(	DeviceConnectionType	connectionType,
														const char				*devicePath);
		virtual					~TelescopeDriverLX200(void);
		virtual	int32_t			RunStateMachine(void);



		virtual	bool				AlpacaConnect(void);
		virtual	bool				AlpacaDisConnect(void);
		virtual	TYPE_ASCOM_STATUS	Telescope_AbortSlew(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SlewToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SyncToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg);


		TelescopeComm				*cTelescopeComm;

};
