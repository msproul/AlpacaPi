//*****************************************************************************
//#include	"controller_obsconditions.h"


#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#include	"controller.h"


#ifndef	_WINDOWTAB_OBSCOND_H_
	#include	"windowtab_obscond.h"
#endif
#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif


//**************************************************************************************
class ControllerObsCond: public Controller
{
	public:
		//
		// Construction
		//
				ControllerObsCond(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice);
		virtual	~ControllerObsCond(void);

	//-			void	ControllerSwitchInit(void);

		virtual	void	SetupWindowControls(void);
	//	virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	AlpacaGetCapabilities(void);
		virtual	bool	AlpacaGetStartupData_OneAAT(void);
		virtual	bool	AlpacaGetStatus(void);
		virtual	void	UpdateStartupData(void);
		virtual	void	UpdateConnectedStatusIndicator(void);
		virtual	void	UpdateStatusData(void);
		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceTypeStr,
														const int	deviveNum,
														const char	*valueString);
		virtual	bool	AlpacaProcessReadAllIdx(const char	*deviceTypeStr,
												const int	deviceNum,
												const int	keywordEnum,
												const char	*valueString);
		virtual	void	UpdateSupportedActions(void);
		virtual	void	UpdateCommonProperties(void);
				bool	AlpacaGetStatus_OneAAT(void);	//*	One At A Time


				TYPE_ObsConditionProperties	cObsCondProp;

			//*	tab information
				WindowTabObsCond		*cObsConditionsTabObjPtr;
				WindowTabDeviceState	*cDeviceStateTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;
};

