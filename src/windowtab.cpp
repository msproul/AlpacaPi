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
//*	Feb 23,	2020	<MLS> Started on windowtab.cpp
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
//*	Dec 30,	2020	<MLS> Added HandleKeyDown()
//*	Dec 31,	2020	<MLS> Added help text string to widget definition
//*	Dec 31,	2020	<MLS> Added DisplayButtonHelpText() & SetHelpTextBoxNumber()
//*	Dec 31,	2020	<MLS> Added ProcessMouseEvent()
//*	Jan  1,	2021	<MLS> Added flag needsUpdated for selective updating
//*	Jan  1,	2021	<MLS> Added FrameEllipse()
//*	Jan  4,	2021	<MLS> Added ForceUpdate()
//*	Jan  4,	2021	<MLS> Added ProcessMouseLeftButtonDown() & ProcessMouseLeftButtonDragged()
//*	Jan  5,	2021	<MLS> Added RunBackgroundTasks()
//*	Jan  5,	2021	<MLS> Added LLD_PenSize()
//*	Jan  6,	2021	<MLS> Added GetWidgetText()
//*	Jan 10,	2021	<MLS> Added new version of AlpacaSendPutCmd()
//*	Jan 15,	2021	<MLS> Added SetWidgetTabStops()
//*	Jan 23,	2021	<MLS> Added overflow checking to help text string
//*	Jan 30,	2021	<MLS> Added ComputeWidgetColumns()
//*	Feb 10,	2021	<MLS> Added SetAlpacaLogoBottomCorner()
//*	Feb 13,	2021	<MLS> Added UpdateSliderValue()
//*	Feb 13,	2021	<MLS> Dragging the slider now works
//*	Feb 16,	2021	<MLS> Added SetHelpTextBoxColor()
//*	Mar 25,	2021	<MLS> Added SetWidgetTextPtr()
//*	Jul 18,	2021	<MLS> Added SetWidgetScrollBarLimits()
//*	Jul 18,	2021	<MLS> Added SetWidgetScrollBarValue()
//*	Aug  9,	2021	<MLS> Added ProcessMouseWheelMoved()
//*	Aug 11,	2021	<MLS> Added SetCurrentTab()
//*	Aug 31,	2021	<MLS> Added fontIndex arg to DrawCString()
//*	Sep  8,	2021	<MLS> Added AlpacaSetConnected()
//*	Sep  9,	2021	<MLS> Added SetUpConnectedIndicator()
//*	Oct 29,	2021	<MLS> Added FloodFill()
//*	Nov 13,	2021	<MLS> Added ProcessDoubleClick_RtBtn()
//*	Jan 24,	2022	<MLS> Added flags argument to ProcessMouseWheelMoved()
//*	Feb 21,	2022	<MLS> Changed all references of CvScalar to cv::Scalar
//*	Feb 26,	2022	<MLS> floodfill working under opencv C++
//*	Apr  9,	2022	<MLS> Made some changes to be compatible with openCV ver 2
//*	Apr  9,	2022	<MLS> OpenCV version 2.4.9.1 is default on R-Pi 3 (stretch)
//*	May 29,	2022	<MLS> Added ForceAlpacaUpdate()
//*	Jun  4,	2022	<MLS> Added flags arg to ProcessButtonClick()
//*	Jun 13,	2022	<MLS> Added ClearLastAlpacaCommand()
//*	Jun 29,	2022	<MLS> Added cLeftButtonDown & cRightButtonDown to WindowTab
//*	Sep 28,	2022	<MLS> Added SetWidgetNumber() with specified number of decimal places
//*	Oct  4,	2022	<MLS> Added HandleSpecialKeys()
//*	Oct  4,	2022	<MLS> Moved a bunch of the list stuff to the main windowtab.cpp class
//*	Oct  4,	2022	<MLS> Added page-up, page-down and home processing for lists
//*	Oct 20,	2022	<MLS> Changed SetIPaddressBoxes() to have errorMsgBox instead of versionBox
//*	Oct 21,	2022	<MLS> Removed AlpacaDisplayErrorMessage from all windowtab classes
//*	Oct 21,	2022	<MLS> Added SetupWindowBottomBoxes()
//*	Oct 21,	2022	<MLS> Updated all windowtabs to use SetupWindowBottomBoxes()
//*	Oct 21,	2022	<MLS> Removed SetIPaddressBoxes() use SetupWindowBottomBoxes() instead
//*	Oct 21,	2022	<MLS> Removed SetAlpacaLogo()
//*	Nov 16,	2022	<MLS> Changed RunBackgroundTasks() to RunWindowBackgroundTasks()
//*	Dec 17,	2022	<MLS> Added DrawGraph()
//*	Dec 18,	2022	<MLS> Added LLD_GetColor()
//*	Dec 25,	2022	<MLS> Added SetWidgetAltText() (initially for progress bar text)
//*	Feb 22,	2023	<MLS> Added SetTitleBox()
//*	Feb 24,	2023	<MLS> Added connected box to SetTitleBox()
//*	Feb 24,	2023	<MLS> Switched all window tabs to use SetTitleBox()
//*	Feb 26,	2023	<MLS> Added DrawHistogram()
//*	Feb 27,	2023	<MLS> Added default option to HandleSpecialKeys()
//*	Mar 14,	2023	<MLS> Added LaunchWebHelp()
//*	Mar 14,	2023	<MLS> Added helpbuttonbox and logo side of window to SetupWindowBottomBoxes()
//*	May 25,	2023	<MLS> Added CloseWindow()
//*	Jun  2,	2023	<MLS> Changed LLD_ to LLG_  Low Level Graphics
//*	Jun 18,	2023	<MLS> Added DeviceSelect to SetupWindowBottomBoxes()
//*	Jun 18,	2023	<MLS> Adding DS indicator next to Readall indicator on all windowtabs
//*	Jun 19,	2023	<MLS> Added DumpWidget()
//*	Jun 23,	2023	<MLS> Improvements to DumpWidget()
//*	Aug 24,	2023	<MLS> Added angle to ellipse functions
//*	Sep 26,	2023	<MLS> Added SetAlpacaDeviceType()
//*	Dec 17,	2023	<MLS> Added SetWidgetNumber() with unsigned int argument
//*	Dec 17,	2023	<MLS> Cleaned up widget types in SetupWindowBottomBoxes()
//*	Jan 19,	2024	<MLS> Added ClearWidgetSelect()
//*	Jan 20,	2024	<MLS> Added SetWidgetLineSelect()
//*	Jan 20,	2024	<MLS> Added SetWidgetBGColorSelected()
//*	Jan 21,	2024	<MLS> Added ProcessLineSelect()
//*	Feb  1,	2024	<MLS> Added LaunchWebRemoteDevice()
//*	Mar 21,	2024	<MLS> Added kWidgetType_TextBox_MonoSpace
//*	Mar 23,	2024	<MLS> Added LaunchWebURL()
//*	Mar 26,	2024	<MLS> Added LLG_FrameRect(cv::Rect *theRect) & LLG_FillRect(cv::Rect *theRect)
//*	Mar 27,	2024	<MLS> Added SetRunFastBackgroundMode()
//*	Mar 28,	2024	<MLS> Added SetWidgetTextColor() with color index arg
//*	Apr  1,	2024	<MLS> Added SetWebHelpURLstring()
//*****************************************************************************


#include	<stdio.h>
#include	<stdlib.h>
#include	<stdbool.h>
#include	<unistd.h>
#include	<sys/time.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"commoncolor.h"
#include	"helper_functions.h"
#include	"widget.h"
#include	"windowtab.h"
#include	"controller.h"

TYPE_WINDOWTAB_COLORSCHEME	gWT_ColorScheme;

int	gCurrWindowTabColorScheme	=	0;

//*****************************************************************************
//*	W_WHITE
//*	W_BLACK
cv::Scalar	gColorTable[]	=
{
	//	https://www.htmlcolor-picker.com/
	//*	these MUST be in the same order as the enums
	CV_RGB(255,	255,	255),	//*	W_WHITE
	CV_RGB(000,	000,	000),	//*	W_BLACK

	CV_RGB(255,	000,	000),	//*	W_RED
	CV_RGB(000,	255,	000),	//*	W_GREEN
	CV_RGB(000,	000,	255),	//*	W_BLUE

	CV_RGB(000,	255,	255),	//*	W_CYAN
	CV_RGB(255,	000,	255),	//*	W_MAGENTA
	CV_RGB(255,	255,	000),	//*	W_YELLOW

	CV_RGB(96,	000,	000),	//*	W_DARKRED
	CV_RGB(000,	 110,	000),	//*	W_DARKGREEN
	CV_RGB(000,	000,	150),	//*	W_DARKBLUE

	CV_RGB(192,	192,	192),	//*	W_LIGHTGRAY
	CV_RGB(90,	 90,	 90),	//*	W_DARKGRAY
	CV_RGB(45,	 45,	 45),	//*	W_VDARKGRAY

	CV_RGB(255,	128,	255),	//*	W_LIGHTMAGENTA

	CV_RGB(0x66, 0x3d,	0x14),	//*	W_BROWN
	CV_RGB(231,		5,	254),	//*	W_PINK
	CV_RGB(255,	100,	0),		//*	W_ORANGE,

	//*	these are special case so that I can have cross hairs to match my scope colors
	CV_RGB(255,	000,	255),	//*	W_PURPLE
	CV_RGB(255,	255,	000),	//*	W_GOLD
	CV_RGB(255,	000,	000),	//*	W_RED2

	CV_RGB(0,	113,	193),	//*	W_STAR_O,
	CV_RGB(152,	205,	255),	//*	W_STAR_B,
	CV_RGB(255,	255,	255),	//*	W_STAR_A,
	CV_RGB(254,	255,	153),	//*	W_STAR_F,
	CV_RGB(255,	255,	0),		//*	W_STAR_G,
	CV_RGB(255,	102,	00),	//*	W_STAR_K,
	CV_RGB(254,	0,		0),		//*	W_STAR_M,
	CV_RGB(127,	216,	250),	//*	W_FILTER_OIII,	//*	these are for filter colors
	CV_RGB(151,	253,	151),	//*	W_FILTER_HA,
	CV_RGB(255,	85,		85),	//*	W_FILTER_SII,
};



//**************************************************************************************
WindowTab::WindowTab(	const int	xSize,
						const int	ySize,
						cv::Scalar	backGrndColor,
						const char	*windowName)
{
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("xSize        \t=",	xSize);
//	CONSOLE_DEBUG_W_NUM("ySize        \t=",	ySize);

	cWidth				=	xSize;
	cHeight				=	ySize;
	cParentObjPtr		=	NULL;
	cWindowName[0]		=	0;
	cLeftButtonDown		=	false;
	cRightButtonDown	=	false;
	cHistogramPenSize	=	2;

	ComputeWidgetColumns(cWidth);

	cIpAddrTextBox				=	-1;
	cLastCmdTextBox				=	-1;
	cHelpTextBoxNumber			=	-1;
	cConnectedStateBoxNumber	=	-1;
	cHelpTextBoxColor			=	CV_RGB(255,	255,	255);
	cPrevDisplayedHelpBox		=	-1;

	//*	for windows that have a list
	cLinesOnScreen			=	0;
	cSortColumn				=	-1;
	cFirstLineIdx			=	0;
	cTotalLines				=	0;

	//*	openCV stuff
	cOpenCV_Image			=	NULL;
	cCurrentXloc			=	0;
	cCurrentYloc			=	0;
	cCurrentColor			=	CV_RGB(255,	255,	255);
	cCurrentLineWidth		=	1;
	cWebURLstring[0]		=	0;
	cAlpacaDeviceTypeStr[0]	=	0;

	if (windowName != NULL)
	{
		strcpy(cWindowName, windowName);
	}

	//*	set defaults for the widgets
	for (iii=0; iii<kMaxWidgets; iii++)
	{
		memset((void *)&cWidgetList[iii], 0, sizeof(TYPE_WIDGET));

		cWidgetList[iii].widgetType		=	kWidgetType_Default;
		cWidgetList[iii].fontNum		=	kFont_Large;
		cWidgetList[iii].justification	=	kJustification_Center;

		cWidgetList[iii].bgColor			=	backGrndColor;
		cWidgetList[iii].textColor			=	CV_RGB(255,	0,		0);
		cWidgetList[iii].borderColor		=	CV_RGB(255,	255,	255);
		cWidgetList[iii].bgColorSelected	=	CV_RGB(100,	100,	100);
		cWidgetList[iii].includeBorder		=	true;
	}
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTab::~WindowTab(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void WindowTab::CloseWindow(void)
{
Controller	*myControllerObj;

//	CONSOLE_DEBUG(__FUNCTION__);

	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		myControllerObj->cKeepRunning	=	false;
	}
}

//**************************************************************************************
void WindowTab::SetAlpacaDeviceType(const char *deviceTypeString)
{
	strcpy(cAlpacaDeviceTypeStr, deviceTypeString);
}

//**************************************************************************************
void WindowTab::ComputeWidgetColumns(const int windowWitdh)
{
	//*	these are for consistency between window tabs
	cClmWidth		=	windowWitdh / 6;
	cClmWidth		-=	2;
	cBtnWidth		=	windowWitdh / 6;
	cClm1_offset	=	3;
	cClm2_offset	=	1 * cClmWidth;
	cClm3_offset	=	2 * cClmWidth;
	cClm4_offset	=	3 * cClmWidth;
	cClm5_offset	=	4 * cClmWidth;
	cClm6_offset	=	5 * cClmWidth;
	cLrgBtnWidth	=	cClmWidth;
	cLrgBtnHeight	=	cClmWidth / 2;
	cFullWidthBtn	=	windowWitdh - cClm1_offset - 1;
}

//**************************************************************************************
void WindowTab::RunWindowBackgroundTasks(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTab::SetRunFastBackgroundMode(bool newRunFastMode)
{
Controller	*myControllerObj;

//	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_BOOL("newRunFastMode\t=", newRunFastMode);

	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		myControllerObj->SetRunFastBackgroundMode(newRunFastMode);
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
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
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetType(const int widgetIdx, const int widetType)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].widgetType	=	widetType;
		cWidgetList[widgetIdx].needsUpdated	=	true;

		//*	special processing for some of the widget types
		switch(widetType)
		{
			case kWidgetType_RadioButton:
			case kWidgetType_CheckBox:
				cWidgetList[widgetIdx].includeBorder	=	false;
				cWidgetList[widgetIdx].justification	=	kJustification_Left;
				break;

			case kWidgetType_TextInput:
				strcpy(cWidgetList[widgetIdx].textString, "_");
				cWidgetList[widgetIdx].justification	=	kJustification_Left;
				break;
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
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}


//**************************************************************************************
void	WindowTab::SetWidgetFont(const int widgetIdx, const int fontNum)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].fontNum		=	fontNum;
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetJustification(	const int widgetIdx, int justification)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].justification	=	justification;
		cWidgetList[widgetIdx].needsUpdated		=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetText(const int widgetIdx, const char *newText)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		if (strlen(newText) < kMaxWidgetStrLen)
		{
			strcpy(cWidgetList[widgetIdx].textString, newText);
			cWidgetList[widgetIdx].needsUpdated	=	true;
		}
		else
		{
			CONSOLE_DEBUG_W_STR("String to long\t=", newText);
		//	CONSOLE_ABORT(__FUNCTION__);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("widgetIdx out of range\t=", widgetIdx);
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetAltText(const int widgetIdx, const char *newText)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		if (strlen(newText) < kAltTextLen)
		{
			strcpy(cWidgetList[widgetIdx].alternateText, newText);
		}
		else
		{
			CONSOLE_DEBUG_W_STR("String to long\t=", newText);
		//	CONSOLE_ABORT(__FUNCTION__);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("widgetIdx out of range\t=", widgetIdx);
	}
}


//**************************************************************************************
void	WindowTab::SetWidgetTextPtr(const int widgetIdx, char *textPtr)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		//*	it is OK if the argument is NULL
		cWidgetList[widgetIdx].textPtr		=	textPtr;
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("widgetIdx out of range\t=", widgetIdx);
	}
}


//**************************************************************************************
void	WindowTab::GetWidgetText(const int widgetIdx, char *getText)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		strcpy(getText, cWidgetList[widgetIdx].textString);
		if (cWidgetList[widgetIdx].widgetType == kWidgetType_TextInput)
		{
		int	textLen;

			textLen	=	strlen(getText);
			if (textLen > 0)
			{
				if (getText[textLen - 1] == '_')
				{
					getText[textLen - 1]	=	0;
				}
			}
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("widgetIdx out of range\t=", widgetIdx);
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetHelpText(const int widgetIdx, const char *newText)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(newText);
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		if (strlen(newText) < kMaxHelpTextStrLen)
		{
			strcpy(cWidgetList[widgetIdx].helpText, newText);
		}
		else
		{
			strncpy(cWidgetList[widgetIdx].helpText, newText, (kMaxHelpTextStrLen - 2));
			cWidgetList[widgetIdx].helpText[kMaxHelpTextStrLen -1]	=	0;

			CONSOLE_DEBUG_W_LONG("Help text is to long, length=", (long)strlen(newText));
			CONSOLE_DEBUG(newText);
//			CONSOLE_ABORT(__FUNCTION__);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("widgetIdx out of range\t=", widgetIdx);
	}
}

//**************************************************************************************
//*	tab list must end with a zero value
//**************************************************************************************
void	WindowTab::SetWidgetTabStops(const int widgetIdx, const short *tabStopList)
{
int		iii;
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(newText);
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].hasTabs	=	true;
		//*	copy over the tab stop array
		iii	=	0;
		while ((tabStopList[iii] > 0) && (iii < kMaxTabStops))
		{
			cWidgetList[widgetIdx].tabStops[iii]	=	tabStopList[iii];
			iii++;
		}
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
void	WindowTab::SetWidgetNumber(		const int widgetIdx, const unsigned int number)
{
char	lineBuff[64];

	sprintf(lineBuff, "%d", number);
	SetWidgetText(widgetIdx, lineBuff);
}

//**************************************************************************************
void	WindowTab::SetWidgetNumber(const int widgetIdx, const long number)
{
char	lineBuff[64];

	sprintf(lineBuff, "%ld", number);
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
void	WindowTab::SetWidgetNumber(const int widgetIdx, const double number, const int decimalPlaces)
{
char	lineBuff[64];

	switch(decimalPlaces)
	{
		case 0:		sprintf(lineBuff, "%0.0f", number);	break;
		case 1:		sprintf(lineBuff, "%0.1f", number);	break;
		case 2:		sprintf(lineBuff, "%0.2f", number);	break;
		case 3:		sprintf(lineBuff, "%0.3f", number);	break;
		case 4:		sprintf(lineBuff, "%0.4f", number);	break;
		case 5:		sprintf(lineBuff, "%0.5f", number);	break;
		case 6:		sprintf(lineBuff, "%0.6f", number);	break;
		case 7:		sprintf(lineBuff, "%0.7f", number);	break;
		case 8:		sprintf(lineBuff, "%0.8f", number);	break;
		default:	sprintf(lineBuff, "%0.4f", number);	break;
	}
	SetWidgetText(widgetIdx, lineBuff);
}

//**************************************************************************************
void	WindowTab::SetWidgetNumber6F(const int widgetIdx, const double number)
{
char	lineBuff[64];

	sprintf(lineBuff, "%0.6f", number);
	SetWidgetText(widgetIdx, lineBuff);
}

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//**************************************************************************************
void	WindowTab::SetWidgetImage(const int widgetIdx, cv::Mat *argImagePtr)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("widgetIdx  \t=",	widgetIdx);
//	CONSOLE_DEBUG_W_HEX("argImagePtr\t=",	argImagePtr);
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].openCVimagePtr	=	argImagePtr;
		cWidgetList[widgetIdx].widgetType		=	kWidgetType_Image;
		cWidgetList[widgetIdx].needsUpdated		=	true;
	}
	if (argImagePtr == NULL)
	{
//		CONSOLE_DEBUG("argImagePtr is NULL!!!!!!!!!!!!!!!!!!!");
//		CONSOLE_ABORT(__FUNCTION__);
	}
}
#else
//**************************************************************************************
void	WindowTab::SetWidgetImage(const int widgetIdx, IplImage *argImagePtr)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].openCVimagePtr	=	argImagePtr;
		cWidgetList[widgetIdx].widgetType		=	kWidgetType_Image;
		cWidgetList[widgetIdx].needsUpdated		=	true;
	}
}
#endif // _USE_OPENCV_CPP_

//**************************************************************************************
void	WindowTab::SetWidgetTextColor(const int widgetIdx, cv::Scalar newtextColor)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].textColor	=	newtextColor;
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetTextColor(const int widgetIdx, int colorIndex)
{
	if ((colorIndex >= 0) && (colorIndex < W_COLOR_LAST))
	{
		SetWidgetTextColor(widgetIdx, gColorTable[colorIndex]);
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetBGColor(const int widgetIdx, cv::Scalar newBackGroundColor)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].bgColor		=	newBackGroundColor;
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetBGColorSelected(const int widgetIdx, cv::Scalar newBackGroundColor)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].bgColorSelected	=	newBackGroundColor;
		cWidgetList[widgetIdx].needsUpdated		=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetBorderColor(const int widgetIdx, cv::Scalar newtextColor)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].borderColor	=	newtextColor;
		cWidgetList[widgetIdx].needsUpdated	=	true;
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
			CONSOLE_DEBUG("ERROR!!!, bounding box size is messed up");
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
//*	starting at the bottom
//*		Bottom row
//*					ip address box
//*					readall indicator
//*					optional connect button
//*		2nd from bottom
//*					Error msg box
//*		3rd from bottom
//*					Last command
//*		4th from bottom
//*					Alpaca logo
//**************************************************************************************
void	WindowTab::SetupWindowBottomBoxes(	const int	ipaddrBox,
											const int	readAllBox,
											const int	deviceStateBox,
											const int	errorMsgBox,
											const int	lastCmdWidgetIdx,
											const int	logoWidgetIdx,
											const int	helpBtnBox ,
											const bool	logoSideOfScreen,
											const int	connectBtnBox)
{
int		xLoc;
int		yLoc;
int		connBtnWidth;
int		errMsgBoxHeight;
int		logoWidth;
int		logoHeight;
int		indicatorBoxWid;
int		indicatorBoxHgt;

	cIpAddrTextBox	=	ipaddrBox;
	//*	start at the bottom and work our way up
	yLoc	=	cHeight - 1;

	//==========================================================================
	//*	BOTTOM ROW
	//==========================================================================
	//*	IP address
	if (ipaddrBox >= 0)
	{
		//*	check for connect button
		if (connectBtnBox > 0)
		{
			//*	a connect button was specified, adjust the IP box to be smaller
			SetWidget(		ipaddrBox,	0,	(yLoc - cBtnHeight),		(cClmWidth * 4),	cBtnHeight);
			SetWidgetFont(	ipaddrBox,	kFont_Medium);

			//*	now setup the "Connect" button
			connBtnWidth	=	cWidth - cClm5_offset - 2;
			SetWidget(				connectBtnBox,	cClm5_offset + 1,	(yLoc - cBtnHeight),	connBtnWidth,		cBtnHeight);
			SetWidgetType(			connectBtnBox,	kWidgetType_Button);
			SetWidgetFont(			connectBtnBox,	kFont_Medium);
			SetWidgetText(			connectBtnBox,	"Connect");

			SetWidgetBGColor(		connectBtnBox,	CV_RGB(255, 255, 255));
			SetWidgetBorderColor(	connectBtnBox,	CV_RGB(0, 0, 0));
			SetWidgetTextColor(		connectBtnBox,	CV_RGB(0, 0, 0));
		}
		else
		{
			SetWidget(		ipaddrBox,	0,	(yLoc - cBtnHeight),		cWidth,	cBtnHeight);
			SetWidgetFont(	ipaddrBox, kFont_Medium);
		}
	}

	//=======================================================
	//*	this is ON TOP of the IP box on purpose
	//*	this is an indicator, "R" to signify that the alpaca driver supports READALL
	xLoc				=	2;
	indicatorBoxWid		=	cBtnHeight - 10;
	indicatorBoxHgt		=	cBtnHeight - 4;
	if (readAllBox >= 0)
	{
		SetWidget(				readAllBox,	xLoc,	(yLoc - cBtnHeight)+2,		indicatorBoxWid,	indicatorBoxHgt);
		SetWidgetType(			readAllBox,	kWidgetType_TextBox);
		SetWidgetBorderColor(	readAllBox,	CV_RGB(0,	0,	0));
		SetWidgetText(			readAllBox,	"R");
		SetWidgetFont(			readAllBox,	kFont_Medium);
		SetWidgetTextColor(		readAllBox,	CV_RGB(0,255,	0));
		SetWidgetValid(			readAllBox,	false);		//*	will only be enabled if READALL command exists
		SetWidgetBorder(		readAllBox,	false);
	}

	//=======================================================
	//*	this is ON TOP of the IP box on purpose
	//*	this is an indicator, "DS" to signify that the alpaca driver supports DeviceState
	xLoc			+=	indicatorBoxWid;
	indicatorBoxWid	+=	2;
	if (deviceStateBox >= 0)
	{
		SetWidget(				deviceStateBox,	xLoc,	(yLoc - cBtnHeight)+2,		indicatorBoxWid,	indicatorBoxHgt);
		SetWidgetType(			deviceStateBox,	kWidgetType_TextBox);
		SetWidgetBorderColor(	deviceStateBox,	CV_RGB(0,	0,	0));
		SetWidgetText(			deviceStateBox,	"DS");
		SetWidgetFont(			deviceStateBox,	kFont_Medium);
		SetWidgetTextColor(		deviceStateBox,	CV_RGB(0,255,	0));
		SetWidgetValid(			deviceStateBox,	false);		//*	will only be enabled if READALL command exists
		SetWidgetBorder(		deviceStateBox,	false);
	}


	if ((ipaddrBox >= 0) || (readAllBox >= 0))
	{
		yLoc	-=	cBtnHeight;
	}

	//==========================================================================
	//*	2nd ROW from bottom
	//==========================================================================
	if (errorMsgBox > 0)
	{
		//*	for narrow screens we need 2 lines
//		CONSOLE_DEBUG_W_NUM("cWidth\t=",	cWidth);
		if (cWidth < 550)
		{
			errMsgBoxHeight	=	2 * cBtnHeight;
		}
		else
		{
			errMsgBoxHeight	=	cBtnHeight;
		}
		SetWidget(			errorMsgBox,	0,	(yLoc - errMsgBoxHeight),	cWidth,	errMsgBoxHeight);
		SetWidgetType(		errorMsgBox,	kWidgetType_MultiLineText);
		SetWidgetFont(		errorMsgBox,	kFont_Medium);
		SetWidgetTextColor(	errorMsgBox,	CV_RGB(255,	0,	0));
		SetWidgetText(		errorMsgBox,	"Error messages will be displayed here");
		yLoc	-=	errMsgBoxHeight;
	}

	//==========================================================================
	//*	3rd ROW from bottom
	//==========================================================================
	if (lastCmdWidgetIdx >= 0)
	{
		cLastCmdTextBox	=	lastCmdWidgetIdx;
		SetWidget(			lastCmdWidgetIdx,	0,			(yLoc - cBtnHeight),		cWidth,		cBtnHeight);
		SetWidgetType(		lastCmdWidgetIdx,	kWidgetType_TextBox);
		SetWidgetText(		lastCmdWidgetIdx,	"---");
		SetWidgetFont(		lastCmdWidgetIdx,	kFont_Medium);
		SetWidgetTextColor(	lastCmdWidgetIdx,	CV_RGB(0,	255,	0));
		yLoc	-=	cBtnHeight;
	}

	//==========================================================================
	//*	4th ROW from bottom
	//==========================================================================
	if (logoWidgetIdx >= 0)
	{
		//*	now set the Alpaca Logo
		SetWidgetText(logoWidgetIdx,	"logo");	//*	this is for debugging
		LoadAlpacaLogo();
		if (gAlpacaLogoPtr != NULL)
		{
		#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
			logoWidth	=	gAlpacaLogoPtr->cols;
			logoHeight	=	gAlpacaLogoPtr->rows;
		#else
			logoWidth	=	gAlpacaLogoPtr->width;
			logoHeight	=	gAlpacaLogoPtr->height;
		#endif // _USE_OPENCV_CPP_
			if (logoSideOfScreen)
			{
				//*	true means right side of screen
				xLoc		=	cWidth - logoWidth;
			}
			else
			{
				//*	false means left side of screen
				xLoc		=	1;
			}
			yLoc		-=	logoHeight;

			SetWidget(		logoWidgetIdx,	xLoc,	yLoc,	logoWidth,	logoHeight);
			SetWidgetImage(	logoWidgetIdx, gAlpacaLogoPtr);
		}
	}
}

//**************************************************************************************
//*	returns logo height
//**************************************************************************************
int	WindowTab::SetAlpacaLogoBottomCorner(const int logoWidgetIdx)
{
int	logoWidth;
int	logoHeight;
int	xLoc;
int	yLoc;

//	CONSOLE_DEBUG(__FUNCTION__);
	logoHeight	=	0;
	if (logoWidgetIdx >= 0)
	{
		//*	now set the Alpaca Logo
		LoadAlpacaLogo();
		if (gAlpacaLogoPtr != NULL)
		{
		#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
			logoWidth	=	gAlpacaLogoPtr->cols;
			logoHeight	=	gAlpacaLogoPtr->rows;
		#else
			logoWidth	=	gAlpacaLogoPtr->width;
			logoHeight	=	gAlpacaLogoPtr->height;

		#endif // _USE_OPENCV_CPP_
//			CONSOLE_DEBUG_W_NUM("logoWidth \t=", logoWidth);
//			CONSOLE_DEBUG_W_NUM("logoHeight\t=", logoHeight);

			if ((logoWidth > 0) && (logoHeight > 0))
			{
				xLoc		=	cWidth - logoWidth;
				yLoc		=	cHeight - logoHeight;
				SetWidget(		logoWidgetIdx,	xLoc,	yLoc,	logoWidth,	logoHeight);
				SetWidgetImage(	logoWidgetIdx, gAlpacaLogoPtr);
			}
			else
			{
				CONSOLE_DEBUG("Failed to load Alpaca Logo correctly");
//				CONSOLE_ABORT(__FUNCTION__);
			}
		}
		else
		{
			CONSOLE_DEBUG("Failed to load Alpaca Logo");
//			CONSOLE_ABORT(__FUNCTION__);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("logoWidgetIdx is invalid", logoWidgetIdx);
//		CONSOLE_ABORT(__FUNCTION__);
	}
	return(logoHeight);
}

//**************************************************************************************
void	WindowTab::ClearLastAlpacaCommand(void)
{
Controller	*myControllerObj;

//	CONSOLE_DEBUG(__FUNCTION__);

	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		myControllerObj->cLastAlpacaCmdString[0]	=	0;
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
	if (cLastCmdTextBox >= 0)
	{
		SetWidgetText(	cLastCmdTextBox, "");
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
//				CONSOLE_DEBUG_W_NUM("cLastCmdTextBox\t=", cLastCmdTextBox);
//				CONSOLE_DEBUG_W_STR("cLastAlpacaCmdString\t=", textStr);
			}
			else
			{
//				CONSOLE_DEBUG("Nothing to show");
			}
		}
		else
		{
//			CONSOLE_DEBUG("myControllerObj is NULL");
		}
	}
	else
	{
		CONSOLE_DEBUG("cLastCmdTextBox not set");
//		CONSOLE_ABORT(__FUNCTION__);
	}
}


//**************************************************************************************
void	WindowTab::SetWidgetValid(const int widgetIdx, bool valid)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, widgetIdx);

	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].valid		=	valid;
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetBorder(const int widgetIdx, bool onOff)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].includeBorder	=	onOff;
		cWidgetList[widgetIdx].needsUpdated		=	true;
	}
}


//**************************************************************************************
void	WindowTab::SetWidgetChecked(	const int widgetIdx, bool checked)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].selected	=	checked;
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetCrossedout(const int widgetIdx, bool crossedout)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].crossedOut	=	crossedout;
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetHighlighted(	const int widgetIdx, bool highLighted)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].highLighted	=	highLighted;
		cWidgetList[widgetIdx].needsUpdated	=	true;
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
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetScrollBarLimits(const int widgetIdx, int scrollBarLines, int scrollBarMax)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].scrollBarLines	=	scrollBarLines;
		cWidgetList[widgetIdx].scrollBarMax		=	scrollBarMax;
		cWidgetList[widgetIdx].needsUpdated		=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetScrollBarValue(const int widgetIdx, int scrollBarValue)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].scrollBarValue	=	scrollBarValue;
		cWidgetList[widgetIdx].needsUpdated		=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetSliderValue(const int widgetIdx, double sliderValue)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("Widget index \t=", widgetIdx);
//	CONSOLE_DEBUG_W_DBL("new value \t=", sliderValue);
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].sliderValue	=	sliderValue;
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Widget index out of range", widgetIdx);
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
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}

//*****************************************************************************
void	WindowTab::SetWidgetSensorValue(const int widgetNum, TYPE_InstSensor *sensorData, const int decimalPlaces)
{

	if (sensorData->ValidData)
	{
		SetWidgetNumber(	widgetNum,	sensorData->Value, decimalPlaces);
		SetWidgetTextColor(	widgetNum, CV_RGB(0x00, 0xff, 0x00));
	}
	else
	{
		SetWidgetText(	widgetNum,	"-N/A-");
		SetWidgetTextColor(	widgetNum, CV_RGB(0xff, 0x00, 0x00));
	}
	ForceWindowUpdate();
}


//**************************************************************************************
void	WindowTab::SetCurrentTab(const int tabIdx)
{
Controller	*myControllerObj;

//	CONSOLE_DEBUG(__FUNCTION__);

	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		myControllerObj->SetCurrentTab(tabIdx);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("cParentObjPtr is NULL, windowname=", cWindowName);
	}
}

//**************************************************************************************
void	WindowTab::SetupWindowControls(void)
{

}

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//**************************************************************************************
void	WindowTab::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, TYPE_WIDGET *theWidget, const int widgetIdx)
{
	//*	this routine should be overloaded
	CONSOLE_DEBUG("This routine MUST be overloaded in the windowtab subclass");
	CONSOLE_ABORT(__FUNCTION__);
}

//**************************************************************************************
void	WindowTab::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		DrawWidgetCustomGraphic(openCV_Image, &cWidgetList[widgetIdx], widgetIdx);
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
}
#else
//**************************************************************************************
void	WindowTab::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
{
	//*	this routine should be overloaded
	CONSOLE_DEBUG("This routine MUST be overloaded in the windowtab subclass");
	CONSOLE_ABORT(__FUNCTION__);
}
#endif // _USE_OPENCV_CPP_

//**************************************************************************************
void	WindowTab::ForceWindowUpdate(void)
{
Controller	*myControllerObj;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		myControllerObj->cUpdateWindow	=	true;
	}
	else
	{
		CONSOLE_DEBUG_W_STR("cParentObjPtr is NULL, windowname=", cWindowName);
	}
}

//**************************************************************************************
void	WindowTab::UpdateWindowAsNeeded(void)
{
Controller	*myControllerObj;

	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		myControllerObj->UpdateWindowAsNeeded();
	}
	else
	{
		CONSOLE_DEBUG_W_STR("cParentObjPtr is NULL, windowname=", cWindowName);
	}
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
bool	WindowTab::IsWidgetButton(const int widgetIdx)
{
bool	widgetIsButton;

	widgetIsButton	=	false;
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		switch (cWidgetList[widgetIdx].widgetType)
		{
			case kWidgetType_Button:
			case kWidgetType_CheckBox:
			case kWidgetType_Icon:
			case kWidgetType_RadioButton:
				widgetIsButton	=	true;
				break;

			case kWidgetType_Graph:
			case kWidgetType_CustomGraphic:
			case kWidgetType_Image:
			case kWidgetType_MultiLineText:
			case kWidgetType_OutlineBox:
			case kWidgetType_ProessBar:
			case kWidgetType_ScrollBar:
			case kWidgetType_Slider:
			case kWidgetType_TextBox:
			case kWidgetType_TextBox_MonoSpace:
			case kWidgetType_TextInput:
			default:
				widgetIsButton	=	false;
				break;

		}
	}
	return(widgetIsButton);
}


//*****************************************************************************
bool	WindowTab::IsWidgetTextInput(const int widgetIdx)
{
bool	widgetIsTextInput;

//	CONSOLE_DEBUG(__FUNCTION__);

	widgetIsTextInput	=	false;
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		if (cWidgetList[widgetIdx].widgetType == kWidgetType_TextInput)
		{
			widgetIsTextInput	=	true;
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Widget index out of bounds=", widgetIdx);
	}
	return(widgetIsTextInput);
}

//*****************************************************************************
void	WindowTab::HandleKeyDown(const int keyPressed)
{
	//*	this routine should be overloaded
//	CONSOLE_DEBUG_W_HEX("this routine should be overloaded: keyPressed=", keyPressed);
}

//*****************************************************************************
void	WindowTab::HandleSpecialKeys(const int keyPressed)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	switch(keyPressed)
	{
		case 0x10FF50:	//*	home key
			cFirstLineIdx	=	0;
			break;

		case 0x10FF56:	//*	page down
			cFirstLineIdx	+=	10;
			break;

		case 0x10FF55:	//*	page up
			cFirstLineIdx	-=	10;
			break;

		case 0x10FF57:	//*	end
			break;

		default:
			HandleKeyDown(keyPressed);
			break;
	}
	if (cFirstLineIdx < 0)
	{
		cFirstLineIdx	=	0;
	}
	UpdateOnScreenWidgetList();
	ForceWindowUpdate();
}

//*****************************************************************************
void	WindowTab::ProcessButtonClick(const int buttonIdx, const int flags)
{
//	CONSOLE_DEBUG_W_NUM("this routine should be overloaded: buttonIdx=", buttonIdx);
	//*	this routine should be overloaded
}

//*****************************************************************************
void	ProcessDoubleClick(const int buttonIdx)
{
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
//*	this routine can be overloaded
void	WindowTab::ProcessDoubleClick(	const int	widgetIdx,
										const int	event,
										const int	xxx,
										const int	yyy,
										const int	flags)
{
	CONSOLE_DEBUG("this routine should be overloaded");
//	CONSOLE_ABORT(__FUNCTION__);
	//*	this routine should be overloaded
}

//*****************************************************************************
void	WindowTab::ProcessDoubleClick_RtBtn(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
//	CONSOLE_DEBUG("this routine should be overloaded");
	//*	this routine should be overloaded
}

//*****************************************************************************
//*	this routine can be overloaded
//*****************************************************************************
void	WindowTab::ProcessMouseEvent(	const int	widgetIdx,
										const int	event,
										const int	xxx,
										const int	yyy,
										const int	flags)
{

//	CONSOLE_DEBUG(__FUNCTION__);
	switch(event)
	{
		case cv::EVENT_MOUSEMOVE:
			break;

		case cv::EVENT_LBUTTONDOWN:
//			CONSOLE_DEBUG_W_NUM("EVENT_LBUTTONDOWN", widgetIdx);
			cLeftButtonDown	=	true;
			break;

		case cv::EVENT_LBUTTONUP:
//			CONSOLE_DEBUG_W_NUM("EVENT_LBUTTONUP", widgetIdx);
			cLeftButtonDown	=	false;
			ProcessLineSelect(widgetIdx);
			break;

//
//		case cv::EVENT_RBUTTONDOWN:
//			cRightButtonDown		=	true;
//			break;
//
//		case cv::EVENT_MBUTTONDOWN:
//			break;
//
//		case cv::EVENT_RBUTTONUP:
//			cRightButtonDown		=	false;
//			break;
//
//		case cv::EVENT_MBUTTONUP:
//			break;
//
//		case cv::EVENT_LBUTTONDBLCLK:
//			break;
//
//		case cv::EVENT_RBUTTONDBLCLK:
//			break;
//
//		case cv::EVENT_MBUTTONDBLCLK:
//			break;
//
#if (CV_MAJOR_VERSION >= 3)
		case cv::EVENT_MOUSEWHEEL:
//			CONSOLE_DEBUG("EVENT_MOUSEWHEEL");
			break;
		case cv::EVENT_MOUSEHWHEEL:
//			CONSOLE_DEBUG("EVENT_MOUSEHWHEEL");
			break;
#endif
		default:
//			CONSOLE_DEBUG_W_NUM("UNKNOWN EVENT", event);
			break;
	}
}

//*****************************************************************************
void	WindowTab::ProcessMouseLeftButtonDown(const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, xxx);
	//*	this routine can be overloaded
}

//*****************************************************************************
void	WindowTab::ProcessMouseLeftButtonUp(const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, xxx);
	//*	this routine can be overloaded
}

//**************************************************************************************
//*	gets called when the window tab changes
//**************************************************************************************
void	WindowTab::ActivateWindow(void)
{
	//*	dont do anything, this is supposed to be over-ridden if needed
}

//*****************************************************************************
//*	this routine SHOULD be overloaded
void	WindowTab::UpdateControls(void)
{
}

//*****************************************************************************
//*	this routine SHOULD be overloaded
void	WindowTab::UpdateSliderValue(const int	widgetIdx, double newSliderValue)
{
//	CONSOLE_DEBUG_W_DBL(__FUNCTION__, newSliderValue);
	SetWidgetSliderValue(widgetIdx, newSliderValue);
}

//*****************************************************************************
//*	handle dragging of the mouse in a slider widget
//*	this routine can be overloaded
//*****************************************************************************
void	WindowTab::ProcessMouseLeftButtonDragged(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)
{
bool	sliderIsHorizontal;
int		minimumPixelValue;
int		maximumPixelValue;
int		pixelRange;
int		myPixelOffset;
double	mySliderValuePercent;
double	sliderRange;
double	newSliderValue;

	if (cWidgetList[widgetIdx].widgetType == kWidgetType_Slider)
	{
//		CONSOLE_DEBUG("WE have a slider -------------------------------------------------");
//		CONSOLE_DEBUG_W_NUM("xxx  \t=", xxx);
		if (cWidgetList[widgetIdx].width > cWidgetList[widgetIdx].height)
		{
			sliderIsHorizontal	=	true;
		}
		else
		{
			sliderIsHorizontal	=	false;
		}

		if (sliderIsHorizontal)
		{
			//*	figure out min and max and the current relative position
			minimumPixelValue	=	cWidgetList[widgetIdx].left + 50;
			maximumPixelValue	=	cWidgetList[widgetIdx].left +  cWidgetList[widgetIdx].width - 50;
			pixelRange			=	maximumPixelValue - minimumPixelValue;

			myPixelOffset		=	xxx - minimumPixelValue;
//			CONSOLE_DEBUG_W_NUM("myPixelOffset    \t=", myPixelOffset);
			if (myPixelOffset < 0)
			{
				myPixelOffset	=	0;
			}
			else if (myPixelOffset > pixelRange)
			{
				myPixelOffset	=	pixelRange;
			}
			mySliderValuePercent	=	(1.0 * myPixelOffset) / (1.0 * pixelRange);
//			CONSOLE_DEBUG_W_DBL("sliderMin           \t=",	cWidgetList[widgetIdx].sliderMin);
//			CONSOLE_DEBUG_W_DBL("sliderMax           \t=",	cWidgetList[widgetIdx].sliderMax);
//			CONSOLE_DEBUG_W_NUM("minimumPixelValue   \t=",	minimumPixelValue);
//			CONSOLE_DEBUG_W_NUM("maximumPixelValue   \t=",	maximumPixelValue);
//			CONSOLE_DEBUG_W_NUM("myPixelOffset       \t=",	myPixelOffset);
//			CONSOLE_DEBUG_W_NUM("pixelRange          \t=",	pixelRange);
//			CONSOLE_DEBUG_W_DBL("mySliderValuePercent\t=", mySliderValuePercent);

			//*	now translate this to a range within the sliders min/max
			sliderRange		=	cWidgetList[widgetIdx].sliderMax - cWidgetList[widgetIdx].sliderMin;
			newSliderValue	=	cWidgetList[widgetIdx].sliderMin;		//*	minimum value
			newSliderValue	+=	(mySliderValuePercent * sliderRange);	//*	plus slider value
//			CONSOLE_DEBUG_W_DBL("sliderRange      \t=", sliderRange);
//			CONSOLE_DEBUG_W_DBL("newSliderValue   \t=", newSliderValue);
			UpdateSliderValue(widgetIdx, newSliderValue);
		}
		else
		{
			//*	vertical slider not finished
			CONSOLE_DEBUG("Vertical slider not finished");
		}
	}
	else
	{
	//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, xxx);
	}
}

//*****************************************************************************
void	WindowTab::ProcessMouseWheelMoved(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	wheelMovement,
											const int	flags)
{
	//*	this is a virtual function and should be overridden if you need to do anything with it.
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, wheelMovement);
//	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	WindowTab::ProcessLineSelect(int widgetIdx)
{
	//*	this is a virtual function and should be overridden if you need to do anything with it.
//	CONSOLE_ABORT(__FUNCTION__);
}



//*****************************************************************************
void	WindowTab::UpdateOnScreenWidgetList(void)
{
	//*	only needs to be overloaded if the window tab has a list to update
}


//*****************************************************************************
void	WindowTab::LaunchWebURL(const char *urlString)
{
char	webCommandString[128];

	strcpy(webCommandString, gWebBrowserCmdString);
	strcat(webCommandString, " ");
	strcat(webCommandString, urlString);
//	CONSOLE_DEBUG(webCommandString);
	RunCommandLine(webCommandString);
}

//*****************************************************************************
void	WindowTab::LaunchWebRemoteDevice(const char *urlString)
{
char		httpString[128];
char		ipAddrStr[64];
Controller	*myControllerPtr;

	myControllerPtr	=   (Controller *)cParentObjPtr;
	if (myControllerPtr != NULL)
	{
		inet_ntop(AF_INET, &(myControllerPtr->cDeviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
//		sprintf(httpString, "%s http://%s:%d/&s &",
//								gWebBrowserCmdString,
//								ipAddrStr,
//								myControllerPtr->cPort,
//								urlString);
		sprintf(httpString, "%s http://%s:%d/%s &",
								gWebBrowserCmdString,
								ipAddrStr,
								myControllerPtr->cPort,
								urlString);
		CONSOLE_DEBUG(httpString);
//		CONSOLE_DEBUG_W_NUM("cAlpacaDevNum\t=", myControllerPtr->cAlpacaDevNum);
//		CONSOLE_ABORT(__FUNCTION__);
		RunCommandLine(httpString);
	}
	else
	{
		CONSOLE_DEBUG("myControllerPtr is NULL!!!!");
	}
}

//*****************************************************************************
void	WindowTab::SetWebHelpURLstring(const char *webpagestring)
{
	strcpy(cWebURLstring, webpagestring);		//*	set the web help url string
}

//*****************************************************************************
void	WindowTab::LaunchWebHelp(const char *webpagestring)
{
char	urlString[128];

	strcpy(urlString, gWebBrowserCmdString);
	strcat(urlString, " ");
	strcat(urlString, "docs/");
	if (webpagestring != NULL)
	{
		strcat(urlString, webpagestring);
	}
	else if (strlen(cWebURLstring) > 0)
	{
		strcat(urlString, cWebURLstring);
	}
	else
	{
		strcat(urlString, "index.html");
	}
//	CONSOLE_DEBUG(urlString);
	RunCommandLine(urlString);
}


//*****************************************************************************
int	WindowTab::SetTitleBox(	const int	titleWidgetIdx,
							const int	connectionWidgetIdx,
							const int	yLoc,
							const char *titleString)
{
	SetWidget(		titleWidgetIdx,	1,			yLoc,		cWidth-2,		cTitleHeight);
	SetWidgetText(	titleWidgetIdx,	titleString);
	SetWidgetFont(	titleWidgetIdx,	kFont_Medium);
	SetBGcolorFromWindowName(titleWidgetIdx);

//	SetWidgetBGColor(	titleWidgetIdx,	CV_RGB(128, 128, 128));

	//*	was a connection widget index specified?
	if (connectionWidgetIdx >= 0)
	{
		SetUpConnectedIndicator(connectionWidgetIdx, yLoc);
	}
	return(yLoc + cTitleHeight + 2);
}


//*****************************************************************************
void	WindowTab::SetHelpTextBoxNumber(const int buttonIdx)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cHelpTextBoxNumber	=	buttonIdx;
}


//*****************************************************************************
void	WindowTab::SetUpConnectedIndicator(const int buttonIdx, const int yLoc)
{
	cConnectedStateBoxNumber	=	buttonIdx;
	SetWidget(buttonIdx,		5,	(yLoc + 2),	(cTitleHeight - 4),	(cTitleHeight - 4));
	SetWidgetText(				buttonIdx,	"?");
	SetWidgetTextColor(			buttonIdx,	CV_RGB(255,	255,	0));
	SetWidgetFont(				buttonIdx,	kFont_Medium);
	SetBGcolorFromWindowName(	buttonIdx);
	SetWidgetBorder(			buttonIdx,	false);
}

//*****************************************************************************
//*	this is so that the box can be used for error messages that might set to red
//*****************************************************************************
void	WindowTab::SetHelpTextBoxColor(cv::Scalar newtextColor)
{
	cHelpTextBoxColor	=	newtextColor;
}

//*****************************************************************************
//*	returns true if update occurred
bool	WindowTab::DisplayButtonHelpText(const int buttonIdx)
{
bool	updateOccured;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cHelpTextBoxNumber\t=", cHelpTextBoxNumber);
	updateOccured	=	false;
	if (cHelpTextBoxNumber >= 0)
	{
		if (strlen(cWidgetList[buttonIdx].helpText) > 0)
		{
			//*	dont update the text if it has already been updated
			if (buttonIdx != cPrevDisplayedHelpBox)
			{
				SetWidgetTextColor(	cHelpTextBoxNumber, cHelpTextBoxColor);
				SetWidgetText(		cHelpTextBoxNumber, cWidgetList[buttonIdx].helpText);
				updateOccured			=	true;
				//*	keep track of which button we did
				cPrevDisplayedHelpBox	=	buttonIdx;
			}
		}
	}
	return(updateOccured);
}


//*****************************************************************************
void	WindowTab::SetParentObjectPtr(void *argParentObjPtr)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	cParentObjPtr	=	argParentObjPtr;
//	CONSOLE_DEBUG_W_HEX("cParentObjPtr\t=", cParentObjPtr);
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
	//	CONSOLE_DEBUG("cIpAddrTextBox not set!!!!!");
	}
}

//*****************************************************************************
void	WindowTab::DumpWidget(TYPE_WIDGET *theWidget)
{
	printf("%s\t",	(theWidget->valid ? "T" : "F"));

	switch(theWidget->widgetType)
	{
		case kWidgetType_Default:		printf("Def\t");	break;

		case kWidgetType_Button:		printf("BTN\t");	break;
		case kWidgetType_CheckBox:		printf("CBX\t");	break;
		case kWidgetType_Graph:			printf("GRP\t");	break;
		case kWidgetType_CustomGraphic:	printf("CG\t");		break;
		case kWidgetType_Custom:		printf("CUS\t");	break;
		case kWidgetType_Icon:			printf("ICN\t");	break;
		case kWidgetType_Image:			printf("IMG\t");	break;
		case kWidgetType_MultiLineText:	printf("MLT\t");	break;
		case kWidgetType_OutlineBox:	printf("OLB\t");	break;
		case kWidgetType_RadioButton:	printf("RDB\t");	break;
		case kWidgetType_ProessBar:		printf("PB\t");		break;
		case kWidgetType_ScrollBar:		printf("SB\t");		break;
		case kWidgetType_Slider:		printf("SLD\t");	break;
		case kWidgetType_TextBox:		printf("TXB\t");	break;
		case kWidgetType_TextInput:		printf("TXI\t");	break;

		case kWidgetType_Disabled:		printf("DIS\t");	break;

		case kWidgetType_Last:
		default:	printf("%d\t", theWidget->widgetType);
		break;
	}
	printf("%d\t%d\t%d\t%d\t",
						theWidget->left,
						theWidget->top,
						theWidget->width,
						theWidget->height
						);

	printf("%s\t",	(theWidget->hasTabs ? "T" : "F"));
	printf("%s\r\n", theWidget->textString);
}


//*****************************************************************************
void	WindowTab::DumpWidgetList(const int startIdx, const int stopIdx, const char *callingFunctionName)
{
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);
	if (callingFunctionName != NULL)
	{
		printf("Called from %s\r\n", callingFunctionName);
	}
	printf("idx\t");
	printf("val\t");
	printf("typ\t");
	printf("lft\t");
	printf("top\t");
	printf("wid\t");
	printf("hgt\t");
	printf("tab\t");
	printf("txt\t");
	printf("\r\n");
	printf("------------------------------------------------------------------------------------------------\r\n");
	for (iii=startIdx; ((iii<=stopIdx) && (iii < kMaxWidgets)); iii++)
	{
		printf("%2d\t", iii);
		DumpWidget(&cWidgetList[iii]);
	}
}

//*****************************************************************************
void	WindowTab::ClearWidgetSelect(void)
{
int		iii;

	for (iii=0; iii<kMaxWidgets; iii++)
	{
		cWidgetList[iii].lineSelected	=	false;
	}
}

//*****************************************************************************
void	WindowTab::SetWidgetLineSelect(const int widgetIdx, const bool newState)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].lineSelected	=	newState;
	}
}

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
			case	kWidgetType_TextBox:
			case	kWidgetType_TextBox_MonoSpace:
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

			case	kWidgetType_Graph:
			case	kWidgetType_CustomGraphic:
			case	kWidgetType_Icon:
			case	kWidgetType_Image:
			default:
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

#define	TRANSLATE_Y(rect, ddd)	((rect->y + rect->height - 4) - ddd)


//**************************************************************************************
void	WindowTab::DrawGraph(	TYPE_WIDGET	*theWidget,
								const int	numEntries,
								double		*graphArray,
								bool		drawCurrentTimeMarker,
								const int	stepX)
{
cv::Rect		myCVrect;
int				jjj;
int				previousX;
int				pt1_X;
int				pt1_Y;
int				pt2_X;
int				pt2_Y;
int				currentYvalue;
int				preivousYvalue;
//double			avgTotal;
//int				qqq;

//	CONSOLE_DEBUG(__FUNCTION__);

	if ((graphArray != NULL) && (numEntries > 0) && (numEntries < 3000))
	{
		myCVrect.x		=	theWidget->left;
		myCVrect.y		=	theWidget->top;
		myCVrect.width	=	theWidget->width;
		myCVrect.height	=	theWidget->height;

		//=========================================================
		//*	draw tick mark lines
	//	yLoc	=	50;
	//	while (yLoc < 325)
	//	{
	//		DrawTickLine(&myCVrect, yLoc);
	//		yLoc	+=	50;
	//	}
	//	//*	draw a special one at 30
	//	DrawTickLine(&myCVrect, 30);

	//	CONSOLE_DEBUG_W_NUM("numEntries\t=", numEntries);
		previousX		=	theWidget->left;
		preivousYvalue	=	graphArray[0];
		for (jjj=0; jjj<numEntries; jjj += stepX)
		{
			if ((stepX == 1) || (jjj < stepX))
			{
				currentYvalue	=	graphArray[jjj];
			}
			else
			{
				currentYvalue	=	graphArray[jjj];
				if (graphArray[jjj-1] > currentYvalue)
				{
					currentYvalue	=	graphArray[jjj-1];
				}
	//			avgTotal	=	0.0;
	//			for (qqq=0; qqq<stepX; qqq++)
	//			{
	//				avgTotal	+=	graphArray[jjj - qqq];
	//			}
	//			currentYvalue	=	avgTotal / stepX;
			}
			//*	compute the x,y points for the line
			pt1_X			=	previousX;
			pt1_Y			=	TRANSLATE_Y((&myCVrect), preivousYvalue);
			pt2_X			=	previousX + 1;
			pt2_Y			=	TRANSLATE_Y((&myCVrect), currentYvalue);
			LLG_MoveTo(pt1_X, pt1_Y);
			LLG_LineTo(pt2_X, pt2_Y);

			previousX		=	pt2_X;
			preivousYvalue	=	currentYvalue;
		}
	}
	else
	{
		CONSOLE_DEBUG("Something is wrong with input parameters");
//		CONSOLE_DEBUG_W_HEX("graphArray\t=",	graphArray);
		CONSOLE_DEBUG_W_NUM("numEntries\t=",	numEntries);
	}

	//=========================================================
	//*	now draw a vertical line for the CURRENT time
	if (drawCurrentTimeMarker)
	{
	int	minutesSinceMidnight;
	int	xValue;

		minutesSinceMidnight	=	GetMinutesSinceMidnight();
		xValue					=	minutesSinceMidnight / stepX;
		pt1_X					=	theWidget->left + xValue;
		pt1_Y					=	theWidget->top - 1;
		pt2_X					=	pt1_X;
		pt2_Y					=	(theWidget->top + theWidget->height) - 1;
		LLG_SetColor(W_RED);
		LLG_MoveTo(pt1_X, pt1_Y);
		LLG_LineTo(pt2_X, pt2_Y);
	}
}

//**************************************************************************************
void	WindowTab::DrawHistogram(	TYPE_WIDGET		*theWidget,
									const int32_t	*graphArray,
									const int		numEntries,
									const int		yDivideFactor,
									cv::Scalar		lineColor)
{
cv::Rect		myCVrect;
int				jjj;
int				xOffset;
int				pt1_X;
int				pt1_Y;
int				pt2_X;
int				pt2_Y;
int				currArrayValue;
double			adjustedArrayValueDbl;
int				preivousYvalue;
double			logOfPixlCntDbl;
//int				logOfPixlCntInt;
int				adjustedPixlCntLog;
double			logOf255;
int				graphHeight;
int				graphBottom;

//	CONSOLE_DEBUG(__FUNCTION__);

	if ((graphArray != NULL) && (numEntries > 0) && (numEntries < 3000))
	{
		myCVrect.x		=	theWidget->left;
		myCVrect.y		=	theWidget->top;
		myCVrect.width	=	theWidget->width;
		myCVrect.height	=	theWidget->height;
		graphHeight		=	theWidget->height;
		graphBottom		=	theWidget->top + theWidget->height;

		if (numEntries < theWidget->width)
		{
			xOffset	=	(theWidget->width - numEntries) / 2;
		}
		else
		{
			xOffset	=	0;
		}

		logOf255		=	log(255.0);
//
//			for (iii=0; iii<256; iii++)
//			{
//				histYvalue	=	theArray[iii] / yDivideFactor;
//				if (histYvalue > 0)
//				{
//					logOfPixlCntDbl	=	log(histYvalue);
//					histYvalue		=	(logOfPixlCnt * graphHeight) / logOf255;
//					point1.x		=	xStart + iii;
//					point1.y		=	windowHeight;
//
//					point2.x		=	xStart + iii;
//					point2.y		=	windowHeight - histYvalue;

#ifndef __arm__
//		CONSOLE_DEBUG_W_NUM("xOffset      \t=", xOffset);
//		CONSOLE_DEBUG_W_NUM("graphHeight  \t=", graphHeight);
//		CONSOLE_DEBUG_W_NUM("yDivideFactor\t=", yDivideFactor);
//		CONSOLE_DEBUG_W_DBL("logOf255     \t=",	logOf255);
#endif
		LLG_SetColor(lineColor);
		preivousYvalue	=	0;

		LLG_PenSize(cHistogramPenSize);
		for (jjj=0; jjj<numEntries; jjj++)
		{
			currArrayValue			=	graphArray[jjj];
			adjustedArrayValueDbl	=	(1.0 * currArrayValue) / yDivideFactor;
			if (adjustedArrayValueDbl >= 0.0)
			{
				logOfPixlCntDbl		=	log(adjustedArrayValueDbl);
//				logOfPixlCntInt		=	logOfPixlCntDbl;
				adjustedPixlCntLog	=	(logOfPixlCntDbl * graphHeight) / logOf255;
//			#ifndef __arm__
//				if ((jjj < 20) || (adjustedPixlCntLog < 0))
//				{
//					CONSOLE_DEBUG_W_NUM("jjj                  \t=",	jjj);
//					CONSOLE_DEBUG_W_NUM("currArrayValue       \t=",	currArrayValue);
//					CONSOLE_DEBUG_W_DBL("adjustedArrayValueDbl\t=",	adjustedArrayValueDbl);
//					CONSOLE_DEBUG_W_DBL("logOfPixlCntDbl      \t=",	logOfPixlCntDbl);
//					CONSOLE_DEBUG_W_NUM("logOfPixlCntInt      \t=",	logOfPixlCntInt);
//					CONSOLE_DEBUG_W_NUM("adjustedPixlCntLog   \t=",	adjustedPixlCntLog);
//				}
//			#endif // __arm__

				if (adjustedPixlCntLog >= 0)
				{
					//*	compute the x,y points for the line
					pt1_X			=	theWidget->left + xOffset + jjj;
					pt1_Y			=	graphBottom - preivousYvalue;
					pt2_X			=	pt1_X + 1;
					pt2_Y			=	graphBottom - adjustedPixlCntLog;
					LLG_MoveTo(pt1_X, pt1_Y);
					LLG_LineTo(pt2_X, pt2_Y);
					preivousYvalue	=	adjustedPixlCntLog;
				}
				else
				{
					preivousYvalue	=	0;
				}
			}
			else if (adjustedArrayValueDbl < 0.0)
			{
				CONSOLE_DEBUG_W_DBL("adjustedArrayValueDbl is NEGATIVE\t=", adjustedArrayValueDbl);
			}
		}
		LLG_PenSize(1);
	}
	else
	{
		CONSOLE_DEBUG("Something is wrong with input parameters");
//		CONSOLE_DEBUG_W_HEX("graphArray\t=",	graphArray);
		CONSOLE_DEBUG_W_NUM("numEntries\t=",	numEntries);
	}
}

//*****************************************************************************
void	WindowTab::LLG_MoveTo(const int xx, const int yy)
{
	cCurrentXloc	=	xx;
	cCurrentYloc	=	yy;
}

//*****************************************************************************
void	WindowTab::LLG_LineTo(const int xx, const int yy)
{
//	CONSOLE_DEBUG(__FUNCTION__);

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	if (cOpenCV_Image != NULL)
	{
	cv::Point		pt1;
	cv::Point		pt2;

		pt1.x	=	cCurrentXloc;
		pt1.y	=	cCurrentYloc;

		pt2.x	=	xx;
		pt2.y	=	yy;
		cv::line(	*cOpenCV_Image,
					pt1,
					pt2,
					cCurrentColor,		//	cv::Scalar color,
					cCurrentLineWidth,	//	int thickness CV_DEFAULT(1),
					8,					//	int line_type CV_DEFAULT(8),
					0);					//	int shift CV_DEFAULT(0));

		cCurrentXloc	=	xx;
		cCurrentYloc	=	yy;
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
	}

#else
	if (cOpenCV_Image != NULL)
	{
	CvPoint		pt1;
	CvPoint		pt2;
		pt1.x	=	cCurrentXloc;
		pt1.y	=	cCurrentYloc;

		pt2.x	=	xx;
		pt2.y	=	yy;
		cvLine(	cOpenCV_Image,
				pt1,
				pt2,
				cCurrentColor,		//	cv::Scalar color,
				cCurrentLineWidth,	//	int thickness CV_DEFAULT(1),
				8,					//	int line_type CV_DEFAULT(8),
				0);					//	int shift CV_DEFAULT(0));

		cCurrentXloc	=	xx;
		cCurrentYloc	=	yy;
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
	}
#endif // _USE_OPENCV_CPP_
}

//*****************************************************************************
void	WindowTab::LLG_DrawCString(	const int	xx,
									const int	yy,
									const char	*theString,
									const int	fontIndex)
{
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	if (cOpenCV_Image != NULL)
	{
	cv::Point		textLoc;
	int				curFontNum;
		curFontNum	=	1;
		if ((curFontNum >= 0) && (curFontNum < kFont_last))
		{
			curFontNum	=	fontIndex;
		}

		textLoc.x	=	xx;
		textLoc.y	=	yy;
		cv::putText(	*cOpenCV_Image,
						theString,
						textLoc,
						gFontInfo[curFontNum].fontID,
						gFontInfo[curFontNum].scale,
						cCurrentColor,
						gFontInfo[curFontNum].thickness
						);

	}
	else
	{
		CONSOLE_ABORT("cOpenCV_Image is NULL");
	}
#else
//	CONSOLE_DEBUG(theString);
	if (cOpenCV_Image != NULL)
	{
	CvPoint		textLoc;
	int			myFontIdx;
		myFontIdx	=	1;
		if ((fontIndex >= 0) && (fontIndex < kFont_last))
		{
			myFontIdx	=	fontIndex;
		}
		else
		{
			myFontIdx	=	1;
		}
		textLoc.x	=	xx;
		textLoc.y	=	yy;
		cvPutText(	cOpenCV_Image,
					theString,
					textLoc,
					&gTextFont[myFontIdx],
					cCurrentColor
				);

	}
	else
	{
		CONSOLE_ABORT("cOpenCV_Image is NULL");
	}
#endif // _USE_OPENCV_CPP_
}

//*****************************************************************************
int	WindowTab::LLG_GetTextSize(const char *textString, const int fontIndex)
{
int	textWidthPixels;

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
cv::Size		textSize;
	textSize	=	cv::getTextSize(textString,
									gFontInfo[fontIndex].fontID,
									gFontInfo[fontIndex].scale,
									gFontInfo[fontIndex].thickness,
									&cCurrentFontBaseLine);
	textWidthPixels		=	textSize.width;
	cCurrentFontHeight	=	textSize.height;
#else
CvSize		textSize;
	cvGetTextSize(	textString,
					&gTextFont[fontIndex],
					&textSize,
					&cCurrentFontBaseLine);
	textWidthPixels		=	textSize.width;
	cCurrentFontHeight	=	textSize.height;
#endif
	return(textWidthPixels);
}

//*****************************************************************************
void	WindowTab::LLG_PenSize(const int newLineWidth)
{
	if ((newLineWidth >= 0) && (newLineWidth < 10))
	{
		cCurrentLineWidth	=	newLineWidth;
	}
	else
	{
		cCurrentLineWidth	=	1;
	}
}


//*****************************************************************************
cv::Scalar	WindowTab::LLG_GetColor(const int theColor)
{
cv::Scalar	myColorScaler;

	if ((theColor >= 0) && (theColor < W_COLOR_LAST))
	{
		myColorScaler	=	gColorTable[theColor];
	}
	else
	{
		myColorScaler	=	CV_RGB(255,	255,	255);
	}
	return(myColorScaler);
}


//*****************************************************************************
void	WindowTab::LLG_SetColor(const int theColor)
{
	if ((theColor >= 0) && (theColor < W_COLOR_LAST))
	{
		cCurrentColor	=	gColorTable[theColor];
	}
	else
	{
		cCurrentColor	=	CV_RGB(255,	255,	255);
	}
}

//*****************************************************************************
void	WindowTab::LLG_SetColor(cv::Scalar newColor)
{
	cCurrentColor	=	newColor;
}

//*****************************************************************************
void	WindowTab::LLG_Putpixel(const int xx, const int yy, const int theColor)
{
//	CONSOLE_DEBUG(__FUNCTION__);

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	if (cOpenCV_Image != NULL)
	{
	cv::Point		pt1;
	cv::Point		pt2;

		pt1.x	=	xx;
		pt1.y	=	yy;

		pt2.x	=	xx;
		pt2.y	=	yy;
		if ((theColor >= 0) && (theColor < W_COLOR_LAST))
		{
			cv::line(	*cOpenCV_Image,
						pt1,
						pt2,
						gColorTable[theColor],	//	cv::Scalar color,
						1,						//	int thickness CV_DEFAULT(1),
						8,						//	int line_type CV_DEFAULT(8),
						0);						//	int shift CV_DEFAULT(0));
		}
		else
		{
			cv::line(	*cOpenCV_Image,
						pt1,
						pt2,
						cCurrentColor,		//	cv::Scalar color,
						1,					//	int thickness CV_DEFAULT(1),
						8,					//	int line_type CV_DEFAULT(8),
						0);					//	int shift CV_DEFAULT(0));
		}

		cCurrentXloc	=	xx;
		cCurrentYloc	=	yy;
	}
	else
	{
		CONSOLE_ABORT("cOpenCV_Image is NULL");
	}
#else
	if (cOpenCV_Image != NULL)
	{
	CvPoint		pt1;
	CvPoint		pt2;
		pt1.x	=	xx;
		pt1.y	=	yy;

		pt2.x	=	xx;
		pt2.y	=	yy;
		if ((theColor >= 0) && (theColor < W_COLOR_LAST))
		{
			cvLine(	cOpenCV_Image,
					pt1,
					pt2,
					gColorTable[theColor],	//	cv::Scalar color,
					1,						//	int thickness CV_DEFAULT(1),
					8,						//	int line_type CV_DEFAULT(8),
					0);						//	int shift CV_DEFAULT(0));
		}
		else
		{
			cvLine(	cOpenCV_Image,
					pt1,
					pt2,
					cCurrentColor,		//	cv::Scalar color,
					1,					//	int thickness CV_DEFAULT(1),
					8,					//	int line_type CV_DEFAULT(8),
					0);					//	int shift CV_DEFAULT(0));
		}

		cCurrentXloc	=	xx;
		cCurrentYloc	=	yy;
	}
	else
	{
		CONSOLE_ABORT("cOpenCV_Image is NULL");
	}
#endif // _USE_OPENCV_CPP_
}
//**************************************************************************************
//*	Low Level FrameRect
//**************************************************************************************
void	WindowTab::LLG_FrameRect(int left, int top, int width, int height, int lineWidth)
{

	if ((width <= 0) || (height <= 0))
	{
		CONSOLE_DEBUG_W_NUM("width\t=", width);
		CONSOLE_DEBUG_W_NUM("height\t=", height);
		CONSOLE_ABORT(__FUNCTION__);
	}

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	if (cOpenCV_Image != NULL)
	{
	cv::Rect	myCVrect;

		myCVrect.x		=	left;
		myCVrect.y		=	top;
		myCVrect.width	=	width;
		myCVrect.height	=	height;

		cv::rectangle(	*cOpenCV_Image,
						myCVrect,
						cCurrentColor,
						lineWidth);

	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
#else
	if (cOpenCV_Image != NULL)
	{
	CvRect		myCVrect;
		myCVrect.x		=	left;
		myCVrect.y		=	top;
		myCVrect.width	=	width;
		myCVrect.height	=	height;

		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						cCurrentColor,				//	cv::Scalar color,
						lineWidth,					//	int thickness CV_DEFAULT(1),
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));

	}
#endif // _USE_OPENCV_CPP_
}

//**************************************************************************************
void	WindowTab::LLG_FrameRect(cv::Rect *theRect)
{
	LLG_FrameRect(theRect->x, theRect->y, theRect->width, theRect->height);
}

//**************************************************************************************
void	WindowTab::LLG_FillRect(		cv::Rect *theRect)
{
	LLG_FillRect(theRect->x, theRect->y, theRect->width, theRect->height);
}

//**************************************************************************************
//*	Low Level FrameRect
//**************************************************************************************
void	WindowTab::LLG_FillRect(int left, int top, int width, int height)
{

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	if (cOpenCV_Image != NULL)
	{
	cv::Rect	myCVrect;

		myCVrect.x		=	left;
		myCVrect.y		=	top;
		myCVrect.width	=	width;
		myCVrect.height	=	height;

		cv::rectangle(	*cOpenCV_Image,
						myCVrect,
						cCurrentColor,
					#if (CV_MAJOR_VERSION >= 3)
						cv::FILLED
					#else
						CV_FILLED
					#endif
						);

	}
#else
	if (cOpenCV_Image != NULL)
	{
	CvRect		myCVrect;

		myCVrect.x		=	left;
		myCVrect.y		=	top;
		myCVrect.width	=	width;
		myCVrect.height	=	height;

		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						cCurrentColor,				//	cv::Scalar color,
						CV_FILLED,					//	int thickness CV_DEFAULT(1),
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));
	}
#endif // _USE_OPENCV_CPP_
}

//*********************************************************************
void	WindowTab::LLG_FillEllipse(int xCenter, int yCenter, int xRadius, int yRadius, const double angle_deg)
{

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	if (cOpenCV_Image != NULL)
	{
	cv::Point	center;
	cv::Size	axes;

		if ((xRadius > 0) && (yRadius > 0))
		{
			center.x	=	xCenter;
			center.y	=	yCenter;
			axes.width	=	1 * xRadius;
			axes.height	=	1 * yRadius;

			cv::ellipse(	*cOpenCV_Image,
							center,
							axes,
							angle_deg,			//*	angle
							0.0,				//*	start_angle
							360.0,				//*	end_angle
							cCurrentColor,		//	cv::Scalar color,
						#if (CV_MAJOR_VERSION >= 3)
							cv::FILLED
						#else
							CV_FILLED
						#endif
							);		//	int thickness CV_DEFAULT(1),
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("xCenter\t=", xCenter);
			CONSOLE_DEBUG_W_NUM("yCenter\t=", yCenter);
			CONSOLE_DEBUG_W_NUM("xRadius\t=", xRadius);
			CONSOLE_DEBUG_W_NUM("yRadius\t=", yRadius);
			CONSOLE_ABORT("Invalid arguments");
		}

	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_matImage is NULL");
	}

#else
//	CONSOLE_DEBUG(__FUNCTION__);
	if (cOpenCV_Image != NULL)
	{
	CvPoint	center;
	CvSize	axes;

		if ((xRadius > 0) && (yRadius > 0))
		{
			center.x	=	xCenter;
			center.y	=	yCenter;
			axes.width	=	1 * xRadius;
			axes.height	=	1 * yRadius;

			cvEllipse(	cOpenCV_Image,
						center,
						axes,
						angle_deg,		//*	angle
						0.0,			//*	start_angle
						360.0,			//*	end_angle
						cCurrentColor,	//	cv::Scalar color,
						CV_FILLED,		//	int thickness CV_DEFAULT(1),
						8,				//	int line_type CV_DEFAULT(8),
						0);				//	int shift CV_DEFAULT(0));
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("xCenter\t=", xCenter);
			CONSOLE_DEBUG_W_NUM("yCenter\t=", yCenter);
			CONSOLE_DEBUG_W_NUM("xRadius\t=", xRadius);
			CONSOLE_DEBUG_W_NUM("yRadius\t=", yRadius);
			CONSOLE_ABORT("Invalid arguments");
		}
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
	}
#endif // _USE_OPENCV_CPP_
}

//*********************************************************************
void	WindowTab::LLG_FrameEllipse(int xCenter, int yCenter, int xRadius, int yRadius, const double angle_deg)
{
//	CONSOLE_DEBUG(__FUNCTION__);

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	if (cOpenCV_Image != NULL)
	{
	cv::Point	center;
	cv::Size	axes;

		if ((xRadius > 0) && (yRadius > 0))
		{
			center.x	=	xCenter;
			center.y	=	yCenter;
			axes.width	=	1 * xRadius;
			axes.height	=	1 * yRadius;

			cv::ellipse(	*cOpenCV_Image,
							center,
							axes,
							angle_deg,			//*	angle
							0.0,				//*	start_angle
							360.0,				//*	end_angle
							cCurrentColor,		//	cv::Scalar color,
							cCurrentLineWidth,	//	int thickness CV_DEFAULT(1),
							8,					//	int line_type CV_DEFAULT(8),
							0);					//	int shift CV_DEFAULT(0));
		}
		else
		{
			CONSOLE_DEBUG("Invalid arguments");
			CONSOLE_DEBUG_W_NUM("xCenter\t=", xCenter);
			CONSOLE_DEBUG_W_NUM("yCenter\t=", yCenter);
			CONSOLE_DEBUG_W_NUM("xRadius\t=", xRadius);
			CONSOLE_DEBUG_W_NUM("yRadius\t=", yRadius);
			CONSOLE_ABORT("Invalid arguments");
		}
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
		CONSOLE_ABORT("cOpenCV_Image is NULL");
	}
#else
	if (cOpenCV_Image != NULL)
	{
	CvPoint	center;
	CvSize	axes;
		if ((xRadius > 0) && (yRadius > 0))
		{
			center.x	=	xCenter;
			center.y	=	yCenter;
			axes.width	=	1 * xRadius;
			axes.height	=	1 * yRadius;

			cvEllipse(	cOpenCV_Image,
						center,
						axes,
						angle_deg,			//*	angle
						0.0,				//*	start_angle
						360.0,				//*	end_angle
						cCurrentColor,		//	cv::Scalar color,
						cCurrentLineWidth,	//	int thickness CV_DEFAULT(1),
						8,					//	int line_type CV_DEFAULT(8),
						0);					//	int shift CV_DEFAULT(0));
		}
		else
		{
			CONSOLE_ABORT("Invalid arguments");
		}
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
	}
#endif // _USE_OPENCV_CPP_
}



//*********************************************************************
void	WindowTab::LLG_FloodFill(const int xxx, const int yyy, const int color)
{
cv::Point	center;
cv::Scalar	newColor;
cv::Scalar	cvScalarAll;

//	CONSOLE_DEBUG(__FUNCTION__);
	center.x	=	xxx;
	center.y	=	yyy;

	newColor	=	gColorTable[color];

	if (cOpenCV_Image != NULL)
	{
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
		cvScalarAll.all(0.0);
		//---try------try------try------try------try------try---
		try
		{
			cv::floodFill(	*cOpenCV_Image,
							center, 					//	CvPoint seed_point,
							newColor,					//	cv::Scalar new_val,
							NULL,
							cvScalarAll,				//	cv::Scalar lo_diff CV_DEFAULT(cvScalarAll(0)),
							cvScalarAll);				//	cv::Scalar up_diff CV_DEFAULT(cvScalarAll(0)),

		}
#else
		cvScalarAll	=	cvRealScalar(0.0);
		//---try------try------try------try------try------try---
		try
		{
			cvFloodFill(cOpenCV_Image, 				//	CvArr* image,
						center, 					//	CvPoint seed_point,
						newColor,					//	cv::Scalar new_val,
						cvScalarAll,				//	cv::Scalar lo_diff CV_DEFAULT(cvScalarAll(0)),
						cvScalarAll,				//	cv::Scalar up_diff CV_DEFAULT(cvScalarAll(0)),
						NULL,						//	CvConnectedComp* comp CV_DEFAULT(NULL),
						4,							//	int flags CV_DEFAULT(4),
						NULL						//	CvArr* mask CV_DEFAULT(NULL));
						);
		}
#endif // _USE_OPENCV_CPP_
		catch(cv::Exception& ex)
		{
			//*	this catch prevents opencv from crashing
			CONSOLE_DEBUG("????????????????????????????????????????????????????");
			CONSOLE_DEBUG("cvFloodFill() had an exception");
			CONSOLE_DEBUG_W_NUM("openCV error code\t=",	ex.code);
		}
	}
}

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//*********************************************************************
void	SetRect(cv::Rect *theRect, const int top, const int left, const int bottom, const int right)
#else
//*********************************************************************
void	SetRect(CvRect *theRect, const int top, const int left, const int bottom, const int right)
#endif
{
	theRect->x		=	left;
	theRect->y		=	top;
	theRect->width	=	right - left;
	theRect->height	=	bottom - top;
}


#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//*********************************************************************
void	InsetRect(cv::Rect *theRect, const int xInset, const int yInset)
#else
//*********************************************************************
void	InsetRect(CvRect *theRect, const int xInset, const int yInset)
#endif
{
	theRect->x		+=	xInset;
	theRect->y		+=	yInset;
	theRect->width	-=	xInset * 2;
	theRect->height	-=	yInset * 2;
}


#ifdef _CONTROLLER_USES_ALPACA_
//*****************************************************************************
void	WindowTab::ForceAlpacaUpdate(void)
{
Controller	*myControllerObj;

//	CONSOLE_DEBUG(__FUNCTION__);

	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		myControllerObj->cForceAlpacaUpdate	=	true;
	}
}

//*****************************************************************************
bool	WindowTab::AlpacaSendPutCmd(	sockaddr_in		*deviceAddress,
										int				devicePort,
										const char		*alpacaDevice,
										const int		alpacaDevNum,
										const char		*alpacaCmd,
										const char		*dataString,
										SJP_Parser_t	*jsonParser)
{
bool		validData	=	false;
Controller	*myControllerObj;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);

	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
		CONSOLE_DEBUG_W_STR(alpacaDevice, alpacaCmd);
		if (jsonParser != NULL)
		{
			validData	=	myControllerObj->AlpacaSendPutCmdwResponse(	deviceAddress,
																		devicePort,
																		alpacaDevice,
																		alpacaDevNum,
																		alpacaCmd,
																		dataString,
																		jsonParser);
		}
		else
		{
		SJP_Parser_t	localJsonStruct;

//			CONSOLE_DEBUG("jsonParser is NULL");
			validData	=	myControllerObj->AlpacaSendPutCmdwResponse(	deviceAddress,
																		devicePort,
																		alpacaDevice,
																		alpacaDevNum,
																		alpacaCmd,
																		dataString,
																		&localJsonStruct);
		}
	}
	else
	{
		validData	=	false;
	}
	return(validData);
}

//*****************************************************************************
bool	WindowTab::AlpacaSendPutCmd(const char		*alpacaDevice,
									const char		*alpacaCmd,
									const char		*dataString,
									SJP_Parser_t	*jsonParser)
{
bool		validData;
Controller	*myControllerObj;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);

	myControllerObj	=	(Controller *)cParentObjPtr;
	if (myControllerObj != NULL)
	{
//		CONSOLE_DEBUG_W_STR(alpacaDevice, alpacaCmd);
		if (jsonParser != NULL)
		{
			validData	=	myControllerObj->AlpacaSendPutCmdwResponse(	alpacaDevice,
																		alpacaCmd,
																		dataString,
																		jsonParser);
			cLastAlpacaErrNum	=	AlpacaCheckForErrors(jsonParser, cLastAlpacaErrStr, true);


//			CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t=", cLastAlpacaErrNum);
//			CONSOLE_DEBUG_W_STR("cLastAlpacaErrStr\t=", cLastAlpacaErrStr);
		}
		else
		{
		SJP_Parser_t	localJsonParser;

			SJP_Init(&localJsonParser);
//			CONSOLE_DEBUG("jsonParser is NULL");
//			validData	=	myControllerObj->AlpacaSendPutCmd(	alpacaDevice,
//																alpacaCmd,
//																dataString);
			validData	=	myControllerObj->AlpacaSendPutCmdwResponse(	alpacaDevice,
																		alpacaCmd,
																		dataString,
																		&localJsonParser);
			cLastAlpacaErrNum	=	AlpacaCheckForErrors(&localJsonParser, cLastAlpacaErrStr, true);
		}
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
//*	this is the same function as in the controller class, but it easier to have it two places
//*****************************************************************************
int	WindowTab::AlpacaCheckForErrors(	SJP_Parser_t	*jsonParser,
										char			*errorMsg,
										bool 			reportError)
{
int		alpacaErrorCode;
Controller	*myParentObjPtr;

//	CONSOLE_DEBUG(__FUNCTION__);
	myParentObjPtr	=	(Controller *)cParentObjPtr;
	alpacaErrorCode	=	0;
	if (myParentObjPtr != NULL)
	{
		alpacaErrorCode	=	myParentObjPtr->AlpacaCheckForErrors(jsonParser, errorMsg, reportError);
	}
	return(alpacaErrorCode);
}

//*****************************************************************************
bool	WindowTab::AlpacaSetConnected(const char *deviceTypeStr, const bool newConnectedState)
{
bool		okFlag;
Controller	*myControllerObj;

//	CONSOLE_DEBUG(__FUNCTION__);

	myControllerObj	=	(Controller *)cParentObjPtr;
	okFlag			=	false;
	if (myControllerObj != NULL)
	{
		okFlag	=	myControllerObj->AlpacaSetConnected(deviceTypeStr, newConnectedState);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("cParentObjPtr is NULL, windowname=", cWindowName);
	}
	return(okFlag);
}

#endif	//	_CONTROLLER_USES_ALPACA_


