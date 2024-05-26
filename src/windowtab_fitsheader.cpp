//*****************************************************************************
//*		windowtab_fitsheader.cpp		(c) 2024 by Mark Sproul
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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar 21,	2024	<MLS> Created windowtab_fitsheader.cpp
//*	Mar 23,	2024	<MLS> Added double click to open any web link specified
//*****************************************************************************

#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<netinet/in.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


//#include	"alpaca_defs.h"
//#include	"linuxerrors.h"
//#include	"helper_functions.h"

#include	"fits_helper.h"
#include	"windowtab_fitsheader.h"

#include	"controller.h"


//**************************************************************************************
WindowTabFITSheader::WindowTabFITSheader(	const int	xSize,
											const int	ySize,
											cv::Scalar	backGrndColor,
											const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cSortColumn			=	-1;
	cFirstLineIdx		=	0;
	cFitsLineCount		=	0;
	cFitsHeaderText		=	NULL;

	SetupWindowControls();
	UpdateButtons();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabFITSheader::~WindowTabFITSheader(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabFITSheader::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		textBoxHt;
int		textBoxWd;
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	xLoc	=	5;
	yLoc	=	cTabVertOffset;
	yLoc	=	SetTitleBox(kFitsHeader_Title, -1, yLoc, "FITS header");
	yLoc	=	SetTitleBox(kFitsHeader_FileName, -1, yLoc, "filename");



	//=======================================================
	xLoc		=	10;
	textBoxHt	=	14;
	textBoxWd	=	cWidth - (xLoc + 3);
	for (iii=kFitsHeader_Line_01; iii <= kFitsHeader_Line_Last; iii++)
	{
		SetWidgetType(			iii,	kWidgetType_TextBox_MonoSpace);
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetBorder(		iii,	false);
		SetWidgetBGColorSelected(iii,	CV_RGB(100,	100,	100));

		yLoc			+=	textBoxHt;
		yLoc			+=	4;
	}

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	SetAlpacaLogoBottomCorner(kFitsHeader_AlpacaLogo);
}

//*****************************************************************************
void	WindowTabFITSheader::SetNewSelectedDevice(int lineIndex)
{
int		iii;

	if ((cFitsHeaderText != NULL) && (lineIndex >= 0))
	{
		//*	clear out previous selections
		for (iii=0; iii<cFitsLineCount; iii++)
		{
			cFitsHeaderText[iii].lineSelected	=	false;
		}
		cFitsHeaderText[lineIndex].lineSelected	=	true;
		UpdateOnScreenWidgetList();
		ForceWindowUpdate();
	}
}

//*****************************************************************************
void	WindowTabFITSheader::HandleKeyDown(const int keyPressed)
{
int		deviceIndex;
int		iii;
int		theExtendedChar;

//	CONSOLE_DEBUG_W_HEX("keyPressed=", keyPressed);

	//*	find a selected device
	deviceIndex	=	-1;
	if (cFitsHeaderText != NULL)
	{
		for (iii=0; iii<cFitsLineCount; iii++)
		{
			if (cFitsHeaderText[iii].lineSelected)
			{
				deviceIndex	=	iii;
				break;
			}
		}
	}

	theExtendedChar	=	keyPressed & 0x00ffff;
	switch(theExtendedChar)
	{
		//*	return,
		case 0x0d:
		case 0xff8d:
			break;

		//*	delete key,
		case 0x0000ff:
		case 0x00ffff:
		case 0x0080ff:
			break;

		//*	up arrow key
		case 0x00ff52:
			deviceIndex	-=	1;
			if (deviceIndex >= 0)
			{
				SetNewSelectedDevice(deviceIndex);
			}
			break;

		//*	down arrow key
		case 0x00ff54:
			deviceIndex	+=	1;
			if (deviceIndex >= 0)
			{
				SetNewSelectedDevice(deviceIndex);
			}
			break;

		default:
//			CONSOLE_DEBUG_W_HEX("Ignored: keyPressed     \t=", keyPressed);
//			CONSOLE_DEBUG_W_HEX("Ignored: theExtendedChar\t=", theExtendedChar);
			break;
	}
}

//*****************************************************************************
void	WindowTabFITSheader::ProcessButtonClick(const int buttonIdx, const int flags)
{
bool	updateFlag;

//	CONSOLE_DEBUG(__FUNCTION__);
	updateFlag	=	true;
	switch(buttonIdx)
	{
		case 1:
			break;

		default:
			updateFlag	=	false;
			break;
	}
	if (updateFlag)
	{
		UpdateButtons();
		ForceWindowUpdate();
	}
}


//**************************************************************************************
void	WindowTabFITSheader::UpdateButtons(void)
{
//	SetWidgetChecked(kFitsHeader_TempModeRaw, (cGraphMode == kGraphMode_Raw));
//	SetWidgetChecked(kFitsHeader_TempModeAvg, (cGraphMode == kGraphMode_Avg5));
}


//*****************************************************************************
void	WindowTabFITSheader::ProcessDoubleClick(const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{
int		fitsLineIndex;
char	*httpPtr;
char	myURLstring[512];
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("widgetIdx\t=", widgetIdx);

	fitsLineIndex	=	cFirstLineIdx + (widgetIdx - kFitsHeader_Line_01);
//	CONSOLE_DEBUG_W_NUM("fitsLineIndex\t=", fitsLineIndex);

	if ((fitsLineIndex >= 0) && (fitsLineIndex < cFitsLineCount))
	{
		httpPtr	=	strcasestr(cFitsHeaderText[fitsLineIndex].fitsLine, "http");
		if (httpPtr != NULL)
		{
			strcpy(myURLstring, httpPtr);
			iii	=	0;
			while (myURLstring[iii] >= 0x20)
			{
				if (myURLstring[iii] == '\'')
				{
					myURLstring[iii]	=	0;
					break;
				}
				iii++;
			}

			CONSOLE_DEBUG_W_STR("myURLstring\t=", myURLstring);
			LaunchWebURL(myURLstring);
		}
	}
	else
	{
		CONSOLE_DEBUG("Index out of range");
	}
}

//*****************************************************************************
void	WindowTabFITSheader::ProcessMouseEvent(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{
//int		box_XXX;
//int		box_YYY;

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
			break;
		case cv::EVENT_MOUSEHWHEEL:
			break;
#endif
		default:
//			CONSOLE_DEBUG_W_NUM("UNKNOWN EVENT", event);
			break;
	}
}

//*****************************************************************************
void	WindowTabFITSheader::ProcessMouseWheelMoved(const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	wheelMovement,
												const int	flags)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, wheelMovement);

	//*	make sure we have something to scroll
	if (cFitsLineCount > 0)
	{
		cFirstLineIdx	+=	wheelMovement;
		if (cFirstLineIdx < 0)
		{
			cFirstLineIdx	=	0;
		}
		if (cFirstLineIdx >= cFitsLineCount)
		{
			cFirstLineIdx	=	cFitsLineCount -1;
		}
		UpdateOnScreenWidgetList();
		ForceWindowUpdate();
	}
}

//**************************************************************************************
void	WindowTabFITSheader::ProcessLineSelect(int widgetIdx)
{
int		fitsLineIndex;
int		adjustedIdx;
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= kFitsHeader_Line_01) && (widgetIdx < kFitsHeader_Line_Last))
	{
		adjustedIdx		=	widgetIdx - kFitsHeader_Line_01;
		fitsLineIndex	=	adjustedIdx + cFirstLineIdx;

		if ((fitsLineIndex >= 0) && (fitsLineIndex < cFitsLineCount))
		{
			SetNewSelectedDevice(fitsLineIndex);

			//*	clear out previous selections
			for (iii=0; iii<cFitsLineCount; iii++)
			{
				cFitsHeaderText[iii].lineSelected	=	false;
			}
			cFitsHeaderText[fitsLineIndex].lineSelected	=	true;
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("fitsLineIndex is out of bounds\t=", fitsLineIndex);
		}
		UpdateOnScreenWidgetList();
		ForceWindowUpdate();
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("widgetIdx is out of range\t=", widgetIdx);
	}
}

//**************************************************************************************
void	WindowTabFITSheader::UpdateOnScreenWidgetList(void)
{
int		boxId;
int		iii;
int		fitsLineIndex;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_ABORT(__FUNCTION__);

	//*	limit how far we can scroll
	if (cFirstLineIdx >= cFitsLineCount)
	{
		cFirstLineIdx	=	cFitsLineCount -1;
	}
	if (cFirstLineIdx < 0)
	{
		cFirstLineIdx	=	0;
	}

	if (cFitsHeaderText != NULL)
	{
		cLinesOnScreen	=	(kFitsHeader_Line_Last - kFitsHeader_Line_01) + 1;
		iii				=	0;
		while (iii < cLinesOnScreen)
		{
			boxId			=	iii + kFitsHeader_Line_01;
			fitsLineIndex	=	iii + cFirstLineIdx;

			if ((boxId < kFitsHeader_Line_Last) && (fitsLineIndex < cFitsLineCount))
			{
				SetWidgetText(boxId, cFitsHeaderText[fitsLineIndex].fitsLine);

				//-----------------------------------------------------
				//*	deal with selected state
				if (cFitsHeaderText[fitsLineIndex].lineSelected == true)
				{
					SetWidgetLineSelect(boxId, true);
				}
				else
				{
					SetWidgetLineSelect(boxId, false);
				}
			}
			else if (boxId <= kFitsHeader_Line_Last)
			{
				SetWidgetTextColor(		boxId,	CV_RGB(255,	255,	255));
				SetWidgetLineSelect(	boxId, false);
				SetWidgetText(boxId, "---");
			}
			else
			{
				SetWidgetText(boxId, "");
			}
			iii++;
		}
	}
	else
	{
		CONSOLE_DEBUG("cFitsHeaderText is null");
//		CONSOLE_ABORT(__FUNCTION__);
	}
}

//**************************************************************************************
void	WindowTabFITSheader::SetFitsHeaderData(TYPE_FitsHdrLine *fitsHeaderDataPtr, int fitsHeaderDataCnt)
{
char	titleText[64];

	cFitsHeaderText	=	fitsHeaderDataPtr;
	cFitsLineCount	=	fitsHeaderDataCnt;

	sprintf(titleText, "FITS header (%d lines)", cFitsLineCount);
	SetWidgetText(kFitsHeader_Title, titleText);
//	CONSOLE_DEBUG_W_STR("titleText:", titleText);

	UpdateOnScreenWidgetList();
}



