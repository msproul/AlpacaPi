//*****************************************************************************
//#include	"controller_rotator.h"


#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#include	"controller.h"


#ifndef	_WINDOWTAB_ROTATOR_H_
	#include	"windowtab_rotator.h"
#endif
#ifndef	_WINDOWTAB_CAPABILITIES_H_
	#include	"windowtab_capabilities.h"
#endif
#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif


//**************************************************************************************
class ControllerRotator: public Controller
{
	public:
		//
		// Construction
		//
				ControllerRotator(	const char			*argWindowName,
									struct sockaddr_in	*deviceAddress = NULL,
									const int			port = 0,
									const int			deviceNum = 0);


		virtual	~ControllerRotator(void);

	//-			void	ControllerSwitchInit(void);

		virtual	void	SetupWindowControls(void);
	//	virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);
	//	virtual	void	DrawWidgetCustomGraphic(const int widgetIdx);
		virtual	bool	AlpacaGetStartupData(void);
				bool	AlpacaGetStartupData_OneAAT(void);
				bool	AlpacaGetStatus(void);
		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceTypeStr,
														const int	deviveNum,
														const char	*valueString);
		virtual	void	AlpacaProcessReadAll(	const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString);
		virtual	void	UpdateCommonProperties(void);
		virtual	void	UpdateCapabilityList(void);

				bool	AlpacaGetStatus_OneAAT(void);	//*	One At A Time
				void	SendMoveAbsoluteCommand(double newPosition_Deg);
				void	SendMoveRelativeCommand(double positionChange_Deg);
				void	SendHaltCommand(void);


				TYPE_RotatorProperties	cRotatorProp;

				//*	tab information
				WindowTabRotator		*cRotatorTabObjPtr;
				WindowTabCapabilities	*cCapabilitiesTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;
};

