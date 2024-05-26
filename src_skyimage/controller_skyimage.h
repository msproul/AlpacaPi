//*****************************************************************************
//*	Mar  9,	2024	<MLS> Created controller_skyimage.h
//*****************************************************************************
//#include	"controller_skyimage.h"

#ifndef _CONTROLLER_SKYIMAGE_H_
#define _CONTROLLER_SKYIMAGE_H_



#ifndef	_WINDOWTAB_IMAGELIST_H_
	#include	"windowtab_imageList.h"
#endif

#ifndef	_WINDOWTAB_MOONPHASE_H_
	#include	"windowtab_MoonPhase.h"
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
//	kTab_SI_Image,
	kTab_MoonPhase,
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
				ControllerSkyImage(const char	*argWindowName, const char *argDirectoryPath);


		virtual	~ControllerSkyImage(void);


		virtual	void	SetupWindowControls(void);
		virtual	bool	RunFastBackgroundTasks(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);
		void			ReadFileDirectory(const char *directoryPath);
		int				BuildFileList(const char *directoryPath);

//		bool			LoadNextImageFromList(void);
//		bool			LoadPreviousImageFromList(void);

		//====================================================
		//*	tab information

			WindowTabImageList		*cImageListTabObjPtr;
			WindowTabMoonPhase		*cMoonPhaseTabObjPtr;
			WindowTabAbout			*cAboutBoxTabObjPtr;

			char					cDirectoryPath[256];
			int						cDirectoryPathLen;
			int						cFitsProcessCntr;
			int						cFileIndex;
			int						cBackGroundImgIdx;
};

extern	int	gCurrentImageIndex;
extern	int	gImageCount;

bool	LoadNextImageFromList(ControllerImage *imageController);
bool	LoadPreviousImageFromList(ControllerImage *imageController);


#endif // _CONTROLLER_SKYIMAGE_H_
