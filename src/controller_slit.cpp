//*****************************************************************************
//*		controller_slit.cpp		(c) 2021 by Mark Sproul
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
//*	Oct  6,	2021	<MLS> Created controller_slit.cpp (copied from controller_dome.cpp)
//*	Oct  6,	2021	<MLS> Slit tracker controller window starting to work
//*****************************************************************************


#include	<math.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>

#include	"alpaca_defs.h"
#include	"helper_functions.h"

//#include	"discovery_lib.h"
#include	"sendrequest_lib.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"controller.h"
#include	"controller_slit.h"
#include	"windowtab_slit.h"
#include	"windowtab_slitgraph.h"
#include	"windowtab_about.h"

#include	"slittracker_data.h"

TYPE_SLITCLOCK	gSlitDistance[kSensorValueCnt];		//*	current reading
TYPE_SLIT_LOG	gSlitLog[kSlitLogCount];			//*	log of readings
int				gSlitLogIdx;

//#define	_SLIT_TRACKER_DIRECT_

//**************************************************************************************
enum
{
	kTab_SlitTracker	=	1,
	kTab_SlitGraph,
	kTab_DriverInfo,
	kTab_About,

	kTab_Slit_Count

};


#ifdef _SLIT_TRACKER_DIRECT_
	#include	<termios.h>
	#include	<fcntl.h>
	#include	"serialport.h"

	void	OpenSlitTrackerPort(void);
	void	GetSLitTrackerData(void);
	void	SendSlitTrackerCmd(const char *cmdBuffer);
#endif // _SLIT_TRACKER_DIRECT_
	bool	gUpdateSLitWindow	=	true;




//**************************************************************************************
ControllerSlit::ControllerSlit(	const char			*argWindowName,
								TYPE_REMOTE_DEV		*alpacaDevice)
	:Controller(argWindowName, kDomeWindowWidth,  kDomeWindowHeight)
{
int		iii;
char	ipAddrStr[32];

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cAlpacaDeviceTypeStr,	"slittracker");

	cLogSlitData			=	false;
	cSlitDataLogFilePtr		=	NULL;
	cValidGravity			=	false;

	//*	window object ptrs
	cSlitTrackerTabObjPtr	=	NULL;
	cSlitGraphTabObjPtr		=	NULL;
	cAboutBoxTabObjPtr		=	NULL;

	cFirstDataRead			=	true;
	cLastUpdate_milliSecs	=	millis();
	cUpdateDelta			=	kDefaultUpdateDelta;

	if (alpacaDevice != NULL)
	{
		cValidIPaddr	=	true;

		cAlpacaDevNum	=	alpacaDevice->alpacaDeviceNum;
		cDeviceAddress	=	alpacaDevice->deviceAddress;
		cPort			=	alpacaDevice->port;

		strcpy(cAlpacaDeviceNameStr,	alpacaDevice->deviceNameStr);
	#ifdef _ENABLE_SKYTRAVEL_
		//*	make a 2nd copy if we are in SKYTRAVEL
	//	cDomeIpAddress			=	cDeviceAddress;
	//	cDomeIpPort				=	cPort;
	//	cDomeAlpacaDeviceNum	=	cPort;
	#endif

		CheckConnectedState();		//*	check connected and connect if not already connected
	}

	inet_ntop(AF_INET, &(cDeviceAddress.sin_addr), ipAddrStr, INET_ADDRSTRLEN);
	CONSOLE_DEBUG_W_STR("IP address=", ipAddrStr);

	//*	initialize the slit distance detector
	for (iii=0; iii<kSensorValueCnt; iii++)
	{
		gSlitDistance[iii].validData		=	false;
		gSlitDistance[iii].distanceInches	=	0.0;
		gSlitDistance[iii].readCount		=	0;
	}

	//*	initialize the slit distance log
	for (iii=0; iii<kSlitLogCount; iii++)
	{
		memset(&gSlitLog[iii], 0, sizeof(TYPE_SLIT_LOG));
	}
	gSlitLogIdx	=	0;

	SetupWindowControls();

#ifdef _SLIT_TRACKER_DIRECT_
	OpenSlitTrackerPort();
#endif // _SLIT_TRACKER_DIRECT_

#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerSlit::~ControllerSlit(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	if (cLogSlitData)
	{
		CloseSlitTrackerDataFile();
	}


	DELETE_OBJ_IF_VALID(cSlitTrackerTabObjPtr);
	DELETE_OBJ_IF_VALID(cSlitGraphTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
void	ControllerSlit::SetupWindowControls(void)
{
char	lineBuff[64];

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Slit_Count);
	SetTabText(kTab_SlitTracker,	"Slit Tracker");
	SetTabText(kTab_SlitGraph,		"Slit Graph");
	SetTabText(kTab_DriverInfo,		"Driver Info");
	SetTabText(kTab_About,			"About");

	CONSOLE_DEBUG(__FUNCTION__);

	//=============================================================
	cSlitTrackerTabObjPtr		=	new WindowTabSlitTracker(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSlitTrackerTabObjPtr != NULL)
	{
	CONSOLE_DEBUG(__FUNCTION__);
		SetTabWindow(kTab_SlitTracker,	cSlitTrackerTabObjPtr);
		cSlitTrackerTabObjPtr->SetParentObjectPtr(this);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create WindowTabSlitTracker!!!");
		CONSOLE_ABORT(__FUNCTION__);
	}

	//=============================================================
	cSlitGraphTabObjPtr		=	new WindowTabSlitGraph(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSlitGraphTabObjPtr != NULL)
	{
	CONSOLE_DEBUG(__FUNCTION__);
		SetTabWindow(kTab_SlitGraph,	cSlitGraphTabObjPtr);
		cSlitGraphTabObjPtr->SetParentObjectPtr(this);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create WindowTabSlitGraph!!!");
		CONSOLE_ABORT(__FUNCTION__);
	}

	//=============================================================
	cDriverInfoTabObjPtr		=	new WindowTabDriverInfo(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDriverInfoTabObjPtr != NULL)
	{
	CONSOLE_DEBUG(__FUNCTION__);
		SetTabWindow(kTab_DriverInfo,	cDriverInfoTabObjPtr);
		cDriverInfoTabObjPtr->SetParentObjectPtr(this);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create WindowTabDriverInfo!!!");
		CONSOLE_ABORT(__FUNCTION__);
	}

	//=============================================================
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
	CONSOLE_DEBUG(__FUNCTION__);
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create WindowTabAbout!!!");
		CONSOLE_ABORT(__FUNCTION__);
	}

	//*	display the IPaddres/port
	if (cValidIPaddr)
	{
	char	ipString[32];

		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);

		SetWindowIPaddrInfo(lineBuff, true);
	}
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
void	ControllerSlit::UpdateCommonProperties(void)
{
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Name,				cCommonProp.Name);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Description,		cCommonProp.Description);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
	SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);
}


////**************************************************************************************
//void	ControllerSlit::SetAlpacaSlitTrackerInfo(TYPE_REMOTE_DEV *alpacaDevice)
//{
//char	ipString[64];
//char	lineBuff[128];
//
//	CONSOLE_DEBUG(__FUNCTION__);
//
//	if (alpacaDevice != NULL)
//	{
//		cSlitTrackerInfoValid		=	true;
//		cSlitTrackerDeviceAddress	=	alpacaDevice->deviceAddress;
//		cSlitTrackerPort			=	alpacaDevice->port;
//		cSlitTrackerAlpacaDevNum	=	alpacaDevice->alpacaDeviceNum;
//
//		PrintIPaddressToString(cSlitTrackerDeviceAddress.sin_addr.s_addr, ipString);
//		sprintf(lineBuff, "%s:%d/%d", ipString, cSlitTrackerPort, cSlitTrackerAlpacaDevNum);
//
//		SetWidgetText(kTab_SlitTracker,	kSlitTracker_RemoteAddress,	lineBuff);
//	}
//}


//**************************************************************************************
void	ControllerSlit::RunBackgroundTasks(const char *callingFunction, bool enableDebug)
{
uint32_t	currentMillis;
uint32_t	deltaSeconds;
bool		validData;
bool		needToUpdate;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cReadStartup)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		AlpacaGetCommonProperties_OneAAT("slittracker");
		AlpacaGetStartupData();
		cReadStartup	=	false;
//--	cDomeTabObjPtr->UpdateControls();
	}


	needToUpdate	=	false;
	currentMillis	=	millis();
	deltaSeconds	=	(currentMillis - cLastUpdate_milliSecs) / 1000;

	if (cFirstDataRead || (deltaSeconds >= cUpdateDelta))
	{
		needToUpdate	=	true;
	}
	if (cForceAlpacaUpdate)	//*	force update is set when a switch is clicked
	{
		needToUpdate		=	true;
		cForceAlpacaUpdate	=	false;
	}

	if (needToUpdate)
	{
		CONSOLE_DEBUG("Updating");
		//*	is the IP address valid
		if (cValidIPaddr)
		{
			validData	=	AlpacaGetStatus();
			if (validData == false)
			{
				CONSOLE_DEBUG("Failed to get data")
			}
			UpdateConnectedIndicator(kTab_SlitTracker,		kSlitTracker_Connected);
		}
		else
		{
			CONSOLE_ABORT("Invalid IP address")
		}
	}
#ifdef _SLIT_TRACKER_DIRECT_
	GetSLitTrackerData();
#endif // _SLIT_TRACKER_DIRECT_


	if (gUpdateSLitWindow)
	{
		cUpdateWindow		=	true;
		gUpdateSLitWindow	=	false;
	}
}




//*****************************************************************************
bool	ControllerSlit::AlpacaGetStartupData(void)
{
bool			validData;
char			returnString[128];

	CONSOLE_DEBUG(__FUNCTION__);
	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions("slittracker", cAlpacaDevNum);
	if (validData)
	{
		SetWidgetValid(kTab_SlitTracker,	kSlitTracker_Readall,	cHas_readall);
		SetWidgetValid(kTab_SlitGraph,		kSlitGraph_Readall,		cHas_readall);

		if (cHas_readall == false)
		{
			validData	=	AlpacaGetStringValue(	"slittracker", "driverversion",	NULL,	returnString);
			if (validData)
			{
				strcpy(cAlpacaVersionString, returnString);
				SetWidgetText(kTab_SlitTracker,		kSlitTracker_AlpacaDrvrVersion,		cAlpacaVersionString);
			}
		}
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}
	cLastUpdate_milliSecs	=	millis();

	return(validData);
}


//*****************************************************************************
bool	ControllerSlit::AlpacaGetStatus(void)
{
bool	validData;
bool	previousOnLineState;

	CONSOLE_DEBUG(__FUNCTION__);

	validData			=	false;
	previousOnLineState	=	cOnLine;
	if (cHas_readall)
	{
		CONSOLE_DEBUG("Has readall");
		validData	=	AlpacaGetStatus_ReadAll("slittracker", cAlpacaDevNum);
	}
	else
	{
		CONSOLE_DEBUG("No readall");
//--		validData	=	AlpacaGetStatus_DomeOneAAT();
		validData	=	AlpacaGetCommonConnectedState("slittracker");
	}

	if (validData)
	{
		if (cOnLine == false)
		{
			//*	if we were previously off line, force reading startup again
			cReadStartup	=	true;
		}
		cOnLine	=	true;

		//======================================================================
		cUpdateDelta	=	kDefaultUpdateDelta;
	}
	else
	{
		cOnLine	=	false;
	}
	if (cOnLine != previousOnLineState)
	{
		SetWindowIPaddrInfo(NULL, cOnLine);
	}

	//===============================================================
	//*	check for data logging to disk
	if (cLogSlitData)
	{
	struct tm		*linuxTime;
	char			slitLogFileName[48];
	int				jjj;

		linuxTime		=	localtime(&cSlitTrackerLastUpdateTime.tv_sec);
		if (cSlitDataLogFilePtr == NULL)
		{
			sprintf(slitLogFileName, "slitlog-%02d-%02d-%02d.csv",
										(1900 + linuxTime->tm_year),
										(1 +	linuxTime->tm_mon),
												linuxTime->tm_mday);

			cSlitDataLogFilePtr	=	fopen(slitLogFileName, "a");
		}
		if (cSlitDataLogFilePtr != NULL)
		{
			fprintf(cSlitDataLogFilePtr, "%02d:%02d:%02d,",	linuxTime->tm_hour,
															linuxTime->tm_min,
															linuxTime->tm_sec);
			for (jjj=0; jjj<kSensorValueCnt; jjj++)
			{
				fprintf(cSlitDataLogFilePtr, "%1.2f,", gSlitDistance[jjj].distanceInches);
			}
			fprintf(cSlitDataLogFilePtr, "\n");
			fflush(cSlitDataLogFilePtr);
		}
	}


	cLastUpdate_milliSecs	=	millis();
	cFirstDataRead			=	false;
	return(validData);
}

//*****************************************************************************
void	ControllerSlit::ProcessOneReadAllEntry(	const char	*keywordString,
												const char *valueString)
{
int				clockValue;
char			clockString[48];
double			inchValue;
double			gravityValue;
char			gravityVectorChar;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, keywordString);
//	CONSOLE_ABORT(__FUNCTION__);

	if (strncasecmp(keywordString, "sensor-", 6) == 0)
	{
		strcpy(clockString, &keywordString[7]);
		clockValue	=	atoi(clockString);
		inchValue	=	atof(valueString);
		if ((clockValue >= 0) && (clockValue < kSensorValueCnt))
		{
			gSlitDistance[clockValue].distanceInches	=	inchValue;
			gSlitDistance[clockValue].validData			=	true;
			gSlitDistance[clockValue].updated			=	true;
			gSlitDistance[clockValue].readCount++;

			gUpdateSLitWindow	=	true;
		}
	}
	else if (strncasecmp(keywordString, "gravity_", 8) == 0)
	{
	//	CONSOLE_DEBUG_W_STR("Gravity vector:", keywordString);
		gravityVectorChar	=	keywordString[8];
		gravityValue		=	atof(valueString);

		switch(gravityVectorChar)
		{
			case 'X':
				cGravity_X	=	gravityValue;
				break;

			case 'Y':
				cGravity_Y	=	gravityValue;
				break;

			case 'Z':
				cGravity_Z	=	gravityValue;
				break;

			case 'T':
				cGravity_T	=	gravityValue;
				if ((cGravity_T >= 9.7) && (cGravity_T <= 9.9))
				{
					cValidGravity	=	true;
				}
				else
				{
					cValidGravity	=	false;
					CONSOLE_DEBUG_W_DBL("Gravity vector is invalid:", cGravity_T);
				}

				if (cValidGravity)
				{
				double	telescopeElev;
				double	telescopeElev_deg;

					cUpAngle_Rad	=	atan2(cGravity_Z, cGravity_X);
				//	cUpAngle_Rad	=	atan2(cGravity_X, cGravity_Z);
					cUpAngle_Deg	=	cUpAngle_Rad * 180.0 / M_PI;
//							CONSOLE_DEBUG_W_DBL("cUpAngle_Deg\t=", cUpAngle_Deg);

					cUpAngle_Deg	+=	102.858;

//							CONSOLE_DEBUG_W_DBL("cUpAngle_Deg\t=", cUpAngle_Deg);

				//	telescopeElev		=	atan2(cGravity_Z, cGravity_Y);
					telescopeElev		=	atan2(cGravity_Y, cGravity_Z);
					telescopeElev_deg	=	telescopeElev * 180.0 / M_PI;
					telescopeElev_deg	+=	180.0;
					telescopeElev_deg	=	360.0 - telescopeElev_deg;
//							CONSOLE_DEBUG_W_DBL("telescopeElev_deg\t=", telescopeElev_deg);

				}
				break;

			default:
				CONSOLE_DEBUG_W_STR("Gravity vector error:", keywordString);
				break;
		}
	}
}

//*****************************************************************************
void	ControllerSlit::AlpacaProcessReadAll(	const char	*deviceType,
												const int	deviceNum,
												const char	*keywordString,
												const char *valueString)
{
//	CONSOLE_DEBUG_W_2STR("json=",	keywordString, valueString);
	if (strcasecmp(deviceType, "slittracker") == 0)
	{
		ProcessOneReadAllEntry(keywordString, valueString);
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
}

//*****************************************************************************
//*	if this routine gets overloaded, the first part, checking for "readall" must be preserved
//*****************************************************************************
void	ControllerSlit::AlpacaProcessSupportedActions(	const char	*deviceType,
														const int	deviveNum,
														const char	*valueString)
{
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceType);

	if (strcasecmp(deviceType, "slittracker") == 0)
	{
		if (strcasecmp(valueString, "readall") == 0)
		{
			cHas_readall	=	true;
		}

	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
}

//**************************************************************************************
void	ControllerSlit::UpdateCapabilityList(void)
{
//int		boxID;
//int		iii;
//char	textString[80];
//
////	CONSOLE_DEBUG(__FUNCTION__);
//
//	iii	=	0;
//	while (cCapabilitiesList[iii].capabilityName[0] != 0)
//	{
//		boxID	=	kCapabilities_TextBox1 + iii;
//		strcpy(textString,	cCapabilitiesList[iii].capabilityName);
//		strcat(textString,	":\t");
//		strcat(textString,	cCapabilitiesList[iii].capabilityValue);
//
////		CONSOLE_DEBUG(textString);
//
//		if (boxID <= kCapabilities_TextBoxN)
//		{
//			SetWidgetText(kTab_Capabilities, boxID, textString);
//		}
//
//		iii++;
//	}
}


////*****************************************************************************
//void	ControllerSlit::AlpacaGetSlitTrackerReadAll(void)
//{
//SJP_Parser_t	jsonParser;
//bool			validData;
//char			alpacaString[128];
//int				jjj;
//int				clockValue;
//char			clockString[48];
//double			inchValue;
//struct tm		*linuxTime;
//char			slitLogFileName[48];
//double			gravityValue;
//char			gravityVectorChar;
//
////	CONSOLE_DEBUG(__FUNCTION__);
//
//	SJP_Init(&jsonParser);
//	sprintf(alpacaString,	"/api/v1/%s/%d/%s", "slittracker", cSlitTrackerAlpacaDevNum, "readall");
//
//	CONSOLE_DEBUG_W_STR("alpacaString\t=", alpacaString);
//	CONSOLE_DEBUG_W_NUM("cSlitTrackerPort\t=", cSlitTrackerPort);
//	CONSOLE_ABORT(__FUNCTION__);
//
//
//	validData	=	GetJsonResponse(	&cSlitTrackerDeviceAddress,
//										cSlitTrackerPort,
//										alpacaString,
//										NULL,
//										&jsonParser);
//	if (validData)
//	{
//		cLastAlpacaErrNum	=	kASCOM_Err_Success;
//		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
//		{
////			CONSOLE_DEBUG_W_STR(	jsonParser.dataList[jjj].keyword,
////									jsonParser.dataList[jjj].valueString);
//			//	"sensor-0":28.310000,
//
//			if (strncasecmp(jsonParser.dataList[jjj].keyword, "sensor-", 6) == 0)
//			{
//				strcpy(clockString, &jsonParser.dataList[jjj].keyword[7]);
//				clockValue	=	atoi(clockString);
//				inchValue	=	atof(jsonParser.dataList[jjj].valueString);
//				if ((clockValue >= 0) && (clockValue < kSensorValueCnt))
//				{
//					gSlitDistance[clockValue].distanceInches	=	inchValue;
//					gSlitDistance[clockValue].validData			=	true;
//					gSlitDistance[clockValue].updated			=	true;
//					gSlitDistance[clockValue].readCount++;
//
//					gUpdateSLitWindow	=	true;
//				}
//			}
//			else if (strncasecmp(jsonParser.dataList[jjj].keyword, "gravity_", 8) == 0)
//			{
//			//	CONSOLE_DEBUG_W_STR("Gravity vector:", jsonParser.dataList[jjj].keyword);
//				gravityVectorChar	=	jsonParser.dataList[jjj].keyword[8];
//				gravityValue		=	atof(jsonParser.dataList[jjj].valueString);
//
//				switch(gravityVectorChar)
//				{
//					case 'X':
//						cGravity_X	=	gravityValue;
//						break;
//
//					case 'Y':
//						cGravity_Y	=	gravityValue;
//						break;
//
//					case 'Z':
//						cGravity_Z	=	gravityValue;
//						break;
//
//					case 'T':
//						cGravity_T	=	gravityValue;
//						if ((cGravity_T >= 9.7) && (cGravity_T <= 9.9))
//						{
//							cValidGravity	=	true;
//						}
//						else
//						{
//							cValidGravity	=	false;
//							CONSOLE_DEBUG_W_DBL("Gravity vector is invalid:", cGravity_T);
//						}
//
//						if (cValidGravity)
//						{
//						double	telescopeElev;
//						double	telescopeElev_deg;
//
//							cUpAngle_Rad	=	atan2(cGravity_Z, cGravity_X);
//						//	cUpAngle_Rad	=	atan2(cGravity_X, cGravity_Z);
//							cUpAngle_Deg	=	cUpAngle_Rad * 180.0 / M_PI;
////							CONSOLE_DEBUG_W_DBL("cUpAngle_Deg\t=", cUpAngle_Deg);
//
//							cUpAngle_Deg	+=	102.858;
//
////							CONSOLE_DEBUG_W_DBL("cUpAngle_Deg\t=", cUpAngle_Deg);
//
//						//	telescopeElev		=	atan2(cGravity_Z, cGravity_Y);
//							telescopeElev		=	atan2(cGravity_Y, cGravity_Z);
//							telescopeElev_deg	=	telescopeElev * 180.0 / M_PI;
//							telescopeElev_deg	+=	180.0;
//							telescopeElev_deg	=	360.0 - telescopeElev_deg;
////							CONSOLE_DEBUG_W_DBL("telescopeElev_deg\t=", telescopeElev_deg);
//
//						}
//						break;
//
//					default:
//						CONSOLE_DEBUG_W_STR("Gravity vector error:", jsonParser.dataList[jjj].keyword);
//						break;
//				}
//			}
//		}
//		//*	update the time of the last data
//		gettimeofday(&cSlitTrackerLastUpdateTime, NULL);
//
//		UpdateSlitLog();
//
//
//		if (cSlitTrackerCommFailed)
//		{
//			//*	set the indicators back to OK
//			SetWidgetBGColor(	kTab_SlitTracker,	kSlitTracker_RemoteAddress, CV_RGB(0,0,0));
//			SetWidgetTextColor(	kTab_SlitTracker,	kSlitTracker_RemoteAddress, CV_RGB(255,0,0));
//
//		}
//		cSlitTrackerCommFailed	=	false;
//	}
//	else
//	{
////++		SetWidgetText(kTab_SlitTracker,			kSlitTracker_ErrorMsg, "Failed to read data from Slit Tracker");
//		SetWidgetBGColor(kTab_SlitTracker,		kSlitTracker_RemoteAddress, CV_RGB(255,0,0));
//		SetWidgetTextColor(kTab_SlitTracker,	kSlitTracker_RemoteAddress, CV_RGB(0,0,0));
//		cSlitTrackerCommFailed	=	true;
//	}
//}

//*****************************************************************************
void	ControllerSlit::UpdateSlitLog(void)
{
double		totalInches;
int			qqq;
int			sampleCnt;
int			jjj;
char		clockString[48];
struct tm	*linuxTime;

	//===================================================
	//*	update the log of the data
	if ((gSlitLogIdx >= 0) && (gSlitLogIdx < kSlitLogCount))
	{
//		CONSOLE_DEBUG_W_NUM("Logging slit data, index\t=", gSlitLogIdx);

		gSlitLog[gSlitLogIdx].validData		=	true;
		for (jjj=0; jjj<kSensorValueCnt; jjj++)
		{

			gSlitLog[gSlitLogIdx].distanceInches[jjj]	=	gSlitDistance[jjj].distanceInches;

			//*	compute the average over the last 20 values
			#define	kAverageCnt	20
			if (gSlitLogIdx < 2)
			{
				//*	cant average only one value
				gSlitLog[gSlitLogIdx].average20pt[jjj]	=	gSlitLog[gSlitLogIdx].distanceInches[jjj];
			}
			else
			{
				totalInches	=	0.0;
				sampleCnt	=	0;
				qqq			=	gSlitLogIdx - kAverageCnt;
				if (qqq < 0)
				{
					qqq	=	0;
				}
				while (qqq < gSlitLogIdx)
				{
					totalInches	+=	gSlitLog[qqq].distanceInches[jjj];
					sampleCnt++;
					qqq++;
				}
				gSlitLog[gSlitLogIdx].average20pt[jjj]	=	totalInches / sampleCnt;
			}
		}
		gSlitLogIdx++;

		//*	check to see if we are at the end
		if (gSlitLogIdx >= kSlitLogCount)
		{
		const int	moveBack	=	100;

			CONSOLE_DEBUG("Slit distance buffer is full, moving back");
			//*	move everything back by 100
			for (jjj=0; jjj< (kSlitLogCount - moveBack); jjj++)
			{
				gSlitLog[jjj]	=	gSlitLog[jjj + moveBack];
			}
			gSlitLogIdx	=	kSlitLogCount - moveBack;

			//*	set the rest of the data back to zero
			while (jjj < kSlitLogCount)
			{
				memset(&gSlitLog[jjj], 0, sizeof(TYPE_SLIT_LOG));
				jjj++;
			}
			CONSOLE_DEBUG("Done with move back");
		}
	}
	else
	{
		CONSOLE_DEBUG("No room in slit log table");
//			exit(0);
	}


	linuxTime		=	localtime(&cSlitTrackerLastUpdateTime.tv_sec);

	sprintf(clockString,	"Last update %02d:%02d:%02d",
							linuxTime->tm_hour,
							linuxTime->tm_min,
							linuxTime->tm_sec);
	SetWidgetText(kTab_SlitTracker, kSlitTracker_LastUpdate, clockString);
}

//*****************************************************************************
void	ControllerSlit::CloseSlitTrackerDataFile(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	if (cSlitDataLogFilePtr != NULL)
	{
		fflush(cSlitDataLogFilePtr);
		fclose(cSlitDataLogFilePtr);
		cSlitDataLogFilePtr	=	NULL;
	}
	cLogSlitData	=	false;
}


//*****************************************************************************
void	ControllerSlit::ToggleSwitchState(const int switchNum)
{
//SJP_Parser_t	jsonParser;
//bool			validData;
//char			alpacaString[128];
//char			dataString[128];


}

#ifdef _SLIT_TRACKER_DIRECT_
#define	kReadBufferSize		1024
#define	kLineBuffSize		64
int				gSlitTrackerfileDesc	=	-1;				//*	port file descriptor
char			gSlitTrackerLineBuf[kLineBuffSize];
int				gSLitTrackerByteCnt		=	0;
unsigned long	gLastSlitUpdate_MS		=	0;
//*****************************************************************************
void	OpenSlitTrackerPort(void)
{
char	usbPortPath[32]	=	"/dev/ttyACM0";

	CONSOLE_DEBUG(__FUNCTION__);

	gLastSlitUpdate_MS		=	millis();
	gSlitTrackerfileDesc	=	open(usbPortPath, O_RDWR);	//* connect to port
	if (gSlitTrackerfileDesc >= 0)
	{
		Set_Serial_attribs(gSlitTrackerfileDesc, B9600, 0);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open port", usbPortPath);
	}
}

//*****************************************************************************
void	ProcessSlitTrackerLine(char *lineBuff)
{
int				clockValue;
char			*inchesPtr;
double			inchValue;
unsigned long	deltaMilliSecs;

	CONSOLE_DEBUG(lineBuff);

	if ((lineBuff[0] == '=') && (isdigit(lineBuff[1])))
	{
		clockValue	=	atoi(&lineBuff[1]);
		if ((clockValue >= 0) && (clockValue < 12))
		{
			//	0	Distance: 151.25 cm	Inches: 59.55 delta: -0.04
			inchesPtr	=	strstr(lineBuff, "Inches");
			if (inchesPtr != NULL)
			{
				inchesPtr	+=	7;
				while ((*inchesPtr == 0x20) || (*inchesPtr == 0x09))
				{
					inchesPtr++;
				}
				inchValue	=	atof(inchesPtr);
//				CONSOLE_DEBUG_W_DBL("inchValue\t=", inchValue);

				gSlitDistance[clockValue].distanceInches	=	inchValue;
				gSlitDistance[clockValue].validData			=	true;
				gSlitDistance[clockValue].updated			=	true;
				gSlitDistance[clockValue].readCount++;


				deltaMilliSecs	=	millis() - gLastSlitUpdate_MS;
				if (deltaMilliSecs > 1000)
				{
					gUpdateSLitWindow	=	true;
					gLastSlitUpdate_MS	=	millis();
				}
			}
			if (clockValue == 0)
			{
//				UpdateSlitLog();
			}
		}
		else
		{
			CONSOLE_DEBUG_W_STR("clockValue error\t=", lineBuff);
		}
	}
}


//*****************************************************************************
void	GetSLitTrackerData(void)
{
int		readCnt;
char	readBuffer[kReadBufferSize];
char	theChar;
bool	keepGoing;
int		iii;
int		charsRead;


//	CONSOLE_DEBUG(__FUNCTION__);
	if (gSlitTrackerfileDesc >= 0)
	{
		keepGoing	=	true;
		readCnt		=	0;
		while (keepGoing && (readCnt < 10))
		{
			charsRead	=	read(gSlitTrackerfileDesc, readBuffer, (kReadBufferSize - 2));
			if (charsRead > 0)
			{
				readCnt++;
				for (iii=0; iii<charsRead; iii++)
				{
					theChar		=	readBuffer[iii];
					if ((theChar >= 0x20) || (theChar == 0x09))
					{
						if (gSLitTrackerByteCnt < (kLineBuffSize - 2))
						{
							gSlitTrackerLineBuf[gSLitTrackerByteCnt++]	=	theChar;
							gSlitTrackerLineBuf[gSLitTrackerByteCnt]	=	0;
						}
					}
					else if (theChar == 0x0d)
					{
						gSlitTrackerLineBuf[gSLitTrackerByteCnt]	=	0;
						if (strlen(gSlitTrackerLineBuf) > 0)
						{
							ProcessSlitTrackerLine(gSlitTrackerLineBuf);
						}
						gSLitTrackerByteCnt							=	0;
						gSlitTrackerLineBuf[gSLitTrackerByteCnt]	=	0;
					}
				}
			}
			else
			{
				keepGoing	=	false;
			}
		}
//		CONSOLE_DEBUG_W_NUM("readCnt\t=", readCnt);
		if (readCnt > 9)
		{
			//*	slow the read rate down
			SendSlitTrackerCmd("+");
		}
	}
	else
	{
//		CONSOLE_DEBUG("Slit tracker port not open");
	}
}


//*****************************************************************************
void	SendSlitTrackerCmd(const char *cmdBuffer)
{
int	sLen;
ssize_t	bytesWritten;

	CONSOLE_DEBUG_W_STR("cmdBuffer\t=", cmdBuffer);

	if (gSlitTrackerfileDesc >= 0)
	{
		sLen			=	strlen(cmdBuffer);
		bytesWritten	=	write(gSlitTrackerfileDesc, cmdBuffer, sLen);
		if (bytesWritten < 0)
		{
			CONSOLE_DEBUG("write returned error");
		}
	}
}
#endif // _SLIT_TRACKER_DIRECT_

