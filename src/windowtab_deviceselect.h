//*****************************************************************************
//#include	"windowtab_deviceselect.h"

#ifndef	_WINDOWTAB_DEVICE_SELECT_H_
#define	_WINDOWTAB_DEVICE_SELECT_H_



#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif // _DISCOVERY_LIB_H_

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif




#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H

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
//	kDeviceSelect_Dome5,
	kDeviceSelect_DomeOutline,

	kDeviceSelect_TelescopeTitle,
	kDeviceSelect_Telescope1,
	kDeviceSelect_Telescope2,
	kDeviceSelect_Telescope3,
	kDeviceSelect_Telescope4,
//	kDeviceSelect_Telescope5,
	kDeviceSelect_TelescopeOutline,


//	kDeviceSelect_AlpacaLogo,
//	kDeviceSelect_AlpacaDrvrVersion,
//	kDeviceSelect_IPaddr,
//	kDeviceSelect_Readall,


	kDeviceSelect_last
};


#define	kSelectionCnt	4
//**************************************************************************************
class WindowTabDeviceSelect: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabDeviceSelect(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabDeviceSelect(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	ProcessButtonClick(const int buttonIdx);

				void	UpdateRemoteDeviceList(void);

				TYPE_REMOTE_DEV	cDeviceList_Domes[kSelectionCnt];
				int				cDomeCnt;
				int				cSelectedDomeIdx;

				TYPE_REMOTE_DEV	cDeviceList_Telescopes[kSelectionCnt];
				int				cTelescopeCnt;
				int				cSelectedTelescopeIdx;
};

#endif // _WINDOWTAB_DEVICE_SELECT_H_




