//**************************************************************************
//*	Name:			shutterdriver_arduino.cpp
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
//*	Feb 11,	2020	<MLS> Created shutterdriver_arduino.cpp
//*	Feb 11,	2020	<MLS> Started on Shutter control via Arduino
//*	Feb 11,	2020	<MLS> R-Pi is talking to Arduino and retrieving log info
//*	Feb 13,	2020	<MLS> Shutter Open/Close/Stop working
//*	Feb 13,	2020	<MLS> Reading shutter status from Arduino
//*	May  8,	2020	<MLS> Changed Arduino to continuous status output
//*	May  8,	2020	<MLS> Updated Arduino communications code to handle new format
//*	Feb 26,	2021	<MLS> Added logic to look for different USB ports ttyACM0 -> ttyACM4
//*****************************************************************************

#ifdef _ENABLE_SHUTTER_
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<errno.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<unistd.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"eventlogging.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"shutterdriver.h"
#include	"shutterdriver_arduino.h"

#include	"serialport.h"




//*****************************************************************************
void	CreateShuterArduinoObjects(void)
{
	new ShutterArduino(0);
	gVerbose		=	false;
}


//*****************************************************************************
ShutterArduino::ShutterArduino(const int argDevNum)
	:ShutterDriver(argDevNum)
{
	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name, "Arduino-shutter");
	strcpy(gWebTitle, "Arduino-shutter");

	cLogBuffer[0]			=	0;
	cShutterStatus			=	kShutterStatus_Unknown;
	cPreviousShutterStatus	=	kShutterStatus_Unknown;

	cNoDataCnt				=	0;
	cArduinoSkipCnt			=	0;

	Init_Hardware();

}

//*****************************************************************************
// Destructor
//*****************************************************************************
ShutterArduino::~ShutterArduino(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
void		ShutterArduino::ProcessArduinoState(char *stateString)
{
	CONSOLE_DEBUG_W_STR("stateString\t=", stateString);

	if (strncasecmp(stateString, "Idle", 4) != 0)
	{
		cIdleStateCnt	=	0;
	}


	if (strncasecmp(stateString, "Idle", 4) == 0)
	{
		cIdleStateCnt++;
		CONSOLE_DEBUG("IDLE---");
		switch(cShutterStatus)
		{
			case kShutterStatus_Open:
			case kShutterStatus_Closed:
				//*	do nothing
				break;

			case kShutterStatus_Opening:
				if (cIdleStateCnt > 10)
				{
					if (cAltitude_Dbl > 30.0)
					{
						cShutterStatus	=	kShutterStatus_Open;
					}
				}
				break;

			case kShutterStatus_Closing:
				//*	not sure what to do here.
				break;


			case kShutterStatus_Error:
			case kShutterStatus_Unknown:
				CONSOLE_DEBUG_W_DBL("cAltitude_Dbl\t=", cAltitude_Dbl);
				if (cAltitude_Dbl > 30.0)
				{
					cShutterStatus	=	kShutterStatus_Open;
				}
				else if (cAltitude_Dbl > 2.0)
				{
					cShutterStatus	=	kShutterStatus_Opening;
				}
				CONSOLE_DEBUG_W_NUM("cShutterStatus\t=", cShutterStatus);
				break;
		}
	}
	else if (strncasecmp(stateString, "Open", 4) == 0)
	{
		cShutterStatus	=	kShutterStatus_Opening;
	}
	else if (strncasecmp(stateString, "Close", 5) == 0)
	{
		cShutterStatus	=	kShutterStatus_Closing;
	}
}

//*****************************************************************************
void		ShutterArduino::ProcessArduinoLine(char *readBuffer)
{
char		*equalsPtr;
char		doorStatusStr[32];
char		stateString[32];
double		myPercentOpen;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	look for the string "Door is"
	if (strncasecmp(readBuffer, "Door is", 5) == 0)
	{
		equalsPtr	=	strchr(readBuffer, '=');
		if (equalsPtr != NULL)
		{
			equalsPtr++;
			strcpy(doorStatusStr, equalsPtr);
			CONSOLE_DEBUG_W_STR("Door Status\t=", doorStatusStr);
			if (strcasecmp(doorStatusStr, "CLOSED") == 0)
			{
				CONSOLE_DEBUG("Door is CLOSED");
				cShutterStatus	=	kShutterStatus_Closed;
			}
			else if (strcasecmp(doorStatusStr, "OPEN") == 0)
			{
				CONSOLE_DEBUG("Door is OPEN");
				cShutterStatus	=	kShutterStatus_Open;
			}
			else if (strcasecmp(doorStatusStr, "UNKNOWN") == 0)
			{

			}
		}
	}
	else if (strncasecmp(readBuffer, "State", 5) == 0)
	{
		equalsPtr	=	strchr(readBuffer, '=');
		if (equalsPtr != NULL)
		{
			equalsPtr++;
			strcpy(stateString, equalsPtr);
			ProcessArduinoState(stateString);

		}
	}
	else if (strncasecmp(readBuffer, "PercentOpen", 11) == 0)
	{
		equalsPtr	=	strchr(readBuffer, '=');
		if (equalsPtr != NULL)
		{
			equalsPtr++;
			myPercentOpen	=	atof(equalsPtr);
			if (myPercentOpen > cAltitude_Dbl)
			{
				cShutterStatus	=	kShutterStatus_Opening;
			}
			else if (myPercentOpen < cAltitude_Dbl)
			{
				cShutterStatus	=	kShutterStatus_Closing;
			}
			cAltitude_Dbl	=	myPercentOpen;
		//	CONSOLE_DEBUG_W_DBL("cAltitude_Dbl\t=", cAltitude_Dbl);
		}
	}
	else if (strncasecmp(readBuffer, "LogStart", 8) == 0)
	{
		strcpy(cLogBuffer, "");
		cLogLineCnt		=	0;
	}
	else if (strncasecmp(readBuffer, "Time=", 5) == 0)
	{
		if ((strlen(cLogBuffer) + strlen(readBuffer) + 5) < kArduino_LogBufferSize)
		{
			strcat(cLogBuffer, readBuffer);
			strcat(cLogBuffer, "\r");
			cLogLineCnt++;
		}
		else
		{
			CONSOLE_DEBUG("Log buffer overflow, clearing log buffer");
			strcpy(cLogBuffer, "");
			cLogLineCnt		=	0;
		}
	}
	else if (strncasecmp(readBuffer, "TickCount", 9) == 0)
	{
		//*	ignore
	}
	else if (strncasecmp(readBuffer, "Seconds", 7) == 0)
	{
		//*	ignore
	}
	else
	{
		//*	haven't figured out what to do with the data yet
		CONSOLE_DEBUG(readBuffer);
	}
}

#define	kReadBufferSize		256

//*****************************************************************************
void	ShutterArduino::ReadArduinoData(void)
{
int		readCnt;
char	readBuffer[kReadBufferSize];
char	theChar;
bool	keepGoing;
int		iii;
int		charsRead;


//	CONSOLE_DEBUG(__FUNCTION__);
	if (cArduinoFileDesc >= 0)
	{
		keepGoing	=	true;
		readCnt		=	0;
		while (keepGoing && (readCnt < 20))
		{
			//*	read data from serial port
			charsRead	=	read(cArduinoFileDesc, readBuffer, (kReadBufferSize - 2));
			if (charsRead > 0)
			{
				readCnt++;
				for (iii=0; iii<charsRead; iii++)
				{
					theChar		=	readBuffer[iii];
					if ((theChar >= 0x20) || (theChar == 0x09))
					{
						if (cArduinoByteCnt < (kArduino_LineBuffSize - 2))
						{
							cArduinoLineBuf[cArduinoByteCnt++]	=	theChar;
							cArduinoLineBuf[cArduinoByteCnt]	=	0;
						}
						else
						{
							CONSOLE_DEBUG("Buffer overflow");
						//	CONSOLE_DEBUG_W_STR("Buffer overflow, current buf =", cArduinoLineBuf);
							cArduinoByteCnt		=	0;
							cArduinoLineBuf[0]	=	0;
						}
					}
					else if ((theChar == 0x0d) || (theChar == 0x0a))
					{
						cArduinoLineBuf[cArduinoByteCnt]	=	0;
						if (strlen(cArduinoLineBuf) > 2)
						{
							CONSOLE_DEBUG_W_STR("Ard:", cArduinoLineBuf);
							ProcessArduinoLine(cArduinoLineBuf);
						}
						cArduinoByteCnt		=	0;
						cArduinoLineBuf[0]	=	0;
					}
				}
			}
			else
			{
				//*	no more data to read for now
				keepGoing	=	false;
			}
		}
	}
	else
	{
//		CONSOLE_DEBUG("Arduino serial port not open");
	}
}

//*****************************************************************************
//*	return number of microseconds allowed for delay
//*****************************************************************************
int32_t		ShutterArduino::RunStateMachine(void)
{
int32_t		minDealy_microSecs;
char		stateString[48];

	minDealy_microSecs	=	50000;
	if ((gImageDownloadInProgress == false) || (cArduinoSkipCnt > 100))
	{
		ReadArduinoData();

		if (cShutterStatus != cPreviousShutterStatus)
		{
			Shutter_GetStatusString(cShutterStatus, stateString);
			LogEvent(	"shutter",
						"StateChanged",
						NULL,
						kASCOM_Err_Success,
						stateString);
			cPreviousShutterStatus	=	cShutterStatus;
		}
		cArduinoSkipCnt	=	0;;
	}
	else
	{
	//	CONSOLE_DEBUG("Skipping Arduino update due to image downloading");
		cArduinoSkipCnt++;
	}
	return(minDealy_microSecs);
}

//*****************************************************************************
void	ShutterArduino::Init_Hardware(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	OpenArduinoConnection();
	sleep(1);
//-	GetArduinoLog();
}




//*****************************************************************************
void	ShutterArduino::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
int		mySocketFD;
char	lineBuffer[128];
int		cc;
int		ii;
int		sLen;

//	CONSOLE_DEBUG(__FUNCTION__);

	mySocketFD	=	reqData->socket;
	SocketWriteData(mySocketFD,	"<PRE>\r\n");
	sLen			=	strlen(cLogBuffer);
	cc				=	0;
	lineBuffer[0]	=	0;
	for (ii=0; ii<sLen; ii++)
	{
		if (((cLogBuffer[ii] >= 0x20) || (cLogBuffer[ii] == 0x09)) && (strlen(lineBuffer) < 100))
		{
			lineBuffer[cc++]	=	cLogBuffer[ii];
			lineBuffer[cc]		=	0;
		}
		else
		{
		//	CONSOLE_DEBUG_W_STR("lineBuffer=", lineBuffer);
			strcat(lineBuffer, "\r\n");
			SocketWriteData(mySocketFD,	lineBuffer);
			cc				=	0;
			lineBuffer[0]	=	0;
		}
	}
	SocketWriteData(mySocketFD,	"</PRE>\r\n");
}

#pragma mark -

//*****************************************************************************
bool	ShutterArduino::OpenShutter(void)
{
bool	successFlag;

	CONSOLE_DEBUG(__FUNCTION__);
	successFlag	=	SendCommand("open\r");
	if (!successFlag)
	{
		CONSOLE_DEBUG("Open failed");
	}
	return(successFlag);
}


//*****************************************************************************
bool	ShutterArduino::CloseShutter(void)
{
bool	successFlag;

	CONSOLE_DEBUG(__FUNCTION__);
	successFlag	=	SendCommand("close\r");
	if (!successFlag)
	{
		CONSOLE_DEBUG("Close failed");
	}
	return(successFlag);
}


//*****************************************************************************
bool	ShutterArduino::StopShutter(void)
{
bool	successFlag;

	CONSOLE_DEBUG(__FUNCTION__);
	successFlag	=	SendCommand("stop\r");
	if (!successFlag)
	{
		CONSOLE_DEBUG("Stop failed");
	}
	return(successFlag);
}

#pragma mark -

//*****************************************************************************
bool	ShutterArduino::OpenArduinoConnection(void)
{
bool		openOK;
char		serialPortPath[48]	=	"/dev/ttyACM0";
//char		serialPortPath[48]	=	"/dev/ttyUSB0";
struct stat	fileStatus;
int			returnCode;
int			usbPortNum;

	//*	find the Arduino
	usbPortNum	=	0;
	returnCode	=	1;
	//*	fstat - check for existence of file
	while ((returnCode != 0) && (usbPortNum <= 4))
	{
		sprintf(serialPortPath, "/dev/ttyACM%d", usbPortNum);
		CONSOLE_DEBUG_W_STR("Checking for", serialPortPath);
		returnCode	=	stat(serialPortPath, &fileStatus);

		usbPortNum++;
	}
	if (returnCode != 0)
	{
		CONSOLE_ABORT("Did not find the Arduino!!!!!!!")
	}

	CONSOLE_DEBUG_W_STR("port is", serialPortPath);

	cArduinoFileDesc	=	open(serialPortPath, O_RDWR);	//* connect to port
	if (cArduinoFileDesc >= 0)
	{
		openOK	=	true;
		Set_Serial_attribs(cArduinoFileDesc, B115200, 0);	//*	set the baud rate

	}
	else
	{
		CONSOLE_DEBUG_W_STR("failed to open", serialPortPath);
		openOK	=	false;
	}
	return(openOK);
}

//**************************************************************************************
bool	ShutterArduino::SendCommand(const char *theCommand)
{
int		sLen;
int		bytesWritten;
bool	sentOK;

//	CONSOLE_DEBUG_W_STR("Sending command=", theCommand);

	sentOK	=	false;
	if (cArduinoFileDesc >= 0)
	{
		sLen			=	strlen(theCommand);
		bytesWritten	=	write(cArduinoFileDesc, theCommand, sLen);
		if (bytesWritten > 0)
		{
			sentOK			=	true;
		}
		else if (bytesWritten < 0)
		{
			CONSOLE_DEBUG_W_NUM("Error occurred on write, errno=", errno);
		}
		tcflush(cArduinoFileDesc, TCOFLUSH);

		usleep(5000);
	}
	return(sentOK);
}



#endif		//	_ENABLE_SHUTTER_
