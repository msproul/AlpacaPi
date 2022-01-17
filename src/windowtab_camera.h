//*****************************************************************************
//#include	"windowtab_camera.h"



#ifndef	_WINDOWTAB_CAMERA_H_
#define	_WINDOWTAB_CAMERA_H_

#ifndef	_CONTROLLER_H_
//	#include	"controller.h"
#endif // _CONTROLLER_H_

#ifndef _WINDOW_TAB_H_
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H



//*****************************************************************************
enum
{
	kCameraBox_Title	=	0,
	kCameraBox_Connected,

	kCameraBox_Size,
	kCameraBox_Logo,

	kCameraBox_Exposure_Label,
	kCameraBox_Exposure_Slider,
	kCameraBox_Exposure,
	kCameraBox_Exposure_Up,
	kCameraBox_Exposure_Down,

	kCameraBox_Gain_Label,
	kCameraBox_Gain_Slider,
	kCameraBox_Gain,
	kCameraBox_Gain_Up,
	kCameraBox_Gain_Down,

	kCameraBox_Offset_Label,
	kCameraBox_Offset_Slider,
	kCameraBox_Offset,
	kCameraBox_Offset_Up,
	kCameraBox_Offset_Down,


	kCameraBox_State,
	kCameraBox_PercentCompleted,

	kCameraBox_ReadMode0,
	kCameraBox_ReadMode1,
	kCameraBox_ReadMode2,
	kCameraBox_ReadMode3,
	kCameraBox_ReadMode4,
	kCameraBox_ReadModeOutline,

	kCameraBox_LiveMode,
	kCameraBox_SideBar,
	kCameraBox_AutoExposure,
	kCameraBox_DisplayImage,
	kCameraBox_SaveAll,

	kCameraBox_Reset,
	kCameraBox_FlipText,
	kCameraBox_FlipValue,
	kCameraBox_FlipHorzCheckBox,
	kCameraBox_FlipVertCheckBox,

	kCameraBox_Filename,
	kCameraBox_ErrorMsg,

	kCameraBox_StartExposure,
	kCameraBox_StopExposure,
	kCameraBox_Rank2,
	kCameraBox_Rank3,

	kCameraBox_EnableBinary,
	kCameraBox_Btn_8Bit,

	kCameraBox_DownloadImage,
	kCameraBox_DownloadRGBarray,

	kCameraBox_SaveOutline,


	kCameraBox_FilterWheelName,
	kCameraBox_FilterWheel1,
	kCameraBox_FilterWheel2,
	kCameraBox_FilterWheel3,
	kCameraBox_FilterWheel4,
	kCameraBox_FilterWheel5,
	kCameraBox_FilterWheel6,
	kCameraBox_FilterWheel7,
	kCameraBox_FilterWheel8,
//+	kCameraBox_FilterWheel9,

	kCameraBox_FilterOffset1,
	kCameraBox_FilterOffset2,
	kCameraBox_FilterOffset3,
	kCameraBox_FilterOffset4,
	kCameraBox_FilterOffset5,
	kCameraBox_FilterOffset6,
	kCameraBox_FilterOffset7,
	kCameraBox_FilterOffset8,
//+	kCameraBox_FilterOffset9,

	kCameraBox_FilterWheelOutline,

	kCameraBox_Temp_Label,
	kCameraBox_Temperature,
	kCameraBox_CoolerChkBox,
#if 0
	kCameraBox_TempGraph,
	kCameraBox_TempOutline,
#endif

	kCameraBox_LastCmdString,
	kCameraBox_AlpacaLogo,

	kCameraBox_AlpacaDrvrVersion,
	kCameraBox_IPaddr,
	kCameraBox_Readall,

	kCameraBox_last
};

//**************************************************************************************
class WindowTabCamera: public WindowTab
{
	public:
		//
		// Construction
		//
				WindowTabCamera(	const int	xSize,
									const int	ySize,
									CvScalar	backGrndColor,
									const char	*windowName,
									const char	*deviceName,
									const bool	hasFilterWheel = false);
		virtual	~WindowTabCamera(void);

		virtual	void	SetupWindowControls(void);
		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
		virtual	void	UpdateSliderValue(const int widgetIdx, double newSliderValue);

				void	ForceUpdate(void);
				void	BumpGain(const int howMuch);
				void	BumpExposure(const double howMuch);
				void	BumpOffset(const int howMuch);
				void	ToggleLiveMode(void);
				void	ToggleSideBar(void);
				void	ToggleAutoExposure(void);
				void	ToggleDisplayImage(void);
				void	ToggleSaveAll(void);
				void	ToggleCooler(void);
				void	ToggleFlipMode(bool toggleHorz, bool toggleVert);
				void	StartExposure(void);
				void	SetTempartueDisplayEnable(bool enabled);
				void	SetReceivedFileName(const char *newFileName);
				void	DownloadImage(const bool useRGBarray);

				bool	cForce8BitRead;
				bool	cAllowBinaryDownload;

//				void	DrawGraph(IplImage *openCV_Image, TYPE_WIDGET *graphWidget, double *arrayData, int arrayCount);

				char		cAlpacaDeviceName[64];
				bool		cHasFilterWheel;
				char		cDownLoadedFileNameRoot[256];

				uint32_t	cLastExposureUpdate_Millis;
				uint32_t	cLastGainUpdate_Millis;
				uint32_t	cLastOffsetUpdate_Millis;

};


#endif // _WINDOWTAB_CAMERA_H_

