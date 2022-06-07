//*****************************************************************************
//*		windowtab_filelist.cpp		(c) 2020 by Mark Sproul
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
//*	Mar  4,	2020	<MLS> Created windowtab_filelist.cpp
//*****************************************************************************


#ifdef _ENABLE_CTRL_CAMERA_

#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"windowtab_filelist.h"
#include	"controller_camera.h"


//**************************************************************************************
WindowTabFileList::WindowTabFileList(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName)

	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
	CONSOLE_DEBUG(__FUNCTION__);
	SetupWindowControls();

}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabFileList::~WindowTabFileList(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

#define	kFileListHeight	200
#define	kLabelHeight	25
//**************************************************************************************
void	WindowTabFileList::SetupWindowControls(void)
{
int		yLoc;
int		lineHeight;
int		iii;
int		scrollBarWidth;
int		textAreaWidth;
int		textAreaHeight;
int		scrollBarYLoc;

	CONSOLE_DEBUG(__FUNCTION__);
	//------------------------------------------
	yLoc			=	cTabVertOffset;
	lineHeight		=	20;
	scrollBarWidth	=	25;

	//------------------------------------------
	SetWidget(		kFileList_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetFont(	kFileList_Title, kFont_Medium);
	SetBGcolorFromWindowName(kFileList_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//------------------------------------------
	SetWidget(			kFileList_Refresh,		0,			yLoc,		cWidth/3,	cBtnHeight);
	SetWidgetType(		kFileList_Refresh,	kWidgetType_Button);
	SetWidgetFont(		kFileList_Refresh,	kFont_Medium);
	SetWidgetBGColor(	kFileList_Refresh,	CV_RGB(255,	255,	255));
	SetWidgetText(		kFileList_Refresh,	"REFRESH");

	//------------------------------------------
	SetWidget(		kFileList_CountText,	cWidth/2,	yLoc,		cWidth/3,	cBtnHeight);
	SetWidgetType(	kFileList_CountText, kWidgetType_TextBox);
	SetWidgetFont(	kFileList_CountText, kFont_Medium);
	SetWidgetText(	kFileList_CountText, "Count=");

	yLoc			+=	cBtnHeight;
	yLoc			+=	4;
	scrollBarYLoc	=	yLoc;
	textAreaWidth	=	cWidth - scrollBarWidth - 4;
	//------------------------------------------
	for (iii=kFileList_FistEntry; iii<=kFileList_LastEntry; iii++)
	{
		SetWidget(				iii,	2,		yLoc,		textAreaWidth,		lineHeight);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetFont(			iii,	kFont_Medium);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetNumber(		iii,	iii);
		SetWidgetBorder(		iii,	false);
		yLoc			+=	lineHeight;
		yLoc			+=	1;
	}
	SetWidgetOutlineBox(kFileList_Outline, kFileList_FistEntry, kFileList_LastEntry);

	scrollBarYLoc	=	cWidgetList[kFileList_Outline].top;
	textAreaHeight	=	cWidgetList[kFileList_Outline].height;
	SetWidget(		kFileList_ScrollBar,	cWidth - scrollBarWidth,
											scrollBarYLoc,
											scrollBarWidth,
											textAreaHeight);
	SetWidgetType(	kFileList_ScrollBar,	kWidgetType_ScrollBar);


	SetAlpacaLogo(kFileList_AlpacaLogo, -1);


	//=======================================================
	//*	IP address
	SetIPaddressBoxes(kFileList_IPaddr, kFileList_Readall, kFileList_AlpacaDrvrVersion, -1);

}

//**************************************************************************************
void	WindowTabFileList::ProcessButtonClick(const int buttonIdx, const int flags)
{
	CONSOLE_DEBUG(__FUNCTION__);
	switch(buttonIdx)
	{
		case kFileList_Refresh:
			CONSOLE_DEBUG("Refresh");
			AlpacaGetFileList();
			break;

		case kFileList_Title:
		case kFileList_IPaddr:
			//*	do nothing
			break;

		default:
			break;
	}
}

//**************************************************************************************
void	WindowTabFileList::ProcessDoubleClick(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
	CONSOLE_DEBUG(__FUNCTION__);

	switch(widgetIdx)
	{
		case kFileList_Refresh:
		case kFileList_Title:
		case kFileList_IPaddr:
			//*	do nothing
			break;

		case kFileList_CountText:
			DumpWidgetList(0, 20);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("double click on ", widgetIdx);
			break;
	}
}



//*****************************************************************************
void	WindowTabFileList::AlpacaGetFileList(void)
{
ControllerCamera	*myCameraController;

	CONSOLE_DEBUG(__FUNCTION__);
	myCameraController	=	(ControllerCamera *)cParentObjPtr;

	if (myCameraController != NULL)
	{
		myCameraController->AlpacaGetFileList();
	}
	else
	{
		CONSOLE_DEBUG("myCameraController is NULL");
	}
}


#endif // _ENABLE_CTRL_CAMERA_

