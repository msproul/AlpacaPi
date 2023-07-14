//**************************************************************************************
//#include	"controller_focus_ml_hr.h"

#ifndef	_WINDOWTAB_MOONLITE_SINGLE_H_
	#include	"windowtab_ml_single.h"
#endif

#ifndef	_WINDOWTAB_DEVICESTATE_H_
	#include	"windowtab_DeviceState.h"
#endif

#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif

//**************************************************************************************
class ControllerMLsingle: public ControllerFocus
{
	public:
		//
		// Construction
		//
				ControllerMLsingle(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice);

				ControllerMLsingle(	const char			*argWindowName,
									const char			*usbPortPath);


		virtual	~ControllerMLsingle(void);

		virtual	void	CreateWindowTabs(void);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);

		virtual	void	UpdateCommonProperties(void);
		virtual	void	UpdateSupportedActions(void);

		virtual	void	UpdateStartupData(void);
		virtual	void	UpdateStatusData(void);

		virtual	void	UpdateWindowTabs_Everything(void);
		virtual	void	UpdateWindowTabs_ReadAll(		bool hasReadAll);
		virtual	void	UpdateWindowTabs_DesiredFocusPos(const int newDesiredPoistion);
		virtual	void	UpdateWindowTabs_ConnectState(bool connectedState);

				//*	tab information
				WindowTabMLsingle		*cMLsingleTabObjPtr;
				WindowTabConfig			*cConfigTabObjPtr;
				WindowTabDeviceState	*cDeviceStateTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

};

