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
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	May  2,	2020	<MLS> Created slittracker.cpp
//*	May 30,	2020	<MLS> Added gravity vector parsing
//*	May 30,	2020	<MLS> Added gravity to readall output
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

#include	"serialport.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"JsonResponse.h"
#include	"eventlogging.h"
#include	"slittracker.h"



//*****************************************************************************
const TYPE_CmdEntry	gSlitTrackerCmdTable[]	=
{
//?	{	"setrate",				kCmd_SlitTracker_setrate,		kCmdType_PUT	},
	{	"readall",				kCmd_SlitTracker_readall,		kCmdType_GET	},

	{	"",						-1,	0x00	}
};

//**************************************************************************************
typedef struct
{
	char	usbPath[24];
} TYPE_USB_PATH;

//**************************************************************************************
void	CreateSlitTrackerObjects(void)
{
int		accessRC;
int		iii;
bool	keepGoing;
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
	strcpy(cCommonProp.Description,	"Slit tracking  for dome slaving");
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
	JsonResponse_CreateHeader(reqData->jsonTextBuffer, kMaxJsonBuffLen);

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
		case kCmd_SlitTracker_setrate:
//+			alpacaErrCode	=	Put_Setrate(reqData, alpacaErrMsg);
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
							kMaxJsonBuffLen,
							kInclude_HTTP_Header);
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

		//*	now generate links to all of the commands
		GenerateHTMLcmdLinkTable(mySocketFD, "slittracker", 0, gSlitTrackerCmdTable);
	}
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
		Get_Readall_Common(			reqData, alpacaErrMsg);

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

	CONSOLE_DEBUG(__FUNCTION__);

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
				inchValue	=	atof(inchesPtr);
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
		gravityValue		=	atof(&lineBuff[4]);
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

//*****************************************************************************
void	SlitTrackerDriver::GetSlitTrackerData(void)
{
int		readCnt;
char	readBuffer[kReadBufferSize];
char	theChar;
bool	keepGoing;
int		iii;
int		charsRead;


//	CONSOLE_DEBUG(__FUNCTION__);
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
//						CONSOLE_DEBUG(cSlitTrackerLineBuf);
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
		if (readCnt > 0)
		{
	//		CONSOLE_DEBUG_W_NUM("readCnt\t=", readCnt);
	//		if (readCnt > 9)
	//		{
	//			//*	slow the read rate down
	//			SendSlitTrackerCmd("+");
	//		}
		}
	}
	else
	{
//		CONSOLE_DEBUG("Slit tracker port not open");
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

#endif	//	_ENABLE_SLIT_TRACKER_
