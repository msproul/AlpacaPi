//*****************************************************************************
//*	Mar  9,	2024	<MLS> Created controller_skyimage.h
//*****************************************************************************
//#include	"controller_skyimage.h"

#ifndef _CONTROLLER_SKYIMAGE_H_
#define _CONTROLLER_SKYIMAGE_H_



#ifndef	_WINDOWTAB_IMAGELIST_H_
	#include	"windowtab_imageList.h"
#endif

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif


#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif
#include	"controller_image.h"


//**************************************************************************************
enum
{
	kTab_SI_ImgList	=	1,
	kTab_SI_Image,
	kTab_SI_About,

	kTab_SI_Count

};

//**************************************************************************************
class ControllerSkyImage: public Controller
{
	public:
		//
		// Construction
		//
				ControllerSkyImage(const char	*argWindowName);


		virtual	~ControllerSkyImage(void);


		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);
		//====================================================
		//*	tab information

			WindowTabImageList		*cImageListTabObjPtr;
			WindowTabAbout			*cAboutBoxTabObjPtr;

			int						cFitsProcessCntr;
};

extern	int	gCurrentImageIndex;
void	LoadNextImageFromList(ControllerImage *imageController);
void	LoadPreviousImageFromList(ControllerImage *imageController);


#endif // _CONTROLLER_SKYIMAGE_H_
