/****************************************
 * ArtemisHscAPI.h
 *
 * This file is autogenerated.
 *
 ****************************************/
#pragma once

#include <comdef.h>

extern "C"
{
	enum ARTEMISERROR
	{
		ARTEMIS_OK = 0,
		ARTEMIS_INVALID_PARAMETER,
		ARTEMIS_NOT_CONNECTED,
		ARTEMIS_NOT_IMPLEMENTED,
		ARTEMIS_NO_RESPONSE,
		ARTEMIS_INVALID_FUNCTION,
		ARTEMIS_NOT_INITIALIZED,
		ARTEMIS_OPERATION_FAILED,
	};

	// Colour properties
	enum ARTEMISCOLOURTYPE
	{
		ARTEMIS_COLOUR_UNKNOWN = 0,
		ARTEMIS_COLOUR_NONE,
		ARTEMIS_COLOUR_RGGB
	};

	//Other enumeration types
	enum ARTEMISPRECHARGEMODE
	{
		PRECHARGE_NONE = 0,		// Precharge ignored
		PRECHARGE_ICPS,			// In-camera precharge subtraction
		PRECHARGE_FULL,			// Precharge sent with image data
	};

	// Camera State
	enum ARTEMISCAMERASTATE
	{
		CAMERA_ERROR = -1,
		CAMERA_IDLE = 0,
		CAMERA_WAITING,
		CAMERA_EXPOSING,
		CAMERA_READING,
		CAMERA_DOWNLOADING,
		CAMERA_FLUSHING,
	};

	// Connection State:
	enum ARTEMISCONNECTIONSTATE
	{
		CAMERA_CONNECTING      = 1,
		CAMERA_CONNECTED       = 2,
		CAMERA_CONNECT_FAILED  = 3,
		CAMERA_SUSPENDED       = 4,
		CAMERA_CONNECT_UNKNOWN = 5
	};

	// Parameters for ArtemisSendMessage
	enum ARTEMISSENDMSG
	{
		ARTEMIS_LE_LOW = 0,
		ARTEMIS_LE_HIGH = 1,
		ARTEMIS_GUIDE_NORTH = 10,
		ARTEMIS_GUIDE_SOUTH = 11,
		ARTEMIS_GUIDE_EAST = 12,
		ARTEMIS_GUIDE_WEST = 13,
		ARTEMIS_GUIDE_STOP = 14,
	};

	// Parameters for ArtemisGet/SetProcessing
	// These must be powers of 2.
	enum ARTEMISPROCESSING
	{
		ARTEMIS_PROCESS_LINEARISE = 1,	// compensate for JFET nonlinearity
		ARTEMIS_PROCESS_VBE = 2, // adjust for 'Venetian Blind effect'
	};

	// Parameters for ArtemisSetUpADC
	enum ARTEMISSETUPADC
	{
		ARTEMIS_SETUPADC_MODE = 0,
		ARTEMIS_SETUPADC_OFFSETR = (1 << 10),
		ARTEMIS_SETUPADC_OFFSETG = (2 << 10),
		ARTEMIS_SETUPADC_OFFSETB = (3 << 10),
		ARTEMIS_SETUPADC_GAINR = (4 << 10),
		ARTEMIS_SETUPADC_GAING = (5 << 10),
		ARTEMIS_SETUPADC_GAINB = (6 << 10),
	};

	enum ARTEMISPROPERTIESCCDFLAGS
	{
		ARTEMIS_PROPERTIES_CCDFLAGS_INTERLACED = 1, // CCD is interlaced type
		ARTEMIS_PROPERTIES_CCDFLAGS_DUMMY = 0x7FFFFFFF // force size to 4 bytes
	};

	enum ARTEMISPROPERTIESCAMERAFLAGS
	{
		ARTEMIS_PROPERTIES_CAMERAFLAGS_FIFO = 1, // Camera has readout FIFO fitted
		ARTEMIS_PROPERTIES_CAMERAFLAGS_EXT_TRIGGER = 2, // Camera has external trigger capabilities
		ARTEMIS_PROPERTIES_CAMERAFLAGS_PREVIEW = 4, // Camera can return preview data
		ARTEMIS_PROPERTIES_CAMERAFLAGS_SUBSAMPLE = 8, // Camera can return subsampled data
		ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_SHUTTER = 16, // Camera has a mechanical shutter
		ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_GUIDE_PORT = 32, // Camera has a guide port
		ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_GPIO = 64, // Camera has GPIO capability
		ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_WINDOW_HEATER = 128, // Camera has a window heater
		ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_EIGHT_BIT_MODE = 256, // Camera can download 8-bit images
		ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_OVERLAP_MODE = 512, // Camera can overlap
		ARTEMIS_PROPERTIES_CAMERAFLAGS_HAS_FILTERWHEEL = 1024, // Camera has internal filterwheel
		ARTEMIS_PROPERTIES_CAMERAFLAGS_DUMMY = 0x7FFFFFFF // force size to 4 bytes
	};

	enum ARTEMISEFWTYPE
	{
		ARTEMIS_EFW1 = 1,
		ARTEMIS_EFW2 = 2
	};

	//Structures

	// camera/CCD properties
	struct ARTEMISPROPERTIES
	{
		int Protocol;
		int nPixelsX;
		int nPixelsY;
		float PixelMicronsX;
		float PixelMicronsY;
		int ccdflags;
		int cameraflags;
		char Description[40];
		char Manufacturer[40];
	};

	typedef void * ArtemisHandle;

	static HINSTANCE hArtemisDLL = NULL;

	//////////////////////////////////////////////////////////////////////////
	//
	// Interface functions for Artemis CCD Camera Library
	//

	#define artfn extern

	// -------------------  DLL --------------------------
	artfn int  ArtemisAPIVersion();
	artfn int  ArtemisDLLVersion();
	artfn BOOL ArtemisIsLocalConnection();
	artfn void ArtemisAllowDebugToConsole(bool value);
	artfn void ArtemisSetDebugCallback(void(*callback)(const char *message));
	artfn void ArtemisSetDebugCallbackContext(void * context, void(*callback)(void * context, const char *message));
	artfn void ArtemisSetFirmwareDir(const char * firmwareDir);
	artfn void ArtemisSetAtikAir(const char * host, int port);
	artfn void ArtemisShutdown();

	// -------------------  Device --------------------------
	artfn int			ArtemisDeviceCount();
	artfn BOOL			ArtemisDeviceIsPresent(int iDevice);
	artfn BOOL			ArtemisDevicePresent(  int iDevice);
	artfn BOOL			ArtemisDeviceInUse(    int iDevice);

	artfn BOOL			ArtemisDeviceName(          int iDevice, char *pName);
	artfn BOOL			ArtemisDeviceSerial(        int iDevice, char *pSerial);
	artfn BOOL			ArtemisDeviceIsCamera(      int iDevice);
	artfn BOOL			ArtemisDeviceHasFilterWheel(int iDevice);
	artfn BOOL			ArtemisDeviceHasGuidePort(  int iDevice);

	artfn ArtemisHandle	ArtemisConnect(    int iDevice);
	artfn BOOL			ArtemisIsConnected(ArtemisHandle handle);
	artfn BOOL			ArtemisDisconnect( ArtemisHandle handle);
	artfn int			ArtemisRefreshDevicesCount();


	// ------------------- Camera Info -----------------------------------
	artfn int ArtemisCameraSerial(		   ArtemisHandle handle, int* flags, int* serial);
	artfn int ArtemisColourProperties(	   ArtemisHandle handle, ARTEMISCOLOURTYPE *colourType, int *normalOffsetX, int *normalOffsetY, int *previewOffsetX, int *previewOffsetY);
	artfn int ArtemisProperties(		   ArtemisHandle handle, struct ARTEMISPROPERTIES *pProp);
	artfn int ArtemisCameraConnectionState(ArtemisHandle handle, ARTEMISCONNECTIONSTATE * state);

	// ------------------- Exposure Settings -----------------------------------
	artfn int  ArtemisBin(								ArtemisHandle handle, int  x, int  y);
	artfn int  ArtemisGetBin(							ArtemisHandle handle, int *x, int *y);
	artfn int  ArtemisGetMaxBin(						ArtemisHandle handle, int *x, int *y);
	artfn int  ArtemisGetSubframe(						ArtemisHandle handle, int *x, int *y, int *w, int *h);
	artfn int  ArtemisSubframe(							ArtemisHandle handle, int  x, int  y, int  w, int  h);
	artfn int  ArtemisSubframePos(						ArtemisHandle handle, int  x, int y);
	artfn int  ArtemisSubframeSize(						ArtemisHandle handle, int  w, int h);
	artfn int  ArtemisSetSubSample(						ArtemisHandle handle, bool bSub);
	artfn BOOL ArtemisContinuousExposingModeSupported(	ArtemisHandle handle);
	artfn BOOL ArtemisGetContinuousExposingMode(		ArtemisHandle handle);
	artfn int  ArtemisSetContinuousExposingMode(		ArtemisHandle handle, bool bEnable);
	artfn BOOL ArtemisGetDarkMode(						ArtemisHandle handle);
	artfn int  ArtemisSetDarkMode(						ArtemisHandle handle, bool bEnable);
	artfn int  ArtemisSetPreview(						ArtemisHandle handle, bool bPrev);
	artfn int  ArtemisAutoAdjustBlackLevel(				ArtemisHandle handle, bool bEnable);
	artfn int  ArtemisPrechargeMode(					ArtemisHandle handle, int mode);
	artfn int  ArtemisEightBitMode(						ArtemisHandle handle, bool eightbit);
	artfn int  ArtemisGetEightBitMode(					ArtemisHandle handle, bool *eightbit);
	artfn int  ArtemisStartOverlappedExposure(			ArtemisHandle handle);
	artfn BOOL ArtemisOverlappedExposureValid(			ArtemisHandle handle);
	artfn int  ArtemisSetOverlappedExposureTime(		ArtemisHandle handle, float fSeconds);
	artfn int  ArtemisTriggeredExposure(				ArtemisHandle handle, bool bAwaitTrigger);
	artfn int  ArtemisGetProcessing(					ArtemisHandle handle);
	artfn int  ArtemisSetProcessing(					ArtemisHandle handle, int options);


	// ------------------- Exposures -----------------------------------
	artfn int   ArtemisStartExposure(			 ArtemisHandle handle, float seconds);
	artfn int   ArtemisStartExposureMS(			 ArtemisHandle handle, int ms);
	artfn int   ArtemisAbortExposure(			 ArtemisHandle handle);
	artfn int   ArtemisStopExposure(			 ArtemisHandle handle);
	artfn BOOL  ArtemisImageReady(				 ArtemisHandle handle);
	artfn int   ArtemisCameraState(				 ArtemisHandle handle);
	artfn float ArtemisExposureTimeRemaining(	 ArtemisHandle handle);
	artfn int   ArtemisDownloadPercent(			 ArtemisHandle handle);
	artfn int   ArtemisGetImageData(			 ArtemisHandle handle, int *x, int *y, int *w, int *h, int *binx, int *biny);
	artfn void* ArtemisImageBuffer(				 ArtemisHandle handle);
	artfn float ArtemisLastExposureDuration(	 ArtemisHandle handle);
	artfn char* ArtemisLastStartTime(			 ArtemisHandle handle);
	artfn int   ArtemisLastStartTimeMilliseconds(ArtemisHandle handle);
	artfn int   ArtemisClearVReg(				 ArtemisHandle handle);
	
	artfn BOOL ArtemisHasFastMode(      ArtemisHandle handle);
	artfn BOOL ArtemisStartFastExposure(ArtemisHandle handle, int ms);
	artfn BOOL ArtemisSetFastCallback(  ArtemisHandle handle, void(*callback)(ArtemisHandle handle, int x, int y, int w, int h, int binx, int biny, void * imageBuffer));

	// ------------------- Amplifier -----------------------------------
	artfn int  ArtemisAmplifier(		   ArtemisHandle handle, bool bOn);
	artfn BOOL ArtemisGetAmplifierSwitched(ArtemisHandle handle);
	artfn int  ArtemisSetAmplifierSwitched(ArtemisHandle handle, bool bSwitched);
		
	// ------------ Camera Specific Options -------------
	artfn bool ArtemisHasCameraSpecificOption(    ArtemisHandle handle, unsigned short id);
	artfn int  ArtemisCameraSpecificOptionGetData(ArtemisHandle handle, unsigned short id, unsigned char * data, int dataLength, int & actualLength);
	artfn int  ArtemisCameraSpecificOptionSetData(ArtemisHandle handle, unsigned short id, unsigned char * data, int dataLength);

	// ------------------- Column Repair ----------------------------------	
	artfn int ArtemisSetColumnRepairColumns(		ArtemisHandle handle, int   nColumn, unsigned short * columns);
	artfn int ArtemisGetColumnRepairColumns(		ArtemisHandle handle, int * nColumn, unsigned short * columns);
	artfn int ArtemisClearColumnRepairColumns(		ArtemisHandle handle);
	artfn int ArtemisSetColumnRepairFixColumns(		ArtemisHandle handle, bool value);
	artfn int ArtemisGetColumnRepairFixColumns(		ArtemisHandle handle, bool * value);
	artfn int ArtemisGetColumnRepairCanFixColumns(	ArtemisHandle handle, bool * value);

	// ---------------- EEPROM -------------------------
	artfn int ArtemisCanInteractWithEEPROM(ArtemisHandle handle, bool * canInteract);
	artfn int ArtemisWriteToEEPROM(		   ArtemisHandle handle, char * password, int address, int length, const unsigned char * data);
	artfn int ArtemisReadFromEEPROM(	   ArtemisHandle handle, char * password, int address, int length,       unsigned char * data);


	// ------------------- Filter Wheel -----------------------------------
	artfn int			ArtemisFilterWheelInfo(ArtemisHandle handle, int *numFilters, int *moving, int *currentPos, int *targetPos);
	artfn int			ArtemisFilterWheelMove(ArtemisHandle handle, int targetPos);
	artfn BOOL			ArtemisEFWIsPresent(int i);
	artfn int			ArtemisEFWGetDeviceDetails(int i, ARTEMISEFWTYPE * type, char * serialNumber);
	artfn ArtemisHandle ArtemisEFWConnect(int i);
	artfn bool			ArtemisEFWIsConnected(ArtemisHandle handle);
	artfn int			ArtemisEFWDisconnect( ArtemisHandle handle);
	artfn int			ArtemisEFWGetDetails( ArtemisHandle handle, ARTEMISEFWTYPE * type, char * serialNumber);
	artfn int			ArtemisEFWNmrPosition(ArtemisHandle handle, int * nPosition);
	artfn int			ArtemisEFWSetPosition(ArtemisHandle handle, int   iPosition);
	artfn int			ArtemisEFWGetPosition(ArtemisHandle handle, int * iPosition, bool * isMoving);
	
	// ------------------- Firmware ----------------------------------------	
	artfn bool ArtemisCanUploadFirmware(ArtemisHandle handle);
	artfn int  ArtemisUploadFirmware(   ArtemisHandle handle, char * fileName, char * password);

	// ------------------- Gain ----------------------------------
	artfn int ArtemisGetGain(ArtemisHandle handle, bool isPreview, int *gain, int *offset);
	artfn int ArtemisSetGain(ArtemisHandle handle, bool isPreview, int  gain, int  offset);

	// ------------------- GPIO -----------------------------------
	artfn int ArtemisGetGpioInformation(ArtemisHandle handle, int* lineCount, int* lineValues);
	artfn int ArtemisSetGpioDirection(  ArtemisHandle handle, int directionMask);
	artfn int ArtemisSetGpioValues(     ArtemisHandle handle, int lineValues);

	// ------------------- Guiding -----------------------------------
	artfn int ArtemisGuide(					   ArtemisHandle handle, int axis);
	artfn int ArtemisGuidePort(				   ArtemisHandle handle, int nibble);
	artfn int ArtemisPulseGuide(			   ArtemisHandle handle, int axis, int milli);
	artfn int ArtemisStopGuiding(			   ArtemisHandle handle);
	artfn int ArtemisStopGuidingBeforeDownload(ArtemisHandle handle, bool bEnable);

	// ------------------- Lens -----------------------------------
	artfn int ArtemisGetLensAperture(ArtemisHandle handle, int* aperture);
	artfn int ArtemisGetLensFocus(   ArtemisHandle handle, int* focus);
	artfn int ArtemisGetLensLimits(  ArtemisHandle handle, int* apertureMin, int* apertureMax, int* focusMin, int* focusMax);
	artfn int ArtemisInitializeLens( ArtemisHandle handle);
	artfn int ArtemisSetLensAperture(ArtemisHandle handle, int aperture);
	artfn int ArtemisSetLensFocus(   ArtemisHandle handle, int focus);

	// ------------------- Shutter ----------------------------------		
	artfn int ArtemisCanControlShutter( ArtemisHandle handle, bool * canControl);
	artfn int ArtemisOpenShutter(		ArtemisHandle handle);
	artfn int ArtemisCloseShutter(	    ArtemisHandle handle);
	artfn int ArtemisCanSetShutterSpeed(ArtemisHandle handle, bool *canSetShutterSpeed);
	artfn int ArtemisGetShutterSpeed(	ArtemisHandle handle, int *speed);
	artfn int ArtemisSetShutterSpeed(	ArtemisHandle handle, int  speed);

	// ------------------- Temperature -----------------------------------
	artfn int ArtemisTemperatureSensorInfo(ArtemisHandle handle, int sensor, int* temperature);
	artfn int ArtemisSetCooling(		   ArtemisHandle handle, int setpoint);
	artfn int ArtemisSetCoolingPower(      ArtemisHandle handle, int power);
	artfn int ArtemisCoolingInfo(		   ArtemisHandle handle, int* flags, int* level, int* minlvl, int* maxlvl, int* setpoint);
	artfn int ArtemisCoolerWarmUp(		   ArtemisHandle handle);
	artfn int ArtemisGetWindowHeaterPower( ArtemisHandle handle, int* windowHeaterPower);
	artfn int ArtemisSetWindowHeaterPower( ArtemisHandle handle, int  windowHeaterPower);
	


	// Try to load the Artemis DLL.
	// Returns true if loaded ok.
	artfn bool ArtemisLoadDLL(const char * fileName);

	// Unload the Artemis DLL.
	artfn void ArtemisUnLoadDLL();


	#undef artfn
}

