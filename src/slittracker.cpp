//**************************************************************************
//*	Name:			slittracker.cpp
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	May  2,	2020	<MLS> Created slittracker.cpp
//*	May 30,	2020	<MLS> Added gravity vector parsing
//*	May 30,	2020	<MLS> Added gravity to readall output
//*	Mar  8,	2023	<MLS> Added ReadSlittrackerConfig()
//*	Mar  8,	2023	<MLS> Added support for Web setup
//*	Mar  8,	2023	<MLS> Added WriteConfigFile()
//*	Mar  8,	2023	<MLS> Added domeaddress property
//*	Mar  9,	2023	<MLS> Added trackingenabled property
//*	Mar  9,	2023	<MLS> Added web docs support
//*	Jul 10,	2023	<MLS> Switched SlitTracker to use command table
//*****************************************************************************

#ifdef _ENABLE_SLIT_TRACKER_

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<termios.h>
#include	<fcntl.h>
#include	<errno.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"serialport.h"
#include	"JsonResponse.h"
#include	"eventlogging.h"
#include	"readconfigfile.h"

#include	"slittracker.h"

#include	"slittracker_AlpacaCmds.h"
#include	"slittracker_AlpacaCmds.cpp"

static const char	gSlitTrackerConfigFile[]	=	"slittracker-config.txt";


//**************************************************************************************
typedef struct
{
	char	usbPath[24];
} TYPE_USB_PATH;

//**************************************************************************************
void	CreateSlitTrackerObjects(void)
{
int				accessRC;
int				iii;
bool			keepGoing;
TYPE_USB_PATH	usbPathList[]	=
{
	"/dev/ttyACM0",
	"/dev/ttyACM1",
	""
};
	CONSOLE_DEBUG(__FUNCTION__);

	keepGoing	=	true;
	iii			=	0;
	while (keepGoing && (usbPathList[iii].usbPath[0] != 0))
	{
		accessRC	=	access(usbPathList[iii].usbPath, F_OK);
		if (accessRC == 0)
		{
			new SlitTrackerDriver(0, usbPathList[iii].usbPath);
			keepGoing	=	false;
		}
		else
		{
			CONSOLE_DEBUG(__FUNCTION__);
		}
		iii++;
	}
}

//**************************************************************************************
SlitTrackerDriver::SlitTrackerDriver(const int argDevNum, const char *devicePath)
	:AlpacaDriver(kDeviceType_SlitTracker)
{
int	iii;
//	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name,		"SlitTracker");
	strcpy(cCommonProp.Description,	"Slit tracking for dome slaving");
	if (devicePath != NULL)
	{
		strcpy(cUSBpath, devicePath);
	}
	else
	{
		strcpy(cUSBpath, "/dev/ttyACM0");
	}
	LogEvent(	"slittracker",
				__FUNCTION__,
				NULL,
				kASCOM_Err_Success,
				cUSBpath);

	cDriverCmdTablePtr		=	gSlitTrackerCmdTable;
	cSlitTrackerfileDesc	=	-1;				//*	port file descriptor
	cSlitTrackerByteCnt		=	0;
	//*	initialize the slit distance detector
	for (iii=0; iii<kSlitSensorCnt; iii++)
	{
		cSlitDistance[iii].validData		=	false;
		cSlitDistance[iii].distanceInches	=	-1.0;
		cSlitDistance[iii].readCount		=	0;
	}
	cGravity_X	=	0.0;
	cGravity_Y	=	0.0;
	cGravity_Z	=	0.0;
	cGravity_T	=	0.0;

	//========================================
	//*	dome configuration
	cDomeAlpacaPort		=	0;
	cDomeAlpacaDevNum	=	0;
	strcpy(cDomeIPaddressString,		"");

	memset(&cDomeProp, 0, sizeof(TYPE_DomeProperties));
	memset(&cSlitProp, 0, sizeof(TYPE_SlittrackerProperties));

	//========================================
	//*	Setup support
	cDriverSupportsSetup		=	true;

	ReadSlittrackerConfig();

	OpenSlitTrackerPort();
}


//**************************************************************************************
// Destructor
//**************************************************************************************
SlitTrackerDriver::~SlitTrackerDriver(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	SlitTrackerDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
char				alpacaErrMsg[256];
int					cmdEnumValue;
int					cmdType;
int					mySocket;

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("reqData->deviceCommand\t=",	reqData->deviceCommand);
#endif // _DEBUG_CONFORM_

	strcpy(alpacaErrMsg, "");

	//*	make local copies of the data structure to make the code easier to read
	mySocket	=	reqData->socket;

//	DumpRequestStructure(__FUNCTION__, reqData);

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

	//*	look up the command
	cmdEnumValue	=	FindCmdFromTable(reqData->deviceCommand, gSlitTrackerCmdTable, &cmdType);
//	CONSOLE_DEBUG_W_NUM("cmdEnumValue\t=", cmdEnumValue);
	switch(cmdEnumValue)
	{
		//----------------------------------------------------------------------------------------
		//*	Common commands that we want to over ride
		//----------------------------------------------------------------------------------------
		case kCmd_Common_supportedactions:	//*	Returns the list of action names supported by this driver.
			alpacaErrCode	=	Get_SupportedActions(reqData, gSlitTrackerCmdTable);
			break;

		//----------------------------------------------------------------------------------------
		//*	SlitTracker specific commands
		//----------------------------------------------------------------------------------------
		case kCmd_SlitTracker_SetRate:
//+			alpacaErrCode	=	Put_Setrate(reqData, alpacaErrMsg);
			break;

		case kCmd_SlitTracker_DomeAddress:
			alpacaErrCode	=	Get_DomeAddress(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_SlitTracker_TrackingEnabled:
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_TrackingEnabled(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_TrackingEnabled(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_SlitTracker_readall:
			alpacaErrCode	=	Get_Readall(reqData, alpacaErrMsg);
			break;

		//----------------------------------------------------------------------------------------
		//*	let anything undefined go to the common command processor
		//----------------------------------------------------------------------------------------
		default:
			alpacaErrCode	=	ProcessCommand_Common(reqData, cmdEnumValue, alpacaErrMsg);
			break;
	}
	RecordCmdStats(cmdEnumValue, reqData->get_putIndicator, alpacaErrCode);

	//*	send the response information
	JsonResponse_Add_Int32(	mySocket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"ClientTransactionID",
							gClientTransactionID,
							INCLUDE_COMMA);

	JsonResponse_Add_Int32(	mySocket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"ServerTransactionID",
							gServerTransactionID,
							INCLUDE_COMMA);

	JsonResponse_Add_Int32(	mySocket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"ErrorNumber",
							alpacaErrCode,
							INCLUDE_COMMA);


	JsonResponse_Add_String(mySocket,
							reqData->jsonTextBuffer,
							kMaxJsonBuffLen,
							"ErrorMessage",
							alpacaErrMsg,
							NO_COMMA);

	JsonResponse_Add_Finish(mySocket,
							reqData->jsonTextBuffer,
							(cHttpHeaderSent == false));

#ifdef _DEBUG_CONFORM_
	CONSOLE_DEBUG_W_STR("Output JSON\t=", reqData->jsonTextBuffer);
#endif // _DEBUG_CONFORM_

	return(alpacaErrCode);
}

//**************************************************************************************
int32_t	SlitTrackerDriver::RunStateMachine(void)
{
	GetSlitTrackerData();
	return(5000);
}


#pragma mark -

//*****************************************************************************
void	SlitTrackerDriver::OutputHTML(TYPE_GetPutRequestData *reqData)
{
int			mySocketFD;
int			iii;
char		lineBuffer[128];

//	CONSOLE_DEBUG(__FUNCTION__);

	if (reqData != NULL)
	{
		mySocketFD		=	reqData->socket;
		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<H2>Slit Tracker</H2>\r\n");

		SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
		SocketWriteData(mySocketFD,	"\t<TR>\r\n");
		SocketWriteData(mySocketFD, "<TH>Sensor #</TH><TH>inches</TH><TH>Read cnt</TH>\r\n");
		SocketWriteData(mySocketFD,	"\t</TR>\r\n");

		for (iii=0; iii<kSlitSensorCnt; iii++)
		{
			SocketWriteData(mySocketFD,	"\t<TR>\r\n");

			sprintf(lineBuffer, "\t\t<TD><CENTER>%d</TD>\r\n", iii);
			SocketWriteData(mySocketFD,	lineBuffer);
			if (cSlitDistance[iii].validData)
			{
				sprintf(lineBuffer, "\t\t<TD><CENTER>%1.2f</TD>\r\n", cSlitDistance[iii].distanceInches);
			}
			else
			{
				strcpy(lineBuffer, "\t\t<TD>not valid</TD>\r\n");
			}
			SocketWriteData(mySocketFD,	lineBuffer);

			sprintf(lineBuffer, "\t\t<TD><CENTER>%ld</TD>\r\n", cSlitDistance[iii].readCount);
			SocketWriteData(mySocketFD,	lineBuffer);

			SocketWriteData(mySocketFD,	"\t</TR>\r\n");
		}

		SocketWriteData(mySocketFD,	"\t<TR>\r\n");
		sprintf(lineBuffer, "\t\t<TD>Gravity X</TD><TD><CENTER>%1.2f</TD>\r\n", cGravity_X);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"\t</TR>\r\n");

		SocketWriteData(mySocketFD,	"\t<TR>\r\n");
		sprintf(lineBuffer, "\t\t<TD>Gravity Y</TD><TD><CENTER>%1.2f</TD>\r\n", cGravity_Y);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"\t</TR>\r\n");

		SocketWriteData(mySocketFD,	"\t<TR>\r\n");
		sprintf(lineBuffer, "\t\t<TD>Gravity Z</TD><TD><CENTER>%1.2f</TD>\r\n", cGravity_Z);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"\t</TR>\r\n");

		SocketWriteData(mySocketFD,	"\t<TR>\r\n");
		sprintf(lineBuffer, "\t\t<TD>Gravity T</TD><TD><CENTER>%1.2f</TD>\r\n", cGravity_T);
		SocketWriteData(mySocketFD,	lineBuffer);
		SocketWriteData(mySocketFD,	"\t</TR>\r\n");


		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	}
}

//*****************************************************************************
bool	SlitTrackerDriver::GetCommandArgumentString(const int cmdENum, char *agumentString, char *commentString)
{
bool	foundFlag	=	true;

	switch(cmdENum)
	{
		case kCmd_SlitTracker_DomeAddress:		strcpy(agumentString, "-none-");		break;
		case kCmd_SlitTracker_TrackingEnabled:	strcpy(agumentString, "tracking=BOOL");	break;
		case kCmd_SlitTracker_readall:			strcpy(agumentString, "-none-");		break;


		default:
			strcpy(agumentString, "");
			foundFlag	=	false;
			break;
	}
	return(foundFlag);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	SlitTrackerDriver::Get_DomeAddress(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				domeAddrString[128];

	if (reqData != NULL)
	{
		sprintf(domeAddrString, "%s:%d/%d", cDomeIPaddressString, cDomeAlpacaPort, cDomeAlpacaDevNum);
		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									responseString,
									domeAddrString,
									INCLUDE_COMMA);

		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	SlitTrackerDriver::Get_TrackingEnabled(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									responseString,
									cSlitProp.TrackingEnabled,
									INCLUDE_COMMA);

		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	SlitTrackerDriver::Put_TrackingEnabled(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_PropertyNotImplemented;
bool				trackingFound;
char				trackingString[32];

//	CONSOLE_DEBUG(__FUNCTION__);
	trackingFound	=	GetKeyWordArgument(	reqData->contentData,
											"tracking",
											trackingString,
											(sizeof(trackingString) -1));
	if (trackingFound)
	{
		cSlitProp.TrackingEnabled	=	IsTrueFalse(trackingString);
		alpacaErrCode				=	kASCOM_Err_Success;

		CONSOLE_DEBUG_W_BOOL("cSlitProp.TrackingEnabled\t=", cSlitProp.TrackingEnabled);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InvalidValue;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Parameter incorrect");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	SlitTrackerDriver::Get_Readall(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				textBuffer[64];
int					iii;

	if (reqData != NULL)
	{
		//*	do the common ones first
		Get_Readall_Common(		reqData, alpacaErrMsg);
		Get_DomeAddress(		reqData, alpacaErrMsg, "domeaddress");
		Get_TrackingEnabled(	reqData, alpacaErrMsg, "trackingenabled");


		for (iii=0; iii<kSlitSensorCnt; iii++)
		{

			sprintf(textBuffer, "sensor-%d", iii);
			JsonResponse_Add_Double(	reqData->socket,
										reqData->jsonTextBuffer,
										kMaxJsonBuffLen,
										textBuffer,
										cSlitDistance[iii].distanceInches,
										INCLUDE_COMMA);
		}

		JsonResponse_Add_Double(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"gravity_x",
									cGravity_X,
									INCLUDE_COMMA);

		JsonResponse_Add_Double(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"gravity_y",
									cGravity_Y,
									INCLUDE_COMMA);

		JsonResponse_Add_Double(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"gravity_z",
									cGravity_Z,
									INCLUDE_COMMA);

		JsonResponse_Add_Double(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"gravity_t",
									cGravity_T,
									INCLUDE_COMMA);

		alpacaErrCode	=	kASCOM_Err_Success;
		strcpy(alpacaErrMsg, "");
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
bool	SlitTrackerDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gSlitTrackerCmdTable, getPut);
	return(foundIt);
}



//*****************************************************************************
void	SlitTrackerDriver::OpenSlitTrackerPort(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	cSlitTrackerfileDesc	=	open(cUSBpath, O_RDWR);	//* connect to port
	if (cSlitTrackerfileDesc >= 0)
	{
		Serial_Set_Attribs(cSlitTrackerfileDesc, B9600, 0);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open port", cUSBpath);
	}
}

//*****************************************************************************
void	SlitTrackerDriver::ProcessSlitTrackerLine(char *lineBuff)
{
int				clockValue;
char			*inchesPtr;
double			inchValue;
double			gravityValue;
char			gravityVectorChar;

//	CONSOLE_DEBUG(lineBuff);

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
				inchValue	=	AsciiToDouble(inchesPtr);
//				CONSOLE_DEBUG_W_DBL("inchValue\t=", inchValue);

				cSlitDistance[clockValue].distanceInches	=	inchValue;
				cSlitDistance[clockValue].validData			=	true;
//				cSlitDistance[clockValue].updated			=	true;
				cSlitDistance[clockValue].readCount++;
			}
		}
		else
		{
			CONSOLE_DEBUG_W_STR("clockValue error\t=", lineBuff);
		}
	}
	else if ((lineBuff[0] == '=') && (lineBuff[1] == 'g'))
	{
		//*	we have a gravity reading
		//*	=gX:6.87
		//*	=gY:-0.17
		//*	=gZ:6.99
		//*	=gT:9.80

		gravityVectorChar	=	lineBuff[2];
		gravityValue		=	AsciiToDouble(&lineBuff[4]);
//		CONSOLE_DEBUG(lineBuff);
//		CONSOLE_DEBUG_W_HEX("gravityVectorChar\t=", gravityVectorChar);
//		CONSOLE_DEBUG_W_DBL("gravityValue\t\t=",	gravityValue);

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
				break;

			default:
				CONSOLE_DEBUG("Invalid gravity vector");
				break;
		}
	}
}

#define	kReadBufferSize		256

//#define	_DEBUG_SLIT_TRACKER_DATA_

//*****************************************************************************
void	SlitTrackerDriver::GetSlitTrackerData(void)
{
int		readCnt;
char	readBuffer[kReadBufferSize];
char	theChar;
bool	keepGoing;
int		iii;
int		charsRead;

#ifdef _DEBUG_SLIT_TRACKER_DATA_
//	CONSOLE_DEBUG(__FUNCTION__);
#endif
	if (cSlitTrackerfileDesc >= 0)
	{
		keepGoing	=	true;
		readCnt		=	0;
		while (keepGoing && (readCnt < 20))
		{
			//*	read data from serial port
			charsRead	=	read(cSlitTrackerfileDesc, readBuffer, (kReadBufferSize - 2));
			if (charsRead > 0)
			{
				readCnt++;
				for (iii=0; iii<charsRead; iii++)
				{
					theChar		=	readBuffer[iii];
					if ((theChar >= 0x20) || (theChar == 0x09))
					{
						if (cSlitTrackerByteCnt < (kLineBuffSize - 2))
						{
							cSlitTrackerLineBuf[cSlitTrackerByteCnt++]	=	theChar;
							cSlitTrackerLineBuf[cSlitTrackerByteCnt]	=	0;
						}
						else
						{
							CONSOLE_DEBUG("Buffer overflow");
							CONSOLE_DEBUG_W_NUM("cSlitTrackerByteCnt\t=", cSlitTrackerByteCnt);
							CONSOLE_DEBUG(cSlitTrackerLineBuf);
							cSlitTrackerByteCnt		=	0;
							cSlitTrackerLineBuf[0]	=	0;

							cSlitTrackerBufOverflowCnt++;
						}
					}
					else if ((theChar == 0x0d) || (theChar == 0x0a))
					{
						cSlitTrackerLineBuf[cSlitTrackerByteCnt]	=	0;
					#ifdef _DEBUG_SLIT_TRACKER_DATA_
						CONSOLE_DEBUG(cSlitTrackerLineBuf);
					#endif
						if (strlen(cSlitTrackerLineBuf) > 0)
						{
							ProcessSlitTrackerLine(cSlitTrackerLineBuf);
						}
						cSlitTrackerByteCnt		=	0;
						cSlitTrackerLineBuf[0]	=	0;
					}
				}
			}
			else
			{
				//*	no more data to read for now
				keepGoing	=	false;
			}
		}
//		if (readCnt > 0)
//		{
//			CONSOLE_DEBUG_W_NUM("readCnt\t=", readCnt);
//			if (readCnt > 9)
//			{
//				//*	slow the read rate down
//				SendSlitTrackerCmd("+");
//			}
//		}
	}
	else
	{
		CONSOLE_DEBUG("Slit tracker port not open");
//		CONSOLE_ABORT(__FUNCTION__);
	}
}


//*****************************************************************************
void	SlitTrackerDriver::SendSlitTrackerCmd(const char *cmdBuffer)
{
int	sLen;
int	bytesWritten;

	CONSOLE_DEBUG_W_STR("cmdBuffer\t=", cmdBuffer);

	if (cSlitTrackerfileDesc >= 0)
	{
		sLen			=	strlen(cmdBuffer);
		bytesWritten	=	write(cSlitTrackerfileDesc, cmdBuffer, sLen);
		if (bytesWritten < 0)
		{
			CONSOLE_DEBUG_W_NUM("write failure, errno\t=", errno);
		}
	}
}

//**************************************************************************************
static void ProcessSlittrackerConfigEntry(const char *keyword, const char *value, void *userDataPtr)
{
SlitTrackerDriver	*slitTrackerObjPtr;

//	CONSOLE_DEBUG_W_STR(keyword, value);
	slitTrackerObjPtr	=	(SlitTrackerDriver *)userDataPtr;
	if (slitTrackerObjPtr != NULL)
	{
		if (strcasecmp(keyword, "DOMEIP") == 0)
		{
			strcpy(slitTrackerObjPtr->cDomeIPaddressString, value);
		}
		else if (strcasecmp(keyword, "DOMEPORT") == 0)
		{
			slitTrackerObjPtr->cDomeAlpacaPort	=	atoi(value);
		}
		else if (strcasecmp(keyword, "DEVICENUM") == 0)
		{
			slitTrackerObjPtr->cDomeAlpacaDevNum	=	atoi(value);
		}
	}
}

//**************************************************************************************
void	SlitTrackerDriver::ReadSlittrackerConfig(void)
{
int		linesRead;

	//*	returns # of processed lines
	//*	-1 means failed to open config file
	linesRead	=	ReadGenericConfigFile(	gSlitTrackerConfigFile,
											'=',
											&ProcessSlittrackerConfigEntry,
											this);
	if (linesRead > 0)
	{
//		CONSOLE_DEBUG_W_STR("Dome IP addr\t=",	cDomeIPaddressString);
//		CONSOLE_DEBUG_W_NUM("Dome Port   \t=",	cDomeAlpacaPort);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read config file", gSlitTrackerConfigFile);
	}

//	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
static const char	gSlitTrackerInfo[]	=
{
	"The AlpacaPi SlitTracker allows the dome slit to follow the telescope.\r\n"
	"This is done with acoustic sensors mounted around the aperture of the telescope\r\n"
	"that ping the distance to the inside of the dome.\r\n"
	"<P>\r\n"
	"This page allows you to configure which Alpaca Dome device to send the commands to.\r\n"
	"This can also be configured by editing 'slittracker-config.txt'\r\n"
	"on the unit that is running the SlitTracker driver."
};

//*****************************************************************************
//*	https://www.w3schools.com/html/html_forms.asp
//*****************************************************************************
bool	SlitTrackerDriver::Setup_OutputForm(TYPE_GetPutRequestData *reqData, const char *formActionString)
{
FILE		*filePointer;
int			mySocketFD;
char		lineBuff[256];
const char	webpageTitle[]	=	"AlpacaPi Slit-tracker setup";

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("The Action that will be preformed when save is pressed:", formActionString);
	cSetupChangeOccured	=	false;
	mySocketFD			=	reqData->socket;

	SocketWriteData(mySocketFD,	gHtmlHeader);

	SocketWriteData(mySocketFD,	"<!DOCTYPE html>\r\n");
	SocketWriteData(mySocketFD,	"<HTML lang=\"en\">\r\n");
	sprintf(lineBuff,			"<TITLE>%s</TITLE>\r\n", webpageTitle);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	sprintf(lineBuff,			"<H1>%s</H1>\r\n", webpageTitle);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	SocketWriteData(mySocketFD,	gSlitTrackerInfo);

	sprintf(lineBuff, "<form action=\"%s\">\r\n", formActionString);
	SocketWriteData(mySocketFD,	lineBuff);

	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
	//----------------------------------------------------
	//*	table header
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TH COLSPAN=3>Alpaca Dome connection</TH>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	//----------------------------------------------------
	SocketWriteData(mySocketFD,	"<TR>\r\n");

	//---------------------
	//*	IP address
	SocketWriteData(mySocketFD,	"<TD>\r\n");
	SocketWriteData(mySocketFD,	"<label for=\"name\">IP Address:</label><br>\r\n");
	sprintf(	lineBuff,
				"<input type=\"text\" id=\"ipaddr\" name=\"ipaddr\" value=\"%s\">\r\n",
				cDomeIPaddressString);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"</TD>\r\n");

	//---------------------
	//*	IP port
	SocketWriteData(mySocketFD,	"<TD>\r\n");
	SocketWriteData(mySocketFD,	"<label for=\"name\">Port:</label><br>\r\n");
	sprintf(	lineBuff,
				"<input type=\"text\" id=\"port\" name=\"port\" value=\"%d\">\r\n",
				cDomeAlpacaPort);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"</TD>\r\n");

	//---------------------
	//*	Alpaca device number (defaults to zero)
	SocketWriteData(mySocketFD,	"<TD>\r\n");
	SocketWriteData(mySocketFD,	"<label for=\"name\">DeviceNum:</label><br>\r\n");
	sprintf(	lineBuff,
				"<input type=\"text\" id=\"devnum\" name=\"devnum\" value=\"%d\">\r\n",
				cDomeAlpacaDevNum);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"</TD>\r\n");

	//----------------------------------------------------------------------
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TD COLSPAN=3><CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<input type=\"submit\" value=\"Save\">\r\n");
	SocketWriteData(mySocketFD,	"</TD>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");
	SocketWriteData(mySocketFD,	"</form>\r\n");
	SocketWriteData(mySocketFD,	"<P>\r\n");

	//----------------------------------------------------------------------
	//*	now display the current config file
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, gSlitTrackerConfigFile);

	//*	open the file specified
	filePointer	=	fopen(gSlitTrackerConfigFile, "r");
	if (filePointer != NULL)
	{
	char	fileBuffer[2000];
	size_t	byteCount;

		byteCount				=	fread(fileBuffer, 1, (sizeof(fileBuffer)), filePointer);
		fileBuffer[byteCount]	=	0;
//		CONSOLE_DEBUG_W_SIZE("byteCount\t=", byteCount);

		SocketWriteData(mySocketFD,	"<CENTER>\r\n");
		SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
		SocketWriteData(mySocketFD,	"<TR><TH>Current config file<BR>(");
		SocketWriteData(mySocketFD,	gSlitTrackerConfigFile);
		SocketWriteData(mySocketFD,	")</TH></TR>\r\n");

		SocketWriteData(mySocketFD,	"<TR>\r\n");
		SocketWriteData(mySocketFD,	"<TD><PRE>\r\n");

		SocketWriteData(mySocketFD,	fileBuffer);

		SocketWriteData(mySocketFD,	"</PRE>\r\n");

		SocketWriteData(mySocketFD,	"</TD>\r\n");
		SocketWriteData(mySocketFD,	"</TR>\r\n");
		SocketWriteData(mySocketFD,	"</TABLE>\r\n");
		SocketWriteData(mySocketFD,	"</CENTER>\r\n");

		fclose(filePointer);
	}

	sprintf(	lineBuff,
				"<a href=http://%s:%d/ TARGET=DOME>Click to verify Dome connection</A>\r\n",
				cDomeIPaddressString,
				cDomeAlpacaPort);
	SocketWriteData(mySocketFD,	lineBuff);


	return(true);
}

//*****************************************************************************
void	SlitTrackerDriver::Setup_SaveInit(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	cSetupChangeOccured	=	false;
}

//*****************************************************************************
void	SlitTrackerDriver::Setup_SaveFinish(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("cDomeIPaddressString\t=",	cDomeIPaddressString);
	CONSOLE_DEBUG_W_NUM("cDomeAlpacaPort     \t=",	cDomeAlpacaPort);
	CONSOLE_DEBUG_W_NUM("cDomeAlpacaDevNum   \t=",	cDomeAlpacaDevNum);
	if (cSetupChangeOccured)
	{
		//*	write out a new config file
		WriteConfigFile();
		cSetupChangeOccured	=	false;
	}
	else
	{
		CONSOLE_DEBUG("No change occurred");
	}
}

//*****************************************************************************
void	SlitTrackerDriver::WriteConfigFile(void)
{
FILE				*filePointer;
struct timeval		timeStamp;
char				timeStampString[128];

//	CONSOLE_DEBUG(__FUNCTION__);

	filePointer	=	fopen(gSlitTrackerConfigFile, "w");
	if (filePointer != NULL)
	{
		gettimeofday(&timeStamp, NULL);
		FormatDateTimeString_Local(&timeStamp, timeStampString);

		fprintf(filePointer, "#####################################################################\n");
		fprintf(filePointer, "#AlpacaPi Project - %s\n",	gFullVersionString);
		fprintf(filePointer, "#Slit Tracker config file\n");
		fprintf(filePointer, "#Created %s\n",				timeStampString);
		fprintf(filePointer, "DOMEIP   \t=\t%s\n",			cDomeIPaddressString);
		fprintf(filePointer, "DOMEPORT \t=\t%d\n",			cDomeAlpacaPort);
		fprintf(filePointer, "DEVICENUM\t=\t%d\n",			cDomeAlpacaDevNum);

		fclose(filePointer);
	}
}

//*****************************************************************************
bool	SlitTrackerDriver::Setup_ProcessKeyword(const char *keyword, const char *valueString)
{
int			newPortNumber;
int			newDeviceNumber;
uint32_t	iii;
size_t		slen;
int			dotCounter;

//	CONSOLE_DEBUG_W_2STR("kw:value", keyword, valueString);

	if (strcasecmp(keyword, "ipaddr") == 0)
	{
		//*	check for valid IP address
		dotCounter	=	0;
		slen		=	strlen(valueString);
		if ((slen >= 7) && (slen < sizeof(cDomeIPaddressString)))
		{
			for (iii = 0; iii < slen; iii++)
			{
				if (valueString[iii] == '.')
				{
					dotCounter++;
				}
			}
			if (dotCounter == 3)
			{
				if (strcmp(valueString, cDomeIPaddressString) != 0)
				{
					strcpy(cDomeIPaddressString, valueString);
					cSetupChangeOccured	=	true;
				}
			}
			else
			{
				CONSOLE_DEBUG_W_STR("Invalid ip address\t=", valueString);
			}
		}
		else
		{
			CONSOLE_DEBUG_W_STR("Invalid ip address\t=", valueString);
		}
	}
	else if (strcasecmp(keyword, "port") == 0)
	{
		newPortNumber	=	atoi(valueString);
		//*	check for valid port number
		if ((newPortNumber > 0) && (newPortNumber <= 65535))
		{
			if (newPortNumber != cDomeAlpacaPort)
			{
				cDomeAlpacaPort		=	newPortNumber;
				cSetupChangeOccured	=	true;
			}
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Invalid port number\t=", newPortNumber);
		}
	}
	else if (strcasecmp(keyword, "devnum") == 0)
	{
		newDeviceNumber	=	atoi(valueString);
		//*	check for valid device number
		if ((newDeviceNumber >= 0) && (newDeviceNumber <= 64))
		{
			if (newDeviceNumber != cDomeAlpacaDevNum)
			{
				cDomeAlpacaDevNum	=	newDeviceNumber;
				cSetupChangeOccured	=	true;
			}
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("Invalid device number\t=", newDeviceNumber);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_2STR("kw:value", keyword, valueString);
//		CONSOLE_ABORT(__FUNCTION__);
	}

	return(true);
}


#endif	//	_ENABLE_SLIT_TRACKER_
