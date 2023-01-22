//**************************************************************************
//*	Name:			calibrationdriver_Alnitak.cpp
//*
//*	Author:			Mark Sproul (C) 2022
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:	This driver does not implement any actual device,
//*					you must create a sub-class that does the actual control
//*
//*	References:		https://ascom-standards.org/api/
//*					https://ascom-standards.org/Help/Developer/html/N_ASCOM_DeviceInterface.htm
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Sep 28,	2022	<MLS> Created calibrationdriver_Alnitak.cpp
//*****************************************************************************


#if defined(_ENABLE_CALIBRATION_) && defined(_ENABLE_CALIBRATION_ALNITAK_)

#include	<errno.h>
#include	<stdlib.h>
#include	<string.h>
#include	<termios.h>
#include	<fcntl.h>



#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"serialport.h"

#include	"alpacadriver_helper.h"
#include	"calibrationdriver.h"
#include	"calibration_Alnitak.h"

//	Command		Send		Recive			Description
//	Ping		>P000CR		*Pii000CR		Used to find device
//	Open		>O000CR		*Oii000CR		Open cover (FF only)
//	Close		>C000CR		*Cii000CR		Close cover(FF only)
//	Light on	>L000CR		*Lii000CR		Turn on light
//	Light off	>D000CR		*Dii000CR		Turn off light
//	Brightness	>BxxxCR		*BiixxxCR		Set brightness (xxx = 000-255)
//	Brightness	>J000CR		*JiixxxCR		Get brightness from device
//	State		>S000CR		*SiiqrsCR		Get device status
//	Version		>V000CR		*ViivvvCR		Get firmware version

//*****************************************************************************
void	CreateCalibrationObjectsAlnitak(void)
{
	new CalibrationDriverAlnitak();
}

//**************************************************************************************
CalibrationDriverAlnitak::CalibrationDriverAlnitak(void)
	:CalibrationDriver()
{
	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name,		"CoverCalibration-Alnitak");
	strcpy(cCommonProp.Description,	"USB interface to Alnitak controler");
	strcpy(cUSBportPath,			"/dev/ttyUSB0");

	cCoverCalibrationProp.Brightness		=	0;
	cCoverCalibrationProp.CalibratorState	=	kCalibrator_Ready;
	cCoverCalibrationProp.CoverState		=	kCover_NotPresent;
	cCoverCalibrationProp.MaxBrightness		=	255;

	Init_Hardware();
}

//**************************************************************************************
CalibrationDriverAlnitak::~CalibrationDriverAlnitak( void )
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************
static int	ReadUntilChar(const int fileDesc, char *readBuff, const int maxChars, const char terminator)
{
int		readCnt;
char	oneCharBuff[4];
char	theChar;
int		ccc;
int		noDataCnt;
bool	keepGoing;

	readBuff[0]	=	0;	//*	null out the response first
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
		}
		else
		{
			noDataCnt++;
		}
	}
	return(ccc);
}


//*****************************************************************************
void	CalibrationDriverAlnitak::Init_Hardware(void)
{
int		returnCode;
char	cmdBuffer[32];
char	readBuffer[32];
int		sLen;
int		bytesWritten;
int		readCnt;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(cUSBportPath);

	cFileDesc	=	open(cUSBportPath, O_RDWR);	//* connect to port
	if (cFileDesc >= 0)
	{
		CONSOLE_DEBUG_W_STR(cUSBportPath, "Opened OK");
		sleep(2);
		returnCode	=	Serial_Set_Attribs(cFileDesc, B9600, 0);
		if (returnCode != 0)
		{
			CONSOLE_DEBUG("Serial_Set_Attribs() failed");
		}
		sleep(2);
		strcpy(cmdBuffer, ">V000\r");
		sLen			=	strlen(cmdBuffer);
		CONSOLE_DEBUG_W_STR("sending\t=", cmdBuffer);
		CONSOLE_DEBUG_W_NUM("sLen\t=", sLen);

		bytesWritten	=	write(cFileDesc, cmdBuffer, sLen);
		if (bytesWritten < 0)
		{
			CONSOLE_DEBUG_W_NUM("Error occurred on write, errno=", errno);
		}
		CONSOLE_DEBUG_W_NUM("bytesWritten\t=", bytesWritten);
		tcflush(cFileDesc, TCOFLUSH);
		sleep(2);
		readCnt	=	ReadUntilChar(cFileDesc, readBuffer, 20, 0x0d);
		CONSOLE_DEBUG_W_NUM("readCnt\t=", readCnt);
		CONSOLE_DEBUG(readBuffer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR(cUSBportPath, "FAILED TO OPEN!!!!");
		CONSOLE_ABORT(__FUNCTION__);

	}
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Calibrator_TurnOn(const int brightnessValue, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	kASCOM_Err_NotImplemented;
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");

	return(alpacaErrCode);

}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Calibrator_TurnOff(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);


	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");

	return(alpacaErrCode);
}



#endif	//	defined(_ENABLE_CALIBRATION_) && defined(_ENABLE_CALIBRATION_ALNITAK_)

