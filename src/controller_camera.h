//*****************************************************************************
//#include	"controller_camera.h"

#ifndef _CONTROLLER_CAMERA_H_
#define	_CONTROLLER_CAMERA_H_

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif // _DISCOVERY_LIB_H_

#ifndef	_CONTROLLER_H_
	#include	"controller.h"
#endif // _CONTROLLER_H_
#ifndef	_WINDOWTAB_CAMERA_H_
	#include	"windowtab_camera.h"
#endif // _WINDOWTAB_CAMERA_H_
#ifndef	_WINDOWTAB_FILELIST_H_
	#include	"windowtab_filelist.h"
#endif // _WINDOWTAB_FILELIST_H_
#ifndef	_WINDOWTAB_CAMSETTINGS_H_
	#include	"windowtab_camsettings.h"
#endif // _WINDOWTAB_CAMSETTINGS_H_


#ifndef	_WINDOWTAB_ABOUT_H_
	#include	"windowtab_about.h"
#endif // _WINDOWTAB_ABOUT_H_



//**************************************************************************************
typedef struct
{
	char	mode[8];
} READOUTMODE;

//**************************************************************************************
typedef struct
{
	char	filterName[16];
} FILTERWHEEL;


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

#define	kMaxRemoteFileCnt		200
#define	kMaxTemperatureValues	(450)
#define	kMaxReadOutModes		5
#define	kMaxFilters				8

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
		virtual	void	RunBackgroundTasks(void);
	//	virtual	void	DrawGraphWidget(const int widgitIdx);
		virtual	void	DrawWidgetCustom(TYPE_WIDGET *theWidget);

		//*	this is a large list of update routines, they should be implemented in the subclass
		virtual	void	UpdateCameraGain(void);
		virtual	void	UpdateCameraExposure(void);
		virtual	void	UpdateCameraName(void);
		virtual	void	UpdateCameraSize(void);
		virtual	void	UpdateCameraState(void);
		virtual	void	UpdateCameraTemperature(void);

		virtual	void	UpdateReadAllStatus(void);
		virtual	void	UpdateRemoteAlpacaVersion(void);
		virtual	void	UpdateReadoutModes(void);
		virtual	void	UpdateCurrReadoutMode(void);
		virtual	void	UpdateCoolerState(void);
		virtual	void	UpdateDisplayModes(void);
		virtual	void	UpdateFilterWheelInfo(void);
		virtual	void	UpdateFilterWheelPosition(void);
		virtual	void	UpdateFileNameOptions(void);
		virtual	void	UpdateReceivedFileName(const char *newFileName);
		virtual	void	UpdateRemoteFileList(void);
		virtual	void	UpdateDownloadProgress(const int unitsRead, const int totalUnits);
		virtual	void	UpdateBackgroundColor(const int redValue, const int grnValue, const int bluValue);
		virtual	void	UpdateFreeDiskSpace(const double gigabytesFree);



		//*	sub class specific routines
				void	SetErrorTextString(const char	*errorString);
				void	GetConfiguredDevices(void);
				bool	AlpacaGetStartupData(void);
		virtual	void	AlpacaProcessReadAll(const char *keywordString, const char *valueString);
				bool	AlpacaGetStatus_OneAAT(void);	//*	One At A Time
				bool	AlpacaGetStatus(void);
				bool	AlpacaGetFileList(void);
				bool	AlpacaGetFilterWheelStartup(void);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);
		virtual	void	UpdateSettings_Object(const char *filePrefix);
				void	SetFileNameOptions(const int fnOptionBtn);

				void	AddFileToRemoteList(const char *fileName);

				void	BumpGain(const int howMuch);
				void	BumpExposure(const double howMuch);
				void	ToggleLiveMode(void);
				void	ToggleSideBar(void);
				void	ToggleAutoExposure(void);
				void	ToggleDisplayImage(void);
				void	ToggleCooler(void);
				void	StartExposure(void);
				void	SetObjectText(const char *newObjectText, const char *newPrefixText);
				void	LogCameraTemp(const double cameraTemp);


				IplImage	*DownloadImage(void);


				char					cCameraName[48];
				int						cCameraSizeX;
				int						cCameraSizeY;
				int						cAlpacaCameraState;
				int						cGain;
				int						cGainMin;
				int						cGainMax;
				double					cCCDtemperature;
				READOUTMODE				cReadOutModes[kMaxReadOutModes];
				int						cReadOutMode;

				double					cExposure;
				double					cExposureMin;
				double					cExposureMax;
				bool					cLiveMode;
				bool					cSideBar;
				bool					cAutoExposure;
				bool					cDisplayImage;
				bool					cHasCooler;

				//==========================================================
				//*	File name information
				bool				cFN_includeSerialNum;
				bool				cFN_includeManuf;
				bool				cFN_includeFilter;
				bool				cFN_includeRefID;

				//==========================================================
				//*	filter wheel information
				bool				cHasFilterWheel;
				char				cFilterWheelName[32];
				FILTERWHEEL			cFilterNames[kMaxFilters];
				int					cFilterWheelPosition;

				//*	temp related stuf
				bool				cCoolerOn;
				bool				cHasCCDtemp;
				double				cCameraTempLog[kMaxTemperatureValues];
				int					cTempLogCount;

				TYPE_REMOTE_FILE	cRemoteFiles[kMaxRemoteFileCnt];

};

#endif // _CONTROLLER_CAMERA_H_
