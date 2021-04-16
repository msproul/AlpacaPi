//*****************************************************************************
//#include	"windowtab_image.h"



#ifndef	_WINDOWTAB_IMAGE_H_
#define	_WINDOWTAB_IMAGE_H_

#ifndef	_WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _CONTROLLER_H_

//*****************************************************************************
enum
{
	kImageDisplay_Title	=	0,

	kImageDisplay_Logo,



	kImageDisplay_Btn_1,
	kImageDisplay_Btn_2,
	kImageDisplay_Btn_3,
	kImageDisplay_Btn_4,
	kImageDisplay_Btn_5,
	kImageDisplay_Btn_6,

	kImageDisplay_Btn_N,

	kImageDisplay_FrameCnt_Lbl,
	kImageDisplay_FrameCnt,

	kImageDisplay_Exposure_Lbl,
	kImageDisplay_Exposure,
	kImageDisplay_Gain_Lbl,
	kImageDisplay_Gain,
	kImageDisplay_CameraTemp_Lbl,
	kImageDisplay_CameraTemp,
	kImageDisplay_Filter_Lbl,
	kImageDisplay_Filter,
	kImageDisplay_Object_Lbl,
	kImageDisplay_Object,
	kImageDisplay_FramesSaved_Lbl,
	kImageDisplay_FramesSaved,



	kImageDisplay_Histogram,

	kImageDisplay_ImageDisplay,

	kImageDisplay_last
};


//**************************************************************************************
class WindowTabImage: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabImage(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabImage(void);

//		virtual	void	DrawGraphWidget(const int widgetIdx);
		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawWindow(void);
		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
//		virtual void	ProcessMouseEvent(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonDown(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonUp(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonDragged(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);

				void	SetImagePtrs(IplImage *originalImage, IplImage *displayedImage);

				void	ResetImage(void);
				void	ZoomImage(	const int	event,
									const int	xxx,
									const int	yyy,
									const int	flags);
				void	DrawFullScaleIamge(void);
				void	DrawFullScaleIamge(const int image_X, const int	image_Y);

				IplImage	*cOpenCVdownLoadedImage;
				IplImage	*cOpenCVdisplayedImage;

				int			cImageCenterX;
				int			cImageCenterY;

				int			cSavedMouseClick_X;	//*	these are for left mouse drag operation
				int			cSavedMouseClick_Y;
				bool		cMouseDragInProgress;

				//*	these are for zooming in and scrolling
				int			cImageZoomState;


};


#endif // _WINDOWTAB_IMAGE_H_

