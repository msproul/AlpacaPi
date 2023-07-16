//*****************************************************************************
//#include	"controller_focus.h"

#ifndef _CONTROLLER_FOCUS_H_
#define _CONTROLLER_FOCUS_H_

#include	"controller.h"
#ifndef _WINDOW_TAB_H
	#include	"windowtab.h"
#endif

#ifndef	_WINDOWTAB_FOCUSER_H_
//	#include	"windowtab_focuser.h"
#endif

#ifndef _WINDOWTAB_NITECRAWLER_H_
	#include	"windowtab_nitecrawler.h"
#endif

#ifndef	_WINDOWTAB_AUXMOTOR_H_
	#include	"windowtab_auxmotor.h"
#endif

#ifndef	_WINDOWTAB_GRAPHS_H_
	#include	"windowtab_graphs.h"
#endif

#ifndef _WINDOWTAB_CONFIG_H_
	#include	"windowtab_config.h"
#endif

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif

#ifndef	_MOONLITE_COM_H_
	#include	"moonlite_com.h"
#endif

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif

#define	kFocuserBoxWidth	400
#define	kFocuserBoxHeight	775


//**************************************************************************************
enum
{
	kFocuserType_NiteCrawler	=	0,
	kFocuserType_MoonliteSingle,
	kFocuserType_MoonliteDouble,
	kFocuserType_Shelyak,			//*	this is the focuser in the spectrograph

	kFocuserType_Other,

};

//**************************************************************************************
//*	this is for NiteCrawler
enum
{
	kSwitch_Out	=	0,
	kSwitch_In,
	kSwitch_Rot
};


//**************************************************************************************
class ControllerFocus: public Controller
{
	public:
		//
		// Construction
		//
				ControllerFocus(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice,
									const int			focuserType);

				ControllerFocus(	const char			*argWindowName,
									const char			*usbPortPath,
									int					focuserType = kFocuserType_NiteCrawler);

		virtual	~ControllerFocus(void);

				void	ControllerFocusInit(const int comMode, const int focuserType);

		virtual	void	CreateWindowTabs(void);

//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);

		//*	sub class specific routines
		virtual	void	AlpacaGetCapabilities(void);
		virtual	void	GetStartUpData_SubClass(void);
		virtual	void	GetStatus_SubClass(void);

		virtual	void	UpdateSupportedActions(void);
		virtual	void	UpdateConnectedStatusIndicator(void);

		virtual	void	UpdateStartupData(void);
		virtual	void	UpdateStatusData(void);
		virtual	void	UpdateOnlineStatus(void);

		virtual	void	UpdateRotatorPosition(void);
		virtual	void	UpdateStepsPerRev(const int newStepsPerRev);
		virtual	void	UpdateVoltage(const double newVoltage);

		virtual	void	UpdateWindowTabs_Everything(void);
		virtual	void	UpdateWindowTabs_ReadAll(bool hasReadAll);
		virtual	void	UpdateWindowTabs_SwitchBits(unsigned char switchBits);
		virtual	void	UpdateWindowTabs_AuxSwitchBits(unsigned char auxSwitchBits);
		virtual	void	UpdateWindowTabs_DesiredFocusPos(const int newDesiredPoistion);
		virtual	void	UpdateWindowTabs_DesiredRotatorPos(const int newDesiredPoistion);
		virtual	void	UpdateWindowTabs_DesiredAuxPos(const int newDesiredPoistion);

		virtual	void	UpdateWindowTabs_ConnectState(bool connectedState);

		virtual	void	UpdateWindowTabs_SwitchState(int switchId, bool onOffState);

		virtual	bool	AlpacaGetStatus(void);
		virtual	bool	AlpacaProcessReadAllIdx(const char	*deviceTypeStr,
												const int	deviceNum,
												const int	keywordEnum,
												const char	*valueString);
		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceTypeStr,
														const int	deviveNum,
														const char	*valueString);
				bool	AlpacaGetStartupData_OneAAT(void);
				bool	AlpacaGetStatus_OneAAT(void);	//*	One At A Time

				void	SendMoveFocuserCommand(int newDesiredPosition);
				void	SendMoveRotatorCommand(int newDesiredPosition);
				void	SendMoveAuxMotorCommand(int newDesiredPosition);
				void	SendStopMotorsCommand(void);
				void	ZeroMotorValues(void);

				void	ReadNiteCrawlerColors(void);

				void	OpenUSBport(void);
				void	CloseUSBport(void);
				bool	USBGetStatus(void);

				int					cFocuserType;		//*	i.e. nitecrawler, etc
				int					cCommMode;			//*	Alpaca or USB
				char				cUSBpath[48];
				bool				cValidUSB;
				bool				cUSBportOpen;
				TYPE_MOONLITECOM	cMoonliteCom;

				char				cModelName[48];
				char				cSerialNumber[48];
				char				cUnitVersion[48];

				TYPE_FocuserProperties	cFocuserProp;

//				int					cFocuserPosition;
				int					cRotatorPosition;
				int					cFocuserDesiredPos;
				int					cRotatorDesiredPos;
				int					cAuxMotorPosition;
				int					cAuxMotorDesiredPos;
				int					cStepsPerRev;			//*	steps per revolution

				uint32_t			cLastTimeSecs_Temperature;
//				double				cTemperature_DegC;
				double				cVoltage;


};


int		GenerateFocuserWindowName(TYPE_REMOTE_DEV *device, int focuserNum, char *windowName);
int		CheckForFocuser(TYPE_REMOTE_DEV *remoteDevice);

#endif // _CONTROLLER_FOCUS_H_
