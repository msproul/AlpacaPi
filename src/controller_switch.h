//*****************************************************************************
//#include	"controller_switch.h"


#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif

#ifndef	_WINDOWTAB_SWITCH_H_
	#include	"windowtab_switch.h"
#endif

#ifndef	_WINDOWTAB_DEVICESTATE_H_
	#include	"windowtab_DeviceState.h"
#endif

#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif


//**************************************************************************************
class ControllerSwitch: public Controller
{
	public:
		//
		// Construction
		//
				ControllerSwitch(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice);

//				ControllerSwitch(	const char			*argWindowName,
//									struct sockaddr_in	*deviceAddress = NULL,
//									const int			port = 0,
//									const int			deviceNum = 0);


		virtual	~ControllerSwitch(void);


		virtual	void	SetupWindowControls(void);
		virtual	void	GetStartUpData_SubClass(void);
		virtual	void	GetStatus_SubClass(void);
		virtual	void	AlpacaGetCapabilities(void);
		virtual	bool	AlpacaGetStartupData_OneAAT(void);
		virtual	void	UpdateConnectedStatusIndicator(void);
		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceTypeStr,
														const int	deviveNum,
														const char	*valueString);
//*	cannot use index readall due to switch numbers on the keyword strings
//		virtual	bool	AlpacaProcessReadAllIdx(	const char	*deviceTypeStr,
//													const int	deviceNum,
//													const int	keywordEnum,
//													const char	*valueString);
		virtual	bool	AlpacaProcessReadAll(	const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString);

		virtual	void	UpdateSupportedActions(void);
		virtual	void	UpdateStartupData(void);
		virtual	void	UpdateStatusData(void);
		virtual	void	UpdateOnlineStatus(void);

				bool	AlpacaGetStatus_OneAAT(void);	//*	One At A Time
				void	ToggleSwitchState(const int switchNum);
				void	TurnAllSwitchesOff(void);

			//*	tab information
				WindowTabSwitch			*cSwitchTabObjPtr;
				WindowTabDeviceState	*cDeviceStateTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

//				TYPE_SwitchInfo			cSwitchInfo[kMaxSwitches];
				TYPE_SwitchProperties	cSwitchProp;

};

