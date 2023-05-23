//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar 23,	2020	<MLS> Added crossedOut to widget struct
//*	Jun 10,	2020	<MLS> Increased kMaxWidgets from 75 to 125
//*	Jun 15,	2020	<MLS> Increased kMaxWidgets from 125 to 150
//*	Jan 15,	2021	<MLS> Added Tab stops to widget definition
//*	Jan 16,	2021	<MLS> Added font size kFont_TextList
//*	Jan 17,	2021	<MLS> Added textPtr to widget
//*	Feb 21,	2022	<MLS> Changed kWidgetType_Graphic to kWidgetType_CustomGraphic
//*	Feb 21,	2022	<MLS> Deleted kWidgetType_Custom
//*****************************************************************************

//#include	"widget.h"

#ifndef	_WIDGET_H_
#define	_WIDGET_H_

#include	<opencv2/opencv.hpp>

#ifndef _USE_OPENCV_CPP_
	#include	"opencv2/highgui/highgui_c.h"
	#include	"opencv2/imgproc/imgproc_c.h"
#endif // _USE_OPENCV_CPP_


#define	kMaxWidgets	150


//*****************************************************************************
enum
{
	kWidgetType_Default	=	0,

	kWidgetType_Button,
	kWidgetType_CheckBox,
	kWidgetType_Graph,
	kWidgetType_CustomGraphic,
	kWidgetType_Custom,			//*	custom everything, does NOT erase the background
	kWidgetType_Icon,
	kWidgetType_Image,
	kWidgetType_MultiLineText,
	kWidgetType_OutlineBox,
	kWidgetType_RadioButton,
	kWidgetType_ProessBar,
	kWidgetType_ScrollBar,
	kWidgetType_Slider,
	kWidgetType_TextBox,
	kWidgetType_TextInput,

	kWidgetType_Disabled,

	kWidgetType_Last
};

//*****************************************************************************
enum
{
	kFont_Small	=	0,
	kFont_RadioBtn,
	kFont_TextList,
	kFont_Medium,
	kFont_Large,

	kFont_MonoSpace,

//	kFont_Simplex,
	kFont_Triplex_Small,
	kFont_Triplex_Large,
	kFont_Script_Large,

	kFont_last


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
#define	kMaxHelpTextStrLen	96
#define	kAltTextLen			32
#define	kMaxTabStops		15
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
	bool		hasTabs;
	short		tabStops[kMaxTabStops];
	char		textString[kMaxWidgetStrLen];
	char		alternateText[kAltTextLen];		//*	used for progress bar message (can be used for other things)
	char		helpText[kMaxHelpTextStrLen];
	char		*textPtr;			//*	this is for large external text
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	cv::Mat		*openCVimagePtr;
	cv::Rect	roiRect;
	cv::Scalar	bgColor;
	cv::Scalar	textColor;
	cv::Scalar	borderColor;
#else
	IplImage	*openCVimagePtr;
	CvRect		roiRect;
	CvScalar	bgColor;
	CvScalar	textColor;
	CvScalar	borderColor;
#endif // _USE_OPENCV_CPP_
	bool		includeBorder;
	int			fontNum;
	bool		selected;			//*	radio buttons and check boxes
	bool		highLiteEnabled;	//*	true if widget supports highlighting
	double		sliderValue;		//*	uses for sliders, scroll bars, and progress bars
	double		sliderMin;
	double		sliderMax;

	//*	the scroll bar value is between 0 and (max - 2)
	int			scrollBarValue;
	int			scrollBarMax;
	int			scrollBarLines;


	double		*graphArrayPtr;
	int			graphArrayCnt;
} TYPE_WIDGET;

bool	PointInWidget(const int xPoint, const int yPoint, TYPE_WIDGET *theWidget);
void	DumpWidget(TYPE_WIDGET *theWidget);
#endif // _WIDGET_H_
