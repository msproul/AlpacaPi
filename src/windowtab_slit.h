//*****************************************************************************
//#include	"windowtab_slit.h"



#ifndef	_WINDOWTAB_SLIT_H_
#define	_WINDOWTAB_SLIT_H_

#ifndef	_WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kSlitTracker_Title	=	0,
	kSlitTracker_Connected,

	kSlitTracker_SlitClockFace,
	kSlitTracker_LastUpdate,

	kSlitTracker_RadioBtnSlit,
	kSlitTracker_RadioBtnCalib,
	kSlitTracker_RadioBtnOutline,

	kSlitTracker_LogDataCheckBox,

	kSlitTracker_RemoteAddress,

	kSlitTracker_LastCmdString,
	kSlitTracker_AlpacaLogo,
	kSlitTracker_AlpacaErrorMsg,
	kSlitTracker_IPaddr,
	kSlitTracker_Readall,


	kSlitTracker_last
};

//**************************************************************************************
enum
{
	kClockDisplay_Slit	=	0,
	kClockDisplay_Calib
};

#define	kMaxDotColors	6
//**************************************************************************************
class WindowTabSlitTracker: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabSlitTracker(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabSlitTracker(void);

		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
#ifdef _USE_OPENCV_CPP_
		virtual	void	DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx);
#else
		virtual	void	DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx);
#endif // _USE_OPENCV_CPP_

				void	DrawClockFace(TYPE_WIDGET *theWidget);
				void	UpdateClockRadioBtns(void);
				void	ToggleLogData(void);


				int		cClockDisplayMode;

				int			cUpdateColorIdx;
				cv::Scalar	cDotColor[kMaxDotColors];

};


#endif // _WINDOWTAB_SLIT_H_
