//*****************************************************************************
//#include	"controller_cam_normal.h"

#ifndef _CONTROLLER_CAM_NORMAL_H_
#define _CONTROLLER_CAM_NORMAL_H_



#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif // _DISCOVERY_LIB_H_


#include	"windowtab_camera.h"
#include	"windowtab_camsettings.h"
#include	"windowtab_filelist.h"
#include	"windowtab_about.h"

#ifndef	_WINDOWTAB_CAPABILITIES_H_
	#include	"windowtab_capabilities.h"
#endif
#ifndef	_WINDOWTAB_DRIVER_INFO_H_
	#include	"windowtab_drvrInfo.h"
#endif

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif

#ifndef _CONTROLLER_CAMERA_H_
	#include	"controller_camera.h"
#endif // _CONTROLLER_CAMERA_H_

//**************************************************************************************
class ControllerCamNormal: public ControllerCamera
{
	public:
		//
		// Construction
		//

				ControllerCamNormal(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice);

		virtual	~ControllerCamNormal(void);

	//-			void	ControllerCameraInit(void);

		virtual	void	SetupWindowControls(void);
	//	virtual	void	ProcessButtonClick(const int buttonIdx);
	//	virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);


		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);
		virtual	void	UpdateSettings_Object(const char *filePrefix);

		//*	this is a large list of update routines, they should be implemented in the subclass
		virtual	void	UpdateCommonProperties(void);
		virtual	void	UpdateCapabilityList(void);

		virtual	void	UpdateCameraGain(const TYPE_ASCOM_STATUS lastAlpacaErr = kASCOM_Err_Success);
		virtual	void	UpdateCameraOffset(const TYPE_ASCOM_STATUS lastAlpacaErr = kASCOM_Err_Success);
		virtual	void	UpdateCameraExposure(void);
		virtual	void	UpdateCameraName(void);
		virtual	void	UpdateCameraSize(void);
		virtual	void	UpdateCameraState(void);
		virtual	void	UpdateCameraTemperature(void);
		virtual	void	UpdatePercentCompleted(void);


		virtual	void	UpdateSupportedActions(void);
		virtual	void	UpdateReadoutModes(void);
		virtual	void	UpdateFlipMode(void);
		virtual	void	UpdateCurrReadoutMode(void);
		virtual	void	UpdateCoolerState(void);
		virtual	void	UpdateDisplayModes(void);
		virtual	void	UpdateFilterWheelInfo(void);
		virtual	void	UpdateFilterWheelPosition(void);
		virtual	void	UpdateFileNameOptions(void);
		virtual	void	UpdateLiveMode(void);
		virtual	void	UpdateReceivedFileName(const char *newFileName);
		virtual	void	UpdateRemoteFileList(void);
		virtual	void	UpdateDownloadProgress(const int unitsRead, const int totalUnits);
		virtual	void	UpdateConnectedStatusIndicator(void);

		virtual	void	UpdateBackgroundColor(const int redValue, const int grnValue, const int bluValue);

		virtual	void	ProcessReadAll_IMU(		const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString);

				void	DisableFilterWheel(void);

				bool					cIMUdetected;

				WindowTabCamera			*cCameraTabObjPtr;
				WindowTabCamSettings	*cCamSettingsTabObjPtr;
				WindowTabCapabilities	*cCapabilitiesTabObjPtr;
				WindowTabFileList		*cFileListTabObjPtr;
				WindowTabDriverInfo		*cDriverInfoTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

};



#endif	// _CONTROLLER_CAM_NORMAL_H_
