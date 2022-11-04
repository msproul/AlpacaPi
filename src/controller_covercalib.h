//*****************************************************************************
//#include	"controller_covercalib.h"

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif




#include	"controller.h"

#include	"windowtab_covercalib.h"

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
										struct sockaddr_in	*deviceAddress,
										const int			port,
										const int			deviceNum);


		virtual	~ControllerCoverCalib(void);


		virtual	void	SetupWindowControls(void);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);
//		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);
		virtual	void	UpdateCommonProperties(void);
		virtual	bool	AlpacaGetStartupData(void);
		virtual	void	UpdateSupportedActions(void);

				bool	AlpacaGetStatus(void);

				TYPE_CoverCalibrationProperties	cCoverCalibrationProp;


				//*	tab information
				WindowTabCoverCalib	*cCoverCalibTabObjPtr;
				WindowTabDriverInfo	*cDriverInfoTabObjPtr;
				WindowTabAbout		*cAboutBoxTabObjPtr;
};

