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

void	CreateTelescope_SkyWatchObjects(void);


//**************************************************************************************
class TelescopeDriverSkyWatch: public TelescopeDriver
{
	public:

		//
		// Construction
		//
								TelescopeDriverSkyWatch(	DeviceConnectionType	connectionType,
															const char			*devicePath);
		virtual					~TelescopeDriverSkyWatch(void);
		virtual	int32_t			RunStateMachine(void);




		virtual	bool				AlpacaConnect(void);
		virtual	bool				AlpacaDisConnect(void);
		virtual	TYPE_ASCOM_STATUS	Telescope_AbortSlew(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SlewToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SyncToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg);


		char					cSkyWatchDevicePath[64];	//*	this may be an IP address or a /dev/tty... string
};
