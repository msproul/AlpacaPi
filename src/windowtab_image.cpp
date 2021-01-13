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
//*****************************************************************************

#ifdef _ENABLE_IMAGE_


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"

#include	"controller.h"
#include	"controller_image.h"

#include	"windowtab.h"
#include	"windowtab_image.h"




//**************************************************************************************
WindowTabImage::WindowTabImage(	const int	xSize,
									const int	ySize,
									CvScalar	backGrndColor,
									const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
	CONSOLE_DEBUG(__FUNCTION__);

	cOpenCVdownLoadedImage	=	NULL;

	SetupWindowControls();

}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabImage::~WindowTabImage(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	if (cOpenCVdownLoadedImage != NULL)
	{
		CONSOLE_DEBUG("destroy old image");
		SetWidgetImage(kImageDisplay_ImageDisplay, NULL);
		cvReleaseImage(&cOpenCVdownLoadedImage);
		cOpenCVdownLoadedImage	=	NULL;
	}
}


//**************************************************************************************
void	WindowTabImage::SetupWindowControls(void)
{
int			yLoc;
int			yLocSave;
int			iii;
IplImage	*logoImage;

//*	create our own set of column offsets
int		myClm1_offset;
int		myClm2_offset;
int		myClm3_offset;
int		myClm4_offset;
int		myClm5_offset;
int		myClm6_offset;
//int		myClm7_offset;
//int		myClm8_offset;
//int		myClm9_offset;
//int		myClm10_offset;
//int		myClm11_offset;
//int		myClm12_offset;
int		myClmWidth;

	myClmWidth		=	cWidth / 12;
	myClmWidth		-=	2;
	myClm1_offset	=	3;
	myClm2_offset	=	1 * myClmWidth;
	myClm3_offset	=	2 * myClmWidth;
	myClm4_offset	=	3 * myClmWidth;
	myClm5_offset	=	4 * myClmWidth;
	myClm6_offset	=	5 * myClmWidth;
//	myClm7_offset	=	6 * myClmWidth;
//	myClm8_offset	=	7 * myClmWidth;
//	myClm9_offset	=	8 * myClmWidth;
//	myClm10_offset	=	9 * myClmWidth;
//	myClm11_offset	=	10 * myClmWidth;
//	myClm12_offset	=	11 * myClmWidth;


//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(kImageDisplay_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetWidgetText(kImageDisplay_Title, "AlpacaPi project");
	SetBGcolorFromWindowName(kImageDisplay_Title);
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;



	SetWidget(				kImageDisplay_ImageDisplay,	0,	yLoc,		cWidth,		((3 * cWidth) / 7));
	SetWidgetBGColor(		kImageDisplay_ImageDisplay,	CV_RGB(128,	128,	128));
	SetWidgetBorderColor(	kImageDisplay_ImageDisplay,	CV_RGB(255,	255,	255));
	SetWidgetBorder(		kImageDisplay_ImageDisplay,	true);

}

//*****************************************************************************
void	WindowTabImage::ProcessButtonClick(const int buttonIdx)
{

	switch(buttonIdx)
	{

		default:
			CONSOLE_DEBUG(__FUNCTION__);
			CONSOLE_DEBUG_W_NUM("buttonIdx\t",	buttonIdx);

			break;

	}
}

#endif // _ENABLE_IMAGE_
