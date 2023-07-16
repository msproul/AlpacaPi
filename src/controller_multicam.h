//*****************************************************************************
//#include	"controller_multicam.h"

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif

#ifndef	_WINDOWTAB_MULTICAM_H_
	#include	"windowtab_multicam.h"
#endif

#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif

#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif
//**************************************************************************************
typedef struct
{
	char	CameraName[64];
//	int		LastAlpacaError;
//	char	LastErrorMsg[256];

}	TYPE_CameraList;

#define	kMulticamMaxCameraCnt	10

//**************************************************************************************
class ControllerMulticam: public Controller
{
	public:
		//
		// Construction
		//
				ControllerMulticam(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice);
		virtual	~ControllerMulticam(void);

		virtual	void	SetupWindowControls(void);
//		virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);
		virtual	bool	AlpacaGetStartupData(void);
		bool			AlpacaGetStatus(void);

		virtual	void	UpdateSupportedActions(void);
		virtual	void	UpdateCommonProperties(void);
		virtual	void	UpdateOnlineStatus(void);

		virtual	bool	AlpacaProcessReadAll(		const char	*deviceTypeStr,
													const int	deviceNum,
													const char	*keywordString,
													const char	*valueString);

				TYPE_CameraList		cCameraList[kMulticamMaxCameraCnt];

				//*	window tab information
				WindowTabMulticam	*cMultiCamTabObjPtr;
				WindowTabDriverInfo	*cDriverInfoTabObjPtr;
				WindowTabAbout		*cAboutBoxTabObjPtr;

};


