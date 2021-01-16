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
//*****************************************************************************
//#include	"cameradriver.h"

#ifndef	_CAMERA_DRIVER_H_
#define	_CAMERA_DRIVER_H_

#define _INCLUDE_ALPACA_EXTRAS_


#include	<sys/time.h>
#include	<time.h>

//*	moved to make file
//	#define		_ENABLE_FITS_
#define		_INCLUDE_HISTOGRAM_


#ifdef _ENABLE_FITS_
	#ifndef _FITSIO_H
		#include <fitsio.h>
	#endif // _FITSIO_H
#endif // _ENABLE_FITS_


#ifndef _ALPACA_DRIVER_H_
	#include	"alpacadriver.h"
#endif

#ifdef _USE_OPENCV_
	#ifndef __OPENCV_OLD_HIGHGUI_H__
		#include "opencv/highgui.h"
	#endif
	#ifndef __OPENCV_HIGHGUI_H__
		#include "opencv2/highgui/highgui_c.h"
	#endif
#endif

#ifdef _ENABLE_FILTERWHEEL_
	#include	"filterwheeldriver.h"
#endif // _ENABLE_FILTERWHEEL_

#ifdef _ENABLE_FOCUSER_
	#include	"focuserdriver.h"
#endif // _ENABLE_FOCUSER_


#ifdef _ENABLE_ROTATOR_
	#include	"rotatordriver.h"
#endif // _ENABLE_ROTATOR_


#include	"observatory_settings.h"
#include	"alpaca_defs.h"

#define	kImageDataDir	"imagedata"


//*****************************************************************************
//*	Camera states
//*	this is for the state machine to keep track of what each camera is doing
//*	this is DIFFERENT than the alpca state, however there is a correlation between the 2
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
	kImageType_Invalid	=	-1,
	kImageType_RAW8		=	0,
	kImageType_RAW16,
	kImageType_RGB24,
	kImageType_Y8,

	kImageType_last
} TYPE_IMAGE_TYPE;


//*****************************************************************************
typedef struct
{
	TYPE_IMAGE_TYPE	currentROIimageType;
	int				currentROIwidth;
	int				currentROIheight;
	int				currentROIbin;
} TYPE_IMAGE_ROI_Info;

//*****************************************************************************
#define	kImgTypeStrMaxLen	16
typedef struct
{
	bool				isValid;
	char				imageTypeString[kImgTypeStrMaxLen];
	TYPE_IMAGE_TYPE		internalImgageType;

} TYPE_SUPPORTED_IMG_TYPE;
#define		kMaxImageTypes	6

//*****************************************************************************
//*	this is for keeping track of other saved data for the FITS header
#define		kMaxFileNameLen		128
#define		kMaxFNcommentLen	32
#define		kMaxDataProducts	6
typedef struct
{
	char			filename[kMaxFileNameLen];
	char			comment[kMaxFNcommentLen];
} TYPE_FILENAME;


#define	kNumSupportedFormats	8
#define	kMaxCameraNameLen		64
#define	kMaxSensorNameLen		32
#define	kObjectNameMaxLen		31
#define	kTelescopeNameMaxLen	80
#define	kFileNamePrefixMaxLen	16

#define	kAuxiliaryTextMaxLen	128

#define	SAVE_AVI	true



//*****************************************************************************
//*	Camera commands
enum
{
	kCmd_Camera_bayeroffsetX	=	0,	//*	Returns the X offset of the Bayer matrix.
	kCmd_Camera_bayeroffsetY,			//*	Returns the Y offset of the Bayer matrix.
	kCmd_Camera_binX,					//*	Returns the binning factor for the X axis.
										//*	Sets the binning factor for the X axis.
	kCmd_Camera_binY,					//*	Returns the binning factor for the Y axis.
										//*	Sets the binning factor for the Y axis.
	kCmd_Camera_camerastate,			//*	Returns the camera operational state.
	kCmd_Camera_cameraxsize,			//*	Returns the width of the CCD camera chip.
	kCmd_Camera_cameraysize,			//*	Returns the height of the CCD camera chip.
	kCmd_Camera_canabortexposure,		//*	Indicates whether the camera can abort exposures.
	kCmd_Camera_canasymmetricbin,		//*	Indicates whether the camera supports asymmetric binning
	kCmd_Camera_canfastreadout,			//*	Indicates whether the camera has a fast readout mode.
	kCmd_Camera_cangetcoolerpower,		//*	Indicates whether the camera's cooler power setting can be read.
	kCmd_Camera_canpulseguide,			//*	Returns a flag indicating whether this camera supports pulse guiding
	kCmd_Camera_cansetccdtemperature,	//*	Returns a flag indicating whether this camera supports setting the CCD temperature
	kCmd_Camera_canstopexposure,		//*	Returns a flag indicating whether this camera can stop an exposure that is in progress
	kCmd_Camera_ccdtemperature,			//*	Returns the current CCD temperature
	kCmd_Camera_cooleron,				//*	Returns the current cooler on/off state.
										//*	Turns the camera cooler on and off
	kCmd_Camera_coolerpower,			//*	Returns the present cooler power level
	kCmd_Camera_electronsperadu,		//*	Returns the gain of the camera
	kCmd_Camera_exposuremax,			//*	Returns the maximum exposure time supported by StartExposure.
	kCmd_Camera_exposuremin,			//*	Returns the Minimium exposure time
	kCmd_Camera_exposureresolution,		//*	Returns the smallest increment in exposure time supported by StartExposure.
	kCmd_Camera_fastreadout,			//*	Returns whether Fast Readout Mode is enabled.
										//*	Sets whether Fast Readout Mode is enabled.
	kCmd_Camera_fullwellcapacity,		//*	Reports the full well capacity of the camera
	kCmd_Camera_gain,					//*	Returns an index into the Gains array
										//*	Sets the binning factor for the X axis.
	kCmd_Camera_gainmax,				//*	Maximum value of Gain
	kCmd_Camera_gainmin,				//*	Minimum value of Gain
	kCmd_Camera_gains,					//*	Gains supported by the camera
	kCmd_Camera_hasshutter,				//*	Indicates whether the camera has a mechanical shutter
	kCmd_Camera_heatsinktemperature,	//*	Returns the current heat sink temperature.
	kCmd_Camera_imagearray,				//*	Returns an array of integers containing the exposure pixel values
	kCmd_Camera_imagearrayvariant,		//*	Returns an array of int containing the exposure pixel values
	kCmd_Camera_imageready,				//*	Indicates that an image is ready to be downloaded
	kCmd_Camera_ispulseguiding,			//*	Indicates that the camera is pulse guideing.
	kCmd_Camera_lastexposureduration,	//*	Duration of the last exposure
	kCmd_Camera_lastexposurestarttime,	//*	Start time of the last exposure in FITS standard format.
	kCmd_Camera_maxadu,					//*	Camera's maximum ADU value
	kCmd_Camera_maxbinX,				//*	Maximum binning for the camera X axis
	kCmd_Camera_maxbinY,				//*	Maximum binning for the camera Y axis
	kCmd_Camera_numX,					//*	Returns the current subframe width
										//*	Sets the current subframe width
	kCmd_Camera_numY,					//*	Returns the current subframe height
										//*	Sets the current subframe height
	kCmd_Camera_percentcompleted,		//*	Indicates percentage completeness of the current operation
	kCmd_Camera_pixelsizeX,				//*	Width of CCD chip pixels (microns)
	kCmd_Camera_pixelsizeY,				//*	Height of CCD chip pixels (microns)
	kCmd_Camera_readoutmode,			//*	Indicates the canera's readout mode as an index into the array ReadoutModes
										//*	Set the camera's readout mode
	kCmd_Camera_readoutmodes,			//*	List of available readout modes
	kCmd_Camera_sensorname,				//*	Sensor name
	kCmd_Camera_sensortype,				//*	Type of information returned by the the camera sensor (monochrome or colour)
	kCmd_Camera_setccdtemperature,		//*	Returns the current camera cooler setpoint in degrees Celsius.
										//*	Set the camera's cooler setpoint (degrees Celsius).
	kCmd_Camera_startX,					//*	Return the current subframe X axis start position
										//*	Sets the current subframe X axis start position
	kCmd_Camera_startY,					//*	Return the current subframe Y axis start position
										//*	Sets the current subframe Y axis start position
	kCmd_Camera_abortexposure,			//*	Aborts the current exposure
	kCmd_Camera_pulseguide,				//*	Pulse guide in the specified direction for the specified time.
	kCmd_Camera_startexposure,			//*	Starts an exposure
	kCmd_Camera_stopexposure,			//*	Stops the current exposure

//#ifdef _INCLUDE_ALPACA_EXTRAS_
	//=================================================================
	//*	commands added that are not part of Alpaca
	kCmd_Camera_Extras,

	kCmd_Camera_autoexposure,
	kCmd_Camera_displayimage,

	kCmd_Camera_exposuretime,
	kCmd_Camera_filelist,
	kCmd_Camera_filenameoptions,
	kCmd_Camera_framerate,
	kCmd_Camera_livemode,
	kCmd_Camera_rgbarray,
	kCmd_Camera_settelescopeinfo,
	kCmd_Camera_sidebar,
	kCmd_Camera_savenextimage,
	kCmd_Camera_startsequence,
	kCmd_Camera_startvideo,
	kCmd_Camera_stopvideo,

	//*	keep this one last for consistency with other drivers  (i.e. focuser)
	kCmd_Camera_readall,
//#endif // _INCLUDE_ALPACA_EXTRAS_
	kCmd_Camera_last

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
		virtual	TYPE_ASCOM_STATUS	ProcessCommand(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML(TYPE_GetPutRequestData *reqData);
		virtual	void				OutputHTML_Part2(TYPE_GetPutRequestData *reqData);
		virtual	int32_t	RunStateMachine(void);
				int32_t	RunStateMachine_Idle(void);
				int		RunStateMachine_TakingPicture(void);
		virtual	void	RunStateMachine_Device(void);

				void	ProcessExposureOptions(TYPE_GetPutRequestData *reqData);

				void	SetSerialNumInFileName(bool enable);
				void	SetObjectName(const char *newObjectName);
				void	SetTelescopeName(const char *newTelescopeName);
				void	SetInstrumentName(const char *newInstrumentName);
				void	SetFileNamePrefix(const char *newFNprefix);
				void	SetFileNameSuffix(const char *newFNprefix);

				void	SaveImageData(void);
				void	SaveNextImage(void);
	protected:
		//*	Camera routines for all cameras
		TYPE_ASCOM_STATUS	Get_binX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_binY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_binX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_binY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Camerastate(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

				//*	the functions starting with "Get" and "Put" generate the JSON msg
		TYPE_ASCOM_STATUS	Get_CCDtemperature(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Cooleron(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Cooleron(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_CoolerPowerLevel(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_Exposuremax(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Exposuremin(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_Fullwellcapacity(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Fastreadout(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Fastreadout(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_Gain(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Gain(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_numX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_numY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_numX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_numY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_startX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_startY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_startX(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_startY(				TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Put_Pulseguide(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_Readoutmode(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Readoutmode(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Readoutmodes(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Get_Sensortype(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);

		TYPE_ASCOM_STATUS	Get_SetCCDtemperature(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg,	const char *responseString);
		TYPE_ASCOM_STATUS	Put_SetCCDtemperature(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);

		TYPE_ASCOM_STATUS	Get_Lastexposureduration(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg,	const char *responseString);
		TYPE_ASCOM_STATUS	Get_Lastexposurestarttime(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg,	const char *responseString);
		TYPE_ASCOM_STATUS	Get_ImageReady(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg,	const char *responseString);

		TYPE_ASCOM_STATUS	Get_Imagearray(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_StartExposure(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_StopExposure(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Put_AbortExposure(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


		//-------------------------------------------------------------------------------------------------
		//*	Added by MLS
		TYPE_ASCOM_STATUS	Get_LiveMode(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_LiveMode(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);
		TYPE_ASCOM_STATUS	Get_Sidebar(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_Sidebar(			TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);




		TYPE_ASCOM_STATUS	Get_ExposureTime(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString);
		TYPE_ASCOM_STATUS	Put_ExposureTime(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg);


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

				bool	AllcateImageBuffer(long bufferSize);

				void	WriteFireCaptureTextFile(void);
				void	GenerateFileNameRoot(void);

				void	SetImageTypeIndex(const int alpacaImgTypeIdx, const char *imageTypeString);
				TYPE_IMAGE_TYPE	XlateAlpacaImgIdxToIntImgType(const int alpacaImgTypeIdx);
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

			#endif // _ENABLE_FITS_
			#ifdef _ENABLE_JPEGLIB_
				void	SaveUsingJpegLib(void);
			#endif	//	_ENABLE_JPEGLIB_
				void	SaveUsingPNGlib(void);

				void	AutoAdjustExposure(void);
				void	CheckPulseGuiding(void);

	public:
	#ifdef _USE_OPENCV_
		void			DisplayLiveImage(void);
		void			DisplayLiveImage_wSideBar(void);
		void			DrawSidebar(IplImage *imageDisplay);
		int				CreateOpenCVImage(const unsigned char *imageDataPtr);
		int				SaveOpenCVImage(void);
		void			SetOpenCVcallbackFunction(const char *windowName);
		void			ProcessMouseEvent(int event, int xxx, int yyy, int flags);
		void			DrawOpenCVoverlay(void);
		void			CreateHistogramGraph(IplImage *imageDisplay);
		void			SetOpenCVcolors(IplImage *imageDisplay);
		void			Draw3TextStrings(IplImage *theImage, const char *textStr1, const char *textStr2, const char *textStr3);

	#endif	//	_USE_OPENCV_
		//*****************************************************************************
		//*	image analysis routines
		uint32_t		CalculateMaxPixValue(void);
		uint32_t		CalculateMinPixValue(void);
		uint32_t		CountSaturationPixels(void);
		float			CalculateSaturation(void);
		float			CalculateHistogramMax(void);

		//*****************************************************************************

				TYPE_ASCOM_STATUS		Set_ExposureTime(int32_t exposureMicrosecs);

		//*****************************************************************************
		//*	Camera specific routines
		virtual	bool					IsCameraIDvalid(const int argCameraID);
		virtual	void					ResetCamera(void);
		virtual	TYPE_ASCOM_STATUS		Start_CameraExposure(int32_t exposureMicrosecs);
		virtual	TYPE_ASCOM_STATUS		Stop_Exposure(void);
		virtual	TYPE_EXPOSURE_STATUS	Check_Exposure(bool verboseFlag = false);

		virtual	TYPE_ASCOM_STATUS	SetImageTypeCameraOpen(TYPE_IMAGE_TYPE newImageType);
		virtual	TYPE_ASCOM_STATUS	SetImageType(TYPE_IMAGE_TYPE newImageType);
		virtual	TYPE_ASCOM_STATUS	SetImageType(char *newImageTypeString);

		virtual	TYPE_ASCOM_STATUS	Write_Gain(const int newGainValue);
		virtual	TYPE_ASCOM_STATUS	Read_Gain(int *cameraGainValue);

		virtual	TYPE_ASCOM_STATUS	Start_Video(void);
		virtual	TYPE_ASCOM_STATUS	Stop_Video(void);
		virtual	TYPE_ASCOM_STATUS	Take_Video(void);

		virtual	TYPE_ALPACA_CAMERASTATE		Read_AlapcaCameraState(void);



		virtual	int		GetImage_ROI_info(void);

		virtual	TYPE_ASCOM_STATUS	Cooler_TurnOn(void);
		virtual	TYPE_ASCOM_STATUS	Cooler_TurnOff(void);
		virtual	TYPE_ASCOM_STATUS	Read_SensorTemp(void);
		virtual	TYPE_ASCOM_STATUS	Read_CoolerState(bool *coolerOnOff);
		virtual	TYPE_ASCOM_STATUS	Read_CoolerPowerLevel(void);
		virtual	TYPE_ASCOM_STATUS	Read_Readoutmodes(char *readOutModeString, bool includeQuotes=false);
		virtual	TYPE_ASCOM_STATUS	Read_Fastreadout(void);
		virtual	TYPE_ASCOM_STATUS	Read_ImageData(void);
		virtual bool				GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut);



	//*****************************************************************************
public:
	char					cSensorName[kMaxSensorNameLen];	//*	obtained from my table lookup
	//*****************************************************************************
protected:
	//*	ASCOM variables (properties)
	//*	the ones commented out with //+ need to be implemented.....
//+	short					cBayerOffsetX;
//+	short					cBayerOffsetY;
//+	TYPE_ALPACA_CAMERASTATE	cCameraState;
	int						cCameraXsize;			//*	max width of camera image
	int						cCameraYsize;			//*	max width of camera image
	bool					cCanAbortExposure;
	bool					cCanAsymmetricBin;
	bool					cCanFastReadout;
	bool					cCanGetCoolerPower;
	bool					cCanPulseGuide;
	bool					cCansetccdtemperature;
	bool					cCanStopExposure;
//+	double					cCCDTemperature;
//+	bool					cConnected;
//+	bool					cCoolerOn;
//+	double					cCoolerPower;
	double					cElectronsPerADU;

	double					cExposureMax_seconds;
	double					cExposureMin_seconds;
	long					cExposureMax_us;		//*	micro-seconds
	long					cExposureMin_us;		//*	micro-seconds

	double					cExposureResolution;	//*	exposure resolution in seconds
//+	bool					cFastReadout;
//+	double					cFullWellCapacity;
	int						cGain;					//*	see gain min/max
	int						cGainMax;
	int						cGainMin;
	bool					cHasShutter;
//+	double					cHeatSinkTemperature;
	bool					cImageReady;
	bool					cIsPulseGuiding;

	uint32_t				cLastexposureduration_us;	//*	stored in microseconds, ASCOM wants seconds, convert on the fly
	//*	Reported as a string
	struct timeval			cLastExposureStartTime;		//*	time exposure or video was started for frame rate calculations
	struct timeval			cLastExposureEndTime;		//*	NON-ALPACA----time last exposure ended
	//						cMaxADU;					//*!!!!!!!!!!!!!!	not implemented yet
	int						cMaxbinX;					//*	Maximum binning for the camera X axis
	int						cMaxbinY;					//*	Maximum binning for the camera Y axis

	//===================================
	//*	subframe information
	int						cNumX;
	int						cNumY;
	int						cStartX;
	int						cStartY;


//+	int						cOffset;					//*!!!!!!!!!!!!!!	not implemented yet
//+	int						cOffsetMax;
//+	int						cOffsetMin;
//+	int						Offsets;
//+	int						cPercentCompleted;
	double					cPixelSizeX;				//*	the pixel size of the camera, unit is um. (microns) such as 5.6um
	double					cPixelSizeY;				//*	the pixel size of the camera, unit is um. (microns) such as 5.6um

	//	ReadoutMode is implemented at execution time
	//	ReadoutModes is implemented at execution time

//+	TYPE_SensorType			cSensorType;


	//=========================================================================================
	//=========================================================================================
	//=========================================================================================
	//=========================================================================================
	//*	non-alpaca stuff
	long					cExposureDefault_us;	//*	micro-seconds


	bool		cUpdateOtherDevices;
	//=========================================================================================
	//*	Binning information
	int			cCurrentBinX;
	int			cCurrentBinY;

	bool		cTempReadSupported;			//*	true if temperature can be read from device
	double		cCameraTemp_Dbl;			//*	deg C
	long		cCoolerPowerLevel;
	long		cCoolerState;
	char		cLastCameraErrMsg[128];


	int			cCameraID;						//*	this is used to control everything of the camera in other functions.Start from 0.
	bool		cCameraIsOpen;					//*	flag to tell us the camera is open
	int			cBayerPattern;
	bool		cIsColorCam;					//*	false= monochrome, true = color
	bool		cIsCoolerCam;
	bool		cIsUSB3Host;
	bool		cIsUSB3Camera;
	int			cBitDepth;
	bool		cIsTriggerCam;

	//===================================
	//*	pulse guiding information
	bool			cSt4Port;
	struct timeval	cPulseGuideStartTime;		//*	time pulse guiding was started

	int			cSupportedFormats[kNumSupportedFormats];

	long		cGain_default;


	bool		cCanRead8Bit;
	long		cHighSpeedMode;

	//*****************************************************************************
	TYPE_CAMERA_STATE		cInternalCameraState;
	TYPE_IMAGE_ROI_Info		cROIinfo;
	TYPE_IMAGE_TYPE			cDesiredImageType;


	//*****************************************************************************
	TYPE_SUPPORTED_IMG_TYPE	cSupportedImageTypes[kMaxImageTypes];
	int						cCurrAlpacaImgTypeIdx;	//*	note: this is an index into the array
													//*	of supported image types. IT DOES NOT MATCH TYPE_IMAGE_TYPE

	bool				cNewImageReadyToDisplay;
	long				cCameraDataBuffLen;
	unsigned char		*cCameraDataBuffer;
	unsigned char		*cCameraBGRbuffer;			//*	Blue, Green, Red, for FITS

	int					cAVIfourcc;					//*	the fourCC mode used in the avi file

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
	bool				cSaveImages;
	long				cWorkingLoopCnt;
	long				cFramesRead;
	double				cFrameRate;					//*	primarily used in live mode
	//*****************************************************************************
	int					cNumFramesRequested;
	int					cNumFramesToSave;
	int					cNumFramesSaved;

	int					cTotalFramesSaved;

	//*	data for taking video
	int					cNumVideoFramesSaved;
	double				cVideoDuration_secs;		//*	how many seconds to record
	uint32_t			cVideoStartTime;			//*	time video was started for frame rate calculations (seconds)
	bool				cVideoCreateTimeStampFile;
	FILE				*cVideoTimeStampFilePtr;

	//*	these items are stored on a per camera basis for the purpose of responding
	//*	to some of the Alpaca requests

	int					cAlpacaCameraState;			//*	0 = CameraIdle
													//*	1 = CameraWaiting
													//*	2 = CameraExposing
													//*	3 = CameraReading
													//*	4 = CameraDownload
													//*	5 = CameraError
	struct timeval		cDownloadStartTime;
	struct timeval		cDownloadEndTime;

	TYPE_TELESCOPE_INFO	cTS_info;		//*	TeleScope info

	char				cTelescopeModel[kTelescopeNameMaxLen + 1];

	//==========================================================
	//*	File name information
	bool				cFN_includeSerialNum;
	bool				cFN_includeManuf;
	bool				cFN_includeFilter;
	bool				cFN_includeRefID;

	char				cObjectName[kObjectNameMaxLen + 1];
	char				cFileNameRoot[256];
	char				cLastJpegImageName[256];
	char				cFileNamePrefix[kFileNamePrefixMaxLen + 1];
	char				cFileNameSuffix[kFileNamePrefixMaxLen + 1];

	char				cAuxTextTag[kAuxiliaryTextMaxLen];		//*	auxiliary text information


#ifdef _USE_OPENCV_
	//==========================================================
	bool				cCreateOpenCVwindow;
	IplImage			*cOpenCV_Image;
	IplImage			*cOpenCV_LiveDisplay;
	IplImage			*cOpenCV_Histogram;
	bool				cCreateHistogramWindow;
	CvVideoWriter		*cOpenCV_videoWriter;
	CvFont				cTextFont;
	CvFont				cOverlayTextFont;
	CvScalar			cVideoOverlayColor;

	char				cOpenCV_ImgWindowName[128];
	bool				cOpenCV_ImgWindowValid;

	int					cDisplaySideBar;
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

	CvScalar			cSideBarBGcolor;
	CvScalar			cSideBarTXTcolor;
	CvScalar			cSideBarBlk;
	CvScalar			cSideBarRed;
	CvScalar			cSideBarGrn;
	CvScalar			cSideBarBlu;
	CvScalar			cSideBarGry;
	CvScalar			cSideBarFCblue;
	CvScalar			cCrossHairColor;


#endif // _USE_OPENCV_


#ifdef _USE_THREADS_FOR_ASI_CAMERA_
	bool				cThreadIsActive;
	bool				cKeepRunning;
	pthread_t			cThreadID;
#endif // _USE_THREADS_FOR_ASI_CAMERA_


#ifdef _ENABLE_FILTERWHEEL_
	void				UpdateFilterwheelLink(void);
	FilterwheelDriver	*cConnectedFilterWheel;
	int					cFilterWheelCurrPos;
	char				cFilterWheelCurrName[48];
#endif // _ENABLE_FILTERWHEEL_


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

};


//*****************************************************************************
enum
{
	kSideBar_None	=	0,
	kSideBar_Left,
	kSideBar_Right
};


//*****************************************************************************
typedef struct
{
	char	cameraModel[16];
	char	sensorName[kMaxSensorNameLen];

}	TYPE_SensorName;


extern	const TYPE_CmdEntry	gCameraCmdTable[];
extern	const char			*gCameraStateStrings[];

void	GetImageTypeString(TYPE_IMAGE_TYPE imageType, char *imageTypeString);

#endif		//	_CAMERA_DRIVER_H_
