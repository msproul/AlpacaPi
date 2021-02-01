//*****************************************************************************
//#include	"windowtab_about.h"

#ifndef	_WINDOWTAB_ST_SETTINGS_H_
#define	_WINDOWTAB_ST_SETTINGS_H_




#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif




#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H

//*****************************************************************************
enum
{
	kSkyT_Settings_Title	=	0,

//	kSkyT_Settings_ControllerVersion,

	kSkyT_Settings_LatLonTitle,
	kSkyT_Settings_LatLable,
	kSkyT_Settings_LatValue1,
	kSkyT_Settings_LatValue2,

	kSkyT_Settings_LonLable,
	kSkyT_Settings_LonValue1,
	kSkyT_Settings_LonValue2,

	kSkyT_Settings_TimeZoneLbl,
	kSkyT_Settings_UTCoffset,
	kSkyT_Settings_TimeZoneTxt,

	kSkyT_Settings_ObsSettingsText,

	kSkyT_Settings_SiteInfoOutline,

	kSkyT_Settings_EarthLable,
	kSkyT_Settings_EarthThin,
	kSkyT_Settings_EarthThick,
	kSkyT_Settings_EarthSolidBrn,
	kSkyT_Settings_EarthSolidGrn,
	kSkyT_Settings_EarthOutline,


	kSkyT_Settings_AlpacaLogo,


	kSkyT_Settings_last
};


//**************************************************************************************
class WindowTabSTsettings: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabSTsettings(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabSTsettings(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx);

				void	UpdateSettings(void);

};



#endif // _WINDOWTAB_ST_SETTINGS_H_





