//*****************************************************************************
//*	Name:			cameradriver_livewindow.cpp
//*
//*	Author:			Mark Sproul (C) 2019, 2020, 2021
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
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 14,	2019	<MLS> Created cameradriver_livewindow.c
//*****************************************************************************

#ifdef _ENABLE_CAMERA_

//#define _DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#ifdef _USE_OPENCV_
	#include "opencv/highgui.h"
	#include "opencv2/highgui/highgui_c.h"
	#include "opencv2/imgproc/imgproc_c.h"
#endif


#include	"alpaca_defs.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"cameradriver.h"


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriver::OpenLiveWindow(char *alpacaErrMsg)
{
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "LiveWindow not implemented");
	return(kASCOM_Err_MethodNotImplemented);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriver::CloseLiveWindow(char *alpacaErrMsg)
{
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "LiveWindow not implemented");
	return(kASCOM_Err_MethodNotImplemented);
}


#endif // _ENABLE_CAMERA_
