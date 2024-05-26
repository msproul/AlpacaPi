//*****************************************************************************
//*		windowtab_fov.cpp		(c) 2021 by Mark Sproul
//*
//*	Description:	Window for camera field of view calculations
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
//*	Mar  9,	2021	<MLS> Created windowtab_fov.cpp
//*	Mar  9,	2021	<MLS> Field of View window fully working
//*	Mar 16,	2021	<MLS> Added creation of "camerafov.txt" file
//*	Mar 16,	2021	<MLS> Added ReadCameraFOVfile()
//*	Mar 17,	2021	<MLS> Added RA and DEC offsets to cameraFOV data
//*	Jun  4,	2022	<MLS> Added FOV Edit button
//*	Jun 15,	2022	<MLS> Added FOV Reload button
//*	Nov  4,	2022	<MLS> Added ResetFOVdata()
//*	Nov  4,	2022	<MLS> Added FOV Rescan button
//*	Mar 11,	2023	<MLS> Added aperture to FOV display
//*	Jun 30,	2023	<MLS> Added AlpacaProcessReadAllIdx_Camera()
//*	May 23,	2024	<MLS> Added diagonal dimension in mm (calculated)
//*	May 25,	2024	<MLS> Added ProcessMouseEvent(), ProcessLineSelect(), SetNewSelectedDevice()
//*	May 25,	2024	<MLS> Line select now working in FOV window
//*****************************************************************************

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"discoverythread.h"
#include	"helper_functions.h"

#include	"cameraFOV.h"

#include	"windowtab.h"
#include	"windowtab_fov.h"
#include	"controller.h"
#include	"controller_skytravel.h"
#include	"camera_AlpacaCmds.h"


#define	kFOVboxHeight	100

//**************************************************************************************
WindowTabFOV::WindowTabFOV(	const int	xSize,
							const int	ySize,
							cv::Scalar	backGrndColor,
							const char	*windowName)
	:WindowTab(xSize, ySize, backGrndColor, windowName)
{
int		iii;
//	CONSOLE_DEBUG(__FUNCTION__);

	cAlpacaDevCnt		=	0;
	cPrevAlpacaDevCnt	=	0;
	cSortColumn			=	-1;
	cLastUpdateTime_ms	=	0;
	cFirstRead			=	true;
	cCurrentCamera		=	NULL;

	memset(cCameraData, 0, (kMaxCamaeraFOVcnt * sizeof(TYPE_CameraFOV)));
	//*	default is all camera FOVs enabled
	for (iii=0; iii<kMaxCamaeraFOVcnt; iii++)
	{
		cCameraData[iii].FOVenabled	=	true;
	}

	ClearRemoteDeviceList();

	SetupWindowControls();
}

#define	kCameraFOVfileName	"camerafov.txt"

//**************************************************************************************
// Destructor
//**************************************************************************************
WindowTabFOV::~WindowTabFOV(void)
{
struct stat	fileStatus;
int			returnCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	fstat - check for existence of file
	returnCode	=	stat(kCameraFOVfileName, &fileStatus);
	if (returnCode != 0)
	{
		WriteCameraFOVfile();
	}
}

//**************************************************************************************
void	WindowTabFOV::SetupWindowControls(void)
{
int		xLoc;
int		yLoc;
int		textBoxHt;
int		buttonWidth;
int		boxNum;
int		clmnHdr_xLoc;
int		chkBoxWidth;
int		textBoxOffsetX;
int		iii;
int		clmnHdrWidth;
short	tabValue;
short	tabDeltas[kMaxTabStops]	=	{250, 145, 70, 70, 70, 150, 140, 150, 100, 199, 0, 0};
//short	tabArray[kMaxTabStops]	=	{250, 400, 500, 610, 750, 900, 1000, 1199, 0};
short	tabArray[kMaxTabStops];
int		clmTitleID;
char	textString[80];

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	set up the tab array from the tab offsets
	for (iii=0; iii<kMaxTabStops; iii++)
	{
		tabArray[iii]	=	0;
	}
	tabValue	=	0;
	iii			=	0;
	while ((iii < kMaxTabStops) && (tabDeltas[iii] > 0))
	{
		tabValue		+=	tabDeltas[iii];
		tabArray[iii]	=	tabValue;
		iii++;
	}

	//------------------------------------------
	yLoc	=	cTabVertOffset;
	yLoc	=	SetTitleBox(kFOVbox_Title, -1, yLoc, "Camera Field of View");

	//------------------------------------------
	textBoxHt		=	cRadioBtnHt;
	chkBoxWidth		=	textBoxHt + 3;
	textBoxOffsetX	=	chkBoxWidth + 1;

	//------------------------------------------
	SetWidget(		kFOVbox_FileInfo,	textBoxOffsetX,			yLoc,	(cWidth - textBoxOffsetX),		cBtnHeight);
	SetWidgetType(	kFOVbox_FileInfo,	kWidgetType_TextBox);
	SetWidgetFont(	kFOVbox_FileInfo,	kFont_Medium);
	sprintf(textString, "Edit file '%s' to change these values", kCameraFOVfileName);
	SetWidgetText(		kFOVbox_FileInfo,	textString);
	yLoc			+=	cBtnHeight;
	yLoc			+=	2;

	//------------------------------------------
	clmnHdr_xLoc	=	textBoxOffsetX;
	iii				=	kFOVbox_ClmTitle1;
	while (iii <= kFOVbox_ClmTitle10)
	{
		clmnHdrWidth	=	tabArray[iii - kFOVbox_ClmTitle1] - clmnHdr_xLoc;

//		CONSOLE_DEBUG_W_NUM("clmnHdr_xLoc\t=",	clmnHdr_xLoc);
//		CONSOLE_DEBUG_W_NUM("clmnHdrWidth\t=",	clmnHdrWidth);

		SetWidget(				iii,	clmnHdr_xLoc,			yLoc,		clmnHdrWidth,		cRadioBtnHt);
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetFont(			iii,	kFont_RadioBtn);
		SetWidgetBGColor(		iii,	CV_RGB(128,	128,	128));
	//	SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));

		clmnHdr_xLoc	=	tabArray[iii - kFOVbox_ClmTitle1];
		clmnHdr_xLoc	+=	2;

		iii++;
	}
	clmTitleID	=	kFOVbox_ClmTitle1;
	SetWidgetText(		clmTitleID++,	"Camera");
	SetWidgetText(		clmTitleID++,	"Image Size (pixels)");
	SetWidgetText(		clmTitleID++,	"Pix Size");
	SetWidgetText(		clmTitleID++,	"Diagonal");
	SetWidgetText(		clmTitleID++,	"Aperture");
	SetWidgetText(		clmTitleID++,	"Focal Len");
	SetWidgetText(		clmTitleID++,	"FOV (degrees)");
	SetWidgetText(		clmTitleID++,	"FOV (minutes)");
	SetWidgetText(		clmTitleID++,	"arcsec/pixel");
	SetWidgetText(		clmTitleID++,	"offset");

	yLoc			+=	cRadioBtnHt;
	yLoc			+=	2;


	//------------------------------------------
	boxNum			=	kFOVbox_CamChkBox1;
	while (boxNum < kFOVbox_CamChkBox_Last)
	{
		//*	setup the check box
		SetWidget(				boxNum,	0,				yLoc,		chkBoxWidth,		textBoxHt);
		SetWidgetType(			boxNum,	kWidgetType_CheckBox);
		SetWidgetJustification(	boxNum,	kJustification_Left);
		SetWidgetFont(			boxNum,	kFont_TextList);
		SetWidgetTextColor(		boxNum,	CV_RGB(255,	255,	255));

		//*	setup the text info
		boxNum++;
		SetWidget(				boxNum,	textBoxOffsetX,	yLoc,		cClmWidth * 5,		textBoxHt);
		SetWidgetType(			boxNum,	kWidgetType_TextBox);
		SetWidgetJustification(	boxNum,	kJustification_Left);
		SetWidgetFont(			boxNum,	kFont_TextList);
		SetWidgetTextColor(		boxNum,	CV_RGB(255,	255,	255));
		SetWidgetNumber(boxNum, boxNum);
		SetWidgetTabStops(		boxNum,	tabArray);
		SetWidgetBorder(		boxNum,	false);

//		SetWidgetText(			boxNum, "alpha\tbeta\tgama");
		SetWidgetText(			boxNum, "---");

		yLoc			+=	textBoxHt;
		yLoc			+=	2;

		boxNum++;
	}

	buttonWidth	=	150;
	iii			=	kFOVbox_SaveButton;
	xLoc		=	1;
	yLoc		=	cHeight - cBtnHeight;
	yLoc		-=	2;
	while (iii <= kFOVbox_RescanButton)
	{
		SetWidget(				iii,	xLoc,	yLoc,	buttonWidth,	cBtnHeight);
		SetWidgetType(			iii,	kWidgetType_Button);
		SetWidgetTextColor(		iii,	CV_RGB(0,	0,	0));
		SetWidgetBGColor(		iii,	CV_RGB(255,	255,	255));

		xLoc	+=	buttonWidth;
		xLoc	+=	2;

		iii++;
	}
	SetWidgetText(			kFOVbox_SaveButton,		"Save");
	SetWidgetText(			kFOVbox_EditButton,		"Edit");
	SetWidgetText(			kFOVbox_ReloadButton,	"Reload");
	SetWidgetText(			kFOVbox_RescanButton,	"Rescan");

	SetAlpacaLogoBottomCorner(kFOVbox_AlpacaLogo);
}

//*****************************************************************************
//*	returns arc-seconds / micron
static double	Calc_ImageScale(double focalLength_mm)
{
double	imageScale;

	imageScale	=	206.2649 / focalLength_mm;
//	CONSOLE_DEBUG_W_DBL("imageScale\t=", imageScale);
	return(imageScale);
}

//*****************************************************************************
//*	returns arc-seconds per pixel
//*****************************************************************************
static double	Calc_AngularResolutionPerPixel(double focalLength_mm, double pixelSize_microns)
{
double	imageScale;
double	angularResolution;

	imageScale			=	Calc_ImageScale(focalLength_mm);
	angularResolution	=	pixelSize_microns * imageScale;
	return(angularResolution);
}


//*****************************************************************************
static double	Calc_FieldOfView_arcSecs(double focalLength_mm, double pixelSize_microns, int pixels_x)
{
double	angularResolution;
double	fov_arcSeconds;

	angularResolution	=	Calc_AngularResolutionPerPixel(focalLength_mm, pixelSize_microns);
	fov_arcSeconds		=	angularResolution * pixels_x;
	return(fov_arcSeconds);
}

//**************************************************************************************
//*	this is a little bit lame, we read the file for EACH camera device
//*	we only do it ONCE at startup,
//*	and it is a lot easier than trying to allocate and free a data structure
//**************************************************************************************
static void	ReadCameraFOVfile(TYPE_REMOTE_DEV *remoteDevice, TYPE_CameraFOV *cameraDataPtr)
{
FILE	*filePointer;
char	lineBuff[128];
char	hostName[64];
char	alpacaNumStr[64];
char	apertureStr[64];
char	focalLenStr[64];
char	raOffsetStr[64];
char	decOffsetStr[64];
int		sLen;
int		ccc;
int		iii;
int		alpacaDevNum;
bool	keepGoing;
bool	disableFOVdisplay;

//	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(hostName,		"");
	strcpy(alpacaNumStr,	"0");
	strcpy(apertureStr,		"0");
	strcpy(focalLenStr,		"0");
	strcpy(raOffsetStr,		"0");
	strcpy(decOffsetStr,	"0");


	filePointer	=	fopen(kCameraFOVfileName, "r");
	if (filePointer != NULL)
	{
		//*	read the lines of the camera fov definition file
		//*	we are looking for a name and alpaca number match.
		//*	the alpaca device number is normally zero, but can be anything.
		keepGoing	=	true;
		while (fgets(lineBuff, 100, filePointer) && keepGoing)
		{
			sLen	=	strlen(lineBuff);
			if ((lineBuff[0] != '#') && (sLen > 5))
			{
				iii	=	0;
				ccc	=	0;

				disableFOVdisplay	=	false;
				if (lineBuff[iii] == '-')
				{
					disableFOVdisplay	=	true;
					iii++;
				}

				//*	extract arg 1, host name
				while ((lineBuff[iii] >= 0x20) && (iii <= sLen))
				{
					hostName[ccc++]	=	lineBuff[iii++];
					hostName[ccc]	=	0;
				}
				//*	see if the host name matches
				if (strcasecmp(hostName, remoteDevice->hostName) == 0)
				{
					iii++;
					ccc	=	0;
					//*	arg 2 = alpaca device number (normally 0)
					while ((lineBuff[iii] >= 0x20) && (iii <= sLen))
					{
						alpacaNumStr[ccc++]	=	lineBuff[iii++];
						alpacaNumStr[ccc]	=	0;
					}
					alpacaDevNum	=	atoi(alpacaNumStr);
					if (alpacaDevNum == remoteDevice->alpacaDeviceNum)
					{
						//*	we have the right one
						keepGoing	=	false;
						iii++;
						ccc	=	0;
						//*	arg 3 = aperture (mm)
						while ((lineBuff[iii] >= 0x20) && (iii <= sLen))
						{
							apertureStr[ccc++]	=	lineBuff[iii++];
							apertureStr[ccc]	=	0;
						}
						iii++;
						ccc	=	0;
						//*	arg 4 = focal length (mm)
						while ((lineBuff[iii] >= 0x20) && (iii <= sLen))
						{
							focalLenStr[ccc++]	=	lineBuff[iii++];
							focalLenStr[ccc]	=	0;
						}

						iii++;
						ccc	=	0;
						//	5th column is Right Ascension offset (decimal hours i.e. 0.123)
						while ((lineBuff[iii] >= 0x20) && (iii <= sLen))
						{
							raOffsetStr[ccc++]	=	lineBuff[iii++];
							raOffsetStr[ccc]	=	0;
						}

						iii++;
						ccc	=	0;
						//	6th column is Declination offset (decimal degrees i.e. 0.123)
						while ((lineBuff[iii] >= 0x20) && (iii <= sLen))
						{
							decOffsetStr[ccc++]	=	lineBuff[iii++];
							decOffsetStr[ccc]	=	0;
						}


					//	CONSOLE_DEBUG_W_STR("hostName    \t=",	hostName);
					//	CONSOLE_DEBUG_W_STR("alpacaNumStr\t=",	alpacaNumStr);
					//	CONSOLE_DEBUG_W_STR("apertureStr \t=",	apertureStr);
					//	CONSOLE_DEBUG_W_STR("focalLenStr \t=",	focalLenStr);

						cameraDataPtr->FocalLen_mm			=   AsciiToDouble(focalLenStr);
						cameraDataPtr->Aperture_mm			=	AsciiToDouble(apertureStr);

						cameraDataPtr->RighttAscen_Offset	=	AsciiToDouble(raOffsetStr);
						cameraDataPtr->Declination_Offset	=	AsciiToDouble(decOffsetStr);

						if (disableFOVdisplay)
						{
							cameraDataPtr->FOVenabled	=	false;
						}
					}
				}
			}
		}
		fclose(filePointer);
	}

	//*	set outline colors
	if (strcmp(remoteDevice->hostName, "finder") == 0)
	{
		cameraDataPtr->OutLineColor	=	W_GREEN;
	}
	else if ((cameraDataPtr->RighttAscen_Offset != 0.0) || (cameraDataPtr->Declination_Offset != 0.0))
	{
		cameraDataPtr->OutLineColor	=	W_YELLOW;
	}
	else
	{
		cameraDataPtr->OutLineColor	=	W_RED;
	}
}

//**************************************************************************************
void	UpdateCameraData(TYPE_REMOTE_DEV *remoteDevice, TYPE_CameraFOV *cameraDataPtr)
{
	cameraDataPtr->ImgSizeX_microns	=	cameraDataPtr->CameraProp.CameraXsize
										* cameraDataPtr->CameraProp.PixelSizeX;

	cameraDataPtr->ImgSizeY_microns	=	cameraDataPtr->CameraProp.CameraYsize
										* cameraDataPtr->CameraProp.PixelSizeY;

	cameraDataPtr->ImageDiagonal_mm	=	sqrt((cameraDataPtr->ImgSizeX_microns * cameraDataPtr->ImgSizeX_microns) +
											(cameraDataPtr->ImgSizeY_microns * cameraDataPtr->ImgSizeY_microns))
											/ 1000.0;
//-	ReadCameraFOVfile(remoteDevice, cameraDataPtr);

	//*	do we have a focal length?, calcualte FOV
	if (cameraDataPtr->FocalLen_mm > 0)
	{
		if (cameraDataPtr->Aperture_mm > 0)
		{
			cameraDataPtr->F_Ratio		=	cameraDataPtr->FocalLen_mm / cameraDataPtr->Aperture_mm;
		}
		cameraDataPtr->PixelScale		=	Calc_AngularResolutionPerPixel(	cameraDataPtr->FocalLen_mm,
																			cameraDataPtr->CameraProp.PixelSizeX);
		cameraDataPtr->FOV_X_arcSeconds	=	Calc_FieldOfView_arcSecs(		cameraDataPtr->FocalLen_mm,
																			cameraDataPtr->CameraProp.PixelSizeX,
																			cameraDataPtr->CameraProp.CameraXsize);

		cameraDataPtr->FOV_Y_arcSeconds	=	Calc_FieldOfView_arcSecs(		cameraDataPtr->FocalLen_mm,
																			cameraDataPtr->CameraProp.PixelSizeY,
																			cameraDataPtr->CameraProp.CameraYsize);
	}
}

//**************************************************************************************
void	WindowTabFOV::RunWindowBackgroundTasks(void)
{
uint32_t			currentMilliSecs;
uint32_t			deltaMilliSecs;
int					iii;
bool				validData;
ControllerSkytravel	*skyTravelController;
bool				dataWasUpdated;
char				ipAddrStr[32];
int					updateCount;

//	CONSOLE_DEBUG(__FUNCTION__);

	dataWasUpdated		=	false;
	currentMilliSecs	=	millis();
	updateCount			=	0;
	deltaMilliSecs		=	currentMilliSecs - cLastUpdateTime_ms;
//	if (deltaMilliSecs > 3000)
	if (deltaMilliSecs > 1000)
	{
		//*	check to see if we need to get info on any camera.
		//*	only do one each time through
		for (iii=0; iii<kMaxCamaeraFOVcnt; iii++)
		{
			if ((cCameraData[iii].IsValid) && (cCameraData[iii].PropertyDataValid == false))
			{
				skyTravelController	=	(ControllerSkytravel *)cParentObjPtr;
				if (skyTravelController != NULL)
				{
//					CONSOLE_DEBUG("=================================================");

					cCurrentCamera	=	&cCameraData[iii];

					ReadCameraFOVfile(&cRemoteDeviceList[iii], &cCameraData[iii]);

					inet_ntop(AF_INET, &(cRemoteDeviceList[iii].deviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
//					CONSOLE_DEBUG_W_STR(ipAddrStr, cCameraData[iii].CameraName);
					cCameraData[iii].HasReadAll	=	false;



					validData	=	skyTravelController->AlpacaGetSupportedActions(
																	&cRemoteDeviceList[iii].deviceAddress,
																	cRemoteDeviceList[iii].port,
																	"camera",
																	cRemoteDeviceList[iii].alpacaDeviceNum);

					if (cCameraData[iii].HasReadAll)
					{
//						CONSOLE_DEBUG("Camera has READALL!!!!!!!!!!!!!!!!!!!!!!!!!!!");
						inet_ntop(AF_INET, &(cRemoteDeviceList[iii].deviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
//						CONSOLE_DEBUG_W_STR(ipAddrStr, cCameraData[iii].CameraName);
						skyTravelController->SetCommandLookupTable(gCameraCmdTable);
						skyTravelController->SetAlternateLookupTable(gCameraExtrasTable);
						validData		=	skyTravelController->AlpacaGetStatus_ReadAll(
																	&cRemoteDeviceList[iii].deviceAddress,
																	cRemoteDeviceList[iii].port,
																	"camera",
																	cRemoteDeviceList[iii].alpacaDeviceNum);
						skyTravelController->SetCommandLookupTable(NULL);
						skyTravelController->SetAlternateLookupTable(NULL);
					}
					else
					{
					//	CONSOLE_DEBUG("Camera has does NOT have readall---------------------");
						//*	if readall failed, do it the one at a time way
						validData	=	skyTravelController->AlpacaGetStartupData_Camera(
																&cRemoteDeviceList[iii],
																&cCameraData[iii].CameraProp);

					}

					if (validData)
					{
						UpdateCameraData(&cRemoteDeviceList[iii], &cCameraData[iii]);
					}
					dataWasUpdated		=	true;
					updateCount++;
				}
				cCameraData[iii].PropertyDataValid	=	true;
				break;
			}
			cCurrentCamera		=	NULL;
		}
//		if (updateCount > 0)
//		{
//			CONSOLE_DEBUG_W_NUM("updateCount\t=", updateCount);
//		}

		cLastUpdateTime_ms	=	currentMilliSecs;
		if (dataWasUpdated)
		{
			//*	now update the widget text
			UpdateOnScreenWidgetList();
			ForceWindowUpdate();
		}
	}
}


//*****************************************************************************
void	WindowTabFOV::ProcessButtonClick(const int buttonIdx, const int flags)
{
bool	forceUpdateFlg;
int		cameraIdx;

//	CONSOLE_DEBUG(__FUNCTION__);

	forceUpdateFlg	=	true;
	switch(buttonIdx)
	{
		case kFOVbox_ClmTitle1:
		case kFOVbox_ClmTitle2:
		case kFOVbox_ClmTitle3:
		case kFOVbox_ClmTitle4:
			forceUpdateFlg	=	false;
			break;

		case kFOVbox_CamChkBox1:
		case kFOVbox_CamChkBox2:
		case kFOVbox_CamChkBox3:
		case kFOVbox_CamChkBox4:
		case kFOVbox_CamChkBox5:
		case kFOVbox_CamChkBox6:
		case kFOVbox_CamChkBox7:
		case kFOVbox_CamChkBox8:
		case kFOVbox_CamChkBox9:
		case kFOVbox_CamChkBox10:
		case kFOVbox_CamChkBox11:
		case kFOVbox_CamChkBox12:
		case kFOVbox_CamChkBox13:
		case kFOVbox_CamChkBox14:
		case kFOVbox_CamChkBox15:
		case kFOVbox_CamChkBox16:
			cameraIdx	=	(buttonIdx - kFOVbox_CamChkBox1) / 2;
			if (cameraIdx < kMaxCamaeraFOVcnt)
			{
				cCameraData[cameraIdx].FOVenabled	=	!cCameraData[cameraIdx].FOVenabled;
				SetWidgetChecked(buttonIdx, cCameraData[cameraIdx].FOVenabled);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("Camera index out of bounds", cameraIdx);
			}
			break;

		case kFOVbox_SaveButton:
			WriteCameraFOVfile();
			break;

		case kFOVbox_EditButton:
			EditTextFile(kCameraFOVfileName);
			break;

		case kFOVbox_ReloadButton:
			CONSOLE_DEBUG("kFOVbox_ReloadButton");
			{
			int		iii;
				for (iii=0; iii<kMaxCamaeraFOVcnt; iii++)
				{
					cCameraData[iii].PropertyDataValid	=	false;
				}
			}
			break;

		case kFOVbox_RescanButton:
			ResetFOVdata();
			break;

		default:
			forceUpdateFlg	=	false;
			break;
	}
	if (forceUpdateFlg)
	{
		ForceWindowUpdate();
	}
}

//*****************************************************************************
void	WindowTabFOV::ProcessDoubleClick(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{

}

//*****************************************************************************
void	WindowTabFOV::ProcessMouseEvent(	const int	widgetIdx,
											const int	event,
											const int	xxx,
											const int	yyy,
											const int	flags)
{
//int		box_XXX;
//int		box_YYY;

	switch(event)
	{
		case cv::EVENT_MOUSEMOVE:
			break;

		case cv::EVENT_LBUTTONDOWN:
//			CONSOLE_DEBUG_W_NUM("EVENT_LBUTTONDOWN", widgetIdx);
			cLeftButtonDown	=	true;
			break;

		case cv::EVENT_LBUTTONUP:
			CONSOLE_DEBUG_W_NUM("EVENT_LBUTTONUP", widgetIdx);
			cLeftButtonDown	=	false;
			ProcessLineSelect(widgetIdx);
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

//**************************************************************************************
void	WindowTabFOV::ProcessLineSelect(int widgetIdx)
{
int		deviceIdx;
int		adjustedIdx;
//int		iii;

	CONSOLE_DEBUG(__FUNCTION__);
	if ((widgetIdx >= kFOVbox_CamChkBox1) && (widgetIdx <= kFOVbox_CamChkBox_Last))
	{
		adjustedIdx	=	widgetIdx - kFOVbox_CamChkBox1;
		adjustedIdx	=	adjustedIdx / 2;
		deviceIdx	=	adjustedIdx + cFirstLineIdx;

		if ((deviceIdx >= 0) && (deviceIdx < kMaxAlpacaIPaddrCnt))
		{
			SetNewSelectedDevice(deviceIdx);
//
//			//*	clear out previous selections
//			for (iii=0; iii<kMaxAlpacaIPaddrCnt; iii++)
//			{
//				gAlpacaUnitList[iii].lineSelected	=	false;
//			}
//			gAlpacaUnitList[deviceIdx].lineSelected	=	true;
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("deviceIdx is out of bounds\t=", deviceIdx);
		}
//		UpdateOnScreenWidgetList();
//		ForceWindowUpdate();
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("widgetIdx is out of range\t=", widgetIdx);
	}
}

//*****************************************************************************
void	WindowTabFOV::SetNewSelectedDevice(int deviceIndex)
{
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	if (deviceIndex >= 0)
	{
		//*	clear out previous selections
		for (iii=0; iii<kMaxAlpacaIPaddrCnt; iii++)
		{
			cRemoteDeviceList[iii].lineSelected	=	false;
		}
		cRemoteDeviceList[deviceIndex].lineSelected	=	true;
		UpdateOnScreenWidgetList();
		ForceWindowUpdate();
	}
}

//**************************************************************************************
void	WindowTabFOV::ClearRemoteDeviceList(void)
{
int		iii;

	for (iii=kFOVbox_CamChkBox1; iii<=kFOVbox_CamChkBox_Last; iii++)
	{
		SetWidgetTextColor(		iii,	CV_RGB(255,	255,	255));
		SetWidgetText(			iii,	"");

	}

	for (iii=0; iii<kMaxCamaeraFOVcnt; iii++)
	{
		memset(&cRemoteDeviceList[iii], 0, sizeof(TYPE_REMOTE_DEV));
	}
	cAlpacaDevCnt		=	0;
	cPrevAlpacaDevCnt	=	-1;
	cSortColumn			=	-1;

//	SetWidgetBGColor(kAlpacaList_ClmTitle1,	((cSortColumn == 0) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
//	SetWidgetBGColor(kAlpacaList_ClmTitle2,	((cSortColumn == 1) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
//	SetWidgetBGColor(kAlpacaList_ClmTitle3,	((cSortColumn == 2) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
//	SetWidgetBGColor(kAlpacaList_ClmTitle4,	((cSortColumn == 3) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
//	SetWidgetBGColor(kAlpacaList_ClmTitle5,	((cSortColumn == 4) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));

}

//**************************************************************************************
void	WindowTabFOV::UpdateRemoteDeviceList(void)
{
int		iii;
int		foundIndex;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("gRemoteCnt\t=", gRemoteCnt);
//	CONSOLE_DEBUG_W_NUM("kMaxCamaeraFOVcnt\t=", kMaxCamaeraFOVcnt);

	for (iii=0; iii<gRemoteCnt; iii++)
	{
		//*	first, see if this device is already in our list.
		foundIndex		=	FindDeviceInList(&gRemoteList[iii], cRemoteDeviceList, kMaxCamaeraFOVcnt);
		if (foundIndex >= 0)
		{
			//*	its already in the list
		}
		else if (strcasecmp(gRemoteList[iii].deviceTypeStr, "camera") == 0)
		{
			if (cAlpacaDevCnt < kMaxCamaeraFOVcnt)
			{
				cRemoteDeviceList[cAlpacaDevCnt]			=	gRemoteList[iii];
				cRemoteDeviceList[cAlpacaDevCnt].validEntry	=	true;
				cCameraData[cAlpacaDevCnt].IsValid			=	true;

				strcpy(cCameraData[cAlpacaDevCnt].HostName,		gRemoteList[iii].hostName);
				strcpy(cCameraData[cAlpacaDevCnt].CameraName,	gRemoteList[iii].deviceNameStr);

				cAlpacaDevCnt++;
			}
		}
	}

	//*	now update the widget text
	UpdateOnScreenWidgetList();

	if (cAlpacaDevCnt != cPrevAlpacaDevCnt)
	{
		ForceWindowUpdate();
		cPrevAlpacaDevCnt	=	cAlpacaDevCnt;
	}
}

//**************************************************************************************
void	WindowTabFOV::UpdateOnScreenWidgetList(void)
{
int		checkBoxId;
int		textBoxId;
int		iii;
char	nameString[128];
char	textString[512];
char	offsetsString[64];
char	tempString[64];
//char	ipAddrStr[32];
int		myDevCount;

//	CONSOLE_DEBUG(__FUNCTION__);

	iii			=	0;
	myDevCount	=	0;
	while ((iii < kMaxCamaeraFOVcnt))
	{
		checkBoxId	=	(2 * iii) + kFOVbox_CamChkBox1;
		textBoxId	=	(2 * iii) + kFOVbox_CamInfo1;

		if ((textBoxId <= kFOVbox_CamChkBox_Last) && (cRemoteDeviceList[iii].validEntry))
		{
			SetWidgetChecked(checkBoxId, cCameraData[iii].FOVenabled);
			//*	fill in the check box
			if (cRemoteDeviceList[iii].alpacaDeviceNum > 0)
			{
				sprintf(nameString, "%s/%d-%s",	cRemoteDeviceList[iii].hostName,
												cRemoteDeviceList[iii].alpacaDeviceNum,
												cRemoteDeviceList[iii].deviceNameStr);
			}
			else
			{
				sprintf(nameString, "%s-%s",	cRemoteDeviceList[iii].hostName,
												cRemoteDeviceList[iii].deviceNameStr);
			}
			if (strlen(nameString) > 25)
			{
				nameString[25]	=	0;
			}

//			inet_ntop(AF_INET, &(cRemoteDeviceList[iii].deviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);

			strcpy(textString,	nameString);
			strcat(textString,	"\t");
			//------------------
			sprintf(tempString,	"%4d x %4d\t%5.3f\t", 	cCameraData[iii].CameraProp.CameraXsize,
														cCameraData[iii].CameraProp.CameraYsize,
														cCameraData[iii].CameraProp.PixelSizeX);
			strcat(textString,	tempString);

			//------------------

			sprintf(tempString,	"%5.2f\t",					cCameraData[iii].ImageDiagonal_mm);
			strcat(textString,	tempString);
			//------------------
			sprintf(tempString,	"%4.1f\t%3.0f / F%3.2f\t",	cCameraData[iii].Aperture_mm,
															cCameraData[iii].FocalLen_mm,
															cCameraData[iii].F_Ratio);
			strcat(textString,	tempString);
			//------------------
			sprintf(tempString,	"%3.2f x %3.2f\t",			(cCameraData[iii].FOV_X_arcSeconds / 3600.0),
															(cCameraData[iii].FOV_Y_arcSeconds / 3600.0));
			strcat(textString,	tempString);
			//------------------
			sprintf(tempString,	"%3.1f x %3.1f\t",			(cCameraData[iii].FOV_X_arcSeconds / 60.0),
															(cCameraData[iii].FOV_Y_arcSeconds / 60.0));
			strcat(textString,	tempString);
			//------------------
			sprintf(tempString,	"%6.4f",					cCameraData[iii].PixelScale);
			strcat(textString,	tempString);


			if ((cCameraData[iii].RighttAscen_Offset != 0) || (cCameraData[iii].Declination_Offset != 0))
			{
				sprintf(offsetsString,	"\t%6.3f x %6.3f", cCameraData[iii].RighttAscen_Offset, cCameraData[iii].Declination_Offset);
				strcat(textString, offsetsString);

			}
			SetWidgetText(textBoxId, textString);

			//--------------------------------------------------------
			//*	check for line selection
			if (cRemoteDeviceList[iii].lineSelected == true)
			{
				SetWidgetLineSelect(textBoxId, true);
			}
			else
			{
				SetWidgetLineSelect(textBoxId, false);
			}

			myDevCount++;
		}
		else if (textBoxId <= kFOVbox_CamChkBox_Last)
		{
			SetWidgetText(textBoxId, "---");
		}
		else
		{
		//	CONSOLE_DEBUG_W_NUM("iii\t=", iii);
			break;
		}
		iii++;
	}


//	sprintf(textString, "Total Alpaca Devices found=%d", myDevCount);
//	SetWidgetText(kAlpacaList_AlpacaDev_Total, textString);

//	SetWidgetBGColor(kAlpacaList_ClmTitle1,	((cSortColumn == 0) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
//	SetWidgetBGColor(kAlpacaList_ClmTitle2,	((cSortColumn == 1) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
//	SetWidgetBGColor(kAlpacaList_ClmTitle3,	((cSortColumn == 2) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
//	SetWidgetBGColor(kAlpacaList_ClmTitle4,	((cSortColumn == 3) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
//	SetWidgetBGColor(kAlpacaList_ClmTitle5,	((cSortColumn == 4) ? CV_RGB(255,	255,	255) : CV_RGB(128,	128,	128)));
}

//*****************************************************************************
bool	WindowTabFOV::AlpacaProcessReadAllIdx_Camera(	const int	deviceNum,
														const int	keywordEnum,
														const char *valueString)
{
bool	dataWasHandled;

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, keywordEnum);
	dataWasHandled	=	true;
	switch(keywordEnum)
	{
		case kCmd_Camera_camerastate:			//*	Returns the camera operational state.
			break;

		case kCmd_Camera_cameraxsize:			//*	Returns the width of the CCD camera chip.
			cCurrentCamera->HasReadAll	=	true;
			cCurrentCamera->CameraProp.CameraXsize	=	atoi(valueString);
			break;

		case kCmd_Camera_cameraysize:			//*	Returns the height of the CCD camera chip.
			cCurrentCamera->CameraProp.CameraYsize	=	atoi(valueString);
			break;

		case kCmd_Camera_PixelSizeX:			//*	Width of CCD chip pixels (microns)
			cCurrentCamera->CameraProp.PixelSizeX	=	AsciiToDouble(valueString);
			break;

		case kCmd_Camera_PixelSizeY:			//*	Height of CCD chip pixels (microns)
			cCurrentCamera->CameraProp.PixelSizeY	=	AsciiToDouble(valueString);
			break;

		//*	commands borrowed from the telescope device
		case kCmd_Camera_ApertureArea:			//*	Returns the telescope's aperture.
			break;

		case kCmd_Camera_ApertureDiameter:		//*	Returns the telescope's effective aperture.
			//*	the value is in meters
			cCurrentCamera->Aperture_mm	=	AsciiToDouble(valueString) * 1000.0;
			break;

		case kCmd_Camera_FocalLength:			//*	Returns the telescope's focal length in meters.
			//*	the value is in meters
			cCurrentCamera->FocalLen_mm	=	AsciiToDouble(valueString) * 1000.0;
			break;

		default:
			dataWasHandled	=	true;
			break;
	}
	return(dataWasHandled);
}



//*****************************************************************************
TYPE_CameraFOV	*WindowTabFOV::GetCameraFOVptr(void)
{
	return(cCameraData);
}

//*****************************************************************************
void	WindowTabFOV::WriteCameraFOVfile(void)
{
FILE	*filePointer;
char	separaterLine[]	=	"###############################################################################\n";
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	filePointer	=	fopen(kCameraFOVfileName, "w");
	if (filePointer != NULL)
	{
		fprintf(filePointer, "%s",	separaterLine);
		fprintf(filePointer, "#	Modify this file with your information\n");
		fprintf(filePointer, "#	columns are separated by the tab char\n");
		fprintf(filePointer, "#	\n");
		fprintf(filePointer, "#	1st column is the device name from /etc/hosts\n");
		fprintf(filePointer, "#	2nd column is Alpaca device number (normally 0)\n");
		fprintf(filePointer, "#	3rd column is aperture in mm\n");
		fprintf(filePointer, "#	4th column is focal length in mm\n");
		fprintf(filePointer, "#	5th column is Right Ascension offset (decimal hours i.e. 0.123) \n");
		fprintf(filePointer, "#	6th column is Declination offset (decimal degrees i.e. 0.123) \n");
		fprintf(filePointer, "#	\n");
		fprintf(filePointer, "#	The offset is for multiple telescopes/cameras on the same mount that may not be perfectly aligned\n");
		fprintf(filePointer, "#	The main  or primary telescope should have offsets of zero\n");
		fprintf(filePointer, "#	This is also used for camera arrays such as a project I am working on\n");
		fprintf(filePointer, "%s",	separaterLine);
		fprintf(filePointer, "#	Example (remove '#')\n");
		fprintf(filePointer, "#newt16\t0\t406.40\t1689.00\t0.000000\t0.000000\n");
		fprintf(filePointer, "%s",	separaterLine);
		for (iii=0; iii<kMaxCamaeraFOVcnt; iii++)
		{
			if (cCameraData[iii].IsValid&& cCameraData[iii].PropertyDataValid)
			{
				fprintf(filePointer, "%s\t",	cRemoteDeviceList[iii].hostName);
				fprintf(filePointer, "%d\t",	cRemoteDeviceList[iii].alpacaDeviceNum);
				fprintf(filePointer, "%3.2f\t",	cCameraData[iii].Aperture_mm);
				fprintf(filePointer, "%3.2f\t",	cCameraData[iii].FocalLen_mm);

				fprintf(filePointer, "%f\t",	cCameraData[iii].RighttAscen_Offset);
				fprintf(filePointer, "%f",		cCameraData[iii].Declination_Offset);



				fprintf(filePointer, "\n");
			}
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create template file");
	}
}


//*****************************************************************************
void	WindowTabFOV::ResetFOVdata(void)
{
int	iii;

	CONSOLE_DEBUG(__FUNCTION__);

	for (iii=0; iii<kMaxCamaeraFOVcnt; iii++)
	{
		cCameraData[iii].PropertyDataValid	=	false;
	}
}
