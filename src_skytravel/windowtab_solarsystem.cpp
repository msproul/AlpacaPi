//*****************************************************************************
//*		windowtab_solarsystem.cpp		(c) 2024 by Mark Sproul
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jun  9,	2024	<MLS> Created windowtab_solarsystem.cpp
//*	Jun 10,	2024	<MLS> Solar system display working
//*	Jun 10,	2024	<MLS> Verified display is correct https://www.theplanetstoday.com/index.html
//*	Jun 11,	2024	<MLS> Added time adjustment key commands identical to SkyTravel window
//*	Jun 12,	2024	<MLS> Added DrawAsteriods() & DrawAstronomicalScale()
//*	Jun 12,	2024	<MLS> Added DrawCrossHairs()
//*	Jun 13,	2024	<MLS> Added DrawKuiperBelt()
//*	Jun 14,	2024	<MLS> Added DrawOrbits()
//*	Sep 29,	2024	<MLS> Added Kuiper Belt button ('k')
//*****************************************************************************
//
//	https://ssd.jpl.nasa.gov/planets/approx_pos.html
//	https://github.com/mayakraft/Kepler
//	https://www.theplanetstoday.com/index.html
//
//	https://ssd.jpl.nasa.gov/sats/elem/
//*****************************************************************************

#include	"controller.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"helper_functions.h"
#include	"windowtab.h"
#include	"windowtab_solarsystem.h"
//#include	"alpacadriver_helper.h"

#include	"alpacadriver_helper.h"
#include	"KeplerEquations.h"
#include	"SkyTravelTimeRoutines.h"

#define _USE_SKY_TRAVEL_TIME_

#define	kDefaultScaleFactor				10.0
#define	kInnerSolarSystemScaleFactor	200.0

//**************************************************************************************
WindowTabSolarSystem::WindowTabSolarSystem(	const int	xSize,
								const int	ySize,
								cv::Scalar	backGrndColor)
	:WindowTab(xSize, ySize, backGrndColor)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cScaleFactor		=	kDefaultScaleFactor;

	cDislayAsteriods	=	true;
	cDislayAUscale		=	true;
	cDislayCrossHairs	=	true;
	cDislayKuiperBelt	=	false;
	cDislayOrbit		=	false;
	cAsteroidMagLimit	=	13;

	Kepler_InitPlanets();

	SetupWindowControls();
	UpdateButtonStatus();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabSolarSystem::~WindowTabSolarSystem(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

#define	kGraphHeight	200
#define	kLabelHeight	25
//**************************************************************************************
void	WindowTabSolarSystem::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		iii;
int		skyBoxHeight;
int		buttonBoxWidth;

//	CONSOLE_DEBUG(__FUNCTION__);
	//------------------------------------------
	yLoc			=	cTabVertOffset;

	//------------------------------------------------------------------------------
	//*	set up all of the buttons
	xLoc			=	5;
	for (iii = kSolarSystem_Btn_First; iii <= kSolarSystem_AsteroidMagLimit; iii++)
	{
		buttonBoxWidth	=	cTitleHeight - 2;
		//*	we have some special cases
		switch(iii)
		{
			case kSolarSystem_UTCtime:
				buttonBoxWidth	=	(cTitleHeight * 10);
				break;

			case kSolarSystem_AsteroidMagLimit:
				buttonBoxWidth	=	(cTitleHeight * 3);
				break;
		}

		SetWidget(		iii,	xLoc,	yLoc,		buttonBoxWidth,		cTitleHeight);
		SetWidgetType(	iii, 	kWidgetType_Button);
		xLoc	+=	buttonBoxWidth;
		xLoc	+=	2;
	}
	yLoc	+=	cTitleHeight;
	yLoc	+=	2;

	SetWidgetText(kSolarSystem_Btn_Reset,			"r");
	SetWidgetText(kSolarSystem_Btn_AutoAdvTime,		"@");
	SetWidgetText(kSolarSystem_Btn_DispAsteriods,	"a");
	SetWidgetText(kSolarSystem_Btn_DispKuiperBelt,	"k");
	SetWidgetText(kSolarSystem_Btn_DispOrbit,		"o");
	SetWidgetText(kSolarSystem_Btn_DispAUscale,		"u");
	SetWidgetText(kSolarSystem_Btn_DispCrossHairs,	"x");

	SetWidgetType(kSolarSystem_UTCtime, 			kWidgetType_TextBox);
	SetWidgetFont(kSolarSystem_UTCtime, 			kFont_Medium);

	SetWidgetType(kSolarSystem_AsteroidMagLimit, 	kWidgetType_TextBox);
	SetWidgetFont(kSolarSystem_AsteroidMagLimit, 	kFont_Medium);

	//---------------------------------------------------------------------------
	//*	set the help text for the rest of them
	SetWidgetHelpText(	kSolarSystem_Btn_Reset,				"Reset");
	SetWidgetHelpText(	kSolarSystem_Btn_AutoAdvTime,		"Toggle Auto Advance Time");
	SetWidgetHelpText(	kSolarSystem_Btn_DispAsteriods,		"Toggle Asteroids Display");
	SetWidgetHelpText(	kSolarSystem_Btn_DispAUscale,		"Toggle Astronomical Unit Scale");
	SetWidgetHelpText(	kSolarSystem_Btn_DispOrbit,			"Toggle Orbit Display");
	SetWidgetHelpText(	kSolarSystem_Btn_DispKuiperBelt,	"Toggle Kuiper Belt objects");
	SetWidgetHelpText(	kSolarSystem_Btn_DispCrossHairs,	"Toggle Cross Hair Display");
	SetWidgetHelpText(	kSolarSystem_UTCtime,				"Time of display");
	SetWidgetHelpText(	kSolarSystem_AsteroidMagLimit,		"Magnitude limit for asteroid display");


	//-----------------------------------------------
	SetWidget(				kSolarSystem_MsgTextBox,	1,		yLoc,	(cWidth - 2),		cTitleHeight);
	SetWidgetType(			kSolarSystem_MsgTextBox,	kWidgetType_TextBox);
	SetWidgetFont(			kSolarSystem_MsgTextBox,	kFont_Medium);
	SetWidgetTextColor(		kSolarSystem_MsgTextBox,	CV_RGB(128,	128, 128));
	SetWidgetText(			kSolarSystem_MsgTextBox,	"message text box");
	SetWidgetJustification(	kSolarSystem_MsgTextBox,	kJustification_Left);

	SetHelpTextBoxNumber(	kSolarSystem_MsgTextBox);
	yLoc	+=	cTitleHeight;
	yLoc	+=	2;



	skyBoxHeight		=	cHeight - yLoc;

	SetWidget(				kSolarSystem_DisplayBox,	0,	yLoc,		cWidth,		skyBoxHeight);
	SetWidgetType(			kSolarSystem_DisplayBox, 	kWidgetType_CustomGraphic);
	SetWidgetBGColor(		kSolarSystem_DisplayBox,	CV_RGB(0,	0,		0));
	SetWidgetBorderColor(	kSolarSystem_DisplayBox,	CV_RGB(255,	255,	255));
	SetWidgetBorder(		kSolarSystem_DisplayBox,	true);


	cCenter_X	=	cWidgetList[kSolarSystem_DisplayBox].left + (cWidgetList[kSolarSystem_DisplayBox].width / 2);
	cCenter_Y	=	cWidgetList[kSolarSystem_DisplayBox].top + (cWidgetList[kSolarSystem_DisplayBox].height / 2);

}

//**************************************************************************************
void	WindowTabSolarSystem::ActivateWindow(void)
{
	UpdateButtonStatus();
}

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
//**************************************************************************************
void	WindowTabSolarSystem::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
//**************************************************************************************
void	WindowTabSolarSystem::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif // _USE_OPENCV_CPP_
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cOpenCV_Image	=	openCV_Image;
	switch(widgetIdx)
	{
		case kSolarSystem_DisplayBox:
			DrawSolarSystem(&cWidgetList[widgetIdx]);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}

//*****************************************************************************
void	WindowTabSolarSystem::HandleKeyDown(const int keyPressed)
{
//bool	controlKeyDown;
int		myChar;

	myChar			=	keyPressed & 0x00ffff;
//	controlKeyDown	=	false;
	if (keyPressed & 0x040000)
	{
//		controlKeyDown	=	true;
		myChar			=	myChar & 0x3f;
	}
//	CONSOLE_DEBUG_W_HEX("keyPressed\t=", keyPressed);
//	CONSOLE_DEBUG_W_HEX("myChar    \t=", myChar);

	switch(myChar)
	{
		case '@':
			gAutoAdvanceTime	=	!gAutoAdvanceTime;
			break;

		case 'A':
		case 'a':
			cDislayAsteriods	=	!cDislayAsteriods;
			break;

		case 'c':
		case 'C':
			cCenter_X	=	cWidgetList[kSolarSystem_DisplayBox].left + (cWidgetList[kSolarSystem_DisplayBox].width / 2);
			cCenter_Y	=	cWidgetList[kSolarSystem_DisplayBox].top + (cWidgetList[kSolarSystem_DisplayBox].height / 2);
			break;

		//*	set to Inner solar system
		case 'I':
		case 'i':
			cScaleFactor	=	kInnerSolarSystemScaleFactor;
			cCenter_X		=	cWidgetList[kSolarSystem_DisplayBox].left + (cWidgetList[kSolarSystem_DisplayBox].width / 2);
			cCenter_Y		=	cWidgetList[kSolarSystem_DisplayBox].top + (cWidgetList[kSolarSystem_DisplayBox].height / 2);
			break;

		case 'K':
		case 'k':
			cDislayKuiperBelt	=	!cDislayKuiperBelt;
			break;

		case 'O':
		case 'o':
			cDislayOrbit	=	!cDislayOrbit;
			break;

		case 'P':
		case 'p':
			gTransNeptunianCount++;
			if (gTransNeptunianCount > 5)
			{
				gTransNeptunianCount	=	0;
			}
			break;

		case 'R':
		case 'r':
			cScaleFactor		=	kDefaultScaleFactor;
			cDislayOrbit		=	false;
			gAutoAdvanceTime	=	true;
			cDislayCrossHairs	=	true;
			cDislayAUscale		=	true;
			ProcessTimeAdjustmentChar('=');
			//*	set back to center
			cCenter_X	=	cWidgetList[kSolarSystem_DisplayBox].left + (cWidgetList[kSolarSystem_DisplayBox].width / 2);
			cCenter_Y	=	cWidgetList[kSolarSystem_DisplayBox].top + (cWidgetList[kSolarSystem_DisplayBox].height / 2);
			break;

		case 'U':
		case 'u':
			cDislayAUscale	=	!cDislayAUscale;
			break;

		case 'X':
		case 'x':
			cDislayCrossHairs	=	!cDislayCrossHairs;
			break;

		case 0x0ffab:
		case '+':
			cAsteroidMagLimit	+=	0.25;
			if (cAsteroidMagLimit > 20)
			{
				cAsteroidMagLimit	=	20.0;
			}
			break;

		case 0x0ffad:
		case '-':
			cAsteroidMagLimit	-=	0.25;
			if (cAsteroidMagLimit < 0)
			{
				cAsteroidMagLimit	=	0.0;
			}
			break;

		case 0x0ff51:
			cCenter_X	+=  (cWidth / 10);
			break;

		case 0x0ff52:
			cCenter_Y	+=  (cWidth / 10);
			break;

		case 0x0ff53:
			cCenter_X	-=  (cWidth / 10);
			break;

		case 0x0ff54:
			cCenter_Y	-=  (cWidth / 10);
			break;


		case '=':	//*	Set to current time
		case '<':	//*	Back one hour
		case '>':	//*	Forward one hour
		case '[':	//*	Back one day
		case ']':	//*	Forward one day
		case '{':	//*	Back one month
		case '}':	//*	Forward one month
		case '(':	//*	Back one year
		case ')':	//*	Forward one year
		case '9':	//*	Back one minute
		case '0':	//*	Forward one minute
			ProcessTimeAdjustmentChar(keyPressed  & 0x07f);
			gAutoAdvanceTime	=	false;
			break;
	}
	UpdateButtonStatus();
	ForceWindowUpdate();
}

//*****************************************************************************
void	WindowTabSolarSystem::ProcessButtonClick(const int buttonIdx, const int flags)
{

	switch(buttonIdx)
	{
		case kSolarSystem_Btn_Reset:
			HandleKeyDown('r');
			break;

		case kSolarSystem_Btn_AutoAdvTime:
			HandleKeyDown('@');
			break;

		case kSolarSystem_Btn_DispAsteriods:
			HandleKeyDown('a');
			break;

		case kSolarSystem_Btn_DispKuiperBelt:
			HandleKeyDown('k');
			break;

		case kSolarSystem_Btn_DispOrbit:
			HandleKeyDown('o');
			break;

		case kSolarSystem_Btn_DispAUscale:
			HandleKeyDown('u');
			break;

		case kSolarSystem_Btn_DispCrossHairs:
			HandleKeyDown('x');
			break;

	}
	UpdateButtonStatus();
	ForceWindowUpdate();
}

//*****************************************************************************
void	WindowTabSolarSystem::ProcessDoubleClick(	const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	flags)
{
int		deltaX;
int		deltaY;

	switch(widgetIdx)
	{
		case kSolarSystem_DisplayBox:
			deltaX		=	xxx - cCenter_X;
			deltaY		=	yyy - cCenter_Y;
			cCenter_X	-=	deltaX;
			cCenter_Y	-=	deltaY;
//			cCenter_X	=	xxx;
//			cCenter_Y	=	yyy;
			break;
	}
	ForceWindowUpdate();
}

//*****************************************************************************
void	WindowTabSolarSystem::ProcessMouseWheelMoved(	const int	widgetIdx,
														const int	event,
														const int	xxx,
														const int	yyy,
														const int	wheelMovement,
														const int	flags)
{
double		moveAmount;

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, wheelMovement);
//	CONSOLE_DEBUG_W_HEX("flags\t=", flags);

	moveAmount	=	wheelMovement;
	//*	check to see if the control key is down
	if (flags & 0x08)
	{
		moveAmount	=	5 * moveAmount;
	}

	cScaleFactor	-=	moveAmount;
	if (cScaleFactor < 1)
	{
		cScaleFactor	=	1;
	}
	ForceWindowUpdate();
}

//**************************************************************************************
void	WindowTabSolarSystem::DrawSolarSystem(	TYPE_WIDGET	*graphWidget)
{
int		pt1_X;
int		pt1_Y;
int		iii;
int		myColors[]	=	{ W_CYAN, W_MAGENTA, W_YELLOW, W_ORANGE, W_PURPLE, W_RED, W_GREEN, W_PINK, W_PINK, W_PINK};
double	x, y, z;
char	textBuff[64];
double	orbitStepSize;
double	orbitLimit;
double	orbitYears;
int		stepCount;
bool	drawFlag;
double	date;
double	dayOfYear;
double	year;
double	sunRadius;

//	CONSOLE_DEBUG(__FUNCTION__);
	cDisplayTop		=	graphWidget->top;
	cDisplayLeft	=	graphWidget->left;
	cDisplayWidth	=	graphWidget->width;
	cDisplayHeight	=	graphWidget->height;

#ifdef _USE_SKY_TRAVEL_TIME_
int		daysOfMonth[12]	= {	31,	28,	31,	30,	31,	30,	31,	31,	30,	31,	30,	31};

	CalanendarTime(&gCurrentSkyTime);
	Local_Time(&gCurrentSkyTime);		//* compute local time from gmt and timezone

	year		=	gCurrentSkyTime.year;
	dayOfYear	=	0;
	for (iii=0; iii < (gCurrentSkyTime.month -1); iii++)
	{
		dayOfYear	+=	daysOfMonth[iii];
	}
	dayOfYear	+=	gCurrentSkyTime.day;
	dayOfYear	+=	gCurrentSkyTime.hour / 24.0;

//	CONSOLE_DEBUG_W_NUM("gCurrentSkyTime.month\t=", gCurrentSkyTime.month);
//	CONSOLE_DEBUG_W_DBL("dayOfYear            \t=", dayOfYear);

	sprintf(textBuff, "%02d/%02d/%02d U%02d:%02d:%02d - L%02d",
							gCurrentSkyTime.year,
							gCurrentSkyTime.month,
							gCurrentSkyTime.day,
							gCurrentSkyTime.hour,
							gCurrentSkyTime.min,
							gCurrentSkyTime.sec,
							gCurrentSkyTime.local_hour);
	SetWidgetText(kSolarSystem_UTCtime, textBuff);

#else
time_t		currentTime;
struct tm	myUtcTime;

	currentTime	=	time(NULL);
	myUtcTime	=	*gmtime(&currentTime);
	year		=	1900 + myUtcTime.tm_year;
	dayOfYear	=	myUtcTime.tm_yday;
#endif
	//	date = (year-2000)/100 + day_of_the_year/36525
	date		=	((year - 2000.0) / 100.0) + ((dayOfYear + 1) / 36525.0);

	if (cDislayKuiperBelt)
	{
		if (cScaleFactor <= 7.0)
		{
			DrawKuiperBelt();
		}
		if (gTransNeptunianData != NULL)
		{
//			DrawOrbits(gTransNeptunianData, 5, -1);
			DrawOrbits(gTransNeptunianData, gTransNeptunianCount, -1);
		}
	}

	if (cDislayCrossHairs)
	{
		DrawCrossHairs();
	}

	sunRadius	=	cScaleFactor * 0.0046524726;
	if (sunRadius < 3)
	{
		sunRadius	=	3;
	}
	LLG_SetColor(W_YELLOW);
	LLG_FillEllipse(cCenter_X, cCenter_Y, sunRadius, sunRadius, 0.0);

	//---------------------------------------------------------
	//*	draw the orbit ellipses
	if (cDislayOrbit)
	{
		DrawOrbits(gPlanetKepData, kPlanet_Last);
	}

	for (iii=0; iii<kPlanet_Last; iii++)
	{
		LLG_SetColor(myColors[iii]);

		orbitYears		=   gPlanetKepData[iii].oribit_days / 365.25;
		orbitStepSize	=   orbitYears / 20000;
		if (orbitStepSize < 0.000001)
		{
			orbitStepSize	=	0.000001;
		}
		date		=	((year - 2000.0) / 100.0) + ((dayOfYear + 1) / 36525.0);
		orbitLimit	=	date + (orbitYears / 100.0);
//		orbitLimit	+=	(orbitYears / 1000.0);	//*	a little bit father to complete the circle

//		CONSOLE_DEBUG_W_STR("Planet-------\t=",	gPlanetKepData[iii].name);
//		CONSOLE_DEBUG_W_DBL("orbitYears   \t=",	orbitYears);
//		CONSOLE_DEBUG_W_DBL("date         \t=",	date);
//		CONSOLE_DEBUG_W_DBL("orbitLimit   \t=",	orbitLimit);
//		CONSOLE_DEBUG_W_DBL("orbitStepSize\t=",	orbitStepSize);

		drawFlag	=	false;
		stepCount	=	0;
		while ((date <= orbitLimit) || (stepCount < 100))
		{
			Kepler_CalculatePlanetLocation(&gPlanetKepData[iii], date, &x, &y, &z);
			pt1_X	=	cCenter_X + (cScaleFactor * gPlanetKepData[iii].planet_x);
			pt1_Y	=	cCenter_Y - (cScaleFactor * gPlanetKepData[iii].planet_y);

			if (drawFlag && (pt1_Y >= graphWidget->top))
			{
				LLG_LineTo(pt1_X, pt1_Y);
			}
			else
			{
				LLG_MoveTo(pt1_X, pt1_Y);
				drawFlag	=	true;
			}
			date	+=	orbitStepSize;
			stepCount++;
		}
//		CONSOLE_DEBUG_W_NUM("stepCount    \t=",	stepCount);
	}

	//*	draw the planets for today
	date	=	((year - 2000.0) / 100.0) + ((dayOfYear + 1) / 36525.0);

	for (iii=0; iii<kPlanet_Last; iii++)
	{
		Kepler_CalculatePlanetLocation(&gPlanetKepData[iii], date, &x, &y, &z);
		pt1_X	=	cCenter_X + (cScaleFactor * gPlanetKepData[iii].planet_x);
		pt1_Y	=	cCenter_Y - (cScaleFactor * gPlanetKepData[iii].planet_y);
		if (pt1_Y >= graphWidget->top)
		{
			LLG_SetColor(myColors[iii]);
			LLG_FillEllipse(pt1_X, pt1_Y, 5, 5, 0.0);
			LLG_DrawCString(pt1_X + 5, pt1_Y, gPlanetKepData[iii].name);
		}
	}
	sprintf(textBuff, "Scale Factor = %3.0f (pixels per AU)", cScaleFactor);
	LLG_SetColor(W_WHITE);
	LLG_DrawCString(graphWidget->left + 10, graphWidget->top + 30, textBuff);

	//-----------------------------------------------------------------
	//*	Astronomical Units Scale
	if (cDislayAUscale)
	{
		DrawAstronomicalScale();
	}
	//-----------------------------------------------------------------
	if (cDislayAsteriods)
	{
		DrawAsteriods();
	}
}

//*****************************************************************************
void	WindowTabSolarSystem::DrawCrossHairs(void)
{
int		pt1_X;
int		pt1_Y;

	LLG_SetColor(W_DARKGRAY);
	//*	draw the horizontal line
	pt1_X	=	cDisplayLeft;
	pt1_Y	=	cCenter_Y;
	LLG_MoveTo(pt1_X, pt1_Y);

	pt1_X	+=	cDisplayWidth;
	LLG_LineTo(pt1_X, pt1_Y);


	//*	draw the vertical line
	pt1_X	=	cCenter_X;
	pt1_Y	=	cDisplayTop;
	LLG_MoveTo(pt1_X, pt1_Y);

	pt1_Y	+=	cDisplayHeight;
	LLG_LineTo(pt1_X, pt1_Y);
}

//*****************************************************************************
void	WindowTabSolarSystem::DrawAstronomicalScale(void)
{
double		stepFactor;
double		auValue;
int			pt1_X;
int			pt1_Y;
char		textBuff[64];

	//*	figure out how much to step for each tick mark
	if (cScaleFactor >= 2200)
	{
		stepFactor	=	0.02;
	}
	else if (cScaleFactor >= 1000)
	{
		stepFactor	=	0.05;
	}
	else if (cScaleFactor >= 600)
	{
		stepFactor	=	0.1;
	}
	else if (cScaleFactor >= 300)
	{
		stepFactor	=	0.25;
	}
	else if (cScaleFactor >= 150)
	{
		stepFactor	=	0.5;
	}
	else if (cScaleFactor >= 30)
	{
		stepFactor	=	1;
	}
	else if (cScaleFactor >= 15)
	{
		stepFactor	=	2;
	}
	else if (cScaleFactor >= 7)
	{
		stepFactor	=	5;
	}
	else if (cScaleFactor >= 2)
	{
		stepFactor	=	25;
	}
	else
	{
		stepFactor	=	50;
	}

	LLG_SetColor(W_DARKGRAY);
	//----------------------------------------------------------
	//*	draw tick marks on horizontal axis
	pt1_X		=	cCenter_X;
	pt1_Y		=	cCenter_Y;
	auValue		=	0;
	while (pt1_X < 	cWidth)
	{
		//*	figure out what type of number to display
		if (cScaleFactor >= 150)
		{
			sprintf(textBuff, "%2.2f", auValue);
		}
		else
		{
			sprintf(textBuff, "%1.0f", auValue);
		}

		pt1_X	=	cCenter_X + (auValue * cScaleFactor);
		LLG_MoveTo(pt1_X, pt1_Y);
		LLG_LineTo(pt1_X, pt1_Y + 10);
		LLG_DrawCString(pt1_X - 5, pt1_Y + 20, textBuff);

		auValue	+=	stepFactor;
	}
	//----------------------------------------------------------
	//*	draw tick marks on horizontal axis
	//*	lets do the left side as well
	pt1_X		=	cCenter_X;
	pt1_Y		=	cCenter_Y;
	auValue		=	stepFactor;	//*	dont bother with location zero, it was done already
	while (pt1_X > 	0)
	{
		//*	figure out what type of number to display
		if (cScaleFactor >= 150)
		{
			sprintf(textBuff, "%2.2f", auValue);
		}
		else
		{
			sprintf(textBuff, "%1.0f", auValue);
		}

		//----------------------------------------------
		//*	lets do the left side as well
		pt1_X	=	cCenter_X - (auValue * cScaleFactor);
		LLG_MoveTo(pt1_X, pt1_Y);
		LLG_LineTo(pt1_X, pt1_Y + 10);
		LLG_DrawCString(pt1_X - 5, pt1_Y + 20, textBuff);

		auValue	+=	stepFactor;
	}

	//----------------------------------------------------------
	//*	draw tick marks on vertical axis
	pt1_X		=	cCenter_X;
	pt1_Y		=	cCenter_Y;
	auValue		=	stepFactor;	//*	dont bother with location zero, it was done already
	while (pt1_Y > 0)
	{
		//*	figure out what type of number to display
		if (cScaleFactor >= 150)
		{
			sprintf(textBuff, "%2.2f", auValue);
		}
		else
		{
			sprintf(textBuff, "%1.0f", auValue);
		}

		pt1_Y	=	cCenter_Y - (auValue * cScaleFactor);
		if (pt1_Y > cDisplayTop)
		{
			LLG_MoveTo(pt1_X, pt1_Y);
			LLG_LineTo(pt1_X + 10, pt1_Y);
			LLG_DrawCString(pt1_X + 5, pt1_Y, textBuff);
		}
		auValue	+=	stepFactor;
	}

	//----------------------------------------------------------
	//*	draw tick marks on the bottom vertical axis
	pt1_X		=	cCenter_X;
	pt1_Y		=	cCenter_Y;
	auValue		=	stepFactor;	//*	dont bother with location zero, it was done already
	while (pt1_Y < (cDisplayTop + cDisplayHeight))
	{
		//*	figure out what type of number to display
		if (cScaleFactor >= 150)
		{
			sprintf(textBuff, "%2.2f", auValue);
		}
		else
		{
			sprintf(textBuff, "%1.0f", auValue);
		}

		pt1_Y	=	cCenter_Y + (auValue * cScaleFactor);
		if (pt1_Y > cDisplayTop)
		{
			LLG_MoveTo(pt1_X, pt1_Y);
			LLG_LineTo(pt1_X + 10, pt1_Y);
			LLG_DrawCString(pt1_X + 5, pt1_Y, textBuff);
		}
		auValue	+=	stepFactor;
	}

}


//*****************************************************************************
void	WindowTabSolarSystem::DrawAsteriods(void)
{
int		iii;
int		pt1_X;
int		pt1_Y;
//double	x, y, z;


	if ((gAsteriodData != NULL) && (gAsteroidCount > 0))
	{
		for (iii=0; iii<gAsteroidCount; iii++)
		{
//			Kepler_CalculatePlanetLocation(&gPlanetKepData[iii], date, &x, &y, &z);
			if (gAsteriodData[iii].magnitude < cAsteroidMagLimit)
			{
				pt1_X	=	cCenter_X + (cScaleFactor * gAsteriodData[iii].planet_x);
				pt1_Y	=	cCenter_Y - (cScaleFactor * gAsteriodData[iii].planet_y);
				if (pt1_Y >= cDisplayTop)
				{
					LLG_Putpixel(pt1_X, pt1_Y, W_CYAN);
					if ((cScaleFactor >= 200) || (iii < 100))
					{
						LLG_DrawCString(pt1_X + 5, pt1_Y, gAsteriodData[iii].name);
					}
				}
			}
		}
	}
}

//*****************************************************************************
//*	The Kuiper belt is a circumstellar disc in the outer Solar System,
//*	extending from the orbit of Neptune at 30 astronomical units (AU)
//*	to approximately 50 AU from
//*****************************************************************************
void	WindowTabSolarSystem::DrawKuiperBelt(void)
{
int			kuiperBeltRadiusOuter;
int			kuiperBeltRadiusInner;
cv::Scalar	darkestGray	=	CV_RGB(17, 17, 17);
int			pt1_X;
int			pt1_Y;

	kuiperBeltRadiusInner	=	30 * cScaleFactor;
	kuiperBeltRadiusOuter	=	50 * cScaleFactor;
	LLG_SetColor(darkestGray);
	LLG_FillEllipse(cCenter_X, cCenter_Y, kuiperBeltRadiusOuter, kuiperBeltRadiusOuter, 0.0);

	LLG_SetColor(W_BLACK);
	LLG_FillEllipse(cCenter_X, cCenter_Y, kuiperBeltRadiusInner, kuiperBeltRadiusInner, 0.0);

	pt1_X	=	cCenter_X + (0.7071 * kuiperBeltRadiusOuter);
	pt1_Y	=	cCenter_Y - (0.7071 * kuiperBeltRadiusOuter);
	if (pt1_Y < cDisplayTop)
	{
		pt1_Y	=	cCenter_Y + (0.7071 * kuiperBeltRadiusOuter);
	}
	LLG_SetColor(W_WHITE);
	LLG_DrawCString(pt1_X, pt1_Y, "Kuiper belt");
}


//*****************************************************************************
void	WindowTabSolarSystem::DrawOrbits(TYPE_KeplerData *objectData, const int objectCount, const int color)
{
int		iii;
int		pt1_X;
int		pt1_Y;
double	majorAxis;
double	minorAxis;
int		majorRadius;
int		minorRadius;
double	inclination_deg;
double	qqq_minor;
double	QQQ_major;
double	ellipseCenterOffset_Radius;
double	ellipseCenterOffset_X;
double	ellipseCenterOffset_Y;
int		myCenter_X;
int		myCenter_Y;
int		myColors[5]	=	{W_CYAN, W_MAGENTA, W_YELLOW, W_GREEN, W_WHITE};

	for (iii=0; iii<objectCount; iii++)
	{
		if (color >= 0)
		{
			LLG_SetColor(color);
		}
		else
		{
			LLG_SetColor(myColors[iii % 5]);
		}
		myCenter_X	=	cCenter_X;
		myCenter_Y	=	cCenter_Y;
		majorAxis	=	objectData[iii].a;
		minorAxis	=	majorAxis * (1.0 - objectData[iii].e);

		//*	check for offset orbit
		qqq_minor	=	objectData[iii].elem_q;
		QQQ_major	=	objectData[iii].elem_Q;

		majorRadius	=	(cScaleFactor * majorAxis);
		minorRadius	=	(cScaleFactor * minorAxis);
		if ((majorRadius > 0) && (minorRadius > 0))
		{
			inclination_deg	=	objectData[iii].elem_I;
//			inclination_deg	-=	180.0;
			inclination_deg	+=	90.0;
			if (inclination_deg > 360.0)
			{
				inclination_deg	-=	360.0;
			}
			if (inclination_deg < 0.0)
			{
				inclination_deg	+=	360.0;
			}
			if ((qqq_minor > 0.0) && (QQQ_major > 0.0))
			{
				//*	we have an offset orbit
				ellipseCenterOffset_Radius	=	(QQQ_major - qqq_minor) / 2.0;
				ellipseCenterOffset_X		=	cos(RADIANS(inclination_deg)) * ellipseCenterOffset_Radius;
				ellipseCenterOffset_Y		=	sin(RADIANS(inclination_deg)) * ellipseCenterOffset_Radius;

				myCenter_X	=	cCenter_X + (ellipseCenterOffset_X * cScaleFactor);
				myCenter_Y	=	cCenter_Y + (ellipseCenterOffset_Y * cScaleFactor);

				pt1_X	=	myCenter_X + (cos(RADIANS(inclination_deg)) * majorRadius);
				pt1_Y	=	myCenter_Y + (sin(RADIANS(inclination_deg)) * majorRadius);
			}
			else
			{
				//*	location for the name text
				pt1_X	=	cCenter_X + majorRadius;
				pt1_Y	=	cCenter_Y;
			}

			LLG_FrameEllipse(myCenter_X, myCenter_Y, majorRadius, minorRadius, inclination_deg);

			if (strlen(objectData[iii].name) > 0)
			{
				LLG_DrawCString(pt1_X, pt1_Y, objectData[iii].name);
			}
		}
	}
}

//*****************************************************************************
void	WindowTabSolarSystem::UpdateButtonStatus(void)
{

	SetWidgetChecked(	kSolarSystem_Btn_AutoAdvTime,		gAutoAdvanceTime);

	SetWidgetChecked(	kSolarSystem_Btn_DispAsteriods,		cDislayAsteriods);
	SetWidgetChecked(	kSolarSystem_Btn_DispAUscale,		cDislayAUscale);
	SetWidgetChecked(	kSolarSystem_Btn_DispOrbit,			cDislayOrbit);
	SetWidgetChecked(	kSolarSystem_Btn_DispKuiperBelt,	cDislayKuiperBelt);

	SetWidgetChecked(	kSolarSystem_Btn_DispCrossHairs,	cDislayCrossHairs);

	SetWidgetNumber(	kSolarSystem_AsteroidMagLimit,		cAsteroidMagLimit, 2);


}



