//**************************************************************************
//*	Name:			cameradriver_OGMA.cpp
//*
//*	Author:			Mark Sproul (C) 2024
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
//*
//*	References:
//*		https://getogma.com/
//*		https://getogma.com/download
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 21,	2024	<MLS> Discussed Alpaca protocol with Juan Martinez at OGMA
//*	Apr 21,	2024	<MLS> Juan told me that OGMA is almost identical to ToupTek, nearly identical SDK
//*	Apr 21,	2024	<MLS> OGMA cameras gave me one of their low-end cameras https://getogma.com/
//*	Apr 22,	2024	<MLS> Downloaded SDK from https://github.com/OGMAvision/OGMAcamSDK
//*	Apr 23,	2024	<MLS> Created cameradriver_OGMA.cpp
//*****************************************************************************


#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_OGMA_)

#include	<math.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<time.h>
#include	<unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#define _USE_PDB_ADDITIONS_

#include	"JsonResponse.h"
#include	"eventlogging.h"


#define OGMACAM_HRESULT_ERRORCODE_NEEDED
#include	"../OGMAcamSDK/inc/ogmacam.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"cameradriver.h"
#include	"cameradriver_OGMA.h"

#ifndef S_OK
//	//*	these are not defined in toupcam.h, they should be
//	#define		S_OK			0x00000000		//	Operation successful
//	#define		S_FALSE			0x00000001		//
//	#define		E_FAIL			0x80004005		//	Unspecified failure
//	#define		E_ACCESSDENIED	0x80070005		//	General access denied error
//	#define		E_INVALIDARG	0x80070057		//	One or more arguments are not valid
//	#define		E_NOTIMPL		0x80004001		//	Not supported or not implemented
//	#define		E_NOINTERFACE	0x80004002		//	Interface not supported
//	#define		E_POINTER		0x80004003		//	Pointer that is not valid
//	#define		E_UNEXPECTED	0x8000FFFF		//	Unexpected failure
//	#define		E_OUTOFMEMORY	0x8007000E		//	Out of memory
//	#define		E_WRONG_THREAD	0x8001010E		//	call function in the wrong thread
//	#define		E_GEN_FAILURE	0x8007001F		//	device not functioning
#endif // S_OK


#define	kMaxCameraCnt	5



//**************************************************************************************
int	CreateCameraObjects_OGMA(void)
{
int				cameraCount;
int				ii;
bool			rulesFileOK;
int				ogmaCameraCnt;
OgmacamDeviceV2	ogmaCamList[OGMACAM_MAX];
char			rulesFileName[]	=	"99-toupcam.rules";
char			driverVersionString[64];

	CONSOLE_DEBUG(__FUNCTION__);
	cameraCount	=	0;
	strcpy(driverVersionString, Ogmacam_Version());
	CONSOLE_DEBUG_W_STR("Ogmacam_Version", driverVersionString);
	LogEvent(	"camera",
				"Library version (TOUP)",
				NULL,
				kASCOM_Err_Success,
				driverVersionString);
	AddLibraryVersion("camera", "TOUP", driverVersionString);


	ogmaCameraCnt	=	Ogmacam_EnumV2(ogmaCamList);
	CONSOLE_DEBUG_W_NUM("ogmaCameraCnt\t=", ogmaCameraCnt);

	//*	check to make sure the rules file is present
	rulesFileOK	=	Check_udev_rulesFile(rulesFileName);
	if (rulesFileOK)
	{
		CONSOLE_DEBUG_W_STR("rules is present:", rulesFileName);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("rules is MISSING:", rulesFileName);
		LogEvent(	"camera",
					"Problem with TOUP rules",
					NULL,
					kASCOM_Err_Success,
					rulesFileName);
	}


	if (ogmaCameraCnt > 0)
	{
		for (ii=0; ii < ogmaCameraCnt; ii++)
		{
			new CameraDriverOGMA(ii);
			cameraCount++;
		}
//		CONSOLE_ABORT(__FUNCTION__);
	}
	return(cameraCount);
}




//**************************************************************************************
static void __stdcall EventCallback(unsigned nEvent, void* pCallbackCtx)
{
CameraDriverOGMA	*thisPtr;

//	CONSOLE_DEBUG(__FUNCTION__);

	thisPtr	=	(CameraDriverOGMA *)pCallbackCtx;
	if (thisPtr != NULL)
	{
		thisPtr->HandleOGMAcallbackEvent(nEvent);
	}
}


//**************************************************************************************
CameraDriverOGMA::CameraDriverOGMA(const int deviceNum)
	:CameraDriver()
{
HRESULT			ogmaResult;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cDeviceManufAbrev,	"OGMA");


	cCameraID					=	deviceNum;
	cOGMApicReady				=	false;
	cIsTriggerCam				=	false;
	cCameraProp.ExposureMin_us	=	400;				//*	0.4 ms
	cCameraProp.ExposureMax_us	=	800 * 1000 *1000;	//*	800 seconds

	ReadCameraInfo();

	strcpy(cCommonProp.Description, cDeviceManufacturer);
	strcat(cCommonProp.Description, " - Model:");
	strcat(cCommonProp.Description, cCommonProp.Name);

#ifdef _USE_OPENCV_
	sprintf(cOpenCV_ImgWindowName, "%s-%d", cCommonProp.Name, cCameraID);
#endif // _USE_OPENCV_

	if (cOGMAcamH != NULL)
	{
		ogmaResult	=	Ogmacam_StartPullModeWithCallback(cOGMAcamH, EventCallback, this);
		if (FAILED(ogmaResult))
		{
			CONSOLE_DEBUG("FAILED PULLBACK");
		}
	}

}


//**************************************************************************************
// Destructor
//**************************************************************************************
CameraDriverOGMA::~CameraDriverOGMA(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
typedef struct
{
	uint64_t		flag;
	char			description[64];
} TYPE_FLAGtable;

TYPE_FLAGtable	gCameraFlagTable[]	=
{

	{	OGMACAM_FLAG_CMOS,					"OGMACAM_FLAG_CMOS"	},
	{	OGMACAM_FLAG_CCD_PROGRESSIVE,		"OGMACAM_FLAG_CCD_PROGRESSIVE"	},
	{	OGMACAM_FLAG_CCD_INTERLACED,		"OGMACAM_FLAG_CCD_INTERLACED"	},
	{	OGMACAM_FLAG_ROI_HARDWARE,			"OGMACAM_FLAG_ROI_HARDWARE"	},
	{	OGMACAM_FLAG_MONO,					"OGMACAM_FLAG_MONO"	},
	{	OGMACAM_FLAG_BINSKIP_SUPPORTED,		"OGMACAM_FLAG_BINSKIP_SUPPORTED"	},
	{	OGMACAM_FLAG_USB30,					"OGMACAM_FLAG_USB30"	},
	{	OGMACAM_FLAG_TEC,					"OGMACAM_FLAG_TEC"	},
	{	OGMACAM_FLAG_USB30_OVER_USB20,		"OGMACAM_FLAG_USB30_OVER_USB20"	},
	{	OGMACAM_FLAG_ST4,					"OGMACAM_FLAG_ST4"	},
	{	OGMACAM_FLAG_GETTEMPERATURE,		"OGMACAM_FLAG_GETTEMPERATURE"	},
	{	OGMACAM_FLAG_HIGH_FULLWELL,			"OGMACAM_FLAG_HIGH_FULLWELL"	},
	{	OGMACAM_FLAG_RAW10,					"OGMACAM_FLAG_RAW10"	},
	{	OGMACAM_FLAG_RAW12,					"OGMACAM_FLAG_RAW12"	},
	{	OGMACAM_FLAG_RAW14,					"OGMACAM_FLAG_RAW14"	},
	{	OGMACAM_FLAG_RAW16,					"OGMACAM_FLAG_RAW16"	},
	{	OGMACAM_FLAG_FAN,					"OGMACAM_FLAG_FAN"	},
	{	OGMACAM_FLAG_TEC_ONOFF,				"OGMACAM_FLAG_TEC_ONOFF"	},
	{	OGMACAM_FLAG_ISP,					"OGMACAM_FLAG_ISP"	},
	{	OGMACAM_FLAG_TRIGGER_SOFTWARE,		"OGMACAM_FLAG_TRIGGER_SOFTWARE"	},
	{	OGMACAM_FLAG_TRIGGER_EXTERNAL,		"OGMACAM_FLAG_TRIGGER_EXTERNAL"	},
	{	OGMACAM_FLAG_TRIGGER_SINGLE,		"OGMACAM_FLAG_TRIGGER_SINGLE"	},
	{	OGMACAM_FLAG_BLACKLEVEL,			"OGMACAM_FLAG_BLACKLEVEL"	},
	{	OGMACAM_FLAG_AUTO_FOCUS,			"OGMACAM_FLAG_AUTO_FOCUS"	},
	{	OGMACAM_FLAG_BUFFER,				"OGMACAM_FLAG_BUFFER"	},
	{	OGMACAM_FLAG_DDR,					"OGMACAM_FLAG_DDR"	},
	{	OGMACAM_FLAG_CG,					"OGMACAM_FLAG_CG"	},
	{	OGMACAM_FLAG_YUV411,				"OGMACAM_FLAG_YUV411"	},
	{	OGMACAM_FLAG_VUYY,					"OGMACAM_FLAG_VUYY"	},
	{	OGMACAM_FLAG_YUV444,				"OGMACAM_FLAG_YUV444"	},
	{	OGMACAM_FLAG_RGB888,				"OGMACAM_FLAG_RGB888"	},
	{	OGMACAM_FLAG_RAW8,					"OGMACAM_FLAG_RAW8"	},
	{	OGMACAM_FLAG_GMCY8,					"OGMACAM_FLAG_GMCY8"	},
	{	OGMACAM_FLAG_GMCY12,				"OGMACAM_FLAG_GMCY12"	},
	{	OGMACAM_FLAG_UYVY,					"OGMACAM_FLAG_UYVY"	},
	{	OGMACAM_FLAG_CGHDR,					"OGMACAM_FLAG_CGHDR"	},
	{	OGMACAM_FLAG_GLOBALSHUTTER,			"OGMACAM_FLAG_GLOBALSHUTTER"	},
	{	OGMACAM_FLAG_FOCUSMOTOR,			"OGMACAM_FLAG_FOCUSMOTOR"	},
	{	OGMACAM_FLAG_PRECISE_FRAMERATE,		"OGMACAM_FLAG_PRECISE_FRAMERATE"	},
	{	OGMACAM_FLAG_HEAT,					"OGMACAM_FLAG_HEAT"	},
	{	OGMACAM_FLAG_LOW_NOISE,				"OGMACAM_FLAG_LOW_NOISE"	},
	{	OGMACAM_FLAG_LEVELRANGE_HARDWARE,	"OGMACAM_FLAG_LEVELRANGE_HARDWARE"	},
	{	OGMACAM_FLAG_EVENT_HARDWARE,		"OGMACAM_FLAG_EVENT_HARDWARE"	},
	{	OGMACAM_FLAG_LIGHTSOURCE,			"OGMACAM_FLAG_LIGHTSOURCE"	},
	{	OGMACAM_FLAG_FILTERWHEEL,			"OGMACAM_FLAG_FILTERWHEEL"	},
	{	OGMACAM_FLAG_GIGE,					"OGMACAM_FLAG_GIGE"	},
	{	OGMACAM_FLAG_10GIGE,				"OGMACAM_FLAG_10GIGE"	},
	{	OGMACAM_FLAG_5GIGE,					"OGMACAM_FLAG_5GIGE"	},
	{	OGMACAM_FLAG_25GIGE,				"OGMACAM_FLAG_25GIGE"	},
	{	OGMACAM_FLAG_AUTOFOCUSER,			"OGMACAM_FLAG_AUTOFOCUSER"	},
	{	OGMACAM_FLAG_LIGHT_SOURCE,			"OGMACAM_FLAG_LIGHT_SOURCE"	},
	{	OGMACAM_FLAG_CAMERALINK,			"OGMACAM_FLAG_CAMERALINK"	},
	{	OGMACAM_FLAG_CXP,					"OGMACAM_FLAG_CXP"	},
	{	OGMACAM_FLAG_RAW12PACK,				"OGMACAM_FLAG_RAW12PACK"	},
	{	OGMACAM_FLAG_SELFTRIGGER,			"OGMACAM_FLAG_SELFTRIGGER"	},
	{	OGMACAM_FLAG_RAW11,					"OGMACAM_FLAG_RAW11"	},
	{	OGMACAM_FLAG_GHOPTO,				"OGMACAM_FLAG_GHOPTO"	},
	{	0,									"foo"	}
};

//*****************************************************************************
static void	DumpCameraFlags(uint64_t flags)
{
int		iii;

	iii	=	0;
	while (gCameraFlagTable[iii].flag != 0)
	{
		printf("%-48s\t%s\r\n",	gCameraFlagTable[iii].description,
								((flags & gCameraFlagTable[iii].flag) ? "True" : "False"));
		iii++;
	}
}


//*****************************************************************************
void	CameraDriverOGMA::ReadCameraInfo(void)
{
unsigned int	ii;
int				ogmaCameraCnt;
OgmacamDeviceV2	ogmaCamList[OGMACAM_MAX];
HRESULT			ogmaResult;
char			hardwareVer[16];
char			productionDate[10];
char			fpgaVersion[32];
int				bAutoExposure;
int				rgbMode;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cDeviceVersion, Ogmacam_Version());
	CONSOLE_DEBUG_W_STR("Ogmacam_Version", cDeviceVersion);
	ogmaCameraCnt	=	Ogmacam_EnumV2(ogmaCamList);

	if (cCameraID < ogmaCameraCnt)
	{
		cOGMAdeviceInfo	=	ogmaCamList[cCameraID];

		strcpy(cDeviceManufacturer,	"OGMA");
		strcpy(cDeviceManufAbrev,	"OGMA");
		strcpy(cDeviceSerialNum,	cOGMAdeviceInfo.id);

		SetCommonPropertyName(NULL, cOGMAdeviceInfo.displayname);

		cCameraProp.CameraXsize	=	cOGMAdeviceInfo.model->res[0].width;
		cCameraProp.CameraYsize	=	cOGMAdeviceInfo.model->res[0].height;
		cCameraProp.NumX		=	cCameraProp.CameraXsize;
		cCameraProp.NumY		=	cCameraProp.CameraYsize;
		cCameraProp.PixelSizeY	=	cOGMAdeviceInfo.model->xpixsz;
		cCameraProp.PixelSizeX	=	cOGMAdeviceInfo.model->ypixsz;
		cIsColorCam				=	true;

		if ((cCameraProp.CameraXsize > 0) && (cCameraProp.CameraYsize > 0))
		{
			AllocateImageBuffer(0);
		}

		CONSOLE_DEBUG_W_STR("Display name\t=",	cOGMAdeviceInfo.displayname);
		CONSOLE_DEBUG_W_STR("ID\t\t\t=",		cOGMAdeviceInfo.id);

		CONSOLE_DEBUG_W_STR("name\t\t=",		cOGMAdeviceInfo.model->name);

		CONSOLE_DEBUG_W_HEX("flag\t\t=",		(int)cOGMAdeviceInfo.model->flag);
		CONSOLE_DEBUG_W_HEX("flag\t\t=",		(int)(cOGMAdeviceInfo.model->flag >> 32));
		CONSOLE_DEBUG_W_NUM("preview\t\t=",		cOGMAdeviceInfo.model->preview);
		CONSOLE_DEBUG_W_NUM("still\t\t=",		cOGMAdeviceInfo.model->still);
		CONSOLE_DEBUG_W_NUM("ioctrol\t\t=",		cOGMAdeviceInfo.model->ioctrol);
		CONSOLE_DEBUG_W_NUM("maxfanspeed\t=",	cOGMAdeviceInfo.model->maxfanspeed);
		DumpCameraFlags(cOGMAdeviceInfo.model->flag);

		for (ii=0; ii<cOGMAdeviceInfo.model->preview; ii++)
		{
			CONSOLE_DEBUG_W_NUM("width\t\t=",		cOGMAdeviceInfo.model->res[ii].width);
			CONSOLE_DEBUG_W_NUM("height\t\t=",		cOGMAdeviceInfo.model->res[ii].height);
		}

		//*	check these in this order to make sure we record the highest value
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_RAW8)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_RAW8");
			cBitDepth		=	8;
			cOGMAcamFormat	=	OGMACAM_FLAG_RAW8;
			AddReadoutModeToList(kImageType_RAW8);
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_RAW10)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_RAW10");
			cBitDepth		=	10;
			cOGMAcamFormat	=	OGMACAM_FLAG_RAW16;
			AddReadoutModeToList(kImageType_RAW16);
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_RAW12)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_RAW12");
			cBitDepth		=	12;
			cOGMAcamFormat	=	OGMACAM_FLAG_RAW16;
			AddReadoutModeToList(kImageType_RAW16);
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_RAW14)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_RAW14");
			cBitDepth		=	14;
			cOGMAcamFormat	=	OGMACAM_FLAG_RAW16;
			AddReadoutModeToList(kImageType_RAW16);
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_RAW16)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_RAW16");
			cBitDepth		=	16;
			cOGMAcamFormat	=	OGMACAM_FLAG_RAW16;
			AddReadoutModeToList(kImageType_RAW16);
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_RGB888)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_RGB888");
			cBitDepth		=	8;
			cOGMAcamFormat	=	OGMACAM_FLAG_RGB888;
			AddReadoutModeToList(kImageType_RGB24);
		}
			AddReadoutModeToList(kImageType_RGB24);

		//-------------------------------------------------------------
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_ST4)
		{
			cSt4Port	=	true;
			CONSOLE_DEBUG("OGMACAM_FLAG_ST4");
		}


		//---------------------------------------------
		cOGMAcamH	=	Ogmacam_Open(cOGMAdeviceInfo.id);
		if (cOGMAcamH != NULL)
		{
		unsigned int	nFourCC;
		unsigned int	bitsperpixel;
		char			fourCCstring[8];
		char			serialNumberStr[64];
//		int				finalWidth;
//		int				finalHeight;
		char			firmWareVersion[32];

			CONSOLE_DEBUG("Ogmacam_Open OK");
#ifdef _USE_PDB_ADDITIONS_
			ogmaResult  =	Ogmacam_put_Option(cOGMAcamH, OGMACAM_OPTION_RAW, 1);
			CONSOLE_DEBUG_W_HEX("ogmaResult\t\t=",		ogmaResult);
			if (FAILED(ogmaResult))
			{
				CONSOLE_DEBUG("Failed RAW");
			}
			if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_TRIGGER_SOFTWARE)
			{
				cIsTriggerCam	=	true;	//*	<MLS> 1/25/2021
				CONSOLE_DEBUG("OGMACAM_FLAG_TRIGGER_SOFTWARE");
				ogmaResult	=	Ogmacam_put_Option(cOGMAcamH, OGMACAM_OPTION_TRIGGER, 1);
			}
			ogmaResult	=	Ogmacam_get_RawFormat(cOGMAcamH, &nFourCC, &bitsperpixel);
#endif


			ogmaResult	=	Ogmacam_get_RawFormat(cOGMAcamH, &nFourCC, &bitsperpixel);
			CONSOLE_DEBUG_W_HEX("ogmaResult\t\t=",		ogmaResult);
			CONSOLE_DEBUG_W_NUM("bitsperpixel\t=",		bitsperpixel);
			CONSOLE_DEBUG_W_HEX("nFourCC\t\t=",			nFourCC);

			if (nFourCC == 'GRBG')
			{
				CONSOLE_DEBUG("nFourCC == 'GRBG'");
				AddReadoutModeToList(kImageType_RGB24);
			}
			fourCCstring[0]	=	(nFourCC >> 24) & 0x0ff;
			fourCCstring[1]	=	(nFourCC >> 16) & 0x0ff;
			fourCCstring[2]	=	(nFourCC >> 8) & 0x0ff;
			fourCCstring[3]	=	(nFourCC) & 0x0ff;
			fourCCstring[4]	=	0;
			CONSOLE_DEBUG_W_STR("nFourCC\t\t=",					fourCCstring);

			cBitDepth	=	Ogmacam_get_MaxBitDepth(cOGMAcamH);
			CONSOLE_DEBUG_W_NUM("cBitDepth\t\t=",				cBitDepth);

			ogmaResult	=	Ogmacam_get_SerialNumber(cOGMAcamH,	serialNumberStr);
			CONSOLE_DEBUG_W_STR("serialNumber\t=",				serialNumberStr);

			ogmaResult	=	Ogmacam_get_FwVersion(cOGMAcamH,	firmWareVersion);
			CONSOLE_DEBUG_W_STR("firmWareVersion\t=",			firmWareVersion);
			if (SUCCEEDED(ogmaResult))
			{
				strcpy(cDeviceFirmwareVersStr, firmWareVersion);
			}

			//*	get the camera hardware version, such as: 3.12
			ogmaResult	=	Ogmacam_get_HwVersion(cOGMAcamH, hardwareVer);
			CONSOLE_DEBUG_W_STR("hardware version\t=",		hardwareVer);

			//*	get the production date, such as: 20150327, YYYYMMDD, (YYYY: year, MM: month, DD: day)
			ogmaResult	=	Ogmacam_get_ProductionDate(cOGMAcamH, productionDate);
			if (SUCCEEDED(ogmaResult))
			{
				CONSOLE_DEBUG_W_STR("production date\t=",		productionDate);
				strcpy(cCameraProp.ProductionDate,				productionDate);
			}
			//*	get the FPGA version, such as: 1.13
			ogmaResult	=	Ogmacam_get_FpgaVersion(cOGMAcamH, fpgaVersion);
			if (SUCCEEDED(ogmaResult))
			{
				CONSOLE_DEBUG_W_STR("FPGA version\t=",		fpgaVersion);
				strcpy(cCameraProp.FPGAversion,				fpgaVersion);
			}
			//-----------------------------------------------------------
			ogmaResult	=	Ogmacam_get_Option(cOGMAcamH, OGMACAM_OPTION_RGB, &rgbMode);
			if (SUCCEEDED(ogmaResult))
			{
				CONSOLE_DEBUG_W_NUM(	"rgbMode\t=", rgbMode);
				CONSOLE_DEBUG(			"           0 => RGB24;");
				CONSOLE_DEBUG(			"           1 => enable RGB48 format when bitdepth > 8;");
				CONSOLE_DEBUG(			"           2 => RGB32;");
				CONSOLE_DEBUG(			"           3 => 8 Bits Gray (only for mono camera");
				CONSOLE_DEBUG(			"           4 => 16 Bits Gray (only for mono camera when bitdepth > 8)");
				CONSOLE_DEBUG(			"           5 => 64(RGB64)");
			}
			else
			{
				CONSOLE_DEBUG_W_HEX("Ogmacam_get_Option failed, ogmaResult\t=", ogmaResult);
			}

			//-----------------------------------------------------------
			//-----------------------------------------------------------
			ogmaResult	=	Ogmacam_get_MonoMode(cOGMAcamH);
			if (SUCCEEDED(ogmaResult))
			{
				//*	S_FALSE:    color mode
				//*	S_OK:       mono mode, such as EXCCD00300KMA and UHCCD01400KMA
				if (ogmaResult == S_OK)
				{
					CONSOLE_DEBUG("Ogmacam_get_MonoMode() returned S_OK");
					cIsColorCam	=	false;
				}
				else if (ogmaResult == S_FALSE)
				{
					CONSOLE_DEBUG("Ogmacam_get_MonoMode() returned S_FALSE");
					cIsColorCam	=	true;
				}
				CONSOLE_DEBUG_W_BOOL("cIsColorCam\t=", cIsColorCam);
			}
			else
			{
				CONSOLE_DEBUG_W_HEX("Ogmacam_get_MonoMode() failed, ogmaResult\t=", ogmaResult);
			}

			// Disable AutoExposure
			ogmaResult	=	Ogmacam_put_AutoExpoEnable(cOGMAcamH, false);

			ogmaResult	=	Ogmacam_get_AutoExpoEnable(cOGMAcamH, &bAutoExposure);
			if (SUCCEEDED(ogmaResult))
			{
				CONSOLE_DEBUG_W_NUM("Auto Exposure enabled\t=",	bAutoExposure);
				if (bAutoExposure != 0)
				{
					cCameraAutoExposure	=	true;
				}
				else
				{
					cCameraAutoExposure	=	false;
				}
			}

		//	CONSOLE_DEBUG("Calling Ogmacam_get_FinalSize");
		//	ogmaResult	=	Ogmacam_get_FinalSize(cOGMAcamH, &finalWidth, &finalHeight);
		//	CONSOLE_DEBUG_W_NUM("finalWidth\t=",	finalWidth);
		//	CONSOLE_DEBUG_W_NUM("finalHeight\t=",	finalHeight);
		//	int		testPattern;
		//	ogmaResult	=	Ogmacam_get_Option(cOGMAcamH, Ogmacam_OPTION_TESTPATTERN, &testPattern);
		//	if (SUCCEEDED(ogmaResult))
		//	{
		//		CONSOLE_DEBUG_W_NUM("testPattern\t=",	testPattern);
		//		ogmaResult	=	Ogmacam_put_Option(cOGMAcamH, Ogmacam_OPTION_TESTPATTERN, 9);
		//	}
		//	else
		//	{
		//		CONSOLE_DEBUG_W_HEX("Ogmacam_get_Option() failed, ogmaResult = ", ogmaResult);
		//	}


		}
		else
		{
			CONSOLE_DEBUG("Ogmacam_Open FAILED!!!!");
		}
		//---------------------------------------------

		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_CMOS)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_CMOS");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_CCD_PROGRESSIVE)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_CCD_PROGRESSIVE");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_CCD_INTERLACED)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_CCD_INTERLACED");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_ROI_HARDWARE)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_ROI_HARDWARE");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_MONO)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_MONO");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_BINSKIP_SUPPORTED)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_BINSKIP_SUPPORTED");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_USB30)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_USB30");
			cIsUSB3Camera	=	true;
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_TEC)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_TEC");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_USB30_OVER_USB20)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_USB30_OVER_USB20");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_GETTEMPERATURE)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_GETTEMPERATURE");
		}
#ifdef OGMACAM_FLAG_PUTTEMPERATURE
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_PUTTEMPERATURE)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_PUTTEMPERATURE");
		}
#endif
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_FAN)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_FAN");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_TEC_ONOFF)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_TEC_ONOFF");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_ISP)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_ISP");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_TRIGGER_SOFTWARE)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_TRIGGER_SOFTWARE");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_TRIGGER_EXTERNAL)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_TRIGGER_EXTERNAL");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_TRIGGER_SINGLE)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_TRIGGER_SINGLE");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_BLACKLEVEL)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_BLACKLEVEL");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_AUTO_FOCUS)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_AUTO_FOCUS");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_BUFFER)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_BUFFER");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_DDR)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_DDR");
		}

		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_CG)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_CG");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_YUV411)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_YUV411");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_VUYY)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_VUYY");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_YUV444)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_YUV444");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_RGB888)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_RGB888");
			AddReadoutModeToList(kImageType_RGB24);
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_GMCY8)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_GMCY8");
			AddReadoutModeToList(kImageType_Y8);
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_GMCY12)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_GMCY12");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_UYVY)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_UYVY");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_CGHDR)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_CGHDR");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_GLOBALSHUTTER)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_GLOBALSHUTTER");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_FOCUSMOTOR)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_FOCUSMOTOR");
		}
		if (cOGMAdeviceInfo.model->flag & OGMACAM_FLAG_PRECISE_FRAMERATE)
		{
			CONSOLE_DEBUG("OGMACAM_FLAG_PRECISE_FRAMERATE");
		}
	}
}

//*****************************************************************************
//*	the camera must already be open when this is called
//*****************************************************************************
bool	CameraDriverOGMA::GetImage_ROI_info(void)
{

//	memset(&cROIinfo, 0, sizeof(TYPE_IMAGE_ROI_Info));

//	cROIinfo.currentROIimageType	=	kImageType_RAW8; // PDB kImageType_RGB24;
	cROIinfo.currentROIwidth		=	cCameraProp.CameraXsize;
	cROIinfo.currentROIheight		=	cCameraProp.CameraYsize;
	cROIinfo.currentROIbin			=	1;

	return(true);
}


//*****************************************************************************
//*	http://192.168.0.201:6800/api/v1.0.0-oas3/camera/0/startexposure Content-Type: -dDuration=0.001&Light=true
//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverOGMA::Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;
HRESULT					ogmaResult;

	//*	this camera is a bit different, it is ALWAYS exposing,
	//*	istrigger

	cInternalCameraState	=	kCameraState_TakingPicture;
	alpacaErrCode			=	kASCOM_Err_Success;

	gettimeofday(&cCameraProp.Lastexposure_StartTime, NULL);

	// Still to add some error checking --PDB--
	Ogmacam_put_ExpoTime(cOGMAcamH, exposureMicrosecs);

	// Trigger an exposure if triggercam

	// Still to add some error checking --PDB--
	if (cIsTriggerCam == true)
	{
		ogmaResult	=	Ogmacam_Trigger(cOGMAcamH, 1);
		if (FAILED(ogmaResult))
		{
			CONSOLE_DEBUG_W_NUM("Ogmacam_Trigger() returned error code:", ogmaResult);
		}
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverOGMA::Stop_Exposure(void)
{
TYPE_ASCOM_STATUS				alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);

	CONSOLE_DEBUG(cLastCameraErrMsg);

	alpacaErrCode	=	kASCOM_Err_Success;

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_EXPOSURE_STATUS	CameraDriverOGMA::Check_Exposure(bool verboseFlag)
{
TYPE_EXPOSURE_STATUS	exposureState;

	CONSOLE_DEBUG_W_STR("cInternalCameraState\t=", gCameraStateStrings[cInternalCameraState]);
	switch(cInternalCameraState)
	{
		case kCameraState_Idle:
			exposureState	=	kExposure_Idle;
			break;

		case kCameraState_TakingPicture:
			if (cOGMApicReady)
			{
				exposureState	=	kExposure_Success;
				cOGMApicReady	=	false;
			}
			else
			{
				exposureState	=	kExposure_Working;
			}
			break;

		case kCameraState_StartVideo:
			exposureState	=	kExposure_Idle;
			break;

		case kCameraState_TakingVideo:
			exposureState	=	kExposure_Idle;
			break;

		default:
			exposureState	=	kExposure_Idle;
			break;
	}
	return(exposureState);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverOGMA::SetImageType(TYPE_IMAGE_TYPE newImageType)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					ogma_bitDepth;
int					ofma_pixelFormat;
int					rgbMode;
HRESULT				ogmaResult;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cOGMAcamH != NULL)
	{
		ogma_bitDepth	=	-1;
		ogmaResult			=	Ogmacam_get_Option(cOGMAcamH, OGMACAM_OPTION_BITDEPTH, &ogma_bitDepth);
		if (SUCCEEDED(ogmaResult))
		{
			CONSOLE_DEBUG_W_NUM("ogma_bitDepth (0 = 8 bits mode, 1 = 16 bits mode)\t=", ogma_bitDepth);
#define OGMACAM_PIXELFORMAT_RAW8			0x00
#define OGMACAM_PIXELFORMAT_RAW10			0x01
#define OGMACAM_PIXELFORMAT_RAW12			0x02
#define OGMACAM_PIXELFORMAT_RAW14			0x03
#define OGMACAM_PIXELFORMAT_RAW16			0x04
#define OGMACAM_PIXELFORMAT_YUV411			0x05
#define OGMACAM_PIXELFORMAT_VUYY			0x06
#define OGMACAM_PIXELFORMAT_YUV444			0x07
#define OGMACAM_PIXELFORMAT_RGB888			0x08
			switch(newImageType)
			{
				case kImageType_RAW8:
				case kImageType_Y8:
					ogma_bitDepth		=	0;
					ofma_pixelFormat	=	OGMACAM_PIXELFORMAT_RAW8;
					break;

				case kImageType_RGB24:
					ogma_bitDepth		=	0;
					ofma_pixelFormat	=	OGMACAM_PIXELFORMAT_RGB888;
					break;

				case kImageType_RAW16:
					ogma_bitDepth		=	1;
					ofma_pixelFormat	=	OGMACAM_PIXELFORMAT_RAW16;
					break;

				default:
					ogma_bitDepth		=	0;
					ofma_pixelFormat	=	OGMACAM_PIXELFORMAT_RAW8;
					break;
			}
			//--------------------------------------------------------------------------------------
			//*	set bit depth
			ogmaResult	=	Ogmacam_put_Option(cOGMAcamH, OGMACAM_OPTION_BITDEPTH, ogma_bitDepth);
			if (SUCCEEDED(ogmaResult))
			{
				cROIinfo.currentROIimageType	=	newImageType;
				cDesiredImageType				=	newImageType;
				alpacaErrCode					=   kASCOM_Err_Success;
			}
			else
			{
				CONSOLE_DEBUG_W_HEX("Ogmacam_put_Option failed, ogmaResult\t=", ogmaResult);
			}
			ogmaResult	=	Ogmacam_get_Option(cOGMAcamH, OGMACAM_OPTION_BITDEPTH, &ogma_bitDepth);
			CONSOLE_DEBUG_W_NUM("bitDepth (0 = 8 bits mode, 1 = 16 bits mode)\t=", ogma_bitDepth);

			ogmaResult	=	Ogmacam_get_Option(cOGMAcamH, OGMACAM_OPTION_RGB, &rgbMode);
			if (SUCCEEDED(ogmaResult))
			{
				CONSOLE_DEBUG_W_NUM(	"rgbMode\t=", rgbMode);
				CONSOLE_DEBUG(			"           0 => RGB24;");
				CONSOLE_DEBUG(			"           1 => enable RGB48 format when bitdepth > 8;");
				CONSOLE_DEBUG(			"           2 => RGB32;");
				CONSOLE_DEBUG(			"           3 => 8 Bits Gray (only for mono camera");
				CONSOLE_DEBUG(			"           4 => 16 Bits Gray (only for mono camera when bitdepth > 8)");
				CONSOLE_DEBUG(			"           5 => 64(RGB64)");
			}
			else
			{
				CONSOLE_DEBUG_W_HEX("Ogmacam_get_Option failed, ogmaResult\t=", ogmaResult);
			}
			//--------------------------------------------------------------------------------------
			//*	set pixel format
			ogmaResult	=	Ogmacam_put_Option(cOGMAcamH, OGMACAM_OPTION_PIXEL_FORMAT, ofma_pixelFormat);
			if (SUCCEEDED(ogmaResult))
			{

			}
			else
			{
				CONSOLE_DEBUG_W_HEX("OGMACAM_OPTION_PIXEL_FORMAT failed, ogmaResult\t=", ogmaResult);
			}


		}
		else if ((unsigned int)ogmaResult == E_NOTIMPL)
		{
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			strcpy(cLastCameraErrMsg, "Camera does not support specified pixel depth");
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_FailedUnknown;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=", alpacaErrCode);
	return(alpacaErrCode);
}

#pragma mark -
#pragma mark Virtual functions

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverOGMA::Read_Gain(int *cameraGainValue)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
unsigned short		gain;
HRESULT				ogmaResult;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cOGMAcamH != NULL)
	{
//		CONSOLE_DEBUG("reading gain");
		ogmaResult	=	Ogmacam_get_ExpoAGain(cOGMAcamH, &gain);
		if (SUCCEEDED(ogmaResult))
		{
//			CONSOLE_DEBUG("found gain");
			*cameraGainValue	=	gain;
			alpacaErrCode		=	kASCOM_Err_Success;
		}
		else
		{
			if ((unsigned int)ogmaResult == E_NOTIMPL)
			{
				alpacaErrCode	=	kASCOM_Err_NotImplemented;
				strcpy(cLastCameraErrMsg, "Camera does not support gain");
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_FailedUnknown;
			}
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
//	CONSOLE_DEBUG(cLastCameraErrMsg);
	return(alpacaErrCode);
}


//*****************************************************************************
//	Set Cooling:
//	int ArtemisSetCooling(ArtemisHandle hCam, int setpoint);
//	This function is used to set the temperature of the camera. The setpoint is in 1/100 of a degree
//	(Celcius). So, to set the cooling to -10C, you need to call the function with setpoint = -1000.
//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverOGMA::Cooler_TurnOn(void)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);

	CONSOLE_DEBUG(cLastCameraErrMsg);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverOGMA::Cooler_TurnOff(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
	CONSOLE_DEBUG(cLastCameraErrMsg);

	return(alpacaErrCode);
}


//**************************************************************************
//*	returns error code,
//*	sets class varible to current temp
//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverOGMA::Read_SensorTemp(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
short				temperature;
HRESULT				ogmaResult;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cOGMAcamH != NULL)
	{
		ogmaResult	=	Ogmacam_get_Temperature(cOGMAcamH, &temperature);
		if (SUCCEEDED(ogmaResult))
		{
			cCameraProp.CCDtemperature	=	(temperature * 1.0) / 10.0;
		}
		else if ((unsigned int)ogmaResult == E_NOTIMPL)
		{
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			strcpy(cLastCameraErrMsg, "Camera does not support tempeature");
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_FailedUnknown;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverOGMA::Read_CoolerState(bool *coolerOnOff)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
//	CONSOLE_DEBUG(cLastCameraErrMsg);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverOGMA::Read_CoolerPowerLevel(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
	CONSOLE_DEBUG(cLastCameraErrMsg);

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverOGMA::Read_Fastreadout(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
//	CONSOLE_DEBUG(cLastCameraErrMsg);
	return(alpacaErrCode);
}

//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverOGMA::Read_ImageData(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
//OgmacamFrameInfoV2	ogmaFrameInfo;

	CONSOLE_DEBUG(__FUNCTION__);

//	memset((void*)&ogmaFrameInfo, 0, sizeof(OgmacamFrameInfoV2));

	GetImage_ROI_info();

	if ((cOGMAcamH != NULL) && (cCameraDataBuffer != NULL))
	{
		cNewImageReadyToDisplay	=	true;
		cOGMApicReady			=	false;
	}
	else
	{
		CONSOLE_DEBUG("Invalid device number or camera buffer failed to allocate");
	}
	return(alpacaErrCode);
}

//**************************************************************************
void	CameraDriverOGMA::HandleOGMAcallbackEvent(unsigned nEvent)
{
OgmacamFrameInfoV2	ogmaFrameInfo;
HRESULT				ogmaResult;

	memset((void*)&ogmaFrameInfo, 0, sizeof(OgmacamFrameInfoV2));

	switch(nEvent)
	{
		case OGMACAM_EVENT_EXPOSURE:		//*	exposure time changed
			CONSOLE_DEBUG("OGMACAM_EVENT_EXPOSURE");
			if (cOGMAcamH != NULL)
			{
				ogmaResult	=	Ogmacam_get_ExpoTime(cOGMAcamH, &cOGMAautoExpTime_us);	//*	in microseconds
				if (SUCCEEDED(ogmaResult))
				{
					CONSOLE_DEBUG_W_NUM("expTime_us\t=", cOGMAautoExpTime_us);
					cCurrentExposure_us						=	cOGMAautoExpTime_us;
					cCameraProp.Lastexposure_duration_us	=	cOGMAautoExpTime_us;
				}
				else
				{
					CONSOLE_DEBUG_W_HEX("Ogmacam_get_ExpoTime() failed, ogmaResult = ", ogmaResult);
				}
			}
			break;

		case OGMACAM_EVENT_TEMPTINT:		//*	white balance changed, Temp/Tint mode
			CONSOLE_DEBUG_W_NUM(__FUNCTION__, nEvent);
			break;

		case OGMACAM_EVENT_IMAGE:			//*	live image arrived, use Toupcam_PullImage to get this image
//			CONSOLE_DEBUG("OGMACAM_EVENT_IMAGE");
// Think it is better move this code out of the call back function
// Probable better place is read_imagedata function, but then some
// indicator should be set here that read_imageda should handle the pull_image
//
// PDB

			if ((cOGMAcamH != NULL) && (cCameraDataBuffer != NULL))
			{
				//*	we do not want to read the image if an image save is in progress

				// Hardcoded 16bit for now. Must be changed to the proper value based on camerasetting
				// To be done -PDB-
				ogmaResult	=	Ogmacam_PullImageV2(cOGMAcamH, cCameraDataBuffer, 16, &ogmaFrameInfo);
				if (SUCCEEDED(ogmaResult))
				{
				//	CONSOLE_DEBUG_W_HEX("ogmaFrameInfo.flag\t\t=",		ogmaFrameInfo.flag);
				//	CONSOLE_DEBUG_W_NUM("ogmaFrameInfo.timestamp\t=",	ogmaFrameInfo.timestamp);
					if (cCameraAutoExposure)
					{
						cCurrentExposure_us						=	cOGMAautoExpTime_us;
						cCameraProp.Lastexposure_duration_us	=	cOGMAautoExpTime_us;
					}

					cNewImageReadyToDisplay	=	true;
					if (cInternalCameraState == kCameraState_TakingPicture)
					{
						cOGMApicReady	=	true;
					}
					//*	get the frame rate: framerate (fps) = Frame * 1000.0 / nTime
					unsigned	nFrame;
					unsigned	nTime;
					unsigned	nTotalFrame;
				//	double		myFrameRate;
					ogmaResult	=	Ogmacam_get_FrameRate(cOGMAcamH, &nFrame, &nTime, &nTotalFrame);
					if (SUCCEEDED(ogmaResult))
					{
				//		myFrameRate	=	(nFrame * 1000.0) / nTime;
					//	CONSOLE_DEBUG_W_NUM("nFrame\t=",		nFrame);
					//	CONSOLE_DEBUG_W_NUM("nTime\t\t=",		nTime);
					//	CONSOLE_DEBUG_W_NUM("nTotalFrame\t=",	nTotalFrame);
//						CONSOLE_DEBUG_W_DBL("myFrameRate\t=",	myFrameRate);
					}
				}
				else
				{
					CONSOLE_DEBUG_W_HEX("failed to pull image, ogmaResult = ", ogmaResult);
				}
			}
			else
			{
				CONSOLE_DEBUG("Internal error");
				CONSOLE_ABORT(__FUNCTION__);
			}
			break;

		case OGMACAM_EVENT_STILLIMAGE:		//*	snap (still) frame arrived, use Toupcam_PullStillImage to get this frame
		case OGMACAM_EVENT_WBGAIN:			//*	white balance changed, RGB Gain mode
		case OGMACAM_EVENT_TRIGGERFAIL:		//*	trigger failed
		case OGMACAM_EVENT_BLACK :			//*	black balance changed
		case OGMACAM_EVENT_FFC:				//*	flat field correction status changed
		case OGMACAM_EVENT_DFC:				//*	dark field correction status changed
		case OGMACAM_EVENT_ROI:				//*	roi changed
		case OGMACAM_EVENT_ERROR:			//*	generic error
		case OGMACAM_EVENT_DISCONNECTED:	//*	camera disconnected
		case OGMACAM_EVENT_NOFRAMETIMEOUT:	//*	no frame timeout error
//		case OGMACAM_EVENT_AFFEEDBACK:		//*	auto focus feedback information
//		case OGMACAM_EVENT_AFPOSITION:		//*	auto focus sensor board position
		case OGMACAM_EVENT_NOPACKETTIMEOUT:	//*	no packet timeout
		case OGMACAM_EVENT_FACTORY:			//*	restore factory settings

		default:
			CONSOLE_DEBUG_W_NUM(__FUNCTION__, nEvent);
			break;
	}
}




#endif	//	defined(_ENABLE_CAMERA_) && defined(_ENABLE_OGMA_)

