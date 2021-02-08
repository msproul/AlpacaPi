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
															const char			*devicePath);
		virtual					~TelescopeDriverSkyWatch(void);
//		virtual	int32_t			RunStateMachine(void);





};
