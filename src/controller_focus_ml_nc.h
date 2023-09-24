//*****************************************************************************
//#include	"controller_focus_ml_nc.h"

//#define		_ENABLE_CONFIG_TAB_

#include	"controller.h"
#ifndef _WINDOW_TAB_H
	#include	"windowtab.h"
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
class ControllerNiteCrawler: public ControllerFocus
{
	public:
		//
		// Construction
		//
				ControllerNiteCrawler(	const char			*argWindowName,
										TYPE_REMOTE_DEV		*alpacaDevice);

				ControllerNiteCrawler(	const char			*argWindowName,
										const char			*usbPortPath);

		virtual	~ControllerNiteCrawler(void);


		virtual	void	CreateWindowTabs(void);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);

		virtual	void	UpdateStartupData(void);
		virtual	void	UpdateStatusData(void);
		virtual	void	UpdateOnlineStatus(void);
		virtual	void	UpdateConnectedStatusIndicator(void);

		//*	sub class specific routines

		virtual void	UpdateSupportedActions(void);
		virtual void	UpdateCommonProperties(void);

//		virtual	void	UpdateFocuserPosition(void);
		virtual	void	UpdateRotatorPosition(void);
//		virtual	void	UpdateAuxMotorPosition(const int newAuxMotorPosition);
		virtual	void	UpdateStepsPerRev(const int newStepsPerRev);
		virtual	void	UpdateVoltage(const double newVoltage);

		virtual	void	UpdateWindowTabs_Everything(void);
		virtual	void	UpdateWindowTabs_SwitchBits(unsigned char switchBits);
		virtual	void	UpdateWindowTabs_AuxSwitchBits(unsigned char auxSwitchBits);
		virtual	void	UpdateWindowTabs_DesiredFocusPos(const int newDesiredPoistion);
		virtual	void	UpdateWindowTabs_DesiredRotatorPos(const int newDesiredPoistion);
		virtual	void	UpdateWindowTabs_DesiredAuxPos(const int newDesiredPoistion);
		virtual	void	UpdateWindowTabs_ConnectState(bool connectedState);
		virtual	void	UpdateWindowTabs_SwitchState(int switchId, bool onOffState);

				bool	AlpacaGetStatus(void);


				WindowTabNitecrawler	*cNiteCrawlerTabObjPtr;
				WindowTabAuxMotor		*cAuxTabObjPtr;
		#ifdef _ENABLE_CONFIG_TAB_
				WindowTabConfig			*cConfigTabObjPtr;
		#endif // _ENABLE_CONFIG_TAB_
				WindowTabGraph			*cGraphTabObjPtr;
				WindowTabDeviceState	*cDeviceStateTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;


};
