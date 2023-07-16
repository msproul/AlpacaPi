//*****************************************************************************
//*	Jan 12,	2021	<MLS> Added _ENABLE_EXTERNAL_SHUTTER_
//*	Jan 12,	2021	<MLS> Added _ENABLE_SLIT_TRACKER_
//*	Mar  9,	2023	<MLS> Removed _ENABLE_SLIT_TRACKER_
//*****************************************************************************
//#include	"controller_dome.h"

#ifndef _CONTROLLER_DOME_H_
#define	_CONTROLLER_DOME_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif

#include	"controller.h"

//===========================================
#ifndef	_WINDOWTAB_DOME_H_
	#include	"windowtab_dome.h"
#endif

//===========================================
#ifndef	_WINDOWTAB_CAPABILITIES_H_
	#include	"windowtab_capabilities.h"
#endif

//===========================================
#ifndef	_WINDOWTAB_DEVICESTATE_H_
	#include	"windowtab_DeviceState.h"
#endif

//===========================================
#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif


//===========================================
#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif

#define	_ENABLE_EXTERNAL_SHUTTER_


#define	kDomeWindowWidth	475
#define	kDomeWindowHeight	720



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
		virtual	void	UpdateConnectedStatusIndicator(void);
		virtual	void	GetStartUpData_SubClass(void);
		virtual	bool	AlpacaGetStartupData_OneAAT(void);
		virtual	void	UpdateStartupData(void);
		virtual	void	UpdateStatusData(void);
		virtual	void	UpdateOnlineStatus(void);
		virtual	void	AlpacaGetCapabilities(void);

		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceType,
														const int	deviveNum,
														const char	*valueString);

		virtual	void	UpdateSupportedActions(void);
		virtual	void	UpdateCapabilityList(void);

		virtual	void	AlpacaProcessSupportedActions_Dome(const int deviveNum, const char *valueString);
		virtual	bool	AlpacaProcessReadAllIdx(		const char	*deviceTypeStr,
														const int	deviceNum,
														const int	keywordEnum,
														const char	*valueString);
				bool	AlpacaProcessReadAllIdx_Dome(	const int	deviceNum,
														const int	keywordEnum,
														const char	*valueString);

				bool	AlpacaGetStatus_DomeOneAAT(void);	//*	One At A Time
				void	UpdateDomeAzimuth(const double newAzimuth);

				void	UpdateShutterStatus(const TYPE_ShutterStatus newShutterStatus);
				void	UpdateShutterAltitude(const double newAltitude);

			//===================================================================
				void	SetAlpacaShutterInfo(TYPE_REMOTE_DEV *alpacaDevice);
				void	SendShutterCommand(const char *shutterCmd);

				//===================================================================
				//*	tab information
				WindowTabDome			*cDomeTabObjPtr;
				WindowTabCapabilities	*cCapabilitiesTabObjPtr;
				WindowTabDeviceState	*cDeviceStateTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

				TYPE_DomeProperties		cDomeProp;

			#ifdef _ENABLE_EXTERNAL_SHUTTER_
				//*	shutter device info
				void	AlpacaGetShutterReadAll(void);
				bool	ShutterSendPutCmd(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString);

				bool				cShutterInfoValid;
				struct sockaddr_in	cShutterDeviceAddress;
				int					cShutterPort;
				int					cShutterAlpacaDevNum;
				bool				cShutterCommFailed;			//*	failed to communicate with shutter
				int					cShutterCommFailCnt;
			#endif // _ENABLE_EXTERNAL_SHUTTER_
};


#endif // _CONTROLLER_DOME_H_

