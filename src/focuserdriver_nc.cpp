//**************************************************************************
//*	Name:			focuserdriver_nc.cpp
//*
//*	Author:			Mark Sproul (C) 2019
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
//*	Re-distribution of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*	Limitations:	This driver was originally written for the MoonLite Nitecrawler focuser only
//*					It has since been updated to work with MoonLite HighRes controlers as well
//*	Usage notes:
//*
//*	References:
//*	https://ascom-standards.org/api/#/Dome%20Specific%20Methods/get_dome__device_number__athome
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Dec  4,	2019	<MLS> Started working on moonlite focuser
//*	Dec  5,	2019	<MLS> Looking into talking to /dev/ttyUSBx directly (FTDI chip)
//*	Dec  7,	2019	<MLS> Talking to NiteCrawler with /dev/ttyUSBx, no longer using libusb
//*	Dec  7,	2019	<MLS> Reading NiteCrawler focuser position values
//*	Dec 13,	2019	<MLS> Successfully sending set position commands to NiteCrawler
//*	Dec 16,	2019	<MLS> Added SetStepperPosition()
//*	Dec 19,	2019	<MLS> Added HaltStepper()
//*	Dec 19,	2019	<MLS> Halt working, command is "nSQ0"
//*	Jan  1,	2020	<MLS> Added OpenFocuserConnection() to separate functionality
//*	Jan  1,	2020	<MLS> Successfully tested 2 NiteCrawlers attached to the same R-Pi
//*	Feb 29,	2020	<MLS> Switching over to using moonlite_com.c interface
//*	Mar 17,	2020	<MLS> Fixed bug, /dev directory was not being closed
//*	Sep 18,	2022	<MLS> Changed RunStateMachine() to return 100000 (100 ms) instead of 500 (.5 ms)
//*****************************************************************************

#if defined(_ENABLE_FOCUSER_) && defined (_ENABLE_FOCUSER_MOONLITE_)

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

#if defined(__arm__)
	#include <wiringPi.h>
#endif

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"focuserdriver.h"
#include	"focuserdriver_nc.h"
#include	"eventlogging.h"
#include	"serialport.h"



#ifdef	_ENABLE_ROTATOR_
	#include	"rotatordriver_nc.h"
#endif // _ENABLE_ROTATOR_

#ifndef _USE_MOONLITE_COM_
	static int	CountMoonliteFocusers(void);
#endif // _USE_MOONLITE_COM_


//*****************************************************************************
typedef struct
{
	char	deviceString[16];
} TYPE_SERIALPORTS;

#define	kMaxMoonliteFocusers	5

TYPE_SERIALPORTS	gValidSerialPorts[kMaxMoonliteFocusers];
int					gValidPortCnt	=	0;

//*****************************************************************************
void	CreateFocuserNiteCrawlerObjects(void)
{
int					moonliteCnt;
int					ii;
TYPE_MOONLITECOM	myMoonliteDesc;

	CONSOLE_DEBUG(__FUNCTION__);
	moonliteCnt	=	MoonLite_CountFocusers();

	CONSOLE_DEBUG_W_NUM("moonliteCnt\t=", moonliteCnt);
	for (ii=0; ii<moonliteCnt; ii++)
	{
		MoonLite_InitCom(&myMoonliteDesc, ii);
		new FocuserNiteCrawler(ii, myMoonliteDesc.usbPortPath);
	}
}

//**************************************************************************************
FocuserNiteCrawler::FocuserNiteCrawler(const int argDevNum, const char *devicePath)
	:FocuserDriver(argDevNum)
{

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cDeviceManufacturer,	"Moonlite");

	cFocuserProp.Absolute		=	true;
	cFocuserProp.MaxIncrement	=	10000;
	cFocuserProp.MaxStep		=	87000;
	cFocuserProp.StepSize		=	0.2667;	//	Step size (microns) for the focuser.

	cLastTimeSecs_Temperature	=	0;
	cLastTimeMilSecs_Position	=	0;
	cInvalidStringErrCnt		=	0;

	CONSOLE_DEBUG_W_NUM("argDevNum", argDevNum);
	if (devicePath != NULL)
	{
		CONSOLE_DEBUG_W_STR("port is", devicePath);
		OpenFocuserConnection(devicePath);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("port is", gValidSerialPorts[argDevNum].deviceString);
		OpenFocuserConnection(gValidSerialPorts[argDevNum].deviceString);
	}
	switch (cMoonliteCom.model)
	{
		case kMoonLite_NiteCrawler:
			strcpy(cCommonProp.Name,		"NiteCrawler Focuser");
			strcpy(cCommonProp.Description,	"Moonlite NiteCrawler Focuser");
			cIsNiteCrawler				=	true;
			cFocuserSupportsRotation	=	true;
			cFocuserSupportsAux			=	true;
			cFocuserHasVoltage			=	true;
			cFocuserHasTemperature		=	true;
			break;

		case kMoonLite_HighRes:
			strcpy(cCommonProp.Name,		"Moonlite");
			strcpy(cCommonProp.Description,	"Moonlite HiRes Focuser");
			cFocuserHasTemperature		=	true;
			break;

		default:
			strcpy(cCommonProp.Name,		"unknown");
			break;

	}
	cUniqueID.part1				=	'MOON';					//*	4 byte manufacturer code
	cUniqueID.part2				=	kBuildNumber;			//*	software version number
	cUniqueID.part3				=	1;						//*	model number
	cUniqueID.part4				=	0;						//*	model number
	cUniqueID.part5				=	atoi(cDeviceSerialNum);	//*	serial number

}

//**************************************************************************************
// Destructor
//**************************************************************************************
FocuserNiteCrawler::~FocuserNiteCrawler(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	MoonLite_CloseFocuserConnection(&cMoonliteCom);
}

//*****************************************************************************
bool	FocuserNiteCrawler::OpenFocuserConnection(const char *usbPortPath)
{
bool	openOK;

	CONSOLE_DEBUG_W_STR("port is", usbPortPath);

	MoonLite_InitCom(&cMoonliteCom, -1);
	strcpy(cMoonliteCom.usbPortPath, usbPortPath);
	CONSOLE_DEBUG_W_STR("port is", cMoonliteCom.usbPortPath);
	openOK	=	MoonLite_OpenFocuserConnection(&cMoonliteCom, true);
	if (openOK)
	{
		LogEvent(	"focuser",
					__FUNCTION__,
					NULL,
					kASCOM_Err_Success,
					usbPortPath);


		strcat(cDeviceModel,		cMoonliteCom.deviceModelString);
		strcat(cDeviceVersion,		cMoonliteCom.deviceVersion);
		strcpy(cDeviceSerialNum,	cMoonliteCom.deviceSerialNum);
		cRotatorStepsPerRev		=	cMoonliteCom.stepsPerRev;

		if (strlen(cDeviceVersion) > 0)
		{
		char	modelVersionStr[64];
			if (cMoonliteCom.model == kMoonLite_NiteCrawler)
			{
				strcpy(modelVersionStr, "NiteCrawler-");
			}
			else if (cMoonliteCom.model == kMoonLite_HighRes)
			{
				strcpy(modelVersionStr, "HiRes-");
			}
			else
			{
				strcpy(modelVersionStr, "unknown?-");
			}
			strcat(modelVersionStr, cDeviceVersion);
			AddLibraryVersion("focuser", "MoonLite", modelVersionStr);

			LogEvent(	"focuser",
						"Firmware version",
						NULL,
						kASCOM_Err_Success,
						modelVersionStr);
		}


		CONSOLE_DEBUG_W_STR("cDeviceModel",		cDeviceModel);
		CONSOLE_DEBUG_W_STR("cDeviceVersion",	cDeviceVersion);
		CONSOLE_DEBUG_W_STR("cDeviceSerialNum",	cDeviceSerialNum);
		cFileDesc	=	cMoonliteCom.fileDesc;

	#ifdef	_ENABLE_ROTATOR_
		if (cMoonliteCom.model == kMoonLite_NiteCrawler)
		{
			new RotatorDriver_NiteCrawler(cDeviceNum, this);
		}
	#endif // _ENABLE_ROTATOR_

	}
	else
	{
		CONSOLE_DEBUG_W_STR("failed to open", usbPortPath);
		openOK	=	false;
	}
	return(openOK);
}


//*****************************************************************************
//*	returns true if valid
bool	FocuserNiteCrawler::GetPosition(const int axisNumber, int32_t *valueToUpdate)
{
bool	validFlag	=	false;

	validFlag	=	MoonLite_GetPosition(	&cMoonliteCom,
											axisNumber,
											valueToUpdate);


	return(validFlag);
}


//*****************************************************************************
int32_t	FocuserNiteCrawler::RunStateMachine(void)
{
bool		validFlag;
uint32_t	currentMillis;
uint32_t	currentSeconds;

//	CONSOLE_DEBUG(__FUNCTION__);

#if 0
	//*	make sure the read buffer is empty
	readCnt	=	1;
	while (readCnt > 0)
	{
		readCnt	=	ReadUntilChar(cFileDesc, readBuffer, 40, '#');
		if (readCnt > 0)
		{
			CONSOLE_DEBUG_W_STR("NON-EMPTY BUFFER=", readBuffer);
		}
	}
#endif

	currentMillis	=	millis();
	currentSeconds	=	currentMillis / 1000;

	//*	check the temperature every 10 seconds
	if ((currentSeconds - cLastTimeSecs_Temperature) > 10)
	{
		//===============================================================
		if (cFocuserHasTemperature)
		{
			validFlag	=	MoonLite_GetTemperature(&cMoonliteCom,	&cFocuserTemp);
			if (validFlag == false)
			{
				CONSOLE_DEBUG("MoonLite_GetTemperature returned false");
			}
		}

		//===============================================================
		if (cFocuserHasVoltage)
		{
			validFlag	=	MoonLite_GetVoltage(&cMoonliteCom,		&cFocuserVoltage);
			if (validFlag == false)
			{
				CONSOLE_DEBUG("MoonLite_GetVoltage returned false");
			}
		}
		cLastTimeSecs_Temperature	=	currentSeconds;
	}

	//*	get the position information every 200 milliseconds (5 times a second)
	if ((currentMillis - cLastTimeMilSecs_Position) > 200)
	{
		GetPosition(1, &cFocuserProp.Position);
		if (cFocuserSupportsRotation)
		{
			GetPosition(2, &cRotatorPosition);
		}
		if (cFocuserSupportsAux)
		{
			GetPosition(3, &cAuxPosition);
		}

//		CONSOLE_DEBUG_W_INT32("pos1=", cFocuserProp.Position);
//		CONSOLE_DEBUG_W_INT32("pos2=", cRotatorPosition);
//		CONSOLE_DEBUG_W_INT32("pos3=", cAuxPosition);

		//*	check to see if the focuser is moving...
		if (cFocuserProp.Position != cPrevFocuserPosition)
		{
			cFocuserProp.IsMoving	=	true;
			CONSOLE_DEBUG_W_NUM("pos1=", cFocuserProp.Position);
		}
		else
		{
			cFocuserProp.IsMoving	=	false;
		}
		cPrevFocuserPosition	=	cFocuserProp.Position;


		//*	check to see if the rotator moving...
		if (cRotatorPosition != cPrevRotatorPosition)
		{
			cRotatorIsMoving	=	true;
			CONSOLE_DEBUG_W_NUM("pos2=", cRotatorPosition);
		}
		else
		{
			cRotatorIsMoving	=	false;
		}
		cPrevRotatorPosition		=	cRotatorPosition;


		//*	check to see if the Aux moving...
		if (cAuxPosition != cPrevAuxPosition)
		{
			cAuxIsMoving	=	true;
			CONSOLE_DEBUG_W_NUM("pos3=", cAuxPosition);
		}
		else
		{
			cAuxIsMoving	=	false;
		}
		cPrevAuxPosition		=	cAuxPosition;

		//*	if anything is moving, get the switch bits
		//*	The GS query the switch status for the limit and rotation home switches:
		//*	b0=Rotation switch
		//*	b1=Out limit switch
		//*	b2= In limit switch
		//*
		//*	GA is the AUX channel switch status:
		//*	b0= Out limit
		//*	b1=In lmit
		if (cFocuserProp.IsMoving || cRotatorIsMoving || cAuxIsMoving)
		{
		unsigned char	switchBits;

			validFlag	=	MoonLite_GetSwiches(&cMoonliteCom,	&switchBits);
//			CONSOLE_DEBUG_W_HEX("switchBits\t=", switchBits);
			cSwitchROT	=	((switchBits & 0x01) ? true : false);
			cSwitchOUT	=	((switchBits & 0x02) ? true : false);
			cSwitchIN	=	((switchBits & 0x04) ? true : false);

			validFlag	=	MoonLite_GetAuxSwiches(&cMoonliteCom,	&switchBits);
			cSwitchAUX1	=	((switchBits & 0x01) ? true : false);
			cSwitchAUX2	=	((switchBits & 0x02) ? true : false);
		}

		cLastTimeMilSecs_Position	=	currentMillis;
	}

	return(100 * 1000);
}


//**************************************************************************************
void	FocuserNiteCrawler::SendCommand(const char *theCommand)
{
char			cmdBuffer[32];
int				sLen;
int				bytesWritten;

	if (cFileDesc >= 0)
	{
		strcpy(cmdBuffer, theCommand);
		strcat(cmdBuffer, "#");
		strcpy(cLastCmdSent, cmdBuffer);	//*	keep a copy of the last command for debugging
		sLen			=	strlen(cmdBuffer);
		bytesWritten	=	write(cFileDesc, cmdBuffer, sLen);
		if (bytesWritten < 0)
		{
			CONSOLE_DEBUG_W_NUM("Error occurred on write, errno=", errno);
		}
		tcflush(cFileDesc, TCOFLUSH);



		usleep(5000);
//		CONSOLE_DEBUG_W_NUM("bytesWritten=", bytesWritten);
	}
}

//**************************************************************************
int	FocuserNiteCrawler::ReadUntilChar(const int fd, char *readBuff, const int maxChars, const char terminator)
{
int		readCnt;
char	oneCharBuff[4];
char	theChar;
int		ccc;
int		noDataCnt;
bool	keepGoing;

	//*	null out the response first
	for (ccc=0; ccc<maxChars; ccc++)
	{
		readBuff[ccc]	=	0;
	}
	ccc			=	0;
	noDataCnt	=	0;
	keepGoing	=	true;
	while (keepGoing && (ccc < maxChars) && (noDataCnt < 2))
	{
		readCnt	=	read(fd, oneCharBuff, 1);
		if (readCnt > 0)
		{
			theChar			=	oneCharBuff[0];
			readBuff[ccc++]	=	theChar;
			readBuff[ccc]	=	0;		//*	always terminate the string
			if (theChar == terminator)
			{
				keepGoing	=	false;
			}
		}
		else
		{
			noDataCnt++;
		}
	}
	if (strncmp(readBuff, "NAK", 3) == 0)
	{
		CONSOLE_DEBUG_W_STR("NAK received, last cmd\t=", cLastCmdSent)
		//*	A nak was received, log it
		LogEvent(	"focuser",
					"NAK received",
					NULL,
					kASCOM_Err_Success,
					cLastCmdSent);
	}
	return(ccc);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	FocuserNiteCrawler::SetStepperPosition(const int axisNumber, const int32_t newPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				validFlag;
//	<1GN# >00058200 <1SN 63200# <1SM#

	CONSOLE_DEBUG(__FUNCTION__);
	if ((axisNumber >= 1) && (axisNumber <= 3))
	{
		validFlag	=	MoonLite_SetPosition(&cMoonliteCom,	axisNumber, newPosition);
		if (validFlag)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Stop the motor
//*****************************************************************************
TYPE_ASCOM_STATUS		FocuserNiteCrawler::HaltStepper(const int axisNumber)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				validFlag;

	CONSOLE_DEBUG(__FUNCTION__);

	if ((axisNumber >= 1) && (axisNumber <= 3))
	{
		validFlag	=	MoonLite_StopAxis(&cMoonliteCom,	axisNumber);
		if (validFlag)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			CONSOLE_DEBUG("MoonLite_StopAxis returned false");
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS		FocuserNiteCrawler::SetFocuserPosition(const int32_t newPosition, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	<1GN# >00058200 <1SN 63200# <1SM#

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	SetStepperPosition(1, newPosition);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS		FocuserNiteCrawler::HaltFocuser(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	HaltStepper(1);

	return(alpacaErrCode);
}

//*****************************************************************************
int	DevicePathSort(const void *e1, const void* e2)
{
int					retValue;
TYPE_SERIALPORTS	*entry1;
TYPE_SERIALPORTS	*entry2;

	entry1		=	(TYPE_SERIALPORTS *)e1;
	entry2		=	(TYPE_SERIALPORTS *)e2;
	retValue	=	strcmp(entry1->deviceString, entry2->deviceString);
	return(retValue);
}

#ifndef _USE_MOONLITE_COM_
#error "What are we doing here -- _USE_MOONLITE_COM_"
//**************************************************************************************
//*	open the /dev devices and see if there are moonlite focusers
static int	CountMoonliteFocusers(void)
{
int				moonliteCnt;
DIR				*directory;
struct dirent	*dir;
bool			keepGoing;
int				errorCode;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "using /dev/ttuUSB...");
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
			//	printf("%s\r\n", dir->d_name);
				if (strncmp(dir->d_name, "ttyUSB", 6) == 0)
				{
					printf("%s\r\n", dir->d_name);
					moonliteCnt++;
					LogEvent(	"Focuser",
								"Nitecrawler found",
								NULL,
								kASCOM_Err_Success,
								dir->d_name);

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
		LogEvent(	"focuser",
					"Failure",
					NULL,
					kASCOM_Err_Success,
					"Failed to open /dev");
	}

	return(moonliteCnt);
}

#endif	//	_USE_MOONLITE_COM_

#endif	//	_ENABLE_FOCUSER_


