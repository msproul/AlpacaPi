//*****************************************************************************
//#include	"controller_filterwheel.h"


#include	"controller.h"

#include	"windowtab_filterwheel.h"
#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif
#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_


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
		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);
		virtual	bool	AlpacaGetStartupData(void);
		virtual	void	UpdateCommonProperties(void);

				bool	AlpacaGetFilterWheelStartup(void);
				bool	AlpacaGetFilterWheelStatus(void);
				void	UpdateFilterWheelInfo(void);
				void	UpdateFilterWheelPosition(void);

		TYPE_FilterWheelProperties	cFilterWheelProp;
				int					cPositionCount;
				char				cFilterWheelName[32];

				//*	tab information
				WindowTabFilterWheel	*cFilterWheelTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;
};

