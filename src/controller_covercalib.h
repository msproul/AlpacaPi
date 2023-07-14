//*****************************************************************************
//#include	"controller_covercalib.h"

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif




#include	"controller.h"

#include	"windowtab_covercalib.h"

//===========================================
#ifndef	_WINDOWTAB_CAPABILITIES_H_
	#include	"windowtab_capabilities.h"
#endif

#ifndef	_WINDOWTAB_DEVICESTATE_H_
	#include	"windowtab_DeviceState.h"
#endif

#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_


//**************************************************************************************
class ControllerCoverCalib: public Controller
{
	public:
		//
		// Construction
		//
				ControllerCoverCalib(	const char			*argWindowName,
										TYPE_REMOTE_DEV		*alpacaDevice);


		virtual	~ControllerCoverCalib(void);


		virtual	void	SetupWindowControls(void);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
//		virtual	bool	AlpacaGetStartupData(void);
		virtual	bool	AlpacaGetStartupData_OneAAT(void);
		virtual	void	AlpacaGetCapabilities(void);
		virtual	void	UpdateSupportedActions(void);
		virtual void	UpdateCapabilityList(void);

		virtual	void	UpdateConnectedStatusIndicator(void);
		virtual	void	UpdateStartupData(void);
		virtual	void	UpdateStatusData(void);

		virtual	bool	AlpacaGetStatus(void);
		virtual	bool	AlpacaProcessReadAllIdx(const char	*deviceTypeStr,
												const int	deviceNum,
												const int	keywordEnum,
												const char	*valueString);

				TYPE_CoverCalibrationProperties	cCoverCalibrationProp;


				//*	tab information
				WindowTabCoverCalib		*cCoverCalibTabObjPtr;
				WindowTabCapabilities	*cCapabilitiesTabObjPtr;
				WindowTabDeviceState	*cDeviceStateTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;
};

