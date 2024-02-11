//*****************************************************************************
//#include	"controller_telescope.h"

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#include	"controller.h"
#include	"windowtab_telescope.h"

#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

#ifndef	_WINDOWTAB_TELE_SETTINGS_H_
	#include	"windowtab_teleSettings.h"
#endif
#ifndef	_WINDOWTAB_CAPABILITIES_H_
	#include	"windowtab_capabilities.h"
#endif
#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif

#include	"windowtab_drvrInfo.h"
#include	"windowtab_DeviceState.h"

void	GetSideOfPierString(TYPE_PierSide sideOfPier, char *sideOfPierString);


//*****************************************************************************
class ControllerTelescope: public Controller
{
	public:
		//
		// Construction
		//
				ControllerTelescope(	const char			*argWindowName,
										TYPE_REMOTE_DEV		*alpacaDevice);
//				ControllerTelescope(const char			*argWindowName,
//									struct sockaddr_in	*deviceAddress,
//									const int			port,
//									const int			deviceNum);


		virtual	~ControllerTelescope(void);

		virtual	void	SetupWindowControls(void);
//		virtual	void	ProcessButtonClick(const int buttonIdx);

		virtual	void	UpdateCommonProperties(void);
		virtual	void	UpdateStartupData(void);
		virtual	void	UpdateConnectedStatusIndicator(void);
		virtual	void	UpdateStatusData(void);
		virtual	void	UpdateOnlineStatus(void);
		virtual	bool	AlpacaGetStartupData(void);
		virtual	void	AlpacaGetCapabilities(void);
		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceType,
														const int	deviveNum,
														const char	*valueString);

		virtual	bool	AlpacaProcessReadAllIdx(		const char	*deviceTypeStr,
														const int	deviceNum,
														const int	keywordEnum,
														const char	*valueString);
//		virtual	void	UpdateDeviceStateEntry(const int index, const char *nameString, const char *valueString);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);

				void	AlpacaProcessSupportedActions_Telescope(	const int	deviveNum,
																	const char	*valueString);
				bool	AlpacaProcessReadAll_TelescopeIdx(	const int	deviceNum,
															const int	keywordEnum,
															const char *valueString);
				bool	AlpacaProcessReadAll_Telescope(		const int	deviceNum,
															const char	*keywordString,
															const char *valueString);
		virtual	void	GetStatus_SubClass(void);
				bool	AlpacaGetStatus_TelescopeOneAAT(void);	//*	One At A Time
				void	ReadOneTelescopeCapability(const char *propertyStr, const char	*reportedStr, bool *booleanValue);
				bool	AlpacaGetStartupData_TelescopeOneAAT(void);	//*	One At A Time
			//*	tab information
				WindowTabTelescope		*cTelescopeTabObjPtr;
				WindowTabTeleSettings	*cTeleSettingsTabObjPtr;
				WindowTabCapabilities	*cCapabilitiesTabObjPtr;
				WindowTabDeviceState	*cDeviceStateTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

				//---------------------------------------------------
				void			Update_TelescopeRtAscension(void);
				void			Update_TelescopeDeclination(void);
				void			Update_TelescopeSideOfPier(void);

		virtual	void			UpdateSupportedActions(void);
		virtual	void			UpdateCapabilityList(void);

				//---------------------------------------------------
				//*	telescope driver variables
				TYPE_TelescopeProperties	cTelescopeProp;

};

