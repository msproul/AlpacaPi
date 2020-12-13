//*****************************************************************************
//#include	"windowtab_about.h"

#ifndef	_WINDOWTAB_ABOUT_H_
#define	_WINDOWTAB_ABOUT_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H

//*****************************************************************************
enum
{
	kAboutBox_Title	=	0,

	kAboutBox_ControllerVersion,

	kAboutBox_TextBox1,
	kAboutBox_TextBox2,
	kAboutBox_TextBox3,

	kAboutBox_AlpacaLogo,
	kAboutBox_AlpacaDrvrVersion,
	kAboutBox_IPaddr,
	kAboutBox_Readall,


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
								CvScalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabAbout(void);

		virtual	void	SetupWindowControls(void);

};


#endif // _WINDOWTAB_ABOUT_H_
