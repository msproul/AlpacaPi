//**************************************************************************
//*	Name:			cameradriver_readthread.cpp
//*
//*	Author:			Mark Sproul (C) 2019
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
//*	Sep  9,	2023	<MLS> Created cameradriver_readthread.cpp
//*	Sep 23,	2023	<MLS> Moved camera temp logging to CameraDriver::RunThread_Loop()
//*****************************************************************************

#ifdef _ENABLE_CAMERA_



//----------------------------------------------------------------------------
#ifdef _USE_CAMERA_READ_THREAD_

//#define _DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpaca_defs.h"
#include	"linuxerrors.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"cameradriver.h"
#include	"helper_functions.h"


//*****************************************************************************
void	CameraDriver::RunThread_Startup(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
void	CameraDriver::RunThread_Loop(void)
{
time_t		deltaSeconds;
time_t		currentSeconds;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Camera");
	switch(cInternalCameraState)
	{
		case kCameraState_Idle:
			sleep(1);
			break;

		case kCameraState_TakingPicture:
			break;


		case kCameraState_StartVideo:
//				CameraThread_StartVideo();
//				cInternalCameraState	+=	1;	//*	bump to the next state
			break;

		case kCameraState_TakingVideo:
//				CameraThread_GetVideo();
			break;

		default:
			//*	we should never get here
			break;
	}
	if (cTempReadSupported)
	{
		//*	if we support camera temperature, log it every 30 seconds
		currentSeconds	=   GetSecondsSinceEpoch();
		deltaSeconds	=	currentSeconds - cLastTempUpdate_Secs;
		if (deltaSeconds >= 30)
		{
		TYPE_ASCOM_STATUS	alpacaErrCode;

			alpacaErrCode	=	Read_SensorTemp();
			if (alpacaErrCode == kASCOM_Err_Success)
			{
				TemperatureLog_AddEntry(cCameraProp.CCDtemperature);
			}
			cLastTempUpdate_Secs	=	currentSeconds;
		}
	}
	usleep(25000);
}




#endif // _USE_CAMERA_READ_THREAD_
#endif // _ENABLE_CAMERA_
