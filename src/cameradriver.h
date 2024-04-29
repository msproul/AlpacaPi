//**************************************************************************
//*	Name:			cameradriver.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Aug 26,	2019	<MLS> Created cameradriver.h
//*	Oct  2,	2019	<MLS> Added image data buffer to base class
//*	Nov  2,	2019	<MLS> Added _ENABLE_FITS_ compile flag
//*	Nov  7,	2019	<MLS> Changed exposure time start from time_t to struct timeval
//*	Dec 24,	2019	<MLS> Added _INCLUDE_HISTOGRAM_
//*	Dec 24,	2019	<MLS> Added CalculateHistogram()
//*	Jan  6,	2020	<MLS> Added TYPE_IMAGE_MODE
//*	Jan 29,	2020	<MLS> Added _ENABLE_JPEGLIB_
//*	Feb  8,	2020	<MLS> Added SetImageTypeCameraOpen()
//*	Mar  3,	2020	<MLS> Added TYPE_SUPPORTED_IMG_TYPE
//*	Nov 29,	2020	<MLS> Updated return values to TYPE_ASCOM_STATUS
//*	Dec 11,	2020	<MLS> Updating class variable names to match ASCOM property names
//*	Feb 21,	2021	<MLS> Deleted TYPE_SUPPORTED_IMG_TYPE
//*	Jun 23,	2021	<MLS> Added kCmd_Camera_subexposureduration
//*	Feb 27,	2022	<MLS> Changed cOpenCV_Image to cOpenCV_ImagePtr
//*	May  5,	2022	<MLS> Added cOffsetSupported flag
//*	May 15,	2022	<MLS> Added cSubDurationSupported flag
//*	Oct  9,	2022	<MLS> Added cCameraIsSiumlated flag
//*	Jun  4,	2023	<MLS> Added cSaveAsFITS, cSaveAsJPEG, cSaveAsPNG, cSaveAsRAW
//*	Aug 31,	2023	<MLS> Adding support for GPS, specifically the QHY174-GPS
//*	Apr 19,	2024	<MLS> Added kImageType_MONO8
//*****************************************************************************
//#include	"cameradriver.h"

#ifndef	_CAMERA_DRIVER_H_
#define	_CAMERA_DRIVER_H_

//#define _USE_CAMERA_READ_THREAD_

#define _INCLUDE_ALPACA_EXTRAS_

#include	<sys/time.h>
#include	<time.h>
#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif

//*	moved to make file
//	#define		_ENABLE_FITS_

#define		_INCLUDE_HISTOGRAM_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif

#ifdef _ENABLE_FITS_
	#ifndef _FITSIO_H
		#include <fitsio.h>
	#endif // _FITSIO_H
	#define	kMaxFitsRecLen	90
	#define	kMaxFitsRecords	200
	//*****************************************************************************
	typedef struct	//	TYPE_FITS_RECORD
	{
		char	fitsRec[kMaxFitsRecLen];
	} TYPE_FITS_RECORD;

#endif // _ENABLE_FITS_


#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif

#if defined(_ENABLE_FILTERWHEEL_) || defined(_ENABLE_FILTERWHEEL_ZWO_) || defined(_ENABLE_FILTERWHEEL_ATIK_)
	#include	"filterwheeldriver.h"
#endif

#ifdef _ENABLE_FOCUSER_
	#include	"focuserdriver.h"
#endif


#ifdef _ENABLE_ROTATOR_
	#include	"rotatordriver.h"
#endif

#ifndef _GPS_DATA_H_
	#include	"gps_data.h"
#endif


#include	"observatory_settings.h"

#include	"camera_defs.h"

#define	kDefaultImageDataDir	"imagedata"
extern	char	gImageDataDir[];

//*****************************************************************************
//*	Camera states
//*	this is for the state machine to keep track of what each camera is doing
//*	this is DIFFERENT than the alpaca state, however there is a correlation between the 2
//*****************************************************************************
typedef enum
{
	kCameraState_Idle	=	0,
	kCameraState_TakingPicture,
	kCameraState_StartVideo,
	kCameraState_TakingVideo,

	kCameraState_last
} TYPE_CAMERA_STATE;



//*****************************************************************************
//*	Image mode,
//*		Single means single frame
//*		Sequence is a finate series with an optional delay between them
//*		Live is every frame as fast as we can
//*
//*****************************************************************************
typedef enum
{
	kImageMode_Single	=	0,
	kImageMode_Sequence,
	kImageMode_Live,

	kImageMode_Last

} TYPE_IMAGE_MODE;



//*****************************************************************************
typedef enum
{
	kExposure_Idle	=	0,
	kExposure_Working,
	kExposure_Success,
	kExposure_Failed,

	kExposure_Unknown,

	kExposure_Last
} TYPE_EXPOSURE_STATUS;

//*****************************************************************************
typedef enum
{
	//*	these are in the same order as the ZWO values
	kBAYER_PAT_RG	=	0,
	kBAYER_PAT_BG,
	kBAYER_PAT_GR,
	kBAYER_PAT_GB
} TYPE_BAYER_PAT;



//*****************************************************************************
typedef enum
{
	//*	these are NOT the same order as the ZWO values
	//*	these are so we can work across multiple camera brands AND still work with Alpaca
	kImageType_Invalid	=	-1,
	kImageType_RAW8		=	0,
	kImageType_RAW16,
	kImageType_RGB24,
	kImageType_Y8,
	kImageType_MONO8,

	kImageType_last
} TYPE_IMAGE_TYPE;


//*****************************************************************************
typedef struct	//	TYPE_IMAGE_ROI_Info
{
	TYPE_IMAGE_TYPE	currentROIimageType;
	int				currentROIwidth;
	int				currentROIheight;
	int				currentROIbin;
} TYPE_IMAGE_ROI_Info;


//*****************************************************************************
//*	this is for keeping track of other saved data for the FITS header
#define		kMaxFileNameLen		128
#define		kMaxFNcommentLen	32
#define		kMaxDataProducts	6
typedef struct	//	TYPE_FILENAME
{
	char			filename[kMaxFileNameLen];
	char			comment[kMaxFNcommentLen];
} TYPE_FILENAME;


//#define	kNumSupportedFormats	8
#define	kMaxCameraNameLen		64
#define	kObjectNameMaxLen		31
#define	kTelescopeNameMaxLen	80
#define	kFileNamePrefixMaxLen	16

#define	kAuxiliaryTextMaxLen	128

#define	SAVE_AVI	true



//**************************************************************************************
//*	image flip, this is the ZWO definition, we will adopt that
//*	Flip: 0->None 1->Horiz 2->Vert 3->Both
enum
{
	kFlip_None	=	0,
	kFlip_Horiz,
	kFlip_Vert,
	kFlip_Both
};



//**************************************************************************************
class CameraDriver: public AlpacaDriver
{
	public:

		//
		// Construction
		//
									CameraDriver(void);
		virtual						~CameraDriver(void);
		virtual	bool				AlpacaConnect(void);
		virtual	bool				AlpacaDisConnect(void);
		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
		virtual bool				GetCommandArgumentString(const int cmdNumber, char *agumentString, char *commentString);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);
		virtual	int32_t	RunStateMachine(void);
				int32_t	RunStateMachine_Idle(void);
				int		RunStateMachine_TakingPicture(void);
		virtual	void	RunStateMachine_Device(void);

		virtual	bool	DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen);

				void	ProcessExposureOptions(TYPE_GetPutRequestData *reqData);

				void	SetCommonPropertyName(const char *namePrefix, const char *newName);
				void	SetSerialNumInFileName(bool enable);
				void	SetObjectName(const char *newObjectName);
				void	SetTelescopeName(const char *newTelescopeName);
				void	SetInstrumentName(const char *newInstrumentName);
				void	SetFileNamePrefix(const char *newFNprefix);
				void	SetFileNameSuffix(const char *newFNprefix);

				void	SaveImageData(void);
				void	SaveNextImage(void);
				void	SetLastExposureInfo(void);
	protected:
		//*	Camera routines for all cameras
		//*	the functions starting with "Get" and "Put" generate the JSON msg
		TYPE_ASCOM_STATUS	Get_BayerOffsetX(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_BayerOffsetY(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);


		TYPE_ASCOM_STATUS	Get_BinX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_BinY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_BinX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_BinY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Camerastate(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_CanAbortExposure(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanAsymmetricBin(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanFastReadout(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanGetCoolerPower(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanPulseGuide(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanSetCCDtemperature(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_CanStopExposure(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);


		TYPE_ASCOM_STATUS	Get_CCDtemperature(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Cooleron(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Cooleron(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_CoolerPower(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_ElectronsPerADU(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_Exposuremax(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Exposuremin(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_Fullwellcapacity(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Fastreadout(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Fastreadout(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_Gain(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Gain(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_GainMax(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_GainMin(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_Gains(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_HeatSinkTemperature(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_IsPulseGuiding(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_MaxADU(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_MaxBinX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_MaxBinY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_NumX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_NumY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_NumX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_NumY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_Offset(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Offset(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_OffsetMax(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_OffsetMin(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Offsets(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);


		TYPE_ASCOM_STATUS	Get_PercentCompleted(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_PixelSizeX(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_PixelSizeY(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);


		TYPE_ASCOM_STATUS	Put_Pulseguide(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_Readoutmode(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Readoutmode(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Readoutmodes(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_SensorName(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Sensortype(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_SetCCDtemperature(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg,	const char *responseString);
		TYPE_ASCOM_STATUS	Put_SetCCDtemperature(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_StartX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_StartY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_StartX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_StartY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_Lastexposureduration(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg,	const char *responseString);
		TYPE_ASCOM_STATUS	Get_Lastexposurestarttime(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg,	const char *responseString);
		TYPE_ASCOM_STATUS	Get_ImageReady(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg,	const char *responseString);

		TYPE_ASCOM_STATUS	Get_Imagearray(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_StartExposure(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_StopExposure(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_AbortExposure(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_SubExposureDuration(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg,	const char *responseString);
		TYPE_ASCOM_STATUS	Put_SubExposureDuration(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		TYPE_ASCOM_STATUS	Get_Imagearray_JSON(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Imagearray_Binary(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		int					BuildBinaryImage_Raw8(			unsigned char	*binaryDataBuffer, int startOffset, int bufferSize);
		int					BuildBinaryImage_Raw8_16bit(	unsigned char	*binaryDataBuffer, int startOffset, int bufferSize);
		int					BuildBinaryImage_Raw8_32bit(	unsigned char	*binaryDataBuffer, int startOffset, int bufferSize);
		int					BuildBinaryImage_Raw16(			unsigned char	*binaryDataBuffer, int startOffset, int bufferSize);
		int					BuildBinaryImage_Raw32(			unsigned char	*binaryDataBuffer, int startOffset, int bufferSize);
		int					BuildBinaryImage_RGB24(			unsigned char	*binaryDataBuffer, int startOffset, int bufferSize);
		int					BuildBinaryImage_RGB24_32bit(	uint32_t		*binaryDataBuffer, int startOffset, int bufferSize);
		int					BuildBinaryImage_RGBx16(		unsigned char	*binaryDataBuffer, int startOffset, int bufferSize);

		//-------------------------------------------------------------------------------------------------
		//*	Added by MLS
		TYPE_ASCOM_STATUS	Get_LiveMode(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_LiveMode(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Sidebar(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Sidebar(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		TYPE_ASCOM_STATUS	Get_Flip(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Flip(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		TYPE_ASCOM_STATUS	Get_ExposureTime(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_ExposureTime(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_SaveAllImages(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_SaveAllImages(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		//------------------------------------------
		//*	Save as routines
		TYPE_ASCOM_STATUS	Get_SaveAsFITS(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_SaveAsFITS(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_SaveAsJPEG(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_SaveAsJPEG(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_SaveAsPNG(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_SaveAsPNG(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_SaveAsRAW(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_SaveAsRAW(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_SavedImages(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);



		TYPE_ASCOM_STATUS	Put_SaveNextImage(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_StartSequence(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Put_TelescopeInfo(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Filelist(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Put_StartVideo(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_StopVideo(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_AutoExposure(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_AutoExposure(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		TYPE_ASCOM_STATUS	Get_DisplayImage(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_DisplayImage(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Put_Filenameoptions(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_RGBarray(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Readall(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		//*	these are borrowed from the telescope device
		TYPE_ASCOM_STATUS	Get_ApertureArea(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_ApertureDiameter(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_FocalLength(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);



				bool	AllocateImageBuffer(long bufferSize);

				void	GenerateFileNameRoot(void);
				void	WriteFireCaptureTextFile(void);
				void	WriteIMUtextFile(void);


				void	Send_imagearray_rgb24(	const int		socketFD,
												unsigned char	*pixelPtr,
												const int		numRows,
												const int		numClms,
												const int		pixelCount);
				void	Send_imagearray_raw8(	const int		socketFD,
												unsigned char	*pixelPtr,
												const int		numRows,
												const int		numClms,
												const int		pixelCount);
				void	Send_imagearray_raw16(	const int		socketFD,
												uint16_t		*pixelPtr,
												const int		numRows,
												const int		numClms,
												const int		pixelCount);

				void	Send_RGBarray_rgb24(const int socketFD, unsigned char *pixelPtr, const int pixelCount);
				void	Send_RGBarray_raw8(const int socketFD, unsigned char *pixelPtr, const int pixelCount);

			#ifdef _ENABLE_FITS_
				int		SaveImageAsFITS(bool headerOnly=false);
				void	CreateFitsBGRimage(void);
				void	WriteFITS_Seperator(fitsfile *fitsFilePtr, const char *blockName);

				void	WriteFITS_CameraInfo(		fitsfile *fitsFilePtr);
				void	WriteFITS_EnvironmentInfo(	fitsfile *fitsFilePtr);
				void	WriteFITS_FilterwheelInfo(	fitsfile *fitsFilePtr);
				void	WriteFITS_FocuserInfo(		fitsfile *fitsFilePtr);
				void	WriteFITS_ObservationInfo(	fitsfile *fitsFilePtr, bool includeAnalysis);
				void	WriteFITS_ObservatoryInfo(	fitsfile *fitsFilePtr);
				void	WriteFITS_RotatorInfo(		fitsfile *fitsFilePtr);
				void	WriteFITS_SoftwareInfo(		fitsfile *fitsFilePtr);
				void	WriteFITS_TelescopeInfo(	fitsfile *fitsFilePtr);
				void	WriteFITS_VersionInfo(		fitsfile *fitsFilePtr);
				void	WriteFITS_MoonInfo(			fitsfile *fitsFilePtr);
				void	WriteFITS_GPSinfo(			fitsfile *fitsFilePtr);
				void	WriteFITS_QHY_GPSinfo(		fitsfile *fitsFilePtr);
				void	WriteFITS_Global_GPSinfo(	fitsfile *fitsFilePtr);

			#ifdef _ENABLE_IMU_
				void	WriteFITS_IMUinfo(			fitsfile *fitsFilePtr);
			#endif

				TYPE_ASCOM_STATUS	Get_FitsHeader(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
				int					ExtractFitsHeader(fitsfile *fitsFilePtr);
				TYPE_FITS_RECORD	cFitsHeader[kMaxFitsRecords];

			#endif // _ENABLE_FITS_
			#ifdef _ENABLE_IMU_
				void	ReadIMUdata(void);
				bool	cIMU_EulerValid;
				bool	cIMU_QuatValid;
				double	cIMU_Heading;
				double	cIMU_Roll;
				double	cIMU_Pitch;

				double	cIMU_www;
				double	cIMU_xxx;
				double	cIMU_yyy;
				double	cIMU_zzz;
				int		cIMU_Cal_Gyro;
				int		cIMU_Cal_Acce;
				int		cIMU_Cal_Magn;
				int		cIMU_Cal_Syst;
			#endif

			#ifdef _ENABLE_JPEGLIB_
				void	SaveUsingJpegLib(void);
			#endif	//	_ENABLE_JPEGLIB_
				void	SaveUsingPNGlib(void);

				void	AutoAdjustExposure(void);
				void	CheckPulseGuiding(void);
				int		GetPrecentCompleted(void);

	public:
	#ifdef _USE_OPENCV_
		//*	new live window as of 4/1/2021
		virtual	TYPE_ASCOM_STATUS		OpenLiveWindow(char *alpacaErrMsg);
//-		virtual	TYPE_ASCOM_STATUS		CloseLiveWindow(char *alpacaErrMsg);
		virtual	void					UpdateLiveWindow(void);

		void			OpenLiveImage(void);
		void			CloseLiveImage(void);

		void			DisplayLiveImage(void);
		void			DisplayLiveImage_wSideBar(void);
		int				CreateOpenCVImage(const unsigned char *imageDataPtr);
		int				SaveOpenCVImage(void);
		void			SetOpenCVcallbackFunction(const char *windowName);
		void			ProcessMouseEvent(int event, int xxx, int yyy, int flags);
		void			DrawOpenCVoverlay(void);

	#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		void			DrawSidebar(			cv::Mat *imageDisplay);
		void			CreateHistogramGraph(	cv::Mat *imageDisplay);
		void			SetOpenCVcolors(		cv::Mat *imageDisplay);
		void			Draw3TextStrings(		cv::Mat *theImage, const char *textStr1, const char *textStr2, const char *textStr3);
	#else
		void			DrawSidebar(			IplImage *imageDisplay);
		void			CreateHistogramGraph(	IplImage *imageDisplay);
		void			SetOpenCVcolors(		IplImage *imageDisplay);
		void			Draw3TextStrings(		IplImage *theImage, const char *textStr1, const char *textStr2, const char *textStr3);
	#endif // _USE_OPENCV_CPP_

	#endif	//	_USE_OPENCV_
		//*****************************************************************************
		//*	image analysis routines
		uint32_t		CalculateMaxPixValue(void);
		uint32_t		CalculateMinPixValue(void);
		uint32_t		CountSaturationPixels(void);
		double			CalculateSaturation(void);
		float			CalculateHistogramMax(void);

		//*****************************************************************************

				TYPE_ASCOM_STATUS		Set_ExposureTime(int32_t exposureMicrosecs);

		//*****************************************************************************
		//*	Camera specific routines
		virtual	bool					IsCameraIDvalid(const int argCameraID);
		virtual	void					ResetCamera(void);
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame=true);
				TYPE_ASCOM_STATUS		Start_CameraExposure(void);
		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);
		virtual	TYPE_ASCOM_STATUS		Abort_Exposure(void);
		virtual	TYPE_EXPOSURE_STATUS	Check_Exposure(bool verboseFlag = false);

		virtual	TYPE_ASCOM_STATUS		SetImageTypeCameraOpen(TYPE_IMAGE_TYPE newImageType);
		virtual	TYPE_ASCOM_STATUS		SetImageType(TYPE_IMAGE_TYPE newImageType);
		virtual	TYPE_ASCOM_STATUS		SetImageType(char *newImageTypeString);

		//*	write_binXY is for ease of implementation for those cameras that do not support asymmetric binning
		virtual	TYPE_ASCOM_STATUS		Write_BinXY(const int newBinXvalue);
		virtual	TYPE_ASCOM_STATUS		Write_BinX(const int newBinXvalue);
		virtual	TYPE_ASCOM_STATUS		Write_BinY(const int newBinYvalue);


		virtual	TYPE_ASCOM_STATUS		Read_Gain(int *cameraGainValue);
		virtual	TYPE_ASCOM_STATUS		Write_Gain(const int newGainValue);

		virtual	TYPE_ASCOM_STATUS		Read_Offset(int *cameraOffsetValue);
		virtual	TYPE_ASCOM_STATUS		Write_Offset(const int newOffsetValue);

		virtual	TYPE_ASCOM_STATUS		Start_Video(void);
		virtual	TYPE_ASCOM_STATUS		Stop_Video(void);
		virtual	TYPE_ASCOM_STATUS		Take_Video(void);

		virtual	TYPE_ASCOM_STATUS		SetFlipMode(const int newFlipMode);

		virtual	TYPE_ALPACA_CAMERASTATE	Read_AlapcaCameraState(void);



		virtual	bool					GetImage_ROI_info(void);

		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOn(void);
		virtual	TYPE_ASCOM_STATUS		Cooler_TurnOff(void);
		virtual	TYPE_ASCOM_STATUS		Read_SensorTemp(void);

		virtual	TYPE_ASCOM_STATUS		Read_SensorTargetTemp(void);
		virtual	TYPE_ASCOM_STATUS		Write_SensorTargetTemp(const double newCCDtargetTemp);


		virtual	TYPE_ASCOM_STATUS		Read_CoolerState(bool *coolerOnOff);
		virtual	TYPE_ASCOM_STATUS		Read_CoolerPowerLevel(void);
		virtual	TYPE_ASCOM_STATUS		Read_Fastreadout(void);
		virtual	TYPE_ASCOM_STATUS		Read_ImageData(void);



				TYPE_ASCOM_STATUS	Read_Readoutmodes(char *readOutModeString, bool includeQuotes=false);
				void				AddReadoutModeToList(const TYPE_IMAGE_TYPE imageType, const char *imgTypeStr=NULL);

				void				DumpCameraProperties(const char *callingFunctionName);
				void				CreateFakeImageData(unsigned char *cameraDataPtr, int imageWith, int imageHeight, int bytesPerPixel);

		//-------------------------------------------------------------------------
		//*	this is for the setup function
		virtual	bool					Setup_OutputForm(TYPE_GetPutRequestData *reqData, const char *formActionString);
		virtual void					Setup_SaveInit(void);
		virtual void					Setup_SaveFinish(void);
		virtual	bool					Setup_ProcessKeyword(const char *keyword, const char *valueString);
				TYPE_FilenameOptions	cSetupFileNameOptions;
				char					cSetupOtherPrefix[32];


	//*****************************************************************************
public:
//-	char					cSensorName[kMaxSensorNameLen];	//*	obtained from my table lookup
	char		cLastCameraErrMsg[128];

	//*****************************************************************************
protected:
	//*	ASCOM camera properties
	TYPE_CameraProperties	cCameraProp;

	bool					cResponseIsJSON;		//*	this is for the binary option in imageArray

	//*****************************************************************************
	TYPE_IMAGE_ROI_Info		cLastExposure_ROIinfo;

	//=========================================================================================
	//=========================================================================================
	//=========================================================================================
	//=========================================================================================
	//*	non-alpaca stuff


	bool		cCameraIsSiumlated;
	bool		cUpdateOtherDevices;
	bool		cCanFlipImage;
	int			cFlipMode;
	long		cExposureDefault_us;	//*	micro-seconds
	//=========================================================================================
	bool		cTempReadSupported;		//*	true if temperature can be read from device
	bool		cOffsetSupported;		//*	true pixel value offset is supported
	bool		cSubDurationSupported;
	long		cCoolerState;


	int			cCameraID;				//*	this is used to control everything of the camera in other functions.Start from 0.
	bool		cCameraIsOpen;			//*	flag to tell us the camera is open
	int			cBayerPattern;
	bool		cIsColorCam;			//*	false= monochrome, true = color
	bool		cIsCoolerCam;
	bool		cIsUSB3Host;
	bool		cIsUSB3Camera;
	int			cBitDepth;
	bool		cIsTriggerCam;

	//===================================
	//*	pulse guiding information
	bool			cSt4Port;
	struct timeval	cPulseGuideStartTime;		//*	time pulse guiding was started

	long			cGain_default;


	bool			cCanRead8Bit;
	long			cHighSpeedMode;

	//*****************************************************************************
	TYPE_CAMERA_STATE		cInternalCameraState;
	TYPE_IMAGE_ROI_Info		cROIinfo;
	TYPE_IMAGE_TYPE			cDesiredImageType;


	//*****************************************************************************
	bool				cNewImageReadyToDisplay;
	long				cCameraDataBuffLen;
	unsigned char		*cCameraDataBuffer;
	unsigned char		*cCameraBGRbuffer;			//*	Blue, Green, Red, for FITS

	int					cAVIfourCC;					//*	the fourCC mode used in the avi file

	bool				cCameraAutoExposure;		//*	true if the camera is doing the auto exposure
	int32_t				cCurrentExposure_us;		//*	micro seconds

	int					cExposureFailureCnt;

	bool				cAutoAdjustExposure;		//*	true if we are doing auto exposure
	long				cAutoAdjustStepSz_us;		//*	step size in micro seconds
	TYPE_IMAGE_MODE		cImageMode;
	uint32_t			cSequenceDelay_us;			//*	sequence delay in microseconds
	int32_t				cSeqDeltaExposure_us;
	int					cImageSeqNumber;
	bool				cDisplayImage;
	bool				cSaveNextImage;				//*	this will get reset each time an image is taken
	bool				cSaveAllImages;
	long				cWorkingLoopCnt;
	long				cFramesRead;
	double				cFrameRate;					//*	primarily used in live mode
	//*****************************************************************************
	bool				cSaveAsFITS;
	bool				cSaveAsJPEG;
	bool				cSaveAsPNG;
	bool				cSaveAsRAW;
	int					cNumFramesRequested;
	int					cNumFramesToSave;
//	int					cNumFramesSaved;

	int					cTotalFramesSaved;

	//===========================================================================
	//*	data for taking video
	int					cNumVideoFramesSaved;
	double				cVideoDuration_secs;		//*	how many seconds to record
	uint32_t			cVideoStartTime;			//*	time video was started for frame rate calculations (seconds)
	bool				cVideoCreateTimeStampFile;
	FILE				*cVideoTimeStampFilePtr;


	struct timeval		cDownloadStartTime;
	struct timeval		cDownloadEndTime;

	TYPE_TELESCOPE_INFO	cTS_info;		//*	TeleScope info

	char				cTelescopeModel[kTelescopeNameMaxLen + 1];

	//===========================================================================
	//*	File name information
	TYPE_FilenameOptions	cFN;

	char					cObjectName[kObjectNameMaxLen + 1];
	char					cFileNameRoot[256];
	char					cLastJpegImageName[256];
	char					cFileNamePrefix[kFileNamePrefixMaxLen + 1];
	char					cFileNameSuffix[kFileNamePrefixMaxLen + 1];

	char					cAuxTextTag[kAuxiliaryTextMaxLen];		//*	auxiliary text information


#ifdef _USE_OPENCV_
	//===========================================================================
	bool				cCreateOpenCVwindow;
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	cv::Mat				*cOpenCV_ImagePtr;
	cv::Mat				*cOpenCV_LiveDisplayPtr;
	cv::Mat				*cOpenCV_Histogram;
	cv::VideoWriter		*cOpenCV_videoWriter;
#else
	IplImage			*cOpenCV_ImagePtr;
	IplImage			*cOpenCV_LiveDisplayPtr;
	IplImage			*cOpenCV_Histogram;
	CvVideoWriter		*cOpenCV_videoWriter;
#endif // _USE_OPENCV_CPP_
#ifdef _ENABLE_CVFONT_
	CvFont				cTextFont;
	CvFont				cOverlayTextFont;
#endif // _ENABLE_CVFONT_
	bool				cCreateHistogramWindow;
	cv::Scalar			cVideoOverlayColor;

	char				cOpenCV_ImgWindowName[128];
	bool				cOpenCV_ImgWindowValid;

	const static int	cSideBarWidth	=	290;
	const static int	cSideFrameWidth	=	16;
	int					cLiveDisplayWidth;
	int					cLiveDisplayHeight;
	bool				cDisplayCrossHairs;
	int					cCrossHairX;
	int					cCrossHairY;

	bool				cLeftButtonDown;
	bool				cRightButtonDown;
	int					cLastLClickX;
	int					cLastLClickY;

	int					cCurrentMouseX;
	int					cCurrentMouseY;

	bool				cDrawRectangle;

	cv::Scalar			cSideBarBGcolor;
	cv::Scalar			cSideBarTXTcolor;
	cv::Scalar			cSideBarBlk;
	cv::Scalar			cSideBarRed;
	cv::Scalar			cSideBarGrn;
	cv::Scalar			cSideBarBlu;
	cv::Scalar			cSideBarGry;
	cv::Scalar			cSideBarFCblue;
	cv::Scalar			cCrossHairColor;


#endif // _USE_OPENCV_


#ifdef _USE_CAMERA_READ_THREAD_
protected:
	virtual	void				RunThread_Startup(void);
	virtual	void				RunThread_Loop(void);
	virtual	void				RunThread_CheckPictureStatus(void);
public:
#endif // _USE_CAMERA_READ_THREAD_


#if defined(_ENABLE_FILTERWHEEL_) || defined(_ENABLE_FILTERWHEEL_ZWO_) || defined(_ENABLE_FILTERWHEEL_ATIK_)
	void				UpdateFilterwheelLink(void);
	FilterwheelDriver	*cConnectedFilterWheel;
	int					cFilterWheelCurrPos;
	char				cFilterWheelCurrName[48];
#endif


#ifdef _ENABLE_FOCUSER_
	void				UpdateFocuserLink(void);
	FocuserDriver		*cConnectedFocuser;
#endif // _ENABLE_FOCUSER_

#ifdef	_ENABLE_ROTATOR_
	void				UpdateRotatorLink(void);
	RotatorDriver		*cConnectedRotator;
#endif // _ENABLE_ROTATOR_

	bool				cFocuserInfoValid;
	bool				cRotatorInfoValid;
	bool				cFilterWheelInfoValid;

	void			AddToDataProductsList(const char *newDataProductName, const char *newDatacomment=NULL);
	TYPE_FILENAME	cOtherDataProducts[kMaxDataProducts];
	int				cOtherDataCnt;


#ifdef _INCLUDE_HISTOGRAM_
	//*****************************************************************************
	//*	image analysis data
	void		CalculateHistogramArray(void);
	void		SaveHistogramFile(void);

	int32_t		cHistogramLum[256];
	int32_t		cHistogramRed[256];
	int32_t		cHistogramGrn[256];
	int32_t		cHistogramBlu[256];
	int32_t		cMinHistogramValue;
	int32_t		cMaxHistogramValue;
	int32_t		cPeakHistogramValue;
	int32_t		cMaxHistogramPixCnt;

	uint8_t		cMaxRedValue;
	uint8_t		cMaxGrnValue;
	uint8_t		cMaxBluValue;
	uint8_t		cMaxGryValue;

#endif // _INCLUDE_HISTOGRAM_
	//===========================================================================
	//*	Overlay info
	//*	this is for overlaying GPS data onto the image primarily for occultation use
	//*	however, it could have other uses as well
	uint8_t		cOverlayMode;	//*	0 = none
	uint8_t		cOverlayPosition;
	uint8_t		cOverlayColor;
	void		DrawOverlayOntoImage(void);

	//===========================================================================
	//*	GPS info
	//*	currently the only camera that has a GPS is the QHY174-GPS
	void				GPS_ResetNMEAbuffer(void);
	void				GPS_AddNMEAstring(const char *nmeaString);
	TYPE_QHY_GPSdata	cGPS;
};


//*****************************************************************************
typedef struct	//	TYPE_SensorName
{
	char	cameraModel[16];
	char	sensorName[kMaxSensorNameLen];

}	TYPE_SensorName;

int		CreateCameraObjects(void);

//extern	const TYPE_CmdEntry	gCameraCmdTable[];
extern	const char			*gCameraStateStrings[];

void	GetImageTypeString(TYPE_IMAGE_TYPE imageType, char *imageTypeString);
void	*StartCameraReadThread(void *arg);

#endif		//	_CAMERA_DRIVER_H_
