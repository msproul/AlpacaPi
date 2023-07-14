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
//*	Jun  2,	2023	<MLS> Received Alnitak FlipFlat from CloudyNights
//*	Jun  2,	2023	<MLS> Added SendFlipFlatCmd()
//*	Jun  7,	2023	<MLS> Added Cover_Open(), Cover_Close(), Cover_Halt()
//*	Jun  8,	2023	<MLS> Added thread to deal with Alnitak communications
//*	Jun  8,	2023	<MLS> Added series of routines to talk directly to the hardware
//*	Jun  8,	2023	<MLS> Added RunThread()
//*	Jun  8,	2023	<MLS> All comm done from thread, main driver sets flags
//*	Jun  8,	2023	<MLS> Everything seems to be working properly
//*	Jun  8,	2023	<MLS> conform does like the delay of status updates
//*	Jun  9,	2023	<MLS> CONFORM-covercalibrarion-Alnitak -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	Jun  9,	2023	<MLS> CONFORM-1 issue, halt not supported
//*****************************************************************************
//*	Vendor documenation
//*		https://optecinc.com/astronomy/catalog/alnitak/flipflat.htm
//*
//*	Issues with motor slipage and timage out
//*		https://imt.gingergeeks.co.uk/2020/12/12/optec-flip-flat-service-vendor-review/
//*
//*****************************************************************************


//--------------------------------------------------------------------------------
//	Flip-Flat and Flat-Man Generic Commands
//	8/15/11
//	Phil Gaudet
//	Rev 3
//	The following is a list of the generic commands that can be sent to the Alinitak
//	Astrosystems Flip-Flat and Flat-Man from a terminal emulator or scripting language.
//		Serial Data Rate: 9600 Baud
//			Parity: None
//			Data Bits: 8
//			Stop Bits: 1
//			RTS: negated
//		Command		Send		Recive		Description
//		Ping		>P000CR		*Pii000CR	Used to find device
//		Open		>O000CR		*Oii000CR	Open cover (FF only)
//		Close		>C000CR		*Cii000CR	Close cover(FF only)
//		Light on	>L000CR		*Lii000CR	Turn on light
//		Light off	>D000CR		*Dii000CR	Turn off light
//		Brightness	>BxxxCR		*BiixxxCR	Set brightness (xxx = 000-255)
//		Brightness	>J000CR		*JiixxxCR	Get brightness from device
//		State		>S000CR		*SiiqrsCR	Get device status
//		Version		>V000CR		*ViivvvCR	Get firmware version
//	Where:
//		> is the leading character sent to the device
//		CR is carriage return
//		xxx is a three digit integer ranging from 0-255
//		* is the leading character returned from the device
//		Alnitak Astrosystems
//
//	ii is a two digit product id, 99 for Flip-Flat, 19 for Flat-Man, 10 for XL, 15 Flat-Man_L.
//	qrs is device status where:
//	s = 0 cover not open/closed
//	s = 1 cover closed
//	s = 2 cover open
//	s = 3 timed out (open/closed not detected)
//	r = 0 light off
//	r = 1 light on
//	q = 0 motor stopped
//	q = 1 motor running
//	vvv is firmware version
//	Alnitak Astrosystems
//--------------------------------------------------------------------------------

#if defined(_ENABLE_CALIBRATION_) && defined(_ENABLE_CALIBRATION_ALNITAK_)

#include	<errno.h>
#include	<stdlib.h>
#include	<string.h>
#include	<termios.h>
#include	<fcntl.h>
#include	<pthread.h>



#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"serialport.h"
#include	"usbmanager.h"
#include	"eventlogging.h"

#include	"helper_functions.h"
#include	"alpacadriver_helper.h"
#include	"calibrationdriver.h"
#include	"calibration_Alnitak.h"

//*****************************************************************************
void	CreateCalibrationObjects_Alnitak(void)
{
bool	validUSBpath;
char	usbPath[64];
bool	keepGoing;

	USB_InitTable();
	keepGoing	=	true;
	while (keepGoing)
	{
		validUSBpath	=	USB_GetPathFromID("Optec", usbPath);
		if (validUSBpath)
		{
			CONSOLE_DEBUG_W_STR("usbPath\t=", usbPath);
			new CalibrationDriverAlnitak(usbPath);
			keepGoing	=	false;
		}
	}
}

//**************************************************************************************
static void	*AlnitakCommThread(void *arg)
{
CalibrationDriverAlnitak	*objectPtr;

	objectPtr	=	(CalibrationDriverAlnitak *)arg;
	objectPtr->RunThread();
	return(NULL);
}

//**************************************************************************************
CalibrationDriverAlnitak::CalibrationDriverAlnitak(const char *argUSBpath)
	:CalibrationDriver()
{
int			threadErr;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cUSBportPath,			"/dev/ttyUSB0");	//*	set the default
	//*	this path is optional
	if (argUSBpath != NULL)
	{
		if (strlen(argUSBpath) > 0)
		{
			strcpy(cUSBportPath,	argUSBpath);
		}
	}
	LogEvent(	"covercalibration",
				__FUNCTION__,
				NULL,
				kASCOM_Err_Success,
				cUSBportPath);

	strcpy(cCommonProp.Name,		"CoverCalibration-Alnitak");
	strcpy(cCommonProp.Description,	"USB interface to Alnitak controller");

	cCoverCalibrationProp.Brightness		=	0;
	cCoverCalibrationProp.CalibratorState	=	kCalibrator_Ready;
	cCoverCalibrationProp.CoverState		=	kCover_Unknown;
	cCoverCalibrationProp.MaxBrightness		=	255;
	cOpenCover								=	false;
	cCloseCover								=	false;
	cTurnOn									=	false;
	cTurnOff								=	false;
	cCalibrationIsOn						=	false;
	cForceUpdate							=	false;
	cLastUpdate_ms							=	0;
	cLastUpdate_milliSecs					=	0;

	cThreadKeepRunning	=	true;
	threadErr			=	pthread_create(&cThreadID, NULL, &AlnitakCommThread, this);
	if (threadErr != 0)
	{
		CONSOLE_DEBUG("Error created thread");
	}
}

//**************************************************************************************
CalibrationDriverAlnitak::~CalibrationDriverAlnitak(void)
{
	cThreadKeepRunning	=	false;
	CONSOLE_DEBUG(__FUNCTION__);
}

//**************************************************************************************
void	CalibrationDriverAlnitak::RunThread(void)
{
char				alpacaErrMsg[128];
TYPE_ASCOM_STATUS	alpacaErrCode;
bool				openOK;
uint32_t			currentTime_ms;
uint32_t			deltaTime_ms;

	CONSOLE_DEBUG(__FUNCTION__);
	openOK	=	Init_Hardware();
	if (openOK)
	{
		cCommonProp.Connected	=	true;
	}
	//*	get the status on startup
	Alnitak_Cover_GetStatus(alpacaErrMsg);
	Alnitak_GetBrightness(alpacaErrMsg);

	//*	loop forever
	while (cThreadKeepRunning)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
		//*	close cover takes priority over open
		if (cCloseCover)
		{
			alpacaErrCode						=	Alnitak_Cover_Close(alpacaErrMsg);
			cCoverCalibrationProp.CoverState	=	kCover_Moving;
			cCloseCover							=	false;
			usleep(350 * 1000);
		}
		else if (cOpenCover)
		{
			alpacaErrCode						=	Alnitak_Cover_Open(alpacaErrMsg);
			cCoverCalibrationProp.CoverState	=	kCover_Moving;
			cOpenCover							=	false;
			usleep(350 * 1000);
		}
		if (alpacaErrCode != kASCOM_Err_Success)
		{
			CONSOLE_DEBUG_W_NUM("open/close failure; Error=", alpacaErrCode);
		}
		//-----------------------------------
		if (cTurnOff)
		{
			alpacaErrCode	=	Alnitak_TurnOff(alpacaErrMsg);
			cTurnOff		=	false;
		}
		else if (cTurnOn)
		{
			alpacaErrCode	=	Alnitak_TurnOn(cNewBrightnessValue, alpacaErrMsg);
			cTurnOn			=	false;
		}
		currentTime_ms	=	millis();
		deltaTime_ms	=	currentTime_ms - cLastUpdate_ms;
		if ((deltaTime_ms > 2000) || cForceUpdate)
		{
//			CONSOLE_DEBUG("Updating..........");
			Alnitak_Cover_GetStatus(alpacaErrMsg);
			Alnitak_GetBrightness(alpacaErrMsg);
			cLastUpdate_ms	=	millis();
			cForceUpdate	=	false;
		}
		//*	if we are moving, delay for a short time
		if (cCoverCalibrationProp.CoverState == kCover_Moving)
		{
			usleep(5 * 1000);
			cForceUpdate	=	true;
		}
		else
		{
			usleep(250 * 1000);
		}
	}
	CONSOLE_DEBUG("Thread Exit");
}

//*****************************************************************************
//*	this gets updated in the background thread, just return success
//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::GetBrightness(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

	alpacaErrCode			=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Calibrator_TurnOn(const int brightnessValue, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, brightnessValue);
	if (cCommonProp.Connected)
	{
		cTurnOn								=	true;
		cNewBrightnessValue					=	brightnessValue;
		cCoverCalibrationProp.Brightness	=	brightnessValue;
		alpacaErrCode						=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode		=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Calibrator_TurnOff(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cCommonProp.Connected)
	{
		cTurnOff							=	true;
		alpacaErrCode						=	kASCOM_Err_Success;
		cCoverCalibrationProp.Brightness	=	0;
		cNewBrightnessValue					=	0;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Cover_Open(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cCommonProp.Connected)
	{
		cOpenCover							=	true;
		alpacaErrCode						=	kASCOM_Err_Success;
		cCoverCalibrationProp.CoverState	=	kCover_Moving;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Port not open");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Cover_Close(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cCommonProp.Connected)
	{
		cCloseCover							=	true;
		alpacaErrCode						=	kASCOM_Err_Success;
		cCoverCalibrationProp.CoverState	=	kCover_Moving;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Port not open");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Cover_Halt(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	CONSOLE_DEBUG(alpacaErrMsg);
	return(alpacaErrCode);
}


//*****************************************************************************
bool	CalibrationDriverAlnitak::Init_Hardware(void)
{
bool	openOK;
int		returnCode;
char	readBuffer[32];
int		readCnt;
char	modelString[8];
//char	versionString[8];

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(cUSBportPath);
	openOK		=	false;
	cFileDesc	=	open(cUSBportPath, O_RDWR);	//* connect to port
	if (cFileDesc >= 0)
	{
		openOK		=	true;
		CONSOLE_DEBUG_W_STR(cUSBportPath, "Opened OK");
		usleep(50 * 1000);
		returnCode	=	Serial_Set_Attribs(cFileDesc, B9600, 0);
		if (returnCode != 0)
		{
			CONSOLE_DEBUG("Serial_Set_Attribs() failed");
		}
		Serial_Set_RTS(cFileDesc, false);
		usleep(2000 * 1000);
		readCnt	=	SendCmd_GetResponse('V', 0, readBuffer);
		if (readCnt > 0)
		{
			if (strncmp(readBuffer, "*V", 2) == 0)
			{
				modelString[0]	=	readBuffer[2];
				modelString[1]	=	readBuffer[3];
				modelString[2]	=	0;
				//	ii is a two digit product id, 99 for Flip-Flat, 19 for Flat-Man, 10 for XL, 15 Flat-Man_L.
				if (strcmp(modelString, "99") == 0)
				{
					strcat(cCommonProp.Name,	"-Flip-Flat");
				}
				else if (strcmp(modelString, "19") == 0)
				{
					strcat(cCommonProp.Name,	"-Flip-Man");
				}
				else if (strcmp(modelString, "10") == 0)
				{
					strcat(cCommonProp.Name,	"-XL");
				}
				else if (strcmp(modelString, "15") == 0)
				{
					strcat(cCommonProp.Name,	"-Flat-Man_L");
				}
			}
		}

//		CONSOLE_DEBUG_W_NUM("readCnt\t=", readCnt);
		CONSOLE_DEBUG(readBuffer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR(cUSBportPath, "FAILED TO OPEN!!!!");
		CONSOLE_ABORT(__FUNCTION__);
	}
	return(openOK);
}

#if 0
//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Cover_GetStatus(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	alpacaErrCode	=	kASCOM_Err_Success;

	return(alpacaErrCode);
}
#endif

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Alnitak_GetBrightness(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
char				readBuffer[32];
char				brightnessValStr[32];
int					readCnt;

//	CONSOLE_DEBUG(__FUNCTION__);
//	Brightness	>J000CR		*JiixxxCR		Get brightness from device

	readCnt	=	SendCmd_GetResponse('J', 0, readBuffer);
	if (readCnt > 0)
	{
//		CONSOLE_DEBUG(readBuffer);
		alpacaErrCode	=	kASCOM_Err_Success;
		if (strncmp(readBuffer, "*J", 2) == 0)
		{
			brightnessValStr[0]	=	readBuffer[4];
			brightnessValStr[1]	=	readBuffer[5];
			brightnessValStr[2]	=	readBuffer[6];
			brightnessValStr[3]	=	0;
			cCoverCalibrationProp.Brightness	=	atoi(brightnessValStr);
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;

			CONSOLE_DEBUG_W_STR("invalid response\t=", readBuffer);
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "J command failed");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;

		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "J command failed");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
int	CalibrationDriverAlnitak::SendFlipFlatCmd(const char cmdChr, int commandArgValue)
{
char	cmdBuffer[32];
int		bytesWritten;

//	CONSOLE_DEBUG(__FUNCTION__);

	sprintf(cmdBuffer, ">%c%03d", cmdChr, commandArgValue);
//	CONSOLE_DEBUG_W_STR("sending\t\t=", cmdBuffer);
	strcat(cmdBuffer, "\r");
	bytesWritten	=	Serial_Send_Data(cFileDesc, cmdBuffer, true);

//	CONSOLE_DEBUG_W_NUM("bytesWritten\t=", bytesWritten);
	return(bytesWritten);
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
	while (keepGoing && (ccc < maxChars) && (noDataCnt < 3))
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

//#define	kCharDeltaTime	(1000000/960)
#define	kXmitCharCnt	6
#define	kRcvCharCnt		8

//*****************************************************************************
//*	returns bytes received
//*****************************************************************************
int		CalibrationDriverAlnitak::SendCmd_GetResponse(const char cmdChr, int commandArgValue, char *returnedString)
{
int			readCnt;
useconds_t	sleepTime_usecs;

	memset(returnedString, 0, 10);
	SendFlipFlatCmd(cmdChr, commandArgValue);
	//*	calculate how long it takes to send and receive the data at 9600 baud
	sleepTime_usecs	=	(kXmitCharCnt + kRcvCharCnt)  * (1000000 / 9600);
	sleepTime_usecs	+=	1000;
	//*	now wait that long for the chars to come into the buffer
	usleep(sleepTime_usecs);

	readCnt	=	ReadUntilChar(cFileDesc, returnedString, 20, 0x0d);
	if (readCnt > 0)
	{
		returnedString[7]	=	0;
//		CONSOLE_DEBUG(returnedString);
	}
	else
	{
		CONSOLE_DEBUG("FlipFlat Command failed");
	}
	return(readCnt);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Alnitak_Cover_Open(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
char				readBuffer[32];
int					readCnt;

//	CONSOLE_DEBUG(__FUNCTION__);
	//------------------------------------------------------------------------------------------
	//*	on the Alnitak flip flat, the calibration light goes off automatically in the open state.
	//*	so that it is easier on the user, set the brightness level to 0 and the light off
	//------------------------------------------------------------------------------------------
	if (cCoverCalibrationProp.Brightness > 0)
	{
		Alnitak_TurnOff(alpacaErrMsg);
	}

	cOpenCover	=	true;

	readCnt	=	SendCmd_GetResponse('O', 0, readBuffer);
	if (readCnt > 0)
	{
		cCoverCalibrationProp.CoverState	=	kCover_Moving;
		alpacaErrCode						=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;

		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "O command failed");
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Alnitak_Cover_Close(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
char				readBuffer[32];
int					readCnt;

//	CONSOLE_DEBUG(__FUNCTION__);

	readCnt	=	SendCmd_GetResponse('C', 0, readBuffer);
	if (readCnt > 0)
	{
		CONSOLE_DEBUG(readBuffer);
		cCoverCalibrationProp.CoverState	=	kCover_Moving;
		alpacaErrCode						=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;

		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "C command failed");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Alnitak_Cover_Halt(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	CONSOLE_DEBUG(alpacaErrMsg);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Alnitak_TurnOn(const int brightnessValue, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
char				readBuffer[32];
int					readCnt;
int					myBrightnessVal;
char				status_R;

	CONSOLE_DEBUG_W_NUM(__FUNCTION__, brightnessValue);

	if (cCalibrationIsOn == false)
	{
		CONSOLE_DEBUG("Sending cmd to turn on calibration light....");
		readCnt	=	SendCmd_GetResponse('L', 0, readBuffer);
		if (readCnt > 0)
		{
			CONSOLE_DEBUG(readBuffer);
			alpacaErrCode		=	kASCOM_Err_Success;
			cCalibrationIsOn	=	true;
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			CONSOLE_DEBUG("L command failed");
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "L command failed");
		}
		usleep(200 * 1000);
	}
	readCnt	=	SendCmd_GetResponse('S', 0, readBuffer);
	if (readCnt > 0)
	{
		CONSOLE_DEBUG(readBuffer);
		status_R		=	readBuffer[5];
		CONSOLE_DEBUG_W_HEX("status_R\t=", status_R);
	}
	if (cCalibrationIsOn == true)
	{
		myBrightnessVal	=	brightnessValue;
		if (myBrightnessVal < 0)
		{
			myBrightnessVal	=	0;
		}
		if (myBrightnessVal > 255)
		{
			myBrightnessVal	=	255;
		}

		//	Brightness	>BxxxCR		*BiixxxCR		Set brightness (xxx = 000-255)
		readCnt	=	SendCmd_GetResponse('B', myBrightnessVal, readBuffer);
		if (readCnt > 0)
		{
			cCoverCalibrationProp.Brightness	=	myBrightnessVal;
			CONSOLE_DEBUG(readBuffer);
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_NotConnected;
			CONSOLE_DEBUG("B command failed");
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "B command failed");
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		CONSOLE_DEBUG("cCalibrationIsOn is false")
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "L command failed");
	}

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Alnitak_TurnOff(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
char				readBuffer[32];
int					readCnt;

	CONSOLE_DEBUG(__FUNCTION__);

	//*	set the brightness level to 0 first
	readCnt	=	SendCmd_GetResponse('B', 0, readBuffer);
	if (readCnt > 0)
	{
		CONSOLE_DEBUG(readBuffer);
		alpacaErrCode					=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "D command failed");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	cCoverCalibrationProp.Brightness	=	0;
	cNewBrightnessValue					=	0;

	//*	now turn OFF the calibration panel
	readCnt	=	SendCmd_GetResponse('D', 0, readBuffer);
	if (readCnt > 0)
	{
		CONSOLE_DEBUG(readBuffer);
		alpacaErrCode					=	kASCOM_Err_Success;
		cCalibrationIsOn				=	false;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "D command failed");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
//		State		>S000CR		*SiiqrsCR	Get device status
//	ii is a two digit product id, 99 for Flip-Flat, 19 for Flat-Man, 10 for XL, 15 Flat-Man_L.
//	qrs is device status where:
//	s = 0 cover not open/closed
//	s = 1 cover closed
//	s = 2 cover open
//	s = 3 timed out (open/closed not detected)
//	r = 0 light off
//	r = 1 light on
//	q = 0 motor stopped
//	q = 1 motor running
//*****************************************************************************
TYPE_ASCOM_STATUS	CalibrationDriverAlnitak::Alnitak_Cover_GetStatus(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
char				readBuffer[32];
int					readCnt;
char				status_Q;
char				status_R;
char				status_S;

//	CONSOLE_DEBUG(__FUNCTION__);

	readCnt	=	SendCmd_GetResponse('S', 0, readBuffer);
	if (readCnt > 0)
	{
//		CONSOLE_DEBUG(readBuffer);
		if (strncmp(readBuffer, "*S", 2) == 0)
		{
			alpacaErrCode	=	kASCOM_Err_Success;
			status_Q		=	readBuffer[4];
			status_R		=	readBuffer[5];
			status_S		=	readBuffer[6];
//			CONSOLE_DEBUG_W_HEX("status_Q\t=", status_Q);
//			CONSOLE_DEBUG_W_HEX("status_R\t=", status_R);
//			CONSOLE_DEBUG_W_HEX("status_S\t=", status_S);
			if (status_Q == '1')
			{
				CONSOLE_DEBUG("kCover_Moving");
				cCoverCalibrationProp.CoverState	=	kCover_Moving;
			}
			switch(status_S)
			{
				case '0':
					cCoverCalibrationProp.CoverState	=	kCover_Unknown;
//					cCoverCalibrationProp.CoverState	=	kCover_Moving;
					break;
				case '1':
					cCoverCalibrationProp.CoverState	=	kCover_Closed;
					break;
				case '2':
					cCoverCalibrationProp.CoverState	=	kCover_Open;
					break;
				case '3':
					cCoverCalibrationProp.CoverState	=	kCover_Unknown;
					break;
			}
		}
		else
		{
			CONSOLE_DEBUG_W_STR("invalid response\t=", readBuffer);
		}

		//--------------------------
		//*	now look at the light
		if (status_R == '1')
		{
			cCalibrationIsOn	=	true;
		}
		else if (status_R == '0')
		{
			cCalibrationIsOn	=	false;
		}
//		CONSOLE_DEBUG_W_NUM( "CoverState      \t=", cCoverCalibrationProp.CoverState);
//		CONSOLE_DEBUG_W_BOOL("cCalibrationIsOn\t=", cCalibrationIsOn);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;

		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "S command failed");
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

#endif	//	defined(_ENABLE_CALIBRATION_) && defined(_ENABLE_CALIBRATION_ALNITAK_)

