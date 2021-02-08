//**************************************************************************
//*	Name:			telescopedriver_comm.h
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
//*	Feb  7,	2021	<MLS> Created telescopedriver_comm.h
//*****************************************************************************
//#include	"telescopedriver_comm.h"


#ifndef _TELESCOPE_DRIVER_COMM_H_
#define	_TELESCOPE_DRIVER_COMM_H_


#ifndef _TELESCOPE_DRIVER_H_
	#include	"telescopedriver.h"
#endif

void	CreateTelescopeObjects(void);


//**************************************************************************************
class TelescopeDriverComm: public TelescopeDriver
{
	public:

		//
		// Construction
		//
								TelescopeDriverComm(	DeviceConnectionType	connectionType,
														const char				*devicePath);
		virtual					~TelescopeDriverComm(void);
		virtual	int32_t			RunStateMachine(void);



		virtual	bool				AlpacaConnect(void);
		virtual	bool				AlpacaDisConnect(void);
		virtual	TYPE_ASCOM_STATUS	Telescope_AbortSlew(char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SlewToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg);
		virtual	TYPE_ASCOM_STATUS	Telescope_SyncToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg);


		//####################################################################
		//*	communications to a telescope device
		virtual	bool	StartThread(void);
		virtual	void	StopThread(void);
		virtual	void	*RunThread(void);
		virtual	bool	SendCmdsFromQueue(void);
		virtual	bool	SendCmdsPeriodic(void);


		virtual	bool	SlewScopeDegrees(	const double	newRtAscen_Hours,
											const double	newDeclination_Degrees,
											char			*returnErrMsg);

		virtual	bool	SyncScope(			const double	newRtAscen_Radians,
											const double	new_Declination_Radians,
											char			*returnErrMsg);

		virtual	bool	SyncScopeDegrees(	const double	newRtAscen_Hours,
											const double	newDeclination_Degrees,
											char			*returnErrMsg);


		virtual	bool	StopMovement(void);

		int		OpenSocket(struct sockaddr_in	*deviceAddress, const int port);
		int		OpenSocket(const char *ipAddress, const int port);


				//*	device communications information
//-				DeviceConnectionType	cDeviceConnType;
//-				char					cDeviceConnPath[64];	//*	this may be an IP address or a /dev/tty... string
//-				int						cDeviceConnFileDesc;	//*	port file descriptor

//+				DeviceConnectionType	cDeviceConnType;
//+				char					cDeviceConnPath[128];	//*	Ip addr, or device path
				char					cDeviceIPaddress[128];
				int						cTCPportNum;
				int						cSocket_desc;
				int						cBaudRate;
				bool					cThreadIsActive;
				bool					cKeepRunningFlag;
				pthread_t				cThreadID;
				char					cTelescopeErrorString[256];
				bool					cNewTelescopeDataAvailble;

				//---------------------------------
				//*	command queue
				int						cQueuedCmdCnt;

};


#endif // _TELESCOPE_DRIVER_COMM_H_
