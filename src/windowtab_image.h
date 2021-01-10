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

//		virtual	void	DrawGraphWidget(const int widgitIdx);
		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawWindow(void);
//		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx);
//		virtual	void	ProcessDoubleClick(const int buttonIdx);

				IplImage	*cOpenCVdownLoadedImage;


};


#endif // _WINDOWTAB_IMAGE_H_

