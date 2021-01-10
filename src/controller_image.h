//*****************************************************************************
//#include	"controller_image.h"


#include	"controller.h"

#ifndef	_WINDOWTAB_IMAGE_H_
	#include	"windowtab_image.h"
#endif

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_


#ifdef _ENABLE_SKYTRAVEL_
	#ifndef	_WINDOWTAB_SKYTRAVEL_H_
		#include	"windowtab_skytravel.h"
	#endif // _WINDOWTAB_SKYTRAVEL_H_
#endif // _ENABLE_SKYTRAVEL_

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


				IplImage *cDownLoadedImage;
				IplImage *cDisplayedImage;

		//*	tab information
				WindowTabImage		*cImageTabObjPtr;
#ifdef _ENABLE_SKYTRAVEL_
				WindowTabSkyTravel	*cSkyTravelTabOjbPtr;
#endif
				WindowTabAbout		*cAboutBoxTabObjPtr;
};



