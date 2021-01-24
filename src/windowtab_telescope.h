//*****************************************************************************
//#include	"windowtab_telescope.h"



#ifndef	_WINDOWTAB_TELESCOPE_H_
#define	_WINDOWTAB_TELESCOPE_H_

#ifndef	_WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _CONTROLLER_H_

//*****************************************************************************
enum
{
	kTelescope_Title	=	0,

//	kTelescope_Logo,

	kTelescope_BtnUp,
	kTelescope_BtnDown,
	kTelescope_BtnEast,
	kTelescope_BtnWest,
	kTelescope_BtnEmergencyStop,

	kTelescope_BtnTrackingOn,
	kTelescope_BtnTrackingOff,
	kTelescope_TrackingStatus,


	kTelescope_SlewRate_Title,
	kTelescope_SlewRate_Fast,
	kTelescope_SlewRate_Med,
	kTelescope_SlewRate_Slow,
	kTelescope_SlewRate_VerySlow,
	kTelescope_SlewingStatus,
	kTelescope_SlewRate_Outline,

	kTelescope_RA_label,
	kTelescope_RA_value,
	kTelescope_HA_label,
	kTelescope_HA_value,
	kTelescope_DEC_label,
	kTelescope_DEC_value,

	kTelescope_ErrorMsg,
	kTelescope_LastCmdString,
	kTelescope_AlpacaLogo,

	kTelescope_AlpacaDrvrVersion,
	kTelescope_IPaddr,
	kTelescope_Readall,

	kTelescope_last
};


//**************************************************************************************
class WindowTabTelescope: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabTelescope(	const int	xSize,
								const int	ySize,
								CvScalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabTelescope(void);

//		virtual	void	DrawGraphWidget(const int widgitIdx);
		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawWindow(void);
//		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx);
//		virtual	void	ProcessDoubleClick(const int buttonIdx);

				void	UpdateButtons(void);
				int		cCurrentSlewRate;


};


#endif // _WINDOWTAB_TELESCOPE_H_


