//*****************************************************************************
//#include	"windowtab_telescope.h"



#ifndef	_WINDOWTAB_TELESCOPE_H_
#define	_WINDOWTAB_TELESCOPE_H_

#ifndef	_WINDOW_TAB_H_
	#include	"windowtab.h"
#endif

//*****************************************************************************
enum
{
	kTelescope_Title	=	0,

	kTelescope_Connected,
	kTelescope_Parked,
	kTelescope_PulseGuiding,

	kTelescope_BtnUp,
	kTelescope_BtnDown,
	kTelescope_BtnEast,
	kTelescope_BtnWest,
	kTelescope_BtnEmergencyStop,

	kTelescope_BtnTrackingOn,
	kTelescope_BtnTrackingOff,
	kTelescope_TrackingStatus,


	kTelescope_SlewRate_Title,

	kTelescope_SlewRate_VerySlow,
	kTelescope_SlewRate_Slow,
	kTelescope_SlewRate_Med,
	kTelescope_SlewRate_Fast,

	kTelescope_SlewRate_VerySlow_Val,
	kTelescope_SlewRate_Slow_Val,
	kTelescope_SlewRate_Med_Val,
	kTelescope_SlewRate_Fast_Val,

	kTelescope_SlewingStatus,
	kTelescope_SlewRate_Outline,

	kTelescope_HA_label,
	kTelescope_HA_value,
	kTelescope_RA_label,
	kTelescope_RA_value,
	kTelescope_DEC_label,
	kTelescope_DEC_value,

	kTelescope_SideOfPier_label,
	kTelescope_SideOfPier_value,

	kTelescope_PhysSideOfPier_label,
	kTelescope_PhysSideOfPier_value,

	kTelescope_IMU_Roll_label,
	kTelescope_IMU_Roll_value,
	kTelescope_IMU_Pitch_label,
	kTelescope_IMU_Pitch_value,
	kTelescope_IMU_Yaw_label,
	kTelescope_IMU_Yaw_value,

	kTelescope_Coord_Outline,


	kTelescope_HelpMsg,
	kTelescope_LastCmdString,
	kTelescope_AlpacaLogo,

	kTelescope_AlpacaErrorMsg,
	kTelescope_IPaddr,
	kTelescope_Readall,
	kTelescope_DeviceState,

	kTelescope_last
};

#define	kSupportedSlewRates	4

//**************************************************************************************
class WindowTabTelescope: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabTelescope(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabTelescope(void);

//		virtual	void	DrawGraphWidget(const int widgetIdx);
		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawWindow(void);
//		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
//		virtual	void	ProcessDoubleClick(const int	widgetIdx,
//											const int	event,
//											const int	xxx,
//											const int	yyy,
//											const int	flags);

				bool	ProcessMovementButtons(const int buttonIdx, const int flags);
				void	UpdateButtons(void);
				void	UpdateTelescopeInfo(TYPE_TelescopeProperties *telescopeProp, bool updateAll);
				int		cCurrentSlewRate;
	private:
				bool		cSlewingUp;
				bool		cSlewingDown;
				bool		cSlewingEast;
				bool		cSlewingWest;
				//*	button colors
				cv::Scalar	cBtnBGcolor_Normal;
				cv::Scalar	cBtnBGcolor_Slewing;
				cv::Scalar	cBtnBGcolor_Disabled;
				cv::Scalar	cBtnTXTcolor;

				bool		cSlewRates_RA_valid;
				bool		cSlewRates_DEC_valid;
				double		cSlewRate_RAmin;
				double		cSlewRate_RAmax;
				double		cRA_slewRates[kSupportedSlewRates];

				double		cSlewRate_DECmin;
				double		cSlewRate_DECmax;
				double		cDEC_slewRates[kSupportedSlewRates];

				bool		cAtPark;
};


#endif // _WINDOWTAB_TELESCOPE_H_


