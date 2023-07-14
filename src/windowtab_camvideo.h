//*****************************************************************************
//#include	"windowtab_camvideo.h"



#ifndef	_WINDOWTAB_CAMVIDEO_H_
#define	_WINDOWTAB_CAMVIDEO_H_

#ifndef	_CONTROLLER_H_
//	#include	"controller.h"
#endif

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kCamVideo_Title	=	0,

	kCamVideo_Connected,

	kCamVideo_StartVideoBtn,
	kCamVideo_RecordingStatus,
	kCamVideo_StopVideoBtn,


	kCamVideo_IncludeTimeStamp,
	kCamVideo_LastChkBox,

	kCamVideo_LastCmdString,
	kCamVideo_AlpacaLogo,

	kCamVideo_AlpacaErrorMsg,
	kCamVideo_IPaddr,
	kCamVideo_Readall,
	kCamVideo_DeviceSelect,

	kCamVideo_last
};

#define	kMaxSwitchControlBoxes	12

//**************************************************************************************
class WindowTabCamVideo: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabCamVideo(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabCamVideo(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ActivateWindow(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);

				void	StartVideo(void);
				void	StopVideo(void);


};


#endif // _WINDOWTAB_CAMVIDEO_H_
