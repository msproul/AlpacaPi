//*****************************************************************************
//#include	"windowtab_usb.h"



#ifndef	_WINDOWTAB_USB_H_
#define	_WINDOWTAB_USB_H_

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif // _CONTROLLER_H_

//*****************************************************************************
enum
{
	kUSBselect_Title	=	0,


	kUSBselect_Instructions,

	kUSBselect_BtnNiteCrawler,
	kUSBselect_BtnSingle,
//	kUSBselect_BtnCrawler,

	kUSBselect_AlpacaDrvrVersion,
	kUSBselect_IPaddr,



	kUSBselect_last
};

#define	kUSBselect_LastOption	kUSBselect_BtnSingle

//**************************************************************************************
class WindowTabUSB: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabUSB(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabUSB(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx);

};


#endif // _WINDOWTAB_ABOUT_H_
