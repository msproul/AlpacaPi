//*****************************************************************************
//#include	"windowtab_DeviceState.h"



#ifndef	_WINDOWTAB_DEVICESTATE_H_
#define	_WINDOWTAB_DEVICESTATE_H_

#ifndef	_CONTROLLER_H_
//	#include	"controller.h"
#endif

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kDeviceState_Title	=	0,

//	kDeviceState_Connected,
	kDeviceState_Stats,

	kDeviceState_FirstBoxName,
	kDeviceState_FirstBoxValue,

	kDeviceState_LastBoxName	=	(kDeviceState_FirstBoxName + (18 * 2)),
	kDeviceState_LastBoxValue,


//	kDeviceState_AlpacaLogo,
	kDeviceState_IPaddr,
	kDeviceState_Readall,
	kDeviceState_DeviceState,

	kDeviceState_last
};


//**************************************************************************************
class WindowTabDeviceState: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabDeviceState(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName);
		virtual	~WindowTabDeviceState(void);

		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
				void	SetDeviceStateNotSupported(void);
				int		cFirstBoxXloc;
				int		cFirstBoxYloc;

};


#endif // _WINDOWTAB_DEVICESTATE_H_
