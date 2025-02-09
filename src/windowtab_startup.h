//*****************************************************************************
//#include	"windowtab_startup.h"

#ifndef	_WINDOWTAB_STARTUP_H_
#define	_WINDOWTAB_STARTUP_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kStartup_Title1	=	0,
	kStartup_Title2,
	kStartup_TitleOutline,

	kStartup_TextBox1,
	kStartup_TextBoxN = 23,
	kStartup_CloseBtn,

	kStartup_DumpBtn,

	kStartup_AlpacaLogo,
	kStartup_last
};


//**************************************************************************************
class WindowTabStartup: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabStartup(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabStartup(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int	flags);

};



#endif // _WINDOWTAB_STARTUP_H_




