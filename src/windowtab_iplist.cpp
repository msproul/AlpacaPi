//*****************************************************************************
//*		windowtab_iplist.cpp		(c) 2021 by Mark Sproul
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
//*	Feb  5,	2021	<MLS> Created windowtab_iplist.cpp
//*	Aug  6,	2022	<MLS> Added CPU temperature graph
//*	Aug  6,	2022	<MLS> Added DrawCpuTempGraph()
//*	Aug  7,	2022	<MLS> Added cpu temp sorted list
//*	Aug  8,	2022	<MLS> Added ExportCSV()
//*	Sep 29,	2022	<MLS> Added dynamic display CPU temp when mouse is moved over graph
//*	Sep 30,	2022	<MLS> Added HandleMouseMovedInGraph()
//*	Oct  4,	2022	<MLS> Added scrolling to IP list
//*****************************************************************************

#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<netinet/in.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"discoverythread.h"
#include	"discovery_lib.h"
#include	"linuxerrors.h"
#include	"helper_functions.h"

#include	"windowtab_iplist.h"

#include	"controller.h"
#include	"controller_alpacaUnit.h"

static  int CPUtempSortProc(const void *e1, const void *e2);


//**************************************************************************************
WindowTabIPList::WindowTabIPList(	const int	xSize,
									const int	ySize,
									cv::Scalar	backGrndColor,
									const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	cSortColumn			=	-1;
	cFirstLineIdx		=	0;
	cGraphMode			=	kGraphMode_Raw;
	iii	=	0;
	cCPUcolors[iii++]	=	CV_RGB(255,	0,		0),
	cCPUcolors[iii++]	=	CV_RGB(0,	255,	0),
	cCPUcolors[iii++]	=	CV_RGB(0,	0,		255),
	cCPUcolors[iii++]	=	CV_RGB(255,	255,	0),
	cCPUcolors[iii++]	=	CV_RGB(255,	0,		255),
	cCPUcolors[iii++]	=	CV_RGB(0,	255,	255),
	cCPUcolors[iii++]	=	CV_RGB(255,	128,	128),
	cCPUcolors[iii++]	=	CV_RGB(128,	255,	128),
	cCPUcolors[iii++]	=	CV_RGB(128,	64,		255),
	cCPUcolors[iii++]	=	CV_RGB(128,	128,	128),
	cCPUcolors[iii++]	=	CV_RGB(64,	255,	255),
	cCPUcolors[iii++]	=	CV_RGB(255,	64,		255),

	SetupWindowControls();
	UpdateButtons();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabIPList::~WindowTabIPList(void)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
}

//**************************************************************************************
void	WindowTabIPList::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		yLoc2;
int		textBoxHt;
int		textBoxWd;
int		widgetWidth;
int		iii;
short	widthArray[kMaxTabStops]	=	{150, 100, 100, 63, 63, 63, 63, 63, 325, 200, 0, 0, 0};
short	tabArray[kMaxTabStops];
int		clmnHdr_xLoc;
int		clmnHdrWidth;
int		tabOffset;
int		graphHeight;
int		statusMsgWidth;
int		statusMsgHeight;
int		tempListWidth;
int		myButtonWidth;

//	CONSOLE_DEBUG(__FUNCTION__);

	//------------------------------------------
	yLoc	=	cTabVertOffset;
	yLoc	=	SetTitleBox(kIPaddrList_Title, -1, yLoc, "IP address list of Alpaca devices");

	//------------------------------------------
	//*	set up the tab stops
	for (iii=0; iii<kMaxTabStops; iii++)
	{
		tabArray[iii]	=	0;
	}
	tabOffset	=	widthArray[0];
	iii			=	0;
	while ((widthArray[iii] > 0) && (iii < kMaxTabStops))
	{
		tabArray[iii]	=	tabOffset;
		tabOffset		+=	widthArray[iii+1];
//		CONSOLE_DEBUG_W_NUM("tabArray[iii]\t=", tabArray[iii]);
		iii++;
	}


	//------------------------------------------
	xLoc			=	5;
	myButtonWidth	=	cBtnWidth * 2;
	SetWidget(		kIPaddrList_DiscoveryThrdStatus,		xLoc,		yLoc,	myButtonWidth,	cTitleHeight);
	SetWidgetType(	kIPaddrList_DiscoveryThrdStatus,		kWidgetType_TextBox);
	SetWidgetFont(	kIPaddrList_DiscoveryThrdStatus,		kFont_Medium);
	SetWidgetText(	kIPaddrList_DiscoveryThrdStatus,		"Status");
	xLoc	+=	myButtonWidth;
	xLoc	+=	2;

	myButtonWidth	=	cBtnWidth - 8;
//	CONSOLE_DEBUG_W_NUM("cBtnWidth    \t=",	cBtnWidth);
//	CONSOLE_DEBUG_W_NUM("myButtonWidth\t=",	myButtonWidth);
	iii				=	kIPaddrList_DiscoveryThrdStop;
	while (iii <= kIPaddrList_DiscoveryReScan)
	{
		SetWidget(			iii,		xLoc,	yLoc,	myButtonWidth,	cTitleHeight);
		SetWidgetType(		iii,		kWidgetType_Button);
		SetWidgetFont(		iii,		kFont_Medium);
		SetWidgetBGColor(	iii,		CV_RGB(255,	255,	255));
		xLoc	+=	myButtonWidth;
		xLoc	+=	2;

		iii++;
	}
//	CONSOLE_DEBUG_W_NUM("cWidth    \t=",	cWidth);
//	CONSOLE_DEBUG_W_NUM("xLoc      \t=",	xLoc);
	SetWidgetText(		kIPaddrList_DiscoveryThrdStop,		"Stop");
	SetWidgetText(		kIPaddrList_DiscoveryThrdReStart,	"Restart");
	SetWidgetText(		kIPaddrList_DiscoveryClear,			"Clear");
	SetWidgetText(		kIPaddrList_DiscoveryReScan,		"Re-Scan");

	statusMsgWidth	=	xLoc;
	xLoc			+=	2;


	yLoc	+=	cTitleHeight;
	yLoc	+=	2;

	//------------------------------------------
	xLoc			=	5;
	statusMsgWidth	-=	xLoc;
	statusMsgHeight	=	2 * cTitleHeight;
	SetWidget(				kIPaddrList_StatusMsg,	xLoc,		yLoc,		statusMsgWidth,		statusMsgHeight);
	SetWidgetType(			kIPaddrList_StatusMsg,	kWidgetType_MultiLineText);
	SetWidgetJustification(	kIPaddrList_StatusMsg,	kJustification_Left);
	SetWidgetFont(			kIPaddrList_StatusMsg,	kFont_Medium);
	yLoc			+=	statusMsgHeight;
	yLoc			+=	2;

	//------------------------------------------
	xLoc		=	5;
	graphHeight	=	175;

	SetWidget(			kIPaddrList_TemperatureGraph,	xLoc,	yLoc,	kMaxCPUtempEntries,	graphHeight);
	SetWidgetType(		kIPaddrList_TemperatureGraph,	kWidgetType_CustomGraphic);

	xLoc		+=	kMaxCPUtempEntries;
	xLoc		+=	2;
	xLoc		+=	2;

	//------------------------------------------
	tempListWidth	=	(cClmWidth + 40) / 2;
	SetWidget(				kIPaddrList_SortedCPUList,	xLoc,	yLoc,	tempListWidth,	(graphHeight - 2));
	SetWidgetType(			kIPaddrList_SortedCPUList,	kWidgetType_MultiLineText);
	SetWidgetJustification(	kIPaddrList_SortedCPUList,	kJustification_Left);
	SetWidgetBorder(		kIPaddrList_SortedCPUList,	false);
	SetWidgetFont(			kIPaddrList_SortedCPUList,	kFont_TextList);
	xLoc		+=	tempListWidth;
	xLoc		+=	2;

	//------------------------------------------
	SetWidget(				kIPaddrList_SortedTempList,	xLoc,	yLoc,	tempListWidth,	(graphHeight - 2));
	SetWidgetType(			kIPaddrList_SortedTempList,	kWidgetType_MultiLineText);
	SetWidgetJustification(	kIPaddrList_SortedTempList,	kJustification_Right);
	SetWidgetBorder(		kIPaddrList_SortedTempList,	false);
	SetWidgetFont(			kIPaddrList_SortedTempList,	kFont_TextList);

	SetWidgetOutlineBox(kIPaddrList_SortedOutline, kIPaddrList_SortedCPUList, (kIPaddrList_SortedOutline - 1));
	xLoc		+=	tempListWidth;
	xLoc		+=	4;

	//------------------------------------------
	iii			=	kIPaddrList_TempModeRaw;
	yLoc2		=	yLoc;
	while (iii <= kIPaddrList_TempModeAvg)
	{
		SetWidget(			iii,	xLoc,	yLoc2,	cClmWidth,	cRadioBtnHt);
		SetWidgetType(		iii,	kWidgetType_RadioButton);
		SetWidgetFont(		iii,	kFont_RadioBtn);
		yLoc2	+=	cRadioBtnHt;
		yLoc2	+=	2;

		iii++;
	}
	SetWidgetText(		kIPaddrList_TempModeRaw,	"Raw");
	SetWidgetText(		kIPaddrList_TempModeAvg,	"Average (last 5)");


	//------------------------------------------
	SetWidget(			kIPaddrList_ExportCSV,	xLoc,	yLoc2,	cClmWidth,	cBtnHeight);
	SetWidgetType(		kIPaddrList_ExportCSV,	kWidgetType_Button);
	SetWidgetText(		kIPaddrList_ExportCSV,	"Export CSV");
	SetWidgetBGColor(	kIPaddrList_ExportCSV,	CV_RGB(255,	255,	255));

	yLoc2	+=	cBtnHeight;
	yLoc2	+=	2;


	yLoc	+=	graphHeight;
	yLoc	+=	2;


	clmnHdr_xLoc		=	1;
	iii	=	kIPaddrList_ClmTitle1;
	while(iii < kIPaddrList_ClmOutline)
	{
		clmnHdrWidth	=	tabArray[iii - kIPaddrList_ClmTitle1] - clmnHdr_xLoc;


		SetWidget(				iii,	clmnHdr_xLoc,			yLoc,		clmnHdrWidth,		cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_TextBox);
		SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetBGColor(		iii,	CV_RGB(192,	192,	192));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,		0));

		clmnHdr_xLoc	=	tabArray[iii - kIPaddrList_ClmTitle1];
		clmnHdr_xLoc	+=	2;


		iii++;
	}
	SetWidgetText(		kIPaddrList_ClmTitle1,	"ip-address");
	SetWidgetText(		kIPaddrList_ClmTitle2,	"port");
	SetWidgetText(		kIPaddrList_ClmTitle3,	"/etc/hosts");
	SetWidgetText(		kIPaddrList_ClmTitle4,	"Valid#");
	SetWidgetText(		kIPaddrList_ClmTitle5,	"Err#");
	SetWidgetText(		kIPaddrList_ClmTitle6,	"Uptime");
	SetWidgetText(		kIPaddrList_ClmTitle7,	"Cpu(F)");
	SetWidgetText(		kIPaddrList_ClmTitle8,	"Max");
	SetWidgetText(		kIPaddrList_ClmTitle9,	"Status");
	SetWidgetText(		kIPaddrList_ClmTitle10,	"TimeStamp");

	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;
	yLoc			+=	6;

	SetWidgetOutlineBox(kIPaddrList_ClmOutline, kIPaddrList_DiscoveryThrdStatus, (kIPaddrList_ClmOutline - 1));

	//=======================================================
	xLoc		=	10;
	textBoxHt	=	14;
	textBoxWd	=	cWidth - (xLoc + 3);
	for (iii=kIPaddrList_AlpacaDev_01; iii <= kIPaddrList_AlpacaDev_Last; iii++)
	{
		SetWidget(				iii,	xLoc,			yLoc,		textBoxWd,		textBoxHt);
		SetWidgetJustification(	iii,	kJustification_Left);
		SetWidgetFont(			iii,	kFont_TextList);
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetBorder(		iii,	false);
		SetWidgetTabStops(		iii,	tabArray);

		yLoc			+=	textBoxHt;
		yLoc			+=	4;
	}

	xLoc		=	0;
	widgetWidth	=	cWidth / 2;
	SetWidget(				kIPaddrList_AlpacaDev_Total,	xLoc,	yLoc,	widgetWidth,	cTitleHeight);
	SetWidgetFont(			kIPaddrList_AlpacaDev_Total,	kFont_Medium);
	SetWidgetText(			kIPaddrList_AlpacaDev_Total,	"Total units =?");
	SetWidgetJustification(	kIPaddrList_AlpacaDev_Total,	kJustification_Left);
	SetWidgetTextColor(		kIPaddrList_AlpacaDev_Total,	CV_RGB(255,	255,	255));

	yLoc			+=	cTitleHeight;
	yLoc			+=	2;

	SetAlpacaLogoBottomCorner(kIPaddrList_AlpacaLogo);
}

//*****************************************************************************
void	WindowTabIPList::ProcessButtonClick(const int buttonIdx, const int flags)
{
bool	updateFlag;

//	CONSOLE_DEBUG(__FUNCTION__);
	updateFlag	=	true;
	switch(buttonIdx)
	{
		case kIPaddrList_DiscoveryThrdStop:
			gDiscoveryThreadKeepRunning	=	false;
			SetWidgetText(	kIPaddrList_StatusMsg,	"Stop message sent");
			break;

		case kIPaddrList_DiscoveryThrdReStart:
//			CONSOLE_DEBUG("kIPaddrList_DiscoveryThrdReStart");
			if (gDiscoveryThreadIsRunning == false)
			{
				StartDiscoveryQuerryThread();
				SetWidgetText(	kIPaddrList_StatusMsg,	"Start message sent");
			}
			else
			{
				SetWidgetText(	kIPaddrList_StatusMsg,	"Discovery thread already running");
			}
			WakeUpDiscoveryThread();
			break;

		case kIPaddrList_DiscoveryClear:
			Discovery_ClearIPAddrList();
			ClearIPaddrList();
			WakeUpDiscoveryThread();
			break;

		case kIPaddrList_DiscoveryReScan:
			WakeUpDiscoveryThread();
			break;

		case kIPaddrList_TempModeRaw:
			cGraphMode	=	kGraphMode_Raw;
			break;

		case kIPaddrList_TempModeAvg:
			cGraphMode	=	kGraphMode_Avg5;
			break;

		case kIPaddrList_ExportCSV:
			ExportCSV();
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
void	WindowTabIPList::UpdateButtons(void)
{
	SetWidgetChecked(kIPaddrList_TempModeRaw, (cGraphMode == kGraphMode_Raw));
	SetWidgetChecked(kIPaddrList_TempModeAvg, (cGraphMode == kGraphMode_Avg5));
}


//*****************************************************************************
void	WindowTabIPList::ProcessDoubleClick(const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{
int		deviceIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("widgetIdx\t=", widgetIdx);

	deviceIdx	=	cFirstLineIdx + (widgetIdx - kIPaddrList_AlpacaDev_01);
//	CONSOLE_DEBUG_W_NUM("deviceIdx\t=", deviceIdx);

	if ((deviceIdx >= 0) && (deviceIdx < gAlpacaUnitCnt))
	{
		CreateAlpacaUnitWindow(&gAlpacaUnitList[deviceIdx]);
	}
	else
	{
		CONSOLE_DEBUG("Index out of range");
	}
}

//*****************************************************************************
void	WindowTabIPList::HandleMouseMovedInGraph(	TYPE_WIDGET *theWidget,
													const int	box_XXX,
													const int	box_YYY)
{
int				time_TotalMin;
int				time_Hours;
int				time_Minutes;
int				temp_degF;
char			msgString[400];
char			shortString[32];
int				cpuTemp;
int				iii;
int				cpuIdx;
TYPE_CPU_SORT	myCPUnameList[kMaxCPUs];

	//*	each pixel is worth 2 minutes
	time_TotalMin	=	box_XXX * 2;
	time_Hours		=	time_TotalMin / 60;
	time_Minutes	=	time_TotalMin % 60;
	temp_degF		=	theWidget->height - box_YYY;
	temp_degF		-=	4;

	//*	make sure we are in valid data territory
	if ((temp_degF >= 0) && (time_TotalMin < (24 * 60)))
	{
		sprintf(msgString, "Time: %02d:%02d, ", time_Hours, time_Minutes);

		temp_degF		=	theWidget->height - box_YYY;
		temp_degF		-=	4;

		//*	get a list of the valid temperatures
		cpuIdx	=	0;
		for (iii=0; iii<kMaxAlpacaIPaddrCnt; iii++)
		{
			if (gAlpacaUnitList[iii].cpuTempValid)
			{
				cpuTemp		=	gAlpacaUnitList[iii].cpuTempLog[box_XXX];
				if (cpuTemp > 20)
				{
					myCPUnameList[cpuIdx].cpuTemp		=	gAlpacaUnitList[iii].cpuTempLog[box_XXX];
					if (strlen(gAlpacaUnitList[iii].hostName) > 0)
					{
						strcpy(myCPUnameList[cpuIdx].cpuName, gAlpacaUnitList[iii].hostName);
					}
					else
					{
						inet_ntop(AF_INET, &(gAlpacaUnitList[iii].deviceAddress.sin_addr), myCPUnameList[cpuIdx].cpuName, INET_ADDRSTRLEN);
					}
					cpuIdx++;
				}
			}
		}

		//*	now we have a list, sort it.
		qsort(myCPUnameList, cpuIdx, sizeof(TYPE_CPU_SORT), CPUtempSortProc);

		//*	now build the string to display
		//*	the list is in descending order
		for (iii=0; iii<cpuIdx; iii++)
		{
			sprintf(shortString, " %s=%2.1f",	myCPUnameList[iii].cpuName,
												myCPUnameList[iii].cpuTemp);
			strcat(msgString, shortString);
		}
		SetWidgetText(kIPaddrList_StatusMsg, msgString);
		ForceWindowUpdate();
	}
	else
	{
	//		CONSOLE_DEBUG(__FUNCTION__);
	}
}

//*****************************************************************************
void	WindowTabIPList::ProcessMouseEvent(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{
int		box_XXX;
int		box_YYY;


	switch(event)
	{
		case cv::EVENT_MOUSEMOVE:
			if (widgetIdx == kIPaddrList_TemperatureGraph)
			{
				//*	compute the offsets within the box
				box_XXX	=	xxx - cWidgetList[widgetIdx].left;
				box_YYY	=	yyy - cWidgetList[widgetIdx].top;
				HandleMouseMovedInGraph(&cWidgetList[widgetIdx], box_XXX, box_YYY);
			}
			break;

		case cv::EVENT_LBUTTONDOWN:
			cLeftButtonDown	=	true;
			break;

		case cv::EVENT_LBUTTONUP:
			cLeftButtonDown	=	false;
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
void	WindowTabIPList::ProcessMouseWheelMoved(const int	widgetIdx,
												const int	event,
												const int	xxx,
												const int	yyy,
												const int	wheelMovement,
												const int	flags)
{
//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, wheelMovement);

	//*	make sure we have something to scroll
	if (gAlpacaUnitCnt > 0)
	{
		cFirstLineIdx	+=	wheelMovement;
		if (cFirstLineIdx < 0)
		{
			cFirstLineIdx	=	0;
		}
		if (cFirstLineIdx >= gAlpacaUnitCnt)
		{
			cFirstLineIdx	=	gAlpacaUnitCnt -1;
		}
		UpdateOnScreenWidgetList();
		ForceWindowUpdate();
	}
}


//**************************************************************************************
#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
void	WindowTabIPList::DrawWidgetCustomGraphic(cv::Mat *openCV_Image, const int widgetIdx)
#else
void	WindowTabIPList::DrawWidgetCustomGraphic(IplImage *openCV_Image, const int widgetIdx)
#endif // _USE_OPENCV_CPP_
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cOpenCV_Image	=	openCV_Image;
	switch(widgetIdx)
	{
		case kIPaddrList_TemperatureGraph:
			DrawCpuTempGraph(&cWidgetList[widgetIdx]);
			break;

		default:
			CONSOLE_DEBUG_W_NUM("widgetIdx\t",	widgetIdx);
			break;
	}
}

//**************************************************************************************
void	WindowTabIPList::ClearIPaddrList(void)
{
int		boxId;

	for (boxId=kIPaddrList_AlpacaDev_01; boxId < kIPaddrList_AlpacaDev_Last; boxId++)
	{
		SetWidgetTextColor(	boxId,	CV_RGB(255,	255,	255));
		SetWidgetText(		boxId,	"");
	}
}

//**************************************************************************************
void	WindowTabIPList::UpdateOnScreenWidgetList(void)
{
int		boxId;
int		iii;
char	textString[128];
char	extraString[32];
char	ipAddrStr[32];
int		deviceIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_ABORT(__FUNCTION__);

	if (gDiscoveryThreadIsRunning)
	{
		SetWidgetText(		kIPaddrList_DiscoveryThrdStatus,	"Discovery Thread is running");
		SetWidgetBGColor(	kIPaddrList_DiscoveryThrdStatus,	CV_RGB(0,	255,	0));
		SetWidgetTextColor(	kIPaddrList_DiscoveryThrdStatus,	CV_RGB(0,	0,	0));

		SetWidgetBGColor(	kIPaddrList_DiscoveryThrdReStart,	CV_RGB(128,	128,	128));
	}
	else
	{
		SetWidgetText(		kIPaddrList_DiscoveryThrdStatus,	"Discovery Thread is stopped");
		SetWidgetBGColor(	kIPaddrList_DiscoveryThrdStatus,	CV_RGB(255,	0,	0));
		SetWidgetTextColor(	kIPaddrList_DiscoveryThrdStatus,	CV_RGB(255,	255,	255));

		SetWidgetBGColor(	kIPaddrList_DiscoveryThrdReStart,	CV_RGB(255,	255,	255));
	}

	//*	limit how far we can scroll
	if (cFirstLineIdx >= gAlpacaUnitCnt)
	{
		cFirstLineIdx	=	gAlpacaUnitCnt -1;
	}
	if (cFirstLineIdx < 0)
	{
		cFirstLineIdx	=	0;
	}

	cLinesOnScreen	=	(kIPaddrList_AlpacaDev_Last - kIPaddrList_AlpacaDev_01) + 1;

	iii			=	0;
	while (iii < cLinesOnScreen)
	{
		boxId		=	iii + kIPaddrList_AlpacaDev_01;
		deviceIdx	=	cFirstLineIdx + iii;
		if ((boxId < kIPaddrList_AlpacaDev_Last) && (deviceIdx < gAlpacaUnitCnt))
		{
			inet_ntop(AF_INET, &(gAlpacaUnitList[deviceIdx].deviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);

			sprintf(textString, "%s\t%d\t%s\t%d\t%d",	ipAddrStr,
														gAlpacaUnitList[deviceIdx].port,
														gAlpacaUnitList[deviceIdx].hostName,
														gAlpacaUnitList[deviceIdx].queryOKcnt,
														gAlpacaUnitList[deviceIdx].queryERRcnt);

			if (gAlpacaUnitList[deviceIdx].upTimeValid)
			{
				sprintf(extraString, "\t%d", gAlpacaUnitList[deviceIdx].upTimeDays);
				strcat(textString, extraString);
			}
			else
			{
				strcat(textString, "\t-");
			}
			//*	CPU temp
			if (gAlpacaUnitList[deviceIdx].cpuTempValid)
			{
				sprintf(extraString, "\t%5.1f", gAlpacaUnitList[deviceIdx].cpuTemp_DegF);
				strcat(textString, extraString);
			}
			else
			{
				strcat(textString, "\t-");
			}

			//*	Max CPU temp
			if (gAlpacaUnitList[deviceIdx].cpuTempValid)
			{
				sprintf(extraString, "\t%5.1f", gAlpacaUnitList[deviceIdx].cpuTemp_DegF_max);
				strcat(textString, extraString);
			}
			else
			{
				strcat(textString, "\t-");
			}

			if (gAlpacaUnitList[deviceIdx].currentlyActive)
			{
				strcat(textString, "\t");
				strcat(textString, gAlpacaUnitList[deviceIdx].versionString);

				SetWidgetTextColor(		boxId,	CV_RGB(0,	255,	0));
			}
			else if (gAlpacaUnitList[deviceIdx].queryERRcnt > 0)
			{
				SetWidgetTextColor(		boxId,	CV_RGB(255,	0,	0));
				strcat(textString, "\tOff-line");
			}
			else
			{
				strcat(textString, "\t");
				SetWidgetTextColor(		boxId,	CV_RGB(255,	255,	255));
			}

			//*	add the time stamp string
			if (strlen(gAlpacaUnitList[deviceIdx].timeStampString) > 0)
			{
				strcat(textString, "\t");
				strcat(textString, gAlpacaUnitList[deviceIdx].timeStampString);
			}
//			else
//			{
//				strcat(textString, "\tNo time");
//			}

			SetWidgetText(boxId, textString);
		}
		else if (boxId <= kIPaddrList_AlpacaDev_Last)
		{
			SetWidgetTextColor(		boxId,	CV_RGB(255,	255,	255));
			SetWidgetText(boxId, "---");
		}
		else
		{
			SetWidgetText(boxId, "");
		}
		iii++;
	}
	sprintf(textString, "Total Alpaca IP address found=%d", gAlpacaUnitCnt);
	SetWidgetText(kIPaddrList_AlpacaDev_Total, textString);

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "exit");
}


#define	TRANSLATE_Y(rect, ddd)	((rect->y + rect->height - 4) - ddd)

//**************************************************************************************
static int	TranslateYvalue(cv::Rect	*myCVrect, double yValue)
{
int	myYvalue;

//	if (yValue > 50)
//	{
//		myYvalue	=	TRANSLATE_Y(myCVrect, (2 * yValue));
//		myYvalue	=	myYvalue + 125;
//	}
//	else
	{
		myYvalue	=	TRANSLATE_Y(myCVrect, yValue);
	}
	return(myYvalue);
}


#define	kMaxCPUs	32

//**************************************************************************************
static  int CPUtempSortProc(const void *e1, const void *e2)
{
TYPE_CPU_SORT	*obj1, *obj2;
int				returnValue;

	obj1		=	(TYPE_CPU_SORT *)e1;
	obj2		=	(TYPE_CPU_SORT *)e2;

	returnValue	=	0;
	if (obj1->cpuTemp < obj2->cpuTemp)
	{
		returnValue	=	1;
	}
	else if (obj1->cpuTemp > obj2->cpuTemp)
	{
		returnValue	=	-1;
	}
	return(returnValue);
}


//**************************************************************************************
void	WindowTabIPList::DrawCpuTempGraph(TYPE_WIDGET *theWidget)
{
cv::Rect		myCVrect;
int				iii;
int				jjj;
int				qqq;
int				previousX;
int				pt1_X;
int				pt1_Y;
int				pt2_X;
int				pt2_Y;
int				cpuTemp;
int				previousCPUtemp;
int				colorIdx;
char			cpuNameListStr[2048];
char			cpuTempStr[48];
int				minutesSinceMidnight;
int				cpuTempIndex;
int				validCPUtempCount;

//	CONSOLE_DEBUG(__FUNCTION__);

	for (iii=0; iii<kMaxCPUs; iii++)
	{
		memset((void *)&cCPU_NameList, 0, sizeof(TYPE_CPU_SORT));
	}


	myCVrect.x		=	theWidget->left;
	myCVrect.y		=	theWidget->top;
	myCVrect.width	=	theWidget->width;
	myCVrect.height	=	theWidget->height;

	//=========================================================
	//*	draw tick mark lines
//	yLoc	=	50;
//	while (yLoc < 325)
//	{
//		DrawTickLine(&myCVrect, yLoc);
//		yLoc	+=	50;
//	}
//	//*	draw a special one at 30
//	DrawTickLine(&myCVrect, 30);

	validCPUtempCount	=	0;
	//=========================================================
	for (iii=0; iii<gAlpacaUnitCnt; iii++)
	{
		colorIdx		=	iii % kCpuColorCnt;
		cCurrentColor	=	cCPUcolors[colorIdx];

		if (gAlpacaUnitList[iii].cpuTempValid)
		{
			if (strlen(gAlpacaUnitList[iii].hostName) > 0)
			{
				strcpy(cCPU_NameList[validCPUtempCount].cpuName, gAlpacaUnitList[iii].hostName);
			}
			else
			{
				inet_ntop(AF_INET, &(gAlpacaUnitList[iii].deviceAddress.sin_addr), cCPU_NameList[validCPUtempCount].cpuName, INET_ADDRSTRLEN);
			}
			cCPU_NameList[validCPUtempCount].cpuColor	=	cCurrentColor;
			cCPU_NameList[validCPUtempCount].cpuTemp	=	gAlpacaUnitList[iii].cpuTemp_DegF;
			validCPUtempCount++;

			previousX		=	theWidget->left;
			previousCPUtemp	=	0;
			for (jjj=0; jjj<kMaxCPUtempEntries; jjj++)
			{
				switch(cGraphMode)
				{
					case kGraphMode_Raw:
						cpuTemp		=	gAlpacaUnitList[iii].cpuTempLog[jjj];
						break;

					case kGraphMode_Avg5:
					default:
						if (jjj < 5)
						{
							cpuTemp	=	gAlpacaUnitList[iii].cpuTempLog[jjj];
						}
						else if (gAlpacaUnitList[iii].cpuTempLog[jjj] > 0)
						{
						double	tempTotal;
						int		avgCnt;

							tempTotal	=	0;
							avgCnt		=	0;
							for (qqq=0; qqq < 5; qqq++)
							{
								if (gAlpacaUnitList[iii].cpuTempLog[jjj - qqq] > 0)
								{
									tempTotal	+=	gAlpacaUnitList[iii].cpuTempLog[jjj - qqq];
									avgCnt++;
								}
							}
							if (avgCnt > 0)
							{
								cpuTemp	=	tempTotal / avgCnt;
							}
						}
						else
						{
							cpuTemp	=	0;
						}
						break;
				}
				//*	compute the x,y points for the line
				pt1_X			=	previousX;
				pt1_Y			=	TranslateYvalue((&myCVrect), previousCPUtemp);
				pt2_X			=	previousX + 1;
				pt2_Y			=	TranslateYvalue((&myCVrect), cpuTemp);
				LLG_MoveTo(pt1_X, pt1_Y);
				LLG_LineTo(pt2_X, pt2_Y);

				previousX		=	pt2_X;
				previousCPUtemp	=	cpuTemp;
			}
		}
	}
	//=========================================================
	//*	now draw a vertical line for the CURRENT time
	minutesSinceMidnight	=	GetMinutesSinceMidnight();
	cpuTempIndex			=	minutesSinceMidnight / 2;
	pt1_X					=	theWidget->left + cpuTempIndex;
	pt1_Y					=	theWidget->top - 1;
	pt2_X					=	pt1_X;
	pt2_Y					=	(theWidget->top + theWidget->height) - 1;
	LLG_SetColor(W_RED);
	LLG_MoveTo(pt1_X, pt1_Y);
	LLG_LineTo(pt2_X, pt2_Y);

	//=========================================================
	//*	were there any valid temperatures
	if (validCPUtempCount > 0)
	{
		//*	do the sort here
		qsort(cCPU_NameList, validCPUtempCount, sizeof(TYPE_CPU_SORT), CPUtempSortProc);

		//*	now create the name list
		cpuNameListStr[0]	=	0;
		for (iii=0; iii<validCPUtempCount; iii++)
		{
			sprintf(cpuTempStr, "%s\r", cCPU_NameList[iii].cpuName);
			strcat(cpuNameListStr, cpuTempStr);

			if (iii > 10)
			{
				break;
			}
		}
		SetWidgetText(kIPaddrList_SortedCPUList, cpuNameListStr);

		cpuNameListStr[0]	=	0;
		for (iii=0; iii<validCPUtempCount; iii++)
		{
			sprintf(cpuTempStr, "%3.2f\r", cCPU_NameList[iii].cpuTemp);
			strcat(cpuNameListStr, cpuTempStr);

			if (iii > 10)
			{
				break;
			}
		}
		SetWidgetText(kIPaddrList_SortedTempList, cpuNameListStr);
	}
}

//**************************************************************************************
void	WindowTabIPList::ExportCSV(void)
{
FILE	*filePointer;
char	linuxErrStr[128];
int		iii;
int		jjj;
char	cpuTempFileName[]	=	"cputemp.csv";

	CONSOLE_DEBUG(__FUNCTION__);
	filePointer	=	fopen(cpuTempFileName, "w");
	if (filePointer != NULL)
	{
		//*	write out a header with the cpu names
		for (iii=0; iii<gAlpacaUnitCnt; iii++)
		{
			if (gAlpacaUnitList[iii].cpuTempValid)
			{
				fprintf(filePointer, "%s,",		gAlpacaUnitList[iii].hostName);
			}
		}
		fprintf(filePointer, "\r\n");


		//*	write out the cpu temp info
		for (jjj=0; jjj<kMaxCPUtempEntries; jjj++)
		{
			for (iii=0; iii<gAlpacaUnitCnt; iii++)
			{
				if (gAlpacaUnitList[iii].cpuTempValid)
				{
					fprintf(filePointer, "%f,",		gAlpacaUnitList[iii].cpuTempLog[jjj]);
				}
			}
			fprintf(filePointer, "\r\n");
		}
		fclose(filePointer);
	}
	else
	{
		//*	something went wrong, we failed to create the file
		GetLinuxErrorString(errno, linuxErrStr);
		CONSOLE_DEBUG_W_STR("Failed to create template file:", linuxErrStr);
	}
}


