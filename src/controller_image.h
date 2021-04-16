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
				ControllerImage(const char	*argWindowName, IplImage *downloadedImage);


		virtual	~ControllerImage(void);


		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(void);
//		virtual	void	AlpacaProcessReadAll(const char *deviceType, const char *keywordString, const char *valueString);
		virtual	void	RefreshWindow(void);

		virtual	void	DrawWidgetImage(TYPE_WIDGET *theWidget);

				void	SetLiveWindowImage(IplImage *newOpenCVImage);
				void	UpdateLiveWindowImage(IplImage *newOpenCVImage, const char *imageFileName=NULL);
				void	UpdateLiveWindowInfo(	TYPE_CameraProperties	*cameraProp,
												const int				framesRead,
												const double			exposure_Secs,
												const char				*filterName,
												const char				*objectName
												);
				void	CopyImageToLiveImage(IplImage *newOpenCVImage);

				IplImage *cDownLoadedImage;
				IplImage *cDisplayedImage;
				IplImage *cColorImage;

		//*	tab information
				WindowTabImage		*cImageTabObjPtr;
				WindowTabAbout		*cAboutBoxTabObjPtr;
};



