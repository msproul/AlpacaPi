//**************************************************************************
//*	Name:			cameradriver_FLIR.cpp
//*
//*	Author:			Mark Sproul (C) 2020
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
//*
//*	Usage notes:
//*
//*	References:
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar 30,	2020	<MLS> Created cameradriver_FLIR.cpp
//*	Apr  3,	2020	<MLS> FLIR C++ examples wont compile with GCC 5
//*	Apr 22,	2020	<MLS> Switched over to use FLIR C example
//*	Apr 23,	2020	<MLS> Finally can read image data from FLIR camera
//*	Apr 23,	2020	<MLS> SUPPORTED: FLIR cameras
//*	Apr 29,	2020	<MLS> Added image processing
//*	Feb 13,	2021	<MLS> Added ExtractColorImage()
//*	Feb 13,	2021	<MLS> Working on FLIR color image support
//*	Feb 25,	2021	<MLS> Added CheckUSBFS_memory()
//*	Apr 14,	2021	<MLS> Working on FLIR single frame mode
//*	Apr 14,	2021	<MLS> Added _FLIR_SINGLE_FRAME_
//*	Jun 30,	2022	<MLS> Added GetSpinnakerErrorString()
//*	Jun 30,	2022	<MLS> Added InitFlirCamera()
//*****************************************************************************

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_FLIR_)
#include	<stdio.h>
#include	<string.h>

#define	_FLIR_SINGLE_FRAME_

#include	<spinnaker/spinc/SpinnakerC.h>
//#include	<spinnaker/Camera.h>
#include	<spinnaker/spinc/QuickSpinDefsC.h>
#include	<spinnaker/spinc/CameraDefsC.h>
//#include	"FLIR-SDK/spinnaker/spinc/QuickSpinC.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"eventlogging.h"
#include	"cameradriver_FLIR.h"

char	gSpinakerVerString[64]	=	"unknown";

static	spinSystem			gSpinSystemHandle;
static	spinLibraryVersion	gLibraryVersionHandle;
static	spinCameraList		gCameraList;

static	void	CheckUSBFS_memory(void);
static	void	GetSpinnakerErrorString(_spinError errorCode, char *errorString);


//**************************************************************************************
int		CreateFLIR_CameraObjects(void)
{
int				cameraCreatedCount;
char			rulesFileName[]	=	"40-flir-spinnaker.rules";
bool			rulesFileOK;
unsigned int	iii;
size_t			numCameras;
//spinError		errReturn;
spinError		spinErr;

	CONSOLE_DEBUG(__FUNCTION__);
	cameraCreatedCount	=	0;
	numCameras			=	0;

	rulesFileOK	=	Check_udev_rulesFile(rulesFileName);
	if (rulesFileOK)
	{
		CONSOLE_DEBUG_W_STR("Rules file is OK:", rulesFileName);
	}
	else
	{
		LogEvent(	"camera",
					"Problem with FLIR rules",
					NULL,
					kASCOM_Err_Success,
					rulesFileName);
	}

	spinErr	=	spinSystemGetInstance(&gSpinSystemHandle);
	if (spinErr == SPINNAKER_ERR_SUCCESS)
	{
		// Print out current library version

		spinSystemGetLibraryVersion(gSpinSystemHandle, &gLibraryVersionHandle);

		sprintf(gSpinakerVerString,	"%d.%d.%d.%d",
									gLibraryVersionHandle.major,
									gLibraryVersionHandle.minor,
									gLibraryVersionHandle.type,
									gLibraryVersionHandle.build);

		LogEvent(	"camera",
					"Library version (FLIR)",
					NULL,
					kASCOM_Err_Success,
					gSpinakerVerString);

		AddLibraryVersion("camera", "FLIR", gSpinakerVerString);
		CONSOLE_DEBUG_W_STR("Spinnaker library version:", gSpinakerVerString);


		// Retrieve list of cameras from the system
		spinErr	=	spinCameraListCreateEmpty(&gCameraList);
		if (spinErr == SPINNAKER_ERR_SUCCESS)
		{
			spinErr	=	spinSystemGetCameras(gSpinSystemHandle, gCameraList);
			if (spinErr == SPINNAKER_ERR_SUCCESS)
			{

				// Retrieve number of cameras
				spinErr		=	spinCameraListGetSize(gCameraList, &numCameras);
				if (spinErr == SPINNAKER_ERR_SUCCESS)
				{
					CONSOLE_DEBUG_W_LONG("Number of cameras detected:", numCameras);
					for (iii = 0; iii<numCameras; iii++)
					{
					spinCamera hCamera	=	NULL;

						spinErr	=	spinCameraListGet(gCameraList, iii, &hCamera);
						if (spinErr == SPINNAKER_ERR_SUCCESS)
						{
							new CameraDriverFLIR(hCamera);
							cameraCreatedCount++;
						}
						else
						{
							printf("Unable to retrieve camera from list. Aborting with error %d...\n\n", spinErr);
						}
					}
				}
				else
				{
					printf("Unable to retrieve number of cameras. Aborting with error %d...\n\n", spinErr);
				}
			}
			else
			{
				printf("Unable to retrieve camera list. Aborting with error %d...\n\n", spinErr);
			}
		}
		else
		{
			printf("Unable to create camera list. Aborting with error %d...\n\n", spinErr);
		}
	}
	else
	{
		printf("Unable to retrieve system instance. Aborting with error %d...\n\n", spinErr);
	}

	//------------------------------------------------------------
	//*	check the usbfs_memory_mb system parameter
	CheckUSBFS_memory();


	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
	return(cameraCreatedCount);
}

//**************************************************************************************
//	cat /sys/module/usbcore/parameters/usbfs_memory_mb
//**************************************************************************************
static void	CheckUSBFS_memory(void)
{
FILE			*filePointer;
char			lineBuff[256];
int				slen;
char			fileName[]	=	"/sys/module/usbcore/parameters/usbfs_memory_mb";
int				usbfs_memory_mb;
char			*charPtr;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	check for the observatory settings file
	filePointer	=	fopen(fileName, "r");
	if (filePointer != NULL)
	{
		lineBuff[0]	=	0;
		charPtr	=	fgets(lineBuff, 200, filePointer);
		if (charPtr != NULL)
		{
			slen	=	strlen(lineBuff);
			if (slen > 0)
			{
				usbfs_memory_mb	=	atoi(lineBuff);
	//			CONSOLE_DEBUG_W_NUM("usbfs_memory_mb\t=", usbfs_memory_mb);
				if (usbfs_memory_mb < 1000)
				{
					sprintf(lineBuff, "usbfs_memory_mb=%d", usbfs_memory_mb);
					LogEvent(	"camera-FLIR",
								lineBuff,
								NULL,
								kASCOM_Err_Success,
								"Should be >= 1000");
				}
			}
		}
		else
		{
			CONSOLE_DEBUG_W_STR("Failed to open ubfs memory file\t=", fileName);
		}
		fclose(filePointer);
	}
}

//**************************************************************************************
CameraDriverFLIR::CameraDriverFLIR(spinCamera hCamera)
	:CameraDriver()
{
spinError			spinErr;
quickSpin			quickSpinStruct;

	CONSOLE_DEBUG(__FUNCTION__);
//	cCameraID		=	deviceNum;

//	cVerboseDebug	=	true;

	strcpy(cDeviceManufAbrev,		"FLIR");
	strcpy(cDeviceManufacturer,		"FLIR");
	strcpy(cCommonProp.Description,	"FLIR Camera");
	strcpy(cDeviceVersion,			gSpinakerVerString);
	strcpy(cCommonProp.Name,		"FLIR");

	//*	we have to have something here
	cCameraProp.CameraXsize	=	4240;
	cCameraProp.CameraYsize	=	2824;
	cCameraProp.PixelSizeX	=	3.1;		//*	https://www.flir.com/products/grasshopper3-usb3/?model=GS3-U3-120S6M-C
	cCameraProp.PixelSizeY	=	3.1;
	cCameraProp.NumX		=	cCameraProp.CameraXsize;
	cCameraProp.NumY		=	cCameraProp.CameraYsize;

	cSpinCameraHandle		=	hCamera;
	cSpinImageHandle		=	NULL;
	cSpinNodeMapHandle		=	NULL;
	cSpinNodeMapTLDeviceH	=	NULL;

//	quickSpinInit(cSpinCameraHandle, &quickSpinStruct);
//
//	CONSOLE_DEBUG_W_LONG("quickSpinStruct.Width", (long)quickSpinStruct.Width);
//	CONSOLE_DEBUG_W_SIZE("sizeof(quickSpin)", sizeof(quickSpin));
//
//	ReadFLIRcameraInfo();

	// Initialize camera
	InitFlirCamera();


	cUUID.part5					=	atoi(cDeviceSerialNum);			//*	serial number


	strcpy(cCommonProp.Description, cDeviceManufacturer);
	strcat(cCommonProp.Description, " spinnaker Version");
	strcat(cCommonProp.Description, gSpinakerVerString);

	SetCommonPropertyName("FLIR-", cDeviceModel);


	DumpCameraProperties(__FUNCTION__);

#ifdef _USE_OPENCV_
	sprintf(cOpenCV_ImgWindowName, "%s-%d", cCommonProp.Name, cCameraID);
#endif // _USE_OPENCV_

}


//**************************************************************************************
// Destructor
//**************************************************************************************
CameraDriverFLIR::~CameraDriverFLIR(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//**************************************************************************************
void CameraDriverFLIR::InitFlirCamera(void)
{
spinError			spinErr;
quickSpin			quickSpinStruct;
char				spinErrorString[64];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "------------------------");
	quickSpinInit(cSpinCameraHandle, &quickSpinStruct);

	CONSOLE_DEBUG_W_LONG("quickSpinStruct.Width",	(long)quickSpinStruct.Width);
	CONSOLE_DEBUG_W_SIZE("sizeof(quickSpin)",		sizeof(quickSpin));

	ReadFLIRcameraInfo();

	if (cSpinCameraHandle != NULL)
	{
		spinErr	=	spinCameraInit(cSpinCameraHandle);
		if (spinErr == SPINNAKER_ERR_SUCCESS)
		{
		}
		else
		{
			GetSpinnakerErrorString(spinErr, spinErrorString);
			CONSOLE_DEBUG_W_STR("Unable to initialize camera. Aborting with error=", spinErrorString);
		}
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
}


// This macro helps with C-strings.
#define MAX_BUFF_LEN 256

//**************************************************************************************
// This function helps to check if a node is available and readable
bool8_t IsAvailableAndReadable(spinNodeHandle hNode, const char nodeName[])
{
bool8_t		pbAvailable	=	False;
bool8_t		pbReadable	=	False;
spinError	spinErr		=	SPINNAKER_ERR_SUCCESS;

	spinErr		=	spinNodeIsAvailable(hNode, &pbAvailable);
	if (spinErr != SPINNAKER_ERR_SUCCESS)
	{
		printf("Unable to retrieve node availability (%s node), with error %d...\n\n", nodeName, spinErr);
	}

	spinErr	=	spinNodeIsReadable(hNode, &pbReadable);
	if (spinErr != SPINNAKER_ERR_SUCCESS)
	{
		printf("Unable to retrieve node readability (%s node), with error %d...\n\n", nodeName, spinErr);
	}
	return(pbReadable && pbAvailable);
}

//**************************************************************************************
// This function helps to check if a node is available and writable
bool8_t IsAvailableAndWritable(spinNodeHandle hNode, const char nodeName[])
{
bool8_t		pbAvailable	=	False;
bool8_t		pbWritable	=	False;
spinError	spinErr		=	SPINNAKER_ERR_SUCCESS;

	spinErr	=	spinNodeIsAvailable(hNode, &pbAvailable);
	if (spinErr != SPINNAKER_ERR_SUCCESS)
	{
		printf("Unable to retrieve node availability (%s node), with error %d...\n\n", nodeName, spinErr);
	}

	spinErr	=	spinNodeIsWritable(hNode, &pbWritable);
	if (spinErr != SPINNAKER_ERR_SUCCESS)
	{
		printf("Unable to retrieve node writability (%s node), with error %d...\n\n", nodeName, spinErr);
	}
	return(pbWritable && pbAvailable);
}

//**************************************************************************************
// This function handles the error prints when a node or entry is unavailable or
// not readable/writable on the connected camera
//**************************************************************************************
void PrintRetrieveNodeFailure(const char node[], const char name[])
{
	printf("Unable to get %s (%s %s retrieval failed).\n\n", node, name, node);
}


//**************************************************************************************
// This function prints the device information of the camera from the transport
// layer; please see NodeMapInfo_C example for more in-depth comments on
// printing device information from the nodemap.
//**************************************************************************************
spinError PrintDeviceInfo(spinNodeMapHandle hNodeMap)
{
spinError err	=	SPINNAKER_ERR_SUCCESS;
unsigned int i	=	0;

	printf("\n*** DEVICE INFORMATION ***\n\n");

	// Retrieve device information category node
	spinNodeHandle hDeviceInformation	=	NULL;

	err	=	spinNodeMapGetNode(hNodeMap, "DeviceInformation", &hDeviceInformation);
	if (err != SPINNAKER_ERR_SUCCESS)
	{
		printf("Unable to retrieve node. Non-fatal error %d...\n\n", err);
		return err;
	}

	// Retrieve number of nodes within device information node
	size_t numFeatures	=	0;

	if (IsAvailableAndReadable(hDeviceInformation, "DeviceInformation"))
	{
		err	=	spinCategoryGetNumFeatures(hDeviceInformation, &numFeatures);
		if (err != SPINNAKER_ERR_SUCCESS)
		{
			printf("Unable to retrieve number of nodes. Non-fatal error %d...\n\n", err);
			return err;
		}
	}
	else
	{
		PrintRetrieveNodeFailure("node", "DeviceInformation");
		return SPINNAKER_ERR_ACCESS_DENIED;
	}

	// Iterate through nodes and print information
	for (i = 0; i < numFeatures; i++)
	{
		spinNodeHandle hFeatureNode	=	NULL;

		err	=	spinCategoryGetFeatureByIndex(hDeviceInformation, i, &hFeatureNode);
		if (err != SPINNAKER_ERR_SUCCESS)
		{
			printf("Unable to retrieve node. Non-fatal error %d...\n\n", err);
			continue;
		}

		spinNodeType featureType	=	UnknownNode;

		// get feature node name
		char featureName[MAX_BUFF_LEN];
		size_t lenFeatureName	=	MAX_BUFF_LEN;
		err	=	spinNodeGetName(hFeatureNode, featureName, &lenFeatureName);
		if (err != SPINNAKER_ERR_SUCCESS)
		{
			strcpy(featureName, "Unknown name");
		}

		if (IsAvailableAndReadable(hFeatureNode, featureName))
		{
			err	=	spinNodeGetType(hFeatureNode, &featureType);
			if (err != SPINNAKER_ERR_SUCCESS)
			{
				printf("Unable to retrieve node type. Non-fatal error %d...\n\n", err);
				continue;
			}
		}
		else
		{
			printf("%s: Node not readable\n", featureName);
			continue;
		}

		char featureValue[MAX_BUFF_LEN];
		size_t lenFeatureValue	=	MAX_BUFF_LEN;

		err = spinNodeToString(hFeatureNode, featureValue, &lenFeatureValue);
		if (err != SPINNAKER_ERR_SUCCESS)
		{
			strcpy(featureValue, "Unknown value");
		}

		printf("%s: %s\n", featureName, featureValue);
	}
	printf("\n");

	return err;
}


//*****************************************************************************
int	CameraDriverFLIR::ReadFLIRcameraInfo(void)
{
char				featureValue[MAX_BUFF_LEN];
char				featureName[MAX_BUFF_LEN];
spinError			spinErr;
spinNodeHandle		hDeviceInformation;
unsigned int		iii;
int					sLen;
size_t				lenFeatureName;
size_t				lenFeatureValue;
size_t				numFeatures;
spinNodeHandle		hFeatureNode;
spinNodeType		featureType;

	CONSOLE_DEBUG("-----------------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);
	// Retrieve TL device nodemap and print device information
	cSpinNodeMapTLDeviceH	=	NULL;
	hDeviceInformation		=	NULL;
	numFeatures				=	0;



	spinErr	=	spinCameraGetTLDeviceNodeMap(cSpinCameraHandle, &cSpinNodeMapTLDeviceH);
	if (spinErr == SPINNAKER_ERR_SUCCESS)
	{
		CONSOLE_DEBUG(__FUNCTION__);
//		spinErr	=	PrintDeviceInfo(cSpinNodeMapTLDeviceH);
		// Retrieve device information category node

		spinErr	=	spinNodeMapGetNode(cSpinNodeMapTLDeviceH, "DeviceInformation", &hDeviceInformation);
		if (spinErr == SPINNAKER_ERR_SUCCESS)
		{
			// Retrieve number of nodes within device information node

			if (IsAvailableAndReadable(hDeviceInformation, "DeviceInformation"))
			{
				spinErr = spinCategoryGetNumFeatures(hDeviceInformation, &numFeatures);
				if (spinErr != SPINNAKER_ERR_SUCCESS)
				{
					printf("Unable to retrieve number of nodes. Non-fatal error %d...\n\n", spinErr);
					return spinErr;
				}
			}
			else
			{
				PrintRetrieveNodeFailure("node", "DeviceInformation");
				return SPINNAKER_ERR_ACCESS_DENIED;
			}

			// Iterate through nodes and print information
			for (iii = 0; iii < numFeatures; iii++)
			{
				hFeatureNode	=	NULL;

				spinErr	=	spinCategoryGetFeatureByIndex(hDeviceInformation, iii, &hFeatureNode);
				if (spinErr != SPINNAKER_ERR_SUCCESS)
				{
					printf("Unable to retrieve node. Non-fatal error %d...\n\n", spinErr);
					continue;
				}

				// get feature node name
				featureType		=	UnknownNode;
				lenFeatureName	=	MAX_BUFF_LEN;
				spinErr			=	spinNodeGetName(hFeatureNode, featureName, &lenFeatureName);
				if (spinErr == SPINNAKER_ERR_SUCCESS)
				{
				//	CONSOLE_DEBUG_W_STR("featureName\t=", featureName);
				}
				else
				{
					strcpy(featureName, "Unknown name");
				}

				if (IsAvailableAndReadable(hFeatureNode, featureName))
				{
					spinErr	=	spinNodeGetType(hFeatureNode, &featureType);
					if (spinErr != SPINNAKER_ERR_SUCCESS)
					{
						printf("Unable to retrieve node type. Non-fatal error %d...\n\n", spinErr);
						continue;
					}
				}
				else
				{
					printf("%s: Node not readable\n", featureName);
					continue;
				}

				lenFeatureValue	=	MAX_BUFF_LEN;
				spinErr			=	spinNodeToString(hFeatureNode, featureValue, &lenFeatureValue);
				if (spinErr != SPINNAKER_ERR_SUCCESS)
				{
					strcpy(featureValue, "Unknown value");
				}

		//		CONSOLE_DEBUG_W_2STR("Feature:", featureName, featureValue);
				//==========================================================
				//*	extract the information we care about
				if (strcasecmp(featureName, "DeviceSerialNumber") == 0)
				{
					strcpy(cDeviceSerialNum,	featureValue);
				}
				else if (strcasecmp(featureName, "DeviceModelName") == 0)
				{
					strcpy(cDeviceModel,		featureValue);
					sLen	=	strlen(cDeviceModel);
					if (cDeviceModel[sLen - 1] == 'C')
					{
						cIsColorCam		=	true;
						AddReadoutModeToList(kImageType_RGB24);
					}
					else
					{
						AddReadoutModeToList(kImageType_RAW8);
					}
				}
				else if (strcasecmp(featureName, "DeviceVersion") == 0)
				{
					strcpy(cDeviceVersion,		featureValue);
				}
				printf("%s: %s\n", featureName, featureValue);
			}
		}
		else
		{
			printf("Unable to retrieve node. Non-fatal error %d...\n\n", spinErr);
		}
	}
	else
	{
		printf("Unable to retrieve TL device nodemap. Non-fatal error %d...\n\n", spinErr);
	}
	return(spinErr);
}



//**************************************************************************
spinError	CameraDriverFLIR::SetFlirAqcuistionMode(int flirAcquistionMode)
{
spinError		spinErr;
spinNodeHandle	hAcquisitionMode			=	NULL;
spinNodeHandle	hAcquisitionModeContinuous	=	NULL;
int64_t			acquisitionModeContinuous	=	0;
int64_t			acquisitionModeSingleFrame	=	0;

	CONSOLE_DEBUG(__FUNCTION__);

	// Retrieve enumeration node from nodemap
	spinErr	=	spinNodeMapGetNode(cSpinNodeMapHandle, "AcquisitionMode", &hAcquisitionMode);
	if (spinErr == SPINNAKER_ERR_SUCCESS)
	{
		CONSOLE_DEBUG("spinNodeMapGetNode  SUCCESS!!!");
	}
	else
	{
		printf("Unable to get acquisition mode Handle. Aborting with error %d...\n\n", spinErr);
		return spinErr;
	}

	// Retrieve entry node from enumeration node
	if (IsAvailableAndReadable(hAcquisitionMode, "AcquisitionMode"))
	{
#ifdef _FLIR_SINGLE_FRAME_
		spinErr	=	spinEnumerationGetEntryByName(hAcquisitionMode, "SingleFrame", &hAcquisitionModeContinuous);
		if (spinErr == SPINNAKER_ERR_SUCCESS)
		{
			CONSOLE_DEBUG("spinEnumerationGetEntryByName (SingleFrame)  SUCCESS!!!");
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Unable to set acquisition mode to SingleFrame (entry 'SingleFrame' retrieval). Aborting with error=",
								spinErr);
			return spinErr;
		}
#else
		spinErr	=	spinEnumerationGetEntryByName(hAcquisitionMode, "Continuous", &hAcquisitionModeContinuous);
		if (spinErr == SPINNAKER_ERR_SUCCESS)
		{
			CONSOLE_DEBUG("spinEnumerationGetEntryByName  SUCCESS!!!");
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Unable to set acquisition mode to continuous (entry 'continuous' retrieval). Aborting with error=",
								spinErr);
			return spinErr;
		}
#endif
	}
	else
	{
		PrintRetrieveNodeFailure("entry", "AcquisitionMode");
		return SPINNAKER_ERR_ACCESS_DENIED;
	}
#ifdef _FLIR_SINGLE_FRAME_
	if (IsAvailableAndReadable(hAcquisitionModeContinuous, "AcquisitionModeSingleFrame"))
	{
		spinErr	=	spinEnumerationEntryGetIntValue(hAcquisitionModeContinuous, &acquisitionModeSingleFrame);
		if (spinErr == SPINNAKER_ERR_SUCCESS)
		{
			CONSOLE_DEBUG("spinEnumerationEntryGetIntValue  SUCCESS!!!");
			CONSOLE_DEBUG_W_LONG("acquisitionModeContinuous\t=", acquisitionModeContinuous);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Unable to set acquisition mode to continuous (entry int value retrieval). Aborting with error=",
					spinErr);
			return spinErr;
		}
	}
	else
	{
		PrintRetrieveNodeFailure("entry", "AcquisitionMode 'Continuous'");
		return SPINNAKER_ERR_ACCESS_DENIED;
	}
#endif
	// Retrieve integer from entry node

	if (IsAvailableAndReadable(hAcquisitionModeContinuous, "AcquisitionModeContinuous"))
	{
		spinErr	=	spinEnumerationEntryGetIntValue(hAcquisitionModeContinuous, &acquisitionModeContinuous);
		if (spinErr == SPINNAKER_ERR_SUCCESS)
		{
			CONSOLE_DEBUG("spinEnumerationEntryGetIntValue  SUCCESS!!!");
			CONSOLE_DEBUG_W_LONG("acquisitionModeContinuous\t=", acquisitionModeContinuous);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Unable to set acquisition mode to continuous (entry int value retrieval). Aborting with error=",
					spinErr);
			return spinErr;
		}
	}
	else
	{
		PrintRetrieveNodeFailure("entry", "AcquisitionMode 'Continuous'");
		return SPINNAKER_ERR_ACCESS_DENIED;
	}
	// Set integer as new value of enumeration node
	if (IsAvailableAndWritable(hAcquisitionMode, "AcquisitionMode"))
	{
		spinErr	=	spinEnumerationSetIntValue(hAcquisitionMode, acquisitionModeContinuous);
		if (spinErr == SPINNAKER_ERR_SUCCESS)
		{
			CONSOLE_DEBUG("spinEnumerationSetIntValue  SUCCESS!!!");
		}
		else
		{
			printf("Unable to set acquisition mode to continuous (entry int value setting). Aborting with error %d...\n\n",
					spinErr);
			return spinErr;
		}
	}
	else
	{
		PrintRetrieveNodeFailure("entry", "AcquisitionMode");
		return SPINNAKER_ERR_ACCESS_DENIED;
	}

	CONSOLE_DEBUG("Acquisition mode set to continuous...");

	return(spinErr);
}


//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverFLIR::Start_CameraExposure(int32_t exposureMicrosecs, const bool lightFrame)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
spinError			spinErr;
char				spinErrorString[64];

	CONSOLE_DEBUG(__FUNCTION__);

	gettimeofday(&cCameraProp.Lastexposure_StartTime, NULL);
	//
	// Begin acquiring images
	//
	// *** NOTES ***
	// What happens when the camera begins acquiring images depends on the
	// acquisition mode. Single frame captures only a single image, multi
	// frame captures a set number of images, and continuous captures a
	// continuous stream of images. Because the example calls for the retrieval
	// of 10 images, continuous mode has been set.
	//
	// *** LATER ***
	// Image acquisition must be ended when no more images are needed.
	//
	if (cSpinCameraHandle != NULL)
	{
//*testing 7/5/2022
//		if (cSpinNodeMapHandle == NULL)
		{
			spinErr	=	spinCameraGetNodeMap(cSpinCameraHandle, &cSpinNodeMapHandle);
			if (spinErr == SPINNAKER_ERR_SUCCESS)
			{
				CONSOLE_DEBUG("Calling SetFlirAqcuistionMode()");
				spinErr	=	SetFlirAqcuistionMode(1);
				CONSOLE_DEBUG_W_NUM("SetFlirAqcuistionMode returned", spinErr);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("Failed to get node map (spinCameraGetNodeMap)Aborting with error=",
										spinErr);
			}
		}

		CONSOLE_DEBUG(__FUNCTION__);
		spinErr	=	spinCameraBeginAcquisition(cSpinCameraHandle);
		if (spinErr == SPINNAKER_ERR_SUCCESS)
		{
			CONSOLE_DEBUG("spinCameraBeginAcquisition  SUCCESS!!!");
			cInternalCameraState	=	kCameraState_TakingPicture;
			alpacaErrCode			=	kASCOM_Err_Success;
		}
		else if (spinErr == SPINNAKER_ERR_NOT_INITIALIZED)
		{
			CONSOLE_DEBUG("SPINNAKER_ERR_NOT_INITIALIZED");
			InitFlirCamera();
			spinErr	=	spinCameraBeginAcquisition(cSpinCameraHandle);
			if (spinErr == SPINNAKER_ERR_SUCCESS)
			{
				CONSOLE_DEBUG("spinCameraBeginAcquisition  SUCCESS!!!");
				cInternalCameraState	=	kCameraState_TakingPicture;
				alpacaErrCode			=	kASCOM_Err_Success;
			}
		}
		CONSOLE_DEBUG(__FUNCTION__);

		if (spinErr != SPINNAKER_ERR_SUCCESS)
		{
			GetSpinnakerErrorString(spinErr, spinErrorString);
			strcpy(cLastCameraErrMsg, "Unable to begin image acquisition. spinErr=");
			strcat(cLastCameraErrMsg, spinErrorString);
			CONSOLE_DEBUG(cLastCameraErrMsg);
			alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
		strcpy(cLastCameraErrMsg, "cSpinCameraHandle is NULL");
		CONSOLE_DEBUG(cLastCameraErrMsg);
	}
	if (alpacaErrCode != kASCOM_Err_Success)
	{
		CONSOLE_DEBUG_W_NUM("Returning Alpaca error code\t=", alpacaErrCode);
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");
	return(alpacaErrCode);
}


static int	gFLIRcheckInProgress	=	0;

//*****************************************************************************
TYPE_EXPOSURE_STATUS	CameraDriverFLIR::Check_Exposure(bool verboseFlag)
{
TYPE_EXPOSURE_STATUS	exposureState;

	if (cVerboseDebug)
	{
		CONSOLE_DEBUG("-------------------------------------------------------------");
		CONSOLE_DEBUG(__FUNCTION__);
	}
//	exposureState	=	kExposure_Working;
//	exposureState	=	kExposure_Idle;
	exposureState	=	kExposure_Success;
//	exposureState	=	kExposure_Failed;
//	exposureState	=	kExposure_Unknown;

	if (gFLIRcheckInProgress == 0)
	{
		gFLIRcheckInProgress++;
	#if 0
	//*	experimenting 1/28/2021
	spinError				spinErr;
	bool8_t					isIncomplete	=	false;

		if (cSpinCameraHandle != NULL)
		{
//			CONSOLE_DEBUG("cSpinCameraHandle is OK");
			cSpinImageHandle	=	NULL;
			spinErr	=	spinCameraGetNextImage(cSpinCameraHandle, &cSpinImageHandle);
			if (spinErr == SPINNAKER_ERR_SUCCESS)
			{
				CONSOLE_DEBUG("spinCameraGetNextImage returned SPINNAKER_ERR_SUCCESS");
				if (cSpinImageHandle != NULL)
				{
					// *** NOTES ***
					// Images can easily be checked for completion. This should be done
					// whenever a complete image is expected or required. Further, check
					// image status for a little more insight into why an image is
					// incomplete.
					//

					spinErr	=	spinImageIsIncomplete(cSpinImageHandle, &isIncomplete);
					if (spinErr == SPINNAKER_ERR_SUCCESS)
					{
						CONSOLE_DEBUG("spinImageIsIncomplete");
						if (isIncomplete)
						{
							CONSOLE_DEBUG("kExposure_Working");
							exposureState	=	kExposure_Working;
						}
						else
						{
							CONSOLE_DEBUG("isIncomplete == FALSE");
							exposureState	=	kExposure_Success;
						}
					}
					else
					{
						CONSOLE_DEBUG_W_NUM("Unable to determine image completion. Non-fatal error=", spinErr);
					}

					spinErr				=	spinImageRelease(cSpinImageHandle);
					cSpinImageHandle	=	NULL;
				}
				else
				{
					CONSOLE_DEBUG_W_NUM("Unable to determine image completion. Non-fatal error=", spinErr);
				}
			}
			else
			{
//				CONSOLE_DEBUG("Failed to get cSpinImageHandle");
			}
		}
		else
		{
			CONSOLE_DEBUG("cSpinCameraHandle is NULL!!!!");
		}
	#endif

		gFLIRcheckInProgress--;
	}
	else
	{
		exposureState	=	kExposure_Working;
		CONSOLE_DEBUG("Check_Exposure already in progress");
//		CONSOLE_ABORT(__FUNCTION__);
	}
	if (cVerboseDebug)
	{
		CONSOLE_DEBUG("EXIT---EXIT---EXIT---EXIT---EXIT---EXIT---EXIT---EXIT---EXIT---");
	}
	return(exposureState);
}


//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverFLIR::Read_ImageData(void)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
spinError				spinErr;
bool8_t					isIncomplete	=	false;
size_t					spinnakerWidth;
size_t					spinnakerHeight;
size_t					spinnakerBitsPerPixel;
spinPixelFormatEnums	spinnakerPixelFormat;
bool					imageIsGood;
char					myImageFileName[64];
char					spinnakerPixelFormatName[64];
size_t					bufferLen;
char					spinErrorString[64];

//	CONSOLE_DEBUG(__FUNCTION__);
	//
	// Retrieve next received image
	//
	// *** NOTES ***
	// Capturing an image houses images on the camera buffer. Trying to
	// capture an image that does not exist will hang the camera.
	//
	// *** LATER ***
	// Once an image from the buffer is saved and/or no longer needed, the
	// image must be released in order to keep the buffer from filling up.
	//

	if (cSpinCameraHandle != NULL)
	{
		cSpinImageHandle	=	NULL;
		spinErr	=	spinCameraGetNextImage(cSpinCameraHandle, &cSpinImageHandle);
		if (spinErr == SPINNAKER_ERR_SUCCESS)
		{
			imageIsGood	=	true;
//			CONSOLE_DEBUG("spinCameraGetNextImage");
			//
			// Ensure image completion
			//
			// *** NOTES ***
			// Images can easily be checked for completion. This should be done
			// whenever a complete image is expected or required. Further, check
			// image status for a little more insight into why an image is
			// incomplete.
			//

			spinErr	=	spinImageIsIncomplete(cSpinImageHandle, &isIncomplete);
			if (spinErr == SPINNAKER_ERR_SUCCESS)
			{
//				CONSOLE_DEBUG("spinImageIsIncomplete");
				if (isIncomplete)
				{
					imageIsGood	=	false;
				}
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("Unable to determine image completion. Non-fatal error=", spinErr);
				imageIsGood	=	false;
			}

			if (imageIsGood)
			{
				CONSOLE_DEBUG("imageIsGood");
				alpacaErrCode		=	kASCOM_Err_Success;

				//*	get the info about the image

				// Retrieve image width
				spinErr	=	spinImageGetWidth(cSpinImageHandle, &spinnakerWidth);
				if (spinErr == SPINNAKER_ERR_SUCCESS)
				{
					cCameraProp.CameraXsize	=	spinnakerWidth;
				}
				// Retrieve image height
				spinErr	=	spinImageGetHeight(cSpinImageHandle, &spinnakerHeight);
				if (spinErr == SPINNAKER_ERR_SUCCESS)
				{
					cCameraProp.CameraYsize	=	spinnakerHeight;
				}
				// Retrieve bits per pixel
				spinErr	=	spinImageGetBitsPerPixel(cSpinImageHandle, &spinnakerBitsPerPixel);
				if (spinErr == SPINNAKER_ERR_SUCCESS)
				{
					cBitDepth	=	spinnakerBitsPerPixel;
					CONSOLE_DEBUG_W_NUM("cBitDepth\t=", cBitDepth);
				}
				spinErr	=	spinImageGetPixelFormat(cSpinImageHandle, &spinnakerPixelFormat);
				if (spinErr == SPINNAKER_ERR_SUCCESS)
				{
					CONSOLE_DEBUG_W_NUM("spinnakerPixelFormat=", spinnakerPixelFormat);
				}
				else
				{
					CONSOLE_DEBUG_W_NUM("spinImageGetPixelFormat() returned error=", spinErr);
				}

				bufferLen	=	sizeof(spinnakerPixelFormatName) - 1;
				spinErr		=	spinImageGetPixelFormatName(cSpinImageHandle,
															spinnakerPixelFormatName,
															&bufferLen);
				if (spinErr == SPINNAKER_ERR_SUCCESS)
				{
					CONSOLE_DEBUG_W_STR("spinImageGetPixelFormatName=", spinnakerPixelFormatName);
				}
				else
				{
					CONSOLE_DEBUG_W_NUM("spinImageGetPixelFormatName() returned error=", spinErr);
				}

				if (cSpinImageHandle != NULL)
				{
					CONSOLE_DEBUG("cSpinImageHandle GOOD!!!!!");
				//	ExtractColorImage();
				//	ConvertToMono();
				}
				else
				{
					CONSOLE_DEBUG("cSpinImageHandle is NULL");
				}

				if (cSaveNextImage)
				{
					CONSOLE_DEBUG("cSaveNextImage");
					GenerateFileNameRoot();
					strcpy(myImageFileName, cFileNameRoot);
					strcat(myImageFileName, "-spin.jpg");
					spinErr	=	spinImageSave(cSpinImageHandle, myImageFileName, JPEG);
					if (spinErr == SPINNAKER_ERR_SUCCESS)
					{
						CONSOLE_DEBUG_W_STR("Image saved as", myImageFileName);
					}
					else
					{
						GetSpinnakerErrorString(spinErr, spinErrorString);
						CONSOLE_DEBUG_W_STR("Unable to save image. Non-fatal error=", spinErrorString);
					}
				}
				if (cSpinImageHandle == NULL)
				{
					CONSOLE_DEBUG("cSpinImageHandle is NULL");
				}
				ConvertToMono();
			}
			else
			{
				CONSOLE_DEBUG("Image not saved");
			}

			spinErr				=	spinImageRelease(cSpinImageHandle);
			cSpinImageHandle	=	NULL;
			//
			// End acquisition
			//
			// *** NOTES ***
			// Ending acquisition appropriately helps ensure that devices clean up
			// properly and do not need to be power-cycled to maintain integrity.
			//
			spinErr	=	spinCameraEndAcquisition(cSpinCameraHandle);
			if (spinErr != SPINNAKER_ERR_SUCCESS)
			{
				GetSpinnakerErrorString(spinErr, spinErrorString);
				CONSOLE_DEBUG_W_STR("Unable to end acquisition. Non-fatal error=", spinErrorString);
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
			GetSpinnakerErrorString(spinErr, spinErrorString);
			CONSOLE_DEBUG_W_STR("Unable to get next image. Non-fatal error", spinErrorString);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
		CONSOLE_DEBUG("cSpinCameraHandle is NULL");
		CONSOLE_ABORT(__FUNCTION__);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
int	CameraDriverFLIR::ExtractColorImage(void)
{
spinError		spinErr;
bool			buffOk;
int				imageSize;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cSpinImageHandle != NULL)
	{

		buffOk	=	AllocateImageBuffer(0);
		if (buffOk && (cCameraDataBuffer != NULL) && (cCameraDataBuffLen > 0))
		{
		void	 	**data;
		size_t		memBuffSize;


			imageSize	=	cCameraProp.CameraXsize * cCameraProp.CameraYsize * (cBitDepth / 8);

			CONSOLE_DEBUG_W_LONG("cCameraDataBuffLen\t\t=",		cCameraDataBuffLen);
			CONSOLE_DEBUG_W_NUM("cCameraProp.CameraXsize\t=",	cCameraProp.CameraXsize);
			CONSOLE_DEBUG_W_NUM("cCameraProp.CameraYsize\t=",	cCameraProp.CameraYsize);
			CONSOLE_DEBUG_W_NUM("cBitDepth\t\t\t=",				cBitDepth);
			CONSOLE_DEBUG_W_NUM("imageSize\t\t\t=",				imageSize);


			memBuffSize	=	3 * imageSize * sizeof(void*);
			CONSOLE_DEBUG_W_LONG("memBuffSize=", memBuffSize);
			CONSOLE_DEBUG("Allocating spinnaker data buffer");

			data	=	(void **)malloc(memBuffSize);

			if (data != NULL)
			{
				CONSOLE_DEBUG("Calling spinImageGetData");
				spinErr		=	spinImageGetData(	cSpinImageHandle,	data);

				if (spinErr == SPINNAKER_ERR_SUCCESS)
				{
					CONSOLE_DEBUG_W_LONG("memBuffSize\t=",			memBuffSize);
					CONSOLE_DEBUG_W_LONG("cCameraDataBuffLen\t=",	cCameraDataBuffLen);
					CONSOLE_DEBUG("Calling memcpy");
					memcpy(cCameraDataBuffer, *data, imageSize);
					CONSOLE_DEBUG("Done with memcpy");
				}
				else
				{
					CONSOLE_DEBUG_W_NUM("spinImageGetData failed, err=", spinErr);
				}

				free(data);
			}
			else
			{
				CONSOLE_DEBUG("Failed to allocate image data buffer");
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("cSpinImageHandle is NULL");
	}
	return(0);
}

//*****************************************************************************
int	CameraDriverFLIR::ConvertToMono(void)
{
spinError				spinErr;
spinImage				hConvertedImage = NULL;
char					myImageFileName[128];
size_t					spinnakerWidth;
size_t					spinnakerHeight;
size_t					spinnakerBitsPerPixel;
spinPixelFormatEnums	spinnakerPixelFormat;
size_t					spinnakerPaddingX;
size_t					spinnakerPaddingY;
size_t					spinnakerStride;
size_t					spinnakerImageSize;
char					spinnakerPixelFormatName[64];
size_t					bufferLen;
bool					buffOk;

	CONSOLE_DEBUG(__FUNCTION__);
	//
	// Convert image to mono 8
	//
	// *** NOTES ***
	// Images not gotten from a camera directly must be created and
	// destroyed. This includes any image copies, conversions, or
	// otherwise. Basically, if the image was gotten, it should be
	// released, if it was created, it needs to be destroyed.
	//
	// Images can be converted between pixel formats by using the
	// appropriate enumeration value. Unlike the original image, the
	// converted one does not need to be released as it does not affect the
	// camera buffer.
	//
	// Optionally, the color processing algorithm can also be set using
	// the alternate spinImageConvertEx() function.
	//
	// *** LATER ***
	// The converted image was created, so it must be destroyed to avoid
	// memory leaks.
	//
	if (cSpinImageHandle != NULL)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		spinErr	=	spinImageCreateEmpty(&hConvertedImage);
		if (spinErr == SPINNAKER_ERR_SUCCESS)
		{
			spinErr = spinImageConvert(cSpinImageHandle, PixelFormat_Mono8, hConvertedImage);
			if (spinErr == SPINNAKER_ERR_SUCCESS)
			{
				if (cSaveNextImage)
				{
					GenerateFileNameRoot();
					strcpy(myImageFileName, cFileNameRoot);
					strcat(myImageFileName, "-mono.jpg");
					spinErr	=	spinImageSave(hConvertedImage, myImageFileName, JPEG);
					if (spinErr == SPINNAKER_ERR_SUCCESS)
					{
			//			CONSOLE_DEBUG_W_STR("Image saved as", myImageFileName);
					}
					else
					{
						CONSOLE_DEBUG_W_NUM("Unable to save image. Non-fatal error=", spinErr);
					}
				}
				bufferLen	=	sizeof(spinnakerPixelFormatName) - 1;

				spinErr	=	spinImageGetWidth(			hConvertedImage,	&spinnakerWidth);
				spinErr	=	spinImageGetHeight(			hConvertedImage,	&spinnakerHeight);
				spinErr	=	spinImageGetBitsPerPixel(	hConvertedImage,	&spinnakerBitsPerPixel);
				spinErr	=	spinImageGetPixelFormat(	hConvertedImage,	&spinnakerPixelFormat);
				spinErr	=	spinImageGetPaddingX(		hConvertedImage, 	&spinnakerPaddingX);
				spinErr	=	spinImageGetPaddingY(		hConvertedImage, 	&spinnakerPaddingY);
				spinErr	=	spinImageGetStride(			hConvertedImage,	&spinnakerStride);
				spinErr	=	spinImageGetPixelFormatName(hConvertedImage,	spinnakerPixelFormatName, &bufferLen);
				spinErr	=	spinImageGetBufferSize(		hConvertedImage,	&spinnakerImageSize);

//				CONSOLE_DEBUG_W_LONG("spinnakerWidth\t\t\t=",			spinnakerWidth);
//				CONSOLE_DEBUG_W_LONG("spinnakerWidth\t\t\t=",			spinnakerHeight);
//				CONSOLE_DEBUG_W_LONG("spinnakerPaddingX\t\t\t=",		spinnakerPaddingX);
//				CONSOLE_DEBUG_W_LONG("spinnakerPaddingY\t\t\t=",		spinnakerPaddingY);
//				CONSOLE_DEBUG_W_LONG("spinnakerStride\t\t\t=",			spinnakerStride);
//				CONSOLE_DEBUG_W_NUM("spinnakerPixelFormat\t\t=",		spinnakerPixelFormat);
//				CONSOLE_DEBUG_W_STR("spinImageGetPixelFormatName\t=",	spinnakerPixelFormatName);
//				CONSOLE_DEBUG_W_LONG("spinnakerImageSize\t\t\t=",		spinnakerImageSize);

				cROIinfo.currentROIimageType	=	kImageType_RAW8;
				cROIinfo.currentROIwidth		=	spinnakerWidth;
				cROIinfo.currentROIheight		=	spinnakerHeight;


				buffOk	=	AllocateImageBuffer(0);
				if (buffOk && (cCameraDataBuffer != NULL) && (cCameraDataBuffLen > 0))
				{
				void	 	**data;
				size_t		memBuffSize;

//					CONSOLE_DEBUG_W_LONG("cCameraDataBuffLen\t=",	cCameraDataBuffLen);

					memBuffSize	=	2 * spinnakerImageSize * sizeof(void*);
//					CONSOLE_DEBUG_W_LONG("memBuffSize=", memBuffSize);
//					CONSOLE_DEBUG("Allocating spinnaker data buffer");
					data	=	(void **)malloc(memBuffSize);

					if (data != NULL)
					{
						CONSOLE_DEBUG("Calling spinImageGetData");
						spinErr		=	spinImageGetData(	hConvertedImage,	data);

						if (spinErr == SPINNAKER_ERR_SUCCESS)
						{
							CONSOLE_DEBUG_W_LONG("memBuffSize\t=",			memBuffSize);
							CONSOLE_DEBUG_W_LONG("cCameraDataBuffLen\t=",	cCameraDataBuffLen);
							CONSOLE_DEBUG("Calling memcpy");
							memcpy(cCameraDataBuffer, *data, spinnakerImageSize);
							CONSOLE_DEBUG("Done with memcpy");
						}
						else
						{
							CONSOLE_DEBUG_W_NUM("spinImageGetData failed, err=", spinErr);
						}

						free(data);
					}
					else
					{
						CONSOLE_DEBUG("Failed to allocate image data buffer");
					}
				}
//		CONSOLE_DEBUG(__FUNCTION__);
		//		CreateOpenCVImage(cCameraDataBuffer);

//		CONSOLE_DEBUG(__FUNCTION__);
		//		SaveImageData();

//		CONSOLE_DEBUG(__FUNCTION__);
				spinErr	=	spinImageDestroy(hConvertedImage);
//		CONSOLE_DEBUG(__FUNCTION__);
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("Unable to convert image. Non-fatal error=", spinErr);
			}
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Unable to create image. Non-fatal error=", spinErr);
		}
	}
	else
	{
		CONSOLE_DEBUG("cSpinImageHandle is NULL");
	}
	return(0);
}

//**************************************************************************
//*	sets class variable to current temp
//**************************************************************************
TYPE_ASCOM_STATUS	CameraDriverFLIR::Read_SensorTemp(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
//double				cameraTemp_DegC;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cSpinCameraHandle != NULL)
	{
	//	cameraTemp_DegC	=	cSpinCameraHandle->DeviceTemperature;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverFLIR::Write_Gain(const int newGainValue)
{
TYPE_ASCOM_STATUS				alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CameraDriverFLIR::Read_Gain(int *cameraGainValue)
{
TYPE_ASCOM_STATUS			alpacaErrCode	=	kASCOM_Err_NotImplemented;

	return(alpacaErrCode);
}

//*****************************************************************************
typedef struct
{
	int		errorNum;
	char	errorString[64];
}	TYPE_SPIN_ERROR_MSG;

TYPE_SPIN_ERROR_MSG	gSpinkerErrorStrings[]	=
{

	{	SPINNAKER_ERR_ERROR,				"SPINNAKER_ERR_ERROR"				},
	{	SPINNAKER_ERR_NOT_INITIALIZED,		"SPINNAKER_ERR_NOT_INITIALIZED"		},
	{	SPINNAKER_ERR_NOT_IMPLEMENTED,		"SPINNAKER_ERR_NOT_IMPLEMENTED"		},
	{	SPINNAKER_ERR_RESOURCE_IN_USE,		"SPINNAKER_ERR_RESOURCE_IN_USE"		},
	{	SPINNAKER_ERR_ACCESS_DENIED,		"SPINNAKER_ERR_ACCESS_DENIED"		},
	{	SPINNAKER_ERR_INVALID_HANDLE,		"SPINNAKER_ERR_INVALID_HANDLE"		},
	{	SPINNAKER_ERR_INVALID_ID,			"SPINNAKER_ERR_INVALID_ID"			},
	{	SPINNAKER_ERR_NO_DATA,				"SPINNAKER_ERR_NO_DATA"				},
	{	SPINNAKER_ERR_INVALID_PARAMETER,	"SPINNAKER_ERR_INVALID_PARAMETER"	},
	{	SPINNAKER_ERR_IO,					"SPINNAKER_ERR_IO"					},
	{	SPINNAKER_ERR_TIMEOUT,				"SPINNAKER_ERR_TIMEOUT"				},
	{	SPINNAKER_ERR_ABORT,				"SPINNAKER_ERR_ABORT"				},
	{	SPINNAKER_ERR_INVALID_BUFFER,		"SPINNAKER_ERR_INVALID_BUFFER"		},
	{	SPINNAKER_ERR_NOT_AVAILABLE,		"SPINNAKER_ERR_NOT_AVAILABLE"		},
	{	SPINNAKER_ERR_INVALID_ADDRESS,		"SPINNAKER_ERR_INVALID_ADDRESS"		},
	{	SPINNAKER_ERR_BUFFER_TOO_SMALL,		"SPINNAKER_ERR_BUFFER_TOO_SMALL"	},
	{	SPINNAKER_ERR_INVALID_INDEX,		"SPINNAKER_ERR_INVALID_INDEX"		},
	{	SPINNAKER_ERR_PARSING_CHUNK_DATA,	"SPINNAKER_ERR_PARSING_CHUNK_DATA"	},
	{	SPINNAKER_ERR_INVALID_VALUE,		"SPINNAKER_ERR_INVALID_VALUE"		},
	{	SPINNAKER_ERR_RESOURCE_EXHAUSTED,	"SPINNAKER_ERR_RESOURCE_EXHAUSTED"	},
	{	SPINNAKER_ERR_OUT_OF_MEMORY,		"SPINNAKER_ERR_OUT_OF_MEMORY"		},
	{	SPINNAKER_ERR_BUSY,					"SPINNAKER_ERR_BUSY"				},



	//*
	//*The error codes in the range of -2000 to -2999 are reserved for
	//* Gen API related errors.

	{	GENICAM_ERR_INVALID_ARGUMENT,		"GENICAM_ERR_INVALID_ARGUMENT"	},
	{	GENICAM_ERR_OUT_OF_RANGE,			"GENICAM_ERR_OUT_OF_RANGE"		},
	{	GENICAM_ERR_PROPERTY,				"GENICAM_ERR_PROPERTY"			},
	{	GENICAM_ERR_RUN_TIME,				"GENICAM_ERR_RUN_TIME"			},
	{	GENICAM_ERR_LOGICAL,				"GENICAM_ERR_LOGICAL"			},
	{	GENICAM_ERR_ACCESS,					"GENICAM_ERR_ACCESS"			},
	{	GENICAM_ERR_TIMEOUT,				"GENICAM_ERR_TIMEOUT"			},
	{	GENICAM_ERR_DYNAMIC_CAST,			"GENICAM_ERR_DYNAMIC_CAST"		},
	{	GENICAM_ERR_GENERIC,				"GENICAM_ERR_GENERIC"			},
	{	GENICAM_ERR_BAD_ALLOCATION,			"GENICAM_ERR_BAD_ALLOCATION"	},

	//*	The error codes in the range of -3000 to -3999 are reserved for
	//*	image processing related errors.

	{	SPINNAKER_ERR_IM_CONVERT,			"SPINNAKER_ERR_IM_CONVERT"			},
	{	SPINNAKER_ERR_IM_COPY,				"SPINNAKER_ERR_IM_COPY"				},
	{	SPINNAKER_ERR_IM_MALLOC,			"SPINNAKER_ERR_IM_MALLOC"			},
	{	SPINNAKER_ERR_IM_NOT_SUPPORTED,		"SPINNAKER_ERR_IM_NOT_SUPPORTED"	},
	{	SPINNAKER_ERR_IM_HISTOGRAM_RANGE,	"SPINNAKER_ERR_IM_HISTOGRAM_RANGE"	},
	{	SPINNAKER_ERR_IM_HISTOGRAM_MEAN,	"SPINNAKER_ERR_IM_HISTOGRAM_MEAN"	},
	{	SPINNAKER_ERR_IM_MIN_MAX,			"SPINNAKER_ERR_IM_MIN_MAX"			},
	{	SPINNAKER_ERR_IM_COLOR_CONVERSION,	"SPINNAKER_ERR_IM_COLOR_CONVERSION"	},

	//*	Error codes less than -10000 are reserved for user-defined customerrors.
	{	SPINNAKER_ERR_CUSTOM_ID,			"SPINNAKER_ERR_CUSTOM_ID"			},

	{	0,	""	}

};

//*****************************************************************************
static void	GetSpinnakerErrorString(_spinError errorCode, char *errorString)
{
int		iii;
bool	keepLooking;

	keepLooking	=	true;
	iii			=	0;
	while ((gSpinkerErrorStrings[iii].errorNum != 0) && (iii < 40))
	{
		if (errorCode == gSpinkerErrorStrings[iii].errorNum)
		{
			strcpy(errorString, gSpinkerErrorStrings[iii].errorString);
			keepLooking	=	false;
			break;
		}
		iii++;
	}
	if (keepLooking)
	{
		sprintf(errorString, "Err# %d - Not found in table", errorCode);
	}
}

#endif // defined(_ENABLE_CAMERA_) && defined(_ENABLE_FLIR_)
