//*****************************************************************************
//#include	"windowtab_about.h"

#ifndef	_WINDOWTAB_ABOUT_H_
#define	_WINDOWTAB_ABOUT_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kAboutBox_Title	=	0,

	kAboutBox_ControllerVersion,

	kAboutBox_TextBox1,
	kAboutBox_TextBox2,
	kAboutBox_TextBox3,

	kAboutBox_CPUinfo,

	kAboutBox_AlpacaLogo,


	kAboutBox_last
};


//**************************************************************************************
class WindowTabAbout: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabAbout(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabAbout(void);

		virtual	void	SetupWindowControls(void);

};



#endif // _WINDOWTAB_ABOUT_H_




