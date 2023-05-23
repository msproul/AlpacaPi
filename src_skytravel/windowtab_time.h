//*****************************************************************************
//#include	"windowtab_time.h"

#ifndef	_WINDOWTAB_TIME_H_
#define	_WINDOWTAB_TIME_H_


#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kTimeTab_Title	=	0,

//	kTimeTab_ControllerVersion,
	kTimeTab_Documentation,

	kTimeTab_LinuxTimeTitle,
	kTimeTab_LinuxTimeUTC_lbl,
	kTimeTab_LinuxTimeUTC,

	kTimeTab_LinuxTimeLocal_lbl,
	kTimeTab_LinuxTimeLocal,

	kTimeTab_LinuxTimeSidereal_lbl,
	kTimeTab_LinuxTimeSidereal,

	kTimeTab_LinuxTimeOutline,

	kTimeTab_SkyTravelTitle,
	kTimeTab_SkyTravelUTC_lbl,
	kTimeTab_SkyTravelUTC,
	kTimeTab_SkyTravelLocal_lbl,
	kTimeTab_SkyTravelLocal,
	kTimeTab_SkyTravelSidereal_lbl,
	kTimeTab_SkyTravelSidereal,

	kTimeTab_SkyTravelTimeZone_lbl,
	kTimeTab_SkyTravelTimeZone,

	kTimeTab_SkyTravelDayLightSavTm_lbl,
	kTimeTab_SkyTravelDayLightSavTm,

	kTimeTab_SkyTravelOutline,


	kTimeTab_AlpacaLogo,


	kTimeTab_last
};


//**************************************************************************************
class WindowTabTime: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabTime(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabTime(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	RunWindowBackgroundTasks(void);
				void	UpdateTime(void);
};



#endif // _WINDOWTAB_ABOUT_H_




