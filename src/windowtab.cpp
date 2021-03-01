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
//*	Dec 30,	2020	<MLS> Added HandleKeyDown()
//*	Dec 31,	2020	<MLS> Added help text string to widget definition
//*	Dec 31,	2020	<MLS> Added DisplayButtonHelpText() & SetHelpTextBoxNumber()
//*	Dec 31,	2020	<MLS> Added ProcessMouseEvent()
//*	Jan  1,	2021	<MLS> Added flag needsUpdated for selective updating
//*	Jan  1,	2021	<MLS> Added FrameEllipse()
//*	Jan  4,	2021	<MLS> Added ForceUpdate()
//*	Jan  4,	2021	<MLS> Added ProcessMouseLeftButtonDown() & ProcessMouseLeftButtonDragged()
//*	Jan  5,	2021	<MLS> Added RunBackgroundTasks()
//*	Jan  5,	2021	<MLS> Added CPenSize()
//*	Jan  6,	2021	<MLS> Added GetWidgetText()
//*	Jan 10,	2021	<MLS> Added new version of AlpacaSendPutCmd()
//*	Jan 15,	2021	<MLS> Added SetWidgetTabStops()
//*	Jan 23,	2021	<MLS> Added overflow checking to help text string
//*	Jan 30,	2021	<MLS> Added ComputeWidgetColumns()
//*	Feb 10,	2021	<MLS> Added SetAlpacaLogoBottomCorner()
//*	Feb 13,	2021	<MLS> Added UpdateSliderValue()
//*	Feb 13,	2021	<MLS> Dragging the slider now works
//*	Feb 16,	2021	<MLS> Added SetHelpTextBoxColor()
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
#include	"widget.h"
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

	ComputeWidgetColumns(cWidth);

	cIpAddrTextBox		=	-1;
	cLastCmdTextBox		=	-1;
	cHelpTextBoxNumber	=	-1;
	cHelpTextBoxColor	=	CV_RGB(255,	255,	255);
	cPervDisplayedHelpBox	=	-1;

	cOpenCV_Image		=	NULL;
	cCurrentXloc		=	0;
	cCurrentYloc		=	0;

	cCurrentColor		=	CV_RGB(255,	255,	255);
	cCurrentLineWidth	=	1;

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
void WindowTab::RunBackgroundTasks(void)
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
		if ((widetType == kWidgetType_RadioButton) || (widetType == kWidgetType_CheckBox))
		{
			cWidgetList[widgetIdx].includeBorder	=	false;
		}

		if (widetType == kWidgetType_TextInput)
		{
			strcpy(cWidgetList[widgetIdx].textString, "_");
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
			CONSOLE_ABORT(__FUNCTION__);
		}
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
			CONSOLE_DEBUG("Help text is to long");
			CONSOLE_DEBUG(newText);
			CONSOLE_ABORT(__FUNCTION__);
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
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetBGColor(const int widgetIdx, CvScalar newtextColor)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].bgColor	=	newtextColor;
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetBorderColor(const int widgetIdx, CvScalar newtextColor)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].borderColor	=	newtextColor;
		cWidgetList[widgetIdx].needsUpdated	=	true;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetImage(			const int widgetIdx, IplImage *argImagePtr)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].openCVimagePtr	=	argImagePtr;
		cWidgetList[widgetIdx].widgetType		=	kWidgetType_Image;
		cWidgetList[widgetIdx].needsUpdated		=	true;
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

	yLoc	=	cHeight - cBtnHeight;
	yLoc	-=	1;
	//=======================================================
	//*	IP address
	if (ipaddrBox >= 0)
	{
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

//	CONSOLE_DEBUG(__FUNCTION__);

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
//*	returns logo height
int	WindowTab::SetAlpacaLogoBottomCorner(const int logoWidgetIdx)
{
int	logoWidth;
int	logoHeight;
int	xLoc;
int	yLoc;

	logoHeight	=	0;
	if (logoWidgetIdx >= 0)
	{
		//*	now set the Alpaca Logo
		LoadAlpacaLogo();
		if (gAlpacaLogoPtr != NULL)
		{
			logoWidth	=	gAlpacaLogoPtr->width;
			logoHeight	=	gAlpacaLogoPtr->height;

			xLoc		=	cWidth - logoWidth;
			yLoc		=	cHeight - logoHeight;

			SetWidget(		logoWidgetIdx,	xLoc,	yLoc,	logoWidth,	logoHeight);
			SetWidgetImage(	logoWidgetIdx, gAlpacaLogoPtr);
		}
	}
	return(logoHeight);
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
//		CONSOLE_DEBUG("cLastCmdTextBox not set");
//		CONSOLE_ABORT(__FUNCTION__);
	}
}


//**************************************************************************************
void	WindowTab::SetWidgetValid(const int widgetIdx, bool valid)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, widgetIdx);

	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].valid	=	valid;
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


//**************************************************************************************
void	WindowTab::SetupWindowControls(void)
{

}

//**************************************************************************************
void	WindowTab::DrawGraphWidget(IplImage *openCV_Image, const int widgetIdx)
{
	//*	this routine should be overloaded
}


//**************************************************************************************
void	WindowTab::ForceUpdate(void)
{
Controller	*myControllerObj;

//	CONSOLE_DEBUG(__FUNCTION__);

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

			case kWidgetType_Custom:
			case kWidgetType_Graph:
			case kWidgetType_Graphic:
			case kWidgetType_Image:
			case kWidgetType_MultiLineText:
			case kWidgetType_OutlineBox:
			case kWidgetType_ProessBar:
			case kWidgetType_ScrollBar:
			case kWidgetType_Slider:
			case kWidgetType_Text:
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
void	WindowTab::ProcessButtonClick(const int buttonIdx)
{
//	CONSOLE_DEBUG_W_NUM("this routine should be overloaded: buttonIdx=", buttonIdx);
	//*	this routine should be overloaded
}

//*****************************************************************************
void	WindowTab::ProcessDoubleClick(	const int	widgetIdx,
										const int	event,
										const int	xxx,
										const int	yyy,
										const int	flags)
{
//	CONSOLE_DEBUG("this routine should be overloaded");
	//*	this routine should be overloaded
}

//*****************************************************************************
void	WindowTab::ProcessMouseEvent(	const int	widgetIdx,
										const int	event,
										const int	xxx,
										const int	yyy,
										const int	flags)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	//*	this routine can be overloaded
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
//*	this routine can be overloaded
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
//		CONSOLE_DEBUG_W_NUM("WE have a slider", xxx);
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
			if (myPixelOffset < 0)
			{
				myPixelOffset	=	0;
			}
			else if (myPixelOffset > pixelRange)
			{
				myPixelOffset	=	pixelRange;
			}
			mySliderValuePercent	=	(1.0 * myPixelOffset) / (1.0 * pixelRange);
//			CONSOLE_DEBUG_W_NUM("minimumPixelValue\t=", minimumPixelValue);
//			CONSOLE_DEBUG_W_NUM("myPixelOffset    \t=", myPixelOffset);
//			CONSOLE_DEBUG_W_NUM("maximumPixelValue\t=", maximumPixelValue);

//			CONSOLE_DEBUG_W_DBL("mySliderValuePercent\t=", mySliderValuePercent);

			//*	now translate this to a range within the sliders min/max
			sliderRange		=	cWidgetList[widgetIdx].sliderMax - cWidgetList[widgetIdx].sliderMin;
			newSliderValue	=	mySliderValuePercent * sliderRange;
//			CONSOLE_DEBUG_W_DBL("newSliderValue\t=", newSliderValue);
			UpdateSliderValue(widgetIdx, newSliderValue);
		}
	}
	else
	{
	//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, xxx);
	}
}

//*****************************************************************************
void	WindowTab::SetHelpTextBoxNumber(const int buttonIdx)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cHelpTextBoxNumber	=	buttonIdx;
}

//*****************************************************************************
//*	this is so that the box can be used for error messages that might set to red
void	WindowTab::SetHelpTextBoxColor(CvScalar newtextColor)
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
			if (buttonIdx != cPervDisplayedHelpBox)
			{
				SetWidgetTextColor(	cHelpTextBoxNumber, cHelpTextBoxColor);
				SetWidgetText(		cHelpTextBoxNumber, cWidgetList[buttonIdx].helpText);
				updateOccured			=	true;
				//*	keep track of which button we did
				cPervDisplayedHelpBox	=	buttonIdx;
			}
		}
	}
	return(updateOccured);
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
	//	CONSOLE_DEBUG("cIpAddrTextBox not set!!!!!");
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

			CONSOLE_DEBUG("jsonParser is NULL");
	//+		validData	=	myControllerObj->AlpacaSendPutCmd(	alpacaDevice,
	//															alpacaCmd,
	//															dataString);
			validData	=	myControllerObj->AlpacaSendPutCmdwResponse(	deviceAddress,
																		devicePort,
																		alpacaDevice,
																		alpacaDevNum,
																		alpacaCmd,
																		dataString,
																		&localJsonStruct);
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
//			CONSOLE_DEBUG("Calling AlpacaCheckForErrors()");
			cLastAlpacaErrNum	=	AlpacaCheckForErrors(jsonParser, cLastAlpacaErrStr, true);


//			CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum\t=", cLastAlpacaErrNum);
//			CONSOLE_DEBUG_W_STR("cLastAlpacaErrStr\t=", cLastAlpacaErrStr);
		}
		else
		{
//			CONSOLE_DEBUG("jsonParser is NULL");
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
//*	this is the same function as in the controller class, but it easier to have it two places
//*****************************************************************************
int	WindowTab::AlpacaCheckForErrors(	SJP_Parser_t	*jsonParser,
										char			*errorMsg,
										bool 			reportError)
{
int		jjj;
int		alpacaErrorCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrorCode	=	0;
	strcpy(errorMsg, "");
	if (jsonParser != NULL)
	{
		for (jjj=0; jjj<jsonParser->tokenCount_Data; jjj++)
		{
			if (strcasecmp(jsonParser->dataList[jjj].keyword, "ErrorNumber") == 0)
			{
				alpacaErrorCode	=	atoi(jsonParser->dataList[jjj].valueString);
			}
			else if (strcasecmp(jsonParser->dataList[jjj].keyword, "ErrorMessage") == 0)
			{
				if (strlen(jsonParser->dataList[jjj].valueString) > 0)
				{
					strcpy(errorMsg, jsonParser->dataList[jjj].valueString);
					if (reportError)
					{
						AlpacaDisplayErrorMessage(errorMsg);
					}
				}
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("jsonParser is NULL");
	}
	return(alpacaErrorCode);
}


//*****************************************************************************
void	WindowTab::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
	//*	this should be overloaded
	CONSOLE_DEBUG_W_STR("Json err:", errorMsgString);
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


//*****************************************************************************
void	WindowTab::CMoveTo(const int xx, const int yy)
{
	cCurrentXloc	=	xx;
	cCurrentYloc	=	yy;
}

//*****************************************************************************
void	WindowTab::CLineTo(const int xx, const int yy)
{
CvPoint		pt1;
CvPoint		pt2;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cOpenCV_Image != NULL)
	{
		pt1.x	=	cCurrentXloc;
		pt1.y	=	cCurrentYloc;

		pt2.x	=	xx;
		pt2.y	=	yy;
		cvLine(	cOpenCV_Image,
				pt1,
				pt2,
				cCurrentColor,		//	CvScalar color,
				cCurrentLineWidth,	//	int thickness CV_DEFAULT(1),
				8,					//	int line_type CV_DEFAULT(8),
				0);					//	int shift CV_DEFAULT(0));

		cCurrentXloc	=	xx;
		cCurrentYloc	=	yy;
	}
	else
	{
		CONSOLE_ABORT("cOpenCV_Image is NULL");
	}
}

//*****************************************************************************
void	WindowTab::DrawCString(const int xx, const int yy, const char *theString)
{
CvPoint		textLoc;

//	CONSOLE_DEBUG(theString);

	if (cOpenCV_Image != NULL)
	{
		textLoc.x	=	xx;
		textLoc.y	=	yy;
		cvPutText(	cOpenCV_Image,
					theString,
					textLoc,
					&gTextFont[1],
					cCurrentColor
				);

	}
	else
	{
		CONSOLE_ABORT("cOpenCV_Image is NULL");
	}
}


//*****************************************************************************
CvScalar	gColorTable[]	=
{
	//	https://www.htmlcolor-picker.com/
	//*	these MUST be in the same order as the enums
	CV_RGB(255,	255,	255),	//*	WHITE
	CV_RGB(000,	000,	000),	//*	BLACK

	CV_RGB(255,	000,	000),	//*	RED
	CV_RGB(000,	255,	000),	//*	GREEN
	CV_RGB(000,	000,	255),	//*	BLUE

	CV_RGB(000,	255,	255),	//*	CYAN
	CV_RGB(255,	000,	255),	//*	MAGENTA
	CV_RGB(255,	255,	000),	//*	YELLOW

	CV_RGB(96,	000,	000),	//*	DARKRED
	CV_RGB(000,	 64,	000),	//*	DARKGREEN
	CV_RGB(000,	000,	150),	//*	DARKBLUE



	CV_RGB(192,	192,	192),	//*	LIGHTGRAY
	CV_RGB(64,	 64,	 64),	//*	DARKGRAY

	CV_RGB(255,	128,	255),	//*	LIGHTMAGENTA

	CV_RGB(0x66, 0x3d,	0x14),	//*	BROWN
	CV_RGB(231,	  5,	254),	//*	PINK

};

//*****************************************************************************
void	WindowTab::SetColor(const int theColor)
{
	if ((theColor >= 0) && (theColor < COLOR_LAST))
	{
		cCurrentColor	=	gColorTable[theColor];
	}
	else
	{
		cCurrentColor	=	CV_RGB(255,	255,	255);
	}
}
//*****************************************************************************
void	WindowTab::CPenSize(const int newLineWidth)
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
void	WindowTab::Putpixel(const int xx, const int yy, const int theColor)
{
CvPoint		pt1;
CvPoint		pt2;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cOpenCV_Image != NULL)
	{
		pt1.x	=	xx;
		pt1.y	=	yy;

		pt2.x	=	xx;
		pt2.y	=	yy;
		if ((theColor >= 0) && (theColor < COLOR_LAST))
		{
			cvLine(	cOpenCV_Image,
					pt1,
					pt2,
					gColorTable[theColor],	//	CvScalar color,
					1,						//	int thickness CV_DEFAULT(1),
					8,						//	int line_type CV_DEFAULT(8),
					0);						//	int shift CV_DEFAULT(0));
		}
		else
		{
			cvLine(	cOpenCV_Image,
					pt1,
					pt2,
					cCurrentColor,		//	CvScalar color,
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
}

//*********************************************************************
void	WindowTab::FillEllipse(int xCenter, int yCenter, int xRadius, int yRadius)
{
CvPoint	center;
CvSize	axes;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("xCenter\t=", xCenter);
//	CONSOLE_DEBUG_W_NUM("yCenter\t=", yCenter);
//	CONSOLE_DEBUG_W_NUM("xRadius\t=", xRadius);
//	CONSOLE_DEBUG_W_NUM("yRadius\t=", yRadius);
	if (cOpenCV_Image != NULL)
	{
		if ((xRadius > 0) && (yRadius > 0))
		{
			center.x	=	xCenter;
			center.y	=	yCenter;
			axes.width	=	2 * xRadius;
			axes.height	=	2 * yRadius;

			cvEllipse(	cOpenCV_Image,
						center,
						axes,
						0.0,			//*	angle
						0.0,			//*	start_angle
						360.0,			//*	end_angle
						cCurrentColor,	//	CvScalar color,
						CV_FILLED,		//	int thickness CV_DEFAULT(1),
						8,				//	int line_type CV_DEFAULT(8),
						0);				//	int shift CV_DEFAULT(0));
		}
		else
		{
			CONSOLE_ABORT("Invalid arguments");
		}
	}
	else
	{
		CONSOLE_ABORT("cOpenCV_Image is NULL");
	}
}

//*********************************************************************
void	WindowTab::FrameEllipse(int xCenter, int yCenter, int xRadius, int yRadius)
{
CvPoint	center;
CvSize	axes;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("xCenter\t=", xCenter);
//	CONSOLE_DEBUG_W_NUM("yCenter\t=", yCenter);
//	CONSOLE_DEBUG_W_NUM("xRadius\t=", xRadius);
//	CONSOLE_DEBUG_W_NUM("yRadius\t=", yRadius);
	if (cOpenCV_Image != NULL)
	{
		if ((xRadius > 0) && (yRadius > 0))
		{
			center.x	=	xCenter;
			center.y	=	yCenter;
			axes.width	=	2 * xRadius;
			axes.height	=	2 * yRadius;

			cvEllipse(	cOpenCV_Image,
						center,
						axes,
						0.0,			//*	angle
						0.0,			//*	start_angle
						360.0,			//*	end_angle
						cCurrentColor,	//	CvScalar color,
						1,				//	int thickness CV_DEFAULT(1),
						8,				//	int line_type CV_DEFAULT(8),
						0);				//	int shift CV_DEFAULT(0));
		}
		else
		{
			CONSOLE_ABORT("Invalid arguments");
		}
	}
	else
	{
		CONSOLE_ABORT("cOpenCV_Image is NULL");
	}
}

//*********************************************************************
void	SetRect(CvRect *theRect, const int top, const int left, const int bottom, const int right)
{
	theRect->x		=	left;
	theRect->y		=	top;
	theRect->width	=	right - left;
	theRect->height	=	bottom - top;
}


//*********************************************************************
void	InsetRect(CvRect *theRect, const int xInset, const int yInset)
{
	theRect->x		+=	xInset;
	theRect->y		+=	yInset;
	theRect->width	-=	xInset * 2;
	theRect->height	-=	yInset * 2;
}
