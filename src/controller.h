//*****************************************************************************
//#include	"controller.h"




#ifndef	_CONTROLLER_H_
#define	_CONTROLLER_H_

#include	<stdbool.h>

#ifndef _ARPA_INET_H
	#include	<arpa/inet.h>
#endif // _ARPA_INET_H

#ifndef __OPENCV_OLD_HIGHGUI_H__
	#include "opencv/highgui.h"
#endif
#ifndef __OPENCV_HIGHGUI_H__
	#include "opencv2/highgui/highgui_c.h"
#endif


#include	"json_parse.h"


#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif




#ifndef _WINDOW_TAB_H
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H


#ifndef	_WIDGET_H_
	#include	"widget.h"
#endif


#ifndef kMagicCookieValue
	#define	kMagicCookieValue	0x55AA7777
#endif

#ifdef _ENABLE_SKYTRAVEL_
	#define	_USE_BACKGROUND_THREAD_
#endif // _ENABLE_SKYTRAVEL_


#define	kMaxControllers	16


#define	kMaxTabs	15
#define	kButtonCnt	30

#define	kDefaultUpdateDelta	4
#define	kLineBufSize		512

extern	CvFont	gTextFont[];
extern	bool	gVerbose;

#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))
#define	DEGREES(radians)	((radians) * (180.0 / M_PI))

//*****************************************************************************
#define	DELETE_OBJ_IF_VALID(objectPtr)	\
	if (objectPtr != NULL)				\
	{									\
		delete objectPtr;				\
		objectPtr	=	NULL;			\
	}


#define	kMaxCapabilities	50
//*****************************************************************************
typedef struct
{
	char	capabilityName[48];
	char	capabilityValue[24];

} TYPE_CAPABILITY;


//*****************************************************************************
class Controller
{
	public:

		//
		// Construction
		//
				Controller(	const char	*argWindowName,
							const int	xSize,
							const int	ySize);
		virtual	~Controller(void);
				void	CheckConnectedState(void);


				void	HandleWindow(void);
				void	HandleWindowUpdate(void);
				void	ProcessMouseEvent(int event, int xxx, int yyy, int flags);

				void	InitWindowTabs(void);
				int		SetTabCount(const int newTabCount);
				void	SetTabText(const int tabIdx, const char *tabName);
				void	GetCurrentTabName(char *currentTabName);
				int		FindClickedTab(const int xxx, const int yyy);
				void	ProcessTabClick(const int tabIdx);

				void	SetCurrentTab(const int tabIdx);


				void	DrawOneWidget(const int widgetIdx);
				void	DrawOneWidget(TYPE_WIDGET *widgetPtr, const int widgetIdx);
				//*	alphabetic order
				void	DrawWidgetButton(TYPE_WIDGET *theWidget);
				void	DrawWidgetCheckBox(TYPE_WIDGET *theWidget);
				void	DrawWidgetGraph(TYPE_WIDGET *theWidget);
				void	DrawWidgetIcon(TYPE_WIDGET *theWidget);
				void	DrawWidgetImage(TYPE_WIDGET *theWidget, IplImage *theOpenCVimage);
		virtual	void	DrawWidgetImage(TYPE_WIDGET *theWidget);
				void	DrawWidgetMultiLineText(TYPE_WIDGET *theWidget);
				void	DrawWidgetOutlineBox(TYPE_WIDGET *theWidget);
				void	DrawWidgetProgressBar(TYPE_WIDGET *theWidget);
				void	DrawWidgetRadioButton(TYPE_WIDGET *theWidget);
				void	DrawWidgetSlider(TYPE_WIDGET *theWidget);
				void	DrawWidgetScrollBar(TYPE_WIDGET *theWidget);
				void	DrawWidgetText(TYPE_WIDGET *theWidget);
				void	DrawWidgetTextWithTabs(TYPE_WIDGET *theWidget);

				void	DisplayButtonHelpText(const int buttonIdx);


		virtual	void	DrawWidgetCustom(TYPE_WIDGET *theWidget);

				int		FindClickedWidget(const int xxx, const int yyy);
				bool	IsWidgetButton(const int widgetIdx);
				bool	IsWidgetTextInput(const int widgetIdx);

				//************************************************************
				//*	these routines handle multiple tabs
				void	SetTabWindow(			const int tabNum, WindowTab *theTabObjectPtr);

				void	SetWidgetText(			const int tabNum, const int widgetIdx, const char *newText);
				void	GetWidgetText(			const int tabNum, const int widgetIdx, char *getText);

				void	SetWidgetNumber(		const int tabNum, const int widgetIdx, const int number);
				void	SetWidgetNumber(		const int tabNum, const int widgetIdx, const double number);

				void	SetWidgetType(			const int tabNum, const int widgetIdx, const int widetType);
				void	SetWidgetFont(			const int tabNum, const int widgetIdx, int fontNum);
				void	SetWidgetTextColor(		const int tabNum, const int widgetIdx, CvScalar newtextColor);
				void	SetWidgetBGColor(		const int tabNum, const int widgetIdx, CvScalar newBGcolor);
				void	SetWidgetBorderColor(	const int tabNum, const int widgetIdx, CvScalar newBoarderColor);
				void	SetWidgetImage(			const int tabNum, const int widgetIdx, IplImage *argImagePtr);

				void	SetWidgetValid(			const int tabNum, const int widgetIdx, bool valid);
				void	SetWidgetChecked(		const int tabNum, const int widgetIdx, bool checked);
				void	SetWidgetCrossedout(	const int tabNum, const int widgetIdx, bool crossedout);

				void	SetWidgetSliderLimits(	const int tabNum, const int widgetIdx, double sliderMin, double sliderMax);
				void	SetWidgetSliderValue(	const int tabNum, const int widgetIdx, double sliderValue);

				void	SetWidgetScrollBarLimits(const int widgetIdx, int scrollBarLines, int scrollBarMax);

				void	SetWidgetHighlighted(	const int tabNum, const int widgetIdx, bool highlighted);
				void	SetWidgetProgress(		const int tabNum, const int widgetIdx, const int currPosition, const int totalValue);

		virtual	void	UpdateWindowTabColors(void);


		virtual	void	RunBackgroundTasks(bool enableDebug=false);
		virtual	void	SetupWindowControls(void);
				void	SetWindowIPaddrInfo(const char	*textString, const bool	onLine);
				void	DrawWindowTabs(void);
		virtual	void	DrawWindowWidgets(void);
				void	DrawWindow(void);
				void	UpdateWindowAsNeeded(void);

		virtual void	HandleKeyDown(const int keyPressed);
				void	HandleKeyDownInTextWidget(const int tabNum, const int widgetIdx,const int keyPressed);

		virtual	void	ProcessButtonClick(const int buttonIdx);

		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);

		virtual	void	ProcessDoubleClick_RtBtn(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);


		virtual	void	RefreshWindow(void);



		uint32_t	cMagicCookie;		//*	an indicator so we know the object is valid
		int			cDebugCounter;
		bool		cKeepRunning;
		bool		cUpdateProtect;

		//*	Window tabs
		int			cCurrentTabNum;
		WindowTab	*cCurrentTabObjPtr;
		WindowTab	*cWindowTabs[kMaxTabs];

		CvScalar	cBackGrndColor;
		TYPE_WIDGET	cTabList[kMaxTabs];
		int			cTabCount;

		bool		cUpdateWindow;
		char		cWindowName[256];
		int			cWidth;
		int			cHeight;
		IplImage	*cOpenCV_Image;
		int			cValidFnts;

		bool		cLeftButtonDown;
		bool		cRightButtonDown;
		int			cLastClicked_Btn;
		int			cLastClicked_Tab;
		int			cHighlightedBtn;
		int			cLastLClickX;
		int			cLastLClickY;

		int			cCurTextInput_Widget;

		int			cCurrentMouseX;
		int			cCurrentMouseY;


		//**********************************************
		//*	Alpaca stuff
		bool				cReadStartup;
		bool				cOnLine;
		bool				cHas_readall;
		bool				cForceAlpacaUpdate;
		TYPE_ASCOM_STATUS	cLastAlpacaErrNum;
		char				cLastAlpacaErrStr[512];

		TYPE_CommonProperties	cCommonProp;

		char				cAlpacaVersionString[128];

		char				cLastAlpacaCmdString[256];
		char				cAlpacaDeviceTypeStr[48];
		char				cAlpacaDeviceNameStr[64];
		bool				cValidIPaddr;
		struct sockaddr_in	cDeviceAddress;
		int					cPort;
		int					cAlpacaDevNum;
		int					cReadFailureCnt;
		bool				cFirstDataRead;
		uint32_t			cLastUpdate_milliSecs;

		//*	alpaca download stats
		uint32_t			cLastDownload_Bytes;
		uint32_t			cLastDownload_Millisecs;
		double				cLastDownload_MegaBytesPerSec;


#ifdef _CONTROLLER_USES_ALPACA_

//------------------------------------------------------------
				bool	AlpacaSetConnected(const char *deviceTypeStr, const bool newConnectedState=true);
		virtual	void	UpdateSupportedActions(void);

				bool	AlpacaGetCommonProperties_OneAAT(const char *deviceTypeStr);
				bool	AlpacaGetCommonConnectedState(const char *deviceTypeStr);
		virtual	void	UpdateCommonProperties(void);


		virtual	bool	AlpacaGetStartupData(void);
				bool	AlpacaGetSupportedActions(		sockaddr_in	*deviceAddress,
														int			devicePort,
														const char	*deviceTypeStr,
														const int	deviceNum);
				bool	AlpacaGetSupportedActions(		const char *deviceTypeStr, const int deviceNum);
		virtual	void	AlpacaProcessSupportedActions(	const char	*deviceTypeStr,
														const int	deviveNum,
														const char	*valueString);
				TYPE_ASCOM_STATUS	AlpacaCheckForErrors(	SJP_Parser_t	*jsonParser,
															char			*errorMsg,
															bool			reportError=false);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);


				bool	AlpacaGetStatus_ReadAll(	const char	*deviceTypeStr, const int deviceNum);

				bool	AlpacaGetStatus_ReadAll(	sockaddr_in	*deviceAddress,
													int			devicePort,
													const char	*deviceTypeStr,
													const int	deviceNum);


		virtual	void	AlpacaProcessReadAll(		const char	*deviceTypeStr,
													const int	deviceNum,
													const char	*keywordString,
													const char	*valueString);

				void	AlpacaProcessReadAll_Common(const char	*deviceTypeStr,
													const int	deviceNum,
													const char	*keywordString,
													const char	*valueString);
		virtual	void	UpdateDownloadProgress(const int unitsRead, const int totalUnits);

				void	UpdateConnectedIndicator(const int tabNum, const int widgetNum);

				bool	AlpacaSendPutCmd(			const char	*alpacaDevice,
													const char	*alpacaCmd,
													const char	*dataString);

				bool	AlpacaSendPutCmdwResponse(	sockaddr_in		*deviceAddress,
													int				devicePort,
													const char		*alpacaDevice,
													const int		alpacaDevNum,
													const char		*alpacaCmd,
													const char		*dataString,
													SJP_Parser_t	*jsonParser);

				bool	AlpacaSendPutCmdwResponse(	const char		*alpacaDevice,
													const char		*alpacaCmd,
													const char		*dataString,
													SJP_Parser_t	*jsonParser);

				bool	AlpacaGetIntegerValue(		struct sockaddr_in	deviceAddress,
													int					port,
													int					alpacaDevNum,
													const char			*alpacaDevice,
													const char			*alpacaCmd,
													const char			*dataString,
													int					*returnValue,
													bool				*rtnValidData = NULL);

				bool	AlpacaGetIntegerValue(	const char	*alpacaDevice,
												const char	*alpacaCmd,
												const char	*dataString,
												int			*returnValue,
												bool		*rtnValidData = NULL);

				bool	AlpacaGetDoubleValue(	struct sockaddr_in	deviceAddress,
												int					port,
												int					alpacaDevNum,
												const char			*alpacaDevice,
												const char			*alpacaCmd,
												const char			*dataString,
												double				*returnValue,
												bool				*rtnValidData = NULL);


				bool	AlpacaGetDoubleValue(	const char	*alpacaDevice,
												const char	*alpacaCmd,
												const char	*dataString,
												double		*returnValue,
												bool		*rtnValidData = NULL);

				bool	AlpacaGetBooleanValue(	const char	*alpacaDevice,
												const char	*alpacaCmd,
												const char	*dataString,
												bool		*returnValue,
												bool		*rtnValidData = NULL,
												bool		printDebug=false);
				bool	AlpacaGetStringValue(	const char	*alpacaDevice,
												const char	*alpacaCmd,
												const char	*dataString,
												char		*returnString,
												bool		*rtnValidData = NULL);

				bool	AlpacaGetMinMax(		const char	*alpacaDevice,
												const char	*alpacaCmd,
												const char	*dataString,
												double		*returnMinValue,
												double		*returnMaxValue,
												bool		*rtnValidData = NULL);
				int		AlpacaGetIntegerArray(	const char	*alpacaDevice,
												const int	alpacaDevNum,
												const char	*alpacaCmd,
												const char	*dataString,
												int			*uint32array,
												int			arrayLength,
												int			*actualValueCnt);

				//**********************************************
				//*	this is a table of capabilities for the purpose of displaying what
				//*	the driver is capable of doing.
				TYPE_CAPABILITY		cCapabilitiesList[kMaxCapabilities];
				void				ClearCapabilitiesList(void);
				void				AddCapability(const char *capability, const char *value);
		virtual	void				UpdateCapabilityList(void);
				void				ReadOneDriverCapability(const char	*driverNameStr,
															const char	*propertyStr,
															const char	*reportedStr,
															bool		*booleanValue);

#endif // _CONTROLLER_USES_ALPACA_

//------------------------------------------------------------
//*	background thread stuff
		int			StartBackgroundThread(void);
		pthread_t	cBackgroundThreadID;
		bool		cBackGroundThreadCreated;
		bool		cButtonClickInProgress;
		bool		cBackgroundTaskActive;


};

#ifdef __cplusplus
	extern "C" {
#endif

CvScalar	Color16BitTo24Bit(const unsigned int color16);
void		Controller_HandleKeyDown(const int keyPressed);
void		LoadAlpacaLogo(void);
bool		CheckForOpenWindowByName(const char *windowName);
void		DumpControllerBackGroundTaskStatus(void);

#ifdef __cplusplus
}
#endif


extern	Controller	*gControllerList[kMaxControllers];
extern	int			gControllerCnt;
extern	bool		gKeepRunning;
extern	char		gColorOverRide;
extern	IplImage	*gAlpacaLogoPtr;
extern	char		gFullVersionString[];	//*	this is version of the controller software
											//*	which may be different from the remote software
extern	char		gFirstArgString[];



#endif // _CONTROLLER_H_
