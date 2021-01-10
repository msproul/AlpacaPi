//*****************************************************************************
//*		controller.cpp		(c) 2020 by Mark Sproul
//*				Controller base class
//*
//*	Description:	This is a base class for a opencv based controller window
//*					that controls Alpaca devices
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
//*	Feb 20,	2020	<MLS> Created controller.cpp
//*	Feb 23,	2020	<MLS> Now using host name from /etc/hosts for window name
//*	Feb 25,	2020	<MLS> Added DrawWidgetMultiLineText()
//*	Mar  2,	2020	<MLS> Added background color to controller class
//*	Mar  2,	2020	<MLS> Added AlpacaGetIntegerValue() & DrawWidgetRadioButton()
//*	Mar  3,	2020	<MLS> Added AlpacaGetBooleanValue()
//*	Mar  3,	2020	<MLS> Added ProcessDoubleClick()
//*	Mar  8,	2020	<MLS> Added DrawWidgetGraph()
//*	Mar  9,	2020	<MLS> Added cLastAlpacaErrNum & cLastAlpacaErrStr
//*	Mar  9,	2020	<MLS> Control-q now sets keep running flag to false and quits
//*	Mar 14,	2020	<MLS> Control-w closes current window
//*	Mar 14,	2020	<MLS> Added Controller_HandleKeyDown()
//*	Mar 14,	2020	<MLS> Can now close a window independently
//*	Mar 14,	2020	<MLS> Image widget now centers undersized image
//*	Mar 15,	2020	<MLS> Added kFont_RadioBtn
//*	Mar 22,	2020	<MLS> Moved AlpacaGetStatus_ReadAll() into parent class
//*	Mar 23,	2020	<MLS> Added SetWidgetCrossedout()
//*	Mar 23,	2020	<MLS> Updated DrawWidgetMultiLineText() to understand justification
//*	Apr  8,	2020	<MLS> Added DrawWidgetIcon()
//*	May  5,	2020	<MLS> Added DrawOneWidget() & DrawWidgetButton()
//*	May  5,	2020	<MLS> Added button highlighting when clicked
//*	May 25,	2020	<MLS> Doubleclick in tab bar now resets window back to correct size
//*	Jun 30,	2020	<MLS> Added RefreshWindow()
//*	Jul  9,	2020	<MLS> Added UpdateWindowTabColors()
//*	Dec 29,	2020	<MLS> Added SetWidgetImage()
//*	Dec 31,	2020	<MLS> Added DisplayButtonHelpText()
//*	Jan  1,	2021	<MLS> Added UpdateWindowAsNeeded()
//*	Jan  1,	2021	<MLS> Added IsWidgetButton()
//*	Jan  6,	2021	<MLS> Started on text input field
//*	Jan  6,	2021	<MLS> Added HandleKeyDownInTextWidget()
//*	Jan  6,	2021	<MLS> Added GetWidgetText()
//*****************************************************************************


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/core/version.hpp"

#if (CV_MAJOR_VERSION >= 3)
	#include "opencv2/imgproc/imgproc.hpp"
#endif

#include	"discovery_lib.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"widget.h"
#include	"controller.h"

Controller	*gControllerList[kMaxControllers];
int			gControllerCnt	=	-1;
CvFont		gTextFont[kFontCnt];

Controller	*gCurrentActiveWindow	=	NULL;

//*****************************************************************************
static void	InitControllerList(void)
{
int		ii;


	for (ii=0; ii<kMaxControllers; ii++)
	{
		gControllerList[ii]	=	NULL;
	}
	gControllerCnt	=	0;
}


//*****************************************************************************
static void	LiveWindowMouseCallback(int event, int x, int y, int flags, void* param)
{
Controller	*myController;

//	CONSOLE_DEBUG(__FUNCTION__);

	myController	=	(Controller *)param;
	if (myController != NULL)
	{
		gCurrentActiveWindow	=	myController;
		myController->ProcessMouseEvent(event, x, y, flags);

	}
	else
	{
		CONSOLE_DEBUG("myController is NULL");
	}
}

//*****************************************************************************
void	Controller_HandleKeyDown(const int keyPressed)
{
//	CONSOLE_DEBUG_W_HEX(__FUNCTION__, keyPressed);

	if (gCurrentActiveWindow != NULL)
	{
	#if 0
	//*	this should cycle through the windows, but it does not work.
		nextCtrlIdx	=	-1;
		if ((keyPressed & 0x0ff) == 0x09)
		{
		int		iii;
		int		nextCtrlIdx;

			CONSOLE_DEBUG("TAB");
			for (iii=0; iii<gControllerCnt; iii++)
			{
				if (gCurrentActiveWindow == gControllerList[iii])
				{
					nextCtrlIdx	=	iii+1;
				}
			}
			if ((nextCtrlIdx < 0) || (nextCtrlIdx >= gControllerCnt))
			{
				nextCtrlIdx	=	0;
			}
			gCurrentActiveWindow	=	gControllerList[nextCtrlIdx];

			if (gCurrentActiveWindow != NULL)
			{
				CONSOLE_DEBUG(gCurrentActiveWindow->cWindowName);

				gCurrentActiveWindow->HandleWindowUpdate();
			}
			else
			{
				CONSOLE_DEBUG("gCurrentActiveWindow is NULL");
			}
		}
		else
	#endif
		{
			gCurrentActiveWindow->HandleKeyDown(keyPressed);
		}
	}
	else
	{
		CONSOLE_DEBUG("No window active");
	}
}

//*****************************************************************************
Controller::Controller(	const char	*argWindowName,
						const int	xSize,
						const int	ySize)
{
int			ii;
int			objCntr;

	CONSOLE_DEBUG(__FUNCTION__);

	cKeepRunning			=	true;
	cDebugCounter			=	0;
	cUpdateProtect			=	false;
	cHasReadAll				=	false;
	cReadStartup			=	true;
	cLeftButtonDown			=	false;
	cRightButtonDown		=	false;
	cLastClicked_Btn		=	-1;
	cLastClicked_Tab		=	-1;
	cHighlightedBtn			=	-1;
	cCurTextInput_Widget	=	-1;

	cOnLine					=	true;		//*	assume its online, if it wasnt, we wouldnt be here
	cAlpacaVersionString[0]	=	0;
	cLastAlpacaCmdString[0]	=	0;
	cLastAlpacaErrStr[0]	=	0;
	cAlpacaDeviceType[0]	=	0;
	cAlpacaDeviceName[0]	=	0;

	if (gControllerCnt < 0)
	{
		InitControllerList();
	}

	//*	find the first empty slot in the list
	for (ii=0; ii<kMaxControllers; ii++)
	{
		if (gControllerList[ii] == NULL)
		{
			CONSOLE_DEBUG_W_STR("Controller added to list", argWindowName);
			gControllerList[ii]	=	this;
			break;
		}
	}
	//*	now go thru and see how many are in the list
	objCntr	=	0;
	for (ii=0; ii<kMaxControllers; ii++)
	{
		if (gControllerList[ii] != NULL)
		{
			objCntr++;
		}
	}
	gControllerCnt	=	objCntr;

	InitWindowTabs();

//	CONSOLE_DEBUG_W_NUM("xSize\t=",	xSize);
//	CONSOLE_DEBUG_W_NUM("ySize\t=",	ySize);
	strcpy(cWindowName, argWindowName);
//	CONSOLE_DEBUG_W_STR("cWindowName\t=",	cWindowName);
	cWidth				=	xSize;
	cHeight				=	ySize;
	cUpdateWindow		=	true;
	cLastAlpacaErrNum	=	0;

	cBackGrndColor		=	CV_RGB(0,	0,	0);					//	CvScalar color,

	cOpenCV_Image		=	cvCreateImage(cvSize(cWidth, cHeight), IPL_DEPTH_8U, 3);

	cvNamedWindow(	cWindowName,
				//	(CV_WINDOW_NORMAL)
					(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
				//	(CV_WINDOW_AUTOSIZE)
				//	(CV_WINDOW_NORMAL | CV_GUI_EXPANDED)
				//	(CV_WINDOW_NORMAL | CV_WINDOW_FULLSCREEN | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
				//	(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
				//+	(CV_WINDOW_NORMAL | CV_WINDOW_KEEPRATIO | CV_GUI_NORMAL)
				//	(CV_WINDOW_AUTOSIZE | CV_WINDOW_KEEPRATIO | CV_GUI_EXPANDED)
				//	(CV_WINDOW_AUTOSIZE)
					);

	cvResizeWindow(		cWindowName, cWidth, cHeight);
//	cvMoveWindow(		cWindowName, (20 + ((gControllerCnt - 1) * (cWidth + 50))), 10);
	cvMoveWindow(		cWindowName, (20 + ((gControllerCnt - 1) * (150))), 10);


	cvSetMouseCallback( cWindowName,
						LiveWindowMouseCallback,
						(void *)this);



	gTextFont[kFont_Small]		=	cvFont(0.7, 1);
	gTextFont[kFont_RadioBtn]	=	cvFont(0.8, 1);
	gTextFont[kFont_Medium]		=	cvFont(1.0, 1);
	gTextFont[kFont_Large]		=	cvFont(1.7, 1);

	//*	set defaults for the tabs
	for (ii=0; ii<kMaxTabs; ii++)
	{
		memset(&cTabList[ii], 0, sizeof(TYPE_WIDGET));
		cTabList[ii].bgColor			=	CV_RGB(128,	128,	128);
		cTabList[ii].textColor			=	CV_RGB(0,	0,		255);
		cTabList[ii].borderColor		=	CV_RGB(255,	255,	255);
		cTabList[ii].fontNum			=	kFont_Medium;
		cTabList[ii].justification		=	kJustification_Center;
	}
	cTabCount		=	0;
	cCurrentTabNum	=	0;

	SetupWindowControls();

	gCurrentActiveWindow	=	this;
}


//**************************************************************************************
// Destructor
//**************************************************************************************
Controller::~Controller(void)
{
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
	//*	if we are the active window, make sure we dont get any more key presses
	if (gCurrentActiveWindow == this)
	{
		gCurrentActiveWindow	=	NULL;
	}
	//*	release the image
	if (cOpenCV_Image != NULL)
	{
		cvReleaseImage(&cOpenCV_Image);
	}
	cvDestroyWindow(cWindowName);

	for (iii=0; iii<kMaxControllers; iii++)
	{
		if (gControllerList[iii] == this)
		{
			gControllerList[iii]	=	NULL;
		}
	}
}



//**************************************************************************************
void	Controller::SetupWindowControls(void)
{

}

//*****************************************************************************
void	Controller::SetWindowIPaddrInfo(	const char	*textString,
											const bool	onLine)
{
int		iii;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	for (iii=0; iii<kMaxTabs; iii++)
	{
		if (cWindowTabs[iii] != NULL)
		{
			cWindowTabs[iii]->SetWindowIPaddrInfo(textString, onLine);
		}
	}
	cUpdateWindow	=	true;
}


//**************************************************************************************
void	Controller::RunBackgroundTasks(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	Controller::HandleWindow(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	RunBackgroundTasks();

	if (cUpdateWindow)
	{
//		CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
		HandleWindowUpdate();
		cUpdateWindow	=	false;
	}
}

//**************************************************************************************
void	Controller::HandleWindowUpdate(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if (cOpenCV_Image != NULL)
	{
		//*	Do drawing
		DrawWindow();

		cvShowImage(cWindowName, cOpenCV_Image);
//		cvUpdateWindow(cWindowName);//
	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
	}
}

//*****************************************************************************
//*	this routine only redraws the widgets marked as needing updating
//*****************************************************************************
void	Controller::UpdateWindowAsNeeded(void)
{
int				iii;
int				updatedCnt;
TYPE_WIDGET		*myWidgetPtr;

	if (cUpdateProtect)
	{
		return;
	}
	cUpdateProtect	=	true;
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, cDebugCounter++);
	if (cOpenCV_Image != NULL)
	{
		if (cCurrentTabObjPtr != NULL)
		{
			myWidgetPtr	=	cCurrentTabObjPtr->cWidgetList;
		}
		else
		{
			CONSOLE_DEBUG("cCurrentTabObjPtr is NULL");
			myWidgetPtr	=	NULL;
		}

		if (myWidgetPtr != NULL)
		{
			updatedCnt	=	0;
			//*	draw the widgets
			for (iii=0; iii<kMaxWidgets; iii++)
			{
				if (myWidgetPtr[iii].needsUpdated)
				{
					//*	first erase that rectangle with background color


					//*	draw the widget that needs updating
					DrawOneWidget(&myWidgetPtr[iii], iii);
					myWidgetPtr[iii].needsUpdated	=	false;
					updatedCnt++;
				}
			}
			cvShowImage(cWindowName, cOpenCV_Image);
			cvWaitKey(15);
//			CONSOLE_DEBUG_W_NUM("updatedCnt\t=", updatedCnt);
		}
		else
		{
			CONSOLE_DEBUG("widget ptr is NULL");
		}

	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_Image is NULL");
	}
	cUpdateProtect	=	false;
}


//*****************************************************************************
int	Controller::FindClickedTab(const int xxx, const int yyy)
{
int		iii;
bool	ptInWidget;
int		widgetIdx;

	widgetIdx	=	-1;
	iii			=	0;
	while ((iii<cTabCount) && (widgetIdx < 0))
	{
		if (cTabList[iii].valid)
		{
			ptInWidget	=	PointInWidget(xxx, yyy, &cTabList[iii]);
			if (ptInWidget)
			{
				widgetIdx	=	iii;
			}
		}
		iii++;
	}
	return(widgetIdx);
}


//*****************************************************************************
void	Controller::InitWindowTabs(void)
{
int		iii;

	for (iii=0; iii<kMaxTabs; iii++)
	{
		cWindowTabs[iii]	=	NULL;
	}
	cCurrentTabObjPtr	=	NULL;
}



//**************************************************************************************
//*	returns the vertical offset for the tabs
int	Controller::SetTabCount(const int newTabCount)
{
int		iii;
int		tabWidth;
int		tabHeight;
int		tabLeft;

	if (newTabCount <= kMaxTabs)
	{
		cTabCount	=	newTabCount;
	}
	else
	{
		CONSOLE_DEBUG("Too many tabs");
		cTabCount	=	kMaxTabs;
	}
	tabWidth	=	(cWidth / cTabCount) - 1;
	tabHeight	=	24;
	tabLeft		=	0;
	for (iii=0; iii<cTabCount; iii++)
	{
		cTabList[iii].valid			=	true;
		cTabList[iii].widgetType	=	kWidgetType_Text;
		cTabList[iii].left			=	tabLeft;
		cTabList[iii].top			=	0;
		cTabList[iii].width			=	tabWidth;
		cTabList[iii].height		=	tabHeight;
		cTabList[iii].fontNum		=	kFont_Small;

		sprintf(cTabList[iii].textString, "Tab-%d", (iii+1));
		tabLeft	+=	tabWidth + 1;
	}
	return(tabHeight + 2);
}

//**************************************************************************************
void	Controller::SetTabText(const int tabIdx, const char *tabName)
{

	if ((tabIdx >= 0) && (tabIdx < kMaxTabs))
	{
		strcpy(cTabList[tabIdx].textString, tabName);
		cUpdateWindow	=	true;
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("tabIdx out of range\t=", tabIdx);
	}
}

//*****************************************************************************
void	Controller::ProcessTabClick(const int tabIdx)
{
	if (tabIdx != cCurrentTabNum)
	{
		cCurrentTabNum		=	tabIdx;
		cUpdateWindow		=	true;
		cCurrentTabObjPtr	=	cWindowTabs[cCurrentTabNum];
	}
	else
	{
		//*	do nothing
//		CONSOLE_DEBUG("Do nothing");
	}
}


//*****************************************************************************
int	Controller::FindClickedWidget(const int xxx, const int yyy)
{
int		widgetIdx;

	if (cCurrentTabObjPtr != NULL)
	{
		widgetIdx	=	cCurrentTabObjPtr->FindClickedWidget(xxx, yyy);
	}
	else
	{
		widgetIdx	=	-1;
		CONSOLE_DEBUG("cCurrentTabObjPtr is NULL");
	}
	return(widgetIdx);
}

//*****************************************************************************
bool	Controller::IsWidgetButton(const int widgetIdx)
{
bool		widgetIsButton;

	if (cCurrentTabObjPtr != NULL)
	{
		widgetIsButton	=	cCurrentTabObjPtr->IsWidgetButton(widgetIdx);
	}
	else
	{
		widgetIsButton	=	false;
		CONSOLE_DEBUG("cCurrentTabObjPtr is NULL");
	}
	return(widgetIsButton);
}

//*****************************************************************************
bool	Controller::IsWidgetTextInput(const int widgetIdx)
{
bool		widgetIsButton;

	if (cCurrentTabObjPtr != NULL)
	{
		widgetIsButton	=	cCurrentTabObjPtr->IsWidgetTextInput(widgetIdx);
	}
	else
	{
		widgetIsButton	=	false;
		CONSOLE_DEBUG("cCurrentTabObjPtr is NULL");
	}
	return(widgetIsButton);
}

//*****************************************************************************
void	Controller::ProcessButtonClick(const int buttonIdx)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCurrentTabObjPtr != NULL)
	{
		cCurrentTabObjPtr->ProcessButtonClick(buttonIdx);
	//	cUpdateWindow	=	true;
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Un-handled button click, tab=\t",	cCurrentTabNum);
		CONSOLE_DEBUG_W_NUM("buttonIdx=\t",	buttonIdx);
	}
}

//*****************************************************************************
void	Controller::ProcessDoubleClick(const int buttonIdx)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCurrentTabObjPtr != NULL)
	{
		cCurrentTabObjPtr->ProcessDoubleClick(buttonIdx);
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Un-handled DOUBLE click, tab=\t",	cCurrentTabNum);
		CONSOLE_DEBUG_W_NUM("buttonIdx=\t",	buttonIdx);
	}
}

//*****************************************************************************
void	Controller::DisplayButtonHelpText(const int buttonIdx)
{
bool	updateOccurred;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCurrentTabObjPtr != NULL)
	{
		updateOccurred	=	cCurrentTabObjPtr->DisplayButtonHelpText(buttonIdx);
		if (updateOccurred)
		{
		//	cUpdateWindow	=	true;
			UpdateWindowAsNeeded();
		}
	}
}


//*****************************************************************************
void	Controller::ProcessMouseEvent(int event, int xxx, int yyy, int flags)
{
int		clickedBtn;
int		myWidgitIdx;
bool	widgitIsButton;

//	CONSOLE_DEBUG(__FUNCTION__);
	myWidgitIdx	=	FindClickedWidget(xxx,  yyy);
	switch(event)
	{
		case CV_EVENT_MOUSEMOVE:
		//	CONSOLE_DEBUG("CV_EVENT_MOUSEMOVE");
			cCurrentMouseX	=	xxx;
			cCurrentMouseY	=	yyy;
			if (cLeftButtonDown)
			{
				if (cHighlightedBtn >= 0)
				{
					//*	if we are no longer in the button, turn off highlighting
					if (myWidgitIdx != cHighlightedBtn)
					{
						SetWidgetHighlighted(cCurrentTabNum, cHighlightedBtn, false);
						DrawOneWidget(cHighlightedBtn);
						cHighlightedBtn	=	-1;
					}
				}
				else if ((cLastClicked_Btn >= 0) && (myWidgitIdx == cLastClicked_Btn))
				{
					//*	we are back in the clicked button, highlight it again
					cHighlightedBtn	=	cLastClicked_Btn;
					//*	highlight the button
					SetWidgetHighlighted(cCurrentTabNum, cHighlightedBtn, true);
					DrawOneWidget(cHighlightedBtn);
				}
				else if (cCurrentTabObjPtr != NULL)
				{
					cCurrentTabObjPtr->ProcessMouseLeftButtonDragged(myWidgitIdx, event,  xxx,  yyy,  flags);
				}
			}
			else
			{
				//*	this will allow the display of help text
				myWidgitIdx	=	FindClickedWidget(xxx,  yyy);
				if (myWidgitIdx >= 0)
				{
					DisplayButtonHelpText(myWidgitIdx);
				}
			}
			break;

		case CV_EVENT_LBUTTONDOWN:
//			CONSOLE_DEBUG("CV_EVENT_LBUTTONDOWN");
			cCurTextInput_Widget	=	-1;
			cLeftButtonDown			=	true;
			cLastLClickX			=	xxx;
			cLastLClickY			=	yyy;
			//*	keep track of the what button/tab was clicked on
			cLastClicked_Tab		=	FindClickedTab(xxx,  yyy);
			cLastClicked_Btn		=	FindClickedWidget(xxx,  yyy);
			if (cLastClicked_Btn >= 0)
			{
				widgitIsButton	=	IsWidgetButton(cLastClicked_Btn);
				if (widgitIsButton)
				{
	//				CONSOLE_DEBUG("Highlighting button");
					cHighlightedBtn	=	cLastClicked_Btn;
					//*	highlight the button
					SetWidgetHighlighted(cCurrentTabNum, cHighlightedBtn, true);
					DrawOneWidget(cHighlightedBtn);
				}
				else if (IsWidgetTextInput(cLastClicked_Btn))
				{
					cCurTextInput_Widget	=	cLastClicked_Btn;
					cLastClicked_Btn		=	-1;
//					CONSOLE_DEBUG("Text input!!!!!");
				}
				else
				{
					cLastClicked_Btn	=	-1;
//					CONSOLE_DEBUG("Not a button or text input");
				}

				if (cCurrentTabObjPtr != NULL)
				{
					cCurrentTabObjPtr->ProcessMouseLeftButtonDown(myWidgitIdx, event,  xxx,  yyy,  flags);
				}
			}
			break;

		case CV_EVENT_RBUTTONDOWN:
			CONSOLE_DEBUG("CV_EVENT_RBUTTONDOWN");
			break;

		case CV_EVENT_MBUTTONDOWN:
			CONSOLE_DEBUG("CV_EVENT_MBUTTONDOWN");
			break;

		case CV_EVENT_LBUTTONUP:
//			CONSOLE_DEBUG("CV_EVENT_LBUTTONUP");
			cLeftButtonDown	=	false;
			if (cHighlightedBtn >= 0)
			{
//				CONSOLE_DEBUG("Turning OFF highlight");
				//*	turn off button highlite
				SetWidgetHighlighted(cCurrentTabNum, cHighlightedBtn, false);
				DrawOneWidget(cHighlightedBtn);
				cHighlightedBtn	=	-1;
			}
			//*	check for tab click first
			clickedBtn		=	FindClickedTab(xxx,  yyy);
			if (clickedBtn >= 0)
			{
				if (clickedBtn == cLastClicked_Tab)
				{
					ProcessTabClick(clickedBtn);
				}
			}
			else
			{
				//*	now check for button (widget) click
				clickedBtn		=	FindClickedWidget(xxx,  yyy);
				if ((clickedBtn >= 0) && (clickedBtn == cLastClicked_Btn))
				{
					ProcessButtonClick(clickedBtn);
				}
			}
			cLastClicked_Btn	=	-1;


			if (cCurrentTabObjPtr != NULL)
			{
				cCurrentTabObjPtr->ProcessMouseLeftButtonUp(myWidgitIdx, event,  xxx,  yyy,  flags);
			}
			break;

		case CV_EVENT_RBUTTONUP:
			cRightButtonDown	=	false;
			CONSOLE_DEBUG("CV_EVENT_RBUTTONUP");
			break;

		case CV_EVENT_MBUTTONUP:
			CONSOLE_DEBUG("CV_EVENT_MBUTTONUP");
			break;

		case CV_EVENT_LBUTTONDBLCLK:
		//	CONSOLE_DEBUG("CV_EVENT_LBUTTONDBLCLK");
			clickedBtn		=	FindClickedTab(xxx,  yyy);
			if (clickedBtn >= 0)
			{
				//*	we have a double click in the tab bar
				cvResizeWindow(cWindowName, cWidth, cHeight);
			}
			else
			{
				clickedBtn		=	FindClickedWidget(xxx,  yyy);
				if (clickedBtn >= 0)
				{
					ProcessDoubleClick(clickedBtn);
				}
			}
			break;

		case CV_EVENT_RBUTTONDBLCLK:
			CONSOLE_DEBUG("CV_EVENT_RBUTTONDBLCLK");
			break;

		case CV_EVENT_MBUTTONDBLCLK:
			CONSOLE_DEBUG("CV_EVENT_MBUTTONDBLCLK");
			break;
	}

	//*	the window tab may also want to deal with a mouse event
	if (cCurrentTabObjPtr != NULL)
	{
		cCurrentTabObjPtr->ProcessMouseEvent(myWidgitIdx, event,  xxx,  yyy,  flags);
	}
}


//**************************************************************************************
void	Controller::DrawWidgetButton(TYPE_WIDGET *theWidget)
{
CvRect		myCVrect;
CvPoint		textLoc;
CvSize		textSize;
int			baseLine;
int			textOffsetX;
int			textOffsetY;
int			curFontNum;
CvScalar	myGBcolor;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, theWidget->textString);
	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;


	myGBcolor	=	theWidget->bgColor;
	if (theWidget->highLighted)
	{
//		CONSOLE_DEBUG("Button is highlighted");
		myGBcolor.val[0]	*=	0.75;
		myGBcolor.val[1]	*=	0.75;
		myGBcolor.val[2]	*=	0.75;
		myGBcolor.val[3]	*=	0.75;
	}
	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					myGBcolor,					//	CvScalar color,
					CV_FILLED,					//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));

	if (theWidget->includeBorder)
	{
		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						theWidget->borderColor,	//	CvScalar color,
						1,							//	int thickness CV_DEFAULT(1),
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));


	}
	if (theWidget->selected)
	{
		myCVrect.x		+=	3;
		myCVrect.y		+=	3;
		myCVrect.width	-=	6;
		myCVrect.height	-=	6;
		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						theWidget->borderColor,	//	CvScalar color,
						CV_FILLED,						//	int thickness CV_DEFAULT(1),
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));
	}
	if (strlen(theWidget->textString) > 0)
	{
		curFontNum	=	theWidget->fontNum,
		cvGetTextSize(	theWidget->textString,
						&gTextFont[curFontNum],
						&textSize,
						&baseLine);

		switch (theWidget->justification)
		{
			case kJustification_Left:
				textLoc.x	=	theWidget->left + 7;
				textOffsetY	=	(theWidget->height / 2) - (textSize.height / 2) + baseLine + 5;
				textLoc.y	=	theWidget->top + textOffsetY;
				break;

			case kJustification_Center:
			case kJustification_Right:
			default:
				textOffsetX	=	(theWidget->width / 2) - (textSize.width / 2);
				textOffsetY	=	(theWidget->height / 2) - (textSize.height / 2) + baseLine + 5;

				textLoc.x	=	theWidget->left + textOffsetX;
				textLoc.y	=	theWidget->top + textOffsetY;
				break;

		}
		if (theWidget->highLighted)
		{
			textLoc.x	+=	2;
			textLoc.y	+=	2;
		}

		cvPutText(	cOpenCV_Image,
					theWidget->textString,
					textLoc,
					&gTextFont[curFontNum],
					theWidget->textColor
					);
	}
}

//**************************************************************************************
void	Controller::DrawWidgetText(TYPE_WIDGET *theWidget)
{
CvRect		myCVrect;
CvPoint		textLoc;
CvSize		textSize;
int			baseLine;
int			textOffsetX;
int			textOffsetY;
int			curFontNum;

	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;


	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					theWidget->bgColor,			//	CvScalar color,
					CV_FILLED,					//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));

	if (theWidget->includeBorder)
	{
		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						theWidget->borderColor,		//	CvScalar color,
						1,							//	int thickness CV_DEFAULT(1),
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));
	}
	if (strlen(theWidget->textString) > 0)
	{
		curFontNum	=	theWidget->fontNum,
		cvGetTextSize(	theWidget->textString,
						&gTextFont[curFontNum],
						&textSize,
						&baseLine);
		switch (theWidget->justification)
		{
			case kJustification_Left:
				textLoc.x	=	theWidget->left + 7;
				textOffsetY	=	(theWidget->height / 2) - (textSize.height / 2) + baseLine + 5;
				textLoc.y	=	theWidget->top + textOffsetY;
				break;

			case kJustification_Center:
			case kJustification_Right:
			default:
				textOffsetX	=	(theWidget->width / 2) - (textSize.width / 2);
				textOffsetY	=	(theWidget->height / 2) - (textSize.height / 2) + baseLine + 5;

				textLoc.x	=	theWidget->left + textOffsetX;
				textLoc.y	=	theWidget->top + textOffsetY;
				break;

		}
		cvPutText(	cOpenCV_Image,
					theWidget->textString,
					textLoc,
					&gTextFont[curFontNum],
					theWidget->textColor
					);
	}
}

//**************************************************************************************
void	Controller::DrawWidgetMultiLineText(TYPE_WIDGET *theWidget)
{
CvRect		myCVrect;
CvPoint		textLoc;
CvSize		textSize;
int			baseLine;
int			curFontNum;
char		lineBuff[128];
int			iii;
int			ccc;
int			sLen;
int			textOffsetX;
bool		drawTextFlg;

//	CONSOLE_DEBUG(__FUNCTION__);

	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;

	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					theWidget->bgColor,			//	CvScalar color,
					CV_FILLED,					//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));

	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					theWidget->borderColor,		//	CvScalar color,
					1,							//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));

	if (strlen(theWidget->textString) > 0)
	{
		curFontNum	=	theWidget->fontNum,
		cvGetTextSize(	theWidget->textString,
						&gTextFont[curFontNum],
						&textSize,
						&baseLine);

		textLoc.x	=	theWidget->left + 10;
		textLoc.y	=	theWidget->top + textSize.height + baseLine;

		ccc			=	0;
		sLen		=	strlen(theWidget->textString);
		drawTextFlg	=	false;
		curFontNum	=	theWidget->fontNum;
		for (iii=0; iii<=sLen; iii++)
		{
			if (theWidget->textString[iii] > 0x20)
			{
				lineBuff[ccc++]	=	theWidget->textString[iii];
				lineBuff[ccc]	=	0;
			}
			else if (theWidget->textString[iii] == 0x20)
			{
				lineBuff[ccc++]	=	theWidget->textString[iii];
				lineBuff[ccc]	=	0;
				cvGetTextSize(	lineBuff,
								&gTextFont[curFontNum],
								&textSize,
								&baseLine);
				if (textSize.width > (theWidget->width - 70))
				{
					drawTextFlg	=	true;
				}
			}
			else
			{
				drawTextFlg	=	true;
			}

			if (drawTextFlg)
			{
				cvGetTextSize(	lineBuff,
								&gTextFont[curFontNum],
								&textSize,
								&baseLine);
				//*	this allows for blank lines
				if (strlen(lineBuff) > 0)
				{
					switch (theWidget->justification)
					{
						case kJustification_Left:
							textLoc.x	=	theWidget->left + 7;
							break;

						case kJustification_Center:
						case kJustification_Right:
						default:
							textOffsetX	=	(theWidget->width / 2) - (textSize.width / 2);
							textLoc.x	=	theWidget->left + textOffsetX;
							break;

					}
					cvPutText(	cOpenCV_Image,
								lineBuff,
								textLoc,
								&gTextFont[curFontNum],
								theWidget->textColor);
				}
				ccc				=	0;
				lineBuff[ccc]	=	0;
				textLoc.y		+=	textSize.height;
				textLoc.y		+=	baseLine;
				drawTextFlg		=	false;
			}
		}
	}
}

//**************************************************************************************
void	Controller::DrawWidgetRadioButton(TYPE_WIDGET *theWidget)
{
CvRect		myCVrect;
CvPoint		pointLoc;
CvPoint		textLoc;
CvSize		textSize;
int			baseLine;
int			textOffsetY;
int			curFontNum;
int			radius;

	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;


	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					theWidget->bgColor,			//	CvScalar color,
					CV_FILLED,					//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));

	if (theWidget->includeBorder)
	{
		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						theWidget->borderColor,		//	CvScalar color,
						1,							//	int thickness CV_DEFAULT(1),
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));

	}
	//*	draw the radio button
	pointLoc.x		=	theWidget->left + (theWidget->height / 2);
	pointLoc.y		=	theWidget->top + (theWidget->height / 2);
	radius			=	theWidget->height / 3;
	cvCircle(		cOpenCV_Image,
					pointLoc,					//	CvPoint center,
					radius,						//	int radius,
//					CV_RGB(255, 255, 255),		//	CvScalar color, int thickness CV_DEFAULT(1),
					theWidget->borderColor,		//	CvScalar color,
					1,							//	int thickness
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));
	//*	check to see if its selected
	if (theWidget->selected)
	{
		cvCircle(		cOpenCV_Image,
						pointLoc,					//	CvPoint center,
						radius - 2,					//	int radius,
//						CV_RGB(255, 255, 255),		//	CvScalar color, int thickness CV_DEFAULT(1),
						theWidget->borderColor,		//	CvScalar color,
						CV_FILLED,					//	int thickness
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));
	}

	if (strlen(theWidget->textString) > 0)
	{
		curFontNum	=	theWidget->fontNum,
		cvGetTextSize(	theWidget->textString,
						&gTextFont[curFontNum],
						&textSize,
						&baseLine);
		textLoc.x	=	theWidget->left + (2 * radius) + 8;
		textOffsetY	=	(theWidget->height / 2) - (textSize.height / 2) + baseLine + 5;
		textLoc.y	=	theWidget->top + textOffsetY;

		cvPutText(	cOpenCV_Image,
					theWidget->textString,
					textLoc,
					&gTextFont[curFontNum],
					theWidget->textColor
					);
	}
}

//**************************************************************************************
void	Controller::DrawWidgetCheckBox(TYPE_WIDGET *theWidget)
{
CvRect		myCVrect;
CvRect		checkBoxRect;
CvPoint		textLoc;
CvPoint		pt1;
CvPoint		pt2;
CvSize		textSize;
int			baseLine;
int			textOffsetY;
int			curFontNum;

	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;


	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					theWidget->bgColor,			//	CvScalar color,
					CV_FILLED,					//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));

	if (theWidget->includeBorder)
	{
//		CONSOLE_DEBUG("includeBorder");
		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						theWidget->borderColor,		//	CvScalar color,
						1,							//	int thickness CV_DEFAULT(1),
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));

	}
	//*	draw the Check box
	checkBoxRect.x		=	theWidget->left;
	checkBoxRect.y		=	theWidget->top;
	checkBoxRect.width	=	theWidget->height;
	checkBoxRect.height	=	theWidget->height;

	cvRectangleR(	cOpenCV_Image,
					checkBoxRect,
					CV_RGB(128, 128, 128),
					CV_FILLED,					//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));



	//*	check to see if its selected
	if (theWidget->selected)
	{
	#define	kInset	2
		pt1.x	=	checkBoxRect.x + kInset;
		pt1.y	=	checkBoxRect.y + kInset;

		pt2.x	=	checkBoxRect.x + checkBoxRect.width		- kInset;
		pt2.y	=	checkBoxRect.y + checkBoxRect.height	- kInset;
		cvLine(	cOpenCV_Image,
				pt1,
				pt2,
				theWidget->borderColor,		//	CvScalar color,
			//	CV_RGB(255, 0, 0),
				2,							//	int thickness CV_DEFAULT(1),
				8,							//	int line_type CV_DEFAULT(8),
				0);							//	int shift CV_DEFAULT(0));

		pt1.x	=	checkBoxRect.x + checkBoxRect.width	- kInset;
		pt1.y	=	checkBoxRect.y 						+ kInset;

		pt2.x	=	checkBoxRect.x			 				+ kInset;
		pt2.y	=	checkBoxRect.y + checkBoxRect.height	- kInset;
		cvLine(	cOpenCV_Image,
				pt1,
				pt2,
				theWidget->borderColor,		//	CvScalar color,
			//	CV_RGB(255, 0, 0),
				2,							//	int thickness CV_DEFAULT(1),
				8,							//	int line_type CV_DEFAULT(8),
				0);							//	int shift CV_DEFAULT(0));
	}
	//*	draw the border AFTER the X mark
	cvRectangleR(	cOpenCV_Image,
					checkBoxRect,
					theWidget->borderColor,		//	CvScalar color,
					1,							//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));

	if (strlen(theWidget->textString) > 0)
	{
		curFontNum	=	theWidget->fontNum,
		cvGetTextSize(	theWidget->textString,
						&gTextFont[curFontNum],
						&textSize,
						&baseLine);
		textLoc.x	=	theWidget->left + theWidget->height + 8;
		textOffsetY	=	(theWidget->height / 2) - (textSize.height / 2) + baseLine + 5;
		textLoc.y	=	theWidget->top + textOffsetY;

		cvPutText(	cOpenCV_Image,
					theWidget->textString,
					textLoc,
					&gTextFont[curFontNum],
					theWidget->textColor
					);
	}
}

//**************************************************************************************
void	Controller::DrawWidgetSlider(TYPE_WIDGET *theWidget)
{
CvRect		myCVrect;
CvPoint		textLoc;
CvSize		textSize;
int			baseLine;
int			textOffsetY;
int			curFontNum;
double		sliderDelata;
char		leftString[32];
char		rightString[32];
int			vertCenter;
int			radius;
int			sliderOffset;

	sliderDelata	=	theWidget->sliderMax - theWidget->sliderMin;
	if (sliderDelata >= 10)
	{
		sprintf(leftString,		"%1.0f",	theWidget->sliderMin);
		sprintf(rightString,	"%1.0f",	theWidget->sliderMax);
	}
	else if (sliderDelata >= 1)
	{
		sprintf(leftString,		"%1.1f",	theWidget->sliderMin);
		sprintf(rightString,	"%1.1f",	theWidget->sliderMax);
	}
	else
	{
		sprintf(leftString,		"%1.3f",	theWidget->sliderMin);
		sprintf(rightString,	"%1.3f",	theWidget->sliderMax);
	}
	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;


	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					theWidget->bgColor,			//	CvScalar color,
					CV_FILLED,					//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));
	if (theWidget->includeBorder)
	{
		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						theWidget->borderColor,		//	CvScalar color,
						1,							//	int thickness CV_DEFAULT(1),
						8,							//	int line_type CV_DEFAULT(8),
						0);							//	int shift CV_DEFAULT(0));
	}
	curFontNum	=	theWidget->fontNum,
	cvGetTextSize(	rightString,
					&gTextFont[curFontNum],
					&textSize,
					&baseLine);

	textLoc.x	=	theWidget->left + 7;
	textOffsetY	=	(theWidget->height / 2) - (textSize.height / 2) + baseLine + 5;
	textLoc.y	=	theWidget->top + textOffsetY;

	cvPutText(	cOpenCV_Image,
				leftString,
				textLoc,
				&gTextFont[curFontNum],
				theWidget->textColor
				);

	textOffsetY	=	(theWidget->height / 2) - (textSize.height / 2) + baseLine + 5;

	textLoc.x	=	(theWidget->left + theWidget->width)- textSize.width;
	textLoc.x	-=	5;
	textLoc.y	=	theWidget->top + textOffsetY;

	cvPutText(	cOpenCV_Image,
				rightString,
				textLoc,
				&gTextFont[curFontNum],
				theWidget->textColor
				);

	//*	now draw the slider itself
	vertCenter		=	theWidget->top + (theWidget->height / 2);
	myCVrect.x		=	theWidget->left + 50;
	myCVrect.y		=	vertCenter - 1;
	myCVrect.width	=	theWidget->width - 100;
	myCVrect.height	=	3;


	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					theWidget->textColor,		//	CvScalar color,
				//	CV_FILLED,					//	int thickness CV_DEFAULT(1),
					1,							//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));
	//*	draw the indicator
	sliderOffset	=	(theWidget->sliderValue / sliderDelata) * myCVrect.width;
	textLoc.x		=	myCVrect.x + sliderOffset;
	textLoc.y		=	vertCenter;
	radius			=	4;

	cvCircle(		cOpenCV_Image,
					textLoc,					//	CvPoint center,
					radius,						//	int radius,
					theWidget->textColor,		//	CvScalar color, int thickness CV_DEFAULT(1),
					CV_FILLED,					//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));
}


//**************************************************************************************
void	Controller::DrawWidgetOutlineBox(TYPE_WIDGET *theWidget)
{
CvRect		myCVrect;

	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;



	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					theWidget->borderColor,		//	CvScalar color,
					1,							//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));

	if (theWidget->crossedOut)
	{
	CvPoint	pt1;
	CvPoint	pt2;

		pt1.x	=	theWidget->left;
		pt1.y	=	theWidget->top;

		pt2.x	=	theWidget->left + theWidget->width;
		pt2.y	=	theWidget->top + theWidget->height;
		cvLine(	cOpenCV_Image,
				pt1,
				pt2,
				CV_RGB(255,	0,	0),	//	CvScalar color,
				2,					//	int thickness CV_DEFAULT(1),
				8,					//	int line_type CV_DEFAULT(8),
				0);					//	int shift CV_DEFAULT(0));

		pt1.x	=	theWidget->left + theWidget->width;
		pt1.y	=	theWidget->top;

		pt2.x	=	theWidget->left;
		pt2.y	=	theWidget->top + theWidget->height;
		cvLine(	cOpenCV_Image,
				pt1,
				pt2,
				CV_RGB(255,	0,	0),	//	CvScalar color,
				2,					//	int thickness CV_DEFAULT(1),
				8,					//	int line_type CV_DEFAULT(8),
				0);					//	int shift CV_DEFAULT(0));
	}
}

//**************************************************************************************
void	Controller::DrawWidgetGraph(TYPE_WIDGET *theWidget)
{
CvPoint	pt1;
CvPoint	pt2;
int		iii;

	if (theWidget->graphArrayPtr != NULL)
	{
		for (iii=0; iii<theWidget->graphArrayCnt; iii++)
		{
			pt1.x	=	iii + 3;
			pt1.y	=	theWidget->top + theWidget->height;

			pt2.x	=	iii + 3;
			pt2.y	=	pt1.y - (theWidget->graphArrayPtr[iii] * 3);
			cvLine(	cOpenCV_Image,
					pt1,
					pt2,
					CV_RGB(255,	0,	0),	//	CvScalar color,
					1,					//	int thickness CV_DEFAULT(1),
					8,					//	int line_type CV_DEFAULT(8),
					0);					//	int shift CV_DEFAULT(0));

		}
	}
}



//**************************************************************************************
void	Controller::DrawWidgetIcon(TYPE_WIDGET *theWidget)
{
CvPoint		pt1;
CvPoint		pt2;
CvRect		myCVrect;

	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;


	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					theWidget->bgColor,			//	CvScalar color,
					CV_FILLED,					//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));

//	if (theWidget->includeBorder)
//	{
//		cvRectangleR(	cOpenCV_Image,
//						myCVrect,
//						theWidget->borderColor,		//	CvScalar color,
//						1,							//	int thickness CV_DEFAULT(1),
//						8,							//	int line_type CV_DEFAULT(8),
//						0);							//	int shift CV_DEFAULT(0));
//	}



	switch(theWidget->iconNum)
	{
		case kIcon_UpArrow:
			pt1.x	=	theWidget->left + (theWidget->width / 2);
			pt1.y	=	theWidget->top + 1;

			pt2.x	=	theWidget->left + (theWidget->width / 2);
			pt2.y	=	theWidget->top + theWidget->height;
			cvLine(	cOpenCV_Image,
					pt1,
					pt2,
					theWidget->textColor,	//	CvScalar color,
					2,						//	int thickness CV_DEFAULT(1),
					8,						//	int line_type CV_DEFAULT(8),
					0);						//	int shift CV_DEFAULT(0));


			pt2.x	=	theWidget->left;
			pt2.y	=	theWidget->top + (theWidget->width / 2);
			cvLine(	cOpenCV_Image,
					pt1,
					pt2,
					theWidget->textColor,	//	CvScalar color,
					1,						//	int thickness CV_DEFAULT(1),
					8,						//	int line_type CV_DEFAULT(8),
					0);						//	int shift CV_DEFAULT(0));

			pt2.x	=	theWidget->left + theWidget->width;
			pt2.y	=	theWidget->top + (theWidget->width / 2);
			cvLine(	cOpenCV_Image,
					pt1,
					pt2,
					theWidget->textColor,	//	CvScalar color,
					1,						//	int thickness CV_DEFAULT(1),
					8,						//	int line_type CV_DEFAULT(8),
					0);						//	int shift CV_DEFAULT(0));

			break;

		case kIcon_DownArrow:
			pt1.x	=	theWidget->left + (theWidget->width / 2);
			pt1.y	=	theWidget->top +1;

			pt2.x	=	theWidget->left + (theWidget->width / 2);
			pt2.y	=	theWidget->top + theWidget->height - 1;
			cvLine(	cOpenCV_Image,
					pt1,
					pt2,
					theWidget->textColor,	//	CvScalar color,
					2,						//	int thickness CV_DEFAULT(1),
					8,						//	int line_type CV_DEFAULT(8),
					0);						//	int shift CV_DEFAULT(0));


			pt1.x	=	theWidget->left + (theWidget->width / 2);
			pt1.y	=	theWidget->top + theWidget->height - 2;

			pt2.x	=	theWidget->left;
			pt2.y	=	theWidget->top + (theWidget->width / 2);
			cvLine(	cOpenCV_Image,
					pt1,
					pt2,
					theWidget->textColor,	//	CvScalar color,
					1,						//	int thickness CV_DEFAULT(1),
					8,						//	int line_type CV_DEFAULT(8),
					0);						//	int shift CV_DEFAULT(0));

			pt2.x	=	theWidget->left + theWidget->width;
			pt2.y	=	theWidget->top + (theWidget->width / 2);
			cvLine(	cOpenCV_Image,
					pt1,
					pt2,
					theWidget->textColor,	//	CvScalar color,
					1,						//	int thickness CV_DEFAULT(1),
					8,						//	int line_type CV_DEFAULT(8),
					0);						//	int shift CV_DEFAULT(0));

			break;

	}
}

//**************************************************************************************
void	Controller::DrawWidgetCustom(TYPE_WIDGET *theWidget)
{
	CONSOLE_DEBUG("this routine should be overloaded");
	//*	this routine should be overloaded
}

//**************************************************************************************
void	Controller::DrawWidgetProgressBar(TYPE_WIDGET *theWidget)
{
CvRect		myCVrect;
CvPoint		textLoc;
CvSize		textSize;
int			baseLine;
int			textOffsetX;
int			textOffsetY;
int			curFontNum;
char		testString[64];
double		percentComplete;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	fill in with background color
	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;
	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					theWidget->bgColor,			//	CvScalar color,
					CV_FILLED,					//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));


	percentComplete	=	theWidget->sliderValue / theWidget->sliderMax;
	myCVrect.width	=	theWidget->width * percentComplete;
	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					theWidget->textColor,		//	CvScalar color,
					CV_FILLED,					//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));

	//*	draw the outline
	myCVrect.width	=	theWidget->width;
	cvRectangleR(	cOpenCV_Image,
					myCVrect,
					theWidget->borderColor,		//	CvScalar color,
					1,							//	int thickness CV_DEFAULT(1),
					8,							//	int line_type CV_DEFAULT(8),
					0);							//	int shift CV_DEFAULT(0));

	sprintf(testString, "Downloading - %4.1f%% complete", percentComplete * 100);

	curFontNum	=	theWidget->fontNum,
	cvGetTextSize(	testString,
					&gTextFont[curFontNum],
					&textSize,
					&baseLine);


	textOffsetX	=	(theWidget->width / 2) - (textSize.width / 2);
	textOffsetY	=	(theWidget->height / 2) - (textSize.height / 2) + baseLine + 5;

	textLoc.x	=	theWidget->left + textOffsetX;
	textLoc.y	=	theWidget->top + textOffsetY;

	cvPutText(	cOpenCV_Image,
				testString,
				textLoc,
				&gTextFont[curFontNum],
			//	CV_RGB(255,	0,		255)
				CV_RGB(255,	255,	255)
				);
}



//**************************************************************************************
void	Controller::DrawWindowTabs(void)
{
int				iii;

	//*	draw the tabs
	if (cTabCount > 0)
	{
		//*	first set the bg colors
		for (iii=0; iii<cTabCount; iii++)
		{
			if (iii == cCurrentTabNum)
			{
				cTabList[iii].bgColor	=	CV_RGB(220,	220,	220);
			}
			else
			{
				cTabList[iii].bgColor	=	CV_RGB(128,	128,	128);
			}
		}


		for (iii=0; iii<cTabCount; iii++)
		{
			if (cTabList[iii].valid)
			{
				DrawWidgetText(&cTabList[iii]);
			}
		}
	}
}

//**************************************************************************************
//*	this will re-draw one widget in the CURRENT tab
void	Controller::DrawOneWidget(const int widgetIdx)
{
TYPE_WIDGET		*myWidgetPtr;

	if (cCurrentTabObjPtr != NULL)
	{
		myWidgetPtr	=	cCurrentTabObjPtr->cWidgetList;
		if (myWidgetPtr != NULL)
		{
			DrawOneWidget(&myWidgetPtr[widgetIdx], widgetIdx);
			cvShowImage(cWindowName, cOpenCV_Image);
		}
	}
}

//**************************************************************************************
void	Controller::DrawOneWidget(TYPE_WIDGET *widgetPtr, const int widgetIdx)
{
CvRect		widgetRect;

	widgetPtr->needsUpdated	=	false;	//*	record the fact that this widget has been updated
	switch(widgetPtr->widgetType)
	{
		case kWidgetType_Button:
			DrawWidgetButton(widgetPtr);
			break;

		case kWidgetType_Graph:
			DrawWidgetGraph(widgetPtr);
			if (widgetPtr->includeBorder)
			{
				widgetRect.x		=	widgetPtr->left;
				widgetRect.y		=	widgetPtr->top;
				widgetRect.width	=	widgetPtr->width;
				widgetRect.height	=	widgetPtr->height;
				cvRectangleR(	cOpenCV_Image,
								widgetRect,
								widgetPtr->borderColor,		//	CvScalar color,
								1,							//	int thickness CV_DEFAULT(1),
								8,							//	int line_type CV_DEFAULT(8),
								0);							//	int shift CV_DEFAULT(0));
			}
			break;

		case kWidgetType_Graphic:
			if (cCurrentTabObjPtr != NULL)
			{
				cCurrentTabObjPtr->DrawGraphWidget(cOpenCV_Image, widgetIdx);
			}
			else
			{
				CONSOLE_DEBUG("cCurrentTabObjPtr is NULL");
			}
			break;

		case kWidgetType_Image:
			if (widgetPtr->openCVimagePtr != NULL)
			{
			CvRect		roiRect;
			int			delta;
			IplImage	*myImage;

			//	CONSOLE_DEBUG_W_NUM("Drawing image, widget#", widgetIdx);
				myImage				=	widgetPtr->openCVimagePtr;
				widgetRect.x		=	widgetPtr->left;
				widgetRect.y		=	widgetPtr->top;
				widgetRect.width	=	widgetPtr->width;
				widgetRect.height	=	widgetPtr->height;

				roiRect				=	widgetRect;
				//*	we have to make sure the the destination rect is the same as the src rect
				//*	if its smaller, then center it
				if (myImage->width < widgetRect.width)
				{
					delta		=	widgetRect.width - myImage->width;
					roiRect.x	=	widgetRect.x + (delta / 2);
				}
				if (myImage->height < widgetRect.height)
				{
					delta		=	widgetRect.height - myImage->height;
					roiRect.y	=	widgetRect.y + (delta / 2);
				}
				roiRect.width	=	myImage->width;
				roiRect.height	=	myImage->height;

				cvSetImageROI(cOpenCV_Image,  roiRect);
				cvCopy(widgetPtr->openCVimagePtr, cOpenCV_Image);
				cvResetImageROI(cOpenCV_Image);

				//*	draw the border if enabled
				if (widgetPtr->includeBorder)
				{

					cvRectangleR(	cOpenCV_Image,
									roiRect,
									widgetPtr->borderColor,		//	CvScalar color,
									1,							//	int thickness CV_DEFAULT(1),
									8,							//	int line_type CV_DEFAULT(8),
									0);							//	int shift CV_DEFAULT(0));
				}

			}
			else
			{
			//	CONSOLE_DEBUG("Image ptr is null");
				if (widgetPtr->includeBorder)
				{
					widgetRect.x		=	widgetPtr->left;
					widgetRect.y		=	widgetPtr->top;
					widgetRect.width	=	widgetPtr->width;
					widgetRect.height	=	widgetPtr->height;

					cvRectangleR(	cOpenCV_Image,
									widgetRect,
									widgetPtr->borderColor,		//	CvScalar color,
									1,							//	int thickness CV_DEFAULT(1),
									8,							//	int line_type CV_DEFAULT(8),
									0);							//	int shift CV_DEFAULT(0));
				}
			}
			break;

		case kWidgetType_MultiLineText:
			DrawWidgetMultiLineText(widgetPtr);
			break;

		case kWidgetType_RadioButton:
			DrawWidgetRadioButton(widgetPtr);
			break;

		case kWidgetType_CheckBox:
			DrawWidgetCheckBox(widgetPtr);
			break;

		case kWidgetType_Slider:
			DrawWidgetSlider(widgetPtr);
			break;

		case kWidgetType_OutlineBox:
		case kWidgetType_ScrollBar:
			DrawWidgetOutlineBox(widgetPtr);
			break;

		case kWidgetType_Custom:
//			CONSOLE_DEBUG("kWidgetType_Custom");
			DrawWidgetCustom(widgetPtr);
			break;


		case kWidgetType_Icon:
			DrawWidgetIcon(widgetPtr);
			break;

		case kWidgetType_ProessBar:
			DrawWidgetProgressBar(widgetPtr);
			break;

		case kWidgetType_Text:
		default:
			DrawWidgetText(widgetPtr);
			break;
	}
}


//**************************************************************************************
void	Controller::DrawWindowWidgets(void)
{
int				iii;
TYPE_WIDGET		*myWidgetPtr;


	if (cCurrentTabObjPtr != NULL)
	{
		myWidgetPtr	=	cCurrentTabObjPtr->cWidgetList;
	}
	else
	{
		CONSOLE_DEBUG(__FUNCTION__);
		CONSOLE_DEBUG("cCurrentTabObjPtr is NULL");
		myWidgetPtr	=	NULL;
	}

	if (myWidgetPtr != NULL)
	{
		//*	draw the widgets
		for (iii=0; iii<kMaxWidgets; iii++)
		{
			if (myWidgetPtr[iii].valid)
			{
				DrawOneWidget(&myWidgetPtr[iii], iii);
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("widget ptr is NULL");
	}
}


//**************************************************************************************
void	Controller::DrawWindow(void)
{
CvRect		myCVrect;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cOpenCV_Image != NULL)
	{
		myCVrect.x		=	0;
		myCVrect.y		=	0;
		myCVrect.width	=	cWidth;
		myCVrect.height	=	cHeight;
		cvRectangleR(	cOpenCV_Image,
						myCVrect,
						cBackGrndColor,			//	CvScalar color,
						CV_FILLED,				//	int thickness CV_DEFAULT(1),
						8,						//	int line_type CV_DEFAULT(8),
						0);						//	int shift CV_DEFAULT(0));

		DrawWindowTabs();

		//*	draw the widgets
		DrawWindowWidgets();
	}
}


//**************************************************************************************
void	Controller::HandleKeyDown(const int keyPressed)
{
int			openCVerr;
char		imageFileName[64];
int			quality[3] = {16, 200, 0};
bool		stillNeedsHandled;

//	CONSOLE_DEBUG_W_HEX("keyPressed\t=", keyPressed);

	stillNeedsHandled	=	true;

	//*	check for control key
	if (keyPressed & 0x040000)
	{
		//*	we have a control key
//		CONSOLE_DEBUG("Control is down");
		stillNeedsHandled	=	false;

		switch(tolower(keyPressed & 0x007f))
		{
			case 'r':
				RefreshWindow();
				break;

			case 's':
				CONSOLE_DEBUG("Save file");
				sprintf(imageFileName, "%s-screenshot.jpg", cWindowName);
				openCVerr	=	cvSaveImage(imageFileName, cOpenCV_Image, quality);
				CONSOLE_DEBUG_W_NUM("openCVerr\t=", openCVerr);
				break;

			case 'q':
//				CONSOLE_DEBUG_W_STR("Quit from  window \t=", cWindowName);
				gKeepRunning	=	false;
				break;

			case 'w':
//				CONSOLE_DEBUG_W_STR("Close  window \t=", cWindowName);
				cKeepRunning	=	false;
				break;

			default:
				stillNeedsHandled	=	true;
				break;
		}
	}
	if (stillNeedsHandled)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		if (cCurTextInput_Widget >= 0)
		{

			HandleKeyDownInTextWidget(cCurrentTabNum, cCurTextInput_Widget, keyPressed);

		}
		else if (cCurrentTabObjPtr != NULL)
		{
			cCurrentTabObjPtr->HandleKeyDown(keyPressed);
		}
	}
}

//**************************************************************************************
void	Controller::HandleKeyDownInTextWidget(const int tabNum, const int widgetIdx,const int keyPressed)
{
char	myTextString[512];
int		textLen;
int		myChar;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_HEX("keyPressed\t=", keyPressed);
//	CONSOLE_DEBUG_W_HEX("0x00ffe0  \t=", (keyPressed & 0x00ffe0));

	GetWidgetText(tabNum, widgetIdx, myTextString);
	textLen	=	strlen(myTextString);

	myChar	=	0;
	//*	check for control key down
	if (keyPressed & 0x040000)
	{
		myChar	=	keyPressed & 0x1f;
	}
	else if ((keyPressed & 0x00ffe0) == 0x00ffe0)
	{
		//	0x10FFE1
		//	0x00FFE0
		//*	ignore these
		myChar	=	0;
//		CONSOLE_DEBUG_W_HEX("myChar\t=", myChar);
	}
	else if ((keyPressed & 0x00ff80) == 0x0ff00)
	{
		myChar	=	keyPressed & 0x7f;
		if (myChar >= 0x20)
		{
			myChar	=	0;
		}
	}
	else
	{
		myChar	=	keyPressed & 0x7f;
	}

//	CONSOLE_DEBUG_W_HEX("myChar\t=", myChar);
	switch(myChar)
	{
		case 0x08:	//*	back space
			textLen--;
			if (textLen < 0)
			{
				textLen	=	0;
			}
			myTextString[textLen]	=	0;
			break;

		default:
			if (myChar >= 0x20)
			{
				myTextString[textLen]	=	myChar;
				myTextString[textLen+1]	=	0;
			}
	}

//	CONSOLE_DEBUG_W_STR("myTextString\t=", myTextString);

	SetWidgetText(tabNum, widgetIdx, myTextString);
}


//**************************************************************************************
void	Controller::RefreshWindow(void)
{
//*	do nothing, this is intended for a subclass to over ride if needed
}

#pragma mark -

//**************************************************************************************
//*	these routines handle multiple tabs
//**************************************************************************************

//**************************************************************************************
void	Controller::SetTabWindow(const int tabNum, WindowTab *theTabObjectPtr)
{
	if ((tabNum >= 0) && (tabNum < kMaxTabs))
	{
		cWindowTabs[tabNum]	=	theTabObjectPtr;
		//*	If the current tab is NOT set, set it tab 0
		if ((tabNum == 0) && (cCurrentTabObjPtr == NULL))
		{
			cCurrentTabObjPtr	=	theTabObjectPtr;
			cCurrentTabNum		=	tabNum;
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetValid(	const int tabNum, const int widgetIdx, bool valid)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, widgetIdx);
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetValid(widgetIdx, valid);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}


//**************************************************************************************
void	Controller::SetWidgetText(const int tabNum, const int widgetIdx, const char *newText)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("tabNum\t=",	tabNum);
//	CONSOLE_DEBUG_W_NUM("widgetIdx\t=",	widgetIdx);
//	CONSOLE_DEBUG_W_STR("newText\t=",	newText);
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetText(widgetIdx, newText);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
			CONSOLE_DEBUG("cWindowTabs[tabNum] is NULL")
		}
	}
	else
	{
		CONSOLE_DEBUG("tabNum is out of bounds")
	}
}

//**************************************************************************************
void	Controller::GetWidgetText(const int tabNum, const int widgetIdx, char *getText)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->GetWidgetText(widgetIdx, getText);
		}
		else
		{
			CONSOLE_DEBUG("cWindowTabs[tabNum] is NULL")
		}
	}
	else
	{
		CONSOLE_DEBUG("tabNum is out of bounds")
	}
}


//**************************************************************************************
void	Controller::SetWidgetNumber(const int tabNum, const int widgetIdx, const int number)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetNumber(widgetIdx, number);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetNumber(const int tabNum, const int widgetIdx, const double number)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetNumber(widgetIdx, number);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}


//**************************************************************************************
void	Controller::SetWidgetType(	const int tabNum, const int widgetIdx, const int widetType)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetType(widgetIdx, widetType);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetFont(const int tabNum, const int widgetIdx, int fontNum)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetFont(widgetIdx, fontNum);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetTextColor(		const int tabNum, const int widgetIdx, CvScalar newtextColor)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetTextColor(widgetIdx, newtextColor);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetBGColor(const int tabNum, const int widgetIdx, CvScalar newBGcolor)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetBGColor(widgetIdx, newBGcolor);
			if (tabNum == cCurrentTabNum)
			{
//				CONSOLE_DEBUG("update window")
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetBorderColor(	const int tabNum, const int widgetIdx, CvScalar newBorderColor)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetBorderColor(widgetIdx, newBorderColor);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}


//**************************************************************************************
void	Controller::SetWidgetImage(			const int tabNum, const int widgetIdx, IplImage *argImagePtr)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetImage(widgetIdx, argImagePtr);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}


//**************************************************************************************
void	Controller::SetWidgetChecked(		const int tabNum, const int widgetIdx, bool checked)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetChecked(widgetIdx, checked);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetCrossedout(const int tabNum, const int widgetIdx, bool crossedout)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetCrossedout(widgetIdx, crossedout);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetSliderLimits(	const int	tabNum,
											const int	widgetIdx,
											double		sliderMin,
											double		sliderMax)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetSliderLimits(widgetIdx, sliderMin, sliderMax);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetSliderValue(	const int	tabNum,
											const int	widgetIdx,
											double		sliderValue)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetSliderValue(widgetIdx, sliderValue);
			if (tabNum == cCurrentTabNum)
			{
				cUpdateWindow	=	true;
			}
		}
		else
		{
		}
	}
}


//**************************************************************************************
void	Controller::SetWidgetHighlighted(	const int	tabNum,
											const int	widgetIdx,
											bool		highlighted)
{
	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetHighlighted(widgetIdx, highlighted);
		}
		else
		{
		}
	}
}

//**************************************************************************************
void	Controller::SetWidgetProgress(	const int	tabNum,
										const int	widgetIdx,
										const int	currPosition,
										const int	totalValue)
{
TYPE_WIDGET		*myWidgetPtr;

//	CONSOLE_DEBUG_W_NUM("currPosition\t=", currPosition);
//	CONSOLE_DEBUG_W_NUM("totalValue\t=", totalValue);

	if ((tabNum >= 0)  && (tabNum < kMaxTabs))
	{
		if (cWindowTabs[tabNum] != NULL)
		{
			cWindowTabs[tabNum]->SetWidgetProgress(widgetIdx, currPosition, totalValue);

			if (cCurrentTabObjPtr != NULL)
			{
				myWidgetPtr	=	cCurrentTabObjPtr->cWidgetList;
				if (myWidgetPtr != NULL)
				{
					DrawOneWidget(&myWidgetPtr[widgetIdx], widgetIdx);
					cvShowImage(cWindowName, cOpenCV_Image);
				}
			}
		}
		else
		{
		}
	}
}


//**************************************************************************************
void	Controller::UpdateWindowTabColors(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


#pragma mark -

//*****************************************************************************
bool	PointInWidget(const int xPoint, const int yPoint, TYPE_WIDGET *theWidget)
{
bool	inRect;

	inRect	=	true;
	if (theWidget->valid == false)
	{
		inRect	=	false;
	}
	else if (xPoint < theWidget->left)
	{
		inRect	=	false;
	}
	else if (xPoint > (theWidget->left + theWidget->width))
	{
		inRect	=	false;
	}
	else if (yPoint < theWidget->top)
	{
		inRect	=	false;
	}
	else if (yPoint > (theWidget->top + theWidget->height))
	{
		inRect	=	false;
	}

	return(inRect);
}

//*****************************************************************************
bool	IsTrueFalse(const char *trueFalseString)
{
bool	trueFalseFlag;

	if (strcasecmp(trueFalseString, "true") == 0)
	{
		trueFalseFlag	=	true;
	}
	else
	{
		trueFalseFlag	=	false;
	}
	return(trueFalseFlag);
}


//*****************************************************************************
void	DumpWidget(TYPE_WIDGET *theWidget)
{
	CONSOLE_DEBUG_W_NUM("theWidget->valid\t\t=",		theWidget->valid);
	CONSOLE_DEBUG_W_NUM("theWidget->widgetType\t=",		theWidget->widgetType);
	CONSOLE_DEBUG_W_NUM("theWidget->left\t\t=",			theWidget->left);
	CONSOLE_DEBUG_W_NUM("theWidget->top\t\t=",			theWidget->top);
	CONSOLE_DEBUG_W_NUM("theWidget->width\t\t=",		theWidget->width);
	CONSOLE_DEBUG_W_NUM("theWidget->height\t\t=",		theWidget->height);
	CONSOLE_DEBUG_W_NUM("theWidget->includeBorder\t=",	theWidget->includeBorder);
	CONSOLE_DEBUG_W_NUM("theWidget->fontNum\t\t=",		theWidget->fontNum);
	CONSOLE_DEBUG_W_NUM("theWidget->selected\t=",		theWidget->selected);
}

static uint32_t	gSystemStartSecs = 0;

//*****************************************************************************
uint32_t	millis(void)
{
uint32_t	elapsedSecs;
uint32_t	milliSecs;
struct timeval	currentTime;

	gettimeofday(&currentTime, NULL);

	if (gSystemStartSecs == 0)
	{
		gSystemStartSecs	=	currentTime.tv_sec;
	}
	elapsedSecs	=	currentTime.tv_sec - gSystemStartSecs;
	milliSecs	=	(elapsedSecs * 1000) + (currentTime.tv_usec / 1000);
	return(milliSecs);
}




//******************************************************************************
//*	<C 20# >00
//*	<C 01# >ffff
//*	<C 02# >0000
//*	<C 03# >39e7
//*	<C 04# >f800
//*	<C 05# >f800
//*	<C 06# >8000
//*	<C 07# >fe00
//*	<C 08# >07ff
//*	<C 12# >39e7
//*	<C 14# >ffff
//*	<C 11# >000f
//*	<C 13# >f800
//******************************************************************************
CvScalar	Color16BitTo24Bit(const unsigned int color16)
{
int			redValue;
int			grnValue;
int			bluValue;
CvScalar	rgbValue;

	redValue	=	((color16 & 0x0ffff) >> 8)	& 0x00f8;
	grnValue	=	((color16 & 0x0ffff) >> 3)	& 0x00fC;
	bluValue	=	((color16 & 0x0ffff) << 3) 	& 0x00f8;

//	CONSOLE_DEBUG_W_HEX("color16\t=",	color16);
//	CONSOLE_DEBUG_W_HEX("redValue\t=",	redValue);
//	CONSOLE_DEBUG_W_HEX("grnValue\t=",	grnValue);
//	CONSOLE_DEBUG_W_HEX("bluValue\t=",	bluValue);

	rgbValue	=	CV_RGB(redValue, grnValue, bluValue);
	return(rgbValue);
}

IplImage	*gAlpacaLogoPtr	=	NULL;

//*****************************************************************************
void	LoadAlpacaLogo(void)
{
	if (gAlpacaLogoPtr == NULL)
	{
		gAlpacaLogoPtr	=	cvLoadImage("AlpacaLogo-vsmall.png", CV_LOAD_IMAGE_COLOR);
	}
}



