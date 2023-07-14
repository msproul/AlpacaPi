//*****************************************************************************
//#include	"windowtab_deviceselect.h"

#ifndef	_WINDOWTAB_DEVICE_SELECT_H_
#define	_WINDOWTAB_DEVICE_SELECT_H_

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kDeviceSelect_Title	=	0,

//	kDeviceSelect_ControllerVersion,

	kDeviceSelect_TextBox1,

	kDeviceSelect_DomeTitle,
	kDeviceSelect_Dome1,
	kDeviceSelect_Dome2,
	kDeviceSelect_Dome3,
	kDeviceSelect_Dome4,
	kDeviceSelect_Dome5,
	kDeviceSelect_Dome6,
	kDeviceSelect_Dome7,
	kDeviceSelect_Dome8,
	kDeviceSelect_DomeOutline,

	kDeviceSelect_TelescopeTitle,
	kDeviceSelect_Telescope1,
	kDeviceSelect_Telescope2,
	kDeviceSelect_Telescope3,
	kDeviceSelect_Telescope4,
	kDeviceSelect_Telescope5,
	kDeviceSelect_Telescope6,
	kDeviceSelect_Telescope7,
	kDeviceSelect_Telescope8,
	kDeviceSelect_TelescopeOutline,


	kDeviceSelect_AlpacaLogo,

	kDeviceSelect_last
};

//*	this number must match the number of boxes described above
#define	kSelectionCnt	8
//**************************************************************************************
class WindowTabDeviceSelect: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabDeviceSelect(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabDeviceSelect(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);

				void	UpdateRemoteDeviceList(void);

				TYPE_REMOTE_DEV	cDeviceList_Domes[kSelectionCnt];
				int				cDomeCnt;
				int				cSelectedDomeIdx;

				TYPE_REMOTE_DEV	cDeviceList_Telescopes[kSelectionCnt];
				int				cTelescopeCnt;
				int				cSelectedTelescopeIdx;
};

#endif // _WINDOWTAB_DEVICE_SELECT_H_




