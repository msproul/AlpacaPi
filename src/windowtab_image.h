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

	kImageDisplay_HistRedPerct,
	kImageDisplay_HistGrnPerct,
	kImageDisplay_HistBluPerct,

	kImageDisplay_LumBar,
	kImageDisplay_RedBar,
	kImageDisplay_GrnBar,
	kImageDisplay_BluBar,

	kImageDisplay_Histogram,
	kImageDisplay_SaveHistBtn,
	kImageDisplay_HistogramOutline,

	//--------------------------------
	kImageDisplay_CrossHairTitle,
	kImageDisplay_CrossHairChkBox,
	kImageDisplay_CrossHair1Chk,
	kImageDisplay_CrossHair1X,
	kImageDisplay_CrossHair1Y,

	kImageDisplay_CrossHair2Chk,
	kImageDisplay_CrossHair2X,
	kImageDisplay_CrossHair2Y,

	kImageDisplay_CrossHair3Chk,
	kImageDisplay_CrossHair3X,
	kImageDisplay_CrossHair3Y,
	kImageDisplay_Reload,
	kImageDisplay_Clear,
	kImageDisplay_Save,

	//*	color scheme radio buttons
	kImageDisplay_RGB,	//*	Red, Green, Blue
	kImageDisplay_CMY,	//*	Cyan, Magenta, Yellow
	kImageDisplay_OHS,	//*	Oiii, HA, Sii
	kImageDisplay_PGR,	//*	Purple, Gold, Red
	kImageDisplay_BPO,	//*	Brown, Pink, Orange
	kImageDisplay_GRY,	//*	gray scale

	kImageDisplay_CrossHairOutline,

	kImageDisplay_ImageDisplayInfo,
	kImageDisplay_ImageDisplay,
	kImageDisplay_ImageCrossHair,

	kImageDisplay_last
};

#define	kCrossHairCnt	3
//**************************************************************************************
typedef struct
{
	int	XLocation;
	int	YLocation;
} TYPE_CROSSHAIR;

//**************************************************************************************
class WindowTabImage: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabImage(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabImage(void);

		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawWindow(void);
		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
//		virtual void	ProcessMouseEvent(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonDown(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonUp(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonDragged(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		virtual	void	DrawWidgetCustomGraphic(cv::Mat *openCV_Image, TYPE_WIDGET *theWidget, const int widgetIdx);
#else
		virtual	void	DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx);
#endif // _USE_OPENCV_CPP_

				void	UpdateButtons(void);

				void	ResetImage(void);
				void	ZoomImage(	const int	event,
									const int	xxx,
									const int	yyy,
									const int	flags);
				void	DrawFullScaleIamge(void);
				void	DrawFullScaleIamge(const int image_X, const int	image_Y);
				void	DrawCrossHairs(TYPE_WIDGET *theWidget);

				void	SaveHistogram(void);
				void	SaveCrossHairList(void);
				void	ReadCrossHairList(void);

			#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
				void		SetImagePtrs(cv::Mat *originalImage, cv::Mat *displayedImage);
				cv::Mat		*cOpenCVdownLoadedImage;
				cv::Mat		*cOpenCVdisplayedImage;
			#else
				void		SetImagePtrs(IplImage *originalImage, IplImage *displayedImage);
				IplImage	*cOpenCVdownLoadedImage;
				IplImage	*cOpenCVdisplayedImage;
			#endif // _USE_OPENCV_CPP_

				int			cImageCenterX;
				int			cImageCenterY;

				int			cSavedMouseClick_X;	//*	these are for left mouse drag operation
				int			cSavedMouseClick_Y;
				bool		cMouseDragInProgress;

				//*	these are for zooming in and scrolling
				int			cImageZoomState;

				//*	cross hair information
				bool		cDisplayCrossHair;
				//*	these are relative to the image box, not the image
				int			cCurrCrossHairNum;
				int			cCurrCrossHairColorSchm;
				TYPE_CROSSHAIR	cCrossHairPos[kCrossHairCnt];

};


#endif // _WINDOWTAB_IMAGE_H_

