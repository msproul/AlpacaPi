//*****************************************************************************
//#include	"windowtab_camsettings.h"



#ifndef	_WINDOWTAB_CAMSETTINGS_H_
#define	_WINDOWTAB_CAMSETTINGS_H_

#ifndef	_CONTROLLER_H_
//	#include	"controller.h"
#endif // _CONTROLLER_H_

#ifndef _WINDOW_TAB_H
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H



//*****************************************************************************
enum
{
	kCamSet_Title	=	0,
	kCamSet_Logo,

	kCamSet_P_Title,
	kCamSet_Preset01,
	kCamSet_Preset02,
	kCamSet_Preset03,
	kCamSet_Preset04,
	kCamSet_Preset05,
	kCamSet_Preset06,
	kCamSet_Preset07,
	kCamSet_Preset08,
	kCamSet_Preset09,
	kCamSet_Preset10,
	kCamSet_Preset11,
	kCamSet_Preset12,
	kCamSet_Preset13,
	kCamSet_Preset14,
	kCamSet_Preset15,
	kCamSet_Preset16,
	kCamSet_Preset17,

	kCamSet_ObjOutline,

	kCamSet_T_Title,
	kCamSet_Time01,
	kCamSet_Time02,
	kCamSet_Time03,
	kCamSet_Time04,
	kCamSet_Time05,
	kCamSet_Time06,
	kCamSet_Time07,
	kCamSet_Time08,
	kCamSet_Time09,
	kCamSet_Time10,
	kCamSet_Time11,
	kCamSet_Time12,
	kCamSet_Time13,
	kCamSet_Time14,
	kCamSet_Time15,
	kCamSet_Time16,
	kCamSet_Time17,

	kCamSet_TimeOutline,


	kCamSet_FilenameTitle,
	kCamSet_FN_IncFilter,
	kCamSet_FN_IncCamera,
	kCamSet_FN_IncSerialNum,
	kCamSet_FN_IncRefID,

	kCamSet_FilenameOutline,

	kCamSet_LastCmdString,


	kCamSet_AlpacaLogo,
	kCamSet_AlpacaDrvrVersion,
	kCamSet_IPaddr,
	kCamSet_Readall,

	kCamSet_last
};

//**************************************************************************************
class WindowTabCamSettings: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabCamSettings(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName,
										const char	*deviceName,
										const bool	hasFilterWheel = false);
		virtual	~WindowTabCamSettings(void);

		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);

				void	UpdateSettings_Object(const char *filePrefix);
				void	SetObjectText(const int presetIndex);
				void	SetExposureRange(const int expTimeIdx);
				void	SetFileNameOptions(const int fnOptionBtn);

};


#endif // _WINDOWTAB_CAMSETTINGS_H_


