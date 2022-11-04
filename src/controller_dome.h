//*****************************************************************************
//*	Jan 12,	2021	<MLS> Added _ENABLE_EXTERNAL_SHUTTER_
//*	Jan 12,	2021	<MLS> Added _ENABLE_SLIT_TRACKER_
//*****************************************************************************
//#include	"controller_dome.h"

#ifndef _CONTROLLER_DOME_H_
#define	_CONTROLLER_DOME_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif // _ALPACA_DEFS_H_

#define	_ENABLE_EXTERNAL_SHUTTER_
//#define	_ENABLE_SLIT_TRACKER_


#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif

#include	"controller.h"

//===========================================
#ifndef	_WINDOWTAB_DOME_H_
	#include	"windowtab_dome.h"
#endif


//===========================================
#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif

//===========================================
#ifndef	_WINDOWTAB_CAPABILITIES_H_
	#include	"windowtab_capabilities.h"
#endif

//===========================================
#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif



#ifdef _ENABLE_SLIT_TRACKER_
	//===========================================
	#ifndef	_WINDOWTAB_SLIT_H_
		#include	"windowtab_slit.h"
	#endif
	//===========================================
	#ifndef	_WINDOWTAB_SLIT_GRAPH_H_
		#include	"windowtab_slitgraph.h"
	#endif
	extern	bool	gUpdateSLitWindow;
#endif


#define	kDomeWindowWidth	475
#define	kDomeWindowHeight	720


//**************************************************************************************
enum
{
	kTab_Dome	=	1,
#ifdef _ENABLE_SLIT_TRACKER_
	kTab_SlitTracker,
	kTab_SlitGraph,
#endif
	kTab_Capabilities,

	kTab_DriverInfo,
	kTab_About,

	kTab_Dome_Count

};

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
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);
		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);
	//	virtual	void	DrawWidgetCustomGraphic(const int widgetIdx);
		virtual	bool	AlpacaGetStartupData(void);
				bool	AlpacaGetStatus(void);

		virtual	void	AlpacaProcessReadAll(			const char	*deviceType,
														const int	deviveNum,
														const char	*keywordString,
														const char	*valueString);
		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceType,
														const int	deviveNum,
														const char	*valueString);

		virtual	void	UpdateCommonProperties(void);
		virtual	void	UpdateCapabilityList(void);

				void	AlpacaProcessSupportedActions_Dome(const int deviveNum, const char *valueString);
				void	AlpacaProcessReadAll_Dome(		const int	deviceNum,
														const char	*keywordString,
														const char	*valueString);


				bool	AlpacaGetStatus_DomeOneAAT(void);	//*	One At A Time
				void	ToggleSwitchState(const int switchNum);
				void	UpdateDomeAzimuth(const double newAzimuth);

				void	UpdateShutterStatus(const int newShutterStatus);
				void	UpdateShutterAltitude(const double newAltitude);

			//===================================================================
				void	SetAlpacaShutterInfo(TYPE_REMOTE_DEV *alpacaDevice);
				void	SendShutterCommand(const char *shutterCmd);
			#ifdef _ENABLE_EXTERNAL_SHUTTER_
				void	AlpacaGetShutterReadAll(void);
				bool	ShutterSendPutCmd(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString);
			#endif
			//===================================================================
			#ifdef _ENABLE_SLIT_TRACKER_
				void	SetAlpacaSlitTrackerInfo(TYPE_REMOTE_DEV *alpacaDevice);
				void	AlpacaGetSlitTrackerReadAll(void);
				void	UpdateSlitLog(void);

				void	CloseSlitTrackerDataFile(void);

				WindowTabSlitTracker	*cSlitTrackerTabObjPtr;
				WindowTabSlitGraph		*cSlitGraphTabObjPtr;
			#endif // _ENABLE_SLIT_TRACKER_

				//*	tab information
				WindowTabDome			*cDomeTabObjPtr;
				WindowTabCapabilities	*cCapabilitiesTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

				TYPE_DomeProperties	cDomeProp;


				uint32_t			cDomeUpdateDelta;

			#ifdef _ENABLE_EXTERNAL_SHUTTER_
				//*	shutter device info
				bool				cShutterInfoValid;
				struct sockaddr_in	cShutterDeviceAddress;
				int					cShutterPort;
				int					cShutterAlpacaDevNum;
				bool				cShutterCommFailed;			//*	failed to communicate with shutter
				int					cShutterCommFailCnt;
			#endif // _ENABLE_EXTERNAL_SHUTTER_

			#ifdef _ENABLE_SLIT_TRACKER_
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
			#endif
};


#ifdef _SLIT_TRACKER_DIRECT_
	void	SendSlitTrackerCmd(const char *cmdBuffer);
#endif


#endif // _CONTROLLER_DOME_H_

