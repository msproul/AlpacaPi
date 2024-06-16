//*****************************************************************************
//#include	"windowtab_STsettings.h"

#ifndef	_WINDOWTAB_ST_SETTINGS_H_
#define	_WINDOWTAB_ST_SETTINGS_H_




#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


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

	kSkyT_Settings_ElevLable,
	kSkyT_Settings_ElevValueMeters,
	kSkyT_Settings_ElevValueFeet,


	kSkyT_Settings_TimeZoneLbl,
	kSkyT_Settings_UTCoffset,
	kSkyT_Settings_TimeZoneTxt,

	kSkyT_Settings_DownLoadPathLbl,
	kSkyT_Settings_DownLoadPath,

	kSkyT_Settings_ObsSettingsBtn,
	kSkyT_Settings_ReloadSettingsBtn,

	kSkyT_Settings_SiteInfoOutline,

	kSkyT_Settings_EarthLable,
	kSkyT_Settings_EarthThin,
	kSkyT_Settings_EarthThick,
	kSkyT_Settings_EarthSolidBrn,
	kSkyT_Settings_EarthSolidGrn,

	kSkyT_Settings_DayNightSky,

	kSkyT_Settings_EarthOutline,

	kSkyT_Settings_GridLable,
	kSkyT_Settings_GridSolid,
	kSkyT_Settings_GridDashed,
	kSkyT_Settings_GridOutline,

	kSkyT_Settings_LineWidthTitle,

	kSkyT_Settings_LineW_MilkyWay,
	kSkyT_Settings_LineW_MilkyWay1,
	kSkyT_Settings_LineW_MilkyWay2,

	kSkyT_Settings_LineW_Const,
	kSkyT_Settings_LineW_Const1,
	kSkyT_Settings_LineW_Const2,

	kSkyT_Settings_LineW_ConstOutlines,
	kSkyT_Settings_LineW_ConstOutlines1,
	kSkyT_Settings_LineW_ConstOutlines2,

	kSkyT_Settings_LineW_NGCoutlines,
	kSkyT_Settings_LineW_NGCoutlines1,
	kSkyT_Settings_LineW_NGCoutlines2,

	kSkyT_Settings_LineW_Grid,
	kSkyT_Settings_LineW_Grid1,
	kSkyT_Settings_LineW_Grid2,
	kSkyT_Settings_LineW_BoxOutline,

	kSkyT_Settings_ResetToDefault,

	kSkyT_Settings_SaveSystemInfo,

	kSkyT_Settings_DataTitle,

	kSkyT_Settings_OriginalData_txt,
	kSkyT_Settings_OriginalData_cnt,

	kSkyT_Settings_DataAAVSO_txt,
	kSkyT_Settings_DataAAVSO_cnt,

	kSkyT_Settings_DataDRAPER_txt,
	kSkyT_Settings_DataDRAPER_cnt,

	kSkyT_Settings_DataHipparcos_txt,
	kSkyT_Settings_DataHipparcos_cnt,

	kSkyT_Settings_DataHYG_txt,
	kSkyT_Settings_DataHYG_cnt,

	kSkyT_Settings_DataMessier_txt,
	kSkyT_Settings_DataMessier_cnt,

	kSkyT_Settings_DataNGC_txt,
	kSkyT_Settings_DataNGC_cnt,

	kSkyT_Settings_DataSAO_txt,
	kSkyT_Settings_DataSAO_cnt,

	kSkyT_Settings_DataYALE_txt,
	kSkyT_Settings_DataYALE_cnt,

	kSkyT_Settings_Constellations_txt,
	kSkyT_Settings_Constellations_cnt,

	kSkyT_Settings_ConstOutLines_txt,
	kSkyT_Settings_ConstOutLines_cnt,

	kSkyT_Settings_Asteroids_txt,
	kSkyT_Settings_Asteroids_cnt,

	kSkyT_Settings_Special_txt,
	kSkyT_Settings_Special_cnt,

	kSkyT_Settings_DataHelpText,

	kSkyT_Settings_DataOutLine,

	//*	star settings
	kSkyT_Settings_OBAFGKM,
	kSkyT_Settings_DispMag,
	kSkyT_Settings_DispSpectral,
	kSkyT_Settings_DispParallax,
	kSkyT_Settings_DispProperMotion,
	kSkyT_Settings_DispPMvector,

	kSkyT_Settings_DispDynMagnitude,
	kSkyT_Settings_DispSpecifiedMagnitude,
	kSkyT_Settings_DispAllMagnitude,

	kSkyT_Settings_MagnitudeLimit,
	kSkyT_Settings_MagUpArrow,
	kSkyT_Settings_MagDownArrow,

	kSkyT_Settings_StarOutLine,

	kSkyT_Settings_RunStartup,
	kSkyT_Settings_RunStartup_Edit,
	kSkyT_Settings_RunShutdown,
	kSkyT_Settings_RunShutdown_Edit,


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
				WindowTabSTsettings(const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName);
		virtual	~WindowTabSTsettings(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ActivateWindow(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		virtual	void	DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx);
				void	DrawOBAFGKM(	cv::Mat *openCV_Image, TYPE_WIDGET *theWidget);
#else
		virtual	void	DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx);
				void	DrawOBAFGKM(IplImage *openCV_Image, TYPE_WIDGET *theWidget);
#endif // _USE_OPENCV_CPP_
				void	UpdateSettings(void);
				void	UpdateObservatorySettings(void);

};



#endif // _WINDOWTAB_ST_SETTINGS_H_





