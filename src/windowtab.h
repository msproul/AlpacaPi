//*****************************************************************************
//#include	"windowtab.h"

#ifndef _WINDOW_TAB_H_
#define	_WINDOW_TAB_H_

#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#ifndef _JSON_PARSE_H_
	#include	"json_parse.h"
#endif // _JSON_PARSE_H_

#include	"widget.h"


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
				void	SetBGcolorFromWindowName(const int widgetIdx);
				void	SetIPaddressBoxes(	const int	ipaddrBox,
											const int	readAllBox,
											const int	versionBox,
											const int	connectBtnBox = -1);
				void	SetAlpacaLogo(const int logoWidgetIdx, const int lastCmdWidgetIdx = -1);
				void	DisplayLastAlpacaCommand(void);

				//*	display value functions
				void	SetWidgetValid(			const int widgetIdx, bool valid);
				void	SetWidgetBorder(		const int widgetIdx, bool onOff);
				void	SetWidgetChecked(		const int widgetIdx, bool checked);
				void	SetWidgetCrossedout(	const int widgetIdx, bool crossedout);

				void	SetWidgetSliderLimits(	const int widgetIdx, double sliderMin, double sliderMax);
				void	SetWidgetSliderValue(	const int widgetIdx, double sliderValue);
				void	SetWidgetText(			const int widgetIdx, const char *newText);
				void	SetWidgetNumber(		const int widgetIdx, const int number);
				void	SetWidgetNumber(		const int widgetIdx, const double number);
				void	SetWidgetIcon(			const int widgetIdx, const int iconNumber);

				void	SetWidgetHighlighted(	const int widgetIdx, bool highLighted);

				int		FindClickedWidget(const int xxx, const int yyy);
				void	SetParentObjectPtr(void *argParentObjPtr);
				bool	AlpacaSendPutCmd(	const char		*alpacaDevice,
											const char		*alpacaCmd,
											const char		*dataString,
											SJP_Parser_t	*jsonParser = NULL);

				bool	AlpacaGetIntegerValue(	const char	*alpacaDevice,
												const char	*alpacaCmd,
												const char	*dataString,
												int			*returnValue);

				void	SetWindowTabColorScheme(const int colorScheme);
				void	BumpColorScheme(void);
		virtual	void	UpdateColors(void);


		virtual	void	DrawGraphWidget(IplImage *openCV_Image, const int widgitIdx);

		virtual	void	SetWindowIPaddrInfo(const char	*textString = NULL,
											const bool	onLine = true);


//		virtual	void	DrawGraphWidget(const int widgitIdx);
		virtual	void	SetupWindowControls(void);
//		virtual	void	DrawWindow(void);
//		virtual void	HandleKeyDown(const int keyPressed);
		virtual	void	ProcessButtonClick(const int buttonIdx);
		virtual	void	ProcessDoubleClick(const int buttonIdx);



		TYPE_WIDGET	cWidgetList[kMaxWidgets];
		int			cWidgetCnt;

		char		cWindowName[48];
		int			cWidth;
		int			cHeight;
		void		*cParentObjPtr;

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
};


#endif // _WINDOW_TAB_H_
