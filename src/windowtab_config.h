//**************************************************************************************
//#include	"windowtab_config.h"

#ifndef _WINDOWTAB_CONFIG_H_
#define	_WINDOWTAB_CONFIG_H_
#include	"windowtab.h"



//*****************************************************************************
enum
{
	kCongfigBox_Config1	=	0,
	kCongfigBox_Config2,
	kCongfigBox_Config3,

	kCongfigBox_Color01,
	kCongfigBox_Color02,
	kCongfigBox_Color03,
	kCongfigBox_Color04,
	kCongfigBox_Color05,
	kCongfigBox_Color06,
	kCongfigBox_Color07,
	kCongfigBox_Color08,
	kCongfigBox_Color09,
	kCongfigBox_Color10,
	kCongfigBox_Color11,
	kCongfigBox_Color12,
	kCongfigBox_Color13,
	kCongfigBox_Color14,

	kCongfigBox_Text01,
	kCongfigBox_Text02,
	kCongfigBox_Text03,
	kCongfigBox_Text04,
	kCongfigBox_Text05,
	kCongfigBox_Text06,
	kCongfigBox_Text07,
	kCongfigBox_Text08,
	kCongfigBox_Text09,
	kCongfigBox_Text10,
	kCongfigBox_Text11,
	kCongfigBox_Text12,
	kCongfigBox_Text13,
	kCongfigBox_Text14,



	kCongfigBox_AlpacaLogo,
	kCongfigBox_AlpacaDrvrVersion,
	kCongfigBox_IPaddr,
	kCongfigBox_Readall,

	kCongfigBox_last
};

//**************************************************************************************
class WindowTabConfig: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabConfig(const int xSize, const int ySize, CvScalar backGrndColor);
		virtual	~WindowTabConfig(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx);


};



#endif // _WINDOWTAB_CONFIG_H_
