//*****************************************************************************
//#include	"controller_preview.h"

#ifndef _CONTROLLER_PREVIEW_H_
#define _CONTROLLER_PREVIEW_H_



#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif // _DISCOVERY_LIB_H_

#ifndef	_WINDOWTAB_PREVIEW_H_
	#include	"windowtab_preview.h"
#endif // _WINDOWTAB_PREVIEW_H_


#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_



#ifndef _CONTROLLER_CAMERA_H_
	#include	"controller_camera.h"
#endif // _CONTROLLER_CAMERA_H_

//**************************************************************************************
class ControllerPreview: public ControllerCamera
{
	public:
		//
		// Construction
		//

				ControllerPreview(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice);

		virtual	~ControllerPreview(void);

	//-			void	ControllerCameraInit(void);

		virtual	void	SetupWindowControls(void);
	//	virtual	void	ProcessButtonClick(const int buttonIdx);
	//	virtual	void	RunBackgroundTasks(const char *callingFunction=NULL, bool enableDebug=false);

		//*	this is a large list of update routines, they should be implemented in the subclass
		virtual	void	UpdateCameraExposure(void);
		virtual	void	UpdateCameraGain(void);
		virtual	void	UpdateCameraName(void);
		virtual	void	UpdateCameraSize(void);
		virtual	void	UpdateCameraState(void);
		virtual	void	UpdateCurrReadoutMode(void);
		virtual	void	UpdateDisplayModes(void);

		virtual	void	UpdateSupportedActions(void);
		virtual	void	UpdateRemoteAlpacaVersion(void);
		virtual	void	UpdateReadoutModes(void);
		virtual	void	UpdateReceivedFileName(const char *newFileName);

		virtual	void	UpdateDownloadProgress(const int unitsRead, const int totalUnits);
		virtual	void	UpdateBackgroundColor(const int redValue, const int grnValue, const int bluValue);

				void	SetRefID(const char *newRefID);

				WindowTabPreview		*cPreviewTabObjPtr;
				WindowTabAbout			*cAboutBoxTabObjPtr;

};



#endif	// _CONTROLLER_PREVIEW_H_
