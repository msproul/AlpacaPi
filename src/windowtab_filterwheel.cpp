//*****************************************************************************
//*		windowtab_filterwheel.cpp		(c) 2020 by Mark Sproul
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
//*	May 27,	2021	<MLS> Created windowtab_filterwheel.cpp
//*	May 29,	2021	<MLS> Filterwheel display working correctly
//*	Feb 20,	2022	<MLS> FilterWheel display working under opencv++
//*****************************************************************************

#define _ENABLE_CTRL_FILTERWHEEL_

#if defined(_ENABLE_CTRL_FILTERWHEEL_)

#include	<math.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"windowtab.h"
#include	"windowtab_filterwheel.h"
#include	"controller.h"


#define	kFilterCirleRadius	40


//**************************************************************************************
WindowTabFilterWheel::WindowTabFilterWheel(	const int	xSize,
											const int	ySize,
											cv::Scalar	backGrndColor,
											const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
int		iii;
//	CONSOLE_DEBUG(__FUNCTION__);

	cFilterWheelPropPtr	=	NULL;
	cPositionCount		=	0;
	for (iii=0; iii<kMaxFiltersPerWheel; iii++)
	{
		cFilterCirleCenterPt[iii].x	=	-10;
		cFilterCirleCenterPt[iii].y	=	-10;
	}

	SetupWindowControls();

}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabFilterWheel::~WindowTabFilterWheel(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}



//**************************************************************************************
void	WindowTabFilterWheel::SetupWindowControls(void)
{
int		yLoc;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------
	SetWidget(					kFilterWheel_Title,		0,			yLoc,		cWidth,		cTitleHeight);
	SetBGcolorFromWindowName(	kFilterWheel_Title);
	SetWidgetText(				kFilterWheel_Title, "AlpacaPi Filter Wheel");

	//*	setup the connected indicator
   	SetUpConnectedIndicator(kFilterWheel_Connected, yLoc);

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	yLoc			+=	2;
	yLoc			+=	2;

	//------------------------------------------
	SetWidget(					kFilterWheel_Name,		0,			yLoc,		cWidth,		cTitleHeight);
//	SetBGcolorFromWindowName(	kFilterWheel_Name);
	SetWidgetText(				kFilterWheel_Name, "");
	yLoc			+=	cTitleHeight;
	yLoc			+=	2;
	yLoc			+=	2;

	//==========================================
	//*	create the filter wheel
	SetWidget(		kFilterWheel_FilterCircle,	0,		yLoc,		(cWidth),	(cWidth + 8));
	SetWidgetType(	kFilterWheel_FilterCircle, kWidgetType_CustomGraphic);
	yLoc			+=	cWidth;
	yLoc			+=	2;

	//=======================================================
	//*	set up all the bottom stuff so that it is the same on all windowtabs
	SetupWindowBottomBoxes(	kFilterWheel_IPaddr,
							kFilterWheel_Readall,
							kFilterWheel_AlpacaErrorMsg,
							kFilterWheel_LastCmdString,
							kFilterWheel_AlpacaLogo,
							-1);
}

//*****************************************************************************
void	WindowTabFilterWheel::ProcessButtonClick(const int buttonIdx, const int flags)
{

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("buttonIdx\t=", buttonIdx);

	switch(buttonIdx)
	{
	}
}

//*****************************************************************************
void	WindowTabFilterWheel::ProcessDoubleClick(	const int	widgetIdx,
													const int	event,
													const int	xxx,
													const int	yyy,
													const int	flags)
{
int		iii;
int		deltaPixels_X;
int		deltaPixels_Y;
int		deltaPixels_Dist;
int		clickedFilter;
int		fwPosition;
char	dataString[128];
bool	validData;

//	CONSOLE_DEBUG(__FUNCTION__);
	switch(widgetIdx)
	{
		case kFilterWheel_FilterCircle:
			//*	see if we can figure out which filter was double clicked
			clickedFilter	=	-1;
			for (iii=0; iii < cPositionCount; iii++)
			{
				deltaPixels_X		=	xxx - cFilterCirleCenterPt[iii].x;
				deltaPixels_Y		=	yyy - cFilterCirleCenterPt[iii].y;
				deltaPixels_Dist	=	sqrt((deltaPixels_X * deltaPixels_X) + (deltaPixels_Y * deltaPixels_Y));
				if (deltaPixels_Dist <= kFilterCirleRadius)
				{
					//*	we have a valid clicked filter position
					CONSOLE_DEBUG_W_NUM("Clicked filter\t=",	iii);
					clickedFilter	=	iii;
				}
			}
			if (clickedFilter >= 0)
			{
				//*	we have a winner. Send the command to change to that filter
				fwPosition	=	clickedFilter + cFilterWheelPropPtr->Position;
				CONSOLE_DEBUG_W_NUM("fwPosition\t=",	fwPosition);
				if (fwPosition >= cPositionCount)
				{
					fwPosition	-=	cPositionCount;
				}
				CONSOLE_DEBUG_W_NUM("fwPosition\t=",	fwPosition);
				sprintf(dataString, "Position=%d", fwPosition);
				CONSOLE_DEBUG_W_STR("dataString\t=",	dataString);
				validData	=	AlpacaSendPutCmd(	"filterwheel", "position",	dataString);
				if (validData)
				{
					cFilterWheelPropPtr->IsMoving	=	true;
					ForceWindowUpdate();
				}
				else
				{
					CONSOLE_DEBUG("Error setting filter wheel position");
				}
				DisplayLastAlpacaCommand();
			}
			break;
	}
}

#ifdef _USE_OPENCV_CPP_
//**************************************************************************************
void	WindowTabFilterWheel::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
//**************************************************************************************
void	WindowTabFilterWheel::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif // _USE_OPENCV_CPP_
{
	cOpenCV_Image	=	openCV_Image;

	switch(widgetIdx)
	{
		case kFilterWheel_FilterCircle:
			DrawFilterWheel(&cWidgetList[widgetIdx]);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}


//**************************************************************************************
void	WindowTabFilterWheel::DrawFilterWheel(TYPE_WIDGET *theWidget)
{
int			center_X;
int			center_Y;
int			pt1_X;
int			pt1_Y;
int			pt2_X;
int			pt2_Y;
int			radius1;
double		degrees;
double		radians;
char		textString[64];
char		myFilterName[32];
int			filterPosition;		//*	0	= 12 o'clock
int			fillColor_Wnum;
int			textColor_Wnum;
double		deltaDegrees;
int			myFilterOffset;
int			textWidthPixels;


	center_X	=	theWidget->left + (theWidget->width / 2);
	center_Y	=	theWidget->top + (theWidget->height / 2);
	radius1		=	((theWidget->width / 2) -2);

	LLD_SetColor(W_WHITE);
	LLD_FrameEllipse(center_X, center_Y, radius1, radius1);

	if (cFilterWheelPropPtr != NULL)
	{
		if (cFilterWheelPropPtr->IsMoving)
		{
			strcpy(textString, "Moving");
		}
		else
		{
			strcpy(textString, "Double click to change filter");
		}

		textWidthPixels	=   LLD_GetTextSize(textString, kFont_Medium);
		pt1_X			=	center_X;
		pt1_X			-=	textWidthPixels / 2;
		pt1_Y			=	center_Y;
		LLD_DrawCString(pt1_X, pt1_Y, textString, kFont_Medium);
	}


	//*	do we have a valid count??
	if (cPositionCount > 0)
	{
		//*	now draw a circle for each filter
		deltaDegrees	=	360.0 / cPositionCount;
		radius1			=	(theWidget->width / 2) - 45;
		radius1			=	(theWidget->width / 2) - 45;
		for (filterPosition=0; filterPosition < cPositionCount; filterPosition++)
		{
			myFilterOffset	=	0;
			if (cFilterWheelPropPtr != NULL)
			{
				myFilterOffset	=	filterPosition + cFilterWheelPropPtr->Position;
				//*	figure out the index into the array for the one we are currently drawing
				if (myFilterOffset >= cPositionCount)
				{
					//*	handle wrap around
					myFilterOffset	-=	cPositionCount;
				}
				strcpy(myFilterName, cFilterWheelPropPtr->Names[myFilterOffset].FilterName);

				//*	figure out what color to make the background
				fillColor_Wnum	=	W_BLACK;
				textColor_Wnum	=	W_WHITE;		//*	default text color

				if (strncasecmp(myFilterName, "red", 3) == 0)
				{
					fillColor_Wnum	=	W_RED;
				}
				else if (strncasecmp(myFilterName, "green", 5) == 0)
				{
					fillColor_Wnum	=	W_GREEN;
					textColor_Wnum	=	W_BLACK;
				}
				else if (strncasecmp(myFilterName, "blue", 4) == 0)
				{
					fillColor_Wnum	=	W_BLUE;
				}
				//	Sii=R, Ha=G, Oiii=B.
				else if ((strncasecmp(myFilterName, "Sii", 3) == 0) || (strncasecmp(myFilterName, "S2", 2) == 0))
				{
					fillColor_Wnum	=	W_FILTER_SII;
				}
				else if (strncasecmp(myFilterName, "Ha", 2) == 0)
				{
					fillColor_Wnum	=	W_FILTER_HA;
					textColor_Wnum	=	W_BLACK;
				}
				else if (strncasecmp(myFilterName, "Oiii", 4) == 0)
				{
					fillColor_Wnum	=	W_FILTER_OIII;
					textColor_Wnum	=	W_BLACK;
				}
			}
			else
			{
				fillColor_Wnum	=	W_LIGHTGRAY;
				textColor_Wnum	=	W_BLACK;		//*	default text color
				sprintf(myFilterName, "-%d-", (myFilterOffset + 1));
			}

			degrees	=	-90.0 + (filterPosition * deltaDegrees);
			radians	=	degrees * M_PI / 180.0;

			pt1_X	=	center_X + (cos(radians) * radius1);
			pt1_Y	=	center_Y + (sin(radians) * radius1);

			//*	save the center of the filter circle
			//*	this is for the double click routine
			cFilterCirleCenterPt[filterPosition].x	=	pt1_X;
			cFilterCirleCenterPt[filterPosition].y	=	pt1_Y;

			LLD_SetColor(fillColor_Wnum);
			LLD_FillEllipse(pt1_X, pt1_Y, kFilterCirleRadius, kFilterCirleRadius);

			LLD_SetColor(W_WHITE);
			LLD_FrameEllipse(pt1_X, pt1_Y, kFilterCirleRadius, kFilterCirleRadius);

			//*	make position zero distinct
			if (filterPosition == 0)
			{
				LLD_PenSize(4);
				LLD_FrameEllipse(pt1_X, pt1_Y, kFilterCirleRadius, kFilterCirleRadius);
				LLD_PenSize(1);
			}

			//*	now work on the names and numbers
			textWidthPixels	=   LLD_GetTextSize(myFilterName, kFont_Medium);
			pt2_X			=	pt1_X - (textWidthPixels / 2);
			pt2_Y			=	pt1_Y + 5;
			LLD_SetColor(textColor_Wnum);
			LLD_DrawCString(pt2_X, pt2_Y, myFilterName, kFont_Medium);

			//*	now print the number of the filter above the name
			sprintf(textString, "-%d-", (myFilterOffset + 1));

			textWidthPixels	=   LLD_GetTextSize(myFilterName, kFont_Medium);
			pt2_X			=	pt1_X - (textWidthPixels / 2);
			pt2_Y			-=	15;

			LLD_DrawCString(pt2_X, pt2_Y, textString, kFont_Medium);

			degrees	+=	deltaDegrees;
		}
	}
}


//**************************************************************************************
void	WindowTabFilterWheel::SetPositonCount(int positionCount)
{
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("positionCount\t=", positionCount);
	cPositionCount	=	positionCount;
}

//**************************************************************************************
void	WindowTabFilterWheel::SetFilterWheelPropPtr(TYPE_FilterWheelProperties *fwProperties)
{
	CONSOLE_DEBUG(__FUNCTION__);

	cFilterWheelPropPtr	=	fwProperties;
}


#endif // _ENABLE_CTRL_FILTERWHEEL_

