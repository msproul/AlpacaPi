//*****************************************************************************
//#include	"windowtab_slitgraph.h"



#ifndef	_WINDOWTAB_SLIT_GRAPH_H_
#define	_WINDOWTAB_SLIT_GRAPH_H_

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kSlitGraph_Title	=	0,

	kSlitGraph_Graph,

	kSlitGraph_Clock00,		//*	00 is 12 o'clock
	kSlitGraph_Clock01,
	kSlitGraph_Clock02,
	kSlitGraph_Clock03,
	kSlitGraph_Clock04,
	kSlitGraph_Clock05,
	kSlitGraph_Clock06,
	kSlitGraph_Clock07,
	kSlitGraph_Clock08,
	kSlitGraph_Clock09,
	kSlitGraph_Clock10,
	kSlitGraph_Clock11,

	kSlitGraph_DisplayRaw,
	kSlitGraph_DisplayAvg,

	kSlitGraph_LastCmdString,
	kSlitGraph_AlpacaLogo,
	kSlitGraph_AlpacaErrorMsg,
	kSlitGraph_IPaddr,
	kSlitGraph_Readall,


	kSlitGraph_last
};

#ifndef kSensorValueCnt
	#define		kSensorValueCnt	12
#endif // kSensorValueCnt


//**************************************************************************************
class WindowTabSlitGraph: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabSlitGraph(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabSlitGraph(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
#ifdef _USE_OPENCV_CPP_
		virtual void	DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx);
#else
		virtual	void	DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx);
#endif // _USE_OPENCV_CPP_

				void	DrawSlitGraph(TYPE_WIDGET *theWidget);
		//		void	DrawTickLine(CvRect *widgetRect, int yLoc);
				void	DrawTickLine(cv::Rect *widgetRect, int yLoc);

				bool		cDisplayClockData[kSensorValueCnt];
				cv::Scalar	cSLitTrackColors[kSensorValueCnt];

				bool		cDisplayRawData;
				bool		cDisplayAvgData;
};


#endif // _WINDOWTAB_SLIT_GRAPH_H_

