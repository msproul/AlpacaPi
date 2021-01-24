//*****************************************************************************
//#include	"controller_telescope.h"


#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif



#include	"controller.h"

#include	"windowtab_telescope.h"

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_



//*****************************************************************************
class ControllerTelescope: public Controller
{
	public:
		//
		// Construction
		//
				ControllerTelescope(const char			*argWindowName,
									struct sockaddr_in	*deviceAddress,
									const int			port,
									const int			deviceNum);


		virtual	~ControllerTelescope(void);


		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(void);

		virtual	bool	AlpacaGetStartupData(void);
		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceType,
														const int	deviveNum,
														const char	*valueString);

		virtual	void	AlpacaProcessReadAll(			const char	*deviceType,
														const int	deviveNum,
														const char	*keywordString,
														const char	*valueString);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);

				void	AlpacaProcessSupportedActions_Telescope(	const int	deviveNum,
																	const char	*valueString);
				void	AlpacaProcessReadAll_Telescope(	const int	deviceNum,
														const char	*keywordString,
														const char *valueString);
				bool	AlpacaGetStatus(void);
				bool	AlpacaGetStatus_TelescopeOneAAT(void);	//*	One At A Time
				bool	AlpacaGetStartupData_TelescopeOneAAT(void);	//*	One At A Time
			//*	tab information
				WindowTabTelescope	*cTelescopeTabObjPtr;
				WindowTabAbout		*cAboutBoxTabObjPtr;


				//---------------------------------------------------
				void			Update_TelescopeRtAscension(void);
				void			Update_TelescopeDeclination(void);

				//---------------------------------------------------
				//*	telescope driver variables
				TYPE_TelescopeProperties	cTelescopeProp;


//				double				cAltitude;
//				double				cApertureArea;
//				double				cApertureDiameter;
//				bool				cAtHome;
//				bool				cAtPark;
//				double				cAzimuth;
//				bool				cCanFindHome;
//				bool				cCanMoveAxis;
//				bool				cCanPark;
//				bool				cCanPulseGuide;
//				bool				cCanSetDeclinationRate;
//				bool				cCanSetGuideRates;
//				bool				cCanSetPark;
//				bool				cCanSetPierSide;
//				bool				cCanSetRightAscensionRate;
//				bool				cCanSetTracking;
//				bool				cCanSlew;
//				bool				cCanSlewAltAz;
//				bool				cCanSlewAltAzAsync;
//				bool				cCanSlewAsync;
//				bool				cCanSync;
//				bool				cCanSyncAltAz;
//				bool				cCanUnpark;

//				bool				cTargetDec_HasBeenSet;
//				double				cDeclination;				//*	degrees
//				double				cDeclinationRate;

//				bool				cTargetRA_HasBeenSet;
//				double				cRightAscension;			//*	hours
//				double				cRightAscensionRate;


//				bool				cDoesRefraction;
//				TYPE_EquatorialCoordinateType	cEquatorialSystem;
//				double				cFocalLength;
//				double				cGuideRateDeclination;
//				double				cGuideRateRightAscension;
//				bool				cIsPulseGuiding;
//				TYPE_PierSide		cSideOfPier;
//				double				cSiderealTime;
//				double				cSiteElevation;
//				double				cSiteLatitude;
//				double				cSiteLongitude;
//				short				cSlewSettleTime;
//				double				cTargetDeclination;
//				double				cTargetRightAscension;

				//*	had to change the name of some of them because of conflict with DOME
//				bool				cTelescopeSlewing;
//				bool				cTelescopeTracking;

//				TYPE_DriveRates		cTrackingRate;
		//+		double				cTrackingRates;
		//+		double				cUTCDate;


};


