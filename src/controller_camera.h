//*****************************************************************************
//#include	"controller_camera.h"

#ifndef _CONTROLLER_CAMERA_H_
#define	_CONTROLLER_CAMERA_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif




#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif

#ifndef	_WINDOWTAB_CAMERA_H_
	#include	"windowtab_camera.h"
#endif

#ifndef	_WINDOWTAB_FILELIST_H_
	#include	"windowtab_filelist.h"
#endif

#ifndef	_WINDOWTAB_CAMSETTINGS_H_
	#include	"windowtab_camsettings.h"
#endif


#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif

#ifndef _SENDREQUEST_LIB_H
	#include	"sendrequest_lib.h"
#endif




//**************************************************************************************
typedef struct
{
	char	filename[128];
	bool	validData;
	bool	localCopy;
	bool	hasFTS;
	bool	hasJPG;
	bool	hasCSV;
	bool	hasPNG;
} TYPE_REMOTE_FILE;

//*****************************************************************************
typedef struct
{
	char	ContentTypeStr[128];
	char	ContentLengthStr[128];
	bool	dataIsBinary;
	bool	dataIsJson;
	int		contentLength;

} TYPE_HTTPheader;


#define	kMaxRemoteFileCnt		200
#define	kMaxTemperatureValues	(450)

//**************************************************************************************
class ControllerCamera: public Controller
{
	public:
		//
		// Construction
		//
				ControllerCamera(	const char			*argWindowName,
									struct sockaddr_in	*deviceAddress = NULL,
									const int			port = 0,
									const int			deviceNum = 0);

				ControllerCamera(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice,
									const int			xSize,
									const int			ySize);

		virtual	~ControllerCamera(void);

	//-			void	ControllerCameraInit(void);

		virtual	void	SetupWindowControls(void);
	//	virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	RunBackgroundTasks(bool enableDebug=false);
	//	virtual	void	DrawGraphWidget(const int widgetIdx);
		virtual	void	DrawWidgetCustom(TYPE_WIDGET *theWidget);

		//*	this is a large list of update routines, they should be implemented in the subclass
		virtual	void	UpdateCameraGain(const TYPE_ASCOM_STATUS lastAlpacaErr = kASCOM_Err_Success);
		virtual	void	UpdateCameraOffset(const TYPE_ASCOM_STATUS lastAlpacaErr = kASCOM_Err_Success);
		virtual	void	UpdateCameraExposure(void);
		virtual	void	UpdateCameraName(void);
		virtual	void	UpdateCameraSize(void);
		virtual	void	UpdateCameraState(void);
		virtual	void	UpdateCameraTemperature(void);
		virtual	void	UpdatePercentCompleted(void);

		virtual	void	UpdateSupportedActions(void);
		virtual	void	UpdateRemoteAlpacaVersion(void);
		virtual	void	UpdateReadoutModes(void);
		virtual	void	UpdateCurrReadoutMode(void);
		virtual	void	UpdateCoolerState(void);
		virtual	void	UpdateDisplayModes(void);
		virtual	void	UpdateFlipMode(void);
		virtual	void	UpdateFilterWheelInfo(void);
		virtual	void	UpdateFilterWheelPosition(void);
		virtual	void	UpdateFileNameOptions(void);
		virtual	void	UpdateReceivedFileName(const char *newFileName);
		virtual	void	UpdateRemoteFileList(void);
		virtual	void	UpdateDownloadProgress(const int unitsRead, const int totalUnits);
		virtual	void	UpdateBackgroundColor(const int redValue, const int grnValue, const int bluValue);
		virtual	void	UpdateFreeDiskSpace(const double gigabytesFree);
		virtual	void	UpdateConnectedStatusIndicator(void);

		//*	sub class specific routines
		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceTypeStr,
														const int	deviveNum,
														const char	*valueString);

				void	SetErrorTextString(const char	*errorString);
				void	GetConfiguredDevices(void);
		virtual	bool	AlpacaGetStartupData(void);
				bool	AlpacaGetStartupData_OneAAT(void);
		virtual	void	AlpacaProcessReadAll(	const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString);
				bool	AlpacaGetStatus_OneAAT(void);	//*	One At A Time
				bool	AlpacaGetStatus_Gain(void);
				bool	AlpacaGetStatus_Exposure(void);
				bool	AlpacaGetStatus(void);
				bool	AlpacaGetFileList(void);
				bool	AlpacaGetFilterWheelStartup(void);
				bool	AlpacaGetFilterWheelStatus(void);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);
		virtual	void	UpdateSettings_Object(const char *filePrefix);
				void	SetFileNameOptions(const int fnOptionBtn);
				void	SetFlipMode(const int newFlipMode);

				void	AddFileToRemoteList(const char *fileName);

				void	BumpExposure(const double howMuch);
				void	SetExposure(const double newExposure);

				void	BumpGain(const int howMuch);
				void	SetGain(const int newGain);

				void	BumpOffset(const int howMuch);
				void	SetOffset(const int newOffset);

				void	ToggleLiveMode(void);
				void	ToggleSideBar(void);
				void	ToggleAutoExposure(void);
				void	ToggleDisplayImage(void);
				void	ToggleSaveAll(void);
				void	ToggleCooler(void);
				void	ToggleFlipMode(bool toggleHorz, bool toggleVert);
				void	StartExposure(void);
				void	SetObjectText(const char *newObjectText, const char *newPrefixText);
				void	LogCameraTemp(const double cameraTemp);


				IplImage	*DownloadImage(const bool force8BitRead,  const bool allowBinary);
				IplImage	*DownloadImage_rgbarray(void);
				IplImage	*DownloadImage_imagearray(const bool force8BitRead, const bool allowBinary);



				TYPE_CameraProperties	cCameraProp;

				//*	temp related stuff
				bool					cHasCCDtemp;
				double					cCameraTempLog[kMaxTemperatureValues];
				int						cTempLogCount;

				//*	download options
				bool					cReadData8Bit;

				bool					cCameraState_imageready;

				double					cExposure;
				bool					cDarkExposure;
				bool					cLiveMode;
				bool					cSideBar;
				bool					cAutoExposure;
				bool					cDisplayImage;
				bool					cSaveAllImages;
				bool					cHasCooler;

				bool					cHas_autoexposure;
				bool					cHas_displayimage;
				bool					cHas_exposuretime;
				bool					cHas_filelist;
				bool					cHas_filenameoptions;
				bool					cHas_livemode;
				bool					cHas_rgbarray;
				bool					cHas_sidebar;
				bool					cHas_SaveAll;
				bool					cHas_Flip;

				//==========================================================
				//*	Image array downloading routines
				int		AlpacaGetIntegerArrayShortLines(	const char	*alpacaDevice,
												const int	alpacaDevNum,
												const char	*alpacaCmd,
												const char	*dataString,
												int			*uint32array,
												int			arrayLength,
												int			*actualValueCnt);

				int		AlpacaGetImageArray(	const char		*alpacaDevice,
												const int		alpacaDevNum,
												const char		*alpacaCmd,
												const char		*dataString,
												const bool		allowBinary,
												TYPE_ImageArray	*imageArray,
												int				arrayLength,
												int				*actualValueCnt);
				int		AlpacaGetImageArray_Binary(	TYPE_ImageArray	*imageArray,
													int				arrayLength,
													int				*actualValueCnt);
				int		AlpacaGetImageArray_JSON(	TYPE_ImageArray	*imageArray,
													int				arrayLength,
													int				*actualValueCnt);
				void	AlpacaGetImageArray_Binary_Byte(TYPE_ImageArray	*imageArray,
														int				arrayLength);

				void	UpdateImageProgressBar(int maxArrayLength);

				//*	these variables are ONLY for image download
				TYPE_HTTPheader 		cHttpHdrStruct;
				TYPE_BinaryImageHdr		cBinaryImageHdr;
				bool					cKeepReading;
				bool					cReadBinaryHeader;
				bool					cValueFoundFlag;
				int						cRanOutOfRoomCnt;
				int						cRecvdByteCnt;
				int						cSocketReadCnt;
				int						cSocket_desc;
				int						cTotalBytesRead;
				int						cLinesProcessed;
				int						cImageArrayIndex;
				int						cFirstCharNotDigitCnt;
				int						cData_iii;
				int						cImgArrayType;
				int						cRGBidx;
				char					cReturnedData[kReadBuffLen + 10];

				uint32_t				tStartMillisecs;
				uint32_t				tCurrentMillisecs;
				uint32_t				tLastUpdateMillisecs;
				uint32_t				tDeltaMillisecs;
				uint32_t				tStopMillisecs;

				//==========================================================
				//*	File name information
				bool					cFN_includeSerialNum;
				bool					cFN_includeManuf;
				bool					cFN_includeFilter;
				bool					cFN_includeRefID;

				//==========================================================
				//*	filter wheel information
				bool						cHas_FilterWheel;
				char						cFilterWheelName[32];
				int							cPositionCount;
				TYPE_FilterWheelProperties	cFilterWheelProp;

				TYPE_REMOTE_FILE		cRemoteFiles[kMaxRemoteFileCnt];

				//==========================================================
				//*	download status stuff
				double					cPrevProgessValue;
				int						cProgressUpdates;
				int						cProgressReDraws;
};

#endif // _CONTROLLER_CAMERA_H_
