//**************************************************************************
//*	Name:			multicam.cpp
//*
//*	Author:			Mark Sproul (C) 2019
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Nov  7,	2019	<MLS> Created multicam.cpp
//*	Nov  8,	2019	<MLS> Multicam working with 2 cameras, a ZWO and ATIK
//*	Nov 13,	2019	<MLS> Added setexposuretime, and setlivemode
//*	Nov 13,	2019	<MLS> Added SetExposureTime() & ExtractDurationList()
//==============================
//*	Mar 29,	2120	<TODO> Change multicam commands to match updated camera commands
//*****************************************************************************

#ifdef _ENABLE_MULTICAM_

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"RequestData.h"
#include	"JsonResponse.h"
#include	"eventlogging.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"cameradriver.h"
#include	"multicam.h"




#define	kRotateDome_CW		0
#define	kRotateDome_CCW		1



#define	kStopRightNow	true
#define	kStopNormal		false



//*****************************************************************************
void	CreateMultiCamObject(void)
{
int	cameraCnt;

	cameraCnt	=	CountDevicesByType(kDeviceType_Camera);
	if (cameraCnt > 0)
	{
		new MultiCam(0);
	}
}


//*****************************************************************************
//*	MultiCam commands
enum
{
	kCmd_MultiCam_startexposure	=	0,	//*	Starts an exposure
	kCmd_MultiCam_stopexposure,			//*	Stops the current exposure

	kCmd_MultiCam_setexposuretime,
	kCmd_MultiCam_setlivemode,


	kCmd_MultiCam_last
};



//*****************************************************************************
static TYPE_CmdEntry	gMultiCamCmdTable[]	=
{

	{	"startexposure",			kCmd_MultiCam_startexposure,		kCmdType_GET	},
	{	"stopexposure",				kCmd_MultiCam_stopexposure,			kCmdType_GET	},
	{	"setexposuretime",			kCmd_MultiCam_setexposuretime,		kCmdType_BOTH	},
	{	"setlivemode",				kCmd_MultiCam_setlivemode,			kCmdType_BOTH	},



	{	"",						-1,	0x00	}

};

//*****************************************************************************
MultiCam::MultiCam(const int argDevNum)
	:AlpacaDriver(kDeviceType_Multicam)
{
	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name, "MultiCam");

}

//**************************************************************************************
// Destructor
//**************************************************************************************
MultiCam::~MultiCam( void )
{
}



#pragma mark -





//*****************************************************************************
//*	valid commands
//*		http://127.0.0.1:6800/api/v1/dome/altitude
//*****************************************************************************
TYPE_ASCOM_STATUS	MultiCam::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
char				alpacaErrMsg[256];
int					cmdEnumValue;
int					cmdType;
int					mySocket;

	CONSOLE_DEBUG(__FUNCTION__);


	//*	make local copies of the data structure to make the code easier to read
	mySocket	=	reqData->socket;
//	myDeviceNum	=	reqData->deviceNumber;

	strcpy(alpacaErrMsg, "");
	alpacaErrCode	=	kASCOM_Err_Success;

	//*	set up the json response
	JsonResponse_CreateHeader(reqData->jsonTextBuffer);

	//*	this is not part of the protocol, I am using it for testing
	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Device",
								cCommonProp.Name,
								INCLUDE_COMMA);

	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Command",
								reqData->deviceCommand,
								INCLUDE_COMMA);

//	JsonResponse_Add_Int32(		mySocket,
//								reqData->jsonTextBuffer,
//								kMaxJsonBuffLen,
//								"PreviousState",
//								domeState,
//								INCLUDE_COMMA);

	//*	look up the command
	cmdEnumValue	=	FindCmdFromTable(reqData->deviceCommand, gMultiCamCmdTable, &cmdType);
	switch(cmdEnumValue)
	{
		//----------------------------------------------------------------------------------------
		//*	Common commands that we want to over ride
		//----------------------------------------------------------------------------------------
		case kCmd_Common_description:		//*	Device description
			JsonResponse_Add_String(	mySocket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										gValueString,
										"Multiple camera driver",
										INCLUDE_COMMA);
			break;


		case kCmd_Common_supportedactions:	//*	Returns the list of action names supported by this driver.
			alpacaErrCode	=	Get_SupportedActions(reqData, gMultiCamCmdTable);
			break;

		//----------------------------------------------------------------------------------------
		//*	Camera specific commands
		//----------------------------------------------------------------------------------------
		case kCmd_MultiCam_startexposure:		//*	Starts an exposure
			CONSOLE_DEBUG("kCmd_MultiCam_startexposure");
			alpacaErrCode	=	StartExposure(reqData, alpacaErrMsg);
			break;

		case kCmd_MultiCam_stopexposure:		//*	Stops the current exposure
			CONSOLE_DEBUG("kCmd_MultiCam_stopexposure");
			break;


		case kCmd_MultiCam_setexposuretime:
			alpacaErrCode	=	SetExposureTime(reqData, alpacaErrMsg);
			break;

		case kCmd_MultiCam_setlivemode:
			break;


		//----------------------------------------------------------------------------------------
		//*	let anything undefined go to the common command processor
		//----------------------------------------------------------------------------------------
		default:
			alpacaErrCode	=	ProcessCommand_Common(reqData, cmdEnumValue, alpacaErrMsg);
			break;
	}
	//*	send the response information
	JsonResponse_Add_Int32(		mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ClientTransactionID",
								gClientTransactionID,
								INCLUDE_COMMA);

	JsonResponse_Add_Int32(		mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ServerTransactionID",
								gServerTransactionID,
								INCLUDE_COMMA);

	JsonResponse_Add_Int32(		mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ErrorNumber",
								alpacaErrCode,
								INCLUDE_COMMA);

//	JsonResponse_Add_Int32(		mySocket,
//								reqData->jsonTextBuffer,
//								kMaxJsonBuffLen,
//								"NewState",
//								domeState,
//								INCLUDE_COMMA);

	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ErrorMessage",
								alpacaErrMsg,
								NO_COMMA);

	JsonResponse_Add_Finish(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								kInclude_HTTP_Header);

	//*	this is for the logging function
	strcpy(reqData->alpacaErrMsg, alpacaErrMsg);
	return(alpacaErrCode);
}

//*****************************************************************************
int32_t	MultiCam::RunStateMachine(void)
{
int32_t		minDealy_microSecs;
//uint32_t	currentMilliSecs;
//char		stateString[48];

//	CONSOLE_DEBUG(__FUNCTION__);

//	currentMilliSecs			=	millis();

	minDealy_microSecs			=	(15 * 1000 * 1000);
	switch(cMultiCamState)
	{
		default:
			break;
	}
//
	return(minDealy_microSecs);
}



//*****************************************************************************
int	CountCharsInString(const char *theString, char theChar)
{
int		charCnt;
int		ii;
int		sLen;

	sLen	=	strlen(theString);
	charCnt	=	0;
	for (ii=0; ii<sLen; ii++)
	{
		if (theString[ii] == theChar)
		{
			charCnt++;
		}
	}

	return(charCnt);
}


//*****************************************************************************
//	This routine populates an array of exposure times from a string.
//*	All of the values first get set to the first value in the string,
//*	then, if there are commas in the list, the subsequent values are updated
//*****************************************************************************
static void	ExtractDurationList(const char *durationString, double *duartionValues, const int durationCnt)
{
double		exposureDuration_secs;
int			ii;
int			cc;
int			commaCnt;
int			sLen;

	//*	get the first value out of the string
	exposureDuration_secs	=	atof(durationString);

	//*	this can be a coma separated list
	//*	we have an array to fill that will be used to set the exposure times
	//*	first we will populate the entire array with the first value
	for (ii=0; ii<durationCnt; ii++)
	{
		duartionValues[ii]	=	exposureDuration_secs;
	}

	commaCnt	=	CountCharsInString(durationString, ',');
	if (commaCnt > 0)
	{
//		CONSOLE_DEBUG_W_NUM("commaCnt\t=",	commaCnt);
		//*	now look for the multiple values in the list.
		sLen	=	strlen(durationString);
		ii		=	0;
		cc		=	1;	//*	index into the value array
		while((ii < sLen) && (cc < kMaxDevices))
		{
			if (durationString[ii] == ',')
			{
				duartionValues[cc]	=	atof(&durationString[ii+1]);
				cc++;
			}
			ii++;
		}
		//*	debugging
//		for (ii=0; ii<kMaxDevices; ii++)
//		{
//			CONSOLE_DEBUG_W_DBL("duartionValues[ii]\t=",	duartionValues[ii]);
//		}
	}
}

//*****************************************************************************
TYPE_ASCOM_STATUS	MultiCam::StartExposure(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
double				exposureDuration_usecs;
double				expDurationValues_secs[kMaxDevices];
int					ii;
int					cc;
CameraDriver		*cameraObj;
struct timeval		currentTime;
long				curr_millisec;
long				prev_millisec;
bool				durationFound;
char				durationString[128];
bool				objectNameFound;
char				myObjectName[kObjectNameMaxLen];
bool				telescopeNameFound;
char				myTelescopeName[kTelescopeNameMaxLen];
bool				filenamePrefixFound;
char				myFileNamePrefix[kFileNamePrefixMaxLen];
bool				filenameSuffixFound;
char				myFileNameSuffix[kFileNamePrefixMaxLen];

//bool			imageTypeFound;
//char			myImageType[32];

	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
//		CONSOLE_DEBUG_W_STR("contentData\t=",	reqData->contentData);


		objectNameFound		=	GetKeyWordArgument(	reqData->contentData,
													"Object",
													myObjectName,
													kObjectNameMaxLen);

		telescopeNameFound	=	GetKeyWordArgument(	reqData->contentData,
													"Telescope",
													myTelescopeName,
													kTelescopeNameMaxLen);

		filenamePrefixFound	=	GetKeyWordArgument(	reqData->contentData,
													"Prefix",
													myFileNamePrefix,
													kFileNamePrefixMaxLen);

		filenameSuffixFound	=	GetKeyWordArgument(	reqData->contentData,
													"Suffix",
													myFileNameSuffix,
													kFileNamePrefixMaxLen);

//		imageTypeFound		=	GetKeyWordArgument(	reqData->contentData,
//													"Imagetype",
//													myImageType,
//													30);

		//*	Duration=1000.0&Light=true HTTP/1.1
		durationFound		=	GetKeyWordArgument(	reqData->contentData,
													"Duration",
													durationString,
													100);
		if (durationFound)
		{
			ExtractDurationList(durationString, expDurationValues_secs, kMaxDevices);
		}
		else
		{
			//*	have to have a fallback default
			for (ii=0; ii<kMaxDevices; ii++)
			{
				expDurationValues_secs[ii]	=	0.01;
			}
		}
		//****************************************************
		//*	first we are going to step through all the cameras and
		//*		Turn serial number naming on
		//*		Set the object name if it was specified
		//*		Set the telescope name if it was specified
		//****************************************************
		for (ii=0; ii<gDeviceCnt; ii++)
		{
			if (gAlpacaDeviceList[ii] != NULL)
			{
				if (gAlpacaDeviceList[ii]->cDeviceType == kDeviceType_Camera)
				{
					cameraObj		=	(CameraDriver *)gAlpacaDeviceList[ii];
					//*	in order to use multiple camers,
					//*	we need the serial number included in the file name
					cameraObj->SetSerialNumInFileName(true);
					if (objectNameFound)
					{
						cameraObj->SetObjectName(myObjectName);
					}
					if (telescopeNameFound)
					{
						cameraObj->SetTelescopeName(myTelescopeName);
					}
					if (filenamePrefixFound)
					{
						cameraObj->SetFileNamePrefix(myFileNamePrefix);
					}
					if (filenameSuffixFound)
					{
						cameraObj->SetFileNameSuffix(myFileNameSuffix);
					}
				}
			}
		}

		//****************************************************
		//*	sit here and waste time until we change millisecs
		//*	this way, the 2 images will have the same time stamp
		gettimeofday(&currentTime, NULL);
		prev_millisec	=	currentTime.tv_usec / 1000;
		curr_millisec	=	currentTime.tv_usec / 1000;
		while (curr_millisec == prev_millisec)
		{
			gettimeofday(&currentTime, NULL);
			curr_millisec	=	currentTime.tv_usec / 1000;
		}

		//****************************************************
		//*	now start the exposures
		//****************************************************
		cc	=	0;		//*	set the camera counter to zero
		for (ii=0; ii<gDeviceCnt; ii++)
		{
			if (gAlpacaDeviceList[ii] != NULL)
			{
				if (gAlpacaDeviceList[ii]->cDeviceType == kDeviceType_Camera)
				{
				//	CONSOLE_DEBUG("We have a camera");
				//	exposureDuration_usecs	=	exposureDuration_secs * 1000 * 1000;
					exposureDuration_usecs	=	expDurationValues_secs[cc] * 1000 * 1000;
					cameraObj		=	(CameraDriver *)gAlpacaDeviceList[ii];
					alpacaErrCode	=	cameraObj->Start_CameraExposure(exposureDuration_usecs);
					cameraObj->SaveNextImage();

					if (alpacaErrCode != 0)
					{
						CONSOLE_DEBUG_W_NUM("Start_CameraExposure->alpacaErrCode\t=",	alpacaErrCode);
					}

					cc++;
				}
			}
		}
//		CONSOLE_DEBUG_W_INT32("currentTime\t=",		millis());
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	MultiCam::SetExposureTime(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
double				exposureDuration_usecs;
double				expDurationValues_secs[kMaxDevices];
int					ii;
int					cc;
CameraDriver		*cameraObj;
bool				durationFound;
char				durationString[128];

//	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		durationFound		=	GetKeyWordArgument(	reqData->contentData,
													"Duration",
													durationString,
													100);
		if (durationFound)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
			CONSOLE_DEBUG_W_STR("durationString\t=",	durationString);

			ExtractDurationList(durationString, expDurationValues_secs, kMaxDevices);

			//****************************************************
			//*	Step through all the cameras and set the exposure
			//****************************************************
			cc	=	0;		//*	set the camera counter to zero
			for (ii=0; ii<gDeviceCnt; ii++)
			{
				if (gAlpacaDeviceList[ii] != NULL)
				{
					if (gAlpacaDeviceList[ii]->cDeviceType == kDeviceType_Camera)
					{
						exposureDuration_usecs	=	expDurationValues_secs[cc] * 1000 * 1000;

						cameraObj		=	(CameraDriver *)gAlpacaDeviceList[ii];
						cameraObj->Start_CameraExposure(exposureDuration_usecs);
						cc++;
					}
				}
			}
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
void	MultiCam::OutputHTML(TYPE_GetPutRequestData *reqData)
{
int				mySocketFD;
char			lineBuffer[512];
int				ii;
CameraDriver	*cameraObj;

	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{

		mySocketFD	=	reqData->socket;

		SocketWriteData(mySocketFD,	"<CENTER>\r\n");

		SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

		//*-----------------------------------------------------------
		SocketWriteData(mySocketFD,	"<TR>\r\n");
		SocketWriteData(mySocketFD,	"\t<TD COLSPAN=3><CENTER>Multicam</TD>");
		SocketWriteData(mySocketFD,	"</TR>\r\n");

		SocketWriteData(mySocketFD,	"<TR>\r\n");
		SocketWriteData(mySocketFD,	"\t<TD COLSPAN=3><CENTER>Available cameras</TD>");
		SocketWriteData(mySocketFD,	"</TR>\r\n");

		SocketWriteData(mySocketFD,	"<TR><TH>Manuf</TH><TH>Model</TH><TH>Serial #</TH></TR>\r\n");

		for (ii=0; ii<gDeviceCnt; ii++)
		{
			if (gAlpacaDeviceList[ii] != NULL)
			{
				if (gAlpacaDeviceList[ii]->cDeviceType == kDeviceType_Camera)
				{
					cameraObj	=	(CameraDriver *)gAlpacaDeviceList[ii];

					sprintf(lineBuffer, "\t<TR><TD>%s</TD><TD>%s</TD><TD>%s</TD></TR>",
												cameraObj->cDeviceManufacturer,
												cameraObj->cCommonProp.Name,
												cameraObj->cDeviceSerialNum);
					SocketWriteData(mySocketFD,	lineBuffer);
				}
			}
		}
		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"<P>\r\n");


		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		//*	now generate links to all of the commands
		GenerateHTMLcmdLinkTable(mySocketFD, "multicam", 0, gMultiCamCmdTable);
	}
}
#endif	//	_ENABLE_MULTICAM_
