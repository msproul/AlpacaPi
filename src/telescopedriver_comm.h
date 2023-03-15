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
//*	Mar 31,	2021	<MLS> Moved command queue struct into telescopedriver_comm class
//*****************************************************************************
//#include	"telescopedriver_comm.h"


#ifndef _TELESCOPE_DRIVER_COMM_H_
#define	_TELESCOPE_DRIVER_COMM_H_


#ifndef _TELESCOPE_DRIVER_H_
	#include	"telescopedriver.h"
#endif



//*****************************************************************************
typedef struct
{
	int		cmdID;			//*	this is so the drive can keep track of what the command was
							//*	so the response can be processed properly
							//*	it is up to the subclass to define this value
	char	cmdString[32];
} TYPE_TelescopeCmdQue;
#define	kMaxTelescopeCmds	16



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


		//####################################################################
		//*	communications to a telescope device
		virtual	bool	StartThread(void);
		virtual	void	StopThread(void);
		virtual	void	*RunThread(void);
		virtual	void	AddCmdToQueue(const char *cmdString);
		virtual	bool	SendCmdsFromQueue(void);
		virtual	bool	SendCmdsPeriodic(void);


		int		OpenSocket(struct sockaddr_in *deviceAddress, const int port);
		int		OpenSocket(const char *ipAddress, const int port);

				//*	device communications information
				DeviceConnectionType	cDeviceConnType;
				char					cDeviceConnPath[128];	//*	Ip addr, or device path
				int						cDeviceConnFileDesc;	//*	port file descriptor
				bool					cIPaddrValid;
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
				TYPE_TelescopeCmdQue	cCmdQueue[kMaxTelescopeCmds];

};


#endif // _TELESCOPE_DRIVER_COMM_H_
