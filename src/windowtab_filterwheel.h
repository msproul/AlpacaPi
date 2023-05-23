//*****************************************************************************
//#include	"windowtab_filterwheel.h"



#ifndef	_WINDOWTAB_FILTERWHEEL_H_
#define	_WINDOWTAB_FILTERWHEEL_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kFilterWheel_Title	=	0,
	kFilterWheel_Connected,

	kFilterWheel_Name,
	kFilterWheel_FilterCircle,

	kFilterWheel_LastCmdString,
	kFilterWheel_AlpacaLogo,
	kFilterWheel_AlpacaErrorMsg,
	kFilterWheel_IPaddr,
	kFilterWheel_Readall,


	kFilterWheel_last

};

#ifndef kSensorValueCnt
	#define		kSensorValueCnt	12
#endif // kSensorValueCnt

//**************************************************************************************
typedef struct
{
	int		x;
	int		y;
} Point;


//**************************************************************************************
class WindowTabFilterWheel: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabFilterWheel(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabFilterWheel(void);

		virtual	void	SetupWindowControls(void);
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		virtual	void	DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx);
#else
		virtual	void	DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx);
#endif // _USE_OPENCV_CPP_
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);

				void	DrawFilterWheel(TYPE_WIDGET *theWidget);
				void	SetFilterWheelPropPtr(TYPE_FilterWheelProperties *fwProperties);
				void	SetPositonCount(int positionCount);

				int		cPositionCount;
				Point	cFilterCirleCenterPt[kMaxFiltersPerWheel];

				TYPE_FilterWheelProperties	*cFilterWheelPropPtr;
};


#endif // _WINDOWTAB_FILTERWHEEL_H_

