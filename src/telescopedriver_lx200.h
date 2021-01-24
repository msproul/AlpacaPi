//**************************************************************************
//*	Name:			telescopedriver.h
//*
//*	Author:			Mark Sproul (C) 2020
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



#ifndef _TELESCOPE_DRIVER_H_
	#include	"telescopedriver.h"
#endif

void	CreateTelescopeObjects(void);

//**************************************************************************************
enum LX200ConnectionType
{
	kLX200_Ethernet,
	kLX200_USB,
	kLX200_Serial

};

//**************************************************************************************
class TelescopeDriverLX200: public TelescopeDriver
{
	public:

		//
		// Construction
		//
								TelescopeDriverLX200(	LX200ConnectionType	connectionType,
															const char			*devicePath);
		virtual					~TelescopeDriverLX200(void);
		virtual	int32_t			RunStateMachine(void);




		virtual	void				AlpacaConnect(void);
		virtual	void				AlpacaDisConnect(void);
		virtual	TYPE_ASCOM_STATUS	Telescope_AbortSlew(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SlewToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SyncToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg);




		LX200ConnectionType		cLX200connType;
		char					cLX200devicePath[64];	//*	this may be an IP address or a /dev/tty... string
};
