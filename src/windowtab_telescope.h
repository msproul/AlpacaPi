//*****************************************************************************
//#include	"windowtab_telescope.h"



#ifndef	_WINDOWTAB_TELESCOPE_H_
#define	_WINDOWTAB_TELESCOPE_H_

#ifndef	_WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _CONTROLLER_H_

//*****************************************************************************
enum
{
	kTelescope_Title	=	0,

	kTelescope_Connected,

	kTelescope_BtnUp,
	kTelescope_BtnDown,
	kTelescope_BtnEast,
	kTelescope_BtnWest,
	kTelescope_BtnEmergencyStop,

	kTelescope_BtnTrackingOn,
	kTelescope_BtnTrackingOff,
	kTelescope_TrackingStatus,


	kTelescope_SlewRate_Title,

	kTelescope_SlewRate_VerySlow,
	kTelescope_SlewRate_Slow,
	kTelescope_SlewRate_Med,
	kTelescope_SlewRate_Fast,

	kTelescope_SlewRate_VerySlow_Val,
	kTelescope_SlewRate_Slow_Val,
	kTelescope_SlewRate_Med_Val,
	kTelescope_SlewRate_Fast_Val,

	kTelescope_SlewingStatus,
	kTelescope_SlewRate_Outline,

	kTelescope_RA_label,
	kTelescope_RA_value,
	kTelescope_HA_label,
	kTelescope_HA_value,
	kTelescope_DEC_label,
	kTelescope_DEC_value,

	//*	drive rates
	kTelescope_TrackingRate_Title,
	kTelescope_TrackingRate_Sidereal,	//*	Sidereal tracking rate (15.041 arcseconds per second).
	kTelescope_TrackingRate_Lunar,		//*	Lunar tracking rate (14.685 arcseconds per second).
	kTelescope_TrackingRate_Solar,		//*	Solar tracking rate (15.0 arcseconds per second).
	kTelescope_TrackingRate_King,		//*	King tracking rate (15.0369 arcseconds per second).
	kTelescope_TrackingRate_Outline,


	kTelescope_ErrorMsg,
	kTelescope_LastCmdString,
	kTelescope_AlpacaLogo,

	kTelescope_AlpacaDrvrVersion,
	kTelescope_IPaddr,
	kTelescope_Readall,

	kTelescope_last
};

#define	kSupportedSlewRates	4

//**************************************************************************************
class WindowTabTelescope: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabTelescope(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabTelescope(void);

//		virtual	void	DrawGraphWidget(const int widgetIdx);
		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawWindow(void);
//		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx);
//		virtual	void	ProcessDoubleClick(const int buttonIdx);

				void	UpdateButtons(void);
				void	UpdateTelescopeInfo(TYPE_TelescopeProperties	*telescopeProp);
				int		cCurrentSlewRate;
	private:
				bool		cSlewingUp;
				bool		cSlewingDown;
				bool		cSlewingEast;
				bool		cSlewingWest;
				//*	button colors
				cv::Scalar	cBtnBGcolor_Normal;
				cv::Scalar	cBtnBGcolor_Slewing;
				cv::Scalar	cBtnBGcolor_Disabled;
				cv::Scalar	cBtnTXTcolor;

				bool		cSlewRates_RA_valid;
				bool		cSlewRates_DEC_valid;
				double		cSlewRate_RAmin;
				double		cSlewRate_RAmax;
				double		cRA_slewRates[kSupportedSlewRates];

				double		cSlewRate_DECmin;
				double		cSlewRate_DECmax;
				double		cDEC_slewRates[kSupportedSlewRates];

};


#endif // _WINDOWTAB_TELESCOPE_H_


