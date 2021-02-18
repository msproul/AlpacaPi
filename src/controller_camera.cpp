//*****************************************************************************
//*		controller_camera.cpp		(c) 2020 by Mark Sproul
//*
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
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar  1,	2020	<MLS> Created controller_camera.cpp
//*	Mar  6,	2020	<MLS> Added filename display
//*	Mar  6,	2020	<MLS> ATIK cameras do not support gain, disabled if ATIK
//*	Mar  8,	2020	<MLS> Added camera temperature logging
//*	Mar  8,	2020	<MLS> Gain now updating properly if set from elsewhere
//*	Mar  9,	2020	<MLS> Added flag so we dont keep asking for ccd temp when not supported
//*	Mar  9,	2020	<MLS> Added cHas_readall flag
//*	Mar  9,	2020	<MLS> Added AlpacaGetStatus_ReadAll() & AlpacaGetStatus_OneAAT()
//*	Mar 10,	2020	<MLS> Re-read startup info when coming back online
//*	Mar 14,	2020	<MLS> On exit, turn the cooler off if it is on.
//*	Mar 17,	2020	<MLS> Error messages are now being displayed
//*	Mar 20,	2020	<MLS> Alpaca driver version now being displayed
//*	Mar 22,	2020	<MLS> Added SetFileNameOptions()
//*	Apr  4,	2020	<MLS> Stopped checking on cooler state if it is not present
//*	Apr  7,	2020	<MLS> Moving temp graph to graph tab
//*	Apr  7,	2020	<MLS> Added advanced tab
//*	Apr 21,	2020	<MLS> Added about box to camera controller
//*	Jun 24,	2020	<MLS> Made decision to switch camera to have sub classes
//*	Jun 24,	2020	<MLS> Added series of Update...() functions for sub class use
//*	Jun 24,	2020	<MLS> Added DownloadImage()
//*	Jun 25,	2020	<MLS> Added UpdateReceivedFileName()
//*	Jun 25,	2020	<MLS> Added UpdateCameraTemperature()
//*	Jun 29,	2020	<MLS> Added UpdateBackgroundColor()
//*	Jun 29,	2020	<MLS> Added UpdateFreeDiskSpace()
//*	Jan 15,	2021	<MLS> Added DownloadImage_rgbarray() & DownloadImage_imagearray()
//*	Jan 16,	2021	<MLS> Now able to download monochrome image using "imagearray"
//*	Jan 17,	2021	<MLS> Changed  UpdateReadAllStatus() to UpdateSupportedActions()
//*	Jan 25,	2021	<MLS> Converted CameraController to use properties struct
//*	Jan 29,	2021	<MLS> Added support for RANK=3 in DownloadImage_imagearray()
//*	Feb 14,	2021	<MLS> Added SetExposure() and SetGain()
//*****************************************************************************
//*	Jan  1,	2121	<TODO> control key for different step size.
//*	Jan  1,	2121	<TODO> work on fits view to handle color fits images
//*	Jan  1,	2121	<TODO> add error list window
//*	Jan  1,	2121	<TODO> save cross hair location
//*	Jan  1,	2121	<TODO> finish exposure step options
//*	Jan  1,	2121	<TODO> clear error msg
//*	Feb  6,	2121	<TODO> Move downloading of images to a separate thread
//*****************************************************************************

#ifdef _ENABLE_CTRL_CAMERA_



#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#include	"alpaca_defs.h"

#define	_DEBUG_TIMING_
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kCamWindowWidth		456
#define	kCamWindowHeight	800


#include	"windowtab_camera.h"
#include	"windowtab_camgraph.h"
#include	"windowtab_filelist.h"
#include	"windowtab_camsettings.h"
#include	"windowtab