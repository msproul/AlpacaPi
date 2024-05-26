//*****************************************************************************
//*	Sep  9,	2023	<MLS> Created controller_remoteview.h
//*****************************************************************************
//#include	"controller_remoteview.h"

#ifndef _CONTROLLER_REMOTEVIEW_H_
#define	_CONTROLLER_REMOTEVIEW_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif

#include	"controller.h"
#ifndef _CONTROLLER_CAMERA_H_
	#include	"controller_camera.h"
#endif

//===========================================
#ifndef	_WINDOWTAB_IMAGE_H_
	#include	"windowtab_image.h"
#endif
#ifndef	_WINDOWTAB_IMAGEINFO_H_
	#include	"windowtab_imageinfo.h"
#endif

//===========================================
#ifndef	_WINDOWTAB_DEVICESTATE_H_
	#include	"windowtab_DeviceState.h"
#endif

//===========================================
#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif


//===========================================
#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif


#undef kDomeWindowWidth
#define	kDomeWindowWidth	1000
#define	kDomeWindowHeight	720



//**************************************************************************************
class ControllerRemote: public ControllerCamera
{
	public:
		//
		// Construction
		//
				ControllerRemote(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice);

		virtual			~ControllerRemote(void);


				void	InitClassVariables(void);
		virtual	void	SetupWindowControls(void);
//		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);
//		virtual	void	UpdateConnectedStatusIndicator(void);
//		virtual	void	GetStartUpData_SubClass(void);
//		virtual	bool	AlpacaGetStartupData_OneAAT(void);
//		virtual	void	UpdateStartupData(void);
//		virtual	void	UpdateStatusData(void);
//		virtual	void	UpdateOnlineStatus(void);
//		virtual	void	AlpacaGetCapabilities(void);
//
//		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceType,
//														const int	deviveNum,
//														const char	*valueString);
//
//		virtual	void	UpdateSupportedActions(void);
//		virtual	void	UpdateCapabilityList(void);
//
//		virtual	void	AlpacaProcessSupportedActions_Dome(const int deviveNum, const char *valueString);
//		virtual	bool	AlpacaProcessReadAllIdx(		const char	*deviceTypeStr,
//														const int	deviceNum,
//														const int	keywordEnum,
//														const char	*valueString);
//
//				void	SetImageWindowInfo(void);
//				void	SetDownloadInfo(double download_MBytes, double download_seconds);

				char					cImageFileName[128];
				TYPE_BinaryImageHdr		cBinaryImageHdr;

				//===================================================================
				//*	tab information
				WindowTabImage			*cImageTabObjPtr;
				WindowTabImageInfo		*cImageInfoTabObjcPtr;
//				WindowTabCapabilities	*cCapabilitiesTabObjPtr;
//				WindowTabDeviceState	*cDeviceStateTabObjPtr;
//				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;


};


#endif // _CONTROLLER_REMOTEVIEW_H_

