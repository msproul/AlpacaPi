//*****************************************************************************
//#include	"windowtab_mount.h"



#ifndef	_WINDOWTAB_MOUNT_H_
#define	_WINDOWTAB_MOUNT_H_

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kMount_Title	=	0,

	kMount_Graph,

	kMount_LegendTextBox,

	kMount_Reset,
	kMount_VertMag_Plus,
	kMount_VertMag_Minus,
	kMount_VertMag_Value,

	kMount_HorzMag_Plus,
	kMount_HorzMag_Minus,
	kMount_HorzMag_Value,
	kMount_Clear,
	kMount_ScaleInfoTextBox,

	kMount_HelpMsgTextBox,
	kMount_IPaddr,
	kMount_Readall,
	kMount_DeviceState,

	kMount_last
};

//**************************************************************************************
class WindowTabMount: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabMount(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabMount(void);

		virtual	void	SetupWindowControls(void);
		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
		virtual void	ProcessMouseWheelMoved(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	wheelMovement,
												const int	flags);
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		virtual void	DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx);
#else
		virtual	void	DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx);
#endif // _USE_OPENCV_CPP_

				void	DrawOneGraphSegment(TYPE_WIDGET		*theWidget,
												const double	*dataArray,
												const int		firstDataPtIdx,
												const char		*labelString);
				void	DrawWidgetCustomGraphicAvg(	TYPE_WIDGET		*theWidget,
												const double	*dataArray,
												const int		firstDataPtIdx,
												const int		numPtsToAverage);
				void	DrawRA_DEC_HA_Graph(TYPE_WIDGET *theWidget);
		//		void	DrawTickLine(CvRect *widgetRect, int yLoc);
				void	DrawTickLine(cv::Rect *widgetRect, int yLoc);
				void	UpdateButtons(void);
	private:
				int		cVerticalMagnification;
				int		cHorizontalMagnification;
				int		cGraphRectTop;
				int		cGraphRectBottom;
				int		cDefaultVertCenter;
				int		cCurrentVertCenter;

				int		cCurrentHorzOffset;

				bool	cDisplayAverage;
				double	cCurrentHAvalue;
};


#endif // _WINDOWTAB_MOUNT_H_

