//*****************************************************************************
//#include	"controller_filterwheel.h"

#include	"controller.h"

#include	"windowtab_filterwheel.h"
#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif
#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif

//**************************************************************************************
class ControllerFilterWheel: public Controller
{
	public:
		//
		// Construction
		//
				ControllerFilterWheel(	const char			*argWindowName,
										TYPE_REMOTE_DEV		*alpacaDevice);
		virtual	~ControllerFilterWheel(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	bool	AlpacaGetStartupData(void);
		virtual	bool	AlpacaGetStartupData_OneAAT(void);
		virtual	void	UpdateConnectedStatusIndicator(void);
		virtual	bool	AlpacaGetStatus(void);
		virtual	void	AlpacaGetCapabilities(void);
		virtual	void	UpdateOnlineStatus(void);
		virtual	bool	AlpacaProcessReadAllIdx(	const char	*deviceTypeStr,
													const int	deviceNum,
													const int	keywordEnum,
													const char	*valueString);
		virtual	void	UpdateStartupData(void);
		virtual	void	UpdateStatusData(void);
		virtual	void	UpdateSupportedActions(void);

				bool	AlpacaGetFilterWheelStartup(void);
				bool	AlpacaGetFilterWheelStatus(void);
				void	UpdateFilterWheelInfo(void);
				void	UpdateFilterWheelPosition(void);

		TYPE_FilterWheelProperties	cFilterWheelProp;
				int					cPositionCount;
				char				cFilterWheelName[kCommonPropMaxStrLen];

				//*	tab information
				WindowTabFilterWheel	*cFilterWheelTabObjPtr;
				WindowTabDeviceState	*cDeviceStateTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;
};

