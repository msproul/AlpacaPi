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

#ifndef _SENDREQUEST_LIB_H
	#include	"sendrequest_lib.h"
#endif

#ifndef _CAMERA_DEFS_H_
	#include	"camera_defs.h"
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
#define	kObjectNameMaxLen		31

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

		virtual	void	SetupWindowControls(void);

		virtual	void	UpdateStatusData(void);

		//*	this is a large list of update routines, they should be implemented in the subclass
		//*	alphabetic order just for ease of finding things
		virtual	void	UpdateBackgroundColor(const int redValue, const int grnValue, const int bluValue);
		virtual	void	UpdateCameraExposureStartup(void);
		virtual	void	UpdateCameraExposure(void);
		virtual	void	UpdateCameraGain(const TYPE_ASCOM_STATUS lastAlpacaErr = kASCOM_Err_Success);
		virtual	void	UpdateCameraName(void);
		virtual	void	UpdateCameraOffset(const TYPE_ASCOM_STATUS lastAlpacaErr = kASCOM_Err_Success);
		virtual	void	UpdateCameraSize(void);
		virtual	void	UpdateCameraState(void);
		virtual	void	UpdateCameraTemperature(void);
		virtual	void	UpdateConnectedStatusIndicator(void);
		virtual	void	UpdateCoolerState(void);
		virtual	void	UpdateCurrReadoutMode(void);
		virtual	void	UpdateDisplayModes(void);
		virtual	void	UpdateDownloadProgress(const int unitsRead, const int totalUnits);
		virtual	void	UpdateFlipMode(void);
		virtual	void	UpdateFreeDiskSpace(const double gigabytesFree);
		virtual	void	UpdateFilterWheelInfo(void);
		virtual	void	UpdateFilterWheelPosition(void);
		virtual	void	UpdateFileNameOptions(void);
		virtual	void	UpdateLiveMode(void);
		virtual	void	UpdatePercentCompleted(void);
		virtual	void	UpdateReadoutModes(void);
		virtual	void	UpdateReceivedFileName(const char *newFileName);
		virtual	void	UpdateSupportedActions(void);
		virtual	void	UpdateRemoteFileList(void);

		virtual	void	SetExposureRange(char *name, double exposureMin, double exposureMax, double exposureStep);

		//*	sub class specific routines
		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceTypeStr,
														const int	deviveNum,
														const char	*valueString);

				void	SetErrorTextString(const char	*errorString);
//-				void	GetConfiguredDevices(void);
		virtual	void	ProcessConfiguredDevices(const char *keyword, const char *valueString);
		virtual	void	GetStartUpData_SubClass(void);
		virtual	void	GetStatus_SubClass(void);

		virtual	bool	AlpacaGetStartupData_OneAAT(void);
		virtual	bool	AlpacaProcessReadAllIdx(	const char	*deviceTypeStr,
													const int	deviceNum,
													const int	keywordEnum,
													const char	*valueString);
		virtual	bool	AlpacaProcessReadAll(	const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString);
		virtual	void	ProcessReadAll_IMU(		const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString);
		virtual	void	ProcessReadAll_SaveAs(	const char	*deviceTypeStr,
												const int	deviceNum,
												const char	*keywordString,
												const char	*valueString);


//		virtual	bool	AlpacaGetStatus(void);
		virtual	void	AlpacaGetCapabilities(void);
		virtual	bool	AlpacaGetStatus_OneAAT(void);	//*	One At A Time
				bool	AlpacaGetStatus_Gain(void);
				bool	AlpacaGetStatus_Exposure(void);
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

				void				ToggleLiveMode(void);
				void				ToggleAutoExposure(void);
				void				ToggleDisplayImage(void);
				void				ToggleSaveAll(void);
				TYPE_ASCOM_STATUS	ToggleCooler(void);
				void				ToggleFlipMode(bool toggleHorz, bool toggleVert);
				TYPE_ASCOM_STATUS	SetCCDtargetTemperature(double newCCDtargetTemp);
				void				StartExposure(void);
				void				SetObjectText(const char *newObjectText, const char *newPrefixText);
				void				LogCameraTemp(const double cameraTemp);

			#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
				cv::Mat		*DownloadImage_rgbarray(void);
				cv::Mat		*DownloadImage_imagearray(const bool force8BitRead, const bool allowBinary);
				cv::Mat		*DownloadImage(const bool force8BitRead,  const bool allowBinary);
			#else
				IplImage	*DownloadImage_rgbarray(void);
				IplImage	*DownloadImage_imagearray(const bool force8BitRead, const bool allowBinary);
				IplImage	*DownloadImage(const bool force8BitRead,  const bool allowBinary);
			#endif

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
				bool					cHas_SaveAll;
				bool					cHas_Flip;

				bool					cSaveAsFITS;
				bool					cSaveAsJPEG;
				bool					cSaveAsPNG;
				bool					cSaveAsRaw;


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
				int		AlpacaGetImageArray_JSON(	TYPE_ImageArray	*imageArray,
													int				arrayLength,
													int				*actualValueCnt);
				void	AlpacaGetImageArray_Binary_Byte(	TYPE_ImageArray	*imageArray,
															int				arrayLength);

				void	AlpacaGetImageArray_Binary_Int16(	TYPE_ImageArray	*imageArray,
															int				arrayLength);

				void	AlpacaGetImageArray_Binary_Int32(	TYPE_ImageArray	*imageArray,
															int				arrayLength);

				int		AlpacaGetImageArray_Binary(			TYPE_ImageArray	*imageArray,
															int				arrayLength,
															int				*actualValueCnt);

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
				TYPE_FilenameOptions	cFN;

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
