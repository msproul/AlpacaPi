//*****************************************************************************
//#include	"windowtab_slitgraph.h"



#ifndef	_WINDOWTAB_SLIT_GRAPH_H_
#define	_WINDOWTAB_SLIT_GRAPH_H_


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

//	kSlitGraph_RemoteAddress,


	kSlitGraph_AlpacaLogo,
	kSlitGraph_AlpacaDrvrVersion,
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
										CvScalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabSlitGraph(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgitIdx);
		virtual	void	ProcessButtonClick(const int buttonIdx);

				void	DrawSlitGraph(IplImage *openCV_Image, TYPE_WIDGET *theWidget);
				void	DrawTickLine(IplImage *openCV_Image, CvRect *widgetRect, int yLoc);

				bool		cDisplayClockData[kSensorValueCnt];
				CvScalar	cSLitTrackColors[kSensorValueCnt];

				bool		cDisplayRawData;
				bool		cDisplayAvgData;
};


#endif // _WINDOWTAB_SLIT_GRAPH_H_

