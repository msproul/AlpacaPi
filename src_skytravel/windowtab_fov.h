//*****************************************************************************
//#include	"windowtab_fov.h"

#ifndef	_WINDOWTAB_FOV_H_
#define	_WINDOWTAB_FOV_H_


#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifndef _CAMERA_FOV_H_
	#include	"cameraFOV.h"
#endif

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif


//*****************************************************************************
enum
{
	kFOVbox_Title	=	0,

	kFOVbox_FileInfo,

	kFOVbox_ClmTitle1,
	kFOVbox_ClmTitle2,
	kFOVbox_ClmTitle3,
	kFOVbox_ClmTitle4,
	kFOVbox_ClmTitle5,
	kFOVbox_ClmTitle6,
	kFOVbox_ClmTitle7,
	kFOVbox_ClmTitle8,
	kFOVbox_ClmTitle9,
	kFOVbox_ClmTitle10,


	kFOVbox_CamChkBox1,
	kFOVbox_CamInfo1,

	kFOVbox_CamChkBox2,
	kFOVbox_CamInfo2,

	kFOVbox_CamChkBox3,
	kFOVbox_CamInfo3,

	kFOVbox_CamChkBox4,
	kFOVbox_CamInfo4,

	kFOVbox_CamChkBox5,
	kFOVbox_CamInfo5,

	kFOVbox_CamChkBox6,
	kFOVbox_CamInfo6,

	kFOVbox_CamChkBox7,
	kFOVbox_CamInfo7,

	kFOVbox_CamChkBox8,
	kFOVbox_CamInfo8,

	kFOVbox_CamChkBox9,
	kFOVbox_CamInfo9,

	kFOVbox_CamChkBox10,
	kFOVbox_CamInfo10,

	kFOVbox_CamChkBox11,
	kFOVbox_CamInfo11,

	kFOVbox_CamChkBox12,
	kFOVbox_CamInfo12,

	kFOVbox_CamChkBox13,
	kFOVbox_CamInfo13,

	kFOVbox_CamChkBox14,
	kFOVbox_CamInfo14,

	kFOVbox_CamChkBox15,
	kFOVbox_CamInfo15,

	kFOVbox_CamChkBox16,
	kFOVbox_CamInfo16,

	kFOVbox_CamChkBox17,
	kFOVbox_CamInfo17,

	kFOVbox_CamChkBox18,
	kFOVbox_CamInfo18,

	kFOVbox_CamChkBox19,
	kFOVbox_CamInfo19,

	kFOVbox_CamChkBox20,
	kFOVbox_CamInfo20,


	kFOVbox_CamChkBox_Last	=	kFOVbox_CamInfo20,

	kFOVbox_SaveButton,
	kFOVbox_EditButton,
	kFOVbox_ReloadButton,
	kFOVbox_RescanButton,

	kFOVbox_AlpacaLogo,


	kFOVbox_last
};


//**************************************************************************************
class WindowTabFOV: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabFOV(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName);
		virtual	~WindowTabFOV(void);

		virtual	void	RunWindowBackgroundTasks(void);
		virtual	void	SetupWindowControls(void);
		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx, const int flags);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
		virtual void	ProcessMouseEvent(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual	void	ProcessLineSelect(int widgetIdx);
				void	SetNewSelectedDevice(int deviceIndex);
		virtual	void	UpdateOnScreenWidgetList(void);

				void	ClearRemoteDeviceList(void);
				void	UpdateRemoteDeviceList(void);
				void	UpdateSortOrder(void);

				bool	AlpacaProcessReadAllIdx_Camera(	const int	deviceNum,
														const int	keywordEnum,
														const char *valueString);

				bool	AlpacaProcessReadAll_Camera(	const int	deviceNum,
														const char	*keywordString,
														const char *valueString);

				TYPE_CameraFOV	*GetCameraFOVptr(void);
				void			WriteCameraFOVfile(void);
				void			ResetFOVdata(void);

				TYPE_REMOTE_DEV	cRemoteDeviceList[kMaxCamaeraFOVcnt];
				TYPE_CameraFOV	cCameraData[kMaxCamaeraFOVcnt];
				TYPE_CameraFOV	*cCurrentCamera;

				int				cAlpacaDevCnt;
				int				cPrevAlpacaDevCnt;

				bool			cFirstRead;
				uint32_t		cLastUpdateTime_ms;
};



#endif // _WINDOWTAB_FOV_H_




