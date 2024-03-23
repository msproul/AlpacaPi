//*****************************************************************************
//#include	"controller_image.h"

#ifndef _CONTROLLER_IMAGE_H_
#define _CONTROLLER_IMAGE_H_

#include	"controller.h"

#ifndef	_WINDOWTAB_IMAGE_H_
	#include	"windowtab_image.h"
#endif
#ifndef	_WINDOWTAB_IMAGEINFO_H_
	#include	"windowtab_imageinfo.h"
#endif

#ifndef	_WINDOWTAB_FITSHEADER_H_
	#include	"windowtab_fitsheader.h"
#endif

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef _FITS_HELPER_H_
	#include	"fits_helper.h"
#endif

//**************************************************************************************
typedef struct
{
	double	ApertureDiam;
	char	Camera[64];
	char	Date[64];
	char	Filter[64];
	double	FocalLength;
	double	FocalRatio;
	char	Location[64];
	char	MoonPhase[64];
	char	Object[64];
	char	Observer[64];
	char	Observatory[64];
	char	Time_Local[64];
	char	Time_UTC[64];
	char	Telescope[64];
	char	Website[128];

} TYPE_FitsHeaderData;

//**************************************************************************************
class ControllerImage: public Controller
{
	public:
		//
		// Construction
		//
			#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
				ControllerImage(const char			*argWindowName,
								cv::Mat				*downloadedImage,
								TYPE_BinaryImageHdr	*binaryImageHdr);
			#else
				ControllerImage(const char			*argWindowName,
								IplImage			*downloadedImage,
								TYPE_BinaryImageHdr	*binaryImageHdr);
			#endif
				ControllerImage(const char			*argWindowName,
								const char			*imageFilePath);

		virtual	~ControllerImage(void);

				void	InitClassVariables(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);
		virtual	void	RefreshWindow(void);

		virtual	void	DrawWidgetImage(TYPE_WIDGET *theWidget);
				void	SetImageWindowTitles(const char *imageFilePath);
				void	LoadImage(const char *imageFilePath);
				void	LoadNextImage(void);


				void	UpdateLiveWindowInfo(	TYPE_CameraProperties	*cameraProp,
												const int				framesRead,
												const double			exposure_Secs,
												const char				*filterName,
												const char				*objectName
												);
			#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
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
				void	SetDownloadInfo(double download_MBytes, double download_seconds);
				void	ProcessFitsHeader(const char *imageFilePath);
				void	DrawTextString2(const int xxLoc1, const int xxLoc2, const int yyLoc, const char *textString1, const char *textString2);
				void	DrawTitleBlock(void);
				void	SaveImage(void);

				char					cImageFileName[128];
				char					cImageFileNameRoot[128];
				TYPE_BinaryImageHdr		cBinaryImageHdr;

				//*	tab information
				WindowTabImage			*cImageTabObjPtr;
				WindowTabImageInfo		*cImageInfoTabObjcPtr;
				WindowTabFITSheader		*cFitsHeaderTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

				//=======================================================
				//*	image information
				TYPE_CameraProperties	cCameraProp;
				bool					cImageFromDisk;
				bool					cImageIsFITS;
				bool					cImageIsPDS;	//*	Planetary Data System
				char					cFileExtension[16];
//				TYPE_IMAGE_ROI_Info		cROIinfo;
				TYPE_FitsHeaderData		cFitsHeaderData;

				//=======================================================
				//*	image analysis data
				//*	these are the same names as in cameradriver.h
				void		CalculateHistogramArray(void);
				void		SaveHistogram(void);

				int32_t		cHistogramLum[256];
				int32_t		cHistogramRed[256];
				int32_t		cHistogramGrn[256];
				int32_t		cHistogramBlu[256];
				int32_t		cMinHistogramValue;
				int32_t		cMaxHistogramValue;
				int32_t		cPeakHistogramValue;
				int32_t		cMaxHistogramPixCnt;

				uint8_t		cMaxRedValue;
				uint8_t		cMaxGrnValue;
				uint8_t		cMaxBluValue;
				uint8_t		cMaxGryValue;

				//=======================================================
				TYPE_FitsHdrLine	cFitsHeaderText[kMaxFitsHdrLines];
				int					cFitsHdrCnt;

};

#endif // _CONTROLLER_IMAGE_H_


