//*****************************************************************************
//#include	"controller_usb.h"


#include	"controller.h"

#include	"windowtab_usb.h"
#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_


//**************************************************************************************
class ControllerUSB: public Controller
{
	public:
		//
		// Construction
		//
				ControllerUSB(	const char	*argWindowName,
								const char	*usbPath);


		virtual	~ControllerUSB(void);


		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);

				char				cUSBpath[48];

			//*	tab information
				WindowTabUSB		*cUSBTabObjPtr;
				WindowTabAbout		*cAboutBoxTabObjPtr;
};

