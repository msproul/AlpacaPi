//*****************************************************************************
//#include	"windowtab_multicam.h"

#ifndef	_WINDOWTAB_MULTICAM_H_
#define	_WINDOWTAB_MULTICAM_H_

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif


//*****************************************************************************
enum
{
	kMultiCam_Title	=	0,

	kMultiCam_Connected,

	kMultiCam_CameraList,

	kMultiCam_StartExpBtn,


	kMultiCam_LastCmdString,

	kMultiCam_AlpacaLogo,
	kMultiCam_AlpacaErrorMsg,
	kMultiCam_IPaddr,
	kMultiCam_Readall,
	kMultiCam_DeviceState,

	kMultiCam_last
};


//**************************************************************************************
class WindowTabMulticam: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabMulticam(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName,
									const bool	parentIsSkyTravel = false);
		virtual	~WindowTabMulticam(void);

		virtual	void	SetupWindowControls(void);

		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
};


#endif // _WINDOWTAB_MULTICAM_H_
