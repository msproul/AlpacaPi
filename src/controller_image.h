//*****************************************************************************
//#include	"controller_image.h"


#include	"controller.h"

#ifndef	_WINDOWTAB_IMAGE_H_
	#include	"windowtab_image.h"
#endif

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif


//**************************************************************************************
class ControllerImage: public Controller
{
	public:
		//
		// Construction
		//
			#ifdef _USE_OPENCV_CPP_
				ControllerImage(const char	*argWindowName,
								cv::Mat		*downloadedImage,
								TYPE_BinaryImageHdr	*binaryImageHdr = NULL);
			#else
				ControllerImage(const char	*argWindowName,
								IplImage	*downloadedImage,
								TYPE_BinaryImageHdr	*binaryImageHdr = NULL);
			#endif

		virtual	~ControllerImage(void);


		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);
//		virtual	void	AlpacaProcessReadAll(const char *deviceType, const char *keywordString, const char *valueString);
		virtual	void	RefreshWindow(void);

		virtual	void	DrawWidgetImage(TYPE_WIDGET *theWidget);


				void	UpdateLiveWindowInfo(	TYPE_CameraProperties	*cameraProp,
												const int				framesRead,
												const double			exposure_Secs,
												const char				*filterName,
												const char				*objectName
												);
			#ifdef _USE_OPENCV_CPP_
				void	SetLiveWindowImage(		cv::Mat *newOpenCVImage);
				void	UpdateLiveWindowImage(	cv::Mat *newOpenCVImage, const char *imageFileName=NULL);
				void	CopyImageToLiveImage(	cv::Mat *newOpenCVImage);

				cv::Mat *cDownLoadedImage;
				cv::Mat *cDisplayedImage;
				cv::Mat *cColorImage;
			#else
				void	SetLiveWindowImage(		IplImage *newOpenCVImage);
				void	UpdateLiveWindowImage(	IplImage *newOpenCVImage, const char *imageFileName=NULL);
				void	CopyImageToLiveImage(	IplImage *newOpenCVImage);

				IplImage *cDownLoadedImage;
				IplImage *cDisplayedImage;
				IplImage *cColorImage;
			#endif // _USE_OPENCV_CPP_
				void	SetImageWindowInfo(void);

				TYPE_BinaryImageHdr		cBinaryImageHdr;

				//*	tab information
				WindowTabImage			*cImageTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

				//----------------------------------------------------
				//*	image information
				TYPE_CameraProperties	cCameraProp;
//				TYPE_IMAGE_ROI_Info		cROIinfo;


};



