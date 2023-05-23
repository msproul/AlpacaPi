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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Oct  6,	2021	<MLS> Created controller_slit.cpp (copied from controller_dome.cpp)
//*	Oct  6,	2021	<MLS> Slit tracker controller window starting to work
//*	Mar  8,	2023	<MLS> Added SlitDome tab to keep track of dome object being controlled
//*	Mar  8,	2023	<MLS> Added GetDomeData_Startup()
//*	Mar  9,	2023	<MLS> Added ProcessOneReadAllEntryDome()
//*	Mar  9,	2023	<MLS> Added GetDomeData_Periodic()
//*	Mar 10,	2023	<MLS> Added AlpacaDisplayErrorMessage() to slittracker controller
//*	Mar 10,	2023	<MLS> Changed dome update to 30 seconds
//*	Apr  1,	2023	<MLS> Made improvements to dome update logic
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


//**************************************************************************************
enum
{
	kTab_SlitTracker	=	1,
	kTab_DomeInfo,
	kTab_SlitGraph,
	kTab_DriverInfo,
	kTab_About,

	kTab_Slit_Count

};


	bool	gUpdateSLitWindow	=	true;


#define		kSlitWindowWidth	475
#define		kSLitWindowHeight	750


//**************************************************************************************
ControllerSlit::ControllerSlit(	const char			*argWindowName,
								TYPE_REMOTE_DEV		*alpacaDevice)
	:Controller(argWindowName, kSlitWindowWidth,  kSLitWindowHeight)
{
int		iii;
char	ipAddrStr[32];

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cAlpacaDeviceTypeStr,	"slittracker");

	cLogSlitData				=	false;
	cSlitDataLogFilePtr			=	NULL;
	cValidGravity				=	false;
	cEnableAutomaticDomeUpdates	=	false;
	cForceDomeUpdate			=	false;

	//*	window object ptrs
	cSlitTrackerTabObjPtr		=	NULL;
	cSlitGraphTabObjPtr			=	NULL;
	cAboutBoxTabObjPtr			=	NULL;

	cFirstDataRead				=	true;
	cLastUpdate_milliSecs		=	millis();
	cLastDomeUpdate_milliSecs	=	millis();
	cUpdateDelta_Secs			=	kDefaultUpdateDelta;
	cDomeUpdateDelta_Secs		=	30;

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

	//*	init dome address info
	cDomeIPaddressString[0]	=	0;
	cDomeAlpacaPort			=	0;
	cDomeAlpacaDevNum		=	0;
	cDomeHas_Readall		=	false;
	memset(&cSlitProp, 0, sizeof(TYPE_SlittrackerProperties));
	memset(&cDomeProp, 0, sizeof(TYPE_DomeProperties));
	cDomeProp.ShutterStatus	=	kShutterStatus_Unknown;


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
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
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
	SetTabText(kTab_DomeInfo,		"Dome Info");
	SetTabText(kTab_DriverInfo,		"Driver Info");
	SetTabText(kTab_About,			"About");

	//=============================================================
	cSlitTrackerTabObjPtr		=	new WindowTabSlitTracker(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSlitTrackerTabObjPtr != NULL)
	{
		SetTabWindow(kTab_SlitTracker,	cSlitTrackerTabObjPtr);
		cSlitTrackerTabObjPtr->SetParentObjectPtr(this);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create WindowTabSlitTracker!!!");
		CONSOLE_ABORT(__FUNCTION__);
	}

	//=============================================================
	cSlitDomeTabObjPtr		=	new WindowTabSlitDome(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSlitDomeTabObjPtr != NULL)
	{
		SetTabWindow(kTab_DomeInfo,	cSlitDomeTabObjPtr);
		cSlitDomeTabObjPtr->SetParentObjectPtr(this);
		cSlitDomeTabObjPtr->SetDomePropertiesPtr(&cDomeProp);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create WindowTabSlitDome!!!");
		CONSOLE_ABORT(__FUNCTION__);
	}

	//=============================================================
	cSlitGraphTabObjPtr		=	new WindowTabSlitGraph(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSlitGraphTabObjPtr != NULL)
	{
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

	//*	display the IP address/port
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
	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Name,				cCommonProp.Name);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Description,		cCommonProp.Description);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
	SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);
}


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
		AlpacaGetStartupData();
		AlpacaGetCommonProperties_OneAAT("slittracker");
		UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);
		cReadStartup	=	false;

	}


	needToUpdate	=	false;
	currentMillis	=	millis();
	deltaSeconds	=	(currentMillis - cLastUpdate_milliSecs) / 1000;

	if (cFirstDataRead || (deltaSeconds >= cUpdateDelta_Secs))
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
//		CONSOLE_DEBUG("Updating");
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

	//========================================================
	//*	now check on the dome updates
	if (cEnableAutomaticDomeUpdates)
	{
		currentMillis	=	millis();
		deltaSeconds	=	(currentMillis - cLastDomeUpdate_milliSecs) / 1000;

		if (cForceDomeUpdate || (deltaSeconds > cDomeUpdateDelta_Secs))
		{
			GetDomeData_Periodic();
			cLastDomeUpdate_milliSecs	=	millis();
			cForceDomeUpdate			=	false;
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
void	ControllerSlit::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
	SetWidgetText(kTab_DomeInfo,	kSlitDome_AlpacaErrorMsg,		errorMsgString);

	if (strlen(errorMsgString) > 0)
	{
		CONSOLE_DEBUG_W_STR("Alpaca error=", errorMsgString);
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
		SetWidgetValid(kTab_DomeInfo,		kSlitDome_Readall,		cHas_readall);
		SetWidgetValid(kTab_SlitGraph,		kSlitGraph_Readall,		cHas_readall);

		if (cHas_readall == false)
		{
			validData	=	AlpacaGetStringValue(	"slittracker", "driverversion",	NULL,	returnString);
			if (validData)
			{
				strcpy(cAlpacaVersionString, returnString);
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

//	CONSOLE_DEBUG(__FUNCTION__);

	validData			=	false;
	previousOnLineState	=	cOnLine;
//	CONSOLE_DEBUG_W_BOOL("cHas_readall\t=", cHas_readall);
	if (cHas_readall)
	{
//		CONSOLE_DEBUG("Has readall");
		validData	=	AlpacaGetStatus_ReadAll("slittracker", cAlpacaDevNum);
	}
	else
	{
		CONSOLE_DEBUG("No readall");
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
		cUpdateDelta_Secs	=	kDefaultUpdateDelta;
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


	if (strcasecmp(keywordString, "trackingenabled") == 0)
	{
		cSlitProp.TrackingEnabled	=	IsTrueFalse(valueString);
		if (cSlitDomeTabObjPtr != NULL)
		{
			cSlitDomeTabObjPtr->UpdateButtons(kSlitDome_DomeEnableTracking,	cSlitProp.TrackingEnabled);
		}
	}
	else if (strncasecmp(keywordString, "sensor-", 6) == 0)
	{
		strcpy(clockString, &keywordString[7]);
		clockValue	=	atoi(clockString);
		inchValue	=	AsciiToDouble(valueString);
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
		gravityValue		=	AsciiToDouble(valueString);

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
//					CONSOLE_DEBUG_W_DBL("cUpAngle_Deg\t=", cUpAngle_Deg);

					cUpAngle_Deg	+=	102.858;

//					CONSOLE_DEBUG_W_DBL("cUpAngle_Deg\t=", cUpAngle_Deg);

				//	telescopeElev		=	atan2(cGravity_Z, cGravity_Y);
					telescopeElev		=	atan2(cGravity_Y, cGravity_Z);
					telescopeElev_deg	=	telescopeElev * 180.0 / M_PI;
					telescopeElev_deg	+=	180.0;
					telescopeElev_deg	=	360.0 - telescopeElev_deg;
//					CONSOLE_DEBUG_W_DBL("telescopeElev_deg\t=", telescopeElev_deg);

				}
				break;

			default:
				CONSOLE_DEBUG_W_STR("Gravity vector error:", keywordString);
				break;
		}
	}
	else if (strcasecmp(keywordString, "domeaddress") == 0)
	{
	char *colonPtr;
	char *slashPtr;

		if (strlen(valueString) < 45)
		{
			strcpy(cDomeIPaddressString, valueString);
			//*	look for the IP port number
			colonPtr	=	strchr(cDomeIPaddressString, ':');
			if (colonPtr != NULL)
			{
				*colonPtr	=	0;	//*	terminate the IP address
				colonPtr++;
				cDomeAlpacaPort	=	atoi(colonPtr);

				SetWidgetText(	kTab_DomeInfo, kSlitDome_DomeIPaddr,	cDomeIPaddressString);
				SetWidgetNumber(kTab_DomeInfo, kSlitDome_DomeAlpacaPort,cDomeAlpacaPort);
			}
			//*	now look for the Alpaca device number
			slashPtr	=	strchr(cDomeIPaddressString, '/');
			if (slashPtr != NULL)
			{
				slashPtr++;
				cDomeAlpacaDevNum	=	atoi(colonPtr);
				SetWidgetNumber(kTab_DomeInfo, kSlitDome_DomeDevNum,	cDomeAlpacaDevNum);
			}
		}
		else
		{
			CONSOLE_DEBUG_W_STR("Argument string too long:", valueString);
		}
	}
}

//*****************************************************************************
void	ControllerSlit::ProcessOneReadAllEntryDome(	const char	*keywordString,
													const char *valueString)
{
char		statusString[64];

//	CONSOLE_DEBUG_W_STR(keywordString, valueString);

	if (strcasecmp(keywordString, "athome") == 0)
	{
		cDomeProp.AtHome	=	IsTrueFalse(valueString);
		if (cDomeProp.AtHome)
		{
			SetWidgetText(		kTab_DomeInfo, kSlitDome_DomePosition, "Home");
			SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomePosition,	CV_RGB(0, 255, 0));
		}
		cDomeReadAllCount++;
	}
	else if (strcasecmp(keywordString, "atpark") == 0)
	{
		cDomeProp.AtPark	=	IsTrueFalse(valueString);
		if (cDomeProp.AtPark)
		{
			SetWidgetText(		kTab_DomeInfo, kSlitDome_DomePosition, "Park");
			SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomePosition,	CV_RGB(0, 255, 0));
		}
		cDomeReadAllCount++;
	}
	else if (strcasecmp(keywordString, "azimuth") == 0)
	{
		cDomeProp.Azimuth	=	atof(valueString);
		sprintf(statusString, "%1.1f", cDomeProp.Azimuth);
		SetWidgetText(		kTab_DomeInfo, kSlitDome_DomeAzimuth, statusString);
		SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomeAzimuth,	CV_RGB(0, 255, 0));
		cDomeReadAllCount++;
	}
	else if (strcasecmp(keywordString, "slewing") == 0)
	{
		cDomeProp.Slewing	=	IsTrueFalse(valueString);
		if (cDomeProp.Slewing)
		{
			SetWidgetText(		kTab_DomeInfo, kSlitDome_DomePosition, "Slewing");
			SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomePosition,	CV_RGB(0, 255, 0));
		}
		cDomeReadAllCount++;
	}

	else if (strcasecmp(keywordString, "shutterstatus") == 0)
	{
		cDomeProp.ShutterStatus	=	(TYPE_ShutterStatus)atoi(valueString);
		GetDomeShutterStatusString(cDomeProp.ShutterStatus, statusString);

		SetWidgetText(		kTab_DomeInfo, kSlitDome_DomeShutter,	statusString);
		SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomeShutter,	CV_RGB(0, 255, 0));
		cDomeReadAllCount++;
	}
}

//*****************************************************************************
void	ControllerSlit::AlpacaProcessReadAll(	const char	*deviceType,
												const int	deviceNum,
												const char	*keywordString,
												const char *valueString)
{
//	CONSOLE_DEBUG_W_2STR(deviceType,	keywordString, valueString);
	if (strcasecmp(deviceType, "slittracker") == 0)
	{
		ProcessOneReadAllEntry(keywordString, valueString);
	}
	else if (strcasecmp(deviceType, "dome") == 0)
	{
		ProcessOneReadAllEntryDome(keywordString, valueString);
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
	else if (strcasecmp(deviceType, "dome") == 0)
	{
		if (strcasecmp(valueString, "readall") == 0)
		{
			cDomeHas_Readall	=	true;
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
//	UpdateCapabilityListID(kTab_Capabilities, kCapabilities_TextBox1, kCapabilities_TextBoxN);
}

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
//		exit(0);
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
//*****************************************************************************
void	ControllerSlit::SetButtonOption(const int widgetBtnIdx, const bool newState)
{
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("widgetBtnIdx\t=",	widgetBtnIdx);
//	CONSOLE_DEBUG_W_BOOL("newState   \t=",	newState);
	switch(widgetBtnIdx)
	{
		case kSlitDome_DomeEnableData:
			cEnableAutomaticDomeUpdates	=	newState;
			if (cEnableAutomaticDomeUpdates)
			{
				//*	force it to read now
				cForceDomeUpdate	=	true;
			}
			break;

		default:
			CONSOLE_ABORT(__FUNCTION__);
			break;
	}
	if (cSlitDomeTabObjPtr != NULL)
	{
		cSlitDomeTabObjPtr->UpdateButtons(kSlitDome_DomeEnableData,		cEnableAutomaticDomeUpdates);
	}
}

//*****************************************************************************
void	ControllerSlit::GetDomeData_Periodic(void)
{
bool				validData;
struct sockaddr_in	deviceAddress;

	CONSOLE_DEBUG(__FUNCTION__);
	inet_pton(AF_INET, cDomeIPaddressString, &(deviceAddress.sin_addr));

	//------------------------
	CONSOLE_DEBUG_W_BOOL("cDomeHas_Readall\t=",	cDomeHas_Readall);
	if (cDomeHas_Readall)
	{
//		CONSOLE_DEBUG("Calling AlpacaGetStatus_ReadAll()");
		cDomeReadAllCount	=	0;
		validData	=	AlpacaGetStatus_ReadAll(&deviceAddress,
												cDomeAlpacaPort,
												"dome",
												cDomeAlpacaDevNum,
												false);	//*	enable debug
		CONSOLE_DEBUG_W_NUM("cDomeReadAllCount\t=",	cDomeReadAllCount);
		if (validData)
		{
			if ((cDomeProp.AtHome == false) && (cDomeProp.AtPark == false) && (cDomeProp.Slewing == false))
			{
				SetWidgetText(kTab_DomeInfo, kSlitDome_DomePosition, "Stopped");
			}
		}
	}
	else
	{
	bool	rtnValidData;
	int		returedIntger;
	double	returedDouble;
	bool	returedBoolean;
	char	statusString[32];

		CONSOLE_DEBUG("Dome does not have ReadAll!!!!");

		//*	get data one at a time
		//------------------------------------------------
		//*	get shutter status
		validData	=	AlpacaGetIntegerValue(	deviceAddress,
												cDomeAlpacaPort,
												cDomeAlpacaDevNum,
												"dome",
												"shutterstatus",
												NULL,
												&returedIntger,
												&rtnValidData);
		if (validData && rtnValidData)
		{
			cDomeProp.ShutterStatus	=	(TYPE_ShutterStatus)returedIntger;
			GetDomeShutterStatusString(cDomeProp.ShutterStatus, statusString);

			SetWidgetText(		kTab_DomeInfo, kSlitDome_DomeShutter,	statusString);
			SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomeShutter,	CV_RGB(255, 255, 0));
		}
		else
		{
			CONSOLE_DEBUG("Error getting shutterstatus");
		}
		//------------------------------------------------
		//*	get dome azimuth
		validData	=	AlpacaGetDoubleValue(	deviceAddress,
												cDomeAlpacaPort,
												cDomeAlpacaDevNum,
												"dome",
												"azimuth",
												NULL,
												&returedDouble,
												&rtnValidData);
		if (validData && rtnValidData)
		{
			cDomeProp.Azimuth	=	returedDouble;
			sprintf(statusString, "%1.1f", cDomeProp.Azimuth);
			SetWidgetText(		kTab_DomeInfo, kSlitDome_DomeAzimuth, statusString);
			SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomeAzimuth,	CV_RGB(255, 255, 0));
		}
		else
		{
			CONSOLE_DEBUG("Error getting azimuth");
		}
		//------------------------------------------------
		//*	get dome atHome
		validData	=	AlpacaGetBooleanValue(	deviceAddress,
												cDomeAlpacaPort,
												cDomeAlpacaDevNum,
												"dome",
												"athome",
												NULL,
												&returedBoolean,
												&rtnValidData);
		if (validData && rtnValidData)
		{
			cDomeProp.AtHome	=	returedBoolean;
			if (cDomeProp.AtHome)
			{
				SetWidgetText(		kTab_DomeInfo, kSlitDome_DomePosition, "Home");
				SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomePosition,	CV_RGB(255, 255, 0));
			}
		}
		else
		{
			CONSOLE_DEBUG("Error getting athome");
		}
		//------------------------------------------------
		//*	get dome atpark
		validData	=	AlpacaGetBooleanValue(	deviceAddress,
												cDomeAlpacaPort,
												cDomeAlpacaDevNum,
												"dome",
												"atpark",
												NULL,
												&returedBoolean,
												&rtnValidData);
		if (validData && rtnValidData)
		{
			cDomeProp.AtPark	=	returedBoolean;
			if (cDomeProp.AtPark)
			{
				SetWidgetText(		kTab_DomeInfo, kSlitDome_DomePosition, "Park");
				SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomePosition,	CV_RGB(255, 255, 0));
			}
		}
		else
		{
			CONSOLE_DEBUG("Error getting atpark");
		}
		//------------------------------------------------
		//*	get dome Slewing
		validData	=	AlpacaGetBooleanValue(	deviceAddress,
												cDomeAlpacaPort,
												cDomeAlpacaDevNum,
												"dome",
												"slewing",
												NULL,
												&returedBoolean,
												&rtnValidData);
		if (validData && rtnValidData)
		{
			cDomeProp.Slewing	=	returedBoolean;
			if (cDomeProp.Slewing)
			{
				SetWidgetText(		kTab_DomeInfo, kSlitDome_DomePosition, "Slewing");
				SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomePosition,	CV_RGB(255, 255, 0));
			}
		}
		else
		{
			CONSOLE_DEBUG("Error getting atpark");
		}
		if ((cDomeProp.AtHome == false) && (cDomeProp.AtPark == false) && (cDomeProp.Slewing == false))
		{
			SetWidgetText(		kTab_DomeInfo, kSlitDome_DomePosition, "Stopped");
			SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomePosition,	CV_RGB(255, 255, 0));
		}
	}
}

//*****************************************************************************
void	ControllerSlit::GetDomeData_Startup(void)
{
bool				validData;
bool				rtnValidData;
struct sockaddr_in	deviceAddress;
char				returnDataString[128];

	CONSOLE_DEBUG(__FUNCTION__);

	SetWidgetText(kTab_DomeInfo, kSlitDome_DomeName,		"---");
	SetWidgetText(kTab_DomeInfo, kSlitDome_DomeDescription,	"---");
	SetWidgetText(kTab_DomeInfo, kSlitDome_DomeShutter,		"---");
	SetWidgetText(kTab_DomeInfo, kSlitDome_DomePosition,	"---");
	SetWidgetText(kTab_DomeInfo, kSlitDome_DomeAzimuth,		"---");

	CONSOLE_DEBUG_W_STR("cDomeIPaddressString\t=",	cDomeIPaddressString);
	CONSOLE_DEBUG_W_NUM("cDomeAlpacaPort     \t=",	cDomeAlpacaPort);
	CONSOLE_DEBUG_W_NUM("cAlpacaDevNum       \t=",	cAlpacaDevNum);

	inet_pton(AF_INET, cDomeIPaddressString, &(deviceAddress.sin_addr));

	//*	first lets get supported actions
	cDomeHas_Readall	=	false;
	validData			=	AlpacaGetSupportedActions(	&deviceAddress,
												cDomeAlpacaPort,
												"dome",
												cAlpacaDevNum);
//	if (validData)
//	{
//
//	}
	//------------------------
	validData	=	AlpacaGetStringValue(	deviceAddress,
											cDomeAlpacaPort,
											cDomeAlpacaDevNum,
											"dome",
											"name",
											NULL,
											returnDataString,
											&rtnValidData);
	if (validData && returnDataString)
	{
		SetWidgetText(		kTab_DomeInfo, kSlitDome_DomeName,	returnDataString);
		SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomeName,	CV_RGB(0, 255, 0));
	}
	else
	{
		CONSOLE_DEBUG_W_BOOL("validData   \t=",	validData);
		CONSOLE_DEBUG_W_BOOL("rtnValidData\t=",	rtnValidData);
		SetWidgetText(		kTab_DomeInfo, kSlitDome_DomeName,	"failed");
		SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomeName,	CV_RGB(255,	0,	0));
	}
	//------------------------
	validData	=	AlpacaGetStringValue(	deviceAddress,
											cDomeAlpacaPort,
											cDomeAlpacaDevNum,
											"dome",
											"description",
											NULL,
											returnDataString,
											&rtnValidData);
	if (validData && returnDataString)
	{
		SetWidgetText(		kTab_DomeInfo, kSlitDome_DomeDescription,	returnDataString);
		SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomeDescription,	CV_RGB(0, 255, 0));
	}
	else
	{
		CONSOLE_DEBUG_W_BOOL("validData   \t=",	validData);
		CONSOLE_DEBUG_W_BOOL("rtnValidData\t=",	rtnValidData);
		SetWidgetText(		kTab_DomeInfo, kSlitDome_DomeDescription,	"failed");
		SetWidgetTextColor(	kTab_DomeInfo, kSlitDome_DomeDescription,	CV_RGB(255,	0,	0));
	}

	GetDomeData_Periodic();
}

//*****************************************************************************
bool	ControllerSlit::SetAlpacaEnableTracking(const bool newState)
{
bool			validData;
SJP_Parser_t	jsonParser;
//int				jjj;
char			parameterString[128];
//int				returnedAlpacaErrNum;
//char			returnedAlpacaErrStr[256];
bool			myReturnDataIsValid;

//	CONSOLE_DEBUG(__FUNCTION__);
	myReturnDataIsValid	=	true;

	sprintf(parameterString, "tracking=%s", (newState ? "True" : "False"));
	validData	=	AlpacaSendPutCmdwResponse(	&cDeviceAddress,
												cPort,
												"slittracker",
												cAlpacaDevNum,
												"trackingenabled",
												parameterString,
												&jsonParser);
	cLastAlpacaErrNum	=	AlpacaCheckForErrors(&jsonParser, cLastAlpacaErrStr);
	if (cLastAlpacaErrNum != kASCOM_Err_Success)
	{
		CONSOLE_DEBUG_W_NUM("cLastAlpacaErrNum        \t=",	cLastAlpacaErrNum);
		myReturnDataIsValid	=	false;
	}

	return(validData);
}

