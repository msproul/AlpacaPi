//*****************************************************************************
//*	Jan 12,	2021	<MLS> Added _ENABLE_SLIT_TRACKER_
//*****************************************************************************
//#include	"controller_slit.h"

#ifndef _CONTROLLER_SLITTRACKER_H_
#define	_CONTROLLER_SLITTRACKER_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif

//===========================================
#ifndef	_WINDOWTAB_SLIT_DOME_H_
	#include	"windowtab_slitdome.h"
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

//===========================================
#ifndef	_WINDOWTAB_SLIT_H_
	#include	"windowtab_slit.h"
#endif
//===========================================
#ifndef	_WINDOWTAB_SLIT_GRAPH_H_
	#include	"windowtab_slitgraph.h"
#endif
extern	bool	gUpdateSLitWindow;


//**************************************************************************************
class ControllerSlit: public Controller
{
	public:
		//
		// Construction
		//
						ControllerSlit(	const char			*argWindowName,
										TYPE_REMOTE_DEV		*alpacaDevice);


		virtual			~ControllerSlit(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	GetStartUpData_SubClass(void);
		virtual	void	GetStatus_SubClass(void);
		virtual	void	UpdateStartupData(void);
		virtual	void	UpdateStatusData(void);
		virtual	void	UpdateOnlineStatus(void);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);

		virtual	bool	AlpacaProcessReadAllIdx(		const char	*deviceTypeStr,
														const int	deviceNum,
														const int	keywordEnum,
														const char	*valueString);
		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceType,
														const int	deviveNum,
														const char	*valueString);

		virtual	void	UpdateSupportedActions(void);
		virtual	void	UpdateCommonProperties(void);
		virtual	void	UpdateCapabilityList(void);

				bool	ProcessOneReadAllEntry(		const char	*keywordString,
													const char *valueString);
				bool	AlpacaProcessReadAllIdx_Dome(	const int	deviceNum,
														const int	keywordEnum,
														const char	*valueString);
				bool	AlpacaProcessReadAllIdx_Slit(	const int	deviceNum,
														const int	keywordEnum,
														const char	*valueString);
				void	ProcessGravityVector(		const int	gravityVecIndex,
													const char *valueString);
				void	SetButtonOption(const int widgetBtnIdx, const bool newState);

				bool	SetAlpacaEnableTracking(const bool newState);

				//===================================================================
				void	AlpacaGetSlitTrackerReadAll(void);
				void	UpdateSlitLog(void);
				void	LogSlitDataToDisk(void);

				void	CloseSlitTrackerDataFile(void);

				void	GetDomeData_Startup(void);
				void	GetDomeData_Periodic(void);

				//===================================================================
				//*	tab information
				WindowTabSlitTracker	*cSlitTrackerTabObjPtr;
				WindowTabSlitGraph		*cSlitGraphTabObjPtr;

				WindowTabSlitDome		*cSlitDomeTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;



				uint32_t			cUpdateDelta_Secs;
				uint32_t			cDomeUpdateDelta_Secs;
				uint32_t			cLastDomeUpdate_milliSecs;
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

				//---------------------------------------------
				//*	connection to dome for slaving
				bool				cEnableAutomaticDomeUpdates;
				bool				cForceDomeUpdate;
				bool				cEnableDomeTracking;
//				bool				cSlaveEnabled;
				char				cDomeIPaddressString[48];
				int					cDomeAlpacaPort;
				int					cDomeAlpacaDevNum;
				bool				cDomeHas_Readall;
				int					cDomeReadAllCount;	//*	used to verify read all
				TYPE_DomeProperties			cDomeProp;
				TYPE_SlittrackerProperties	cSlitProp;

};



#endif // _CONTROLLER_SLITTRACKER_H_

