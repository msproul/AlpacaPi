//*****************************************************************************
//*		controller_windowtabs.cpp		(c) 2020 by Mark Sproul
//*
//*
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



#include	"windowtab.h"
#include	"controller.h"


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
		cWidgetList[iii].boarderColor	=	CV_RGB(255, 255, 255);

		cWidgetList[iii].includeBoarder	=	true;
	}
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTab::~WindowTab(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//**************************************************************************************
void	WindowTab::SetWidget(const int widgetIdx, int left, int top, int width, int height)
{

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
			cWidgetList[widgetIdx].includeBoarder	=	false;
		}
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetValid(const int widgetIdx, bool valid)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].valid	=	valid;
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
void	WindowTab::SetWidgetBoarderColor(const int widgetIdx, CvScalar newtextColor)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].boarderColor	=	newtextColor;
	}
}

//**************************************************************************************
void	WindowTab::SetWidgetImage(			const int widgetIdx, IplImage *argImagePtr)
{
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		cWidgetList[widgetIdx].openCVimagePtr	=	argImagePtr;
	}
}

//**************************************************************************************
void	WindowTab::SetWindowOutlineBox(const int widgetIdx, const int firstItem, const int lastItem)
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

	if ((strstr(cWindowName, "newt16") != NULL) || (strstr(cWindowName, "jetson") != NULL))
	{
		SetWidgetBGColor(	widgetIdx,	CV_RGB(0x72,	0x03,	0xA6));	//	#7203A6
		SetWidgetTextColor(	widgetIdx,	CV_RGB(255,		255,	255));
	}
	else if ((strstr(cWindowName, "wo71") != NULL) || (strstr(cWindowName, "pi-1") != NULL))
	{
		SetWidgetBGColor(	widgetIdx,	CV_RGB(255,	0,	0));
		SetWidgetTextColor(	widgetIdx,	CV_RGB(0,	0,	0));
	}
	else if ((strstr(cWindowName, "wo102") != NULL) || (strstr(cWindowName, "pi") != NULL))
	{
		SetWidgetBGColor(	widgetIdx,	CV_RGB(0xCD,	0xAC,	0x06));	//	#7203A6
		SetWidgetTextColor(	widgetIdx,	CV_RGB(0,		0,		0));
	}
	else if ((strstr(cWindowName, "tty") != NULL) || (strstr(cWindowName, "door") != NULL))
	{
		SetWidgetBGColor(	widgetIdx,	CV_RGB(0,	255,	0));
		SetWidgetTextColor(	widgetIdx,	CV_RGB(0,	0,		0));
	}
	else if ((strstr(cWindowName, "finder") != NULL) || (strstr(cWindowName, "CCTV") != NULL))
	{
		SetWidgetBGColor(	widgetIdx,	CV_RGB(0,		0,		255));
		SetWidgetTextColor(	widgetIdx,	CV_RGB(255,		255,	255));
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
bool	WindowTab::SetWidgetHighlighted(	const int widgetIdx, bool highlighted)
{

//	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= 0) && (widgetIdx < kMaxWidgets))
	{
		//*	check to see if the state is changine
		if (cWidgetList[widgetIdx].highLighted != highlighted)
		{
			cWidgetList[widgetIdx].highLighted	=	highlighted;

		}
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
	CONSOLE_DEBUG("this routine should be overloaded");
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
		validData	=	myControllerObj->AlpacaSendPutCmd(	alpacaDevice,
															alpacaCmd,
															dataString);
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
