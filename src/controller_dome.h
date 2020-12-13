//*****************************************************************************
//#include	"controller_dome.h"

#ifndef _CONTROLLER_DOME_H_
#define	_CONTROLLER_DOME_H_

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif // _DISCOVERY_LIB_H_

#include	"controller.h"

//===========================================
#ifndef	_WINDOWTAB_DOME_H_
	#include	"windowtab_dome.h"
#endif // _WINDOWTAB_DOME_H_
//===========================================
#ifndef	_WINDOWTAB_SLIT_H_
	#include	"windowtab_slit.h"
#endif // _WINDOWTAB_SLIT_H_


//===========================================
#ifndef	_WINDOWTAB_SLIT_GRAPH_H_
	#include	"windowtab_slitgraph.h"
#endif // _WINDOWTAB_SLIT_GRAPH_H_

//===========================================
#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_


extern	bool	gUpdateSLitWindow;

//**************************************************************************************
class ControllerDome: public Controller
{
	public:
		//
		// Construction
		//
						ControllerDome(	const char			*argWindowName,
										TYPE_REMOTE_DEV		*alpacaDevice);


		virtual			~ControllerDome(void);


		virtual	void	SetupWindowControls(void);
		virtual	void	RunBackgroundTasks(void);
	//	virtual	void	DrawGraphWidget(const int widgitIdx);
				bool	AlpacaGetStartupData(void);
				bool	AlpacaGetStatus(void);
		virtual	void	AlpacaProcessReadAll(const char *keywordString, const char *valueString);
				bool	AlpacaGetStatus_OneAAT(void);	//*	One At A Time
				void	ToggleSwitchState(const int switchNum);
				void	UpdateDomeAzimuth(const double newAzimuth);

				void	UpdateShutterStatus(const int newShutterStatus);
				void	UpdateShutterAltitude(const double newAltitude);

				void	SendShutterCommand(const char *shutterCmd);

				void	SetAlpacaShutterInfo(TYPE_REMOTE_DEV *alpacaDevice);
				void	AlpacaGetShutterReadAll(void);
				bool	ShutterSendPutCmd(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString);

				void	SetAlpacaSlitTrackerInfo(TYPE_REMOTE_DEV *alpacaDevice);
				void	AlpacaGetSlitTrackerReadAll(void);
				void	UpdateSlitLog(void);

				void	CloseSlitTrackerDataFile(void);

				//*	tab information
				WindowTabDome			*cDomeTabObjPtr;
				WindowTabSlitTracker	*cSlitTrackerTabObjPtr;
				WindowTabSlitGraph		*cSlitGraphTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

				bool			cCanFindHome;
				bool			cCanPark;
				bool			cCanSetAltitude;
				bool			cCanSetAzimuth;
				bool			cCanSetPark;
				bool			cCanSetShutter;
				bool			cCanSlave;
				bool			cCanSyncAzimuth;
				int				cShutterStatus;

				bool			cAtHome;
				bool			cAtPark;
				bool			cSlaved;
				bool			cSlewing;
				double			cAzimuth_Dbl;
				double			cAltitude_Dbl;

				uint32_t		cUpdateDelta;

				//*	shutter device info
				bool				cShutterInfoValid;
				struct sockaddr_in	cShutterDeviceAddress;
				int					cShutterPort;
				int					cShutterAlpacaDevNum;
				bool				cShutterCommFailed;			//*	failed to communicate with shutter
				int					cShutterCommFailCnt;

				//*	Slit tracker device info
				bool				cSlitTrackerInfoValid;
				struct sockaddr_in	cSlitTrackerDeviceAddress;
				int					cSlitTrackerPort;
				int					cSlitTrackerAlpacaDevNum;
				bool				cSlitTrackerCommFailed;			//*	failed to communicate with shutter
				struct timeval		cSlitTrackerLastUpdateTime;		//*	last time update
				bool				cLogSlitData;
				FILE				*cSlitDataLogFilePtr;

				double				cGravity_X;
				double				cGravity_Y;
				double				cGravity_Z;
				double				cGravity_T;
				bool				cValidGravity;
				double				cUpAngle_Rad;
				double				cUpAngle_Deg;
};


//*****************************************************************************
typedef struct
{
	bool	validData;
	bool	updated;
	double	distanceInches;
	long	readCount;
} TYPE_SLITCLOCK;

#define		kSensorValueCnt	12


extern	TYPE_SLITCLOCK	gSlitDistance[];
extern	int				gSlitLogIdx;


//*****************************************************************************
typedef struct
{
	bool	validData;
	double	distanceInches[kSensorValueCnt];
	double	average20pt[kSensorValueCnt];
} TYPE_SLIT_LOG;

#define	kSlitLogCount	400

extern	TYPE_SLIT_LOG	gSlitLog[kSlitLogCount];

#ifdef _SLIT_TRACKER_DIRECT_
	void	SendSlitTrackerCmd(const char *cmdBuffer);
#endif


#endif // _CONTROLLER_DOME_H_

