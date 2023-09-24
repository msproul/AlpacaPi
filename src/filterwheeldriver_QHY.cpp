//**************************************************************************
//*	Name:			filterwheeldriver_QHY.cpp
//*
//*	Author:			Mark Sproul (C) 2023
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
//*	Sep 19,	2023	<MLS> Purchased QHY 7 pos 2 inch filter wheel used from CloudyNights
//*	Sep 20,	2023	<MLS> Created filterwheeldriver_QHY.cpp
//*	Sep 20,	2023	<MLS> Added driver thread to QHY filter wheel
//*	Sep 20,	2023	<MLS> QHY filter wheel fully working
//*	Sep 20,	2023	<MLS> SUPPORTED: QHY filter wheel
//*****************************************************************************

#ifdef _ENABLE_FILTERWHEEL_QHY_

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<sys/ioctl.h> //ioctl() call defenitions
#include	<fcntl.h>
#include	<termios.h>
#include	<stdbool.h>

#include	"eventlogging.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"filterwheeldriver.h"
#include	"filterwheeldriver_QHY.h"
#include	"usbmanager.h"
#include	"serialport.h"


//**************************************************************************************
//*	this will get moved to the individual implentations later
void	CreateFilterWheelObjects_QHY(void)
{
bool	validUSBpath;
char	usbPath[48];
int		validCnt;

	CONSOLE_DEBUG(__FUNCTION__);
	USB_InitTable();
	USB_DumpTable();

	validUSBpath	=	true;
	validCnt		=	0;
	while (validUSBpath)
	{
		validUSBpath	=	USB_GetPathFromID("Silicon", usbPath);
		if (validUSBpath)
		{
			CONSOLE_DEBUG_W_STR("Found usb path:", usbPath);
			new FilterwheelQHY(usbPath);
			validCnt++;
		}
	}
	if (validCnt == 0)
	{
		CONSOLE_DEBUG("Failed to find USB devices");
	}
}

//**************************************************************************************
FilterwheelQHY::FilterwheelQHY(const char *argUsbPath)
	:FilterwheelDriver(0)
{
int		returnCode;

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cUSBportPath, argUsbPath);

	cFilterWheelDevNum	=	0;
	cNumberOfPositions	=	0;
	cForceReadPosition	=	true;
	cActualPosReadCout	=	0;

	cMoveNewPosition	=	false;
	cNewPosition		=	0;

	strcpy(cCommonProp.Name, "QHY Filterwheel");	//*	put something there in case of failure to open
	strcpy(cCommonProp.Description, "QHY filterwheel");

	cQHYusbPort_fileDesc	=	open(cUSBportPath, O_RDWR);	//* connect to port
	if (cQHYusbPort_fileDesc >= 0)
	{
		CONSOLE_DEBUG_W_STR(cUSBportPath, "Opened OK");
		usleep(50 * 1000);

		returnCode	=	Serial_Set_Attribs(cQHYusbPort_fileDesc, B9600, 0);
		if (returnCode != 0)
		{
			CONSOLE_DEBUG("Serial_Set_Attribs() failed");
		}
		Serial_Set_RTS(cQHYusbPort_fileDesc, true);

		StartDriverThread();
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open:", cUSBportPath);
		CONSOLE_ABORT(__FUNCTION__);
	}
}

//**************************************************************************************
// Destructor
//**************************************************************************************
FilterwheelQHY::~FilterwheelQHY(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

}

//**************************************************************************************
//*	this opens the connection
bool	FilterwheelQHY::AlpacaConnect(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	return(false);
}

//*****************************************************************************
int	FilterwheelQHY::Read_CurrentFWstate(void)
{
int	fwState;

	if (cFilterWheelProp.IsMoving)
	{
		fwState	=	kFilterWheelState_Moving;
	}
	else
	{
		fwState	=	kFilterWheelState_OK;
	}
	return(fwState);
}

//*****************************************************************************
//*	this returns a position starting with 0
//*	return -1 if unable to determine position
//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelQHY::Read_CurrentFilterPositon(int *rtnCurrentPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);

	cReadPosition	=	true;
	return(alpacaErrCode);
}

//*****************************************************************************
//*	this accepts a position starting with 0
//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelQHY::Set_CurrentFilterPositon(const int newPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (newPosition != cFilterWheelProp.Position)
	{
		cMoveNewPosition			=	true;
		cNewPosition				=	newPosition;
		cFilterWheelProp.IsMoving	=	true;
	}

	return(alpacaErrCode);
}

//**************************************************************************************
static void	SendQHYcmd(int fileDesc, const char *qhyCmd, char *returnDataBuffer, int expecteCharCnt)
{
char	readBuffer[256];
int		bytesRead;
int		loopCnt;
int		ccc;
int		iii;

	CONSOLE_DEBUG_W_STR("sending:", qhyCmd);

	Serial_Send_Data(fileDesc, qhyCmd, true);
	usleep(100 * 1000);
//	sleep(1);
	bytesRead			=	0;
	loopCnt				=	0;
	ccc					=	0;
	returnDataBuffer[0]	=	0;
	while ((loopCnt < 10) && (ccc < expecteCharCnt))
	{
		bytesRead	=	Serial_Read_Data(fileDesc, readBuffer, 255);
		if (bytesRead > 0)
		{
			CONSOLE_DEBUG_W_NUM("bytesRead\t=", bytesRead);
			readBuffer[bytesRead]	=	0;
			CONSOLE_DEBUG(readBuffer);
			for (iii=0; iii<bytesRead; iii++)
			{
				returnDataBuffer[ccc++]	=	readBuffer[iii];
				returnDataBuffer[ccc]	=	0;
			}
		}
		else
		{
			usleep(50 * 1000);
		}
		loopCnt++;
	}
//	CONSOLE_DEBUG(returnDataBuffer);
	usleep(250000);
}

//************************************************************************************************
static short	Hextoi(const char theHexChar)
{
short	theValue;

	theValue	=	(short)theHexChar & 0x0f;

	if (theHexChar > '9')
	{
		theValue	+=	9;
	}
	return(theValue);
}

//*****************************************************************************
void	FilterwheelQHY::RunThread_Startup(void)
{
int		loopCnt;
char	readBuffer[256];
int		bytesRead;
int		totalBytesRead;

	CONSOLE_DEBUG(__FUNCTION__);
	sleep(20);
	loopCnt			=	0;
	totalBytesRead	=	0;
	while (loopCnt < 10)
	{
		bytesRead	=	Serial_Read_Data(cQHYusbPort_fileDesc, readBuffer, 255);
		if (bytesRead > 0)
		{
			CONSOLE_DEBUG_W_NUM("bytesRead\t=", bytesRead);
			readBuffer[bytesRead]	=	0;
			CONSOLE_DEBUG(readBuffer);

			totalBytesRead	+=	bytesRead;
		}
		usleep(200 * 1000);
		loopCnt++;
	}
	CONSOLE_DEBUG("--------------------------------------------");

	SendQHYcmd(cQHYusbPort_fileDesc, "MXP", readBuffer, 1);
	if (strlen(readBuffer) > 0)
	{
		CONSOLE_DEBUG_W_STR("MXP",	readBuffer);
		cNumberOfPositions		=	atoi(readBuffer);
	}
	CONSOLE_DEBUG_W_NUM("cNumberOfPositions",	cNumberOfPositions);

	SendQHYcmd(cQHYusbPort_fileDesc, "VRS", readBuffer, 8);
	if (strlen(readBuffer) > 0)
	{
		CONSOLE_DEBUG_W_STR("VRS",	readBuffer);
		strcpy(cCommonProp.Description, "QHY filterwheel, FW-Ver:");
		strcat(cCommonProp.Description, readBuffer);
	}
	CONSOLE_DEBUG_W_STR("cCommonProp.Description",	cCommonProp.Description);

	SendQHYcmd(cQHYusbPort_fileDesc, "NOW", readBuffer, 1);
	CONSOLE_DEBUG_W_STR("NOW",	readBuffer);
	if (strlen(readBuffer) > 0)
	{
		cFilterWheelProp.Position	=	Hextoi(readBuffer[0]);
	}
}

//*****************************************************************************
void	FilterwheelQHY::RunThread_Loop(void)
{
char	cmdString[16];
char	readBuffer[256];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Filterwheel-QHY");

	//*	did we get the number of positions correctly
	if (cNumberOfPositions < 5)
	{
		SendQHYcmd(cQHYusbPort_fileDesc, "MXP", readBuffer, 1);
		if (strlen(readBuffer) > 0)
		{
			CONSOLE_DEBUG_W_STR("MXP",	readBuffer);
			cNumberOfPositions		=	atoi(readBuffer);
		}
		CONSOLE_DEBUG_W_NUM("cNumberOfPositions",	cNumberOfPositions);
	}

	if (cMoveNewPosition)
	{
		CONSOLE_DEBUG_W_NUM("Setting to new position:",	cNewPosition);
		cmdString[0]	=	cNewPosition + 0x30;
		cmdString[1]	=	0;
		CONSOLE_DEBUG_W_STR("cmdString \t=",	cmdString);
		SendQHYcmd(cQHYusbPort_fileDesc, cmdString, readBuffer, 1);
		CONSOLE_DEBUG_W_STR("readBuffer\t=",	readBuffer);

		cMoveNewPosition	=	false;
	}
	//*	if we are moving, get current position
	if (cFilterWheelProp.IsMoving)
	{
		SendQHYcmd(cQHYusbPort_fileDesc, "NOW", readBuffer, 1);
		CONSOLE_DEBUG_W_STR("NOW response\t=",	readBuffer);
		if (strlen(readBuffer) > 0)
		{
			cFilterWheelProp.Position	=	Hextoi(readBuffer[0]);
		}
		cFilterWheelProp.IsMoving	=	false;
	}

	if (cReadPosition)
	{
		SendQHYcmd(cQHYusbPort_fileDesc, "NOW", readBuffer, 1);
		CONSOLE_DEBUG_W_STR("NOW response\t=",	readBuffer);
		if (strlen(readBuffer) > 0)
		{
			cFilterWheelProp.Position	=	Hextoi(readBuffer[0]);
		}
		cReadPosition	=	false;
	}

	usleep(500 * 1000);
}


#endif // _ENABLE_FILTERWHEEL_QHY_
