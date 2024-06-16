//*****************************************************************************
//*	Jan  6,	2022	<MLS> Added RemoteGAIAenabled to skytravel options
//*	Feb 10,	2022	<MLS> Added GaiaRequestMode
//*	Jul 14,	2022	<MLS> Added _ENABLE_CPU_STATS_
//*****************************************************************************
//#include	"controller_skytravel.h"

#ifndef _CONTROLLER_SKYTRAVEL_H_
#define _CONTROLLER_SKYTRAVEL_H_


#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif


#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif

#ifndef	_WINDOWTAB_IMAGE_H_
	#include	"windowtab_image.h"
#endif

#include	"windowtab_STsettings.h"

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif

#ifndef	_WINDOWTAB_DOME_H_
	#include	"windowtab_dome.h"
#endif
#ifndef	_WINDOWTAB_SKYTRAVEL_H_
	#include	"windowtab_skytravel.h"
#endif

#ifndef	_WINDOWTAB_ALPACALIST_H_
	#include	"windowtab_alpacalist.h"
#endif

#ifndef _WINDOWTAB_IP_LIST_H_
	#include	"windowtab_iplist.h"
#endif

#ifndef	_WINDOWTAB_DEVICE_SELECT_H_
	#include	"windowtab_deviceselect.h"
#endif

#ifndef _WINDOWTAB_MOON_H_
	#include	"windowtab_moon.h"
#endif

#ifndef	_WINDOWTAB_MOONPHASE_H_
	#include	"windowtab_MoonPhase.h"
#endif

#ifndef	_WINDOWTAB_FOV_H_
	#include	"windowtab_fov.h"
#endif

#ifndef	_WINDOWTAB_REMOTE_DATA_H_
	#include	"windowtab_RemoteData.h"
#endif

#ifndef	_WINDOWTAB_MOUNT_H_
	#include	"windowtab_mount.h"
#endif

#ifndef	_WINDOWTAB_CPU_STATS_H_
	#include	"windowtab_cpustats.h"
#endif

#ifndef	_WINDOWTAB_SW_VERSIONS_H_
	#include	"windowtab_sw_versions.h"
#endif
#ifndef	_WINDOWTAB_SOLAR_SYSTEM_H_
	#include	"windowtab_solarsystem.h"
#endif

#include	"windowtab_time.h"
	#ifndef	_WINDOWTAB_TIME_H_
#endif

extern	double	gTelescopeRA_Hours;
extern	double	gTelescopeRA_Radians;
extern	double	gTelescopeDecl_Degrees;
extern	double	gTelescopeDecl_Radians;

//**************************************************************************************
enum
{
	kMagnitudeMode_Dynamic	=	0,
	kMagnitudeMode_Specified,
	kMagnitudeMode_All
};

//**************************************************************************************
enum
{
	kGaiaRequestMode_1x1	=	0,
	kGaiaRequestMode_3x1,
	kGaiaRequestMode_3x3
};


//**************************************************************************************
typedef struct
{
	int		EarthDispMode;
	bool	DashedLines;
	int		LineWidth_Constellations;
	int		LineWidth_ConstOutlines;
	int		LineWidth_GridLines;
	int		LineWidth_NGCoutlines;
	int		LineWidth_MilkyWay;
	bool	DispMagnitude;			//*	display magnitude of stars if zoomed in
	bool	DispSpectralType;		//*	display spectral type as a letter if zoomed in
	bool	DispParallax;
	bool	DispProperMotion;		//*	display proper motion as numeric values if zoomed in
	bool	DispProperMotionVector;	//*	display proper motion as a vector

	bool	DayNightSkyColor;		//*	if true, enables changing the sky color based on time of day

	int		MagnitudeMode;
	double	DisplayedMagnitudeLimit;

	//*	remote SQL stuf
	bool	RemoteGAIAenabled;
	int		GaiaRequestMode;



} SkyTravelDispOptions;

//	gST_DispOptions.DashedLines
//	gST_DispOptions.DispSpectralType
extern	SkyTravelDispOptions	gST_DispOptions;

//**************************************************************************************
enum
{
	kTab_SkyTravel	=	1,
	kTab_ST_SolarSystem,
	kTab_ST_Settings,
	kTab_ST_Time,
	kTab_ST_FOV,
	kTab_ST_RemoteData,
	kTab_ST_Dome,
#ifndef __ARM_ARCH
	kTab_ST_Mount,
#endif
	kTab_DeviceList,
	kTab_AlpacaList,
	kTab_IPList,
	kTab_SwVersList,
	kTab_Moon,
	kTab_MoonPhase,
#ifdef _ENABLE_CPU_STATS_
	kTab_CPU_STATS,
#endif
	kTab_ST_About,

	kTab_ST_Count

};

//**************************************************************************************
class ControllerSkytravel: public Controller
{
	public:
		//
		// Construction
		//
				ControllerSkytravel(const char	*argWindowName);


		virtual	~ControllerSkytravel(void);


		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);
		virtual	bool	RunFastBackgroundTasks(void);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);
				bool	AlpacaProcessReadAllIdx(		const char	*deviceTypeStr,
														const int	deviceNum,
														const int	keywordEnum,
														const char	*valueString);
		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceType,
														const int	deviveNum,
														const char	*valueString);

				void	AlpacaProcessSupportedActions_Camera(	const int deviveNum, const char *valueString);
				void	AlpacaProcessSupportedActions_Dome(		const int deviveNum, const char *valueString);
				void	AlpacaProcessSupportedActions_Telescope(const int deviveNum, const char *valueString);
				bool	AlpacaProcessReadAll_Dome(		const int	deviceNum,
														const char	*keywordString,
														const char	*valueString);
				bool	AlpacaProcessReadAllIdx_Dome(	const int	deviceNum,
														const int	keywordEnum,
														const char	*valueString);

				bool	AlpacaProcessReadAll_TelescopeIdx(	const int	deviceNum,
															const int	keywordEnum,
															const char *valueString);

				bool	AlpacaProcessReadAll_Telescope(		const int	deviceNum,
															const char	*keywordString,
															const char	*valueString);
		virtual	void	AlpacaGetCapabilities(void);
				bool	AlpacaGetStartupData_Camera(TYPE_REMOTE_DEV *remoteDevice, TYPE_CameraProperties *cameraProp);
				bool	AlpacaGetStartupData_Dome(void);
				bool	AlpacaGetStartupData_Telescope(void);
				bool	AlpacaGetStatus_DomeOneAAT(void);			//*	One At A Time
				bool	AlpacaGetStatus_TelescopeOneAAT(void);		//*	One At A Time
				bool	AlpacaGetStartupData_TelescopeOneAAT(void);	//*	One At A Time
				void	ReadOneTelescopeCapability(const char *propertyStr, const char	*reportedStr, bool *booleanValue);

		virtual	void	RefreshWindow(void);

		//====================================================
		//*	tab information
				WindowTabSkyTravel		*cSkyTravelTabOjbPtr;
				WindowTabSTsettings		*cSkySettingsTabObjPtr;
				WindowTabFOV			*cFOVTabObjPtr;
				WindowTabRemoteData		*cRemoteDataObjPtr;
				WindowTabDome			*cDomeTabObjPtr;
			#ifndef __ARM_ARCH
				WindowTabMount			*cMountTabObjPtr;
			#endif
				WindowTabDeviceSelect	*cDeviceSelectObjPtr;
				WindowTabAlpacaList		*cAlpacaListObjPtr;
				WindowTabIPList			*cIPaddrListObjPtr;
				WindowTabSwVersions		*cSwVersionsListObjPtr;
				WindowTabMoon			*cMoonTabObjPtr;
				WindowTabMoonPhase		*cMoonPhaseTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

				WindowTabTime			*cTimeTabObjPtr;

				WindowTabSolarSystem	*cSolarSYstemTabObjPtr;
		//====================================================
		//*	CPU status stuff (primarily for debugging)
			#ifdef _ENABLE_CPU_STATS_
				WindowTabCpuStats		*cCpuStatsTabObjPtr;
			#endif

		//====================================================
		//*	alpaca device information
				bool				LookForIPaddress(void);
				void				SetDomeIPaddress(TYPE_REMOTE_DEV *remoteDomeDevice);
				void				SetTelescopeIPaddress(TYPE_REMOTE_DEV *remoteDomeDevice);
				bool				AlpacaGetStatus_Dome(void);

				uint32_t			cUpdateDelta;
				bool				cDomeAddressValid;
				sockaddr_in			cDomeIpAddress;
				int					cDomeIpPort;
				int					cDomeAlpacaDeviceNum;
				bool				cReadStartup_Dome;
				bool				cDomeHas_Readall;

				//----------------------------------------------------------
				//*	these are copied direct from controller_dome
				void				UpdateDomeAzimuth(const double newAzimuth);
				void				UpdateShutterStatus(const TYPE_ShutterStatus newShutterStatus);
				void				UpdateShutterAltitude(const double newAltitude);



				bool				AlpacaGetStatus_Telescope(void);
//-				bool				AlpacaGetTelescopeStatus(void);

				bool				cTelescopeAddressValid;
				sockaddr_in			cTelescopeIpAddress;
				int					cTelescopeIpPort;
				int					cTelescopeAlpacaDeviceNum;
				bool				cReadStartup_Telescope;
				bool				cTelescopeHas_readall;

				//----------------------------------------------------------
				void				Update_TelescopeRtAscension(void);
				void				Update_TelescopeDeclination(void);
				void				Update_TelescopeSideOfPier(void);

				//-----------------------------------------------------
				//*	ASCOM Dome properties
				TYPE_DomeProperties			cDomeProp;
				uint32_t					cLastDomeUpdate_milliSecs;

				//----------------------------------------------------------
				//*	ASCOM Telescope properties
				TYPE_TelescopeProperties	cTelescopeProp;
				uint32_t					cLastTelescopeUpdate_milliSecs;
};

void		CloseAllExceptFirst(void);



#endif // _CONTROLLER_SKYTRAVEL_H_
