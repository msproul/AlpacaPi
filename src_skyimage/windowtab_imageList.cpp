//*****************************************************************************
//*		windowtab_imageList.cpp		(c) 2024 by Mark Sproul
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar 12,	2024	<MLS> Created windowtab_imageList.cpp
//*****************************************************************************

#include	<stdlib.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"windowtab.h"
#include	"windowtab_imageList.h"

#include	"controller.h"
#include	"imagelist.h"
#include	"controller_image.h"
#include	"controller_skyimage.h"

static int	gSortColumn;
static bool	gInvertSort_ImageList	=	false;

//**************************************************************************************
WindowTabImageList::WindowTabImageList(	const int	xSize,
										const int	ySize,
										cv::Scalar	backGrndColor,
										const char	*windowName)
										:WindowTab(xSize, ySize, backGrndColor, windowName)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cImageListCount		=	gImageCount;
	cSortColumn			=	-1;
	cFirstLineIdx		=	0;

//	strcpy(cWebURLstring, "clientapps.html");		//*	set the web help url string

	SetupWindowControls();

	UpdateOnScreenWidgetList();
	ForceWindowUpdate();

}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabImageList::~WindowTabImageList(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabImageList::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		yLoc2;
int		textBoxHt;
int		textBoxWd;
int		widgetWidth;
int		iii;
short	tabArray[kMaxTabStops]	=	{450, 550, 650, 750, 850, 950, 0};
int		clmnHdr_xLoc;
int		clmnHdrWidth;
char	titleText[256];
//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	strcpy(titleText, gDirectoryPath);
	strcat(titleText, " (double click to open image)");
	yLoc	=	SetTitleBox(kImageList_Title, -1, yLoc, titleText);

	clmnHdr_xLoc	=	1;
	iii				=	kImageList_ClmTitle1;
	while (iii <= kImageList_ClmTitle5)
	{
		clmnHdrWidth	=	tabArray[iii - kImageList_ClmTitle1] - clmnHdr_xLoc;

		SetWidget(				iii,	clmnHdr_xLoc,			yLoc,		clmnHdrWidth,		cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetBGColor(		iii,	CV_RGB(128,	128,	128));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));

		clmnHdr_xLoc	=	tabArray[iii - kImageList_ClmTitle1];
		clmnHdr_xLoc	+=	2;


		iii++;
	}
	SetWidgetText(		kImageList_ClmTitle1,	"File Name");
	SetWidgetText(		kImageList_ClmTitle2,	"DATAMIN");
	SetWidgetText(		kImageList_ClmTitle3,	"DATAMAX");
	SetWidgetText(		kImageList_ClmTitle4,	"Exposure");
	SetWidgetText(		kImageList_ClmTitle5,	"%sat");
	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;


	//=======================================================
	xLoc		=	10;
	textBoxHt	=	14;
	textBoxWd	=	cWidth - (xLoc + 3);
	for (iii=kImageList_AlpacaDev_01; iii<=kImageList_AlpacaDev_Last; iii++)
	{
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetBorder(		iii,	false);
		SetWidgetTabStops(		iii,	tabArray);
		SetWidgetBGColorSelected(iii,	CV_RGB(75,	75,	75));

		yLoc			+=	textBoxHt;
		yLoc			+=	3;
	}

	//---------------------------------------------------------------------
	xLoc		=	0;
	widgetWidth	=	(cWidth / 4) + 40;
	SetWidget(				kImageList_AlpacaDev_Total,	xLoc,	yLoc,	widgetWidth,	cSmallBtnHt);
	SetWidgetFont(			kImageList_AlpacaDev_Total,	kFont_Medium);
	SetWidgetText(			kImageList_AlpacaDev_Total,	"Total units =?");
	SetWidgetJustification(	kImageList_AlpacaDev_Total,	kJustification_Left);
	SetWidgetTextColor(		kImageList_AlpacaDev_Total,	CV_RGB(255,	255,	255));

	xLoc		+=	widgetWidth;
	xLoc		+=	2;

	//---------------------------------------------------------------------
	SetWidget(			kImageList_Btn_CloseAll,	xLoc,	yLoc,		widgetWidth,		cSmallBtnHt);
	SetWidgetFont(		kImageList_Btn_CloseAll,	kFont_Medium);
	SetWidgetType(		kImageList_Btn_CloseAll,	kWidgetType_Button);
	SetWidgetBGColor(	kImageList_Btn_CloseAll,	CV_RGB(255,	255,	255));
	SetWidgetText(		kImageList_Btn_CloseAll,	"Close All");

	xLoc		+=	widgetWidth;
	xLoc		+=	2;
	//---------------------------------------------------------------------
	SetWidget(			kImageList_Btn_Help,	xLoc,	yLoc,		widgetWidth,		cSmallBtnHt);
	SetWidgetFont(		kImageList_Btn_Help,	kFont_Medium);
	SetWidgetType(		kImageList_Btn_Help,	kWidgetType_Button);
	SetWidgetBGColor(	kImageList_Btn_Help,	CV_RGB(255,	255,	255));
	SetWidgetHelpText(	kImageList_Btn_Help,	"Click to launch web documentation");
	SetWidgetText(		kImageList_Btn_Help,	"Help");


	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//---------------------------------------------------------------------
	//*	set up the vertical scroll bar
	xLoc			=	5;
	yLoc2			=	cTabVertOffset;
	yLoc2			+=	cTitleHeight;
	yLoc2			+=	2;
	SetWidget(					kImageList_ScrollBar,	(cWidth - 20),		yLoc2,		15,		600);
	SetWidgetType(				kImageList_ScrollBar,	kWidgetType_ScrollBar);
	SetWidgetFont(				kImageList_ScrollBar,	kFont_Small);
	SetWidgetScrollBarLimits(	kImageList_ScrollBar, (kImageList_AlpacaDev_Last - kImageList_AlpacaDev_01), 100);
	SetWidgetScrollBarValue(	kImageList_ScrollBar, 50);

	SetAlpacaLogoBottomCorner(kImageList_AlpacaLogo);
}

//*****************************************************************************
void	ToLowerStr(char *theString)
{
int		ii;

	ii	=	0;
	while (theString[ii] > 0)
	{
		theString[ii]	=	tolower(theString[ii]);
		ii++;
	}
}

//*****************************************************************************
void	WindowTabImageList::RunWindowBackgroundTasks(void)
{
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_ABORT(__FUNCTION__);

	//*	step through all of the devices and see if they needed updating
	//*	only do one each time we get called so we dont use up all the CPU
	for (iii=0; iii<cImageListCount; iii++)
	{
		if (gImageList[iii].validEntry)
		{

		}
	}
}

//*****************************************************************************
void	WindowTabImageList::HandleKeyDown(const int keyPressed)
{
int		deviceIndex;
int		iii;
int		theExtendedChar;
int		retCode;
char	otherFilePath[512];
char	*extensionPtr;
//	CONSOLE_DEBUG_W_HEX("keyPressed=", keyPressed);

	//*	find a selected device
	deviceIndex	=	-1;
	for (iii=0; iii<gImageCount; iii++)
	{
		if (gImageList[iii].lineSelected)
		{
			deviceIndex	=	iii;
			break;
		}
	}

	theExtendedChar	=	keyPressed & 0x00ffff;
	switch(theExtendedChar)
	{
		//*	return, open current selected entry
		case 0x0d:
		case 0xff8d:
//			OpenControllerFromDevIdx(deviceIndex);
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

		//*	delete key
		case 0x0000ff:
		case 0x0080ff:
			CONSOLE_DEBUG_W_HEX("Delete key=", keyPressed);
			break;

		//*	Delete key
		case 0x00FFFF:
			CONSOLE_DEBUG_W_HEX("Delete key=", keyPressed);
			if (deviceIndex >= 0)
			{
				if (gImageList[deviceIndex].lineSelected)
				{
					CONSOLE_DEBUG_W_STR("Deleting:", gImageList[deviceIndex].FilePath);
					retCode	=	remove(gImageList[deviceIndex].FilePath);
					if (retCode == 0)
					{
						strcat(gImageList[deviceIndex].FileName, "-DELETED");
					}
					//*	now delete other files
					strcpy(otherFilePath, gImageList[deviceIndex].FilePath);
					extensionPtr	=	strcasestr(otherFilePath, ".fits");
					if (extensionPtr != NULL)
					{
						strcpy(extensionPtr, ".jpg");
						CONSOLE_DEBUG_W_STR("Deleting:", otherFilePath);
						retCode	=	remove(otherFilePath);
						CONSOLE_DEBUG_W_NUM("Deleting jpg:", retCode);
					}
					strcpy(otherFilePath, gImageList[deviceIndex].FilePath);
					extensionPtr	=	strcasestr(otherFilePath, ".fits");
					if (extensionPtr != NULL)
					{
						strcpy(extensionPtr, ".png");
						CONSOLE_DEBUG_W_STR("Deleting:", otherFilePath);
						retCode	=	remove(otherFilePath);
						CONSOLE_DEBUG_W_NUM("Deleting png:", retCode);
					}
				}
			}
			break;


		default:
			CONSOLE_DEBUG_W_HEX("Ignored: keyPressed=", keyPressed);
			break;
	}
}

//*****************************************************************************
void	CloseAllExceptFirst(void)
{
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
	for (iii=0; iii<kMaxControllers; iii++)
	{
		if (gControllerList[iii] != NULL)
		{
			if (gControllerList[iii]->cWindowType != 'IMAG')
			{
				gControllerList[iii]->cKeepRunning	=	false;
			}
		}
	}
}


//*****************************************************************************
void	WindowTabImageList::ProcessButtonClick(const int buttonIdx, const int flags)
{
int	newSortColumn;

//	CONSOLE_DEBUG(__FUNCTION__);

	switch(buttonIdx)
	{
		case kImageList_ClmTitle1:
		case kImageList_ClmTitle2:
		case kImageList_ClmTitle3:
		case kImageList_ClmTitle4:
		case kImageList_ClmTitle5:
			newSortColumn	=	buttonIdx - kImageList_ClmTitle1;
			if (newSortColumn == cSortColumn)
			{
				gInvertSort_ImageList	=	!gInvertSort_ImageList;
			}
			else
			{
				gInvertSort_ImageList	=	false;
			}
			cSortColumn		=	newSortColumn;
			UpdateSortOrder();
			ForceWindowUpdate();
			break;

		case kImageList_Btn_CloseAll:
			CloseAllExceptFirst();
			break;

		case kImageList_Btn_Help:
			LaunchWebHelp();
			break;
	}
}

//*****************************************************************************
static void	OpenImageFromIndex(int imgIndex)
{
ControllerImage		*imageWindowController;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG("Creating ControllerImage()")
	//*	this will open a new window with the image displayed
	gCurrentImageIndex		=	imgIndex;
	imageWindowController	=	new ControllerImage(	gImageList[imgIndex].DirectoryPath,
														gImageList[imgIndex].FilePath);
}

//*****************************************************************************
void	WindowTabImageList::ProcessDoubleClick(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
int		imageIdx;

//	CONSOLE_DEBUG(__FUNCTION__);

	if ((widgetIdx >= kImageList_AlpacaDev_01) && (widgetIdx <= kImageList_AlpacaDev_Last))
	{
		imageIdx	=	widgetIdx - kImageList_AlpacaDev_01;
		imageIdx	+=	cFirstLineIdx;	//*	adjust for the scrolling
		OpenImageFromIndex(imageIdx);
	}
}

//*****************************************************************************
void	WindowTabImageList::ProcessMouseWheelMoved(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	wheelMovement,
													const int	flags)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, wheelMovement);

	cFirstLineIdx	+=	wheelMovement;
	if (cFirstLineIdx < 0)
	{
		cFirstLineIdx	=	0;
	}
	UpdateOnScreenWidgetList();
	ForceWindowUpdate();
}

//**************************************************************************************
void	WindowTabImageList::ProcessLineSelect(int widgetIdx)
{
int		imageIdx;
int		adjustedIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= kImageList_AlpacaDev_01) && (widgetIdx < kImageList_AlpacaDev_Last))
	{
		adjustedIdx	=	widgetIdx - kImageList_AlpacaDev_01;
		imageIdx	=	adjustedIdx + cFirstLineIdx;

		if ((imageIdx >= 0) && (imageIdx < cImageListCount))
		{
			SetNewSelectedDevice(imageIdx);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("imageIdx is out of bounds\t=", imageIdx);
		}
		UpdateOnScreenWidgetList();
		ForceWindowUpdate();
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("widgetIdx is out of range\t=", widgetIdx);
	}
}

//*****************************************************************************
void	WindowTabImageList::SetNewSelectedDevice(int deviceIndex)
{
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("deviceIndex\t=", deviceIndex);
	if (deviceIndex >= 0)
	{
		//*	clear out previous selections
		for (iii=0; iii<cImageListCount; iii++)
		{
			gImageList[iii].lineSelected	=	false;
		}
		gImageList[deviceIndex].lineSelected	=	true;
		UpdateOnScreenWidgetList();
		ForceWindowUpdate();
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("deviceIndex is out of range\t=", deviceIndex);
	}
}

//*****************************************************************************
void	WindowTabImageList::UpdateSliderValue(const int	widgetIdx, double newSliderValue)
{

	CONSOLE_DEBUG(__FUNCTION__);

	switch(widgetIdx)
	{
		case kImageList_ScrollBar:
			break;

	}
	ForceWindowUpdate();
}


//**************************************************************************************
void	WindowTabImageList::UpdateOnScreenWidgetList(void)
{
int				boxId;
int				iii;
char			textString[128];
int				imageIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("cImageListCount\t=", cImageListCount);

	//*	limit how far we can scroll
	if (cFirstLineIdx >= gImageCount)
	{
		cFirstLineIdx	=	gImageCount -1;
	}
	if (cFirstLineIdx < 0)
	{
		cFirstLineIdx	=	0;
	}

	iii				=	0;
	boxId			=	0;
	cLinesOnScreen	=	(kImageList_AlpacaDev_Last - kImageList_AlpacaDev_01) + 1;
	while (iii < cLinesOnScreen)
	{
		boxId		=	iii + kImageList_AlpacaDev_01;
		imageIdx	=	cFirstLineIdx + iii;
		if ((boxId <= kImageList_AlpacaDev_Last) && (imageIdx < gImageCount) && (gImageList[imageIdx].validEntry))
		{

			sprintf(textString, "%s\t%d\t%d\t%3.6f\t%3.4f",
									gImageList[imageIdx].FileName,
									gImageList[imageIdx].DATAMIN,
									gImageList[imageIdx].DATAMAX,
									gImageList[imageIdx].Exposure_secs,
									gImageList[imageIdx].SaturationPercent);

			//-----------------------------------------------------
			//*	deal with selected state
			if (gImageList[imageIdx].lineSelected == true)
			{
				SetWidgetLineSelect(boxId, true);
			}
			else
			{
				SetWidgetLineSelect(boxId, false);
			}
			SetWidgetText(boxId, textString);
		}
		else if (boxId <= kImageList_AlpacaDev_Last)
		{
			SetWidgetText(boxId, "---");
			SetWidgetLineSelect(	boxId, false);
			SetWidgetTextColor(		boxId,	CV_RGB(0xff,	0xff,	0xff));
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("iii\t=", iii);
			break;
		}

		////

		iii++;
	}
//	CONSOLE_DEBUG_W_NUM("gImageCount\t\t=", gImageCount);


	sprintf(textString, "Image count=%d", cImageListCount);
	SetWidgetText(kImageList_AlpacaDev_Total, textString);

	SetWidgetBGColor(kImageList_ClmTitle1,	((cSortColumn == 0) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kImageList_ClmTitle2,	((cSortColumn == 1) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kImageList_ClmTitle3,	((cSortColumn == 2) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kImageList_ClmTitle4,	((cSortColumn == 3) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
	SetWidgetBGColor(kImageList_ClmTitle5,	((cSortColumn == 4) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));

}


//**************************************************************************************
static  int ImageListQsortProc(const void *e1, const void *e2)
{
TYPE_ImageFile	*obj1, *obj2;
int				returnValue;


	obj1		=	(TYPE_ImageFile *)e1;
	obj2		=	(TYPE_ImageFile *)e2;

	returnValue	=	0;
	switch(gSortColumn)
	{
		case 0:
			returnValue	=	strcasecmp(obj1->FileName, obj2->FileName);
			break;

		case 1:
			returnValue	=	obj1->DATAMIN - obj2->DATAMIN;
			break;

		case 2:
			returnValue	=	obj1->DATAMAX - obj2->DATAMAX;
			break;

		case 3:
			if (obj1->Exposure_secs > obj2->Exposure_secs)
			{
				returnValue	=	1;
			}
			else if (obj2->Exposure_secs > obj1->Exposure_secs)
			{
				returnValue	=	-1;
			}
			else
			{
				returnValue	=	strcasecmp(obj1->FileName, obj2->FileName);
			}
			break;

		case 4:
			if (obj1->SaturationPercent > obj2->SaturationPercent)
			{
				returnValue	=	1;
			}
			else if (obj2->SaturationPercent > obj1->SaturationPercent)
			{
				returnValue	=	-1;
			}
			else
			{
				returnValue	=	strcasecmp(obj1->FileName, obj2->FileName);
			}
			break;
	}

	if (gInvertSort_ImageList)
	{
		returnValue	=	-returnValue;
	}
	return(returnValue);
}

//**************************************************************************************
void	WindowTabImageList::UpdateSortOrder(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("cImageListCount\t=", cImageListCount);

	if (cSortColumn >= 0)
	{
		gSortColumn	=	cSortColumn;
		qsort(gImageList, cImageListCount, sizeof(TYPE_ImageFile), ImageListQsortProc);

		UpdateOnScreenWidgetList();
	}

}
