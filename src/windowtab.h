//*****************************************************************************
//#include	"windowtab.h"

#ifndef _WINDOW_TAB_H_
#define	_WINDOW_TAB_H_

#ifndef _ARPA_INET_H
	#include	<arpa/inet.h>
#endif // _ARPA_INET_H


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#ifndef _JSON_PARSE_H_
	#include	"json_parse.h"
#endif // _JSON_PARSE_H_

#ifndef	_WIDGET_H_
	#include	"widget.h"
#endif // _WIDGET_H_


//*****************************************************************************
typedef struct
{
	CvScalar	bgColor;
	CvScalar	fontColor;
	CvScalar	borderColor;

	CvScalar	btnColor;
	CvScalar	btnFontColor;
	CvScalar	btnBorderColor;


} TYPE_WINDOWTAB_COLORSCHEME;

extern	TYPE_WINDOWTAB_COLORSCHEME	gWT_ColorScheme;
extern	int							gCurrWindowTabColorScheme;

//*****************************************************************************
enum
{
	kColorScheme_BlackRed	=	0,
	kColorScheme_BlackWht,
	kColorScheme_WhiteBlk,
	kColorScheme_GrayBlk,
	kColorScheme_Red,
	kColorScheme_Grn,
	kColorScheme_Blu,
	kColorScheme_Cyan,
	kColorScheme_Magenta,
	kColorScheme_Yellow,

	kMaxColorSchemes

};


//*****************************************************************************
//*	simple colors
//*	4/1/2021	Had to add W_ prefix to avoid conflicts with FLIR
//*				W_  for Window colors
//*****************************************************************************
enum
{
		W_WHITE	=	0,
		W_BLACK,

		W_RED,
		W_GREEN,
		W_BLUE,

		W_CYAN,
		W_MAGENTA,
		W_YELLOW,

		W_DARKRED,
		W_DARKGREEN,
		W_DARKBLUE,

		W_LIGHTGRAY,
		W_DARKGRAY,

		W_LIGHTMAGENTA,

		W_BROWN,
		W_PINK,
		W_COLOR_LAST

};

//*****************************************************************************
class WindowTab
{
	public:

		//
		// Construction
		//
				WindowTab(	const int	xSize,
							const int	ySize,
							CvScalar	backGrndColor,
							const char	*windowName=NULL);
		virtual	~WindowTab(void);

				//*	set up functions
		virtual	void	RunBackgroundTasks(void);
				void	ComputeWidgetColumns(const int windowWitdh);
				void	SetWidget(				const int widgetIdx, int left, int top, int width, int height);
				void	SetWidgetType(			const int widgetIdx, const int widetType);
				void	SetWidgetFont(			const int widgetIdx, int fontNum);
				void	SetWidgetJustification(	const int widgetIdx, int justification);
				void	SetWidgetTextColor(		const int widgetIdx, CvScalar newtextColor);
				void	SetWidgetBGColor(		const int widgetIdx, CvScalar newtextColor);
				void	SetWidgetBorderColor(	const int widgetIdx, CvScalar newtextColor);
				void	SetWidgetImage(			const int widgetIdx, IplImage *argImagePtr);
				void	SetWidgetOutlineBox(	const int widgetIdx, const int firstItem, const int lastItem);
				void	SetWidgetProgress(		const int widgetIdx, const int currPosition, const int totalValue);
				void	DumpWidgetList(			const int startIdx, const int stopIdx);

				//*	special purpose routines
				void	SetBGcolorFromWindowName(	const int	widgetIdx);
				void	SetIPaddressBoxes(			const int	ipaddrBox,
													const int	readAllBox,
													const int	versionBox,
													const int	connectBtnBox = -1);
				void	SetAlpacaLogo(const int logoWidgetIdx, const int lastCmdWidgetIdx = -1);
				int		SetAlpacaLogoBottomCorner(const int logoWidgetIdx);
				void	DisplayLastAlpacaCommand(void);

				//*	display value functions
				void	SetWidgetValid(			const int widgetIdx, bool valid);
				void	SetWidgetBorder(		const int widgetIdx, bool onOff);
				void	SetWidgetChecked(		const int widgetIdx, bool checked);
				void	SetWidgetCrossedout(	const int widgetIdx, bool crossedout);

				void	SetWidgetSliderLimits(	const int widgetIdx, double sliderMin, double sliderMax);
				void	SetWidgetSliderValue(	const int widgetIdx, double sliderValue);
		virtual	void	UpdateControls(void);
		virtual	void	UpdateSliderValue(		const int widgetIdx, double newSliderValue);

				void	SetWidgetText(			const int widgetIdx, const char *newText);
				void	SetWidgetTextPtr(		const int widgetIdx, char *textPtr);
				void	GetWidgetText(			const int widgetIdx, char *getText);
				void	SetWidgetTabStops(		const int widgetIdx, const short *tabStopList);
				void	SetWidgetNumber(		const int widgetIdx, const int number);
				void	SetWidgetNumber(		const int widgetIdx, const double number);
				void	SetWidgetIcon(			const int widgetIdx, const int iconNumber);

				void	SetWidgetHelpText(		const int widgetIdx, const char *newText);

				void	SetWidgetHighlighted(	const int widgetIdx, bool highLighted);


				int		FindClickedWidget(const int xxx, const int yyy);
				bool	IsWidgetButton(const int widgetIdx);
				bool	IsWidgetTextInput(const int widgetIdx);

				void	SetParentObjectPtr(void *argParentObjPtr);



				void	SetWindowTabColorScheme(const int colorScheme);
				void	BumpColorScheme(void);

				void	ForceUpdate(void);
				void	UpdateWindowAsNeeded(void);
		virtual	void	UpdateColors(void);


		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx);

		virtual	void	SetWindowIPaddrInfo(const char	*textString = NULL,
											const bool	onLine = true);


		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawWindow(void);
		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags);
		virtual void	ProcessMouseEvent(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonDown(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonUp(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);
		virtual void	ProcessMouseLeftButtonDragged(const int widgetIdx, const int event, const int xxx, const int yyy, const int flags);

				void	SetHelpTextBoxNumber(const int buttonIdx);
				void	SetHelpTextBoxColor(CvScalar newtextColor);
				bool	DisplayButtonHelpText(const int buttonIdx);


		//=============================================================
		//*	Drawing commands
		//*	these are the beginning of an abstraction layer to make it easier to
		//*	switch graphic environments later. Also they make it easier to
		//*	incorporate some older code
		//=============================================================
		void		CPenSize(const int newLineWidth);
		void		CMoveTo(const int xx, const int yy);
		void		CLineTo(const int xx, const int yy);
		void		DrawCString(const int xx, const int yy, const char *theString);
		void		SetColor(const int theColor);

		void		Putpixel(const int xx, const int yy, const int theColor);
		void		FillEllipse(int xCenter, int yCenter, int xRadius, int yRadius);
		void		FrameEllipse(int xCenter, int yCenter, int xRadius, int yRadius);

		IplImage	*cOpenCV_Image;
		int			cCurrentXloc;
		int			cCurrentYloc;
		CvScalar	cCurrentColor;
		int			cCurrentLineWidth;

		TYPE_WIDGET	cWidgetList[kMaxWidgets];
		int			cWidgetCnt;

		char		cWindowName[48];
		int			cWidth;
		int			cHeight;
		void		*cParentObjPtr;
//		Controller	*cParentObjPtr;

#ifdef _CONTROLLER_USES_ALPACA_
		//------------------------------------------------------------
		//*	Alpaca command stuff
		bool	AlpacaSendPutCmd(	sockaddr_in		*deviceAddress,
									int				devicePort,
									const char		*alpacaDevice,
									const int		alpacaDevNum,
									const char		*alpacaCmd,
									const char		*dataString,
									SJP_Parser_t	*jsonParser);

		bool	AlpacaSendPutCmd(	const char		*alpacaDevice,
									const char		*alpacaCmd,
									const char		*dataString,
									SJP_Parser_t	*jsonParser = NULL);

		bool	AlpacaGetIntegerValue(	const char	*alpacaDevice,
										const char	*alpacaCmd,
										const char	*dataString,
										int			*returnValue);

		int		AlpacaCheckForErrors(	SJP_Parser_t	*jsonParser,
										char			*errorMsg,
										bool			reportError=false);
virtual	void	AlpacaDisplayErrorMessage(const char *errorMsgString);
#endif // _CONTROLLER_USES_ALPACA_




	protected:
		//*	these are for consistency between tab windows
		const static int	cTabVertOffset	=	25;	//*	location for tabs to start drawing
		const static int	cTitleHeight	=	30;
		const static int	cLogoWidth		=	120;
		const static int	cLogoHeight		=	50;
		const static int	cBoxHeight		=	40;
		const static int	cBtnHeight		=	35;
		const static int	cRadioBtnHt		=	20;
		const static int	cSmIconSize		=	14;
		const static int	cSmallBtnHt		=	25;
		int					cFullWidthBtn;
		int					cBtnWidth;
		int					cClm1_offset;
		int					cClm2_offset;
		int					cClm3_offset;
		int					cClm4_offset;
		int					cClm5_offset;
		int					cClm6_offset;
		int					cClmWidth;
		int					cLrgBtnWidth;
		int					cLrgBtnHeight;

		//*	lets keep track of some of the stuff in each window to avoid duplication
		int					cIpAddrTextBox;		//*	the index of the box for the ip address, < 0 means not set

		int					cLastCmdTextBox;	//*	index of the box for the last alpaca command, < 0 means not set
//-		char				cLastAlpacaCmdString[256];

		int					cHelpTextBoxNumber;	//*	index of the box for help text (-1 is not set)
		CvScalar 			cHelpTextBoxColor;
		int					cPervDisplayedHelpBox;

		//*	alpaca stuff duplicated from controller class
		int					cLastAlpacaErrNum;
		char				cLastAlpacaErrStr[512];
};

void	SetRect(CvRect *theRect, const int top, const int left, const int bottom, const int right);
void	InsetRect(CvRect *theRect, const int xInset, const int yInset);

#endif // _WINDOW_TAB_H_
