//*****************************************************************************
//*		controller_dome.cpp		(c) 2020 by Mark Sproul
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
//*	Apr 18,	2020	<MLS> Created controller_dome.cpp
//*	Apr 18,	2020	<MLS> Started on dome controller app
//*	Apr 18,	2020	<MLS> Dome controller communicating with dome
//*	Apr 19,	2020	<MLS> Dome controller working with ReadAll or One at a time
//*	Apr 21,	2020	<MLS> Added About box to dome controller
//*	May  1,	2020	<MLS> Added processing for shutter and slaved status
//*	May  1,	2020	<MLS> Added AlpacaGetShutterReadAll()
//*	May  2,	2020	<MLS> Added SetAlpacaSlitTrackerInfo()
//*	May  2,	2020	<MLS> Slit tracker display working through the network
//*	May  8,	2020	<MLS> Added UpdateShutterAltitude()
//*	May  8,	2020	<MLS> Added Slit distance logging to disk
//*	May  9,	2020	<MLS> Added Slit distance logging to memory for real time graphing
//*	May 10,	2020	<MLS> Added Slit Graph window tab
//*	May 10,	2020	<MLS> Added Slit graphing and averaging
//*	May 23,	2020	<MLS> Added UpdateSlitLog()
//*	May 31,	2020	<MLS> Added gravity vector processing
//*	Jan 14,	2021	<MLS> Dome controller working with ASCOM/Remote
//*****************************************************************************


#ifdef _ENABLE_CTRL_DOME_



#include	<math.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<sys/time.h>


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include	"alpaca_defs.h"

#include	"discovery_lib.h"
#include	"sendrequest_lib.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	450
#define	kWindowHeight	720

#include	"controller.h"
#include	"windowtab_dome.h"
#include	"windowtab_slit.h"
#include	"windowtab_slitgraph.h"
#include	"windowtab_about.h"
#include	"controller_dome.h"


TYPE_SLITCLOCK	gSlitDistance[kSensorValueCnt];		//*	current reading
TYPE_SLIT_LOG	gSlitLog[kSlitLogCount];			//*	log of readings
int				gSlitLogIdx;

//#define	_SLIT_TRACKER_DIRECT_

#ifdef _SLIT_TRACKER_DIRECT_
	#include	<termios.h>
	#include	<fcntl.h>
	#include	"serialport.h"

	void	OpenSlitTrackerPort(void);
	void	GetSLitTrackerData(void);
	void	SendSlitTrackerCmd(const char *cmdBuffer);
#endif // _SLIT_TRACKER_DIRECT_
	bool	gUpdateSLitWindow	=	true;



#define	kDefaultUpdateDelta	5

//**************************************************************************************
ControllerDome::ControllerDome(	const char			*argWindowName,
								TYPE_REMOTE_DEV		*alpacaDevice)
	:Controller(argWindowName, kWindowWidth,  kWindowHeight)
{
int		iii;

	CONSOLE_DEBUG(__FUNCTION__);

	cCanFindHome			=	false;
	cCanPark				=	false;
	cCanSetAltitude			=	false;
	cCanSetAzimuth			=	false;
	cCanSetPark				=	false;
	cCanSetShutter			=	false;
	cCanSlave				=	false;
	cCanSyncAzimuth			=	false;
	cCanPark				=	false;
	cAtHome					=	false;
	cAtPark					=	false;
	cSlewing				=	false;
	cAzimuth_Dbl			=	0.0;
	cAltitude_Dbl			=	0.0;
	cShutterStatus			=	-1;
#ifdef _ENABLE_EXTERNAL_SHUTTER_
	cShutterInfoValid		=	false;
#endif // _ENABLE_EXTERNAL_SHUTTER_
	cSlitTrackerInfoValid	=	false;
	cLogSlitData			=	false;
	cSlitDataLogFilePtr		=	NULL;
	cValidGravity			=	false;

	//*	window object ptrs
	cDomeTabObjPtr			=	NULL;
	cSlitTrackerTabObjPtr	=	NULL;
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

		strcpy(cAlpacaDeviceTypeStr,	alpacaDevice->deviceTypeStr);
		strcpy(cAlpacaDeviceNameStr,	alpacaDevice->deviceNameStr);
	}

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

}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerDome::~ControllerDome(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	if (cLogSlitData)
	{
		CloseSlitTrackerDataFile();
	}
}


//**************************************************************************************
void	ControllerDome::SetAlpacaShutterInfo(TYPE_REMOTE_DEV *alpacaDevice)
{
	CONSOLE_DEBUG(__FUNCTION__);

	if (alpacaDevice != NULL)
	{
#ifdef _ENABLE_EXTERNAL_SHUTTER_
		cShutterInfoValid		=	true;

		cShutterDeviceAddress	=	alpacaDevice->deviceAddress;
		cShutterPort			=	alpacaDevice->port;
		cShutterAlpacaDevNum	=	alpacaDevice->alpacaDeviceNum;

		SetWidgetText(kTab_Dome,		kDomeBox_Readall,		"RS");
		SetWidgetText(kTab_SlitTracker,	kSlitTracker_Readall,	"RS");
		SetWidgetText(kTab_SlitGraph,	kSlitGraph_Readall,		"RS");
		SetWidgetText(kTab_About,		kAboutBox_Readall,		"RS");
#endif // _ENABLE_EXTERNAL_SHUTTER_
	}
}

//**************************************************************************************
void	ControllerDome::SetAlpacaSlitTrackerInfo(TYPE_REMOTE_DEV *alpacaDevice)
{
char	ipString[64];
char	lineBuff[128];

	CONSOLE_DEBUG(__FUNCTION__);

	if (alpacaDevice != NULL)
	{
		cSlitTrackerInfoValid		=	true;
		cSlitTrackerDeviceAddress	=	alpacaDevice->deviceAddress;
		cSlitTrackerPort			=	alpacaDevice->port;
		cSlitTrackerAlpacaDevNum	=	alpacaDevice->alpacaDeviceNum;

		PrintIPaddressToString(cSlitTrackerDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cSlitTrackerPort, cSlitTrackerAlpacaDevNum);

		SetWidgetText(kTab_SlitTracker,	kSlitTracker_RemoteAddress,	lineBuff);
	}
}

//**************************************************************************************
void	ControllerDome::SetupWindowControls(void)
{
char	lineBuff[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_Dome,			"Dome");
	SetTabText(kTab_SlitTracker,	"Slit Tracker");
	SetTabText(kTab_SlitGraph,		"Slit Graph");
	SetTabText(kTab_About,			"About");

	//=============================================================
	cDomeTabObjPtr	=	new WindowTabDome(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDomeTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Dome,	cDomeTabObjPtr);
		cDomeTabObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	cSlitTrackerTabObjPtr		=	new WindowTabSlitTracker(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSlitTrackerTabObjPtr != NULL)
	{
		SetTabWindow(kTab_SlitTracker,	cSlitTrackerTabObjPtr);
		cSlitTrackerTabObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	cSlitGraphTabObjPtr		=	new WindowTabSlitGraph(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cSlitGraphTabObjPtr != NULL)
	{
		SetTabWindow(kTab_SlitGraph,	cSlitGraphTabObjPtr);
		cSlitGraphTabObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}

	//*	display the IPaddres/port
	if (cValidIPaddr)
	{
	char	ipString[32];

		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);

		SetWindowIPaddrInfo(lineBuff, true);
	}
}

//**************************************************************************************
void	ControllerDome::RunBackgroundTasks(void)
{
uint32_t	currentMillis;
uint32_t	deltaSeconds;
bool		validData;
bool		needToUpdate;

	if (cReadStartup)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		AlpacaGetStartupData();
		cReadStartup	=	false;
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
		//*	is the IP address valid
		if (cValidIPaddr)
		{
			validData	=	AlpacaGetStatus();
			if (validData == false)
			{
				CONSOLE_DEBUG("Failed to get data")
			}
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
bool	ControllerDome::AlpacaGetStatus(void)
{
bool	validData;
bool	previousOnLineState;

//	CONSOLE_DEBUG(__FUNCTION__);

	previousOnLineState	=	cOnLine;
	if (cHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll("dome", cAlpacaDevNum);
	}
	else
	{
		validData	=	AlpacaGetStatus_OneAAT();
	}
#ifdef _ENABLE_EXTERNAL_SHUTTER_
	//=================================================
	if (cShutterInfoValid)
	{
		AlpacaGetShutterReadAll();
	}
#endif // _ENABLE_EXTERNAL_SHUTTER_
	//=================================================
	if (cSlitTrackerInfoValid)
	{
		AlpacaGetSlitTrackerReadAll();
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
		if (cSlewing)
		{
			//*	if we slewing, we want to update more often
			cUpdateDelta	=	2;
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Slewing");
		}
		else if (cAtHome)
		{
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Home");
		}
		else if (cAtPark)
		{
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "Park");
		}
		else
		{
			SetWidgetText(kTab_Dome, kDomeBox_CurPosition, "---");
		}

		//*	if we shutter is opening or closing, we want to update more often
		if ((cShutterStatus == kShutterStatus_Opening) || (cShutterStatus == kShutterStatus_Closing))
		{
			cUpdateDelta	=	2;
		}
	}
	else
	{
		cOnLine	=	false;
	}
	if (cOnLine != previousOnLineState)
	{
		SetWindowIPaddrInfo(NULL, cOnLine);
	}

	cLastUpdate_milliSecs	=	millis();
	cFirstDataRead			=	false;
	return(validData);
}

#ifdef  _ENABLE_EXTERNAL_SHUTTER_
//*****************************************************************************
void	ControllerDome::AlpacaGetShutterReadAll(void)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
int				newShutterStatus;

//	CONSOLE_DEBUG(__FUNCTION__);

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", "shutter", cShutterAlpacaDevNum, "readall");

	validData	=	GetJsonResponse(	&cShutterDeviceAddress,
										cShutterPort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
		cLastAlpacaErrNum	=	0;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
//			CONSOLE_DEBUG_W_STR(	jsonParser.dataList[jjj].keyword,
//									jsonParser.dataList[jjj].valueString);

			if (strcasecmp(jsonParser.dataList[jjj].keyword, "shutterstatus") == 0)
			{
				newShutterStatus	=	atoi(jsonParser.dataList[jjj].valueString);
				UpdateShutterStatus(newShutterStatus);
			}
			else if (strcasecmp(jsonParser.dataList[jjj].keyword, "altitude") == 0)
			{
			double	myNewAltitude;

				myNewAltitude	=	atof(jsonParser.dataList[jjj].valueString);
				UpdateShutterAltitude(myNewAltitude);

			}
		}
		if (cShutterCommFailed)
		{
			SetWidgetText(kTab_Dome, kDomeBox_ErrorMsg, "---");
			SetWidgetTextColor(kTab_Dome,	kDomeBox_ShutterStatus,	CV_RGB(0,	255,	0));
		}
		cShutterCommFailed	=	false;
		cShutterCommFailCnt	=	0;
	}
	else
	{
		cShutterCommFailed	=	true;
		cShutterCommFailCnt++;
		sprintf(alpacaString, "Failed to read data from shutter - %d", cShutterCommFailCnt);
		SetWidgetText(kTab_Dome, kDomeBox_ErrorMsg, alpacaString);

		cShutterStatus	=	kShutterStatus_Unknown;
		SetWidgetText(kTab_Dome, kDomeBox_ShutterStatus, "Unknown");
		SetWidgetTextColor(kTab_Dome,	kDomeBox_ShutterStatus,	CV_RGB(255,	0,	0));
	}
}
#endif // _ENABLE_EXTERNAL_SHUTTER_

//*****************************************************************************
void	ControllerDome::AlpacaGetSlitTrackerReadAll(void)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
int				clockValue;
char			clockString[48];
double			inchValue;
struct tm		*linuxTime;
char			slitLogFileName[48];
double			gravityValue;
char			gravityVectorChar;

//	CONSOLE_DEBUG(__FUNCTION__);

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", "slittracker", cSlitTrackerAlpacaDevNum, "readall");

	validData	=	GetJsonResponse(	&cSlitTrackerDeviceAddress,
										cSlitTrackerPort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
		cLastAlpacaErrNum	=	0;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
//			CONSOLE_DEBUG_W_STR(	jsonParser.dataList[jjj].keyword,
//									jsonParser.dataList[jjj].valueString);
			//	"sensor-0":28.310000,

			if (strncasecmp(jsonParser.dataList[jjj].keyword, "sensor-", 6) == 0)
			{
				strcpy(clockString, &jsonParser.dataList[jjj].keyword[7]);
				clockValue	=	atoi(clockString);
				inchValue	=	atof(jsonParser.dataList[jjj].valueString);
				if ((clockValue >= 0) && (clockValue < kSensorValueCnt))
				{
					gSlitDistance[clockValue].distanceInches	=	inchValue;
					gSlitDistance[clockValue].validData			=	true;
					gSlitDistance[clockValue].updated			=	true;
					gSlitDistance[clockValue].readCount++;

					gUpdateSLitWindow	=	true;
				}
			}
			else if (strncasecmp(jsonParser.dataList[jjj].keyword, "gravity_", 8) == 0)
			{
			//	CONSOLE_DEBUG_W_STR("Gravity vector:", jsonParser.dataList[jjj].keyword);
				gravityVectorChar	=	jsonParser.dataList[jjj].keyword[8];
				gravityValue		=	atof(jsonParser.dataList[jjj].valueString);

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
						CONSOLE_DEBUG_W_STR("Gravity vector error:", jsonParser.dataList[jjj].keyword);
						break;
				}
			}
		}
		//*	update the time of the last data
		gettimeofday(&cSlitTrackerLastUpdateTime, NULL);

		UpdateSlitLog();

		//===============================================================
		//*	check for data logging to disk
		if (cLogSlitData)
		{
			if (cSlitDataLogFilePtr == NULL)
			{
				linuxTime		=	localtime(&cSlitTrackerLastUpdateTime.tv_sec);
				sprintf(slitLogFileName, "slitlog-%02d-%02d-%02d.csv",
											(1900 + linuxTime->tm_year),
											(1 +	linuxTime->tm_mon),
													linuxTime->tm_mday);

				cSlitDataLogFilePtr	=	fopen(slitLogFileName, "a");
			}
			if (cSlitDataLogFilePtr != NULL)
			{
				for (jjj=0; jjj<kSensorValueCnt; jjj++)
				{
					fprintf(cSlitDataLogFilePtr, "%1.2f,", gSlitDistance[jjj].distanceInches);
				}
				fprintf(cSlitDataLogFilePtr, "\n");
				fflush(cSlitDataLogFilePtr);
			}
		}

		if (cSlitTrackerCommFailed)
		{
			//*	set the indicators back to OK
			SetWidgetText(		kTab_Dome, 			kDomeBox_ErrorMsg, "---");
			SetWidgetBGColor(	kTab_SlitTracker,	kSlitTracker_RemoteAddress, CV_RGB(0,0,0));
			SetWidgetTextColor(	kTab_SlitTracker,	kSlitTracker_RemoteAddress, CV_RGB(255,0,0));
		}
		cSlitTrackerCommFailed	=	false;
	}
	else
	{
		SetWidgetText(kTab_Dome,				kDomeBox_ErrorMsg, "Failed to read data from Slit Tracker");
		SetWidgetBGColor(kTab_SlitTracker,		kSlitTracker_RemoteAddress, CV_RGB(255,0,0));
		SetWidgetTextColor(kTab_SlitTracker,	kSlitTracker_RemoteAddress, CV_RGB(0,0,0));
		cSlitTrackerCommFailed	=	true;
	}
}

//*****************************************************************************
void	ControllerDome::UpdateSlitLog(void)
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
void	ControllerDome::CloseSlitTrackerDataFile(void)
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

#ifdef _ENABLE_EXTERNAL_SHUTTER_

int	gClientID				=	1368;
int	gClientTransactionID	=	0;
//*****************************************************************************
//*	this is identical to AlpacaSendPutCmd but it sends it to the shutter device address
//*****************************************************************************
bool	ControllerDome::ShutterSendPutCmd(	const char	*alpacaDevice,
											const char	*alpacaCmd,
											const char	*dataString)
{
SJP_Parser_t	jsonParser;
bool			sucessFlag;
char			alpacaString[128];
char			myDataString[512];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, alpacaCmd);

	SJP_Init(&jsonParser);

	sprintf(alpacaString, "/api/v1/%s/%d/%s", alpacaDevice, cShutterAlpacaDevNum, alpacaCmd);
	if (strlen(dataString) > 0)
	{
		sprintf(myDataString, "%s&ClientID=%d&ClientTransactionID=%d",
												dataString,
												gClientID,
												gClientTransactionID);
	}
	else
	{
		sprintf(myDataString, "ClientID=%d&ClientTransactionID=%d",
												gClientID,
												gClientTransactionID);
	}
	CONSOLE_DEBUG_W_STR("Sending", myDataString);
	sucessFlag	=	SendPutCommand(	&cShutterDeviceAddress,
									cShutterPort,
									alpacaString,
									"",
									&jsonParser);
	strcpy(cLastAlpacaCmdString, alpacaString);
	gClientTransactionID++;

	cForceAlpacaUpdate	=	true;
	return(sucessFlag);
}
#endif // _ENABLE_EXTERNAL_SHUTTER_

//*****************************************************************************
void	ControllerDome::SendShutterCommand(const char *shutterCmd)
{
bool		sucessFlag;

	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _ENABLE_EXTERNAL_SHUTTER_
	//*	this is for a separate alpaca device called "shutter"
	if (cShutterInfoValid)
	{
		sucessFlag	=	ShutterSendPutCmd("shutter", shutterCmd, "");
		if (sucessFlag == false)
		{
			CONSOLE_DEBUG_W_STR("ShutterSendPutCmd failed", shutterCmd);
		}

		//*	any time we send a command to the shutter, increase the update rate
		cUpdateDelta	=	2;
	}
	else
	{
		CONSOLE_DEBUG("No shutter info");
		SetWidgetText(kTab_Dome, kDomeBox_ErrorMsg, "Shutter controller not found");
	}
#else
	//*	normal, send command to the dome controller.


#endif
}


//*****************************************************************************
void	ControllerDome::ToggleSwitchState(const int switchNum)
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



#endif // _ENABLE_CTRL_SWITCHES_
