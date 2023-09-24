//*****************************************************************************
//*		windowtab_image.cpp		(c) 2020 by Mark Sproul
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
//*	Dec 29,	2020	<MLS> Created windowtab_image.cpp
//*	Feb  2,	2021	<MLS> Added the ability to zoom and scroll image by draging the mouse.
//*	Feb 28,	2021	<MLS> Working on image scrolling/dragging
//*	Feb 28,	2021	<MLS> Finally got image dragging to work properly
//*	Apr  5,	2021	<MLS> Zoomed in live view now working
//*	Feb 26,	2022	<MLS> Image zooming working under opencv C++
//*	Feb 26,	2023	<MLS> Added histogram drawing
//*	Feb 27,	2023	<MLS> Added cross hairs
//*	Feb 27,	2023	<MLS> Added UpdateButtons()
//*	Feb 27,	2023	<MLS> Added SaveHistogram()
//*	Feb 27,	2023	<MLS> Added DrawCrossHairs()
//*	Feb 28,	2023	<MLS> Added SaveCrossHairList(), ReadCrossHairList()
//*	Feb 28,	2023	<MLS> Expanded 1 cross hair to 3
//*	Feb 28,	2023	<MLS> Added color schemes to cross hairs
//*	Mar  4,	2023	<MLS> Added PGR (Purple, Gold, Red) cross hair color option
//*	Sep  7,	2023	<MLS> Added Live vs Downloaded label to make it obvious
//*****************************************************************************

#ifdef _ENABLE_CTRL_IMAGE_


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"opencv_utils.h"

#include	"alpaca_defs.h"

#include	"controller.h"
#include	"controller_image.h"

#include	"windowtab.h"
#include	"windowtab_image.h"

//**************************************************************************************
WindowTabImage::WindowTabImage(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor,
								const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
int		iii;
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("xSize        \t=",	xSize);
//	CONSOLE_DEBUG_W_NUM("ySize        \t=",	ySize);

	cOpenCVdownLoadedImage	=	NULL;
	cOpenCVdisplayedImage	=	NULL;
	cImageZoomState			=	0;			//*	more state to be defined later

	cMouseDragInProgress	=	false;

	cImageCenterX			=	500;
	cImageCenterY			=	500;
	cDisplayCrossHair		=	false;
	cCurrCrossHairNum		=	0;
	cCurrCrossHairColorSchm	=	0;
	for (iii=0; iii<kCrossHairCnt; iii++)
	{
		cCrossHairPos[iii].XLocation	=	0;
		cCrossHairPos[iii].YLocation	=	0;
	}


	SetupWindowControls();
	ReadCrossHairList();
	UpdateButtons();
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabImage::~WindowTabImage(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cOpenCVdownLoadedImage != NULL)
	{
//		CONSOLE_DEBUG("destroy old image");
//		SetWidgetImage(kImageDisplay_ImageDisplay, NULL);
//		cvReleaseImage(&cOpenCVdownLoadedImage);
//		cOpenCVdownLoadedImage	=	NULL;
	}
}


//**************************************************************************************
void	WindowTabImage::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		save_yLoc;
int		colorRadioBtn_yLoc;
int		colorRadioBtn_xLoc;
int		iii;
int		imageBoxWidth;
int		imageBoxHeight;
int		labelWidth;
int		dataWidth;
int		boxWidth;
int		boxHeight;
int		histLblWidth;
int		hist_xLoc;
char	buttonName[16];
int		crossHairBoxWidth;
int		crossH_Xloc;
int		crossH_Yloc;
int		btnCnt;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	yLoc	=	SetTitleBox(kImageDisplay_Title, -1, yLoc, "AlpacaPi project");

	//------------------------------------------
	//*	do the buttons
	xLoc			=	5;
	buttonName[0]	=	'A';
	buttonName[1]	=	0;
	btnCnt			=	0;
	for (iii=kImageDisplay_Btn_1; iii<=kImageDisplay_Btn_N; iii++)
	{
		SetWidget(				iii,	xLoc, yLoc, cTitleHeight, cTitleHeight);
		SetWidgetType(			iii, 	kWidgetType_Button);
		SetWidgetText(			iii,	buttonName);

		xLoc	+=	cTitleHeight;
		xLoc	+=	2;

		buttonName[0]++;
		btnCnt++;
	}
	//*	Live or Downloaded indicator
	labelWidth	=	cWidth - (btnCnt * (cTitleHeight + 2));
	labelWidth	-=	10;
	SetWidget(				kImageDisplay_LiveOrDownLoad,	xLoc,	yLoc,	labelWidth, cTitleHeight);
	SetWidgetType(			kImageDisplay_LiveOrDownLoad, 	kWidgetType_TextBox);
	SetWidgetText(			kImageDisplay_LiveOrDownLoad,	"---");
	SetWidgetTextColor(		kImageDisplay_LiveOrDownLoad,	CV_RGB(0, 0, 0));
	SetWidgetJustification(	kImageDisplay_LiveOrDownLoad, 	kJustification_Center);

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	save_yLoc		=	yLoc;

	//------------------------------------------
	xLoc		=	5;
	labelWidth	=	155;
	dataWidth	=	110;
	iii		=	kImageDisplay_FrameCnt_Lbl;
	while (iii < kImageDisplay_FramesSaved)
	{
		SetWidget(				iii,	xLoc,	yLoc,		labelWidth,		cSmallBtnHt);
		SetWidgetType(			iii, 	kWidgetType_TextBox);
		SetWidgetFont(			iii, 	kFont_TextList);
		SetWidgetJustification(	iii, 	kJustification_Left);
		iii++;

		SetWidget(				iii,	(xLoc + labelWidth + 2),	yLoc,		dataWidth,		cSmallBtnHt);
		SetWidgetType(			iii, 	kWidgetType_TextBox);
		SetWidgetFont(			iii, 	kFont_TextList);
		SetWidgetJustification(	iii, 	kJustification_Center);

		yLoc	+=	cSmallBtnHt;
		yLoc	+=	2;

		iii++;
	}

	SetWidgetText(kImageDisplay_FrameCnt_Lbl,		"Frames");
	SetWidgetText(kImageDisplay_Exposure_Lbl,		"Exposure");
	SetWidgetText(kImageDisplay_Gain_Lbl,			"Gain");
	SetWidgetText(kImageDisplay_CameraTemp_Lbl,		"CameraTemp");
	SetWidgetText(kImageDisplay_Filter_Lbl,			"Filter");
	SetWidgetText(kImageDisplay_Object_Lbl,			"Object");
	SetWidgetText(kImageDisplay_FramesSaved_Lbl,	"FramesSaved");

	//-------------------------------------------------------------------------
	//*	set up the histogram
	//*	first 3 text boxes for the R,G,B percent values
	boxWidth		=	labelWidth + dataWidth;
	boxHeight		=	cSmallBtnHt * 4;
	histLblWidth	=	(boxWidth - 8) / 3;
	hist_xLoc		=	xLoc +2;
	iii		=	kImageDisplay_HistRedPerct;
	while (iii <= kImageDisplay_HistBluPerct)
	{
		SetWidget(				iii,	hist_xLoc,	yLoc,		histLblWidth,		cSmallBtnHt);
		SetWidgetType(			iii, 	kWidgetType_TextBox);
		SetWidgetFont(			iii, 	kFont_TextList);
		SetWidgetJustification(	iii, 	kJustification_Center);
		SetWidgetBorder(		iii,	true);

		hist_xLoc	+=	histLblWidth;
		hist_xLoc	+=	2;
		iii++;
	}
	//-------------------------------------------------------------------------
	SetWidgetText(kImageDisplay_HistRedPerct,			"R=---%");
	SetWidgetText(kImageDisplay_HistGrnPerct,			"G=---%");
	SetWidgetText(kImageDisplay_HistBluPerct,			"B=---%");

	SetWidgetTextColor(kImageDisplay_HistRedPerct,		CV_RGB(255,		0,		0));
	SetWidgetTextColor(kImageDisplay_HistGrnPerct,		CV_RGB(0,		255,	0));
	SetWidgetTextColor(kImageDisplay_HistBluPerct,		CV_RGB(0x64,	0x8c,	0xff));
	yLoc		+=	cSmallBtnHt;
	yLoc		+=	2;

	yLoc		+=	6;
	boxHeight	=	8;
	iii			=	kImageDisplay_LumBar;
	while (iii <= kImageDisplay_BluBar)
	{
		SetWidget(				iii,	xLoc + 2,	yLoc,		boxWidth,	boxHeight);
		SetWidgetType(			iii, 	kWidgetType_ProessBar);
		SetWidgetBorder(		iii,	false);
		yLoc	+=	boxHeight;
		yLoc	+=	1;
		iii++;
	}
	yLoc		+=	6;

	SetWidgetTextColor(kImageDisplay_LumBar,	CV_RGB(200,		200,	200));
	SetWidgetTextColor(kImageDisplay_RedBar,	CV_RGB(255,		0,		0));
	SetWidgetTextColor(kImageDisplay_GrnBar,	CV_RGB(0,		255,	0));
	SetWidgetTextColor(kImageDisplay_BluBar,	CV_RGB(0x64,	0x8c,	0xff));

	boxHeight	=	150;
	SetWidget(		kImageDisplay_Histogram,	xLoc + 2,	yLoc,		boxWidth,		boxHeight);
	SetWidgetType(	kImageDisplay_Histogram, 	kWidgetType_CustomGraphic);
	yLoc		+=	boxHeight;
	yLoc		+=	4;

	//-------------------------------------------------------------------------
	//*	set up the save histogram button
	SetWidget(			kImageDisplay_SaveHistBtn,	xLoc+2,	yLoc,		boxWidth-4,		cBtnHeight);
	SetWidgetType(		kImageDisplay_SaveHistBtn, 	kWidgetType_Button);
	SetWidgetFont(		kImageDisplay_SaveHistBtn,	kFont_Medium);
	SetWidgetBGColor(	kImageDisplay_SaveHistBtn,	CV_RGB(255,	255,	255));
	SetWidgetTextColor(	kImageDisplay_SaveHistBtn,	CV_RGB(255,	0,	0));
	SetWidgetText(		kImageDisplay_SaveHistBtn,	"Save Histogram");
	yLoc		+=	cBtnHeight;
	yLoc		+=	4;

	SetWidgetOutlineBox(kImageDisplay_HistogramOutline, kImageDisplay_HistRedPerct, (kImageDisplay_HistogramOutline -1));


	//-------------------------------------------------------------------------
	//*	cross hair stuff
	yLoc		+=	4;
	SetWidget(		kImageDisplay_CrossHairTitle,	xLoc,	yLoc,	boxWidth,	cRadioBtnHt);
	SetWidgetFont(	kImageDisplay_CrossHairTitle,	kFont_RadioBtn);
	SetWidgetText(	kImageDisplay_CrossHairTitle,	"ctrl-click to set location");
	yLoc		+=	cRadioBtnHt;
	yLoc		+=	2;

	SetWidget(		kImageDisplay_CrossHairChkBox,	xLoc,	yLoc,	cLrgBtnWidth,	cRadioBtnHt);
	SetWidgetType(	kImageDisplay_CrossHairChkBox,	kWidgetType_CheckBox);
	SetWidgetFont(	kImageDisplay_CrossHairChkBox,	kFont_RadioBtn);
	SetWidgetText(	kImageDisplay_CrossHairChkBox,	"Display Cross Hair");
	yLoc		+=	cRadioBtnHt;
	yLoc		+=	2;

	crossHairBoxWidth	=	(boxWidth / 3) - 7;
	crossH_Xloc			=	xLoc + cSmallBtnHt + 2;
	crossH_Yloc			=	xLoc + cSmallBtnHt + crossHairBoxWidth + 3;
	colorRadioBtn_yLoc	=	yLoc - cRadioBtnHt;
	colorRadioBtn_xLoc	=	crossH_Yloc + crossHairBoxWidth + 3;
	iii					=	kImageDisplay_CrossHair1Chk;
	while (iii <= kImageDisplay_CrossHair3Chk)
	{
		SetWidget(		iii,	xLoc,	yLoc,	cSmallBtnHt,	cSmallBtnHt);
		SetWidgetType(	iii,	kWidgetType_RadioButton);
		SetWidgetFont(	iii, 	kFont_TextList);
		iii++;

		SetWidget(		iii,	crossH_Xloc,	yLoc,	crossHairBoxWidth,	cSmallBtnHt);
		SetWidgetFont(	iii, 	kFont_TextList);
		iii++;

		SetWidget(		iii,	crossH_Yloc,	yLoc,	crossHairBoxWidth,	cSmallBtnHt);
		SetWidgetFont(	iii, 	kFont_TextList);
		iii++;

		yLoc		+=	cSmallBtnHt;
		yLoc		+=	2;

	}

	//---------------------------------------------------------
	//*	now set up the color scheme buttons
	crossHairBoxWidth	=	(boxWidth / 3) - 9;
	iii					=		kImageDisplay_RGB;
	while (iii <= kImageDisplay_GRY)
	{
		SetWidget(		iii,	colorRadioBtn_xLoc,	colorRadioBtn_yLoc,	crossHairBoxWidth,	cRadioBtnHt);
		SetWidgetType(	iii,	kWidgetType_RadioButton);
		SetWidgetFont(	iii, 	kFont_RadioBtn);
		iii++;

		colorRadioBtn_yLoc		+=	cRadioBtnHt;
		colorRadioBtn_yLoc		+=	2;

	}
	SetWidgetText(	kImageDisplay_RGB,	"RGB");
	SetWidgetText(	kImageDisplay_CMY,	"CMY");
	SetWidgetText(	kImageDisplay_OHS,	"OHS");
	SetWidgetText(	kImageDisplay_PGR,	"PGR");
	SetWidgetText(	kImageDisplay_BPO,	"BPO");
	SetWidgetText(	kImageDisplay_GRY,	"GRAY");


	yLoc				=	colorRadioBtn_yLoc;
	//-------------------------------------------------
	//*	reload,clear,save buttons
	yLoc				+=	8;
	iii					=	kImageDisplay_Reload;
	crossHairBoxWidth	=	(boxWidth / 3);
	crossH_Xloc			=	xLoc + 2;
	while (iii <= kImageDisplay_Save)
	{
		SetWidget(			iii,	crossH_Xloc,	yLoc,	crossHairBoxWidth,	cSmallBtnHt);
		SetWidgetType(		iii,	kWidgetType_Button);
		SetWidgetBGColor(	iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(	iii,	CV_RGB(255,	0,	0));
		SetWidgetFont(		iii, 	kFont_TextList);

		crossH_Xloc	+=	crossHairBoxWidth;
		crossH_Xloc	+=	2;
		iii++;
	}

	SetWidgetText(		kImageDisplay_Reload,	"Reload");
	SetWidgetText(		kImageDisplay_Clear,	"Clear");
	SetWidgetText(		kImageDisplay_Save,		"Save");
	yLoc		+=	cSmallBtnHt;
	yLoc		+=	2;

	SetWidgetOutlineBox(kImageDisplay_CrossHairOutline, kImageDisplay_CrossHairTitle, (kImageDisplay_CrossHairOutline -1));

	//-------------------------------------------------------------------------
	//*	set up the image display area
	xLoc			+=	labelWidth + dataWidth;
	xLoc			+=	2;
	imageBoxWidth	=	cWidth - xLoc;
	imageBoxWidth	-=	10;
	yLoc			=	save_yLoc;

	//*	first set the image display info
	SetWidget(		kImageDisplay_ImageDisplayInfo,	xLoc + 5,	yLoc,		imageBoxWidth,		cTitleHeight);
	SetWidgetFont(	kImageDisplay_ImageDisplayInfo, 		kFont_Medium);
	SetWidgetJustification(	kImageDisplay_ImageDisplayInfo, kJustification_Left);
	SetWidgetText(	kImageDisplay_ImageDisplayInfo,	"Image info");
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	//*	now compute the display area
	imageBoxWidth	=	cWidth - xLoc;
	imageBoxHeight	=	cHeight - yLoc;
	imageBoxWidth	-=	2;
	imageBoxHeight	-=	2;


	SetWidget(				kImageDisplay_ImageDisplay,	xLoc,	yLoc,		imageBoxWidth,		imageBoxHeight);
	SetWidgetType(			kImageDisplay_ImageDisplay,	kWidgetType_Image);
	SetWidgetBGColor(		kImageDisplay_ImageDisplay,	CV_RGB(128,	128,	128));
	SetWidgetBorderColor(	kImageDisplay_ImageDisplay,	CV_RGB(255,	255,	255));
	SetWidgetBorder(		kImageDisplay_ImageDisplay,	true);
	//*	the cross hair box has the exact same dimensions as the image
	SetWidget(				kImageDisplay_ImageCrossHair,	xLoc,	yLoc,		imageBoxWidth,		imageBoxHeight);
	SetWidgetType(			kImageDisplay_ImageCrossHair,	kWidgetType_Custom);

	//*	these are relative to the image box, not the image
	//*	they will get changed later
	cCrossHairPos[0].XLocation	=	imageBoxWidth / 2;
	cCrossHairPos[0].YLocation	=	imageBoxHeight / 2;

//	CONSOLE_DEBUG_W_NUM("cWidth        \t=",	cWidth);
//	CONSOLE_DEBUG_W_NUM("xLoc          \t=",	xLoc);
//	CONSOLE_DEBUG_W_NUM("yLoc          \t=",	yLoc);
//	CONSOLE_DEBUG_W_NUM("imageBoxWidth \t=",	imageBoxWidth);
//	CONSOLE_DEBUG_W_NUM("imageBoxHeight\t=",	imageBoxHeight);
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");
}

//*****************************************************************************
void	WindowTabImage::HandleKeyDown(const int keyPressed)
{
bool	updateFlag;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_HEX("keyPressed\t",	keyPressed);

	updateFlag	=	true;
	switch(keyPressed & 0x7f)
	{
		case '.':
			cHistogramPenSize++;
			if (cHistogramPenSize > 3)
			{
				cHistogramPenSize	=	1;
			}
			updateFlag	=	false;
			break;

		case '0':
			ResetImage();
			updateFlag	=	false;
			break;

		case '4':
			cImageCenterX	-=	100;
			break;

		case '6':
			cImageCenterX	+=	100;
			break;

		case '2':
			cImageCenterY	+=	100;
			break;

		case '8':
			cImageCenterY	-=	100;
			break;

		case '7':
			cImageCenterX	-=	100;
			cImageCenterY	-=	100;
			break;

		case '9':
			cImageCenterX	+=	100;
			cImageCenterY	-=	100;
			break;

		case '1':
			cImageCenterX	-=	100;
			cImageCenterY	+=	100;
			break;

		case '3':
			cImageCenterX	+=	100;
			cImageCenterY	+=	100;
			break;

		default:
			updateFlag	=	false;
			break;
	}
	if (updateFlag)
	{
//		CONSOLE_DEBUG(__FUNCTION__);
		DrawFullScaleIamge(cImageCenterX, cImageCenterY);
	}
}

//*****************************************************************************
void	WindowTabImage::UpdateButtons(void)
{
int		iii;
int		radioBtnIdx;
int		xWidgetIdx;
int		yWidgetIdx;

//	CONSOLE_DEBUG_W_BOOL("cDisplayCrossHair\t=", cDisplayCrossHair);
	SetWidgetChecked(kImageDisplay_CrossHairChkBox, cDisplayCrossHair);

	for (iii=0; iii<kCrossHairCnt; iii++)
	{
		//*	set the radio button for the current cross hair
		radioBtnIdx	=	kImageDisplay_CrossHair1Chk + (3 * iii);
		xWidgetIdx	=	kImageDisplay_CrossHair1X + (iii * 3);
		yWidgetIdx	=	xWidgetIdx + 1;

		SetWidgetChecked(	radioBtnIdx, (iii == cCurrCrossHairNum));
		SetWidgetNumber(	xWidgetIdx, cCrossHairPos[iii].XLocation);
		SetWidgetNumber(	yWidgetIdx, cCrossHairPos[iii].YLocation);
	}
	SetWidgetChecked(	kImageDisplay_RGB, (cCurrCrossHairColorSchm == (kImageDisplay_RGB - kImageDisplay_RGB)));
	SetWidgetChecked(	kImageDisplay_CMY, (cCurrCrossHairColorSchm == (kImageDisplay_CMY - kImageDisplay_RGB)));
	SetWidgetChecked(	kImageDisplay_PGR, (cCurrCrossHairColorSchm == (kImageDisplay_PGR - kImageDisplay_RGB)));
	SetWidgetChecked(	kImageDisplay_OHS, (cCurrCrossHairColorSchm == (kImageDisplay_OHS - kImageDisplay_RGB)));
	SetWidgetChecked(	kImageDisplay_BPO, (cCurrCrossHairColorSchm == (kImageDisplay_BPO - kImageDisplay_RGB)));
	SetWidgetChecked(	kImageDisplay_GRY, (cCurrCrossHairColorSchm == (kImageDisplay_GRY - kImageDisplay_RGB)));

	ForceWindowUpdate();
}

//*****************************************************************************
void	WindowTabImage::ProcessButtonClick(const int buttonIdx, const int flags)
{

	switch(buttonIdx)
	{
		case kImageDisplay_SaveHistBtn:
			SaveHistogram();
			break;

		case kImageDisplay_CrossHairChkBox:
			cDisplayCrossHair	=	!cDisplayCrossHair;
			CONSOLE_DEBUG_W_BOOL("cDisplayCrossHair\t=", cDisplayCrossHair);
			break;

		case kImageDisplay_CrossHair1Chk:
			cCurrCrossHairNum	=	0;
			break;

		case kImageDisplay_CrossHair2Chk:
			cCurrCrossHairNum	=	1;
			break;

		case kImageDisplay_CrossHair3Chk:
			cCurrCrossHairNum	=	2;
			break;

		case kImageDisplay_Reload:
			ReadCrossHairList();
			break;

		case kImageDisplay_Clear:
			if ((cCurrCrossHairNum >= 0) && (cCurrCrossHairNum < kCrossHairCnt))
			{
				cCrossHairPos[cCurrCrossHairNum].XLocation	=	0;
				cCrossHairPos[cCurrCrossHairNum].YLocation	=	0;
			}
			break;

		case kImageDisplay_Save:
			SaveCrossHairList();
			break;

		case kImageDisplay_RGB:
		case kImageDisplay_CMY:
		case kImageDisplay_OHS:
		case kImageDisplay_PGR:
		case kImageDisplay_BPO:
		case kImageDisplay_GRY:
			cCurrCrossHairColorSchm	=	(buttonIdx - kImageDisplay_RGB);
			break;

		default:
//			CONSOLE_DEBUG(__FUNCTION__);
			CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

			break;
	}
	UpdateButtons();
}

//*****************************************************************************
void	WindowTabImage::ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{

//	CONSOLE_DEBUG(__FUNCTION__);
	switch(widgetIdx)
	{
		case kImageDisplay_ImageDisplay:
//			CONSOLE_DEBUG("kImageDisplay_ImageDisplay");
			ZoomImage(event, xxx, yyy, flags);
			cMouseDragInProgress	=	false;
			break;

		default:
			//*	this adjusts the blue part of the image, just for testing.
			if (cOpenCVdisplayedImage != NULL)
			{
			int		iii;
			int		myImageSize;
		#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
				myImageSize	=	cOpenCVdisplayedImage->cols * cOpenCVdisplayedImage->rows * 3;
				for (iii= 0; iii< myImageSize; iii+=3)
				{
					cOpenCVdisplayedImage->data[iii]	+=	75;
				}
		#else
				myImageSize	=	cOpenCVdisplayedImage->imageSize;
				for (iii= 0; iii< myImageSize; iii+=3)
				{
					cOpenCVdisplayedImage->imageData[iii]	+=	75;
				}
		#endif // _USE_OPENCV_CPP_
				ForceWindowUpdate();
			}
			else
			{
				CONSOLE_DEBUG("cOpenCVdisplayedImage is NULL");
			}
			break;
	}
}

//*****************************************************************************
void	WindowTabImage::ProcessMouseLeftButtonDown(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)
{
int		cursorXXoffset;
int		cursorYYoffset;

//	CONSOLE_DEBUG("--------------------------------------------------------------------");
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("xxx\t=", xxx);
//	CONSOLE_DEBUG_W_NUM("yyy\t=", yyy);

	if (widgetIdx == kImageDisplay_ImageDisplay)
	{
//		CONSOLE_DEBUG_W_HEX("flags\t=", flags);
		if (flags & cv::EVENT_FLAG_CTRLKEY)
		{
			if ((cCurrCrossHairNum >= 0) && (cCurrCrossHairNum < kCrossHairCnt))
			{
				cCrossHairPos[cCurrCrossHairNum].XLocation	=	xxx - cWidgetList[kImageDisplay_ImageCrossHair].left;
				cCrossHairPos[cCurrCrossHairNum].YLocation	=	yyy - cWidgetList[kImageDisplay_ImageCrossHair].top;
				UpdateButtons();
			}
			ForceWindowUpdate();
		}
		else
		{
			cMouseDragInProgress	=	true;
			cursorXXoffset			=	xxx - cWidgetList[widgetIdx].roiRect.x;
			cursorYYoffset			=	yyy - cWidgetList[widgetIdx].roiRect.y;
	//		CONSOLE_DEBUG_W_NUM("cursorXXoffset\t=", cursorXXoffset);
	//		CONSOLE_DEBUG_W_NUM("cursorYYoffset\t=", cursorYYoffset);

			cSavedMouseClick_X		=	cursorXXoffset;
			cSavedMouseClick_Y		=	cursorYYoffset;

	//		CONSOLE_DEBUG_W_NUM("cSavedMouseClick_X\t=", cSavedMouseClick_X);
	//		CONSOLE_DEBUG_W_NUM("cSavedMouseClick_Y\t=", cSavedMouseClick_Y);
		}
	}
}

//*****************************************************************************
void	WindowTabImage::ProcessMouseLeftButtonUp(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cMouseDragInProgress	=	false;
}

//*****************************************************************************
void	WindowTabImage::ProcessMouseLeftButtonDragged(	const int	widgetIdx,
														const int	event,
														const int	xxx,
														const int	yyy,
														const int	flags)
{
int		deltaXX;
int		deltaYY;
int		cursorXXoffset;
int		cursorYYoffset;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cMouseDragInProgress)
	{
		cImageZoomState			=	1;
//		CONSOLE_DEBUG("--------------------------------------------------------");
//		CONSOLE_DEBUG_W_NUM(__FUNCTION__, xxx);

//		if (widgetIdx == kImageDisplay_ImageDisplay)
		{

			cursorXXoffset	=	xxx - cWidgetList[kImageDisplay_ImageDisplay].roiRect.x;
			cursorYYoffset	=	yyy - cWidgetList[kImageDisplay_ImageDisplay].roiRect.y;

			deltaXX			=	cursorXXoffset - cSavedMouseClick_X;
			deltaYY			=	cursorYYoffset - cSavedMouseClick_Y;

//			CONSOLE_DEBUG_W_NUM("deltaXX\t=", deltaXX);
//			CONSOLE_DEBUG_W_NUM("deltaYY\t=", deltaYY);

			cImageCenterX	-=	deltaXX;
			cImageCenterY	-=	deltaYY;

//			CONSOLE_DEBUG(__FUNCTION__);
			DrawFullScaleIamge(cImageCenterX, cImageCenterY);

			cSavedMouseClick_X	=	cursorXXoffset;
			cSavedMouseClick_Y	=	cursorYYoffset;

			ForceWindowUpdate();
		}
	}
}

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//**************************************************************************************
void	WindowTabImage::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, TYPE_WIDGET *theWidget, const int widgetIdx)
{
int				yDivideFactor;
ControllerImage	*myParentContolerImage;

//	CONSOLE_DEBUG(__FUNCTION__);

	cOpenCV_Image	=	openCV_Image;		//*	set the image data ptr so the window tabs can use it

	switch(widgetIdx)
	{
		case kImageDisplay_Histogram:
			myParentContolerImage	=	(ControllerImage *)cParentObjPtr;
			if (myParentContolerImage != NULL)
			{
				yDivideFactor	=	myParentContolerImage->cMaxHistogramPixCnt / theWidget->height;
				yDivideFactor	+=	1;

				DrawHistogram(theWidget,	myParentContolerImage->cHistogramLum, 256, yDivideFactor, CV_RGB(255, 255, 255));
				DrawHistogram(theWidget,	myParentContolerImage->cHistogramRed, 256, yDivideFactor, CV_RGB(255, 000, 000));
				DrawHistogram(theWidget,	myParentContolerImage->cHistogramGrn, 256, yDivideFactor, CV_RGB(000, 255, 000));
				DrawHistogram(theWidget,	myParentContolerImage->cHistogramBlu, 256, yDivideFactor, CV_RGB(0x64, 0x8c, 0xff));
			}
			else
			{
				CONSOLE_DEBUG("Parent object ptr is NULL");
				CONSOLE_ABORT(__FUNCTION__);
			}
			break;

		case kImageDisplay_ImageCrossHair:
			if (cDisplayCrossHair)
			{
				DrawCrossHairs(theWidget);
			}
			break;
	}
}
#else
//**************************************************************************************
void	WindowTabImage::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
{
	//*	this routine should be overloaded
}
#endif // _USE_OPENCV_CPP_


#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//*****************************************************************************
void	WindowTabImage::SetImagePtrs(cv::Mat *originalImage, cv::Mat *displayedImage)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cOpenCVdownLoadedImage	=	originalImage;
	cOpenCVdisplayedImage	=	displayedImage;
}
#else
//*****************************************************************************
void	WindowTabImage::SetImagePtrs(IplImage *originalImage, IplImage *displayedImage)
{
	cOpenCVdownLoadedImage	=	originalImage;
	cOpenCVdisplayedImage	=	displayedImage;
}
#endif // _USE_OPENCV_CPP_

//*****************************************************************************
void	WindowTabImage::ResetImage(void)
{
//int	rowStepSize;

//	CONSOLE_DEBUG(__FUNCTION__);
	SetWidgetText(	kImageDisplay_ImageDisplayInfo,	"Full image");
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
int	bytesPerPixel;
//	DumpCVMatStruct(__FUNCTION__, cOpenCVdownLoadedImage, __FUNCTION__);

	//*	Check to see if the original is color
//	rowStepSize		=	cOpenCVdownLoadedImage->step[0];
	bytesPerPixel	=	cOpenCVdownLoadedImage->step[1];
//	if ((cOpenCVdownLoadedImage->nChannels == 3) && (cOpenCVdownLoadedImage->depth == 8))
	if ((bytesPerPixel == 3))
	{
	//	cvResize(cOpenCVdownLoadedImage, cOpenCVdisplayedImage, CV_INTER_LINEAR);
		cv::resize(	*cOpenCVdownLoadedImage,
					*cOpenCVdisplayedImage,
					cOpenCVdisplayedImage->size(),
					0,
					0,
					cv::INTER_LINEAR);
	}
//	else if ((cOpenCVdownLoadedImage->nChannels == 1) && (cOpenCVdownLoadedImage->depth == 8))
	else if ((bytesPerPixel == 1))
	{
//-		cvCvtColor(cOpenCVdownLoadedImage, cOpenCVdisplayedImage, CV_GRAY2RGB);
		cv::cvtColor(*cOpenCVdownLoadedImage, *cOpenCVdisplayedImage, cv::COLOR_GRAY2BGR);
	}
#else
	//*	Check to see if the original is color
	if ((cOpenCVdownLoadedImage->nChannels == 3) && (cOpenCVdownLoadedImage->depth == 8))
	{
		cvResize(cOpenCVdownLoadedImage, cOpenCVdisplayedImage, CV_INTER_LINEAR);
	}
	else if ((cOpenCVdownLoadedImage->nChannels == 1) && (cOpenCVdownLoadedImage->depth == 8))
	{
		cvCvtColor(cOpenCVdownLoadedImage, cOpenCVdisplayedImage, CV_GRAY2RGB);
	}
#endif // _USE_OPENCV_CPP_
	cImageZoomState	=	0;
	ForceWindowUpdate();
}

//*****************************************************************************
void	WindowTabImage::ZoomImage(	const int	event,
									const int	xxx,
									const int	yyy,
									const int	flags)
{
int			cursorXXoffset;
int			cursorYYoffset;
int			displayedWidth;
int			displayedHeight;
int			sourceImageWidth;
int			sourceImageHeight;			//*	get the size of the destination image
int			imageCursorXX;
int			imageCursorYY;

//	CONSOLE_DEBUG(__FUNCTION__);

	if ((cOpenCVdownLoadedImage != NULL) && (cOpenCVdisplayedImage != NULL))
	{
		if (cImageZoomState != 0)
		{
			//*	set it back to fit on the screen
//			CONSOLE_DEBUG("Calling ResetImage()");
			ResetImage();
		}
		else
		{
			cursorXXoffset		=	xxx - cWidgetList[kImageDisplay_ImageDisplay].roiRect.x;
			cursorYYoffset		=	yyy - cWidgetList[kImageDisplay_ImageDisplay].roiRect.y;

		#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
			//*	get the size of the displayed image
			displayedWidth		=	cOpenCVdisplayedImage->cols;
			displayedHeight		=	cOpenCVdisplayedImage->rows;

			//*	get the size of the source image
			sourceImageWidth	=	cOpenCVdownLoadedImage->cols;
			sourceImageHeight	=	cOpenCVdownLoadedImage->rows;
		#else
			//*	get the size of the displayed image
			displayedWidth		=	cOpenCVdisplayedImage->width;
			displayedHeight		=	cOpenCVdisplayedImage->height;

			//*	get the size of the source image
			sourceImageWidth	=	cOpenCVdownLoadedImage->width;
			sourceImageHeight	=	cOpenCVdownLoadedImage->height;
		#endif // _USE_OPENCV_CPP_


			//*	calculate the relative position of the cursor WRT the full image
			imageCursorXX		=	sourceImageWidth * cursorXXoffset / displayedWidth;
			imageCursorYY		=	sourceImageHeight * cursorYYoffset / displayedHeight;

//			CONSOLE_DEBUG(__FUNCTION__);
			DrawFullScaleIamge(imageCursorXX, imageCursorYY);

			cImageZoomState	=	1;
		}

		//ForceWindowUpdate();
	}
}


//*****************************************************************************
void	WindowTabImage::DrawFullScaleIamge(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	DrawFullScaleIamge(cImageCenterX, cImageCenterY);
}


//*****************************************************************************
//*	draw the image at full scale centered on these image coordinates
//*****************************************************************************
void	WindowTabImage::DrawFullScaleIamge(const int image_X, const int	image_Y)
{
cv::Rect	displayedImgRect;
int			displayedWidth;
int			displayedHeight;
int			sourceImageWidth;
int			sourceImageHeight;
char		imageInfoText[80];

//	CONSOLE_DEBUG("--------------------------------------------------------------------");
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("image_X\t=", image_X);
//	CONSOLE_DEBUG_W_NUM("image_Y\t=", image_Y);

	sprintf(imageInfoText, "cx=%4d cy=%4d", image_X, image_Y);
	SetWidgetText(	kImageDisplay_ImageDisplayInfo,	imageInfoText);


	if ((cOpenCVdownLoadedImage != NULL) && (cOpenCVdisplayedImage != NULL))
	{
	#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)

		//*	get the size of the source image
		sourceImageWidth		=	cOpenCVdownLoadedImage->cols;
		sourceImageHeight		=	cOpenCVdownLoadedImage->rows;

		//*	get the size of the destination image
		displayedWidth			=	cOpenCVdisplayedImage->cols;
		displayedHeight			=	cOpenCVdisplayedImage->rows;
	#else
		//*	get the size of the source image
		sourceImageWidth		=	cOpenCVdownLoadedImage->width;
		sourceImageHeight		=	cOpenCVdownLoadedImage->height;

		//*	get the size of the destination image
		displayedWidth			=	cOpenCVdisplayedImage->width;
		displayedHeight			=	cOpenCVdisplayedImage->height;
	#endif // _USE_OPENCV_CPP_

//		CONSOLE_DEBUG_W_NUM("sourceImageWidth\t=",	sourceImageWidth);
//		CONSOLE_DEBUG_W_NUM("sourceImageHeight\t=",	sourceImageHeight);
//		CONSOLE_DEBUG_W_NUM("displayedWidth\t=",	displayedWidth);
//		CONSOLE_DEBUG_W_NUM("displayedHeight\t=",	displayedHeight);

		if ((displayedWidth > 0) && (displayedWidth < 10000)
			&& (displayedHeight > 0) && (displayedHeight < 10000))
		{
			displayedImgRect.width	=	displayedWidth;
			displayedImgRect.height	=	displayedHeight;

			//*	now set the top left of the image
			displayedImgRect.x		=	image_X - (displayedWidth / 2);
			displayedImgRect.y		=	image_Y - (displayedHeight / 2);

//			CONSOLE_DEBUG("First pass");
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.x\t\t=",		displayedImgRect.x);
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.y\t\t=",		displayedImgRect.y);

			//*	check minimums
			if (displayedImgRect.x < 0)
			{
				displayedImgRect.x	=	0;
			}
			if (displayedImgRect.y < 0)
			{
				displayedImgRect.y	=	0;
			}

			//*	check maximums
			if (displayedImgRect.x > (sourceImageWidth - displayedWidth))
			{
				displayedImgRect.x	=	(sourceImageWidth - displayedWidth);
			}

			if (displayedImgRect.y > (sourceImageHeight - displayedHeight))
			{
				displayedImgRect.y	=	(sourceImageHeight - displayedHeight);
			}
//			CONSOLE_DEBUG("Adjusted");
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.x\t\t=",		displayedImgRect.x);
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.y\t\t=",		displayedImgRect.y);
//
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.width\t=",	displayedImgRect.width);
//			CONSOLE_DEBUG_W_NUM("displayedImgRect.height\t=",	displayedImgRect.height);

		#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
cv::Mat		image_roi;
			CONSOLE_DEBUG("OpenCV++ not finished!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");

			image_roi	=	cv::Mat(*cOpenCVdownLoadedImage, displayedImgRect);

//			cvSetImageROI(cOpenCVdownLoadedImage,  theWidget->roiRect);
//			cvCopy(theWidget->openCVimagePtr, cOpenCVdownLoadedImage);
//			cvResetImageROI(cOpenCVdownLoadedImage);

			//---try------try------try------try------try------try---
			try
			{
				image_roi.copyTo(*cOpenCVdisplayedImage);
//				cv::waitKey(100);
			}
			catch(cv::Exception& ex)
			{
				//*	this catch prevents opencv from crashing
				CONSOLE_DEBUG("????????????????????????????????????????????????????");
				CONSOLE_DEBUG("copyTo() had an exception");
				CONSOLE_DEBUG_W_NUM("openCV error code\t=",	ex.code);
			//	CONSOLE_ABORT(__FUNCTION__);
			}

		#else
			//*	set the area we want to look at
			cvSetImageROI(cOpenCVdownLoadedImage,  displayedImgRect);
			//*	copy that part from the original to the displayed view
			cvCopy(cOpenCVdownLoadedImage, cOpenCVdisplayedImage);
			cvResetImageROI(cOpenCVdownLoadedImage);				//*	reset ROI
		#endif
			//*	now update the CURRENT center of the displayed image
			cImageCenterX	=	displayedImgRect.x + (displayedWidth / 2);
			cImageCenterY	=	displayedImgRect.y + (displayedHeight / 2);
			ForceWindowUpdate();
		}
		else
		{
			CONSOLE_DEBUG("Somethings not right!!!!!!!!!!!!");
			CONSOLE_DEBUG("cOpenCVdownLoadedImage is NULL or cOpenCVdisplayedImage is NULL");
		}
	}
	else
	{
		CONSOLE_DEBUG("Not finished");
//		CONSOLE_ABORT(__FUNCTION__);
	}
}

//*****************************************************************************
void	WindowTabImage::DrawCrossHairs(TYPE_WIDGET *theWidget)
{
int		crossHairXloc;
int		crossHairYloc;
int		iii;
int		firstColor;

//	CONSOLE_DEBUG(__FUNCTION__);

	switch(cCurrCrossHairColorSchm)
	{
		case (kImageDisplay_RGB - kImageDisplay_RGB):	firstColor	=	W_RED;			break;
		case (kImageDisplay_CMY - kImageDisplay_RGB):	firstColor	=	W_CYAN;			break;
		case (kImageDisplay_PGR - kImageDisplay_RGB):	firstColor	=	W_PURPLE;		break;
		case (kImageDisplay_OHS - kImageDisplay_RGB):	firstColor	=	W_FILTER_OIII;	break;
		case (kImageDisplay_BPO - kImageDisplay_RGB):	firstColor	=	W_BROWN;		break;
		case (kImageDisplay_GRY - kImageDisplay_RGB):	firstColor	=	W_LIGHTGRAY;	break;

		default:firstColor	=	W_RED;			break;
	}

	for (iii=0; iii<kCrossHairCnt; iii++)
	{
		if (cCrossHairPos[iii].XLocation > 0)
		{
			crossHairXloc	=	theWidget->left + cCrossHairPos[iii].XLocation;
			crossHairYloc	=	theWidget->top + cCrossHairPos[iii].YLocation;

			LLG_SetColor(firstColor + iii);
			//*	draw the vertical line
			LLG_MoveTo(crossHairXloc, theWidget->top);
			LLG_LineTo(crossHairXloc, theWidget->top + theWidget->height);

			//*	draw the horizontal line
			LLG_MoveTo(theWidget->left, crossHairYloc);
			LLG_LineTo(theWidget->left + theWidget->width, crossHairYloc);

			LLG_FrameEllipse(crossHairXloc, crossHairYloc, 20, 20);
		}
	}

	ForceWindowUpdate();
}

static const char	gCrossHairListFileName[]	=	"crosshairlist.txt";
//*****************************************************************************
void	WindowTabImage::SaveCrossHairList(void)
{
FILE	*filePointer;
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	filePointer	=	fopen(gCrossHairListFileName, "w");
	if (filePointer != NULL)
	{
		for (iii=0; iii<kCrossHairCnt; iii++)
		{
			if (cCrossHairPos[iii].XLocation > 0)
			{
				fprintf(filePointer, "%d\t%d\r\n",	cCrossHairPos[iii].XLocation, cCrossHairPos[iii].YLocation);
			}
		}
		fclose(filePointer);
	}
}

//*****************************************************************************
void	WindowTabImage::ReadCrossHairList(void)
{
FILE	*filePointer;
int		sLen;
char	lineBuff[128];
int		crossHairXloc;
int		crossHairYloc;
int		crossHairIndex;
char	*argPtr;

//	CONSOLE_DEBUG(__FUNCTION__);

	filePointer	=	fopen(gCrossHairListFileName, "r");
	if (filePointer != NULL)
	{
		crossHairIndex	=	0;
		while (fgets(lineBuff, 100, filePointer) && (crossHairIndex < kCrossHairCnt))
		{
			crossHairXloc	=	0;
			crossHairYloc	=	0;
			sLen			=	strlen(lineBuff);
			crossHairXloc	=	0;
			if ((lineBuff[0] != '#') && (sLen > 5))
			{
				crossHairXloc	=	atoi(lineBuff);
				argPtr			=	strchr(lineBuff, 0x09);
				if (argPtr != NULL)
				{
					argPtr++;
					crossHairYloc	=	atoi(argPtr);
				}

				if ((crossHairXloc > 0) && (crossHairYloc > 0))
				{
					cCrossHairPos[crossHairIndex].XLocation	=	crossHairXloc;
					cCrossHairPos[crossHairIndex].YLocation	=	crossHairYloc;
					crossHairIndex++;
				}
			}
		}
		fclose(filePointer);
	}
}

//*****************************************************************************
void	WindowTabImage::SaveHistogram(void)
{
ControllerImage	*myParentContolerImage;

	CONSOLE_DEBUG(__FUNCTION__);
	myParentContolerImage	=	(ControllerImage *)cParentObjPtr;
	if (myParentContolerImage != NULL)
	{
		myParentContolerImage->SaveHistogram();
	}
	else
	{
		CONSOLE_DEBUG("Parent object ptr is NULL");
		CONSOLE_ABORT(__FUNCTION__);
	}
}

#endif // _ENABLE_CTRL_IMAGE_
