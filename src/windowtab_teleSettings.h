//*****************************************************************************
//#include	"windowtab_teleSettings.h"



#ifndef	_WINDOWTAB_TELE_SETTINGS_H_
#define	_WINDOWTAB_TELE_SETTINGS_H_

#ifndef	_WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _CONTROLLER_H_

//*****************************************************************************
enum
{
	kTeleSettings_Title	=	0,

	kTeleSettings_Connected,

	//*	drive rates
	kTeleSettings_TrackingRate_Title,
	kTeleSettings_TrackingRate_Sidereal,	//*	Sidereal tracking rate (15.041 arcseconds per second).
	kTeleSettings_TrackingRate_Lunar,		//*	Lunar tracking rate (14.685 arcseconds per second).
	kTeleSettings_TrackingRate_Solar,		//*	Solar tracking rate (15.0 arcseconds per second).
	kTeleSettings_TrackingRate_King,		//*	King tracking rate (15.0369 arcseconds per second).
	kTeleSettings_TrackingRate_Outline,

	//---------------------------------
	kTeleSettings_AtHome_Lbl,
	kTeleSettings_AtHome_Val,
	kTeleSettings_AtHome_Btn,

	kTeleSettings_AtPark_Lbl,
	kTeleSettings_AtPark_Val,
	kTeleSettings_AtPark_Btn,

//	kTeleSettings_IsPulseGuiding_Lbl,
//	kTeleSettings_IsPulseGuiding_Val,
//	kTeleSettings_IsPulseGuiding_Btn,

	kTeleSettings_Refraction_Lbl,
	kTeleSettings_Refraction_Val,
	kTeleSettings_Refraction_Btn,


	kTeleSettings_HomePark_Outline,

	//---------------------------------
	kTeleSettings_Rates_Title,
	kTeleSettings_RateRA_Lbl,
	kTeleSettings_RateRA_Val,
	kTeleSettings_RateRA_Units,
	kTeleSettings_RateDec_Lbl,
	kTeleSettings_RateDec_Val,
	kTeleSettings_RateDec_Units,


	kTeleSettings_GuideRateRA_Lbl,
	kTeleSettings_GuideRateRA_Val,
	kTeleSettings_GuideRateRA_Units,
	kTeleSettings_GuideRateDec_Lbl,
	kTeleSettings_GuideRateDec_Val,
	kTeleSettings_GuideRateDec_Units,
	kTeleSettings_GuideRate_Outline,


	//---------------------------------
	kTeleSettings_LatLabel,
	kTeleSettings_LatValue,
	kTeleSettings_LonLabel,
	kTeleSettings_LonValue,
	kTeleSettings_ElevLabel,
	kTeleSettings_ElevValue,
	kTeleSettings_ObsSettingsText,


	kTeleSettings_ErrorMsg,
	kTeleSettings_LastCmdString,
	kTeleSettings_AlpacaLogo,

	kTeleSettings_AlpacaDrvrVersion,
	kTeleSettings_IPaddr,
	kTeleSettings_Readall,


	kTeleSettings_last
};

#define	kSupportedSlewRates	4

//**************************************************************************************
class WindowTabTeleSettings: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabTeleSettings(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabTeleSettings(void);

		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawWindow(void);
//		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(const int buttonIdx);

//				void	UpdateButtons(void);
				void	UpdateTelescopeInfo(TYPE_TelescopeProperties *telescopeProp, bool updateAll);
	private:
				bool	cAtPark;
				bool	cDoesRefraction;

};


#endif // _WINDOWTAB_TELE_SETTINGS_H_


