//**************************************************************************
//*	Name:			cameradriver_SONY.cpp
//*
//*	Author:			Mark Sproul (C) 2020
//*					msproul@skychariot.com
//*	Description:
//*
//*			This program controls the Sony A7R-IV camera
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
//*	Jul 16,	2020	<MLS> Created cameradriver_SONY.cpp
//*	Jul 16,	2020	<MLS> Started working on SONY camera drivers
//*	Jul 16,	2020	<MLS> SONY library is linking and we can get the version #
//*	Jul 17,	2020	<MLS> Added GetSonyErrorString()
//*	Jul 20,	2020	<MLS> Sony call back functions working
//*	Jul 20,	2020	<MLS> Able to retrieve SONY live view jpg image
//*	Jul 22,	2020	<MLS> Added ProcessProperty()
//*	Jul 27,	2020	<MLS> Started on interactive command line interface
//*	Jul 28,	2020	<MLS> Cleaned up warnings in Sony driver code
//*	Jul 28,	2020	<MLS> Added call to SCRSDK::SetSaveInfo(), fixed file save issue
//*	Jul 29,	2020	<MLS> Added LogFunctionCall() for debugging of Sony SDK
//*	Aug  3,	2020	<MLS> Added logging to disk to LogFunctionCall()
//*	Sep  2,	2020	<MLS> Masamichi Nagone from Sony said to turn off RAW
//*	Sep  8,	2020	<MLS> Confirmed, As per Masa
//*	Sep  8,	2020	<MLS> 	If /sys/module/usbcore/parameters/usbfs_memory_mb = 64, Raw mode works
//*	Feb 21,	2021	<MLS> Sony driver updated to use new Alpaca structures
//*****************************************************************************


#ifdef _ENABLE_SONY_
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<unistd.h>
#include	<sys/time.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<time.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

//*	SONY SDK file
//#include	<iomanip>
#include	"CameraRemote_SDK.h"
#include	"IDeviceCallback.h"

using namespace SCRSDK;

#ifdef _INCLUDE_SONY_MAIN_
	#include	"sonyCallback.h"
	#include	"sonyStandAlone.h"
	#undef		_ALPACA_PI_
	CameraDriverSONY	*gSonyCameraObj	=	NULL;
#else
	#ifndef _ALPACA_PI_
		#define		_ALPACA_PI_
	#endif
	#include	"alpacadriver.h"
	#include	"alpacadriver_helper.h"
	#include	"cameradriver.h"
	#include	"sonyCallback.h"
	#include	"cameradriver_SONY.h"


#endif // _INCLUDE_SONY_MAIN_

static void	GetSonyErrorString(CrError sonyErrCode, char *sonyErrorString);
static void	GetSonyPropertyString(uint32_t sonyPropCode, char *sonyPropertyString);
static void	GetSonyExposureMode(uint32_t sonyExposureMode, char *exposureModeString);
static void	GetSonyRecordingSetting(uint32_t sonyRecordingSetting, char *recordingSettingString);
static void	GetSonyWhiteBlanceString(CrError whateBalanceCode, char *whiteBalancceString);
static void	GetSonyPictureEffectsString(CrError pictureEffectsCode, char *pictureEffectsString);
static void	GetSonyDriveModeString(CrError driveModeCode, char *driveModeString);
static void	GetSonyDROmodeString(CrError droMode, char *droModeString);

//**************************************************************************************
typedef struct
{
	char	callingFunc[64];
	char	functionName[64];
	char	comment[64];
	CrError	returnedErrCode;
} TYPE_FUNCTION_LOG;
#define	kFunctionLogSize	100

TYPE_FUNCTION_LOG	gFunctionCallLog[kFunctionLogSize];
int					gFunctionLogIdx;
FILE				*gSonyLogFile	=	NULL;
void				LogFunctionCall(	const char		*callingFunc,
										const int		lineNumber,
										const char		*functionName,
										const CrError	returnedErrCode,
										const char		*comment = "");


//*****************************************************************************
void	CreateSONY_CameraObjects(void)
{
CrInt32u	versionNumber;
int			versionMaj;
int			versionMin;
int			versionPatch;
char		libraryVersionString[64];
bool		sonyRetCode;
CrError		sonyErrCode;
CrInt32u	cameraCount;
CrInt32u	camIdx;
char		sonyErrorString[128];
int			iii;


SCRSDK::ICrEnumCameraObjectInfo	*camera_list	=	NULL;
ICrCameraObjectInfo				*camera_info	=	NULL;

	CONSOLE_DEBUG(__FUNCTION__);

	//*	init the function call log
	for (iii=0; iii<kFunctionLogSize; iii++)
	{
		gFunctionCallLog[iii].returnedErrCode	=	0;
		strcpy(gFunctionCallLog[iii].functionName, "");
	}
	gFunctionLogIdx	=	0;


	versionNumber		=	SCRSDK::GetSDKVersion();

	versionMaj			=	(versionNumber >> 24) & 0x00ff;
	versionMin			=	(versionNumber >> 16) & 0x00ff;
	versionPatch		=	(versionNumber >> 8) & 0x00ff;
	sprintf(libraryVersionString, "%d.%d.%d", versionMaj, versionMin, versionPatch);

	LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::GetSDKVersion", 	0, libraryVersionString);


#ifndef _INCLUDE_SONY_MAIN_
	AddLibraryVersion("camera", "SONY", libraryVersionString);
#endif // _INCLUDE_SONY_MAIN_
	CONSOLE_DEBUG_W_STR("Sony SDK Version #\t=", libraryVersionString);

	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	CONSOLE_DEBUG("Calling SCRSDK::Init(0)");
	sonyRetCode	=	SCRSDK::Init(1);
	LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::Init", 0, (sonyRetCode ? "OK" : "Failed"));
	if (sonyRetCode)
	{
		CONSOLE_DEBUG("Sony Remote SDK successfully initialized.");
		CONSOLE_DEBUG("Calling SCRSDK::EnumCameraObjects()");
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		sonyErrCode	=	SCRSDK::EnumCameraObjects(&camera_list);
		LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::EnumCameraObjects", 	sonyErrCode);
		if (sonyErrCode)
		{
			GetSonyErrorString(sonyErrCode, sonyErrorString);
			CONSOLE_DEBUG_W_STR("Sony SDK err=\t=", sonyErrorString);
		}

//		CONSOLE_DEBUG_W_HEX("camera_list\t=", (void *)camera_list);

		if ((sonyErrCode == CrError_None) && (camera_list != NULL))
		{
			CONSOLE_DEBUG("SUCCESS!!!!");
			cameraCount	=	camera_list->GetCount(); // get number of cameras
			CONSOLE_DEBUG_W_NUM("Sony camera Count\t=", cameraCount);
			for (camIdx = 0; camIdx < cameraCount; camIdx++)
			{
				//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				//*	get the ptr to the camera info
				camera_info	=	(ICrCameraObjectInfo *)camera_list->GetCameraObjectInfo(camIdx);
				if (camera_info != NULL)
				{
					CONSOLE_DEBUG_W_STR("Conn type\t=",		camera_info->GetConnectionTypeName());
					CONSOLE_DEBUG_W_STR("Camera model\t=",	camera_info->GetModel());
					CONSOLE_DEBUG_W_STR("ConnType\t=",		camera_info->GetConnectionTypeName());
					CONSOLE_DEBUG_W_STR("Adapter\t=",		camera_info->GetAdaptorName());
					CONSOLE_DEBUG_W_STR("UUID\t\t=",		camera_info->GetGuid());

					//*	create a camera object
				#ifdef _INCLUDE_SONY_MAIN_
					gSonyCameraObj	=	new CameraDriverSONY(0, camera_info);
				#else
					new CameraDriverSONY(0, camera_info);
				#endif
				}
				else
				{
					CONSOLE_DEBUG("Failed to get camera info");
				}
			}
		}
		else
		{
			// no cameras found
			CONSOLE_DEBUG("no cameras found");
			CONSOLE_DEBUG("Calling SCRSDK::Release()");
			//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			sonyRetCode	=	SCRSDK::Release();
			LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::Release", 	0, (sonyRetCode ? "OK" : "Failed"));
		//	CONSOLE_ABORT(__FUNCTION__);
		}
	}
	else
	{
		CONSOLE_DEBUG("SCRSDK::Init(0) failed");
	}
}


//**************************************************************************************
CameraDriverSONY::CameraDriverSONY(const int deviceNum, ICrCameraObjectInfo *camera_info)
	:CameraDriver()
{
CrError		sonyErrCode;
char		sonyErrorString[128];
CrInt32u	versionNumber;
int			versionMaj;
int			versionMin;
int			versionPatch;
char		savePath[]		=	"./";
char		savePrefix[]	=	"SONY-";

	CONSOLE_DEBUG(__FUNCTION__);



	cCameraID			=	deviceNum;
	cSonyCamera_info	=	camera_info;
	cSonyDeviceHandle	=	(CrDeviceHandle)NULL;
	cIsUSB3Camera		=	true;
	cIsColorCam			=	true;

	cUpdateProperties	=	false;		//*	turn it on once the camera is connected

	strcpy(cDeviceManufAbrev,	"SONY");
	strcpy(cDeviceManufacturer,	"SONY");
	strcpy(cSONYidString,		"");

	versionNumber		=	GetSDKVersion();
	versionMaj			=	(versionNumber >> 24) & 0x00ff;
	versionMin			=	(versionNumber >> 16) & 0x00ff;
	versionPatch		=	(versionNumber >> 8) & 0x00ff;
	sprintf(cCommonProp.DriverVersion, "%d.%d.%d", versionMaj, versionMin, versionPatch);

	ReadSONYcameraInfo();

	cCallbackObjPtr	=	new SonyCallBack(this);

	if (cCallbackObjPtr != NULL)
	{
		CONSOLE_DEBUG("SonyCallBack created");

		CONSOLE_DEBUG("Calling Connect()");
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		sonyErrCode	=	SCRSDK::Connect(	camera_info,			//*	-in- ICrCameraObjectInfo* pCameraObjectInfo,
											cCallbackObjPtr,		//*	-in-  IDeviceCallback* callback,
											&cSonyDeviceHandle);	//*	-out- CrDeviceHandle* deviceHandle);

		LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::Connect", 	sonyErrCode);
		if (sonyErrCode == CrError_None)
		{
			CONSOLE_DEBUG("Connect() OK");
		//	cUpdateProperties	=	true;

			if (cSonyDeviceHandle != (CrDeviceHandle)NULL)
			{
				//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
				//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 30,
				//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 58,
				sonyErrCode	=	SCRSDK::SetSaveInfo(cSonyDeviceHandle, savePath, savePrefix, -1);
				LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::SetSaveInfo", 	sonyErrCode, savePrefix);
			}
		}
		else
		{
			GetSonyErrorString(sonyErrCode, sonyErrorString);
			CONSOLE_DEBUG_W_STR("Sony SDK err=\t=", sonyErrorString);
		}
	}
	else
	{
		CONSOLE_ABORT("Failed to create call back object");
	}

	strcpy(cCommonProp.Description, cDeviceManufacturer);
	strcat(cCommonProp.Description, " - Model:");
	strcat(cCommonProp.Description, cCommonProp.Name);

#ifdef _USE_OPENCV_
	sprintf(cOpenCV_ImgWindowName, "%s-%d", cCommonProp.Name, cCameraID);
#endif // _USE_OPENCV_

	CONSOLE_DEBUG("exit");

}

//**************************************************************************************
// Destructor
//**************************************************************************************
CameraDriverSONY::~CameraDriverSONY(void)
{
CrError		sonyRetCode;

	CONSOLE_DEBUG(__FUNCTION__);

	AlpacaDisConnect();
	sleep(1);
	if (cCallbackObjPtr != NULL)
	{
		delete cCallbackObjPtr;
		cCallbackObjPtr	=	NULL;
	}
	CONSOLE_DEBUG("Calling SCRSDK::Release()");
	//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	LogFunctionCall(__FUNCTION__, __LINE__,	"Calling SCRSDK::Release()", 	0, "");

	sonyRetCode	=	SCRSDK::Release();
	CONSOLE_DEBUG("REtruned from SCRSDK::Release()");
	LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::Release", 	0, (sonyRetCode ? "OK" : "Failed"));
	if (sonyRetCode == false)
	{
		CONSOLE_DEBUG("SCRSDK::Release() Failed!!!");
	}

	if (gSonyLogFile != NULL)
	{
		fclose(gSonyLogFile);
		gSonyLogFile	=	NULL;
	}
	CONSOLE_DEBUG("exit");
}

#ifdef  _ALPACA_PI_
//*****************************************************************************
void	CameraDriverSONY::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
CrError				sonyErrCode;
char				sonyErrorString[128];
char				lineBuffer[512];
CrInt32				numOfProperties;
CrDeviceProperty	*pProperties;
CrInt32				nnn;
CrInt32u			propCode;
CrDataType			currentValueType;
CrInt32u			currentValueSize;
CrInt64u			currentPropValue;
int					numValues;
char				propertyNameStr[256];
char				propertyDisplayStr[256];
bool				propProcessedFlag;
int					processedCnt;
int					notProcessedCnt;
int					iii;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cSonyDeviceHandle != (CrDeviceHandle)NULL)
	{
		pProperties	=	nullptr;
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		sonyErrCode	=	SCRSDK::GetDeviceProperties(cSonyDeviceHandle,	//*	*in*	CrDeviceHandle deviceHandle,
													&pProperties,		//*	*out*	CrDeviceProperty** properties,
													&numOfProperties);	//*	*out*	CrInt32* numOfProperties
		LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::GetDeviceProperties", 	sonyErrCode);
		if (sonyErrCode == CrError_None)
		{
			SocketWriteData(reqData->socket,	"<CENTER>\r\n");
			SocketWriteData(reqData->socket,	"<TABLE BORDER=1>\r\n");


//			CONSOLE_DEBUG_W_NUM("numOfProperties=\t=", numOfProperties);

			SocketWriteData(reqData->socket,	"<TR>");
			SocketWriteData(reqData->socket,	"<TH>#</TH>");
			SocketWriteData(reqData->socket,	"<TH>Prop<BR>code</TH>");
			SocketWriteData(reqData->socket,	"<TH>Property name<BR><I>CrDeviceProperty_...</I></TH>");
			SocketWriteData(reqData->socket,	"<TH>#<BR>values</TH>");
			SocketWriteData(reqData->socket,	"<TH>Value<BR>(hex)</TH>");
			SocketWriteData(reqData->socket,	"<TH>Meaning</TH>");
			SocketWriteData(reqData->socket,	"</TR>\r\n");

			processedCnt	=	0;
			notProcessedCnt	=	0;
			for (nnn=0; nnn < numOfProperties; nnn++)
			{
				SocketWriteData(reqData->socket,	"<TR>\r\n");
				sprintf(lineBuffer,	"<TD>%d</TD>\r\n", (nnn + 1));
				SocketWriteData(reqData->socket,	lineBuffer);


				propCode			=	pProperties[nnn].GetCode();
				currentValueType	=	pProperties[nnn].GetValueType();
				currentValueSize	=	pProperties[nnn].GetValueSize();
				currentPropValue	=	pProperties[nnn].GetCurrentValue();


				//*	figure out the number of values
				switch(currentValueType & 0x0f)
				{
					case CrDataType_UInt8:
						numValues	=	currentValueSize;
						break;

					case CrDataType_UInt16:
						numValues	=	currentValueSize / sizeof(uint16_t);
						break;

					case CrDataType_UInt32:
						numValues	=	currentValueSize / sizeof(uint32_t);
						break;

					case CrDataType_UInt64:
						numValues	=	currentValueSize / sizeof(uint64_t);
						break;

					case CrDataType_UInt128:
						numValues	=	currentValueSize / sizeof(uint64_t);
						break;

					default:
				//		CONSOLE_DEBUG("Unknown data type!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				//		CONSOLE_DEBUG_W_HEX("currentValueType\t=",	currentValueType);
				//		CONSOLE_DEBUG_W_HEX("propCode\t\t=",		propCode);
						numValues	=	-1;
						break;
				}

				GetSonyPropertyString(propCode, propertyNameStr);
				sprintf(lineBuffer,	"<TD>0x%04X</TD><TD>%s</TD><TD><CENTER>%d</TD><TD><CENTER>%lX</TD>\r\n",
														propCode,
														propertyNameStr,
														numValues,
														currentPropValue);
				SocketWriteData(reqData->socket,	lineBuffer);

				strcpy(propertyDisplayStr, "");
				propProcessedFlag	=	ProcessProperty(&pProperties[nnn], propertyDisplayStr);
				if (propProcessedFlag)
				{
					sprintf(lineBuffer,	"<TD>%s</TD>\r\n", propertyDisplayStr);
					SocketWriteData(reqData->socket,	lineBuffer);
					processedCnt++;
				}
				else
				{
					sprintf(lineBuffer,	"<TD>%s <FONT COLOR=ORANGE>Not processed</TD>\r\n", propertyDisplayStr);
					SocketWriteData(reqData->socket,	lineBuffer);
					notProcessedCnt++;
				}
				SocketWriteData(reqData->socket,	"</TR>\r\n");
			}
			//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			sonyErrCode	=	SCRSDK::ReleaseDeviceProperties(cSonyDeviceHandle, pProperties);
			LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::ReleaseDeviceProperties", 	sonyErrCode);
			if (sonyErrCode != CrError_None)
			{
				GetSonyErrorString(sonyErrCode, sonyErrorString);
				CONSOLE_DEBUG_W_STR("ReleaseDeviceProperties() returned SDK err=\t=", sonyErrorString);
			}

			//*	print out the totals of processed and not processed
			sprintf(lineBuffer,	"<TR><TD></TD><TD></TD><TD>Processed</TD><TD COLSPAN=4>%d</TD></TR>\r\n",		processedCnt);
			SocketWriteData(reqData->socket,	lineBuffer);
			if (notProcessedCnt > 0)
			{
				sprintf(lineBuffer,	"<TR><TD></TD><TD></TD><TD>Not Processed</TD><TD COLSPAN=4>%d</TD></TR>\r\n",	notProcessedCnt);
				SocketWriteData(reqData->socket,	lineBuffer);
			}

			//*	close the HTML table
			SocketWriteData(reqData->socket,	"</TABLE>\r\n");
			SocketWriteData(reqData->socket,	"</CENTER>\r\n");
		}
		else
		{
			GetSonyErrorString(sonyErrCode, sonyErrorString);
			CONSOLE_DEBUG_W_STR("Sony SDK err=\t=", sonyErrorString);
			SocketWriteData(reqData->socket,	"<CENTER>\r\n");
			SocketWriteData(reqData->socket,	"<H1>Failed to read properties, </H1>\r\n");
			SocketWriteData(reqData->socket,	"Error code =");
			SocketWriteData(reqData->socket,	sonyErrorString);
			SocketWriteData(reqData->socket,	"</H1>\r\n");
			SocketWriteData(reqData->socket,	"</CENTER>\r\n");
		}
	}
	else
	{
		CONSOLE_DEBUG("cSonyDeviceHandle is NULL");
		SocketWriteData(reqData->socket,	"<CENTER>\r\n");
		SocketWriteData(reqData->socket,	"<H1>Failed to read properties, device handle is null</H1>\r\n");
		SocketWriteData(reqData->socket,	"</CENTER>\r\n");
	}

	SocketWriteData(reqData->socket,	"<P><HR>\r\n");
	SocketWriteData(reqData->socket,	"<CENTER>\r\n");
	SocketWriteData(reqData->socket,	"<TABLE BORDER=1>\r\n");
	for (iii=0; iii<gFunctionLogIdx; iii++)
	{
	char	lineBuff[256];

		SocketWriteData(reqData->socket,	"<TR>");

		sprintf(lineBuff, "<TD>%s</TD>", gFunctionCallLog[iii].callingFunc);
		SocketWriteData(reqData->socket,	lineBuff);

		sprintf(lineBuff, "<TD>%s</TD>", gFunctionCallLog[iii].functionName);
		SocketWriteData(reqData->socket,	lineBuff);


		GetSonyErrorString(gFunctionCallLog[iii].returnedErrCode, sonyErrorString);
		sprintf(lineBuff, "<TD>%X</TD><TD>%s</TD>", gFunctionCallLog[iii].returnedErrCode, sonyErrorString);
		SocketWriteData(reqData->socket,	lineBuff);

		sprintf(lineBuff, "<TD>%s</TD>", gFunctionCallLog[iii].comment);
		SocketWriteData(reqData->socket,	lineBuff);

		SocketWriteData(reqData->socket,	"</TR>\r\n");

	}

	SocketWriteData(reqData->socket,	"</TABLE>\r\n");
	SocketWriteData(reqData->socket,	"</CENTER>\r\n");

	//===============================================================

	CameraDriver::OutputHTML_Part2(reqData);
}
#endif // _ALPACA_PI_

//*****************************************************************************
bool	CameraDriverSONY::AlpacaConnect(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	return(true);
}

//*****************************************************************************
bool	CameraDriverSONY::AlpacaDisConnect(void)
{
CrError		sonyErrCode;
char		sonyErrorString[128];

	CONSOLE_DEBUG(__FUNCTION__);
	if (cSonyDeviceHandle != (CrDeviceHandle)NULL)
	{
		CONSOLE_DEBUG("Calling SCRSDK::Disconnect()");
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		sonyErrCode	=	SCRSDK::Disconnect(cSonyDeviceHandle);
		LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::Disconnect", 	sonyErrCode);
		if (sonyErrCode)
		{
			GetSonyErrorString(sonyErrCode, sonyErrorString);
			CONSOLE_DEBUG_W_STR("SCRSDK::Disconnect() returned SDK err=\t=", sonyErrorString);
		}
		cSonyDeviceHandle		=	(CrDeviceHandle)NULL;
		cCommonProp.Connected	=	false;
	}
	return(true);
}


//*****************************************************************************
void	CameraDriverSONY::SaveData(unsigned char *pdata, int imgSize, const char *fileName)
{
FILE	*filePointer;
char	myFileName[128];


	CONSOLE_DEBUG(__FUNCTION__);

	if (fileName != NULL)
	{
		strcpy(myFileName, fileName);
	}
	else
	{
		GenerateFileNameRoot();
		CONSOLE_DEBUG_W_STR("cFileNameRoot\t=", cFileNameRoot);
		strcpy(myFileName, cFileNameRoot);
		strcat(myFileName, "-small.jpg");
		CONSOLE_DEBUG_W_STR("myFileName\t\t=", myFileName);
	}

	filePointer	=	fopen(myFileName, "w");
	if (filePointer != NULL)
	{
		fwrite(pdata, imgSize, 1, filePointer);
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create data file");
	}
}

//*****************************************************************************
void	CameraDriverSONY::RunStateMachine_Device(void)
{
	if (cUpdateProperties)
	{
		CONSOLE_DEBUG_W_STR("Calling ReadProperties() from", __FUNCTION__);
		ReadProperties();
		cUpdateProperties	=	false;
	}
}

//*****************************************************************************
//*	returns true if the property was processed, i.e. found in the case statement
//*****************************************************************************
bool	CameraDriverSONY::ProcessProperty(	CrDeviceProperty	*cameraProperty,
											char				*propertyDisplayStr)
{
CrInt32u			propCode;
CrInt64u			currentPropValue;
bool				foundFlag;
double				lowWord_Dbl;
double				hiWord_Dbl;
double				shutterSpeed_DBL;


	propCode			=	cameraProperty->GetCode();
	currentPropValue	=	cameraProperty->GetCurrentValue();

	foundFlag			=	true;

#ifdef _INCLUDE_SONY_MAIN_
int					numValues;
CrDataType			currentValueType;
CrInt32u			currentValueSize;

	currentValueSize	=	cameraProperty->GetValueSize();
	currentValueType	=	cameraProperty->GetValueType();
	//*	figure out the number of values
	switch(currentValueType & 0x0f)
	{
		case CrDataType_UInt8:
			numValues	=	currentValueSize;
			break;

		case CrDataType_UInt16:
			numValues	=	currentValueSize / sizeof(uint16_t);
			break;

		case CrDataType_UInt32:
			numValues	=	currentValueSize / sizeof(uint32_t);
			break;

		case CrDataType_UInt64:
			numValues	=	currentValueSize / sizeof(uint64_t);
			break;

		case CrDataType_UInt128:
			numValues	=	currentValueSize / sizeof(uint64_t);
			break;

		default:
		//	CONSOLE_DEBUG("Unknown data type!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		//	CONSOLE_DEBUG_W_HEX("currentValueType\t=",	currentValueType);
		//	CONSOLE_DEBUG_W_HEX("propCode\t\t=",		propCode);
			break;

	}
//	CONSOLE_DEBUG_W_NUM("numValues\t\t=",		numValues);
#endif // _INCLUDE_SONY_MAIN_

	switch (propCode)
	{
		case CrDeviceProperty_S1:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 80
		case CrDeviceProperty_AEL:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 80
		case CrDeviceProperty_FEL:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 80
		case CrDeviceProperty_AWBL:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 80

			currentPropValue		=	cameraProperty->GetCurrentValue();
			switch(currentPropValue)
			{
				case CrLockIndicator_Unlocked:	strcpy(propertyDisplayStr,	"Unlocked");	break;
				case CrLockIndicator_Locked:	strcpy(propertyDisplayStr,	"Locked");		break;
				default:						strcpy(propertyDisplayStr,	"UNKNOWN!!!!");	break;
			}
			break;

		case CrDeviceProperty_ImageSize:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 91
			currentPropValue		=	cameraProperty->GetCurrentValue();
			switch(currentPropValue)
			{
				case CrImageSize_L:		strcpy(propertyDisplayStr,	"L");			break;
				case CrImageSize_M:		strcpy(propertyDisplayStr,	"M");			break;
				case CrImageSize_S:		strcpy(propertyDisplayStr,	"S");			break;
				case CrImageSize_VGA:	strcpy(propertyDisplayStr,	"VGA");			break;
				default:				strcpy(propertyDisplayStr, "UNKNOWN!!!!");	break;
			}
			break;

		case CrDeviceProperty_AspectRatio:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 91
			currentPropValue		=	cameraProperty->GetCurrentValue();
			switch(currentPropValue)
			{
				case CrAspectRatio_3_2:
					strcpy(propertyDisplayStr, "CrAspectRatio_3_2");

					cCameraProp.CameraXsize	=	9504;
					cCameraProp.CameraYsize	=	6336;
					break;

				case CrAspectRatio_16_9:
					strcpy(propertyDisplayStr, "CrAspectRatio_16_9");
					cCameraProp.CameraXsize	=	9504;
					cCameraProp.CameraYsize	=	5344;
					break;

				case CrAspectRatio_4_3:
					strcpy(propertyDisplayStr, "CrAspectRatio_4_3");
					cCameraProp.CameraXsize	=	8448;
					cCameraProp.CameraYsize	=	6336;
					break;

				case CrAspectRatio_1_1:
					strcpy(propertyDisplayStr, "CrAspectRatio_1_1");
					cCameraProp.CameraXsize	=	6336;
					cCameraProp.CameraYsize	=	6336;
					break;

				default:
					strcpy(propertyDisplayStr, "UNKNOWN!!!!");
					break;
			}
			break;

		case CrDeviceProperty_PictureEffect:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 91
			GetSonyPictureEffectsString(currentPropValue, propertyDisplayStr);
			break;

		case CrDeviceProperty_FocusArea:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 83
			switch(currentPropValue)
			{
				case CrFocusArea_Unknown:				strcpy(propertyDisplayStr,	"Unknown");	break;
				case CrFocusArea_Wide:					strcpy(propertyDisplayStr,	"Wide");	break;
				case CrFocusArea_Zone:					strcpy(propertyDisplayStr,	"Zone");	break;
				case CrFocusArea_Center:				strcpy(propertyDisplayStr,	"Center");	break;
				case CrFocusArea_Flexible_Spot_S:		strcpy(propertyDisplayStr,	"Flexible_Spot_S");	break;
				case CrFocusArea_Flexible_Spot_M:		strcpy(propertyDisplayStr,	"Flexible_Spot_M");	break;
				case CrFocusArea_Flexible_Spot_L:		strcpy(propertyDisplayStr,	"Flexible_Spot_L");	break;
				case CrFocusArea_Expand_Flexible_Spot:	strcpy(propertyDisplayStr,	"Expand_Flexible_Spot");	break;
				case CrFocusArea_Flexible_Spot:			strcpy(propertyDisplayStr,	"Flexible_Spot");	break;
				case CrFocusArea_Tracking_Wide:			strcpy(propertyDisplayStr,	"Tracking_Wide");	break;
				case CrFocusArea_Tracking_Zone:			strcpy(propertyDisplayStr,	"Tracking_Zone");	break;
				case CrFocusArea_Tracking_Center:		strcpy(propertyDisplayStr,	"Tracking_Center");	break;
				case CrFocusArea_Tracking_Flexible_Spot_S:		strcpy(propertyDisplayStr,	"Tracking_Flexible_Spot_S");	break;
				case CrFocusArea_Tracking_Flexible_Spot_M:		strcpy(propertyDisplayStr,	"Tracking_Flexible_Spot_M");	break;
				case CrFocusArea_Tracking_Flexible_Spot_L:		strcpy(propertyDisplayStr,	"Tracking_Flexible_Spot_L");	break;
				case CrFocusArea_Tracking_Expand_Flexible_Spot:	strcpy(propertyDisplayStr,	"Tracking_Expand_Flexible_Spot");	break;
				case CrFocusArea_Tracking_Flexible_Spot:		strcpy(propertyDisplayStr,	"Tracking_Flexible_Spot");	break;
				default:										strcpy(propertyDisplayStr,	"UNKNOWN!!!!");	break;
			}
			break;

		case CrDeviceProperty_ShutterSpeed:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 82
			// ShutterSpeed
			// type: CrDataType_UInt32
			// value = low-order word / high-order word
			lowWord_Dbl	=	(currentPropValue & 0x0ffff);
			hiWord_Dbl	=	(currentPropValue >> 16);
			if (hiWord_Dbl > 0.0)	//*	avoid divide by zero
			{
				shutterSpeed_DBL	=	lowWord_Dbl / hiWord_Dbl;
			}
			else
			{
				shutterSpeed_DBL	=	0.0;
			}
			sprintf(propertyDisplayStr,	"%3.5f", shutterSpeed_DBL);
			break;

		case CrDeviceProperty_FNumber:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 81
			// FNumber
			// type: CrDataType_UInt16
			// value = F number * 100
			switch(currentPropValue)
			{
				case CrFnumber_Unknown:		strcpy(propertyDisplayStr, "--");	break;
				case CrFnumber_Nothing:		strcpy(propertyDisplayStr, "");		break;
				default:
					sprintf(propertyDisplayStr,	"%1.1f", (1.0 * currentPropValue / 100));
					break;
			}
			break;

		case CrDeviceProperty_ExposureBiasCompensation:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 81
			//*	The value is obtained by multiplying a real Exposure Bias Compensation value by 1000.
			double	biasComp;
			biasComp	=	currentPropValue / 1000.0;
			sprintf(propertyDisplayStr,	"%1.1fEv", biasComp);
			break;

		case CrDeviceProperty_FlashCompensation:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 81
			//*	The value is obtained by multiplying a real Flash Compensation value by 1000.
			double	flashComp;
			flashComp	=	currentPropValue / 1000.0;
			sprintf(propertyDisplayStr,	"%1.1fEv", flashComp);
			break;

		case CrDeviceProperty_RecordingState:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 104
			switch(currentPropValue)
			{
				case CrMovie_Recording_State_Not_Recording:
							strcpy(propertyDisplayStr, "Not_Recording");
							break;
				case CrMovie_Recording_State_Recording:
							strcpy(propertyDisplayStr, "Recording");
							break;
				case CrMovie_Recording_State_Recording_Failed:
							strcpy(propertyDisplayStr, "Recording_Failed");
							break;
				default:	strcpy(propertyDisplayStr, "UNKNOWN!!!!");
							break;
			}
			break;

		case CrDeviceProperty_LiveViewStatus:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 104
			switch(currentPropValue)
			{
				case CrLiveView_NotSupport:	strcpy(propertyDisplayStr, "CrLiveView_NotSupport");	break;
				case CrLiveView_Disable:	strcpy(propertyDisplayStr, "CrLiveView_Disable");		break;
				case CrLiveView_Enable:		strcpy(propertyDisplayStr, "CrLiveView_Enable");		break;
				default:					strcpy(propertyDisplayStr, "UNKNOWN!!!!");				break;
			}
			break;

		case CrDeviceProperty_FocusIndication:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 104
			switch(currentPropValue)
			{
				case CrFocusIndicator_Unlocked:				strcpy(propertyDisplayStr, "Unlocked");				break;
				case CrFocusIndicator_Focused_AF_S:			strcpy(propertyDisplayStr, "Focused_AF_S");			break;
				case CrFocusIndicator_NotFocused_AF_S:		strcpy(propertyDisplayStr, "NotFocused_AF_S");		break;
				case CrFocusIndicator_Focused_AF_C:			strcpy(propertyDisplayStr, "Focused_AF_C");			break;
				case CrFocusIndicator_NotFocused_AF_C:		strcpy(propertyDisplayStr, "NotFocused_AF_C");		break;
				case CrFocusIndicator_TrackingSubject_AF_C:	strcpy(propertyDisplayStr, "TrackingSubject_AF_C");	break;
				default:					strcpy(propertyDisplayStr, "UNKNOWN!!!!");							break;
			}
			break;

		case CrDeviceProperty_LiveView_Image_Quality:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 101
			switch(currentPropValue)
			{
				case CrPropertyLiveViewImageQuality_Low:	strcpy(propertyDisplayStr, "Low");			break;
				case CrPropertyLiveViewImageQuality_High:	strcpy(propertyDisplayStr, "High");			break;
				default:									strcpy(propertyDisplayStr, "UNKNOWN!!!!");	break;
			}
			break;

		case CrDeviceProperty_IsoSensitivity:
		int isoMode;
		int	isoValue;
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 82
			// value: bit 28-31 ISO mode (CrISOMode), bit 0-27 ISO value
			if (currentPropValue == CrISO_AUTO)
			{
				strcpy(propertyDisplayStr, "CrISO_AUTO");
			}
			else
			{
				isoMode		=	currentPropValue >> 28;
				isoValue	=	currentPropValue & 0x03ffffff;
				sprintf(propertyDisplayStr,	"Mode=%d, ISO=%d", isoMode, isoValue);
			}
			break;

		case CrDeviceProperty_ExposureProgramMode:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 83
			GetSonyExposureMode(currentPropValue, propertyDisplayStr);
			break;

		case CrDeviceProperty_FocusMode:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 86
			switch(currentPropValue)
			{
				case CrFocus_MF:	strcpy(propertyDisplayStr, "CrFocus_MF");	break;
				case CrFocus_AF_S:	strcpy(propertyDisplayStr, "CrFocus_AF_S");	break;
				case CrFocus_AF_C:	strcpy(propertyDisplayStr, "CrFocus_AF_C");	break;
				case CrFocus_AF_A:	strcpy(propertyDisplayStr, "CrFocus_AF_A");	break;
				case CrFocus_AF_D:	strcpy(propertyDisplayStr, "CrFocus_AF_D");	break;
				case CrFocus_DMF:	strcpy(propertyDisplayStr, "CrFocus_DMF");	break;
				case CrFocus_PF:	strcpy(propertyDisplayStr, "CrFocus_PF");	break;
			}
			break;

		case CrDeviceProperty_Focus_Magnifier_Setting:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 95
			//*		Focus Magnifier Ratio | Focus Magnifier Position (x,y)
			//*		Regarding Focus Magnifier Position (x,y) [Lower 4bytes],
			//*		the x coordinate is set in the upper two bytes and the y
			//*		coordinate is set in the lower two bytes.
			//*		If focus magnifier position (x) is 150 and (y) is 100, set
			//*		0x00960064. 0x0096 = 0d150, 0x0064 = 0d100.
			//*		The range of X is 0~639 (0x027F), and the range of Y is
			//*		0~479 (0x01DF).
			//*		Frame size is acquired by CrMagPosInfo. CrMagPosInfo is in LiveViewProperty.
			//*		Since this position specifies the center of the frame, the
			//*		position range is more inside by half the frame size than
			//*		CrMagPosInfo.xDenominator/yDenominator.
			//*		In Enum values, Focus Magnifier Position (x,y) sets
			//*		0xFFFFFFFF as invalid value.
			//*		Ex.) If the camera supports OFF, x1.0, x4.0 and x8.0 as focus magnifier ratio,
			//*	Enum values is the following.
			//*		Enum value[0] = 0x00000000FFFFFFFF (means OFF)
			//*		Enum value[1] = 0x0000000AFFFFFFFF (means x1.0)
			//*		Enum value[2] = 0x00000028FFFFFFFF (means x4.0)
			//*		Enum value[3] = 0x00000050FFFFFFFF (means x8.0)
			int	magnifierPos_X;
			int	magnifierPos_Y;

			magnifierPos_X	=	(currentPropValue >> 16) & 0x00ffff;
			magnifierPos_Y	=	(currentPropValue) & 0x00ffff;
			sprintf(propertyDisplayStr,	"position= %d x %d (raw=%lX)", magnifierPos_X, magnifierPos_Y, currentPropValue);
			break;


		case CrDeviceProperty_NearFar:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 96
			switch(currentPropValue)
			{
				case CrNearFar_Disable:	strcpy(propertyDisplayStr, "Disable");	break;
				case CrNearFar_Enable:	strcpy(propertyDisplayStr, "Enable");	break;
				default:				strcpy(propertyDisplayStr, "UNKNOWN!!!!");	break;
			}
			break;

		case CrDeviceProperty_AF_Area_Position:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 97
			//*	The x coordinate is set in the upper two bytes and
			//*	the y coordinate is set in the lower two bytes
			//*	The range of X is 0~639 (0x027F),
			//*	the range of Y is 0~479 (0x01DF).
			int	autoFocus_XPos;
			int	autoFocus_YPos;
			autoFocus_XPos	=	(currentPropValue >> 16) & 0x00ffff;
			autoFocus_YPos	=	currentPropValue & 0x00ffff;
			sprintf(propertyDisplayStr,	"%d x %d (%lX) ", autoFocus_XPos, autoFocus_YPos, currentPropValue);
			break;

		case CrDeviceProperty_FileType:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 85
			switch(currentPropValue)
			{
				case CrFileType_Jpeg:		strcpy(propertyDisplayStr, "CrFileType_Jpeg");		break;
				case CrFileType_Raw:		strcpy(propertyDisplayStr, "CrFileType_Raw");		break;
				case CrFileType_RawJpeg:	strcpy(propertyDisplayStr, "CrFileType_RawJpeg");	break;
				case CrFileType_RawHeif:	strcpy(propertyDisplayStr, "CrFileType_RawHeif");	break;
				case CrFileType_Heif:		strcpy(propertyDisplayStr, "CrFileType_Heif");		break;
				default:					strcpy(propertyDisplayStr, "UNKNOWN!!!!");			break;
			}
			break;

		case CrDeviceProperty_StillImageStoreDestination:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 94
			switch(currentPropValue)
			{
				case CrStillImageStoreDestination_HostPC:
					strcpy(propertyDisplayStr, "CrStillImageStoreDestination_HostPC");
					break;

				case CrStillImageStoreDestination_MemoryCard:
					strcpy(propertyDisplayStr, "CrStillImageStoreDestination_MemoryCard");
					break;

				case CrStillImageStoreDestination_HostPCAndMemoryCard:
					strcpy(propertyDisplayStr, "CrStillImageStoreDestination_HostPCAndMemoryCard");
					break;

				default:	strcpy(propertyDisplayStr, "UNKNOWN!!!!");
					break;

			}
			break;

		case CrDeviceProperty_PriorityKeySettings:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 94
			switch(currentPropValue)
			{
				case CrPriorityKey_CameraPosition:	strcpy(propertyDisplayStr, "CameraPosition");	break;
				case CrPriorityKey_PCRemote:		strcpy(propertyDisplayStr, "PCRemote");			break;
				default:							strcpy(propertyDisplayStr, "UNKNOWN!!!!");		break;
			}
			break;

		case CrDeviceProperty_Interval_Rec_Mode:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 100
			switch(currentPropValue)
			{
				case CrIntervalRecMode_OFF:	strcpy(propertyDisplayStr, "OFF");	break;
				case CrIntervalRecMode_ON:	strcpy(propertyDisplayStr, "ON");	break;
				default:					strcpy(propertyDisplayStr, "UNKNOWN!!!!");	break;
			}
			break;

		case CrDeviceProperty_Interval_Rec_Status:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 107
			switch(currentPropValue)
			{
				case CrIntervalRecStatus_WaitingStart:		strcpy(propertyDisplayStr, "WaitingStart");	break;
				case CrIntervalRecStatus_IntervalShooting:	strcpy(propertyDisplayStr, "IntervalShooting");	break;
				default:									strcpy(propertyDisplayStr, "UNKNOWN!!!!");	break;
			}
			break;


		case CrDeviceProperty_Still_Image_Trans_Size:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 100
			switch(currentPropValue)
			{
				case CrPropertyStillImageTransSize_Original:
					strcpy(propertyDisplayStr, "Original");
					break;
				case CrPropertyStillImageTransSize_SmallSizeJPEG:
					strcpy(propertyDisplayStr, "SmallSizeJPEG");
					break;

				default:	strcpy(propertyDisplayStr, "UNKNOWN!!!!");
					break;

			}
			break;

		case CrDeviceProperty_RAW_J_PC_Save_Image:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 101
			//*	property value -> enum value describing saved image format
			switch(currentPropValue)
			{
				case CrPropertyRAWJPCSaveImage_RAWAndJPEG:
					strcpy(propertyDisplayStr, "RAWAndJPEG");
					break;

				case CrPropertyRAWJPCSaveImage_JPEGOnly:
					strcpy(propertyDisplayStr, "JPEGOnly");
					break;

				case CrPropertyRAWJPCSaveImage_RAWOnly:
					strcpy(propertyDisplayStr, "RAWOnly");
					break;

				default:	strcpy(propertyDisplayStr, "UNKNOWN!!!!");
					break;

			}
			break;

		case CrDeviceProperty_MediaSLOT1_Status:
		case CrDeviceProperty_MediaSLOT2_Status:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 105
			//*	property value -> enum value describing slot status
			switch(currentPropValue)
			{
				case CrSlotStatus_OK:
					strcpy(propertyDisplayStr, "CrSlotStatus_OK");
					break;

				case CrSlotStatus_NoCard:
					strcpy(propertyDisplayStr, "CrSlotStatus_NoCard");
					break;

				case CrSlotStatus_CardError:
					strcpy(propertyDisplayStr, "CrSlotStatus_CardError");
					break;

				case CrSlotStatus_RecognizingOrLockedError:
					strcpy(propertyDisplayStr, "CrSlotStatus_RecognizingOrLockedError");
					break;

				default:	strcpy(propertyDisplayStr, "UNKNOWN!!!!");
					break;

			}
			break;

		case CrDeviceProperty_MediaSLOT1_RemainingNumber:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 105
		case CrDeviceProperty_MediaSLOT2_RemainingNumber:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 106
			sprintf(propertyDisplayStr,	"space for %ld more images", currentPropValue);
			break;


		case CrDeviceProperty_MediaSLOT1_RemainingTime:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 105
		case CrDeviceProperty_MediaSLOT2_RemainingTime:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 106
			sprintf(propertyDisplayStr,	"space for %ld more seconds", currentPropValue);
			break;

		case CrDeviceProperty_SnapshotInfo:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 103
			if (currentPropValue == 0)
			{
				strcpy(propertyDisplayStr,	"transferable file doesn't exist");
			}
			else if (currentPropValue > 0x8001)
			{
				strcpy(propertyDisplayStr,	"can get the Shot files");
			}
			else
			{
				sprintf(propertyDisplayStr,	"%ld", currentPropValue);
			}
			break;


		case CrDeviceProperty_BatteryRemain:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 103
			sprintf(propertyDisplayStr,	"%ld", currentPropValue);
			break;

		case CrDeviceProperty_BatteryLevel:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 103
			//*	property value -> enum value describing battery level
			switch(currentPropValue)
			{
				case CrBatteryLevel_PreEndBattery:		strcpy(propertyDisplayStr, "PreEndBattery");	break;
				case CrBatteryLevel_1_4:				strcpy(propertyDisplayStr, "1_4");				break;
				case CrBatteryLevel_2_4:				strcpy(propertyDisplayStr, "2_4");				break;
				case CrBatteryLevel_3_4:				strcpy(propertyDisplayStr, "3_4");				break;
				case CrBatteryLevel_4_4:				strcpy(propertyDisplayStr, "4_4");				break;
				case CrBatteryLevel_1_3:				strcpy(propertyDisplayStr, "1_3");				break;
				case CrBatteryLevel_2_3:				strcpy(propertyDisplayStr, "2_3");				break;
				case CrBatteryLevel_3_3:				strcpy(propertyDisplayStr, "3_3");				break;
				case CrBatteryLevel_USBPowerSupply:		strcpy(propertyDisplayStr, "USBPowerSupply");	break;
				case CrBatteryLevel_PreEnd_PowerSupply:	strcpy(propertyDisplayStr, "PreEnd_PowerSupply");	break;
				case CrBatteryLevel_1_4_PowerSupply:	strcpy(propertyDisplayStr, "1_4_PowerSupply");	break;
				case CrBatteryLevel_2_4_PowerSupply:	strcpy(propertyDisplayStr, "2_4_PowerSupply");	break;
				case CrBatteryLevel_3_4_PowerSupply:	strcpy(propertyDisplayStr, "3_4_PowerSupply");	break;
				case CrBatteryLevel_4_4_PowerSupply:	strcpy(propertyDisplayStr, "4_4_PowerSupply");	break;
				case CrBatteryLevel_Fake:				strcpy(propertyDisplayStr, "Fake");				break;
			}
			break;

		case CrDeviceProperty_MeteringMode:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 87
			//*	property value -> enum value describing metering mode
			switch(currentPropValue)
			{
				case CrMetering_Average:				strcpy(propertyDisplayStr, "Average");				break;
				case CrMetering_CenterWeightedAverage:	strcpy(propertyDisplayStr, "CenterWeightedAverage");break;
				case CrMetering_MultiSpot:				strcpy(propertyDisplayStr, "MultiSpot");			break;
				case CrMetering_CenterSpot:				strcpy(propertyDisplayStr, "CenterSpot");			break;
				case CrMetering_Multi:					strcpy(propertyDisplayStr, "Multi");				break;
				case CrMetering_CenterWeighted:			strcpy(propertyDisplayStr, "CenterWeighted");		break;
				case CrMetering_EntireScreenAverage:	strcpy(propertyDisplayStr, "EntireScreenAverage");	break;
				case CrMetering_Spot_Standard:			strcpy(propertyDisplayStr, "Spot_Standard");		break;
				case CrMetering_Spot_Large:				strcpy(propertyDisplayStr, "Spot_Large");			break;
				case CrMetering_HighLightWeighted:		strcpy(propertyDisplayStr, "HighLightWeighted");	break;
				default:								strcpy(propertyDisplayStr, "UNKNOWN");				break;
			}
			break;

		case CrDeviceProperty_FlashMode:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 87
			//*	property value -> enum value describing flash mode
			switch(currentPropValue)
			{
				case CrFlash_Auto:			strcpy(propertyDisplayStr, "Auto");			break;
				case CrFlash_Off:			strcpy(propertyDisplayStr, "Off");			break;
				case CrFlash_Fill:			strcpy(propertyDisplayStr, "Fill");			break;
				case CrFlash_ExternalSync:	strcpy(propertyDisplayStr, "ExternalSync");	break;
				case CrFlash_SlowSync:		strcpy(propertyDisplayStr, "SlowSync");		break;
				case CrFlash_RearSync:		strcpy(propertyDisplayStr, "RearSync");		break;
				default:					strcpy(propertyDisplayStr, "UNKNOWN");		break;
			}
			break;

		case CrDeviceProperty_WirelessFlash:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 87
			//*	property value -> enum value describing flash mode
			switch(currentPropValue)
			{
				case CrWirelessFlash_Off:	strcpy(propertyDisplayStr, "Off");			break;
				case CrWirelessFlash_On:	strcpy(propertyDisplayStr, "On");			break;
				default:					strcpy(propertyDisplayStr, "UNKNOWN");		break;
			}
			break;

		case CrDeviceProperty_RedEyeReduction:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 88
			//*	property value -> enum value describing flash mode
			switch(currentPropValue)
			{
				case CrRedEye_Off:	strcpy(propertyDisplayStr, "Off");			break;
				case CrRedEye_On:	strcpy(propertyDisplayStr, "On");			break;
				default:			strcpy(propertyDisplayStr, "UNKNOWN");		break;
			}
			break;

		case CrDeviceProperty_DriveMode:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 88
			GetSonyDriveModeString(currentPropValue, propertyDisplayStr);
			break;

		case CrDeviceProperty_DRO:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 90
			GetSonyDROmodeString(currentPropValue, propertyDisplayStr);
			break;

		case CrDeviceProperty_Zoom_Bar_Information:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 109
			//	Value Explanation
			//	31-24bit	Total box number				0 min	->	0xFF max		1 step
			//	23-16bit	Current box number				0 min	->	0xFF max		1 step
			//	15- 0bit	Zoom position in Current Box	00 min	->	0x64 max	0x01 step
			sprintf(propertyDisplayStr,	"Total Box#=%ld, Curr Box#=%ld, Zoom=%ld",	(currentPropValue >> 24),
																				((currentPropValue >> 16) & 0x00ff),
																				(currentPropValue & 0x00ffff));
			break;

		case CrDeviceProperty_Zoom_Operation_Status:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 109
			switch(currentPropValue)
			{
				case CrZoomOperationEnableStatus_Disable:	strcpy(propertyDisplayStr, "Disable");	break;
				case CrZoomOperationEnableStatus_Enable:	strcpy(propertyDisplayStr, "Enable");	break;
				default:									strcpy(propertyDisplayStr, "UNKNOWN");	break;
			}

			break;

		case CrDeviceProperty_Zoom_Type_Status:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 110
			switch(currentPropValue)
			{
				case CrZoomTypeStatus_OpticalZoom:		strcpy(propertyDisplayStr, "OpticalZoom");		break;
				case CrZoomTypeStatus_SmartZoom:		strcpy(propertyDisplayStr, "SmartZoom");		break;
				case CrZoomTypeStatus_ClearImageZoom:	strcpy(propertyDisplayStr, "ClearImageZoom");	break;
				case CrZoomTypeStatus_DigitalZoom:		strcpy(propertyDisplayStr, "DigitalZoom");		break;
				default:								strcpy(propertyDisplayStr, "UNKNOWN");			break;
			}
			break;

		case CrDeviceProperty_Zoom_Scale:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 98
			sprintf(propertyDisplayStr,	"%ld", currentPropValue);
			break;

		case CrDeviceProperty_Zoom_Setting:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 98
			switch(currentPropValue)
			{
				case CrZoomSetting_OpticalZoomOnly:		strcpy(propertyDisplayStr, "OpticalZoomOnly");		break;
				case CrZoomSetting_SmartZoomOnly:		strcpy(propertyDisplayStr, "SmartZoomOnly");		break;
				case CrZoomSetting_On_ClearImageZoom:	strcpy(propertyDisplayStr, "On_ClearImageZoom");	break;
				case CrZoomSetting_On_DigitalZoom:		strcpy(propertyDisplayStr, "On_DigitalZoom");		break;
				default:								strcpy(propertyDisplayStr, "UNKNOWN");				break;
			}
			break;

		case CrDeviceProperty_Zoom_Operation:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 98
			switch(currentPropValue)
			{
				case CrZoomOperation_Wide:	strcpy(propertyDisplayStr, "Wide");		break;
				case CrZoomOperation_Stop:	strcpy(propertyDisplayStr, "Stop");		break;
				case CrZoomOperation_Tele:	strcpy(propertyDisplayStr, "Tele");		break;
				default:					strcpy(propertyDisplayStr, "UNKNOWN");	break;
			}
			break;

		case CrDeviceProperty_Movie_File_Format:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 99
			//*	property value -> enum value describing movie recording format
			switch(currentPropValue)
			{
				case CrFileFormatMovie_AVCHD:		strcpy(propertyDisplayStr, "AVCHD");		break;
				case CrFileFormatMovie_MP4:			strcpy(propertyDisplayStr, "MP4");			break;
				case CrFileFormatMovie_XAVC_S_4K:	strcpy(propertyDisplayStr, "XAVC_S_4K");	break;
				case CrFileFormatMovie_XAVC_S_HD:	strcpy(propertyDisplayStr, "XAVC_S_HD");	break;
			}
			break;

		case CrDeviceProperty_Movie_Recording_Setting:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 99
			//*	property value -> enum value describing movie recording setting
			GetSonyRecordingSetting(currentPropValue, propertyDisplayStr);
			break;

		case CrDeviceProperty_JpegQuality:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 85
			//*	property value -> enum value describing movie recording format
			switch(currentPropValue)
			{
				case CrJpegQuality_Unknown:		strcpy(propertyDisplayStr,	"Unknown");		break;
				case CrJpegQuality_Standard:	strcpy(propertyDisplayStr,	"Standard");	break;
				case CrJpegQuality_Fine:		strcpy(propertyDisplayStr,	"Fine");		break;
				case CrJpegQuality_ExFine:		strcpy(propertyDisplayStr,	"ExFine");		break;
			}
			break;

		case CrDeviceProperty_Colortemp:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 92
			sprintf(propertyDisplayStr,	"%ldK", currentPropValue);
			break;

		case CrDeviceProperty_ColorTuningAB:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 93
		case CrDeviceProperty_ColorTuningGM:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 93
			sprintf(propertyDisplayStr,	"0x%lX", currentPropValue);
			break;

		case CrDeviceProperty_LiveViewDisplayEffect:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 93
			switch(currentPropValue)
			{
				case CrLiveViewDisplayEffect_Unknown:	strcpy(propertyDisplayStr,	"Unknown");	break;
				case CrLiveViewDisplayEffect_ON:		strcpy(propertyDisplayStr,	"ON");		break;
				case CrLiveViewDisplayEffect_OFF:		strcpy(propertyDisplayStr,	"OFF");		break;
				default:								strcpy(propertyDisplayStr,	"UNKOWN");	break;
			}
			break;


		case CrDeviceProperty_WhiteBalance:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 86
			//*	property value -> enum value describing white balance
			GetSonyWhiteBlanceString(currentPropValue, propertyDisplayStr);
			if (strlen(propertyDisplayStr) == 0)
			{
				sprintf(propertyDisplayStr,	"%lX", currentPropValue);
			}
			break;

		case CrDeviceProperty_reserved4:
		case CrDeviceProperty_reserved5:
		case CrDeviceProperty_reserved6:
		case CrDeviceProperty_reserved7:
		case CrDeviceProperty_reserved8:
		case CrDeviceProperty_reserved9:
		case CrDeviceProperty_reserved10:
		case CrDeviceProperty_reserved11:
		case CrDeviceProperty_reserved12:
		case CrDeviceProperty_reserved13:
		case CrDeviceProperty_reserved20:
		case CrDeviceProperty_reserved22:
		case CrDeviceProperty_reserved24:
		case CrDeviceProperty_reserved25:
		case CrDeviceProperty_reserved26:
		case CrDeviceProperty_reserved27:
			sprintf(propertyDisplayStr,	"%ld", currentPropValue);
			break;

		case CrDeviceProperty_S2:					//NO DOCUMENTATION
		case CrDeviceProperty_LiveView_Area:		//NO DOCUMENTATION
		case CrDeviceProperty_DateTime_Settings:	//NO DOCUMENTATION
		case CrDeviceProperty_EstimatePictureSize:	//NO DOCUMENTATION
		#ifdef _ALPACA_PI_
			sprintf(propertyDisplayStr,	"%ld - <FONT COLOR=RED>", currentPropValue);
		#else
			sprintf(propertyDisplayStr,	"%ld - ", currentPropValue);
		#endif // _ALPACA_PI_
			strcat(propertyDisplayStr, "NO DOCUMENTATION in PDF or example code");
			break;

		case CrDeviceProperty_CustomWB_Execution_State:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 107
			switch(currentPropValue)
			{
				case CrPropertyCustomWBExecutionState_Invalid:			strcpy(propertyDisplayStr,	"Invalid");			break;
				case CrPropertyCustomWBExecutionState_Standby:			strcpy(propertyDisplayStr,	"Standby");			break;
				case CrPropertyCustomWBExecutionState_Capturing:		strcpy(propertyDisplayStr,	"Capturing");		break;
				case CrPropertyCustomWBExecutionState_OperatingCamera:	strcpy(propertyDisplayStr,	"OperatingCamera");	break;
				default:												strcpy(propertyDisplayStr,	"UNKOWN");			break;
			}
			break;

		case CrDeviceProperty_CustomWB_Capture:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 102
		case CrDeviceProperty_CustomWB_Capturable_Area:
		case CrDeviceProperty_CustomWB_Capture_Frame_Size:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 108
			int		xSize, ySize;

			xSize	=	currentPropValue >> 16;
			ySize	=	currentPropValue & 0x00ffff;

			sprintf(propertyDisplayStr, "%d x %d", xSize, ySize);
			break;

		case CrDeviceProperty_CustomWB_Capture_Operation:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 108
			switch(currentPropValue)
			{
				case CrPropertyCustomWBOperation_Disable:	strcpy(propertyDisplayStr,	"Disable");			break;
				case CrPropertyCustomWBOperation_Enable:	strcpy(propertyDisplayStr,	"Enable");			break;
				default:									strcpy(propertyDisplayStr,	"UNKOWN");			break;
			}
			break;

		case CrDeviceProperty_CustomWB_Capture_Standby:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 101
		case CrDeviceProperty_CustomWB_Capture_Standby_Cancel:
			//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 102
			switch(currentPropValue)
			{
				case CrPropertyCustomWBOperation_Disable:	strcpy(propertyDisplayStr,	"Disable");			break;
				case CrPropertyCustomWBOperation_Enable:	strcpy(propertyDisplayStr,	"Enable");			break;
				default:									strcpy(propertyDisplayStr,	"UNKOWN");			break;
			}
			break;


		default:
			sprintf(propertyDisplayStr,	"%ld", currentPropValue);
			foundFlag			=	false;
			break;
	}
#ifdef _INCLUDE_SONY_MAIN_
char	propertyNameStr[128];

	GetSonyPropertyString(propCode, propertyNameStr);
	printf("0x%04X",	propCode);
	printf("\t%-30s",	propertyNameStr);
	printf("\t%d",		numValues);
	printf("\t%10d",	currentPropValue);



	printf("\t%s", propertyDisplayStr);

	if (foundFlag == false)
	{
		printf("\t(not processed)");
	}
	printf("\r\n");
#endif

	return(foundFlag);
}


//*****************************************************************************
void	CameraDriverSONY::ReadProperties(void)
{
CrError				sonyErrCode;
char				sonyErrorString[128];
CrInt32				numOfProperties;
CrInt32				nnn;
CrDeviceProperty	*pProperties;
char				propertyDisplayStr[256];
bool				propProcessedFlag;
int					processedCnt;
int					notProcessedCnt;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cSonyDeviceHandle != (CrDeviceHandle)NULL)
	{
		pProperties		=	nullptr;
		numOfProperties	=	0;
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		sonyErrCode	=	SCRSDK::GetDeviceProperties(cSonyDeviceHandle,	//*	*in*	CrDeviceHandle deviceHandle,
													&pProperties,		//*	*out*	CrDeviceProperty** properties,
													&numOfProperties);	//*	*out*	CrInt32* numOfProperties
		LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::GetDeviceProperties", 	sonyErrCode);
		if (sonyErrCode == CrError_None)
		{
			processedCnt	=	0;
			notProcessedCnt	=	0;
			for (nnn=0; nnn < numOfProperties; nnn++)
			{
				strcpy(propertyDisplayStr, "");
				propProcessedFlag	=	ProcessProperty(&pProperties[nnn], propertyDisplayStr);
				if (propProcessedFlag)
				{
					processedCnt++;
				}
				else
				{
					notProcessedCnt++;
				}
			}
#ifdef _INCLUDE_SONY_MAIN_
			printf("Properties processed\t\t=%d\r\n",	processedCnt);
			if (notProcessedCnt> 0)
			{
				printf("Properties not processed\t=%d\r\n",	notProcessedCnt);
			}
#endif // _INCLUDE_SONY_MAIN_
		}
		else
		{
			GetSonyErrorString(sonyErrCode, sonyErrorString);
			CONSOLE_DEBUG_W_STR("GetDeviceProperties() returned SDK err=\t=", sonyErrorString);
		}
		if (pProperties != nullptr)
		{
			CONSOLE_DEBUG("Calling ReleaseDeviceProperties()");
			sonyErrCode	=	SCRSDK::ReleaseDeviceProperties(cSonyDeviceHandle, pProperties);
			LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::ReleaseDeviceProperties", 	sonyErrCode);
			if (sonyErrCode != CrError_None)
			{
				GetSonyErrorString(sonyErrCode, sonyErrorString);
				CONSOLE_DEBUG_W_STR("ReleaseDeviceProperties() returned SDK err=\t=", sonyErrorString);
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("cSonyDeviceHandle is NULL");
	}
}

//*****************************************************************************
//*	returns Alpaca error code
//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverSONY::Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame)
{
CrError				sonyErrCode;
char				sonyErrorString[128];

	gettimeofday(&cCameraProp.Lastexposure_StartTime, NULL);

	CONSOLE_DEBUG(__FUNCTION__);
	if (cSonyDeviceHandle != (CrDeviceHandle)NULL)
	{
	CrDeviceProperty	cameraProperty;

	#ifdef _ALPACA_PI_
	char		savePath[]		=	"./";
	char		mySonyFileName[256];

		GenerateFileNameRoot();
		CONSOLE_DEBUG_W_STR("cFileNameRoot\t=", cFileNameRoot);
		strcpy(mySonyFileName, cFileNameRoot);
		strcat(mySonyFileName, "-");
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		sonyErrCode	=	SCRSDK::SetSaveInfo(cSonyDeviceHandle, savePath, mySonyFileName, -1);
		LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::SetSaveInfo", 	sonyErrCode, mySonyFileName);
		if (sonyErrCode == CrError_None)
		{
			CONSOLE_DEBUG("SetSaveInfo OK!!!");
		}
		else
		{
			GetSonyErrorString(sonyErrCode, sonyErrorString);
			CONSOLE_DEBUG_W_STR("!!!!!!!!!!!!!!!!!!!!!!!!!! SetSaveInfo() returned err=\t=", sonyErrorString);
		}
	#endif
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 94,
		CONSOLE_DEBUG("Setting property - CrDeviceProperty_StillImageStoreDestination");
		cameraProperty.SetCode(				CrDeviceProperty_StillImageStoreDestination);
//		cameraProperty.SetCurrentValue(		CrStillImageStoreDestination_HostPCAndMemoryCard);
		cameraProperty.SetCurrentValue(		CrStillImageStoreDestination_HostPC);
		cameraProperty.SetValueType(		CrDataType_UInt16);
		sonyErrCode	=	SCRSDK::SetDeviceProperty(cSonyDeviceHandle, &cameraProperty);
		LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::SetDeviceProperty", 	sonyErrCode, "CrStillImageStoreDestination_HostPC");
		if (sonyErrCode == CrError_None)
		{
			CONSOLE_DEBUG("SetDeviceProperty OK!!!");
		}
		else
		{
			GetSonyErrorString(sonyErrCode, sonyErrorString);
			CONSOLE_DEBUG_W_STR("!!!!!!!!!!!!!!!!!!!!!!!!!!Sony SDK err\t=", sonyErrorString);
		}

#if 1
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		//*	Sony_CameraRemoteSDK_API-Reference_v1.01.00.pdf		-	page 101,
		CONSOLE_DEBUG("Setting property - CrDeviceProperty_RAW_J_PC_Save_Image");
		cameraProperty.SetCode(				CrDeviceProperty_RAW_J_PC_Save_Image);
		cameraProperty.SetCurrentValue(		CrPropertyRAWJPCSaveImage_RAWAndJPEG);
//		cameraProperty.SetCurrentValue(		CrPropertyRAWJPCSaveImage_JPEGOnly);

		cameraProperty.SetValueType(		CrDataType_UInt16);
		sonyErrCode	=	SCRSDK::SetDeviceProperty(cSonyDeviceHandle, &cameraProperty);
		LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::SetDeviceProperty", 	sonyErrCode);
		if (sonyErrCode == CrError_None)
		{
			CONSOLE_DEBUG("SetDeviceProperty OK!!!");
		}
		else
		{
			GetSonyErrorString(sonyErrCode, sonyErrorString);
			CONSOLE_DEBUG_W_STR("!!!!!!!!!!!!!!!!!!!!!!!!!!SetDeviceProperty() returned err=\t=", sonyErrorString);
		}

#endif // 0

//		CONSOLE_DEBUG("calling sleep");
//		sleep(2);
//		CONSOLE_DEBUG("awake");

		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		CONSOLE_DEBUG("Calling SendCommand() with CrCommandId_Release, CrCommandParam_Down");
		sonyErrCode	=	SCRSDK::SendCommand(cSonyDeviceHandle, CrCommandId_Release, CrCommandParam_Down);
		LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::SendCommand", 	sonyErrCode, "CrCommandParam_Down");
		if (sonyErrCode != CrError_None)
		{
			GetSonyErrorString(sonyErrCode, sonyErrorString);
			CONSOLE_DEBUG_W_STR("!!!!!!!!!!!!!!!!!!!!!!!!!!Sony SDK err=\t=", sonyErrorString);
		}

		// Wait, then send shutter up
		CONSOLE_DEBUG("calling sleep");
		usleep(35 * 1000);
//		usleep(335 * 1000);
		sleep(5);
		CONSOLE_DEBUG("awake");
		CONSOLE_DEBUG("Calling SendCommand() with CrCommandId_Release, CrCommandParam_Up");
		sonyErrCode	=	SCRSDK::SendCommand(cSonyDeviceHandle, CrCommandId_Release, CrCommandParam_Up);
		LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::SendCommand", 	sonyErrCode, "CrCommandParam_Up");
		if (sonyErrCode != CrError_None)
		{
			GetSonyErrorString(sonyErrCode, sonyErrorString);
			CONSOLE_DEBUG_W_STR("!!!!!!!!!!!!!!!!!!!!!!!!!!Sony SDK err=\t=", sonyErrorString);
		}
	}
	else
	{
		CONSOLE_DEBUG("cSonyDeviceHandle is NULL");
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "exit");

	return(kASCOM_Err_Success);
}

//*****************************************************************************
TYPE_ASCOM_STATUS		CameraDriverSONY::Stop_Exposure(void)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	strcpy(cLastCameraErrMsg, "Not finished-");
	strcat(cLastCameraErrMsg, __FILE__);
	strcat(cLastCameraErrMsg, ":");
	strcat(cLastCameraErrMsg, __FUNCTION__);
	CONSOLE_DEBUG(cLastCameraErrMsg);

	return(alpacaErrCode);
}


#ifdef _ALPACA_PI_
//*****************************************************************************
TYPE_EXPOSURE_STATUS	CameraDriverSONY::Check_Exposure(bool verboseFlag)
{
	CONSOLE_DEBUG(__FUNCTION__);


	return(kExposure_Unknown);
}
#endif // _ALPACA_PI_

//*****************************************************************************
//*	returns Alpaca error code
//*****************************************************************************
int	CameraDriverSONY::GetLiveView(void)
{
CrError				sonyErrCode;
char				sonyErrorString[128];
CrImageInfo			*pInfo;
CrImageDataBlock	*pLiveViewImage;
CrInt32u			imgSize;
CrInt8u				*pdata;

	CONSOLE_DEBUG(__FUNCTION__);

	gettimeofday(&cCameraProp.Lastexposure_StartTime, NULL);

	if (cSonyDeviceHandle != (CrDeviceHandle)NULL)
	{
		pInfo		=	new CrImageInfo();
		CONSOLE_DEBUG("Calling SCRSDK::GetLiveViewImageInfo()");
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		sonyErrCode	=	SCRSDK::GetLiveViewImageInfo(cSonyDeviceHandle, pInfo);
		LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::GetLiveViewImageInfo", 	sonyErrCode);
		if (sonyErrCode == CrError_None)
		{
			CONSOLE_DEBUG("GetLiveViewImageInfo()\t--SUCCESS!!!!!");
			pLiveViewImage			= new CrImageDataBlock();
			if (pLiveViewImage != NULL)
			{
				CONSOLE_DEBUG("CrImageDataBlock()\t--SUCCESS!!!!!!");
				pLiveViewImage->frameNo	=	0;
		//		pLiveViewImage->size	=	pInfo->bufferSize;
				pLiveViewImage->size	=	pInfo->GetBufferSize();
				if (pLiveViewImage->size > 0)
				{
				//	pLiveViewImage->pData	=	new CrInt8u[pLiveViewImage->size];

					pLiveViewImage->pData	=	(unsigned char *)malloc(pLiveViewImage->size);
					if (pLiveViewImage->pData != NULL)
					{
						CONSOLE_DEBUG("Calling SCRSDK::GetLiveViewImage()");
						//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
						sonyErrCode	=	SCRSDK::GetLiveViewImage(cSonyDeviceHandle, pLiveViewImage);
						LogFunctionCall(__FUNCTION__, __LINE__,	"SCRSDK::GetLiveViewImage", 	sonyErrCode);
						if (sonyErrCode == CrError_None)
						{
							imgSize					=	pLiveViewImage->GetImageSize();
							pdata					=	pLiveViewImage->GetImageData();
						//	CONSOLE_DEBUG_W_NUM("imgSize\t=", imgSize);
						//	CONSOLE_DEBUG_W_HEX("pdata\t=", pdata);

							SaveData(pdata, imgSize, NULL);
						}
						else
						{
							GetSonyErrorString(sonyErrCode, sonyErrorString);
							CONSOLE_DEBUG_W_STR("GetLiveViewImage() returned =\t=", sonyErrorString);
						}
						free(pLiveViewImage->pData);
						pLiveViewImage->pData	=	NULL;
					}
					else
					{
						CONSOLE_DEBUG("Failed to allocate buffer for image");
					}
				}
				else
				{
					CONSOLE_DEBUG("Image size is ZERO!!!!");
					CONSOLE_DEBUG_W_NUM("pLiveViewImage->size\t=", pLiveViewImage->size);
				}
			}
			else
			{
				CONSOLE_DEBUG("Failed to create CrImageDataBlock()");
			}
		}
		else
		{
			GetSonyErrorString(sonyErrCode, sonyErrorString);
			CONSOLE_DEBUG_W_STR("GetLiveViewImageInfo() returned=\t=", sonyErrorString);
		}
	}
	else
	{
		CONSOLE_DEBUG("cSonyDeviceHandle is NULL");
	}
	return(0);
}

//*****************************************************************************
void	CameraDriverSONY::ReadSONYcameraInfo(void)
{
	if (cSonyCamera_info != NULL)
	{
		strcpy(cSONYidString,		cSonyCamera_info->GetModel());
		strcpy(cCommonProp.Name,	cSonyCamera_info->GetModel());

		CONSOLE_DEBUG_W_STR("Conn type\t\t=",	cSONYidString);
		CONSOLE_DEBUG_W_STR("Camera model\t=",	cCommonProp.Name);
	}
	else
	{
		CONSOLE_DEBUG("Failed to get camera info");
	}

}

//*****************************************************************************
void	CameraDriverSONY::OnConnected(DeviceConnectionVersioin version)
{
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("DeviceConnectionVersioin\t=", version);
}
//*****************************************************************************
void	CameraDriverSONY::OnDisconnected(CrInt32u error)
{
	CONSOLE_DEBUG(__FUNCTION__);
}
//*****************************************************************************
//*	Page 21 of documentation
//*		...calling the GetDeviceProperties() function in the OnPropertyChanged() callback
//*		function is not recommended, because the callback function is called on the
//*		thread that communicates with the camera. All callback functions are expected to
//*		return as soon as possible
//*****************************************************************************
void	CameraDriverSONY::OnPropertyChanged(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
//	cUpdateProperties	=	true;

}
//*****************************************************************************
void	CameraDriverSONY::OnLvPropertyChanged(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}
//*****************************************************************************
void	CameraDriverSONY::OnCompleteDownload(CrChar *filename)
{
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(filename);
}
//*****************************************************************************
void	CameraDriverSONY::OnWarning(CrInt32u warning)
{
char	sonyErrorString[128];

	CONSOLE_DEBUG(__FUNCTION__);
	GetSonyErrorString(warning, sonyErrorString);
	CONSOLE_DEBUG_W_HEX("Sony SDK warning\t=", warning);
	CONSOLE_DEBUG_W_STR("Sony SDK warning\t=", sonyErrorString);
}
//*****************************************************************************
void	CameraDriverSONY::OnError(CrInt32u error)
{
char	sonyErrorString[128];

	CONSOLE_DEBUG(__FUNCTION__);
	GetSonyErrorString(error, sonyErrorString);
	CONSOLE_DEBUG_W_STR("Sony SDK err=\t=", sonyErrorString);
}

//*****************************************************************************
void	LogFunctionCall(const char		*callingFunc,
						const int		lineNumber,
						const char		*functionName,
						const CrError	returnedErrCode,
						const char		*comment)
{
char				sonyErrorString[128];
char				timeString[128];
char				fileName[128];
struct timeval		currTime;
struct tm			*linuxTime;

	if (gFunctionLogIdx < kFunctionLogSize)
	{
		strcpy(gFunctionCallLog[gFunctionLogIdx].callingFunc,	callingFunc);
		strcpy(gFunctionCallLog[gFunctionLogIdx].functionName,	functionName);
		strcpy(gFunctionCallLog[gFunctionLogIdx].comment,		comment);


		gFunctionCallLog[gFunctionLogIdx].returnedErrCode	=	returnedErrCode;

		gFunctionLogIdx++;
	}
	else
	{
		CONSOLE_DEBUG("Exceed log table size");
	}

	gettimeofday(&currTime, NULL);
	linuxTime		=	localtime(&currTime.tv_sec);

	sprintf(timeString, "%d-%02d-%02dT%02d_%02d_%02d",
							(1900 + linuxTime->tm_year),
							(1 + linuxTime->tm_mon),
							linuxTime->tm_mday,
							linuxTime->tm_hour,
							linuxTime->tm_min,
							linuxTime->tm_sec);

	//==========================================================
	//*	write to disk
	if (gSonyLogFile == NULL)
	{

		sprintf(fileName, "SonyLog-%s.csv", timeString);

		gSonyLogFile	=	fopen(fileName, "w");
		if (gSonyLogFile != NULL)
		{
			fprintf(gSonyLogFile,	"%s,",	"Time");
			fprintf(gSonyLogFile,	"%s,",	"Calling Function Name");
			fprintf(gSonyLogFile,	"%s,",	"Calling Function Line#");
			fprintf(gSonyLogFile,	"%s,",	"SDK Function");
			fprintf(gSonyLogFile,	"%s,",	"SDK returned err code");
			fprintf(gSonyLogFile,	"%s,",	"SDK returned err string");
			fprintf(gSonyLogFile,	"%s,",	"Comment");
			fprintf(gSonyLogFile,	"\n");
			fflush(gSonyLogFile);
		}
	}

	if (gSonyLogFile != NULL)
	{
		GetSonyErrorString(returnedErrCode, sonyErrorString);

		sprintf(timeString, "%02d:%02d:%02d",
								linuxTime->tm_hour,
								linuxTime->tm_min,
								linuxTime->tm_sec);

		fprintf(gSonyLogFile,	"%s,",		timeString);
		fprintf(gSonyLogFile,	"%s,",		callingFunc);
		fprintf(gSonyLogFile,	"%d,",		lineNumber);
		fprintf(gSonyLogFile,	"%s,",		functionName);
		fprintf(gSonyLogFile,	"0x%X,",	returnedErrCode);
		fprintf(gSonyLogFile,	"%s,",		sonyErrorString);
		fprintf(gSonyLogFile,	"%s,",		comment);
		fprintf(gSonyLogFile,	"\n");
		fflush(gSonyLogFile);
	}
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
//*****************************************************************************
SonyCallBack::SonyCallBack(void *alpacaCameraDriver)
{
	CONSOLE_DEBUG(__PRETTY_FUNCTION__);

	cSonyCameraDriverObjPtr		=	alpacaCameraDriver;

	cOnConnected_Count			=	0;
	cOnDisconnected_Count		=	0;
	cOnPropertyChanged_Count	=	0;
	cOnLvPropertyChanged_Count	=	0;
	cOnCompleteDownload_Count	=	0;
	cOnWarning_Count			=	0;
	cOnError_Count				=	0;
}

//*****************************************************************************
SonyCallBack::~SonyCallBack(void)
{
	CONSOLE_DEBUG(__PRETTY_FUNCTION__);
	cSonyCameraDriverObjPtr	=	NULL;

	CONSOLE_DEBUG_W_NUM("cOnConnected_Count\t\t=",			cOnConnected_Count);
	CONSOLE_DEBUG_W_NUM("cOnDisconnected_Count\t=",			cOnDisconnected_Count);
	CONSOLE_DEBUG_W_NUM("cOnPropertyChanged_Count\t=",		cOnPropertyChanged_Count);
	CONSOLE_DEBUG_W_NUM("cOnLvPropertyChanged_Count\t=",	cOnLvPropertyChanged_Count);
	CONSOLE_DEBUG_W_NUM("cOnCompleteDownload_Count\t=",		cOnCompleteDownload_Count);
	CONSOLE_DEBUG_W_NUM("cOnWarning_Count\t\t=",			cOnWarning_Count);
	CONSOLE_DEBUG_W_NUM("cOnError_Count\t\t=",				cOnError_Count);
}


//*****************************************************************************
void	SonyCallBack::OnConnected(DeviceConnectionVersioin version)
{
CameraDriverSONY	*myObjPtr;

	CONSOLE_DEBUG(__PRETTY_FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("DeviceConnectionVersioin\t=", version);
	myObjPtr	=	(CameraDriverSONY *)cSonyCameraDriverObjPtr;
	if (myObjPtr != NULL)
	{
		myObjPtr->OnConnected(version);
	}
	else
	{
		CONSOLE_DEBUG("myObjPtr error");
	}
	cOnConnected_Count++;
}
//*****************************************************************************
void	SonyCallBack::OnDisconnected(CrInt32u error)
{
CameraDriverSONY	*myObjPtr;

	CONSOLE_DEBUG(__PRETTY_FUNCTION__);
	myObjPtr	=	(CameraDriverSONY *)cSonyCameraDriverObjPtr;
	if (myObjPtr != NULL)
	{
		myObjPtr->OnDisconnected(error);
	}
	else
	{
		CONSOLE_DEBUG("myObjPtr error");
	}
	cOnDisconnected_Count++;
}
//*****************************************************************************
void	SonyCallBack::OnPropertyChanged(void)
{
CameraDriverSONY	*myObjPtr;

	CONSOLE_DEBUG(__PRETTY_FUNCTION__);
	myObjPtr	=	(CameraDriverSONY *)cSonyCameraDriverObjPtr;
	if (myObjPtr != NULL)
	{
		myObjPtr->OnPropertyChanged();
	}
	else
	{
		CONSOLE_DEBUG("myObjPtr error");
	}
	cOnPropertyChanged_Count++;
}
//*****************************************************************************
void	SonyCallBack::OnLvPropertyChanged(void)
{
CameraDriverSONY	*myObjPtr;

	CONSOLE_DEBUG(__PRETTY_FUNCTION__);
	myObjPtr	=	(CameraDriverSONY *)cSonyCameraDriverObjPtr;
	if (myObjPtr != NULL)
	{
		myObjPtr->OnLvPropertyChanged();
	}
	else
	{
		CONSOLE_DEBUG("myObjPtr error");
	}
	cOnLvPropertyChanged_Count++;
}
//*****************************************************************************
void	SonyCallBack::OnCompleteDownload(CrChar *filename)
{
CameraDriverSONY	*myObjPtr;

	CONSOLE_DEBUG(__PRETTY_FUNCTION__);
	CONSOLE_DEBUG(filename);
	myObjPtr	=	(CameraDriverSONY *)cSonyCameraDriverObjPtr;
	if (myObjPtr != NULL)
	{
		myObjPtr->OnCompleteDownload(filename);
	}
	else
	{
		CONSOLE_DEBUG("myObjPtr error");
	}
	cOnCompleteDownload_Count++;
}
//*****************************************************************************
void	SonyCallBack::OnWarning(CrInt32u warning)
{
CameraDriverSONY	*myObjPtr;

	CONSOLE_DEBUG(__PRETTY_FUNCTION__);
	myObjPtr	=	(CameraDriverSONY *)cSonyCameraDriverObjPtr;
	if (myObjPtr != NULL)
	{
		myObjPtr->OnWarning(warning);
	}
	else
	{
		CONSOLE_DEBUG("myObjPtr error");
	}
	cOnWarning_Count	++;
}
//*****************************************************************************
void	SonyCallBack::OnError(CrInt32u error)
{
CameraDriverSONY	*myObjPtr;

	CONSOLE_DEBUG(__PRETTY_FUNCTION__);
	myObjPtr	=	(CameraDriverSONY *)cSonyCameraDriverObjPtr;
	if (myObjPtr != NULL)
	{
		myObjPtr->OnError(error);
	}
	else
	{
		CONSOLE_DEBUG("myObjPtr error");
	}
	cOnError_Count++;
}


//*****************************************************************************
typedef struct
{
	uint32_t	sonyStringCode;
	char		sonyMsgString[64];

} TYPE_SONY_STRINGS;

//*****************************************************************************
static const TYPE_SONY_STRINGS gSonyErrMsgs[]	=
{
	{	CrError_None,						"CrError_None"		},
	{	CrError_Genric,						"CrError_Genric"	},
	{	CrError_File,						"CrError_File"		},
	{	CrError_Connect,					"CrError_Connect"	},
	{	CrError_Memory,						"CrError_Memory"	},
	{	CrError_Api,						"CrError_Api"		},
	{	CrError_Init,						"CrError_Init"		},
	{	CrError_Polling,					"CrError_Polling"	},
	{	CrError_Adaptor,					"CrError_Adaptor"	},
	{	CrError_Device,						"CrError_Device"	},

	{	CrError_Application,				"CrError_Application"			},
	// Uncategorized
	{	CrError_Genric_Unknown,				"CrError_Genric_Unknown"		},
	{	CrError_Genric_Notimpl,				"CrError_Genric_Notimpl"		},
	{	CrError_Genric_Abort,				"CrError_Genric_Abort"			},
	{	CrError_Genric_NotSupported,		"CrError_Genric_NotSupported"	},
	{	CrError_Genric_SeriousErrorNotSupported,	"CrError_Genric_SeriousErrorNotSupported"	},
	{	CrError_Genric_InvalidHandle,				"CrError_Genric_InvalidHandle"	},
	{	CrError_Genric_InvalidParameter,			"CrError_Genric_InvalidParameter"	},

	// File related
	{	CrError_File_Unknown,				"CrError_File_Unknown"			},
	{	CrError_File_IllegalOperation,		"CrError_File_IllegalOperation"	},
	{	CrError_File_IllegalParameter,		"CrError_File_IllegalParameter"	},
	{	CrError_File_EOF,					"CrError_File_EOF"				},
	{	CrError_File_OutOfRange,			"CrError_File_OutOfRange"		},
	{	CrError_File_NotFound,				"CrError_File_NotFound"			},
	{	CrError_File_DirNotFound,			"CrError_File_DirNotFound"		},
	{	CrError_File_AlreadyOpened,			"CrError_File_AlreadyOpened"	},
	{	CrError_File_PermissionDenied,		"CrError_File_PermissionDenied"	},
	{	CrError_File_StorageFull,			"CrError_File_StorageFull"		},
	{	CrError_File_AlreadyExists,			"CrError_File_AlreadyExists"	},
	{	CrError_File_TooManyOpenedFiles,	"CrError_File_TooManyOpenedFiles"	},
	{	CrError_File_ReadOnly,				"CrError_File_ReadOnly"	},
	{	CrError_File_CantOpen,				"CrError_File_CantOpen"	},
	{	CrError_File_CantClose,				"CrError_File_CantClose"	},
	{	CrError_File_CantDelete,			"CrError_File_CantDelete"	},
	{	CrError_File_CantRead,				"CrError_File_CantRead"	},
	{	CrError_File_CantWrite,				"CrError_File_CantWrite"	},
	{	CrError_File_CantCreateDir,			"CrError_File_CantCreateDir"	},
	{	CrError_File_OperationAbortedByUser,	"CrError_File_OperationAbortedByUser"	},
	{	CrError_File_UnsupportedOperation,	"CrError_File_UnsupportedOperation"	},
	{	CrError_File_NotYetCompleted,		"CrError_File_NotYetCompleted"	},
	{	CrError_File_Invalid,				"CrError_File_Invalid"	},
	{	CrError_File_StorageNotExist,		"CrError_File_StorageNotExist"	},
	{	CrError_File_SharingViolation,		"CrError_File_SharingViolation"	},
	{	CrError_File_Rotation,				"CrError_File_Rotation"	},
	{	CrError_File_SameNameFull,			"CrError_File_SameNameFull"	},

	// Connection related
	{	CrError_Connect_Unknown,			"CrError_Connect_Unknown"	},
	{	CrError_Connect_Connect,			"CrError_Connect_Connect"	},
	{	CrError_Connect_Release,			"CrError_Connect_Release"	},
	{	CrError_Connect_GetProperty,		"CrError_Connect_GetProperty"	},
	{	CrError_Connect_SendCommand,		"CrError_Connect_SendCommand"	},
	{	CrError_Connect_HandlePlugin,		"CrError_Connect_HandlePlugin"	},
	{	CrError_Connect_Disconnected,		"CrError_Connect_Disconnected"	},
	{	CrError_Connect_TimeOut,			"CrError_Connect_TimeOut"	},
	{	CrError_Reconnect_TimeOut,			"CrError_Reconnect_TimeOut"	},
	{	CrError_Connect_FailRejected,		"CrError_Connect_FailRejected"	},
	{	CrError_Connect_FailBusy,			"CrError_Connect_FailBusy"	},
	{	CrError_Connect_FailUnspecified,	"CrError_Connect_FailUnspecified"	},
	{	CrError_Connect_Cancel,				"CrError_Connect_Cancel"	},

	// Memory related
	{	CrError_Memory_Unknown,				"CrError_Memory_Unknown"	},
	{	CrError_Memory_OutOfMemory,			"CrError_Memory_OutOfMemory"	},
	{	CrError_Memory_InvalidPointer,		"CrError_Memory_InvalidPointer"	},
	{	CrError_Memory_Insufficient,		"CrError_Memory_Insufficient"	},

	// API related
	{	CrError_Api_Unknown,				"CrError_Api_Unknown"	},
	{	CrError_Api_Insufficient,			"CrError_Api_Insufficient"	},
	{	CrError_Api_InvalidCalled,			"CrError_Api_InvalidCalled"	},

	{	CrError_Polling_Unknown,			"CrError_Polling_Unknown"	},
	{	CrError_Polling_InvalidVal_Intervals,	"CrError_Polling_InvalidVal_Intervals"	},

	//Adaptor related
	{	CrError_Adaptor_Unknown,			"CrError_Adaptor_Unknown"			},
	{	CrError_Adaptor_InvaildProperty,	"CrError_Adaptor_InvaildProperty"	},
	{	CrError_Adaptor_GetInfo,			"CrError_Adaptor_GetInfo"			},
	{	CrError_Adaptor_Create,				"CrError_Adaptor_Create"			},
	{	CrError_Adaptor_SendCommand,		"CrError_Adaptor_SendCommand"		},
	{	CrError_Adaptor_HandlePlugin,		"CrError_Adaptor_HandlePlugin"		},
	{	CrError_Adaptor_CreateDevice,		"CrError_Adaptor_CreateDevice"		},
	{	CrError_Adaptor_EnumDecvice,		"CrError_Adaptor_EnumDecvice"		},
	{	CrError_Adaptor_Reset,				"CrError_Adaptor_Reset"				},
	{	CrError_Adaptor_Read,				"CrError_Adaptor_Read"				},
	{	CrError_Adaptor_Phase,				"CrError_Adaptor_Phase"				},
	{	CrError_Adaptor_DataToWiaItem,		"CrError_Adaptor_DataToWiaItem"		},
	{	CrError_Adaptor_DeviceBusy,			"CrError_Adaptor_DeviceBusy"		},
	{	CrError_Adaptor_Escape,				"CrError_Adaptor_Escape"			},
	{	CrError_Device_Unknown,				"CrError_Device_Unknown"			},

	{	CrWarning_Unknown,					"CrWarning_Unknown"					},
	{	CrWarning_Connect_Reconnected,		"CrWarning_Connect_Reconnected"		},
	{	CrWarning_Connect_Reconnecting,		"CrWarning_Connect_Reconnecting"	},
	{	CrWarning_File_StorageFull,			"CrWarning_File_StorageFull"		},
	{	CrWarning_SetFileName_Failed,		"CrWarning_SetFileName_Failed"		},
	{	CrWarning_GetImage_Failed,			"CrWarning_GetImage_Failed"			},
	{	CrWarning_FailedToSetCWB,			"CrWarning_FailedToSetCWB"			},
	{	CrWarning_NetworkErrorOccurred,		"CrWarning_NetworkErrorOccurred"	},
	{	CrWarning_NetworkErrorRecovered,	"CrWarning_NetworkErrorRecovered"	},
	{	CrWarning_Format_Failed,			"CrWarning_Format_Failed"			},
	{	CrWarning_Format_Invalid,			"CrWarning_Format_Invalid"			},
	{	CrWarning_Format_Complete,			"CrWarning_Format_Complete"			},
	{	CrWarning_Exposure_Started,			"CrWarning_Exposure_Started"		},
	{	CrWarning_DateTime_Setting_Result_Invalid,			"CrWarning_DateTime_Setting_Result_Invalid"	},
	{	CrWarning_DateTime_Setting_Result_OK,				"CrWarning_DateTime_Setting_Result_OK"	},
	{	CrWarning_DateTime_Setting_Result_Parameter_Error,	"CrWarning_DateTime_Setting_Result_Parameter_Error"	},
	{	CrWarning_DateTime_Setting_Result_Exclusion_Error,	"CrWarning_DateTime_Setting_Result_Exclusion_Error"	},
	{	CrWarning_DateTime_Setting_Result_System_Error,		"CrWarning_DateTime_Setting_Result_System_Error"	},
	{	CrWarning_Frame_NotUpdated,			"CrWarning_Frame_NotUpdated"		},

	{	CrNotify_All_Download_Complete,		"CrNotify_All_Download_Complete"	},

	{	0,		""	}

};



//*****************************************************************************
static const TYPE_SONY_STRINGS gExposureModeStrings[]	=
{
	{	CrExposure_M_Manual,					"M_Manual"				},
	{	CrExposure_P_Auto,						"P_Auto"				},
	{	CrExposure_A_AperturePriority,			"A_AperturePriority"	},
	{	CrExposure_S_ShutterSpeedPriority,		"S_ShutterSpeedPriority"},
	{	CrExposure_Program_Creative,			"Program_Creative"		},
	{	CrExposure_Program_Action,				"Program_Action"		},
	{	CrExposure_Portrait,					"Portrait"				},
	{	CrExposure_Auto,						"Auto"					},
	{	CrExposure_Auto_Plus,					"Auto_Plus"				},
	{	CrExposure_P_A,							"P_A"					},
	{	CrExposure_P_S,							"P_S"					},
	{	CrExposure_Sprots_Action,				"Sprots_Action"			},
	{	CrExposure_Sunset,						"Sunset"				},
	{	CrExposure_Night,						"Night"					},
	{	CrExposure_Landscape,					"Landscape"				},
	{	CrExposure_Macro,						"Macro"					},
	{	CrExposure_HandheldTwilight,			"HandheldTwilight"		},
	{	CrExposure_NightPortrait,				"NightPortrait"			},
	{	CrExposure_AntiMotionBlur,				"AntiMotionBlur"		},
	{	CrExposure_Pet,							"Pet"					},
	{	CrExposure_Gourmet,						"Gourmet"				},
	{	CrExposure_Fireworks,					"Fireworks"				},
	{	CrExposure_HighSensitivity,				"HighSensitivity"		},
	{	CrExposure_MemoryRecall,				"MemoryRecall"			},
	{	CrExposure_ContinuousPriority_AE_8pics,	"ContinuousPriority_AE_8pics"	},
	{	CrExposure_ContinuousPriority_AE_10pics,"ContinuousPriority_AE_10pics"	},
	{	CrExposure_ContinuousPriority_AE_12pics,"ContinuousPriority_AE_12pics"	},
	{	CrExposure_3D_SweepPanorama,			"3D_SweepPanorama"		},
	{	CrExposure_SweepPanorama,				"SweepPanorama"			},
	{	CrExposure_Movie_P,						"Movie_P"				},
	{	CrExposure_Movie_A,						"Movie_A"				},
	{	CrExposure_Movie_S,						"Movie_S"				},
	{	CrExposure_Movie_M,						"Movie_M"				},
	{	CrExposure_Movie_Auto,					"Movie_Auto"			},
	{	CrExposure_Movie_SQMotion_P,			"Movie_SQMotion_P"		},
	{	CrExposure_Movie_SQMotion_A,			"Movie_SQMotion_A"		},
	{	CrExposure_Movie_SQMotion_S,			"Movie_SQMotion_S"		},
	{	CrExposure_Movie_SQMotion_M,			"Movie_SQMotion_M"		},
	{	CrExposure_Flash_Off,					"Flash_Off"				},
	{	CrExposure_PictureEffect,				"PictureEffect"			},
	{	CrExposure_HiFrameRate_P,				"HiFrameRate_P"			},
	{	CrExposure_HiFrameRate_A,				"HiFrameRate_A"			},
	{	CrExposure_HiFrameRate_S,				"HiFrameRate_S"			},
	{	CrExposure_HiFrameRate_M,				"HiFrameRate_M"			},
	{	CrExposure_SQMotion_P,					"SQMotion_P"			},
	{	CrExposure_SQMotion_A,					"SQMotion_A"			},
	{	CrExposure_SQMotion_S,					"SQMotion_S"			},
	{	CrExposure_SQMotion_M,					"SQMotion_M"			},
	{	CrExposure_MOVIE,						"MOVIE"					},
	{	CrExposure_STILL,						"STILL"					},

	{	0,		""	}
};

//*****************************************************************************
static const TYPE_SONY_STRINGS gSonyPropertyNames[]	=
{
//	{	CrDeviceProperty_Undefined,			"CrDeviceProperty_Undefined"	},


	{	CrDeviceProperty_S1,							"S1"					},
	{	CrDeviceProperty_AEL,							"AEL"					},
	{	CrDeviceProperty_FEL,							"FEL"					},
	{	CrDeviceProperty_AFL,							"AFL"					},
	{	CrDeviceProperty_AWBL,							"AWBL"					},
	{	CrDeviceProperty_FNumber,						"FNumber"				},
	{	CrDeviceProperty_ExposureBiasCompensation,		"ExposureBiasCompensation"	},
	{	CrDeviceProperty_FlashCompensation,				"FlashCompensation"		},
	{	CrDeviceProperty_ShutterSpeed,					"ShutterSpeed"			},
	{	CrDeviceProperty_IsoSensitivity,				"IsoSensitivity"		},
	{	CrDeviceProperty_ExposureProgramMode,			"ExposureProgramMode"	},
	{	CrDeviceProperty_FileType,						"FileType"				},
	{	CrDeviceProperty_JpegQuality,					"JpegQuality"			},
	{	CrDeviceProperty_WhiteBalance,					"WhiteBalance"			},
	{	CrDeviceProperty_FocusMode,						"FocusMode"				},
	{	CrDeviceProperty_MeteringMode,					"MeteringMode"			},
	{	CrDeviceProperty_FlashMode,						"FlashMode"				},
	{	CrDeviceProperty_WirelessFlash,					"WirelessFlash"			},
	{	CrDeviceProperty_RedEyeReduction,				"RedEyeReduction"		},
	{	CrDeviceProperty_DriveMode,						"DriveMode"				},
	{	CrDeviceProperty_DRO,							"DRO"					},
	{	CrDeviceProperty_ImageSize,						"ImageSize"				},
	{	CrDeviceProperty_AspectRatio,					"AspectRatio"			},
	{	CrDeviceProperty_PictureEffect,					"PictureEffect"			},
	{	CrDeviceProperty_FocusArea,						"FocusArea"				},
	{	CrDeviceProperty_reserved4,						"reserved4"				},
	{	CrDeviceProperty_Colortemp,						"Colortemp"				},
	{	CrDeviceProperty_ColorTuningAB,					"ColorTuningAB"			},
	{	CrDeviceProperty_ColorTuningGM,					"ColorTuningGM"			},
	{	CrDeviceProperty_LiveViewDisplayEffect,			"LiveViewDisplayEffect"	},
	{	CrDeviceProperty_StillImageStoreDestination,	"StillImageStoreDestination"	},
	{	CrDeviceProperty_PriorityKeySettings,			"PriorityKeySettings"	},
	{	CrDeviceProperty_reserved5,						"reserved5"				},
	{	CrDeviceProperty_reserved6,						"reserved6"				},
	{	CrDeviceProperty_Focus_Magnifier_Setting,		"Focus_Magnifier_Setting"	},
	{	CrDeviceProperty_DateTime_Settings,				"DateTime_Settings"		},
	{	CrDeviceProperty_NearFar,						"NearFar"				},
	{	CrDeviceProperty_reserved7,						"reserved7"				},
	{	CrDeviceProperty_AF_Area_Position,				"AF_Area_Position"		},
	{	CrDeviceProperty_reserved8,						"reserved8"				},
	{	CrDeviceProperty_reserved9,						"reserved9"				},
	{	CrDeviceProperty_Zoom_Scale,					"Zoom_Scale"			},
	{	CrDeviceProperty_Zoom_Setting,					"Zoom_Setting"			},
	{	CrDeviceProperty_Zoom_Operation,				"Zoom_Operation"		},
	{	CrDeviceProperty_Movie_File_Format,				"Movie_File_Format"				},
	{	CrDeviceProperty_Movie_Recording_Setting,		"Movie_Recording_Setting"		},
	{	CrDeviceProperty_S2,							"S2"							},
	{	CrDeviceProperty_reserved10,					"reserved10"					},
	{	CrDeviceProperty_reserved11,					"reserved11"					},
	{	CrDeviceProperty_reserved12,					"reserved12"					},
	{	CrDeviceProperty_reserved13,					"reserved13"					},
	{	CrDeviceProperty_Interval_Rec_Mode,				"Interval_Rec_Mode"				},
	{	CrDeviceProperty_Still_Image_Trans_Size,		"Still_Image_Trans_Size"		},
	{	CrDeviceProperty_RAW_J_PC_Save_Image,			"RAW_J_PC_Save_Image"			},
	{	CrDeviceProperty_LiveView_Image_Quality,		"LiveView_Image_Quality"		},
	{	CrDeviceProperty_CustomWB_Capture_Standby,		"CustomWB_Capture_Standby"		},
	{	CrDeviceProperty_CustomWB_Capture_Standby_Cancel,"CustomWB_Capture_Standby_Cancel"	},
	{	CrDeviceProperty_CustomWB_Capture,				"CustomWB_Capture"				},
	{	CrDeviceProperty_GetOnly,						"GetOnly"						},
	{	CrDeviceProperty_SnapshotInfo,					"SnapshotInfo"					},
	{	CrDeviceProperty_BatteryRemain,					"BatteryRemain"					},
	{	CrDeviceProperty_BatteryLevel,					"BatteryLevel"					},
	{	CrDeviceProperty_EstimatePictureSize,			"EstimatePictureSize"			},
	{	CrDeviceProperty_RecordingState,				"RecordingState"				},
	{	CrDeviceProperty_LiveViewStatus,				"LiveViewStatus"				},
	{	CrDeviceProperty_FocusIndication,				"FocusIndication"				},
	{	CrDeviceProperty_MediaSLOT1_Status,				"MediaSLOT1_Status"				},
	{	CrDeviceProperty_MediaSLOT1_RemainingNumber,	"MediaSLOT1_RemainingNumber"	},
	{	CrDeviceProperty_MediaSLOT1_RemainingTime,		"MediaSLOT1_RemainingTime"		},
	{	CrDeviceProperty_MediaSLOT1_FormatEnableStatus,	"MediaSLOT1_FormatEnableStatus"	},
	{	CrDeviceProperty_reserved20,					"reserved20"					},
	{	CrDeviceProperty_MediaSLOT2_Status,				"MediaSLOT2_Status"				},
	{	CrDeviceProperty_MediaSLOT2_FormatEnableStatus,	"MediaSLOT2_FormatEnableStatus"	},
	{	CrDeviceProperty_MediaSLOT2_RemainingNumber,	"MediaSLOT2_RemainingNumber"	},
	{	CrDeviceProperty_MediaSLOT2_RemainingTime,		"MediaSLOT2_RemainingTime"		},
	{	CrDeviceProperty_reserved22,					"reserved22"					},
	{	CrDeviceProperty_Media_FormatProgressRate,		"Media_FormatProgressRate"		},
	{	CrDeviceProperty_reserved24,					"reserved24"					},
	{	CrDeviceProperty_reserved25,					"reserved25"					},
	{	CrDeviceProperty_LiveView_Area,					"LiveView_Area"					},
	{	CrDeviceProperty_reserved26,					"reserved26"					},
	{	CrDeviceProperty_reserved27,					"reserved27"					},
	{	CrDeviceProperty_Interval_Rec_Status,			"Interval_Rec_Status"			},
	{	CrDeviceProperty_CustomWB_Execution_State,		"CustomWB_Execution_State"		},
	{	CrDeviceProperty_CustomWB_Capturable_Area,		"CustomWB_Capturable_Area"		},
	{	CrDeviceProperty_CustomWB_Capture_Frame_Size,	"CustomWB_Capture_Frame_Size"	},
	{	CrDeviceProperty_CustomWB_Capture_Operation,	"CustomWB_Capture_Operation"	},
	{	CrDeviceProperty_FocalPosition,					"FocalPosition"					},
	{	CrDeviceProperty_Zoom_Operation_Status,			"Zoom_Operation_Status"			},
	{	CrDeviceProperty_Zoom_Bar_Information,			"Zoom_Bar_Information"			},
	{	CrDeviceProperty_Zoom_Type_Status,				"Zoom_Type_Status"				},
	{	CrDeviceProperty_MaxVal,						"MaxVal"						},
	{	0,			""	},
};

//*****************************************************************************
static const TYPE_SONY_STRINGS gSonyRecordingSettingsNames[]	=
{
	{	CrRecordingSettingMovie_60p_50M,					"60p_50M"			},
	{	CrRecordingSettingMovie_30p_50M,					"30p_50M"			},
	{	CrRecordingSettingMovie_24p_50M,					"24p_50M"			},
	{	CrRecordingSettingMovie_50p_50M,					"50p_50M"			},
	{	CrRecordingSettingMovie_25p_50M,					"25p_50M"			},
	{	CrRecordingSettingMovie_60i_24M,					"60i_24M"			},
	{	CrRecordingSettingMovie_50i_24M_FX,					"50i_24M_FX"		},
	{	CrRecordingSettingMovie_60i_17M_FH,					"60i_17M_FH"		},
	{	CrRecordingSettingMovie_50i_17M_FH,					"50i_17M_FH"		},
	{	CrRecordingSettingMovie_60p_28M_PS,					"60p_28M_PS"		},
	{	CrRecordingSettingMovie_50p_28M_PS,					"50p_28M_PS"		},
	{	CrRecordingSettingMovie_24p_24M_FX,					"24p_24M_FX"		},
	{	CrRecordingSettingMovie_25p_24M_FX,					"25p_24M_FX"		},
	{	CrRecordingSettingMovie_24p_17M_FH,					"24p_17M_FH"		},
	{	CrRecordingSettingMovie_25p_17M_FH,					"25p_17M_FH"		},
	{	CrRecordingSettingMovie_120p_50M_1280x720,			"120p_50M_1280x720"	},
	{	CrRecordingSettingMovie_100p_50M_1280x720,			"100p_50M_1280x720"	},
	{	CrRecordingSettingMovie_1920x1080_30p_16M,			"1920x1080_30p_16M"	},
	{	CrRecordingSettingMovie_1920x1080_25p_16M,			"1920x1080_25p_16M"	},
	{	CrRecordingSettingMovie_1280x720_30p_6M,			"1280x720_30p_6M"	},
	{	CrRecordingSettingMovie_1280x720_25p_6M,			"1280x720_25p_6M"	},
	{	CrRecordingSettingMovie_1920x1080_60p_28M,			"1920x1080_60p_28M"	},
	{	CrRecordingSettingMovie_1920x1080_50p_28M,			"1920x1080_50p_28M"	},
	{	CrRecordingSettingMovie_60p_25M_XAVC_S_HD,			"60p_25M_XAVC_S_HD"	},
	{	CrRecordingSettingMovie_50p_25M_XAVC_S_HD,			"50p_25M_XAVC_S_HD"	},
	{	CrRecordingSettingMovie_30p_16M_XAVC_S_HD,			"30p_16M_XAVC_S_HD"	},
	{	CrRecordingSettingMovie_25p_16M_XAVC_S_HD,			"25p_16M_XAVC_S_HD"	},
	{	CrRecordingSettingMovie_120p_100M_1920x1080_XAVC_S_HD,	"120p_100M_1920x1080_XAVC_S_HD"	},
	{	CrRecordingSettingMovie_100p_100M_1920x1080_XAVC_S_HD,	"100p_100M_1920x1080_XAVC_S_HD"	},
	{	CrRecordingSettingMovie_120p_60M_1920x1080_XAVC_S_HD,	"120p_60M_1920x1080_XAVC_S_HD"	},
	{	CrRecordingSettingMovie_100p_60M_1920x1080_XAVC_S_HD,	"100p_60M_1920x1080_XAVC_S_HD"	},
	{	CrRecordingSettingMovie_30p_100M_XAVC_S_4K,				"30p_100M_XAVC_S_4K"			},
	{	CrRecordingSettingMovie_25p_100M_XAVC_S_4K,				"25p_100M_XAVC_S_4K"			},
	{	CrRecordingSettingMovie_24p_100M_XAVC_S_4K,				"24p_100M_XAVC_S_4K"			},
	{	CrRecordingSettingMovie_30p_60M_XAVC_S_4K,				"30p_60M_XAVC_S_4K"				},
	{	CrRecordingSettingMovie_25p_60M_XAVC_S_4K,				"25p_60M_XAVC_S_4K"				},
	{	CrRecordingSettingMovie_24p_60M_XAVC_S_4K,				"24p_60M_XAVC_S_4K"				},
	{	0,			""	}
};


//*****************************************************************************
static const TYPE_SONY_STRINGS gSonyWhiteBalanceSettings[]	=
{
	{	CrWhiteBalance_AWB,						"AWB"					},
	{	CrWhiteBalance_Underwater_Auto,			"Underwater_Auto"		},
	{	CrWhiteBalance_Daylight,				"Daylight"				},
	{	CrWhiteBalance_Shadow,					"Shadow"				},
	{	CrWhiteBalance_Cloudy,					"Cloudy"				},
	{	CrWhiteBalance_Tungsten,				"Tungsten"				},
	{	CrWhiteBalance_Fluorescent,				"Fluorescent"			},
	{	CrWhiteBalance_Fluorescent_WarmWhite,	"Fluorescent_WarmWhite"	},
	{	CrWhiteBalance_Fluorescent_CoolWhite,	"Fluorescent_CoolWhite"	},
	{	CrWhiteBalance_Fluorescent_DayWhite,	"Fluorescent_DayWhite"	},
	{	CrWhiteBalance_Fluorescent_Daylight,	"Fluorescent_Daylight"	},
	{	CrWhiteBalance_Flush,					"Flush"					},
	{	CrWhiteBalance_ColorTemp,				"ColorTemp"				},
	{	CrWhiteBalance_Custom_1,				"Custom_1"				},
	{	CrWhiteBalance_Custom_2,				"Custom_2"				},
	{	CrWhiteBalance_Custom_3,				"Custom_3"				},
	{	CrWhiteBalance_Custom,					"Custom"				},
	{	0,			""	}
};


//*****************************************************************************
static const TYPE_SONY_STRINGS gSonyPictureEffects[]	=
{
	{	CrPictureEffect_Off,					"Off"					},
	{	CrPictureEffect_ToyCameraNormal,		"ToyCameraNormal"		},
	{	CrPictureEffect_ToyCameraCool,			"ToyCameraCool"			},
	{	CrPictureEffect_ToyCameraWarm,			"ToyCameraWarm"			},
	{	CrPictureEffect_ToyCameraGreen,			"ToyCameraGreen"		},
	{	CrPictureEffect_ToyCameraMagenta,		"ToyCameraMagenta"		},
	{	CrPictureEffect_Pop,					"Pop"					},
	{	CrPictureEffect_PosterizationBW,		"PosterizationBW"		},
	{	CrPictureEffect_PosterizationColor,		"PosterizationColor"	},
	{	CrPictureEffect_Retro,					"Retro"					},
	{	CrPictureEffect_SoftHighkey,			"SoftHighkey"			},
	{	CrPictureEffect_PartColorRed,			"PartColorRed"			},
	{	CrPictureEffect_PartColorGreen,			"PartColorGreen"		},
	{	CrPictureEffect_PartColorBlue,			"PartColorBlue"			},
	{	CrPictureEffect_PartColorYellow,		"PartColorYellow"		},
	{	CrPictureEffect_HighContrastMonochrome,	"HighContrastMonochrome"	},
	{	CrPictureEffect_SoftFocusLow,			"SoftFocusLow"			},
	{	CrPictureEffect_SoftFocusMid,			"SoftFocusMid"			},
	{	CrPictureEffect_SoftFocusHigh,			"SoftFocusHigh"			},
	{	CrPictureEffect_HDRPaintingLow,			"HDRPaintingLow"		},
	{	CrPictureEffect_HDRPaintingMid,			"HDRPaintingMid"		},
	{	CrPictureEffect_HDRPaintingHigh,		"HDRPaintingHigh"		},
	{	CrPictureEffect_RichToneMonochrome,		"RichToneMonochrome"	},
	{	CrPictureEffect_MiniatureAuto,			"MiniatureAuto"			},
	{	CrPictureEffect_MiniatureTop,			"MiniatureTop"			},
	{	CrPictureEffect_MiniatureMidHorizontal,	"MiniatureMidHorizontal"	},
	{	CrPictureEffect_MiniatureBottom,		"MiniatureBottom"		},
	{	CrPictureEffect_MiniatureLeft,			"MiniatureLeft"			},
	{	CrPictureEffect_MiniatureMidVertical,	"MiniatureMidVertical"	},
	{	CrPictureEffect_MiniatureRight,			"MiniatureRight"		},
	{	CrPictureEffect_MiniatureWaterColor,	"MiniatureWaterColor"	},
	{	CrPictureEffect_MiniatureIllustrationLow,	"MiniatureIllustrationLow"	},
	{	CrPictureEffect_MiniatureIllustrationMid,	"MiniatureIllustrationMid"	},
	{	CrPictureEffect_MiniatureIllustrationHigh,	"MiniatureIllustrationHigh"	},
	{	0,			""	}
};

//*****************************************************************************
static const TYPE_SONY_STRINGS gSonyDriveModes[]	=
{
	{	CrDrive_Single,							"Single"					},
	{	CrDrive_Continuous_Hi,					"Continuous_Hi"				},
	{	CrDrive_Continuous_Hi_Plus,				"Continuous_Hi_Plus"		},
	{	CrDrive_Continuous_Hi_Live,				"Continuous_Hi_Live"		},
	{	CrDrive_Continuous_Lo,					"Continuous_Lo"				},
	{	CrDrive_Continuous,						"Continuous"				},
	{	CrDrive_Continuous_SpeedPriority,		"Continuous_SpeedPriority"	},
	{	CrDrive_Continuous_Mid,					"Continuous_Mid"			},
	{	CrDrive_Continuous_Mid_Live,			"Continuous_Mid_Live"		},
	{	CrDrive_Continuous_Lo_Live,				"Continuous_Lo_Live"		},
	{	CrDrive_SingleBurstShooting_lo,			"SingleBurstShooting_lo"	},
	{	CrDrive_SingleBurstShooting_mid,		"SingleBurstShooting_mid"	},
	{	CrDrive_SingleBurstShooting_hi,			"SingleBurstShooting_hi"	},
	{	CrDrive_Timelapse,						"Timelapse"					},
	{	CrDrive_Timer_2s,						"Timer_2s"					},
	{	CrDrive_Timer_5s,						"Timer_5s"					},
	{	CrDrive_Timer_10s,						"Timer_10s"					},
	{	CrDrive_Continuous_Bracket_03Ev_3pics,	"Continuous_Bracket_03Ev_3pics"	},
	{	CrDrive_Continuous_Bracket_03Ev_5pics,	"Continuous_Bracket_03Ev_5pics"	},
	{	CrDrive_Continuous_Bracket_03Ev_9pics,	"Continuous_Bracket_03Ev_9pics"	},
	{	CrDrive_Continuous_Bracket_05Ev_3pics,	"Continuous_Bracket_05Ev_3pics"	},
	{	CrDrive_Continuous_Bracket_05Ev_5pics,	"Continuous_Bracket_05Ev_5pics"	},
	{	CrDrive_Continuous_Bracket_05Ev_9pics,	"Continuous_Bracket_05Ev_9pics"	},
	{	CrDrive_Continuous_Bracket_07Ev_3pics,	"Continuous_Bracket_07Ev_3pics"	},
	{	CrDrive_Continuous_Bracket_07Ev_5pics,	"Continuous_Bracket_07Ev_5pics"	},
	{	CrDrive_Continuous_Bracket_07Ev_9pics,	"Continuous_Bracket_07Ev_9pics"	},
	{	CrDrive_Continuous_Bracket_10Ev_3pics,	"Continuous_Bracket_10Ev_3pics"	},
	{	CrDrive_Continuous_Bracket_10Ev_5pics,	"Continuous_Bracket_10Ev_5pics"	},
	{	CrDrive_Continuous_Bracket_10Ev_9pics,	"Continuous_Bracket_10Ev_9pics"	},
	{	CrDrive_Continuous_Bracket_20Ev_3pics,	"Continuous_Bracket_20Ev_3pics"	},
	{	CrDrive_Continuous_Bracket_20Ev_5pics,	"Continuous_Bracket_20Ev_5pics"	},
	{	CrDrive_Continuous_Bracket_30Ev_3pics,	"Continuous_Bracket_30Ev_3pics"	},
	{	CrDrive_Continuous_Bracket_30Ev_5pics,	"Continuous_Bracket_30Ev_5pics"	},
	{	CrDrive_Single_Bracket_03Ev_3pics,		"Single_Bracket_03Ev_3pics"	},
	{	CrDrive_Single_Bracket_03Ev_5pics,		"Single_Bracket_03Ev_5pics"	},
	{	CrDrive_Single_Bracket_03Ev_9pics,		"Single_Bracket_03Ev_9pics"	},
	{	CrDrive_Single_Bracket_05Ev_3pics,		"Single_Bracket_05Ev_3pics"	},
	{	CrDrive_Single_Bracket_05Ev_5pics,		"Single_Bracket_05Ev_5pics"	},
	{	CrDrive_Single_Bracket_05Ev_9pics,		"Single_Bracket_05Ev_9pics"	},
	{	CrDrive_Single_Bracket_07Ev_3pics,		"Single_Bracket_07Ev_3pics"	},
	{	CrDrive_Single_Bracket_07Ev_5pics,		"Single_Bracket_07Ev_5pics"	},
	{	CrDrive_Single_Bracket_07Ev_9pics,		"Single_Bracket_07Ev_9pics"	},
	{	CrDrive_Single_Bracket_10Ev_3pics,		"Single_Bracket_10Ev_3pics"	},
	{	CrDrive_Single_Bracket_10Ev_5pics,		"Single_Bracket_10Ev_5pics"	},
	{	CrDrive_Single_Bracket_10Ev_9pics,		"Single_Bracket_10Ev_9pics"	},
	{	CrDrive_Single_Bracket_20Ev_3pics,		"Single_Bracket_20Ev_3pics"	},
	{	CrDrive_Single_Bracket_20Ev_5pics,		"Single_Bracket_20Ev_5pics"	},
	{	CrDrive_Single_Bracket_30Ev_3pics,		"Single_Bracket_30Ev_3pics"	},
	{	CrDrive_Single_Bracket_30Ev_5pics,		"Single_Bracket_30Ev_5pics"	},
	{	CrDrive_WB_Bracket_Lo,					"WB_Bracket_Lo"				},
	{	CrDrive_WB_Bracket_Hi,					"WB_Bracket_Hi"				},
	{	CrDrive_DRO_Bracket_Lo,					"DRO_Bracket_Lo"			},
	{	CrDrive_DRO_Bracket_Hi,					"DRO_Bracket_Hi"			},
	{	CrDrive_Continuous_Timer_3pics,			"Continuous_Timer_3pics"	},
	{	CrDrive_Continuous_Timer_5pics,			"Continuous_Timer_5pics"	},
	{	CrDrive_Continuous_Timer_2s_3pics,		"Continuous_Timer_2s_3pics"	},
	{	CrDrive_Continuous_Timer_2s_5pics,		"Continuous_Timer_2s_5pics"	},
	{	CrDrive_Continuous_Timer_5s_3pics,		"Continuous_Timer_5s_3pics"	},
	{	CrDrive_Continuous_Timer_5s_5pics,		"Continuous_Timer_5s_5pics"	},
	{	CrDrive_LPF_Bracket,					"LPF_Bracket"				},
	{	CrDrive_RemoteCommander,				"RemoteCommander"			},
	{	CrDrive_MirrorUp,						"MirrorUp"					},
	{	CrDrive_SelfPortrait_1,					"SelfPortrait_1"			},
	{	CrDrive_SelfPortrait_2,					"SelfPortrait_2"			},
	{	0,			""	}
};


//*****************************************************************************
static const TYPE_SONY_STRINGS gSonyDROstrings[]	=
{
	{	CrDRangeOptimizer_Off,				"Off"			},
	{	CrDRangeOptimizer_On,				"On"			},
	{	CrDRangeOptimizer_Plus,				"Plus"			},
	{	CrDRangeOptimizer_Plus_Manual_1,	"Plus_Manual_1"	},
	{	CrDRangeOptimizer_Plus_Manual_2,	"Plus_Manual_2"	},
	{	CrDRangeOptimizer_Plus_Manual_3,	"Plus_Manual_3"	},
	{	CrDRangeOptimizer_Plus_Manual_4,	"Plus_Manual_4"	},
	{	CrDRangeOptimizer_Plus_Manual_5,	"Plus_Manual_5"	},
	{	CrDRangeOptimizer_Auto,				"Auto"			},
	{	CrDRangeOptimizer_HDR_Auto,			"HDR_Auto"		},
	{	CrDRangeOptimizer_HDR_10Ev,			"HDR_10Ev"		},
	{	CrDRangeOptimizer_HDR_20Ev,			"HDR_20Ev"		},
	{	CrDRangeOptimizer_HDR_30Ev,			"HDR_30Ev"		},
	{	CrDRangeOptimizer_HDR_40Ev,			"HDR_40Ev"		},
	{	CrDRangeOptimizer_HDR_50Ev,			"HDR_50Ev"		},
	{	CrDRangeOptimizer_HDR_60Ev,			"HDR_60Ev"		},
	{	0,			""	}
};


//*****************************************************************************
static bool	FindSonyString(uint32_t stringCode, const TYPE_SONY_STRINGS *stringTable, char *returnString)
{
int		iii;
bool	foundItFlag;

	iii			=	0;
	foundItFlag	=	false;
	while ((stringTable[iii].sonyStringCode != 0) || (strlen(stringTable[iii].sonyMsgString) > 0))
	{
		if (stringCode == stringTable[iii].sonyStringCode)
		{
			strcpy(returnString, stringTable[iii].sonyMsgString);
			foundItFlag	=	true;
			break;
		}
		iii++;
	}
	if (foundItFlag == false)
	{
		sprintf(returnString, "%X - String code not found in table", stringCode);
	}
	return(foundItFlag);
}

//*****************************************************************************
static void	GetSonyErrorString(CrError sonyErrCode, char *sonyErrorString)
{
	FindSonyString(sonyErrCode, gSonyErrMsgs, sonyErrorString);
}

//*****************************************************************************
static void	GetSonyPropertyString(uint32_t sonyPropCode, char *sonyPropertyString)
{
	FindSonyString(sonyPropCode, gSonyPropertyNames, sonyPropertyString);
}

//*****************************************************************************
static void	GetSonyExposureMode(uint32_t sonyExposureMode, char *exposureModeString)
{
	FindSonyString(sonyExposureMode, gExposureModeStrings, exposureModeString);
}

//*****************************************************************************
static void	GetSonyRecordingSetting(uint32_t sonyRecordingSetting, char *recordingSettingString)
{
	FindSonyString(sonyRecordingSetting, gSonyRecordingSettingsNames, recordingSettingString);
}

//*****************************************************************************
static void	GetSonyWhiteBlanceString(CrError whateBalanceCode, char *whiteBalancceString)
{
	FindSonyString(whateBalanceCode, gSonyWhiteBalanceSettings, whiteBalancceString);
}


//*****************************************************************************
static void	GetSonyPictureEffectsString(CrError pictureEffectsCode, char *pictureEffectsString)
{
	FindSonyString(pictureEffectsCode, gSonyPictureEffects, pictureEffectsString);
}

//*****************************************************************************
static void	GetSonyDriveModeString(CrError driveModeCode, char *driveModeString)
{
	FindSonyString(driveModeCode, gSonyDriveModes, driveModeString);
}

//*****************************************************************************
static void	GetSonyDROmodeString(CrError droMode, char *droModeString)
{
	FindSonyString(droMode, gSonyDROstrings, droModeString);
}



#ifdef	_INCLUDE_SONY_MAIN_

//**************************************************************************************
CameraDriver::CameraDriver(void)
{
}
//**************************************************************************************
CameraDriver::~CameraDriver(void)
{
}
//**************************************************************************************
void CameraDriver::GenerateFileNameRoot(void)
{
	strcpy(cFileNameRoot, "SonyImage");
}

//*****************************************************************************
static void	PrintMenuItem(const char theChar, const char *theDescription)
{
	printf("\t%c\t%s\r\n", theChar, theDescription);
}

//*****************************************************************************
static void	PrintMenu(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	PrintMenuItem('e',	"Exposure");
	PrintMenuItem('h',	"Help");
	PrintMenuItem('l',	"Live view");
	PrintMenuItem('p',	"Print Property list");
	PrintMenuItem('x',	"eXit");
}

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//*****************************************************************************
void segfault_sigaction(int signal, siginfo_t *si, void *arg)
{
	printf("Caught segfault at address %p\n", si->si_addr);
	LogFunctionCall(__FUNCTION__, __LINE__,	"Segfault", 	0, "Seg fault occurred");

	CONSOLE_ABORT(__FUNCTION__);
}


//*****************************************************************************
int	main(int argc, char **argv)
{
bool				keepRunning;
int					loopCntr;
char				theChar;
struct sigaction	signalAction;

	CONSOLE_DEBUG("Sony camera test program");

	memset(&signalAction, 0, sizeof(struct sigaction));
	sigemptyset(&signalAction.sa_mask);
	signalAction.sa_sigaction	=	segfault_sigaction;
	signalAction.sa_flags		=	SA_SIGINFO;

	sigaction(SIGSEGV, &signalAction, NULL);

	CreateSONY_CameraObjects();

	if (gSonyCameraObj != NULL)
	{
		gSonyCameraObj->RunStateMachine_Device();

//		gSonyCameraObj->GetLiveView();
//		sleep(1);

//		gSonyCameraObj->Start_CameraExposure(32);
//		sleep(1);

		keepRunning	=	true;
		loopCntr	=	0;

		CONSOLE_DEBUG(__FUNCTION__);
		PrintMenu();
		theChar	=	0;
		while (keepRunning)
		{
			gSonyCameraObj->RunStateMachine_Device();

			printf("\rSony>");
			fflush(stdout);

			theChar	=	fgetc(stdin);

			switch(tolower(theChar))
			{
				case 'e':
					gSonyCameraObj->Start_CameraExposure(32);
					break;

				case 'h':
					PrintMenu();
					break;

				case 'l':
					gSonyCameraObj->GetLiveView();
					break;

				case 'p':
					gSonyCameraObj->ReadProperties();
					break;

				case 'x':
					keepRunning	=	false;
					break;
			}
		}
		CONSOLE_DEBUG("deleting sony camera object");
		delete gSonyCameraObj;
	}
}
#endif // _INCLUDE_SONY_MAIN_

#endif // _ENABLE_SONY_

