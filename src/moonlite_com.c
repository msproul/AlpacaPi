//*****************************************************************************
//*	Name:			moonlite_com.c
//*
//*	Author:			Mark Sproul (C) 2020
//*
//*	Description:	Serial communications to moonlite focusers
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*	References:
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Feb 21,	2020	<MLS> Created moonlite_com.c
//*	Feb 21,	2020	<MLS> Moving moonlite serial code to separate file
//*	Feb 27,	2020	<MLS> Added MoonLite_SendCommand()
//*	Feb 27,	2020	<MLS> Added MoonLite_GetSwiches() & MoonLite_GetAuxSwiches()
//*	Feb 27,	2020	<MLS> Added MoonLite_StopMotors()
//*	Feb 29,	2020	<MLS> Added MoonLite_StopAxis()
//*	Mar 10,	2020	<MLS> Added ttyACM to the list of possible focusers
//*	Mar 11,	2020	<MLS> Added MoonLite_CheckIfNiteCrawler() & MoonLite_CheckIfHighRes()
//*	Mar 11,	2020	<MLS> Working with hires controler (Arduino based)
//*	Mar 17,	2020	<MLS> Fixed bug, /dev directory was not being closed
//*	Apr 19,	2020	<MLS> Added MoonLite_CloseFocuserConnection()
//*	Apr 26,	2020	<MLS> Added MoonLite_GetNC_Color()
//*	Apr 26,	2020	<MLS> Added MoonLite_FlushReadBuffer()
//*	Jun 30,	2020	<MLS> Added _DEBUG_NITECRAWLER_DETECTION_
//*	Dec 28,	2020	<MLS> Added MoonLite_SetCurrentPosition()
//*	Nov 27,	2022	<MLS> Added more debugging to USB_SendCommand()
//*	Nov 28,	2022	<MLS> Added MoonLite_GetMovingState()
//*	Dec 11,	2022	<MLS> Fixed sign error for HiRes in MoonLite_GetTemperature()
//*****************************************************************************

#if defined(_ENABLE_FOCUSER_MOONLITE_) || defined(_ENABLE_ROTATOR_NITECRAWLER_) || defined(_ENABLE_CTRL_FOCUSERS_)

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<fcntl.h>
#include	<termios.h>
#include	<errno.h>
#include	<dirent.h>
#include	<unistd.h>

#ifdef _ENABLE_FOCUSER_
	#include	"eventlogging.h"
#endif

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#define	_USE_SENDCOMMAND_
#define	_DEBUG_NITECRAWLER_DETECTION_

#include	"serialport.h"

#include	"moonlite_com.h"


char	gLastCmdSent[48]	=	"";

static int	USB_SendCommand(TYPE_MOONLITECOM *moonliteCom, const char *theCommand);
static int	ReadUntilChar(const int fileDesc, char *readBuff, const int maxChars, const char terminator);
static int	gMoonLiteReadFailureCnt	=	0;

//*****************************************************************************
typedef struct
{
	char	deviceString[24];
} TYPE_SERIALPORTS;

#define	kMaxMoonliteFocusers	5

static TYPE_SERIALPORTS	gValidSerialPorts[kMaxMoonliteFocusers];
static int					gValidPortCnt	=	0;

//*****************************************************************************
static int	DevicePathSort(const void *e1, const void* e2)
{
int					retValue;
TYPE_SERIALPORTS	*entry1;
TYPE_SERIALPORTS	*entry2;

	entry1		=	(TYPE_SERIALPORTS *)e1;
	entry2		=	(TYPE_SERIALPORTS *)e2;
	retValue	=	strcmp(entry1->deviceString, entry2->deviceString);
	return(retValue);
}


//**************************************************************************************
//*	open the /dev devices and see if there are moonlite focusers
int	MoonLite_CountFocusers(void)
{
int				moonliteCnt;
DIR				*directory;
struct dirent	*dir;
bool			keepGoing;
int				errorCode;

	CONSOLE_DEBUG(__FUNCTION__);
	moonliteCnt	=	0;

	directory	=	opendir("/dev/");
	if (directory != NULL)
	{
		keepGoing	=	true;
		while (keepGoing)
		{
			dir	=	readdir(directory);
			if (dir != NULL)
			{
				if (strncmp(dir->d_name, "ttyUSB", 6) == 0)
				{
					moonliteCnt++;

					CONSOLE_DEBUG_W_STR("Focuser found->", dir->d_name);

			#ifdef _ENABLE_FOCUSER_
				//	LogEvent(	"focuser",
				//				"Moonlite focuser found",
				//				NULL,
				//				kASCOM_Err_Success,
				//				dir->d_name);
			#endif
					if (gValidPortCnt < kMaxMoonliteFocusers)
					{
						strcpy(gValidSerialPorts[gValidPortCnt].deviceString, "/dev/");
						strcat(gValidSerialPorts[gValidPortCnt].deviceString, dir->d_name);
						gValidPortCnt++;
					}
				}
			}
			else
			{
				keepGoing	=	false;
			}
		}
		//*	if there are more than one, sort them so there is consistency.
		//*	accessing the file path does not always guarantee the same order
		if  (gValidPortCnt > 1)
		{
			qsort(gValidSerialPorts, gValidPortCnt, sizeof(TYPE_SERIALPORTS), DevicePathSort);
		}
		errorCode	=	closedir(directory);
		if (errorCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("closedir errorCode\t=", errorCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}
	}
	else
	{
		CONSOLE_DEBUG("Failed to open /dev");
	#ifdef _ENABLE_FOCUSER_
		LogEvent(	"focuser",
					"Failure",
					NULL,
					kASCOM_Err_Success,
					"Failed to open /dev");
	#endif
	}

	return(moonliteCnt);
}


//*****************************************************************************
void	MoonLite_InitCom(TYPE_MOONLITECOM *moonliteCom, int serialPortIndex)
{
	memset(moonliteCom, 0, sizeof(TYPE_MOONLITECOM));
	if ((serialPortIndex >= 0) && (serialPortIndex < gValidPortCnt))
	{
		strcpy(moonliteCom->usbPortPath, gValidSerialPorts[serialPortIndex].deviceString);
	}
}

//*****************************************************************************
//*	returns true if its a NiteCrawler
//*****************************************************************************
bool	MoonLite_CheckIfNiteCrawler(TYPE_MOONLITECOM *moonliteCom)
{
int		tryCounter;
int		failureCnt;
bool	isNiteCrawler;
char	readBuffer[48];
int		readCnt;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	set the baud rate to 57600
	//*	also sets timeout
	Serial_Set_Attribs(moonliteCom->fileDesc, B57600, 0);
	tryCounter		=	0;
	failureCnt		=	0;
	isNiteCrawler	=	false;
	strcpy(moonliteCom->deviceModelString, "");
	//*	sometimes we dont get this on the first try
	while ((strlen(moonliteCom->deviceModelString) == 0) && (tryCounter < 3))
	{
		//*	get the model
		USB_SendCommand(moonliteCom, "PF");
		readCnt	=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');
		if (readCnt > 3)
		{
		//	CONSOLE_DEBUG_W_NUM("readCnt\t=",	readCnt);
			CONSOLE_DEBUG_W_STR("version is=",	readBuffer);
			readBuffer[readCnt -1]	=	0;	//*	get rid of the trailing "#"
			strcpy(moonliteCom->deviceModelString, "NiteCrawler-");
			strcat(moonliteCom->deviceModelString, readBuffer);
			isNiteCrawler	=	true;

			moonliteCom->model			=	kMoonLite_NiteCrawler;
			moonliteCom->stepsPerRev	=	kStepsPerRev_WR30;	//*	default
			if (strncmp(readBuffer, "2.5", 3) == 0)
			{
				moonliteCom->stepsPerRev	=	kStepsPerRev_WR25;
			}
			else if (strncmp(readBuffer, "3.0", 3) == 0)
			{
				moonliteCom->stepsPerRev	=	kStepsPerRev_WR30;
			}
			else if (strncmp(readBuffer, "3.5", 3) == 0)
			{
				moonliteCom->stepsPerRev	=	kStepsPerRev_WR35;
			}
			CONSOLE_DEBUG_W_NUM("stepsPerRev\t=",	moonliteCom->stepsPerRev);
		}
		else
		{
			failureCnt++;
		//	CONSOLE_DEBUG("Failed to read NiteCrawler model");
		}
		//*	read again because it gives out 2 "#"
		readCnt	=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');

		tryCounter++;
	}
	if (tryCounter > 1)
	{
#ifdef _ENABLE_FOCUSER_
	//	LogEvent(	"focuser",
	//				"Trouble getting version from NiteCrawler",
	//				NULL,
	//				kASCOM_Err_Success,
	//				NULL);
#endif
	}
	//*	get the version
	USB_SendCommand(moonliteCom, "PV");
	readCnt	=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');
	if (readCnt > 0)
	{
		readBuffer[readCnt -1]	=	0;	//*	get rid of the trailing "#"
		strcpy(moonliteCom->deviceVersion, readBuffer);
		CONSOLE_DEBUG_W_STR("deviceVersion\t=", readBuffer);
	}
	else
	{
		failureCnt++;
	}
	readCnt	=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');

	//*	get the serial number
	USB_SendCommand(moonliteCom, "PS");
	readCnt	=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');
	if (readCnt > 0)
	{
		readBuffer[readCnt -1]	=	0;	//*	get rid of the trailing "#"
		strcpy(moonliteCom->deviceSerialNum, readBuffer);
		CONSOLE_DEBUG_W_STR("deviceSerialNum\t=", readBuffer);
	}
	else
	{
		failureCnt++;
	}
	readCnt	=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');

//		USB_SendCommand(moonliteCom, "GH");
//		USB_SendCommand(moonliteCom, "GI");
//		USB_SendCommand(moonliteCom, "GP");
//		USB_SendCommand(moonliteCom, "GC");


	if (failureCnt > 0)
	{
		CONSOLE_DEBUG_W_NUM("Failures during open process", failureCnt);
	}
	return(isNiteCrawler);
}

//*****************************************************************************
//*	returns true if its a NiteCrawler
//*****************************************************************************
bool	MoonLite_CheckIfHighRes(TYPE_MOONLITECOM *moonliteCom)
{
int		tryCounter;
int		failureCnt;
bool	isHighRes;
char	readBuffer[48];
int		readCnt;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	set the baud rate to 9600
	//*	also sets timeout
	Serial_Set_Attribs(moonliteCom->fileDesc, B9600, 0);

	moonliteCom->model	=	kMoonLite_HighRes;

	tryCounter		=	0;
	failureCnt		=	0;
	isHighRes		=	false;
	strcpy(moonliteCom->deviceModelString, "");
	//*	sometimes we dont get this on the first try
	while ((strlen(moonliteCom->deviceModelString) == 0) && (tryCounter < 3))
	{
		//*	get the model
		USB_SendCommand(moonliteCom, "GV");
//		sleep(1);
		readCnt	=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');
//		CONSOLE_DEBUG_W_NUM("readCnt\t=",	readCnt);
		if (readCnt > 2)
		{
		//	CONSOLE_DEBUG_W_NUM("readCnt\t=",	readCnt);
		//	CONSOLE_DEBUG_W_STR("version is=",	readBuffer);
			readBuffer[readCnt -1]	=	0;	//*	get rid of the trailing "#"
			strcpy(moonliteCom->deviceModelString, "Highres");

			strcpy(moonliteCom->deviceVersion, readBuffer);
			strcpy(moonliteCom->deviceSerialNum, "");

			isHighRes			=	true;
			moonliteCom->model	=	kMoonLite_HighRes;

		//	CONSOLE_DEBUG_W_STR("moonliteCom->deviceModelString\t=",	moonliteCom->deviceModelString);

		}
		else
		{
			failureCnt++;
			CONSOLE_DEBUG("Failed to read HighRes model");
		}

		tryCounter++;
	}
	return(isHighRes);
}

//*****************************************************************************
//*	returns file descriptor
//*		< 0 if error
//*****************************************************************************
bool	MoonLite_OpenFocuserConnection(TYPE_MOONLITECOM *moonliteCom, bool checkForNiteCrawler)
{
//-int		failureCnt;
//-char	readBuffer[48];
//-int		readCnt;
//-int		tryCounter;
bool	openOK;
bool	isNiteCrawler;
bool	isHighRes;

//	CONSOLE_DEBUG_W_STR("port is", moonliteCom->usbPortPath);

	openOK					=	false;
	isNiteCrawler			=	false;
	isHighRes				=	false;
//-	failureCnt				=	0;
	moonliteCom->fileDesc	=	open(moonliteCom->usbPortPath, O_RDWR);	//* connect to port
	if (moonliteCom->fileDesc >= 0)
	{
		if (checkForNiteCrawler)
		{
			isNiteCrawler	=	MoonLite_CheckIfNiteCrawler(moonliteCom);
		}
		if (isNiteCrawler == false)
		{
			Serial_Set_Attribs(moonliteCom->fileDesc, B9600, 0);
		//	sleep(5);
			isHighRes	=	MoonLite_CheckIfHighRes(moonliteCom);
		}

		if (isNiteCrawler || isHighRes)
		{
			openOK	=	true;
		}
	}
	else
	{
		CONSOLE_DEBUG_W_STR("failed to open", moonliteCom->usbPortPath);
		openOK	=	false;
	}
	CONSOLE_DEBUG_W_STR("moonliteCom->deviceModelString\t=",	moonliteCom->deviceModelString);
	CONSOLE_DEBUG_W_STR("moonliteCom->deviceVersion    \t=",	moonliteCom->deviceVersion);
	CONSOLE_DEBUG_W_STR("moonliteCom->deviceSerialNum  \t=",	moonliteCom->deviceSerialNum);

	return(openOK);
}

//*****************************************************************************
bool	MoonLite_CloseFocuserConnection(TYPE_MOONLITECOM *moonliteCom)
{
int		returnCode;
bool	closeOK;

	closeOK	=	false;
	if (moonliteCom->fileDesc >= 0)
	{
		returnCode	=	close(moonliteCom->fileDesc);
		if (returnCode == 0)
		{
			closeOK	=	true;
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("failed to close", returnCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}

		moonliteCom->fileDesc	=	-1;
	}
	return(closeOK);
}


//*****************************************************************************
//*	reads the buffer to make sure there is no trailing "#"
//*****************************************************************************
bool	MoonLite_FlushReadBuffer(TYPE_MOONLITECOM *moonliteCom)
{
bool	successFlag	=	false;
#if 1
int		tcReturnCode;
//int		readCnt;
//char	databuffer[50];

//	CONSOLE_DEBUG(__FUNCTION__);
	if (moonliteCom != NULL)
	{
		tcReturnCode	=	tcflush(moonliteCom->fileDesc, TCIFLUSH);
		if (tcReturnCode == 0)
		{
			successFlag	=	true;
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("tcflush had an error, errno=", errno);
		}

//		//*	now read data anyway
//		readCnt	=	read(moonliteCom->fileDesc, databuffer, (sizeof(databuffer) - 2));
//		if (readCnt > 0)
//		{
//			databuffer[readCnt]	=	0;
//			CONSOLE_DEBUG_W_STR("We read:", databuffer);
//		}
	}
#else
char	readBuffer[48];

	if (moonliteCom != NULL)
	{
		ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');
		ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');
		successFlag	=	true;
	}
#endif // 1
	return(successFlag);
}

//*****************************************************************************
//*	returns true if valid
//*****************************************************************************
bool	MoonLite_SendCommand(	TYPE_MOONLITECOM	*moonliteCom,
								const char			*commandString,
								char				*returnString)
{
int		readCnt;
char	cmdBuffer[8];
char	readBuffer[48];
bool	validFlag;
int		poundSignCnt;
int		ii;

	validFlag	=	false;
	if (moonliteCom != NULL)
	{
		if (moonliteCom->fileDesc >= 0)
		{
//			CONSOLE_DEBUG_W_STR("commandString=", commandString);
			USB_SendCommand(moonliteCom, commandString);

			memset(readBuffer, 0, sizeof(readBuffer));
			readCnt	=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');
			if (readCnt > 0)
			{
				readBuffer[readCnt]	=	0;
//				CONSOLE_DEBUG_W_NUM("readCnt\t=", readCnt);
//				CONSOLE_DEBUG_W_STR("readBuffer\t=", readBuffer);
				//*	make sure there is a '#' at the end
				if (readBuffer[readCnt - 1] == '#')
				{
					//*	now double check to make sure there is only one "#"
					poundSignCnt	=	0;
					for (ii=0; ii<readCnt; ii++)
					{
						if (readBuffer[ii] == '#')
						{
							poundSignCnt++;
						}
					}
					if (poundSignCnt == 1)
					{
						//*	we have valid data
						validFlag		=	true;
						strcpy(returnString, readBuffer);
					}
					else
					{
						CONSOLE_DEBUG_W_STR("Invalid string=", readBuffer);
						CONSOLE_DEBUG_W_STR("cmdBuffer=", cmdBuffer);
						moonliteCom->invalidStringErrCnt++;
					}
				}
				else
				{
				//	CONSOLE_DEBUG_W_STR("Invalid string=", readBuffer);
				//	CONSOLE_DEBUG_W_STR("cmdBuffer=", cmdBuffer);
					moonliteCom->invalidStringErrCnt++;
				}
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("No data, readCnt=", readCnt);
			}
		}
		else
		{
			CONSOLE_DEBUG("moonliteCom->fileDesc is NULL=");
		}
	}
	return(validFlag);
}

//*****************************************************************************
static int	hextoi(const char *hexString)
{
int	intValue;
int	iii;

	intValue	=	0;
	iii			=	0;
	while (hexString[iii] >= 0x30)
	{
		intValue	=	intValue << 4;
		if (isdigit(hexString[iii]))
		{
			intValue	+=	hexString[iii] & 0x0f;
		}
		else if (hexString[iii] >= 'A')
		{
			intValue	+=	9;
			intValue	+=	hexString[iii] & 0x0f;

		}
		iii++;
	}
	return(intValue);
}

//*****************************************************************************
//*	returns true if valid
//*****************************************************************************
bool	MoonLite_GetPosition(	TYPE_MOONLITECOM	*moonliteCom,
								const int			axisNumber,
								int32_t				*valueToUpdate)
{
char	resultsBuffer[48];
bool	validFlag;
char	cmdBuffer[8];
int		ccc;

//	CONSOLE_DEBUG(__FUNCTION__);
	MoonLite_FlushReadBuffer(moonliteCom);

	if ((moonliteCom->model == kMoonLite_NiteCrawler) ||
		((moonliteCom->model == kMoonLite_HighRes) && (axisNumber == 1)))
	{
		//*	send GetPosition command
		ccc	=	0;
		if (moonliteCom->model == kMoonLite_NiteCrawler)
		{
			cmdBuffer[ccc++]	=	0x30 + axisNumber;
		}
		//*	send GP
		cmdBuffer[ccc++]	=	'G';
		cmdBuffer[ccc++]	=	'P';
		cmdBuffer[ccc++]	=	0;

//		CONSOLE_DEBUG_W_STR("cmdBuffer\t=", cmdBuffer);
		validFlag	=	MoonLite_SendCommand(moonliteCom, cmdBuffer, resultsBuffer);
		if (validFlag)
		{
//			CONSOLE_DEBUG_W_STR("resultsBuffer\t=", resultsBuffer);
			if (moonliteCom->model == kMoonLite_NiteCrawler)
			{
				*valueToUpdate	=	atoi(resultsBuffer);
			}
			else
			{
				*valueToUpdate	=	hextoi(resultsBuffer);
			}
		}
		else
		{
			gMoonLiteReadFailureCnt++;
			CONSOLE_DEBUG_W_NUM("Failed to read position", gMoonLiteReadFailureCnt);
		}
	}
	else
	{
		CONSOLE_DEBUG("Invalid request");
		validFlag	=	false;
	}
	return(validFlag);
}

//*****************************************************************************
//*
//*****************************************************************************
//	---HiRes commands
//	Command	Response	Description
//	GI		XX			“01” if the motor is moving, otherwise “00”
//*
//*	returns true if valid
//*****************************************************************************
bool	MoonLite_GetMovingState(	TYPE_MOONLITECOM	*moonliteCom,
									const int			axisNumber,
									bool				*isMoving)
{
char	resultsBuffer[48];
bool	validFlag;
char	cmdBuffer[8];
int		ccc;
int		returnValue;

//	CONSOLE_DEBUG(__FUNCTION__);

	if ((moonliteCom->model == kMoonLite_NiteCrawler) ||
		((moonliteCom->model == kMoonLite_HighRes) && (axisNumber == 1)))
	{
		//*	send GetPosition command
		ccc	=	0;
		if (moonliteCom->model == kMoonLite_NiteCrawler)
		{
			cmdBuffer[ccc++]	=	0x30 + axisNumber;
			cmdBuffer[ccc++]	=	'G';
			cmdBuffer[ccc++]	=	'M';
		}
		else
		{
			cmdBuffer[ccc++]	=	'G';
			cmdBuffer[ccc++]	=	'I';
		}
		//*	send GM or GI
		cmdBuffer[ccc++]	=	0;

		validFlag	=	MoonLite_SendCommand(moonliteCom, cmdBuffer, resultsBuffer);
		if (validFlag)
		{
//			CONSOLE_DEBUG_W_STR("resultsBuffer\t=", resultsBuffer);
			returnValue	=	atoi(resultsBuffer);

			if (returnValue == 0)
			{
				*isMoving	=	false;
			}
			else
			{
//				CONSOLE_DEBUG("isMoving");
				*isMoving	=	true;
			}
		}
		else
		{
			gMoonLiteReadFailureCnt++;
			CONSOLE_DEBUG_W_NUM("Failed to read position", gMoonLiteReadFailureCnt);
		}
	}
	else
	{
		CONSOLE_DEBUG("Invalid request");
		validFlag	=	false;
	}
	return(validFlag);
}

//*****************************************************************************
//	---NightCrawler commands
//	xSQ	Stops motor
//	xSM	Starts motor to move to the “NEW” focus position
//
//	xSP“dddddddd”	Set the current position, 32 bit value, signedDEC
//	xSN“dddddddd”	Set the New position count, 32 bit value, signedDEC
//
//	x is the axis number
//
//*****************************************************************************
//	---HiRes commands
//	Command	Response	Description
//	GP		XXXX		Get Current Motor 1 Position, Unsigned Hexadecimal
//	GN		XXXX		Get the New Motor 1 Position, Unsigned Hexadecimal
//	GT		XXXX		Get the Current Temperature, Signed HexadecimalGDXXGet the Motor 1 speed, valid options are “02, 04, 08, 10, 20”
//	GH		XX			“FF” if half step is set, otherwise “00”
//	GI		XX			“01” if the motor is moving, otherwise “00”
//	GB		XX			The current RED Led Backlight value, Unsigned Hexadecimal
//	GV		XX			Code for current firmware version
//
//	SPxxxx				Set the Current Motor 1 Position, Unsigned Hexadecimal
//	SNxxxx				Set the New Motor 1 Position, Unsigned Hexadecimal
//	SF					Set Motor 1 to Full Step
//	SH					Set Motor 1 to Half Step
//	SDxx				Set the Motor 1 speed, valid options are “02, 04, 08, 10, 20”
//	FG					Start a Motor 1 move, moves the motor to the New Position.
//	FQ					Halt Motor 1 move, position is retained, motor is stopped.
//
//*****************************************************************************
//*	returns true if success
//*****************************************************************************
bool	MoonLite_SetPosition(	TYPE_MOONLITECOM	*moonliteCom,
								const int			axisNumber,
								int32_t				newPosition)
{
char	cmdBuffer[32];
char	readBuffer[48];
int		readCnt;
bool	successFlag;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "----------------------");
//*	data sent to NiteCrawler
//*	< means data sent  > means data returned
//	<1GN# >00058200 <1SN 63200# <1SM#
	successFlag	=	false;
	if (moonliteCom != NULL)
	{
		if (moonliteCom->fileDesc >= 0)
		{
			MoonLite_FlushReadBuffer(moonliteCom);
			if (moonliteCom->model == kMoonLite_NiteCrawler)
			{
//				CONSOLE_DEBUG("Sending move command to NiteCrawler");
				//*	NiteCrawler
				if ((axisNumber >= 1) && (axisNumber <= 3))
				{
					sprintf(cmdBuffer, "%dSN %d", axisNumber, newPosition);
					USB_SendCommand(moonliteCom, cmdBuffer);
					usleep(1000);

					readCnt	=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');


					sprintf(cmdBuffer, "%dSM", axisNumber);
					USB_SendCommand(moonliteCom, cmdBuffer);

					usleep(1000);
					readCnt			=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');
					if (readCnt > 0)
					{
						successFlag	=	true;
					}
				}
			}
			else if ((moonliteCom->model == kMoonLite_HighRes) && (axisNumber == 1))
			{
				CONSOLE_DEBUG("Sending move command to HighRes");

				//*	moonlite hi-res stepper controller
				sprintf(cmdBuffer, "SN%04X", newPosition);
				USB_SendCommand(moonliteCom, cmdBuffer);
				usleep(1000);

				readCnt	=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');
				if (readCnt > 0)
				{
					successFlag	=	true;
				}
				CONSOLE_DEBUG_W_STR("readBuffer\t=", readBuffer);

				USB_SendCommand(moonliteCom, "FG");
				usleep(1000);

				readCnt	=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');
				if (readCnt > 0)
				{
					successFlag	=	true;
				}
				CONSOLE_DEBUG_W_STR("readBuffer\t=", readBuffer);
			}
		}
	}
	return(successFlag);
}

//*****************************************************************************
//*	returns true if success
bool	MoonLite_SetCurrentPosition(TYPE_MOONLITECOM	*moonliteCom,
									const int			axisNumber,
									int32_t				newPosition)
{
char	cmdBuffer[32];
char	readBuffer[48];
int		readCnt;
bool	successFlag;

//	CONSOLE_DEBUG(__FUNCTION__);

//	<1GN# >00058200 <1SN 63200# <1SM#
	successFlag	=	false;
	if (moonliteCom != NULL)
	{
		if (moonliteCom->fileDesc >= 0)
		{
			MoonLite_FlushReadBuffer(moonliteCom);
			if (moonliteCom->model == kMoonLite_NiteCrawler)
			{
				//*	NiteCrawler
				if ((axisNumber >= 1) && (axisNumber <= 3))
				{
					sprintf(cmdBuffer, "%dSP %d", axisNumber, newPosition);
					USB_SendCommand(moonliteCom, cmdBuffer);
					readCnt	=	ReadUntilChar(moonliteCom->fileDesc, readBuffer, 40, '#');

					if (readCnt > 0)
					{
						successFlag	=	true;
					}
					else
					{
					}
				}
			}
			else if ((moonliteCom->model == kMoonLite_HighRes) && (axisNumber == 1))
			{
				CONSOLE_DEBUG("Not implemented in HighRes");
			}
		}
	}
	return(successFlag);
}


//*****************************************************************************
//*	returns true if valid
//*****************************************************************************
bool	MoonLite_GetTemperature(	TYPE_MOONLITECOM	*moonliteCom,
									double				*returnTemp_degC)
{
char	resultsBuffer[48];
bool	validFlag;
int		rawIntValue;

//	CONSOLE_DEBUG(__FUNCTION__);
	MoonLite_FlushReadBuffer(moonliteCom);

	if (moonliteCom->model == kMoonLite_HighRes)
	{
		//*	for the hi-res model, we have to do a temp conversion first
		USB_SendCommand(moonliteCom, "C");	//*	send the command and dont wait for response
		//*	and wait 1 second
		usleep(50 * 1000);
	}
	validFlag	=	MoonLite_SendCommand(moonliteCom, "GT", resultsBuffer);
	if (validFlag)
	{
//		CONSOLE_DEBUG_W_STR("resultsBuffer  \t=", resultsBuffer);
		if (moonliteCom->model == kMoonLite_NiteCrawler)
		{
			rawIntValue			=	atof(resultsBuffer);
			*returnTemp_degC	=	rawIntValue / 10.0;
		}
		else
		{
			//*	this is a 16 bit signed number
			rawIntValue			=	hextoi(resultsBuffer);
			if (rawIntValue & 0x8000)	//*	check  the sign big
			{
				//*	extend the sign bit to a 32 bit integer
				rawIntValue	|=	0xffff0000;
			}
			*returnTemp_degC	=	rawIntValue / 2.0;
//			CONSOLE_DEBUG_W_STR("resultsBuffer        \t=", resultsBuffer);
//			CONSOLE_DEBUG_W_NUM("raw temperature value\t=", rawIntValue);
//			CONSOLE_DEBUG_W_DBL("Adjusted temp value  \t=", *returnTemp_degC);
		}
//		CONSOLE_DEBUG_W_NUM("rawIntValue    \t=", rawIntValue);
//		CONSOLE_DEBUG_W_DBL("returnTemp_degC\t=", *returnTemp_degC);
	}
	else
	{
		CONSOLE_DEBUG("Failed to read temperature");
	}
	return(validFlag);
}

//*****************************************************************************
//*	returns true if valid
bool	MoonLite_GetVoltage(	TYPE_MOONLITECOM	*moonliteCom,
								double				*returnVoltage)
{
char	resultsBuffer[48];
bool	validFlag;

	validFlag	=	false;

	MoonLite_FlushReadBuffer(moonliteCom);

	//*	only the nitecrawler has voltage
	if (moonliteCom->model == kMoonLite_NiteCrawler)
	{
//		CONSOLE_DEBUG(__FUNCTION__);

		validFlag	=	MoonLite_SendCommand(moonliteCom, "GV", resultsBuffer);
		if (validFlag)
		{
		//	CONSOLE_DEBUG_W_STR("resultsBuffer\t=", resultsBuffer);
			*returnVoltage	=	atof(resultsBuffer) / 10.0;
		}
		else
		{
			CONSOLE_DEBUG("Failed to read voltage");
		}
	}
	return(validFlag);
}



//*****************************************************************************
//*	returns true if valid
//*	The GS query the switch status for the limit and rotation home switches:
//*	b0=Rotation switch
//*	b1=Out limit switch
//*	b2= In limit switch
//*
//*	GA is the AUX channel switch status:
//*	b0= Out limit
//*	b1=In lmit
//*****************************************************************************
bool	MoonLite_GetSwiches(	TYPE_MOONLITECOM	*moonliteCom,
								unsigned char		*switchBits)
{
char	resultsBuffer[48];
bool	validFlag;

	if (moonliteCom->model == kMoonLite_NiteCrawler)
	{
		validFlag	=	MoonLite_SendCommand(moonliteCom, "GS", resultsBuffer);
		if (validFlag)
		{
			moonliteCom->switchBits	=	atoi(resultsBuffer);
			*switchBits				=	moonliteCom->switchBits;
	//		CONSOLE_DEBUG_W_HEX("*switchBits=", *switchBits);
		}
		else
		{
			CONSOLE_DEBUG("Failed to read switch bits");
		}
	}
	else
	{
		validFlag	=	false;
	}
	return(validFlag);
}

//*****************************************************************************
//*	Get color
//*	returns true if valid
//*	This only applies the NiteCrawler
//*	The default values are as follows:
//*	<C 20# >00
//*	<C 01# >ffff
//*	<C 02# >0000
//*	<C 03# >39e7
//*	<C 04# >f800
//*	<C 05# >f800
//*	<C 06# >8000
//*	<C 07# >fe00
//*	<C 08# >07ff
//*	<C 12# >39e7
//*	<C 14# >ffff
//*	<C 11# >000f
//*	<C 13# >f800
//******************************************************************************
bool	MoonLite_GetNC_Color(	TYPE_MOONLITECOM	*moonliteCom,
								uint16_t			whichColor,
								uint16_t			*color16bit)
{
char	resultsBuffer[48];
char	cmdBuffer[32];
bool	validFlag;

	if (moonliteCom->model == kMoonLite_NiteCrawler)
	{
		sprintf(cmdBuffer, "C%02d", whichColor);

		validFlag	=	MoonLite_SendCommand(moonliteCom, cmdBuffer, resultsBuffer);
		if (validFlag)
		{
			*color16bit		=	hextoi(resultsBuffer);
//			CONSOLE_DEBUG_W_HEX("*color16bit=", *color16bit);
		}
		else
		{
			CONSOLE_DEBUG("Failed to read switch bits");
		}
	}
	else
	{
		validFlag	=	false;
	}
	return(validFlag);
}



//*****************************************************************************
//*	returns true if valid
//*****************************************************************************
bool	MoonLite_GetAuxSwiches(	TYPE_MOONLITECOM	*moonliteCom,
								unsigned char		*auxSwitchBits)
{
char	resultsBuffer[48];
bool	validFlag;

	if (moonliteCom->model == kMoonLite_NiteCrawler)
	{
		validFlag	=	MoonLite_SendCommand(moonliteCom, "GA", resultsBuffer);
		if (validFlag)
		{
			moonliteCom->auxSwitchBits	=	atoi(resultsBuffer);
			*auxSwitchBits				=	moonliteCom->auxSwitchBits;
	//		CONSOLE_DEBUG_W_HEX("*auxSwitchBits=", *auxSwitchBits);
		}
		else
		{
			CONSOLE_DEBUG("Failed to read aux switch bits");
		}
	}
	else
	{
		validFlag	=	false;
	}

	return(validFlag);
}

//*****************************************************************************
//*	returns true if valid
//*****************************************************************************
bool	MoonLite_StopAxis(TYPE_MOONLITECOM	*moonliteCom, int axisNumber)
{
char	resultsBuffer[48];
bool	validFlag;
char	cmdBuffer[8];
int		ccc;

	if (moonliteCom->model == kMoonLite_NiteCrawler)
	{
		//<1SQ0# <2SQ0# <3SQ0# <GV# >120 <GT# >-20
		//*	send stop command
		ccc				=	0;
		cmdBuffer[ccc++]	=	0x30 + axisNumber;
		cmdBuffer[ccc++]	=	'S';
		cmdBuffer[ccc++]	=	'Q';
		cmdBuffer[ccc++]	=	'0';
	}
	else
	{
		ccc					=	0;
		cmdBuffer[ccc++]	=	'F';
		cmdBuffer[ccc++]	=	'Q';
	}
	cmdBuffer[ccc++]	=	0;
	validFlag	=	MoonLite_SendCommand(moonliteCom, cmdBuffer, resultsBuffer);


	return(validFlag);
}

//*****************************************************************************
//*	returns true if valid
//*****************************************************************************
bool	MoonLite_StopMotors(TYPE_MOONLITECOM	*moonliteCom)
{
char	resultsBuffer[48];
bool	validFlag;
char	cmdBuffer[8];
int		ccc;

	validFlag	=	false;

	if (moonliteCom->model == kMoonLite_NiteCrawler)
	{
		//<1SQ0# <2SQ0# <3SQ0# <GV# >120 <GT# >-20
		//*	send stop command
		ccc				=	0;
		cmdBuffer[ccc++]	=	'1';
		cmdBuffer[ccc++]	=	'S';
		cmdBuffer[ccc++]	=	'Q';
		cmdBuffer[ccc++]	=	'0';
		cmdBuffer[ccc++]	=	0;

		validFlag		=	MoonLite_SendCommand(moonliteCom, cmdBuffer, resultsBuffer);
		cmdBuffer[0]	=	'2';
		validFlag		=	MoonLite_SendCommand(moonliteCom, cmdBuffer, resultsBuffer);
		cmdBuffer[0]	=	'3';
		validFlag		=	MoonLite_SendCommand(moonliteCom, cmdBuffer, resultsBuffer);
	}
	else
	{
		MoonLite_StopAxis(moonliteCom, 1);
	}

	return(validFlag);
}


//**************************************************************************************
//*	returns # of bytes written
//**************************************************************************************
static int	USB_SendCommand(TYPE_MOONLITECOM *moonliteCom, const char *theCommand)
{
char			cmdBuffer[32];
int				sLen;
int				bytesWritten;

	bytesWritten	=	0;
	if (moonliteCom->fileDesc >= 0)
	{
//		CONSOLE_DEBUG_W_NUM("moonliteCom->model\t=", moonliteCom->model);
		if (moonliteCom->model != kMoonLite_NiteCrawler)
		{
			strcpy(cmdBuffer, ":");
			strcat(cmdBuffer, theCommand);
		}
		else
		{
			strcpy(cmdBuffer, theCommand);
		}
		strcat(cmdBuffer, "#");
		strcpy(gLastCmdSent, cmdBuffer);	//*	keep a copy of the last command for debugging
		sLen			=	strlen(cmdBuffer);

//		CONSOLE_DEBUG_W_STR("sending:", cmdBuffer);
		bytesWritten	=	write(moonliteCom->fileDesc, cmdBuffer, sLen);
		if (bytesWritten < 0)
		{
			CONSOLE_DEBUG_W_NUM("Error occurred on write, errno=", errno);
		}


		usleep(1000);
//		CONSOLE_DEBUG_W_NUM("bytesWritten=", bytesWritten);
	}
	else
	{
		CONSOLE_DEBUG("Invalid fileDesc");
	}
	return(bytesWritten);
}


//**************************************************************************
//*	returns # chars read
//**************************************************************************
static int	ReadUntilChar(const int fileDesc, char *readBuff, const int maxChars, const char terminator)
{
int		readCnt;
char	oneCharBuff[4];
char	theChar;
int		ccc;
int		noDataCnt;
bool	keepGoing;

	memset(readBuff, 0, maxChars);	//*	null out the response first
	ccc			=	0;
	noDataCnt	=	0;
	keepGoing	=	true;
	while (keepGoing && (ccc < maxChars) && (noDataCnt < 5))
	{
		readCnt	=	read(fileDesc, oneCharBuff, 1);
		if (readCnt > 0)
		{
			theChar			=	oneCharBuff[0];
			readBuff[ccc++]	=	theChar;
			readBuff[ccc]	=	0;		//*	always terminate the string
			if (theChar == terminator)
			{
				keepGoing	=	false;
			}
			noDataCnt	=	0;
		}
		else
		{
			noDataCnt++;
			//*	Nov 28,	2022	<MLS> increased time delay from 25 to 50 ns
			usleep(50);
		}
	}
	if (strncmp(readBuff, "NAK", 3) == 0)
	{
		CONSOLE_DEBUG_W_STR("NAK received, last cmd\t=", gLastCmdSent);
		//*	A nak was received, log it
	#ifdef _ENABLE_FOCUSER_
		LogEvent(	"focuser",
					"NAK received",
					NULL,
					kASCOM_Err_Success,
					gLastCmdSent);
	#endif
	}
	return(ccc);
}

#endif // defined(_ENABLE_FOCUSER_MOONLITE_) || defined(_ENABLE_ROTATOR_NITECRAWLER_)

