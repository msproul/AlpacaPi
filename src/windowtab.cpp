//*****************************************************************************
//*		windowtab.cpp		(c) 2020 by Mark Sproul
//*
//*	Description:	C++ Client to talk to Alpaca devices
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.  You
//*	must determine the suitability of this source code for your use.
//*
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Feb 23,	2020	<MLS> Started on controller_windowtabs.cpp
//*	Feb 23,	2020	<MLS> Window tabs starting to work
//*	Mar  1,	2020	<MLS> Added SetWidgetJustification()
//*	Mar  2,	2020	<MLS> Added SetWidgetChecked() & SetWidgetSliderLimits()
//*	Mar  3,	2020	<MLS> Added SetWidgetSliderValue() & SetWidgetValid()
//*	Mar  7,	2020	<MLS> Moved button sizes etc to parent for consistency
//*	Mar  8,	2020	<MLS> Number display now adjust decimal pts dynamically
//*	Apr 19,	2020	<MLS> Added DisplayLastAlpacaCommand()
//*	Apr 25,	2020	<MLS> Added SetWidgetBoarder()
//*	Jun 25,	2020	<MLS> Added DumpWidgetList()
//*	Jul  9,	2020	<MLS> Added color scheme options, SetWindowTabColorScheme()
//*	Jul  9,	2020	<MLS> Added BumpColorScheme(), UpdateColors()
//*****************************************************************************



#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"



#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"commoncolor.h"
#include	"windowtab.h"
#include	"controller.h"


TYPE_WINDOWTAB_COLORSCHEME	gWT_ColorScheme;

int	gCurrWindowTabColorScheme	=	0;



//**************************************************************************************
WindowTab::WindowTab(	const int	xSize,
						const int	ySize,
						CvScalar	backGrndColor,
						const char	*windowName)
{
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	cWidth			=	xSize;
	cHeight			=	ySize;
	cParentObjPtr	=	NULL;
	cWindowName[0]	=	0;


	//*	these are for consistency between window tabs
	cClmWidth		=	cWidth / 6;
	cClmWidth		-=	2;
	cBtnWidth		=	cWidth / 6;
	cClm1_offset	=	3;
	cClm2_offset	=	1 * cClmWidth;
	cClm3_offset	=	2 * cClmWidth;
	cClm4_offset	=	3 * cClmWidth;
	cClm5_offset	=	4 * cClmWidth;
	cClm6_offset	=	5 * cClmWidth;
	cLrgBtnWidth	=	cClmWidth;
	cLrgBtnHeight	=	cClmWidth / 2;
	cFullWidthBtn	=	cWidth - cClm1_offset - 1;

	cIpAddrTextBox	=	-1;
	cLastCmdTextBox	=	-1;

	if (windowName != NULL)
	{
		strcpy(cWindowName, windowName);
	}

	//*	set defaults for the widgets
	for (iii=0; iii<kMaxWidgets; iii++)
	{
		memset(&cWidgetList[iii], 0, sizeof(TYPE_WIDGET));

		cWidgetList[iii].widgetType		=	kWidgetType_Default;
		cWidgetList[iii].fontNum		=	kFont_Large;
		cWidgetList[iii].justification	=	kJustification_Center;

	//	cWidgetList[iii].bgColor		=	CV_RGB(0, 0, 0);
		cWidgetList[iii].bgColor		=	backGrndColor;
		cWidgetList[iii].textColor		=	CV_RGB(255, 0, 0);
		cWidgetList[iii].borderColor	=	CV_RGB(255, 255, 255);

		cWidgetList[iii].includeBorder	=	true;
	}
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTab::~WindowTab(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}


//**************************************************************************************
void	WindowTab::SetWidget(const int widgetIdx, int left, int top, int width, int height)
{
	if ((left < 0) || (top < 0) || (width < 0) || (height < 0))
	{
		CONSOLE_DEBUG_W_NUM("Invalid widget dimensions, widget#=", widgetIdx);
		CONSOLE_DEBUG_W_NUM("left\t=", left);
		CONSOLE_DEBUG_W_NUM("top\t=", top);
		CONSOLE_DEBUG_W_NUM("width\t=", width);
		CONSOLE_DEBUG_W_NUM("height\t=", height);
	}

	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].valid		=	true;
		cWidgetList[widgetIdx].left			=	left;
		cWidgetList[widgetIdx].top			=	top;
		cWidgetList[widgetIdx].width		=	width;
		cWidgetList[widgetIdx].height		=	height;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetType(const int widgetIdx, const int widetType)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].widgetType	=	widetType;
		if ((widetType == kWidgetType_RadioButton) || (widetType == kWidgetType_CheckBox))
		{
			cWidgetList[widgetIdx].includeBorder	=	false;
		}
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetIcon(const int widgetIdx, const int iconNumber)
{

	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].widgetType	=	kWidgetType_Icon;
		cWidgetList[widgetIdx].iconNum		=	iconNumber;
	}
}


//**************************************************************************************
void	WindowTab::SetWidgetFont(const int widgetIdx, const int fontNum)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].fontNum	=	fontNum;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetJustification(	const int widgetIdx, int justification)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].justification	=	justification;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetText(const int widgetIdx, const char *newText)
{
//*	this is for very specific debugging
//	if (widgetIdx == 41)
//	{
//		CONSOLE_DEBUG_W_STR(__FUNCTION__, newText);
//	}

	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		strcpy(cWidgetList[widgetIdx].textString, newText);
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("widgetIdx out of range\t=", widgetIdx);
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetNumber(const int widgetIdx, const int number)
{
char	lineBuff[64];

	sprintf(lineBuff, "%d", number);
	SetWidgetText(widgetIdx, lineBuff);
}


//**************************************************************************************
void	WindowTab::SetWidgetNumber(const int widgetIdx, const double number)
{
char	lineBuff[64];

	if ((number < 0.002) && (number > 0.00000001))
	{
		sprintf(lineBuff, "%0.6f", number);
	}
	else
	{
		sprintf(lineBuff, "%1.4f", number);
	}
	SetWidgetText(widgetIdx, lineBuff);
}


//**************************************************************************************
void	WindowTab::SetWidgetTextColor(const int widgetIdx, CvScalar newtextColor)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].textColor	=	newtextColor;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetBGColor(const int widgetIdx, CvScalar newtextColor)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].bgColor	=	newtextColor;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetBorderColor(const int widgetIdx, CvScalar newtextColor)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].borderColor	=	newtextColor;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetImage(			const int widgetIdx, IplImage *argImagePtr)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].openCVimagePtr	=	argImagePtr;
		cWidgetList[widgetIdx].widgetType		=	kWidgetType_Image;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetOutlineBox(const int widgetIdx, const int firstItem, const int lastItem)
{
int		iii;
int		boundingBoxLeft;
int		boundingBoxRight;
int		boundingBoxTop;
int		boundingBoxBottom;
int		boundingBoxWidth;
int		boundingBoxHeight;


	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		boundingBoxLeft		=	10000;
		boundingBoxRight	=	0;
		boundingBoxTop		=	10000;
		boundingBoxBottom	=	0;

		for (iii=firstItem; iii<=lastItem; iii++)
		{
			//*	check the left and right boundaries
			if (cWidgetList[iii].left < boundingBoxLeft)
			{
				boundingBoxLeft	=	cWidgetList[iii].left;
			}
			if ((cWidgetList[iii].left + cWidgetList[iii].width) > boundingBoxRight)
			{
				boundingBoxRight	=	cWidgetList[iii].left + cWidgetList[iii].width;
			}
			//*	check the top and bottom boundaries
			if (cWidgetList[iii].top < boundingBoxTop)
			{
				boundingBoxTop	=	cWidgetList[iii].top;
			}
			if ((cWidgetList[iii].top + cWidgetList[iii].height) > boundingBoxBottom)
			{
				boundingBoxBottom	=	cWidgetList[iii].top + cWidgetList[iii].height;
			}
		}
		//*	leave some room
		boundingBoxLeft		-=	2;
		boundingBoxTop		-=	2;
		boundingBoxRight	+=	2;
		boundingBoxBottom	+=	2;
		//*	check limits
		if (boundingBoxLeft < 0)
		{
			boundingBoxLeft	=	0;
		}
		if (boundingBoxLeft < 0)
		{
			boundingBoxLeft	=	0;
		}


		boundingBoxWidth	=	boundingBoxRight - boundingBoxLeft;
		boundingBoxHeight	=	boundingBoxBottom - boundingBoxTop;
		if ((boundingBoxWidth > 0) && (boundingBoxHeight > 0))
		{
			SetWidget(		widgetIdx,
							boundingBoxLeft,
							boundingBoxTop,
							boundingBoxWidth,
							boundingBoxHeight);
			SetWidgetType(	widgetIdx,	kWidgetType_OutlineBox);
		}
		else
		{
			CONSOLE_DEBUG("ERROR!!!");
		}
	}
}

//**************************************************************************************
void	WindowTab::SetBGcolorFromWindowName(const int widgetIdx)
{
RGBcolor	bgColor;
RGBcolor	txColor;

//	CONSOLE_DEBUG(__FUNCTION__);
	GetDefaultColors(gColorOverRide, cWindowName, &bgColor, &txColor);
	SetWidgetBGColor(	widgetIdx,	CV_RGB(bgColor.red,		bgColor.grn,		bgColor.blu));
	SetWidgetTextColor(	widgetIdx,	CV_RGB(txColor.red,		txColor.grn,		txColor.blu));
}

//**************************************************************************************
//*	this is for consistency between window tabs
//*	setup the
//*		ip address box
//*		readall indicator
//*		Version box
//*		optional connect button
//**************************************************************************************
void	WindowTab::SetIPaddressBoxes(	const int	ipaddrBox,
										const int	readAllBox,
										const int	versionBox,
										const int	connectBtnBox)
{
int		yLoc;
int		connBtnWidth;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
//	CONSOLE_DEBUG_W_NUM("ipaddrBox\t=", ipaddrBox);
//	CONSOLE_DEBUG_W_NUM("readAllBox\t=", readAllBox);
//	CONSOLE_DEBUG_W_NUM("versionBox\t=", versionBox);
//	CONSOLE_DEBUG_W_NUM("connectBtnBox\t=", connectBtnBox);

	cIpAddrTextBox	=	ipaddrBox;

	if (versionBox > 0)
	{
		//=======================================================
		yLoc	=	cHeight - (2 *cBtnHeight);
		yLoc	-=	1;
		SetWidget(			versionBox,	0,	yLoc,		cWidth,	cBtnHeight);
		SetWidgetFont(		versionBox,	kFont_Medium);
		SetWidgetTextColor(	versionBox,	CV_RGB(255,	0,	0));
	}

	//=======================================================
	//*	IP address
	yLoc	=	cHeight - cBtnHeight;
	yLoc	-=	1;
	//*	check for connect button
	if (connectBtnBox > 0)
	{
		//*	a connect button was specified, adjust the IP box to be smaller
		SetWidget(		ipaddrBox,	0,			yLoc,		(cClmWidth * 4),	cBtnHeight);
		SetWidgetFont(	ipaddrBox,	kFont_Medium);

		//*	now setup the "Connect" button
		connBtnWidth	=	cWidth - cClm5_offset - 2;
		SetWidget(				connectBtnBox,	cClm5_offset + 1,	yLoc,	connBtnWidth,		cBtnHeight);
		SetWidgetFont(			connectBtnBox,	kFont_Medium);
		SetWidgetText(			connectBtnBox,	"Connect");

		SetWidgetBGColor(		connectBtnBox,		CV_RGB(255, 255, 255));
		SetWidgetBorderColor(	connectBtnBox,		CV_RGB(0, 0, 0));
		SetWidgetTextColor(		connectBtnBox,		CV_RGB(0, 0, 0));
	}
	else
	{
		SetWidget(		ipaddrBox,	0,	yLoc,		cWidth,	cBtnHeight);
		SetWidgetFont(	ipaddrBox, kFont_Medium);
	}

	//=======================================================
	//*	this is ON TOP of the IP box on purpose
	//*	this is an indicator, "R" to signify that the alpaca driver supports READALL
	if (readAllBox >= 0)
	{
		SetWidget(				readAllBox,	4,	yLoc+2,		cBtnHeight-4,	cBtnHeight-4);
		SetWidgetBorderColor(	readAllBox,	CV_RGB(0,	0,	0));
		SetWidgetValid(			readAllBox,	false);		//*	will only be enabled if READALL command exists
		SetWidgetText(			readAllBox,	"R");
		SetWidgetFont(			readAllBox,	kFont_Medium);
		SetWidgetTextColor(		readAllBox,	CV_RGB(0,255,	0));
	}
}

//**************************************************************************************
//*	this puts the alpaca logo in the default place
//*	To override the position, call SetWiget after calling this routine
//**************************************************************************************
void	WindowTab::SetAlpacaLogo(const int logoWidgetIdx, const int lastCmdWidgetIdx)
{
int	logoWidth;
int	logoHeight;
int	xLoc;
int	yLoc;

	if (lastCmdWidgetIdx >= 0)
	{
		cLastCmdTextBox	=	lastCmdWidgetIdx;
		yLoc			=	cHeight - (3 * cBtnHeight);
		SetWidget(			lastCmdWidgetIdx,	0,			yLoc,		cWidth,		cBtnHeight);
		SetWidgetText(		lastCmdWidgetIdx,	"---");
		SetWidgetFont(		lastCmdWidgetIdx,	kFont_Medium);
		SetWidgetTextColor(	lastCmdWidgetIdx,	CV_RGB(0,	255,	0));
	}
	else
	{
		yLoc		=	cHeight - (2 *cBtnHeight);
	}

	if (logoWidgetIdx >= 0)
	{
		//*	now set the Alpaca Logo
		LoadAlpacaLogo();
		if (gAlpacaLogoPtr != NULL)
		{
			logoWidth	=	gAlpacaLogoPtr->width;
			logoHeight	=	gAlpacaLogoPtr->height;

			xLoc		=	cWidth - logoWidth;
			yLoc		-=	logoHeight;

			SetWidget(		logoWidgetIdx,	xLoc,	yLoc,	logoWidth,	logoHeight);
			SetWidgetImage(	logoWidgetIdx, gAlpacaLogoPtr);
		}
	}
}

//**************************************************************************************
void	WindowTab::DisplayLastAlpacaCommand(void)
{
char		textStr[256];
Controller	*myControllerObj;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cLastCmdTextBox >= 0)
	{
		myControllerObj	=	(Controller *)cParentObjPtr;
		if (myControllerObj != NULL)
		{
			if (strlen(myControllerObj->cLastAlpacaCmdString) > 0)
			{
				strcpy(textStr, "Cmd=");
				strcat(textStr, myControllerObj->cLastAlpacaCmdString);
				SetWidgetText(	cLastCmdTextBox, textStr);
				CONSOLE_DEBUG_W_STR("cLastAlpacaCmdString\t=", textStr);
			}
		}
	}
}


//**************************************************************************************
void	WindowTab::SetWidgetValid(const int widgetIdx, bool valid)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, widgetIdx);

	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].valid	=	valid;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetBorder(const int widgetIdx, bool onOff)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].includeBorder	=	onOff;
	}
}


//**************************************************************************************
void	WindowTab::SetWidgetChecked(	const int widgetIdx, bool checked)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].selected	=	checked;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetCrossedout(const int widgetIdx, bool crossedout)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].crossedOut	=	crossedout;
	}
}

//**************************************************************************************

void	WindowTab::SetWidgetHighlighted(	const int widgetIdx, bool highLighted)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].highLighted	=	highLighted;
	}
}


//**************************************************************************************
void	WindowTab::SetWidgetSliderLimits(const int widgetIdx, double sliderMin, double sliderMax)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].sliderMin	=	sliderMin;
		cWidgetList[widgetIdx].sliderMax	=	sliderMax;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetSliderValue(const int widgetIdx, double sliderValue)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].sliderValue	=	sliderValue;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetProgress(const int widgetIdx, const int currPosition, const int totalValue)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].widgetType	=	kWidgetType_ProessBar;
		cWidgetList[widgetIdx].sliderValue	=	currPosition;
		cWidgetList[widgetIdx].sliderMax	=	totalValue;
	}
}


//**************************************************************************************
void	WindowTab::SetupWindowControls(void)
{

}

//**************************************************************************************
void	WindowTab::DrawGraphWidget(IplImage *openCV_Image, const int widgitIdx)
{
	//*	this routine should be overloaded
}



//*****************************************************************************
int	WindowTab::FindClickedWidget(const int xxx, const int yyy)
{
int		ii;
bool	ptInWidget;
int		widgetIdx;

	widgetIdx	=	-1;
	ii			=	0;
	while ((ii<kMaxWidgets) && (widgetIdx < 0))
	{
		if (cWidgetList[ii].valid)
		{
			ptInWidget	=	PointInWidget(xxx, yyy, &cWidgetList[ii]);
			if (ptInWidget)
			{
				widgetIdx	=	ii;
			}
		}
		ii++;
	}
	return(widgetIdx);
}

//*****************************************************************************
void	WindowTab::ProcessButtonClick(const int buttonIdx)
{
	CONSOLE_DEBUG_W_NUM("this routine should be overloaded: buttonIdx=", buttonIdx);
	//*	this routine should be overloaded
}

//*****************************************************************************
void	WindowTab::ProcessDoubleClick(const int buttonIdx)
{
	CONSOLE_DEBUG("this routine should be overloaded");
	//*	this routine should be overloaded
}

//*****************************************************************************
void	WindowTab::SetParentObjectPtr(void *argParentObjPtr)
{
	cParentObjPtr	=	argParentObjPtr;
}

//*****************************************************************************
void	WindowTab::SetWindowIPaddrInfo(	const char	*textString,
										const bool	onLine)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, (onLine ? "online" : "Offline"));
	//*	this can be be overloaded,
	if (cIpAddrTextBox > 0)
	{
		if (textString != NULL)
		{
			SetWidgetText(cIpAddrTextBox, textString);
		}
		if (onLine)
		{
			SetWidgetBGColor(	cIpAddrTextBox,	CV_RGB(0, 0, 0));
			SetWidgetTextColor(	cIpAddrTextBox,	CV_RGB(255, 0, 0));
		}
		else
		{
			SetWidgetBGColor(	cIpAddrTextBox,	CV_RGB(255, 0, 0));
			SetWidgetTextColor(	cIpAddrTextBox,	CV_RGB(255, 255, 255));
		}
	}
	else
	{
		CONSOLE_DEBUG("cIpAddrTextBox not set!!!!!");
	}
}

//*****************************************************************************
bool	WindowTab::AlpacaSendPutCmd(const char		*alpacaDevice,
									const char		*alpacaCmd,
									const char		*dataString,
									SJP_Parser_t	*jsonParser)
{
bool		validData;
Controller	*myControllerObj;

	CONSOLE_DEBUG(__FUNCTION__);
	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		CONSOLE_DEBUG_W_STR(alpacaDevice, alpacaCmd);
		if (jsonParser != NULL)
		{
			validData	=	myControllerObj->AlpacaSendPutCmdwResponse(	alpacaDevice,
																		alpacaCmd,
																		dataString,
																		jsonParser);
		}
		else
		{
			validData	=	myControllerObj->AlpacaSendPutCmd(	alpacaDevice,
																alpacaCmd,
																dataString);
		}
//-		strcpy(cLastAlpacaCmdString, myControllerObj->cLastAlpacaCmdString);
	}
	else
	{
		validData	=	false;
	}
	return(validData);
}


//*****************************************************************************
bool	WindowTab::AlpacaGetIntegerValue(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString,
											int			*returnValue)
{
bool		validData;
Controller	*myControllerObj;

	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		validData	=	myControllerObj->AlpacaGetIntegerValue(	alpacaDevice,
																alpacaCmd,
																dataString,
																returnValue);
	}
	else
	{
		validData	=	false;
	}
	return(validData);
}


//*****************************************************************************
void	WindowTab::DumpWidgetList(const int startIdx, const int stopIdx)
{
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);
	for (iii=startIdx; ((iii<=stopIdx) && (iii < kMaxWidgets)); iii++)
	{
		printf("%2d\t%d\t%d\t%s\r\n",
							iii,
							cWidgetList[iii].valid,
							cWidgetList[iii].widgetType,
							cWidgetList[iii].textString
							);
	}
}

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
void	WindowTab::SetWindowTabColorScheme(const int colorScheme)
{
Controller	*myControllerObj;

	switch(colorScheme)
	{
		case kColorScheme_BlackRed:
			gWT_ColorScheme.bgColor			=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.fontColor		=	CV_RGB(255, 0, 0);
			gWT_ColorScheme.borderColor		=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.btnColor		=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.btnFontColor	=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.btnBorderColor	=	CV_RGB(0, 0, 0);
			break;

		case kColorScheme_BlackWht:
			gWT_ColorScheme.bgColor			=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.fontColor		=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.borderColor		=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.btnColor		=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.btnFontColor	=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.btnBorderColor	=	CV_RGB(0, 0, 0);
			break;


		case kColorScheme_WhiteBlk:
			gWT_ColorScheme.bgColor			=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.fontColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.borderColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.btnColor		=	CV_RGB(128, 128, 128);
			gWT_ColorScheme.btnFontColor	=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.btnBorderColor	=	CV_RGB(0, 0, 0);
			break;

		case kColorScheme_GrayBlk:
			gWT_ColorScheme.bgColor			=	CV_RGB(128, 128, 128);
			gWT_ColorScheme.fontColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.borderColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.btnColor		=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.btnFontColor	=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.btnBorderColor	=	CV_RGB(0, 0, 0);
			break;

		//*	red
		case kColorScheme_Red:
			gWT_ColorScheme.bgColor			=	CV_RGB(255, 128, 128);
			gWT_ColorScheme.fontColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.borderColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.btnColor		=	CV_RGB(128, 0, 0);
			gWT_ColorScheme.btnFontColor	=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.btnBorderColor	=	CV_RGB(255, 255, 255);
			break;

		//*	green
		case kColorScheme_Grn:
			gWT_ColorScheme.bgColor			=	CV_RGB(128, 255, 128);
			gWT_ColorScheme.fontColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.borderColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.btnColor		=	CV_RGB(0, 128, 0);
			gWT_ColorScheme.btnFontColor	=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.btnBorderColor	=	CV_RGB(255, 255, 255);
			break;

		//*	blue
		case kColorScheme_Blu:
			gWT_ColorScheme.bgColor			=	CV_RGB(128, 128, 255);
			gWT_ColorScheme.fontColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.borderColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.btnColor		=	CV_RGB(0, 0, 128);
			gWT_ColorScheme.btnFontColor	=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.btnBorderColor	=	CV_RGB(255, 255, 255);
			break;

		case kColorScheme_Cyan:
			gWT_ColorScheme.bgColor			=	CV_RGB(128, 255, 255);
			gWT_ColorScheme.fontColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.borderColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.btnColor		=	CV_RGB(0, 128, 128);
			gWT_ColorScheme.btnFontColor	=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.btnBorderColor	=	CV_RGB(255, 255, 255);
			break;

		case kColorScheme_Magenta:
			gWT_ColorScheme.bgColor			=	CV_RGB(255, 128, 255);
			gWT_ColorScheme.fontColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.borderColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.btnColor		=	CV_RGB(128, 0, 128);
			gWT_ColorScheme.btnFontColor	=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.btnBorderColor	=	CV_RGB(255, 255, 255);
			break;

		case kColorScheme_Yellow:
			gWT_ColorScheme.bgColor			=	CV_RGB(255, 255, 128);
			gWT_ColorScheme.fontColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.borderColor		=	CV_RGB(0, 0, 0);
			gWT_ColorScheme.btnColor		=	CV_RGB(128, 128, 0);
			gWT_ColorScheme.btnFontColor	=	CV_RGB(255, 255, 255);
			gWT_ColorScheme.btnBorderColor	=	CV_RGB(255, 255, 255);
			break;

	}
	//*	update the master back ground color
	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		myControllerObj->cBackGrndColor	=	gWT_ColorScheme.bgColor;
		myControllerObj->UpdateWindowTabColors();
	}
	UpdateColors();
}


//*****************************************************************************
//*	this is the default, it can be over ridden
//*****************************************************************************
void	WindowTab::UpdateColors(void)
{
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	for (iii=0; iii<kMaxWidgets; iii++)
	{

		switch(cWidgetList[iii].widgetType)
		{
			case	kWidgetType_Default:
			case	kWidgetType_MultiLineText:
			case	kWidgetType_RadioButton:
			case	kWidgetType_OutlineBox:
			case	kWidgetType_ProessBar:
			case	kWidgetType_ScrollBar:
			case	kWidgetType_Slider:
			case	kWidgetType_Text:
				SetWidgetBGColor(		iii,	gWT_ColorScheme.bgColor);
				SetWidgetTextColor(		iii,	gWT_ColorScheme.fontColor);
				SetWidgetBorderColor(	iii,	gWT_ColorScheme.borderColor);
				break;

			case	kWidgetType_Button:
			case	kWidgetType_CheckBox:
				SetWidgetBGColor(		iii,	gWT_ColorScheme.btnColor);
				SetWidgetTextColor(		iii,	gWT_ColorScheme.btnFontColor);
				SetWidgetBorderColor(	iii,	gWT_ColorScheme.btnBorderColor);
				break;


			case	kWidgetType_Custom:
			case	kWidgetType_Graph:
			case	kWidgetType_Graphic:
			case	kWidgetType_Icon:
			case	kWidgetType_Image:
				break;


		}
	}
}

//*****************************************************************************
void	WindowTab::BumpColorScheme(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	gCurrWindowTabColorScheme++;
	if (gCurrWindowTabColorScheme >= kMaxColorSchemes)
	{
		gCurrWindowTabColorScheme	=	0;
	}
	SetWindowTabColorScheme(gCurrWindowTabColorScheme);
}
