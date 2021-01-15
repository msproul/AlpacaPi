//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar 23,	2020	<MLS> Added crossedOut to widget struct
//*	Jun 10,	2020	<MLS> Increased kMaxWidgets from 75 to 125
//*	Jun 15,	2020	<MLS> Increased kMaxWidgets from 125 to 150
//*****************************************************************************

//#include	"widget.h"

#ifndef	_WIDGET_H_
#define	_WIDGET_H_

#ifndef __OPENCV_OLD_HIGHGUI_H__
	#include "opencv/highgui.h"
#endif
#ifndef __OPENCV_HIGHGUI_H__
	#include "opencv2/highgui/highgui_c.h"
#endif


#define	kMaxWidgets	150


//*****************************************************************************
enum
{
	kWidgetType_Default	=	0,

	kWidgetType_Button,
	kWidgetType_CheckBox,
	kWidgetType_Custom,
	kWidgetType_Graph,
	kWidgetType_Graphic,
	kWidgetType_Icon,
	kWidgetType_Image,
	kWidgetType_MultiLineText,
	kWidgetType_OutlineBox,
	kWidgetType_RadioButton,
	kWidgetType_ProessBar,
	kWidgetType_ScrollBar,
	kWidgetType_Slider,
	kWidgetType_Text,
	kWidgetType_TextInput,

	kWidgetType_Last
};

//*****************************************************************************
enum
{
	kFont_Large	=	0,
	kFont_Medium,
	kFont_RadioBtn,
	kFont_Small,
//	kFont_Simplex,

};

//*****************************************************************************
enum
{
	kJustification_Left	=	0,
	kJustification_Center,
	kJustification_Right,

	kJustification_last
};


//*****************************************************************************
enum
{
	kIcon_Undefined	=	0,
	kIcon_UpArrow,
	kIcon_DownArrow,

	kIcon_last
};

#define	kMaxWidgetStrLen	512
#define	kMaxHelpTextStrLen	48
//*****************************************************************************
typedef struct
{
	bool		valid;
	bool		crossedOut;
	bool		highLighted;		//*	only valid for buttons
	bool		needsUpdated;		//*	this is for selective updating
	int			widgetType;
	int			left;
	int			top;
	int			width;
	int			height;
	int			justification;
	int			iconNum;
	char		textString[kMaxWidgetStrLen];
	char		helpText[kMaxHelpTextStrLen];
	CvScalar	bgColor;
	CvScalar	textColor;
	CvScalar	borderColor;
	bool		includeBorder;
	int			fontNum;
	IplImage	*openCVimagePtr;
	bool		selected;			//*	radio buttons and check boxes
	bool		highLiteEnabled;	//*	true if widget supports highlighting
	double		sliderValue;		//*	uses for sliders, scroll bars, and progress bars
	double		sliderMin;
	double		sliderMax;

	double		*graphArrayPtr;
	int			graphArrayCnt;
} TYPE_WIDGET;

bool	PointInWidget(const int xPoint, const int yPoint, TYPE_WIDGET *theWidget);
void	DumpWidget(TYPE_WIDGET *theWidget);
#endif // _WIDGET_H_
