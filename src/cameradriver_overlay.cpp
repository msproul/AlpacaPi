//*****************************************************************************
//*	Name:			cameradriver_overlay.cpp
//*
//*	Author:			Mark Sproul (C) 2023
//*
//*	Description:	C++ Driver for Alpaca protocol
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
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Sep  6,	2023	<MLS> Created cameradriver_overlay.cpp
//*	Mau 24,	2024	<MLS> Added _IMAGE_OVERLAY_
//*****************************************************************************


#if defined(_ENABLE_CAMERA_) && defined(_IMAGE_OVERLAY_)

#include	<stdbool.h>
#include	<stdint.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"opencv_utils.h"

#include	"cameradriver.h"


//*****************************************************************************
void		CameraDriver::DrawOverlayOntoImage(void)
{
char		overlayString[512];
char		timeString[64];
double		exposureTimeSecs;
cv::Scalar	fillColor;
cv::Scalar	textColor;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	have to shift by 8 because it is a 16 bit image
	fillColor	=	CV_RGB(0,		0,		0);
	textColor	=	CV_RGB(255<<8,	255<<8,	255<<8);

	overlayString[0]	=	0;
	switch(cOverlayMode)
	{
		case 1:
			strcpy(overlayString, "ShutterStart:");
			if (cGPS.Present)
			{
				strcat(overlayString, " GPS=");
				strcat(overlayString, cGPS.ShutterStartTimeStr);
			}
			FormatTimeStringISO8601(&cCameraProp.Lastexposure_StartTime, timeString);
			strcat(overlayString, " SYS=");
			strcat(overlayString, timeString);

			//*	compute the exposure time in seconds
			exposureTimeSecs	=	(cCameraProp.Lastexposure_duration_us * 1.0) / 1000000.0;

			sprintf(timeString, " EXP=%3.6f (seconds)", exposureTimeSecs);
			strcat(overlayString, timeString);
			break;

		default:
			strcpy(overlayString, "Somethings wrong");
			break;
	}
//	CONSOLE_DEBUG_W_STR("overlayString\t=", overlayString);

#if defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
	if (cOpenCV_ImagePtr != NULL)
	{
		LLG_FillRect(cOpenCV_ImagePtr, 0, 0, cCameraProp.CameraXsize, 20, fillColor);

		LLG_DrawCString(	cOpenCV_ImagePtr,
							10,
							15,
							overlayString,
							1,
							textColor);

	}
	else
	{
		CONSOLE_DEBUG("cOpenCV_ImagePtr is NULL");
	}
#else
	#warning "OpenCV drawing primitves only implemented in C++ version of alpacaPi"
#endif // defined(_USE_OPENCV_CPP_) || (CV_MAJOR_VERSION >= 4)
}

#endif //	defined(_ENABLE_CAMERA_) && defined(_IMAGE_OVERLAY_)

