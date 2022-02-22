//*****************************************************************************
//#include	"windowtab_capabilities.h"

#ifndef	_WINDOWTAB_CAPABILITIES_H_
#define	_WINDOWTAB_CAPABILITIES_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kCapabilities_Title	=	0,


	kCapabilities_TextBox1,
	kCapabilities_TextBoxN = 22,
	kCapabilities_AlpacaLogo,
	kCapabilities_last
};


//**************************************************************************************
class WindowTabCapabilities: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabCapabilities(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabCapabilities(void);

		virtual	void	SetupWindowControls(void);

};



#endif // _WINDOWTAB_CAPABILITIES_H_




