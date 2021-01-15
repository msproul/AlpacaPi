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


#ifndef _WINDOW_TAB_H
	#include	"windowtab.h"
#endif // _WINDOW_TAB_H


#ifndef	_WIDGET_H_
	#include	"widget.h"
#endif


#define	kMaxControllers	10


#define	kMaxTabs	8
#define	kFontCnt	5
#define	kButtonCnt	30

extern CvFont		gTextFont[kFontCnt];



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


				void	HandleWindow(void);
				void	HandleWindowUpdate(void);
				void	ProcessMouseEvent(int event, int xxx, int yyy, int flags);

				void	InitWindowTabs(void);
				int		SetTabCount(const int newTabCount);
				void	SetTabText(const int tabIdx, const char *tabName);
				int		FindClickedTab(const int xxx, const int yyy);
				void	ProcessTabClick(const int tabIdx);



				void	DrawOneWidget(const int widgetIdx);
				void	DrawOneWidget(TYPE_WIDGET *widgetPtr, const int widgetIdx);
				//*	alphabetic order
				void	DrawWidgetButton(TYPE_WIDGET *theWidget);
				void	DrawWidgetCheckBox(TYPE_WIDGET *theWidget);
				void	DrawWidgetGraph(TYPE_WIDGET *theWidget);
				void	DrawWidgetIcon(TYPE_WIDGET *theWidget);
				void	DrawWidgetMultiLineText(TYPE_WIDGET *theWidget);
				void	DrawWidgetOutlineBox(TYPE_WIDGET *theWidget);
				void	DrawWidgetRadioButton(TYPE_WIDGET *theWidget);
				void	DrawWidgetSlider(TYPE_WIDGET *theWidget);
				void	DrawWidgetText(TYPE_WIDGET *theWidget);
				void	DrawWidgetProgressBar(TYPE_WIDGET *theWidget);


				void	DrawWidgetPressBar(TYPE_WIDGET *theWidget);


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

				void	SetWidgetHighlighted(	const int tabNum, const int widgetIdx, bool highlighted);
				void	SetWidgetProgress(		const int tabNum, const int widgetIdx, const int currPosition, const int totalValue);

		virtual	void	UpdateWindowTabColors(void);

		virtual	void	RunBackgroundTasks(void);
		virtual	void	SetupWindowControls(void);
				void	SetWindowIPaddrInfo(const char	*textString, const bool	onLine);
				void	DrawWindowTabs(void);
		virtual	void	DrawWindowWidgets(void);
				void	DrawWindow(void);
				void	UpdateWindowAsNeeded(void);

		virtual void	HandleKeyDown(const int keyPressed);
				void	HandleKeyDownInTextWidget(const int tabNum, const int widgetIdx,const int keyPressed);

		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(const int buttonIdx);
		virtual	void	RefreshWindow(void);

				bool	AlpacaGetSupportedActions(	sockaddr_in	*deviceAddress,
													int			devicePort,
													const char	*deviceType,
													const int	deviceNum);
				bool	AlpacaGetSupportedActions(const char *deviceType, const int deviceNum);
		virtual	void	AlpacaProcessSupportedAction(	const char	*deviceType,
														const int	deviveNum,
														const char	*valueString);
				int		AlpacaCheckForErrors(	SJP_Parser_t	*jsonParser,
												char			*errorMsg,
												bool			reportError=false);
		virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);


				bool	AlpacaGetStatus_ReadAll(	sockaddr_in	*deviceAddress,
													int			port,
													const char	*deviceType,
													const int	deviceNum);

				bool	AlpacaGetStatus_ReadAll(const char *deviceType, const int deviceNum);
		virtual	void	AlpacaProcessReadAll(	const char	*deviceType,
												const int deviceNum,
												const char	*keywordString,
												const char	*valueString);




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

				bool	AlpacaGetIntegerValue(	const char	*alpacaDevice,
												const char	*alpacaCmd,
												const char	*dataString,
												int			*returnValue,
												bool		*rtnValidData = NULL);

				bool	AlpacaGetDoubleValue(	const char	*alpacaDevice,
												const char	*alpacaCmd,
												const char	*dataString,
												double		*returnValue,
												bool		*rtnValidData = NULL);

				bool	AlpacaGetBooleanValue(	const char	*alpacaDevice,
												const char	*alpacaCmd,
												const char	*dataString,
												bool		*returnValue,
												bool		*rtnValidData = NULL);
				bool	AlpacaGetStringValue(	const char	*alpacaDevice,
												const char	*alpacaCmd,
												const char	*dataString,
												char		*returnString,
												bool		*rtnValidData = NULL);

		virtual	void	UpdateDownloadProgress(const int unitsRead, const int totalUnits);
				bool	AlpacaGetIntegerArray(	const char	*alpacaDevice,
												const int	alpacaDevNum,
												const char	*alpacaCmd,
												const char	*dataString,
												int			*uint32array,
												int			arrayLength,
												int			*actualValueCnt);
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
		bool				cHasReadAll;
		bool				cForceAlpacaUpdate;
		int					cLastAlpacaErrNum;
		char				cLastAlpacaErrStr[512];

		char				cAlpacaVersionString[128];
		char				cLastAlpacaCmdString[256];
		char				cAlpacaDeviceType[48];
		char				cAlpacaDeviceName[64];
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

};


uint32_t	millis(void);
CvScalar	Color16BitTo24Bit(const unsigned int color16);
void		Controller_HandleKeyDown(const int keyPressed);
void		LoadAlpacaLogo(void);
bool		IsTrueFalse(const char *trueFalseString);

extern	Controller	*gControllerList[kMaxControllers];
extern	int			gControllerCnt;
extern	bool		gKeepRunning;
extern	char		gColorOverRide;
extern	IplImage	*gAlpacaLogoPtr;
extern	char		gFullVersionString[];	//*	this is version of the controller software
											//*	which may be different from the remote software
extern	char		gFirstArgString[];



#endif // _CONTROLLER_H_
