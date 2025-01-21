//**************************************************************************
//*	Name:			telescopedriver_ExpSci.cpp
//*
//*	Author:			Mark Sproul (C) 2024
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*					This driver implements an Alpaca Telescope
//*					talking to an PMC8 compatible mount
//*					via USB port
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
//*
//*
//*	References:		https://ascom-standards.org/api/
//*					https://ascom-standards.org/Help/Developer/html/N_ASCOM_DeviceInterface.htm
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 20,	2024	<MLS> Ordered Explore Scientific iEXOS-100-2 PMC-Eight Equatorial Tracker System
//*	Apr 21,	2024	<MLS> Created telescopedriver_ExpSci.cpp
//*	Apr 23,	2024	<MLS> Started on Explore Scientific setup web page
//*	May  8,	2024	<MLS> Received iEXOS-100-2 PMC-Eight from Explore Scientifc
//*	May  9,	2024	<MLS> Communications working to PCM8
//*	May  9,	2024	<MLS> Park command working (mostly)
//*	May  9,	2024	<MLS> Added DumpPMC8data()
//*	May 10,	2024	<MLS> Added SetPMC8parameters()
//*	May 10,	2024	<MLS> Tracking rates working properly
//*	May 11,	2024	<MLS> Added MotorCounts_to_RA() (from Driver.vb)
//*	May 11,	2024	<MLS> Added MotorCounts_to_DEC() (from Driver.vb)
//*	May 12,	2024	<MLS> Added DEC_to_MotorCounts() (from Driver.vb)
//*	May 12,	2024	<MLS> Added RA_to_MotorCounts() (from Driver.vb)
//*	May 12,	2024	<MLS> GOTO working
//*	May 12,	2024	<MLS> Have functioning driver in only 4 days
//*	May 12,	2024	<MLS> Added UpdateRaDec()
//*	May 13,	2024	<MLS> Added CalcTrackingRateFromStepValue()
//*	May 13,	2024	<MLS> Added CalcAxisRateFromStepValue()
//*	May 13,	2024	<MLS> Copied Side of Pier calculations from Driver.vb
//*	May 15,	2024	<MLS> Added ProcessAcceleration_RA() & ProcessAcceleration_DEC()
//*	May 15,	2024	<MLS> Received ASCOM driver V2.0 dated 29-MAR-2024 from <WEH>
//*	May 15,	2024	<MLS> Updated SideOfPier routines
//*	May 17,	2024	<MLS> Added ProcessESGI()
//*	May 20,	2024	<MLS> Added movement limits for slewing
//*****************************************************************************


#ifdef _ENABLE_TELESCOPE_EXP_SCI_

#include	<ctype.h>
#include	<errno.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<math.h>
#include	<termios.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"usbmanager.h"
#include	"sidereal.h"
#include	"observatory_settings.h"
#include	"linuxerrors.h"


#include	"telescopedriver.h"
#include	"telescopedriver_comm.h"
#include	"telescopedriver_ExpSci.h"

static int	hextoi(const char *hexString);
static int	hextoi(const char *hexString, int length);

#define		kAccelerationSeconds			2
#define		kAccelerationSteps				200

#define		kAccelerationInterval_milliSecs	((kAccelerationSeconds * 1000) / kAccelerationSteps)
#define		kAccelerationInterval_microSecs	((kAccelerationSeconds * 1000000) / kAccelerationSteps)
#define		kMaxStepRate					40000
#define		kAccelerationAmount				(kMaxStepRate / kAccelerationSteps)
//**************************************************************************************
//*	returns the number of objects created (1 or 0)
//**************************************************************************************
int		CreateTelescopeObjects_ExploreScientific(void)
{
int		telescopeCnt;
bool	validUSBpath;
char	usbPath[64];

	CONSOLE_DEBUG(__FUNCTION__);
	telescopeCnt	=	0;
	validUSBpath	=	true;
	while (validUSBpath)
	{
		validUSBpath	=	USB_GetPathFromID("FTDI", usbPath);
		CONSOLE_DEBUG_W_BOOL("validUSBpath\t=", validUSBpath);
		if (validUSBpath)
		{
			CONSOLE_DEBUG_W_STR("usbPath\t=", usbPath);
			new TelescopeDriverExpSci(usbPath);
			telescopeCnt++;
		}
	}
	CONSOLE_DEBUG_W_NUM("telescopeCnt\t=", telescopeCnt);
	AddSupportedDevice(kDeviceType_Telescope, "Explore Scientific", "", "");

	return(telescopeCnt);
}

//**************************************************************************************
//*	the device path is one of these options
//*		192.168.1.104:49152
//*		/dev/ttyUSB0
//*		/dev/ttyS0
//**************************************************************************************
TelescopeDriverExpSci::TelescopeDriverExpSci(const char *devicePath)
	:TelescopeDriverComm(kDevCon_USB, devicePath)
{

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name,		"Telescope-ExploreScientific");
	strcpy(cCommonProp.Description,	"Telescope control using PMC-8 protocol");

	//*	setup the options for this driver
	cTelescopeProp.AlginmentMode			=	kAlignmentMode_algGermanPolar;

	cTelescopeProp.CanFindHome				=	true;
	cTelescopeProp.CanMoveAxis[kAxis_RA]	=	true;
	cTelescopeProp.CanMoveAxis[kAxis_DEC]	=	true;
	cTelescopeProp.CanPulseGuide			=	false;	//*	for now make it false
	cTelescopeProp.CanSetDeclinationRate	=	true;
	cTelescopeProp.CanSetRightAscensionRate	=	true;
	cTelescopeProp.CanSetTracking			=	true;
	cTelescopeProp.CanSlewAsync				=	true;
	cTelescopeProp.CanSync					=	true;

	cTelescopeProp.CanPark					=	true;
	cTelescopeProp.CanSetPark				=	true;
	cTelescopeProp.CanUnpark				=	true;


	cDriverSupportsSetup					=	true;
	cDriverSupportsJavaScript				=	true;
	cTelescopeInfoValid						=	false;
	cTelescopeRA_String[0]					=	0;
	cTelescopeDecl_String[0]				=	0;
	cQueuedCmdCnt							=	0;
	cBaudRate								=	B115200;
	cAxisRate_RA							=	0;
	cAxisRate_DEC							=	0;
	cTrackingRate							=	0;
	cPosition_RA							=	0;
	cPosition_DEC							=	0;
	cFormatErrCnt							=	0;
	cUSBxmitErrCnt							=	0;

	cMoveAxisMode_RA						=	kMoveAxisMode_idle;
	cMoveAxisMode_DEC						=	kMoveAxisMode_idle;
	cMoveAxisLimit_RA						=	0;
	cMoveAxisLimit_DEC						=	0;
	cPMC8mountType							=	kPMC8mount_Invalid;
	DumpTelescopeDriverStruct(&cTelescopeProp);

//	SetPMC8parameters(kPMC8mount_iEXOS100_ESP);


	AlpacaConnect();
	AddCmdToQueue("ESGv!");
	AddCmdToQueue("ESGi!");		//*	this gets the model number and sets everything up



//	Gs! 0x0A     get solar or sidereal time            ESGsA! A = 0 for solar, 1 for sidereal
//	Gl! 0x0B     get site location                     ESGlA!  A = 0 for lat, 1 for lon
//	Gy! 0x0C     get date                              ESGy!
//	AddCmdToQueue("ESGs!");
//	AddCmdToQueue("ESGl!");
	AddCmdToQueue("ESGy!");
	AddCmdToQueue("ESGx!");	//	Gx! 0x0F     get current tracking rate value       ESGx!  (Track rate only applies to RA)


	AddCmdToQueue("ESGw!");	//	Gw! 0x09     get WiFi channel                      ESGw!
}

//**************************************************************************************
// Destructor
//**************************************************************************************
TelescopeDriverExpSci::~TelescopeDriverExpSci(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	AlpacaDisConnect();
}



//**************************************************************************************
//*	returns # of bytes written
//**************************************************************************************
static int	USB_SendCommand(const int fileDesc, const char *theCommand)
{
int		sLen;
int		bytesWritten;
char	errorString[128];

	bytesWritten	=	0;
	sLen			=	strlen(theCommand);

	if (sLen > 0)
	{
	//	CONSOLE_DEBUG_W_STR("sending:", cmdBuffer);
		bytesWritten	=	write(fileDesc, theCommand, sLen);
		if (bytesWritten < 0)
		{
			GetLinuxErrorString(errno, errorString);
			CONSOLE_DEBUG_W_STR("Error occurred on write, errno=", errorString);
		}
	}
	return(bytesWritten);
}

//**************************************************************************
//*	returns # chars read
//**************************************************************************
static int	ReadUntilChar(	const int	fileDesc,
							char		*readBuff,
							const int	maxChars,
							const char	terminator)
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
			usleep(100);
		}
	}
	return(ccc);
}


//**************************************************************************************
//*	Input:	raw step rate value, HI RES value
//*	Output: arc-seconds per second
//**************************************************************************************
double	TelescopeDriverExpSci::CalcTrackingRateFromStepValue(const int highResMicroSteps)
{
double	microSteps;
double	arcSecondsPerSec;

//		cPMC8.TrackingRate_MicroStep[iii]	=	siderealStepRates[iii] / cPMC8.ArcSecPerMicroStep;
//		cPMC8.TrackingRate_value[iii]		=	cPMC8.TrackingRate_MicroStep[iii] * 25;

	microSteps			=	(1.0 * highResMicroSteps) / 25.0;
	arcSecondsPerSec	=	microSteps * cPMC8.ArcSecPerMicroStep;
	return(arcSecondsPerSec);
}

//**************************************************************************************
//*	Input:	raw step rate value, HI RES value
//*	Output: arc-seconds per second
//**************************************************************************************
double	TelescopeDriverExpSci::CalcAxisRateFromStepValue(const int normalMicroSteps)
{
double	arcSecondsPerSec;


	arcSecondsPerSec	=	normalMicroSteps * cPMC8.ArcSecPerMicroStep;
	return(arcSecondsPerSec);
}


//*****************************************************************************
const char	*gPMC8mountNames[]	=
{

	"iEXOS100",			//	0x0,	//	0x0300	iEXOS100 ES1A01CB11A01	//	original 100 board with the original ESP module
	"iEXOS100_ESP",		//	0x1,	//	0x0301 iEXOS100 ES1A02AC11A01	//	ESP module
	"iEXOS200",			//	0x2,	//	0x0302 iEXOS200 ES1A02AC11A01
	"iEXOS300",			//	0x3,	//	0x0303 iEXOS300 ES2A07AC11A01
	"G11_RN131",		//	0x4,	//	0x0304 G11 ES2A06BA11A01, RN131
	"G11_ESP32",		//	0x5,	//	0x0305 G11 ES2A06BC11A01, ESP32
	"G11_RN131_2",		//	0x6,	//	0x0306 G11 ES2A07AA11A01, RN131
	"G11_ESP32_2",		//	0x7,	//	0x0307 G11 ES2A07AC11A01, ESP32
	"EXOS2_RN131",		//	0x8,	//	0x0308 EXOS2 ES2A06BA11A01, RN131
	"EXOS2_ESP32",		//	0x9,	//	0x0309 EXOS2 ES2A06BC11A01, ESP32
	"EXOS2_RN131_2",	//	0xA,	//	0x030A EXOS2 ES2A07AA11A01, RN131
	"EXOS2_ESP32_2",	//	0xB,	//	0x030B EXOS2 ES2A07AC11A01, ESP32
	"Scotty_RN131",		//	0xC,	//	0x030C Scotty ES2A07AC11A01, RN131
	"Titan_RN131",		//	0xD,	//	0x0207 Titan ES2A07AC11A01, RN131
};


//**************************************************************************************
//*	values for tracking rates, refer to alpaca_defs.h
//	kDriveRate_driveSidereal	=	0,	//*	Sidereal tracking rate (15.041 arcseconds per second).
//	kDriveRate_driveLunar		=	1,	//*	Lunar tracking rate (14.685 arcseconds per second).
//	kDriveRate_driveSolar		=	2,	//*	Solar tracking rate (15.0 arcseconds per second).
//	kDriveRate_driveKing		=	3	//*	King tracking rate (15.0369 arcseconds per second).
//**************************************************************************************
//	---------------------------------
//	0	15.0410	53.4791	1336	0538
//	1	14.6850	52.2133	1305	0519
//	2	15.0000	53.3333	1333	0535
//	3	15.0369	53.4645	1336	0538
//	---------------------------------
//	0	15.0410	48.1312	1203	04B3
//	1	14.6850	46.9920	1174	0496
//	2	15.0000	48.0000	1200	04B0
//	3	15.0369	48.1181	1202	04B2
//**************************************************************************************
void	TelescopeDriverExpSci::SetPMC8parameters(TYPE_PMC8mount mountType)
{
double	alpacaStepRates[kDriveRate_Count];
double	siderealStepRates[kDriveRate_Count];
int		iii;
double	arcSecondsPerSec_check;
double	microSecs_check;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("mountType\t=", mountType);

	cPMC8mountType	=	mountType;

	//----------------------------------------------------------------
	//*	IMPORTANT NOTE!!!!
	//*	The PMC8 uses sidereal time internally not solar time.
	//*	That makes these values slightly different than the ones published in the ASCOM standards
	//*	https://ascom-standards.org/Help/Platform/html/T_ASCOM_DeviceInterface_DriveRates.htm
	//----------------------------------------------------------------
	//*	These values are in arc-seconds per second (arcsecondspersec)
	//*	Rates		ASCOM			Sidereal
	//*	Sidereal	15.041			15.000
	//*	Lunar		14.685			14.451
	//*	Solar		15.0			14.959
	//*	King		15.0369			14.996
	//----------------------------------------------------------------
	alpacaStepRates[kDriveRate_driveSidereal]	=	15.041;
	alpacaStepRates[kDriveRate_driveLunar]		=	14.685;
	alpacaStepRates[kDriveRate_driveSolar]		=	15.0;
	alpacaStepRates[kDriveRate_driveKing]		=	15.0369;

	siderealStepRates[kDriveRate_driveSidereal]	=	15.000;
	siderealStepRates[kDriveRate_driveLunar]	=	14.451;
	siderealStepRates[kDriveRate_driveSolar]	=	14.959;
	siderealStepRates[kDriveRate_driveKing]		=	14.996;


	strcpy(cPMC8.ModelName, gPMC8mountNames[mountType]);

	switch(mountType)
	{
		case kPMC8mount_iEXOS100:		//	0x0300	iEXOS100 ES1A01CB11A01	//	original 100 board with the original ESP module
		case kPMC8mount_iEXOS100_ESP:	//	0x0301 iEXOS100 ES1A02AC11A01	//	ESP module
		case kPMC8mount_iEXOS200:		//	0x0302 iEXOS200 ES1A02AC11A01
		case kPMC8mount_iEXOS300:		//	0x0303 iEXOS300 ES2A07AC11A01
			cPMC8.StepsPer360_RA		=	kPMC8_RA_steps_EXOS;
			cPMC8.StepsPer360_DEC		=	kPMC8_DEC_steps_EXOS;
			cPMC8.ArcSecPerMicroStep	=	0.31250;	//*	arc-sec / microstep
			break;

		case kPMC8mount_G11_RN131:		//	0x0304 G11 ES2A06BA11A01, RN131
		case kPMC8mount_G11_ESP32:		//	0x0305 G11 ES2A06BC11A01, ESP32
		case kPMC8mount_G11_RN131_2:	//	0x0306 G11 ES2A07AA11A01, RN131
		case kPMC8mount_G11_ESP32_2:	//	0x0307 G11 ES2A07AC11A01, ESP32
			cPMC8.StepsPer360_RA		=	kPMC8_RA_steps_G11;
			cPMC8.StepsPer360_DEC		=	kPMC8_DEC_steps_G11;
			cPMC8.ArcSecPerMicroStep	=	0.28125;	//*	arc-sec / microstep
			break;

		case kPMC8mount_EXOS2_RN131:	//	0x0308 EXOS2 ES2A06BA11A01, RN131
		case kPMC8mount_EXOS2_ESP32:	//	0x0309 EXOS2 ES2A06BC11A01, ESP32
		case kPMC8mount_EXOS2_RN131_2:	//	0x030A EXOS2 ES2A07AA11A01, RN131
		case kPMC8mount_EXOS2_ESP32_2:	//	0x030B EXOS2 ES2A07AC11A01, ESP32
		case kPMC8mount_Scotty_RN131:	//	0x030C Scotty ES2A07AC11A01, RN131
		case kPMC8mount_Titan_RN131:	//	0x0207 Titan ES2A07AC11A01, RN131
		default:
			//*	these values are not right, just a place holder for now
			cPMC8.StepsPer360_RA		=	kPMC8_RA_steps_EXOS;
			cPMC8.StepsPer360_DEC		=	kPMC8_DEC_steps_EXOS;
			cPMC8.ArcSecPerMicroStep	=	0.31250;	//*	arc-sec / microstep
			break;


	}
	cPMC8.EnableMovementLimits	=	true;
	cPMC8.MovementLimit_RA_East	=	-cPMC8.StepsPer360_RA / 4;
	cPMC8.MovementLimit_RA_West	=	cPMC8.StepsPer360_RA / 4;

	//	The calculated precision tracking value for a sidereal rate of 15.000”arc sidereal-sec-1 is:
	//	Precision Tracking value	= (15.000 / 0.28125) * 25
	//								= 53.333 * 25
	//								= 1333 (decimal)
	//								= 0x0535 (hexadecimal)
	printf("---------------------------------\r\n");
	printf("i\t");
	printf("stepRate\t");
	printf("microSteps\t");
	printf("value\t");
	printf("value\t");
	printf("Ck-microsteps\t");
	printf("Ck-Arcsecs\t");
	printf("\r\n");

	for (iii=0; iii<kDriveRate_Count; iii++)
	{
		cPMC8.TrackingRate_ASCOM[iii]		=	alpacaStepRates[iii];
		cPMC8.TrackingRate_Sidereal[iii]	=	siderealStepRates[iii];
		cPMC8.TrackingRate_MicroStep[iii]	=	siderealStepRates[iii] / cPMC8.ArcSecPerMicroStep;
		cPMC8.TrackingRate_value[iii]		=	cPMC8.TrackingRate_MicroStep[iii] * 25.0;

		//*	this is for checking/debugging
		microSecs_check						=	cPMC8.TrackingRate_value[iii] / 25.0;
		arcSecondsPerSec_check				=	CalcTrackingRateFromStepValue(cPMC8.TrackingRate_value[iii]);
		printf("%d\t",		iii);
		printf("%5.5f\t",	cPMC8.TrackingRate_Sidereal[iii]);
		printf("%5.5f\t",	cPMC8.TrackingRate_MicroStep[iii]);
		printf("%4d\t",		cPMC8.TrackingRate_value[iii]);
		printf("%04X\t",	cPMC8.TrackingRate_value[iii]);
		printf("%5.5f\t",	microSecs_check);
		printf("%5.5f\t",	arcSecondsPerSec_check);
		printf("\r\n");
	}
}

//**************************************************************************************
bool	TelescopeDriverExpSci::SendCmdsFromQueue(void)
{
int		iii;
bool	sentOK;

//	CONSOLE_DEBUG(__FUNCTION__);
	sentOK	=	true;
	while ((cQueuedCmdCnt > 0) && (sentOK == true))
	{
//		CONSOLE_DEBUG_W_STR("Sending", cCmdQueue[0].cmdString);
		sentOK	=	SendPMC8command(cCmdQueue[0].cmdString);
		if (sentOK == false)
		{
			CONSOLE_DEBUG_W_STR("SendPMC8command() failed\t=", cCmdQueue[0].cmdString);
			cUSBxmitErrCnt++;
		}
		for (iii=0; iii<cQueuedCmdCnt; iii++)
		{
			cCmdQueue[iii]	=	cCmdQueue[iii + 1];
		}
		cQueuedCmdCnt--;
		if (cQueuedCmdCnt > 0)
		{
			usleep(1000);
		}
	}
	return(sentOK);
}


//**************************************************************************************
//	The total count for a 360-degree rotation of both the RA and DEC axis is
//	equal to 360 teeth * 400 steps * 32 microsteps, which is equal to 4,608,000 counts
//	per 360-degree rotation.
//
//	The angular motion per step in “arc is equal to the total number of “arc/4,608,000 counts.
//	360 degrees is equal to 360 x 60 x 60”arc or 1,296,000”arc. The “arc count-1 is
//	equal to 1,296,000/4,608,000 or 0.28125”arc count-1.
//**************************************************************************************

//**************************************************************************************
//	https://github.com/OpenGOTOCommunity/PMC-EightASCOMDriver/blob/master/Driver.vb
//**************************************************************************************
//    Public Function MotorCounts_to_RA(MC_value As Int32) As Double
//        Dim MotorAngle As Double
//        Dim RA_value As Double
//        Dim HourAngle As Double
//        Dim DECCounts As Int32
//
//        DECCounts = GetDECMotorPosition()
//
//        MotorAngle = (24.0# * MC_value) / Telescope.MountRACounts
//
//        If DECCounts < 0 Then
//            HourAngle = MotorAngle + 6
//        ElseIf DECCounts >= 0 Then
//            HourAngle = MotorAngle - 6
//        End If
//
//        RA_value = SiderealTime - HourAngle
//
//        If RA_value >= 24.0# Then
//            RA_value = RA_value - 24.0#
//        ElseIf RA_value < 0.0# Then
//            RA_value = RA_value + 24.0#
//        End If
//
//        Return RA_value
//    End Function
//**************************************************************************************
double	TelescopeDriverExpSci::MotorCounts_to_RA(const int mcValue)
{
double	motorAngle;
double	ra_value;
double	hourAngle;
int		decCounts;
double	siderealTime;

	siderealTime	=	CalcSiderealTime_dbl(NULL, gObseratorySettings.Longitude_deg);
	decCounts		=	cPosition_DEC;

	motorAngle	=	(24.0 * mcValue) / cPMC8.StepsPer360_RA;

	if (decCounts < 0)
	{
		hourAngle	=	motorAngle + 6;
	}
	else
	{
		//	(decCounts >= 0)
		hourAngle	=	motorAngle - 6;
	}

	ra_value	=	siderealTime - hourAngle;

	if (ra_value >= 24.0)
	{
		ra_value	=	ra_value - 24.0;
	}
	else if (ra_value < 0.0)
	{
		ra_value	=	ra_value + 24.0;
	}

//	CONSOLE_DEBUG("----------------------------------");
//	CONSOLE_DEBUG_W_NUM("decCounts     \t=",	decCounts);
//	CONSOLE_DEBUG_W_NUM("StepsPer360_RA\t=",	cPMC8.StepsPer360_RA);
//	CONSOLE_DEBUG_W_DBL("siderealTime  \t=",	siderealTime);
//	CONSOLE_DEBUG_W_DBL("hourAngle     \t=",	hourAngle);
//	CONSOLE_DEBUG_W_DBL("Longitude_deg \t=",	gObseratorySettings.Longitude_deg);
//	CONSOLE_DEBUG_W_DBL("ra_value      \t=",	ra_value);
	return(ra_value);
}

//**************************************************************************************
//    Public Function MotorCounts_to_DEC(MC_value As Int32) As Double
//        Dim MotorAngle As Double
//        Dim DEC_value As Double
//
//        MotorAngle = (360.0# * MC_value) / Telescope.MountDECCounts
//
//        If MotorAngle >= 0 Then
//            DEC_value = 90 - MotorAngle
//        ElseIf MotorAngle < 0 Then
//            DEC_value = 90 + MotorAngle
//        End If
//
//        Return DEC_value
//    End Function
//**************************************************************************************
double	TelescopeDriverExpSci::MotorCounts_to_DEC(const int mcValue)
{
double	motorAngle;
double	decValue;

//	motorAngle	=	(360.0 * mcValue) / kPMC8_DEC_steps;
	motorAngle	=	(360.0 * mcValue) / cPMC8.StepsPer360_DEC;
	if (motorAngle >= 0.0)
	{
		decValue	=	90.0 - motorAngle;
	}
	else if (motorAngle < 0.0)
	{
		decValue	=	90.0 + motorAngle;
	}
	return(decValue);
}

//**************************************************************************************
// Public Function RA_to_MotorCounts(RA_value As Double, SOP As PierSide) As Int32
//        Dim MotorAngle As Double
//        Dim MotorCounts As Int32
//        Dim HourAngle As Double
//
//        HourAngle = SiderealTime - RA_value
//
//        'limit values to +/- 12 hours
//        If HourAngle > 12 Then
//            HourAngle = HourAngle - 24
//        ElseIf HourAngle <= -12 Then
//            HourAngle = HourAngle + 24
//        End If
//
//        If Telescope.SiteLatitudeValue >= 0 Then
//            If SOP = PierSide.pierEast Then
//                MotorAngle = HourAngle - 6
//            ElseIf SOP = PierSide.pierWest Then
//                MotorAngle = HourAngle + 6
//            End If
//        ElseIf Telescope.SiteLatitudeValue < 0 Then
//            If SOP = PierSide.pierEast Then
//                MotorAngle = -(HourAngle + 6)
//            ElseIf SOP = PierSide.pierWest Then
//                MotorAngle = -(HourAngle - 6)
//            End If
//        End If
//
//        MotorCounts = MotorAngle * MountRACounts / 24
//
//        Return MotorCounts
//    End Function
//**************************************************************************************
int	TelescopeDriverExpSci::RA_to_MotorCounts(double ra_value_hrs, TYPE_PierSide pierSide)
{
int		motorCounts;
double	motorAngle;
double	hourAngle;
double	siderealTime;

	siderealTime	=	CalcSiderealTime_dbl(NULL, gObseratorySettings.Longitude_deg);
	hourAngle		=	siderealTime - ra_value_hrs;
	//*	limit values to +/- 12 hours
	if (hourAngle > 12)
	{
		hourAngle	-=	24.0;
	}
	else if (hourAngle <= -12)
	{
		hourAngle	+=	24.0;
	}

	if (gObseratorySettings.Latitude_deg >= 0.0)
	{
		//*	northern hemisphere
		if (pierSide == kPierSide_pierEast)
		{
			motorAngle	=	hourAngle - 6;
		}
		else if (pierSide == kPierSide_pierWest)
		{
			motorAngle	=	hourAngle + 6;
		}
		else
		{
			CONSOLE_ABORT(__FUNCTION__);
		}
	}
	else
	{
		//*	souther hemisphere
		if (pierSide == kPierSide_pierEast)
		{
			motorAngle	=	-(hourAngle + 6);
		}
		else if (pierSide == kPierSide_pierWest)
		{
			motorAngle	=	-(hourAngle - 6);
		}
		else
		{
			CONSOLE_ABORT(__FUNCTION__);
		}
	}
	motorCounts	=	motorAngle * cPMC8.StepsPer360_RA / 24;
	return(motorCounts);
}


//**************************************************************************************
//    Public Function DEC_to_MotorCounts(DEC_value As Double, SOP As PierSide) As Int32
//        Dim MotorAngle As Double
//        Dim MotorCounts As Int32
//
//
//        'Adjust DEC value for northern or southern hemisiphere (GRH 2018-12-20)
//        If Telescope.SiteLatitudeValue >= 0 Then
//            If SOP = PierSide.pierEast Then
//                MotorAngle = (DEC_value - 90.0#)
//            ElseIf SOP = PierSide.pierWest Then
//                MotorAngle = -(DEC_value - 90.0#)
//            End If
//        ElseIf Telescope.SiteLatitudeValue < 0 Then
//            If SOP = PierSide.pierEast Then
//                MotorAngle = -(DEC_value + 90.0#)
//            ElseIf SOP = PierSide.pierWest Then
//                MotorAngle = (DEC_value + 90.0#)
//            End If
//
//        End If
//
//        MotorCounts = (MotorAngle / 360.0) * Telescope.MountDECCounts
//
//        Return MotorCounts
//    End Function
//**************************************************************************************
int	TelescopeDriverExpSci::DEC_to_MotorCounts(double dec_Value_deg, TYPE_PierSide pierSide)
{
double	motorAngle;
int		motorCounts;

	//*	Adjust DEC value for northern or southern hemisphere
	if (gObseratorySettings.Latitude_deg >= 0.0)
	{
		if (pierSide == kPierSide_pierEast)
		{
			motorAngle	=	(dec_Value_deg - 90.0);
		}
		else
		{
			motorAngle	=	-(dec_Value_deg - 90.0);
		}
	}
	else
	{
		//*	souther hemisphere
		if (pierSide == kPierSide_pierEast)
		{
			motorAngle	=	-(dec_Value_deg - 90.0);
		}
		else
		{
			motorAngle	=	(dec_Value_deg - 90.0);
		}
	}
	motorCounts = (motorAngle / 360.0) * cPMC8.StepsPer360_DEC;
	return(motorCounts);
}


//**************************************************************************************
void	TelescopeDriverExpSci::UpdateRaDec(	const int	axis,
											const int	rawMotorPosition,
											const int	motorDirection,
											const int	rawMotorVelocity)
{
int		motorPosition;
double	siderealTime;

	motorPosition	=	rawMotorPosition;
	if (motorPosition & 0x00800000)
	{
		motorPosition	|=	0xff000000;
	}
	if ((axis == 0) || (axis == '0'))
	{
		//*	right ascension axis
		cPosition_RA						=	motorPosition;
		cTelescopeProp.RightAscension		=	MotorCounts_to_RA(motorPosition);
		//*	HA = SiderealTime - RightAscension
		siderealTime						=	CalcSiderealTime_dbl(NULL, gObseratorySettings.Longitude_deg);
		cTelescopeProp.HourAngle			=	siderealTime - cTelescopeProp.RightAscension;
//		CONSOLE_DEBUG_W_DBL("cTelescopeProp.RightAscension\t=",	cTelescopeProp.RightAscension);
	}
	else if ((axis == 1) || (axis == '1'))
	{
		//*	declination axis
		cPosition_DEC				=	motorPosition;
		cTelescopeProp.Declination	=	MotorCounts_to_DEC(motorPosition);
//		CONSOLE_DEBUG_W_DBL("cTelescopeProp.Declination\t\t=",	cTelescopeProp.Declination);
	}
	else
	{
		CONSOLE_DEBUG("Invalid axis!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
	}
	if ((cPosition_RA == 0) && (cPosition_DEC == 0))
	{
		cTelescopeProp.AtHome	=	true;
	}
	else
	{
		cTelescopeProp.AtHome	=	false;
	}
	//*	update the side of pier
	cTelescopeProp.PhysicalSideOfPier	=	Telescope_GetPhysicalSideOfPier();
	//--------------------------------------------------
	//*	now deal with the velocity, a value of 2 or greater means there is no velocity info
	if (motorDirection < 2)
	{
		if (axis == 0)
		{
			cAxisRate_RA	=	rawMotorVelocity;
		}
		else if (axis == 1)
		{
			cAxisRate_DEC	=	rawMotorVelocity;
		}
		else
		{
			CONSOLE_DEBUG("Invalid axis!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		}
		//-----------------------------------------------------------------
		//*	now figure out what type of operation we are doing
		if ((cAxisRate_RA == 0) && (cAxisRate_DEC == 0))
		{
			cTelescopeProp.Slewing	=	false;
			cTelescopeProp.Tracking	=	false;
		}
		else if ((cAxisRate_RA > 100) || (cAxisRate_DEC > 100))
		{
			cTelescopeProp.Slewing	=	true;
			cTelescopeProp.Tracking	=	false;
		}
		else
		{
			cTelescopeProp.Slewing	=	false;
			cTelescopeProp.Tracking	=	true;
		}
	}
}

//**************************************************************************************
void	TelescopeDriverExpSci::ProcessAcceleration_RA(int parsedAxisRate)
{
int			newStepRateValue;
char		esCmdString[32];
uint32_t	deltaMilliSecs;

//	CONSOLE_DEBUG_W_NUM("RA axis rate \t=", parsedValueInt);
	cAxisRate_RA	=	parsedAxisRate;
	switch (cMoveAxisMode_RA)
	{
		case kMoveAxisMode_idle:
			break;

		case kMoveAxisMode_Accel:
			deltaMilliSecs	=	millis() - cMoveAxisLastMilliSecs_RA;
			if (deltaMilliSecs >= kAccelerationInterval_milliSecs)
			{
//				CONSOLE_DEBUG("---------------------------");
//				CONSOLE_DEBUG("kMoveAxisMode_Accel");
//				CONSOLE_DEBUG_W_NUM("cAxisRate_RA     \t=",	cAxisRate_RA);
//				CONSOLE_DEBUG_W_HEX("cAxisRate_RA     \t=",	cAxisRate_RA);
//				CONSOLE_DEBUG_W_HEX("...CurStepRate_RA\t=",	cMoveAxisCurrentStepRate_RA);
//				CONSOLE_DEBUG_W_HEX("cMoveAxisLimit_RA\t=",	cMoveAxisLimit_RA);
				newStepRateValue	=   cMoveAxisCurrentStepRate_RA + kAccelerationAmount;
				if (newStepRateValue > cMoveAxisLimit_RA)
				{
					newStepRateValue	=	cMoveAxisLimit_RA;
					cMoveAxisMode_RA	=	kMoveAxisMode_Steady;
				}
				sprintf(esCmdString, "ESSr0%04X!", newStepRateValue);
				cMoveAxisLastMilliSecs_RA	=	millis();
				AddCmdToQueue(esCmdString);
				cMoveAxisCurrentStepRate_RA	=	newStepRateValue;
			}
			break;

		case kMoveAxisMode_Steady:
			break;

		case kMoveAxisMode_Decel:
			deltaMilliSecs	=	millis() - cMoveAxisLastMilliSecs_RA;
			if (deltaMilliSecs >= kAccelerationInterval_milliSecs)
			{
//				CONSOLE_DEBUG("---------------------------");
//				CONSOLE_DEBUG("kMoveAxisMode_Decel");
//				CONSOLE_DEBUG_W_HEX("...CurStepRate_RA\t=",	cMoveAxisCurrentStepRate_RA);
				//*	we are going to decelerate a little faster than accerate
				newStepRateValue	=   cMoveAxisCurrentStepRate_RA - kAccelerationAmount;
				if (newStepRateValue <= 0)
				{
					newStepRateValue	=	0;
					cMoveAxisMode_RA	=	kMoveAxisMode_Stop;
				}
				sprintf(esCmdString, "ESSr0%04X!", newStepRateValue);
				cMoveAxisLastMilliSecs_RA	=	millis();
				AddCmdToQueue(esCmdString);
				cMoveAxisCurrentStepRate_RA	=	newStepRateValue;
			}
			break;

		case kMoveAxisMode_Stop:
			cMoveAxisMode_RA	=	kMoveAxisMode_idle;
			break;
	}
}

//**************************************************************************************
void	TelescopeDriverExpSci::ProcessAcceleration_DEC(int parsedAxisRate)
{
int			newStepRateValue;
char		esCmdString[32];
uint32_t	deltaMilliSecs;

//	CONSOLE_DEBUG_W_NUM("RA axis rate \t=", parsedValueInt);
	cAxisRate_DEC	=	parsedAxisRate;
	switch (cMoveAxisMode_DEC)
	{
		case kMoveAxisMode_idle:
			break;

		case kMoveAxisMode_Accel:
			deltaMilliSecs	=	millis() - cMoveAxisLastMilliSecs_DEC;
			if (deltaMilliSecs >= kAccelerationInterval_milliSecs)
			{
//				CONSOLE_DEBUG("---------------------------");
//				CONSOLE_DEBUG("kMoveAxisMode_Accel");
//				CONSOLE_DEBUG_W_NUM("cAxisRate_DEC     \t=",	cAxisRate_DEC);
//				CONSOLE_DEBUG_W_HEX("cAxisRate_DEC     \t=",	cAxisRate_DEC);
//				CONSOLE_DEBUG_W_HEX("...CurStepRate_DEC\t=",	cMoveAxisCurrentStepRate_DEC);
//				CONSOLE_DEBUG_W_HEX("cMoveAxisLimit_DEC\t=",	cMoveAxisLimit_DEC);
				newStepRateValue	=   cMoveAxisCurrentStepRate_DEC + kAccelerationAmount;
				if (newStepRateValue > cMoveAxisLimit_DEC)
				{
					newStepRateValue	=	cMoveAxisLimit_DEC;
					cMoveAxisMode_DEC	=	kMoveAxisMode_Steady;
				}
				sprintf(esCmdString, "ESSr1%04X!", newStepRateValue);
				cMoveAxisLastMilliSecs_DEC		=	millis();
				AddCmdToQueue(esCmdString);
				cMoveAxisCurrentStepRate_DEC	=	newStepRateValue;
			}
			break;

		case kMoveAxisMode_Steady:
			break;

		case kMoveAxisMode_Decel:
			deltaMilliSecs	=	millis() - cMoveAxisLastMilliSecs_DEC;
			if (deltaMilliSecs >= kAccelerationInterval_milliSecs)
			{
//				CONSOLE_DEBUG("---------------------------");
//				CONSOLE_DEBUG("kMoveAxisMode_Decel");
//				CONSOLE_DEBUG_W_HEX("...CurStepRate_DEC\t=",	cMoveAxisCurrentStepRate_DEC);
				//*	we are going to decelerate a little faster than accelerate
				newStepRateValue	=   cMoveAxisCurrentStepRate_DEC - kAccelerationAmount;
				if (newStepRateValue <= 0)
				{
					newStepRateValue	=	0;
					cMoveAxisMode_DEC	=	kMoveAxisMode_Stop;
				}
				sprintf(esCmdString, "ESSr1%04X!", newStepRateValue);
				cMoveAxisLastMilliSecs_DEC		=	millis();
				AddCmdToQueue(esCmdString);
				cMoveAxisCurrentStepRate_DEC	=	newStepRateValue;
			}
			break;

		case kMoveAxisMode_Stop:
			cMoveAxisMode_DEC	=	kMoveAxisMode_idle;
			break;
	}
}

//**************************************************************************************
static int	DecimalStrToInt(const char *theString, const int len)
{
int		theValue;
int		iii;

	theValue	=	0;
	for (iii=0; iii<len; iii++)
	{
		theValue	=	theValue * 10;
		theValue	+=	(theString[iii] & 0x0f);
	}

	return(theValue);
}

//**************************************************************************************
//-------------------------------------------------------------
//*	String def as follows
//*	ESGI000000245677788899mmmmMMMMWWSTw!
//
//*	where
//*		000000 = leading zero pad baud
//*		2 = P2
//*		4 = P4
//*		5 = P5
//*		6 = P6
//*		777 = P7
//*		888 = P8
//*		99 = P9
//*		mmmm = MC1
//*		MMMM = MC2
//*		WW = Wifi channel assigned
//*		S = ST4_Disable flag
//*		T = ST4_Type 1 = analog, 0 = digital
//*		w = Wifi Type 0 = RN131, 1 = 8266, 2 = ESP32
//	P codes are:
//		P0: Serial Port Baud Rate. IF unititialized, code will assume factory first boot and set parameters
//		P1: This is a motor enable. if set to zero the motors will be inhibited but all will run otherwise. Allows debug firmware w/o moving the mount: 1= enaled motors, 0 = motors disabled
//		P2: TCP or UDP (iEXOS)
//		P3:
//		P4: Continuous Tracking setting 0=stop with comms loss, 1=continue to track with comms loss
//		P5: Bootup Sidereal Track Enable 0 = no track, 1 = track on boot
//		P6: Bootup Hemisphere 0 = Southern, 1 = Norhtern
//		P7: Sidereal rate fraction RA, ST4 port
//		P8: Sidereal rate fraction DEC, ST4 port
//		P9: UNIT TYPE Model # 0=G11 old, 1=Titan old, 2=EXOS2 old , 3=iEXOS, 4=iEXOS200 FUTURE: 5=G11 new, 6=Titan new, 7=EXOS2 new


//**************************************************************************************
void	TelescopeDriverExpSci::ProcessESGI(const char *esgiString)
{
int		parameters[20];
int		iii;
const char	*names[]	=
{
	"BaudRate",
	"MotorEnable",
	"TCP or UDP (iEXOS)",
	"",
	"Continuous Tracking",
	"Bootup Sidereal Track Enable 0 = no track, 1 = track on boot",
	"Bootup Hemisphere 0 = Southern, 1 = Northern",
	"Sidereal Rate RA",
	"Sidereal Rate DEC",
	"Unit Type  0=G11 old, 1=Titan old, 2=EXOS2 old , 3=iEXOS, 4=iEXOS200........",
	"MC1",
	"MC2",
	"WiFi channel",
	"ST4_Disable flag",
	"ST4_Type 1 = analog, 0 = digital",
	"Wifi Type 0 = RN131, 1 = 8266, 2 = ESP32",
	"",
	""
};
	CONSOLE_DEBUG(__FUNCTION__);

//*	0123456789 123456789 123456789 123456789
//*	ESGI000000245677788899mmmmMMMMWWSTw!
//*	ESGi1152001101040040010900060006002!

	parameters[0]	=	DecimalStrToInt(&esgiString[4], 6);
	parameters[1]	=	0;
	parameters[2]	=	DecimalStrToInt(&esgiString[10], 1);
	parameters[3]	=	0;
	parameters[4]	=	DecimalStrToInt(&esgiString[11], 1);
	parameters[5]	=	DecimalStrToInt(&esgiString[12], 1);
	parameters[6]	=	DecimalStrToInt(&esgiString[13], 1);
	parameters[7]	=	DecimalStrToInt(&esgiString[14], 3);
	parameters[8]	=	DecimalStrToInt(&esgiString[17], 3);
	parameters[9]	=	DecimalStrToInt(&esgiString[20], 2);
	parameters[10]	=	DecimalStrToInt(&esgiString[22], 4);	//*	mmmm
	parameters[11]	=	DecimalStrToInt(&esgiString[26], 4);	//*	MMMM
	parameters[12]	=	DecimalStrToInt(&esgiString[30], 2);	//*	WW
	parameters[13]	=	DecimalStrToInt(&esgiString[32], 1);	//*	S
	parameters[14]	=	DecimalStrToInt(&esgiString[33], 1);	//*	T
	parameters[15]	=	DecimalStrToInt(&esgiString[34], 1);	//*	w

//p00 =  115200	BaudRate
//p01 =       0	MotorEnable
//p02 =       1	TCP or UDP (iEXOS)
//p03 =       0
//p04 =       1	Continuous Tracking
//p05 =       0	Bootup Sidereal Track Enable 0 = no track, 1 = track on boot
//p06 =       1	Bootup Hemisphere 0 = Southern, 1 = Northern
//p07 =      40	Sidereal Rate RA
//p08 =      40	Sidereal Rate DEC
//p09 =       1	Unit Type  0=G11 old, 1=Titan old, 2=EXOS2 old , 3=iEXOS, 4=iEXOS200........
//p10 =     900	MC1
//p11 =     600	MC2
//p12 =       6	WiFi channel
//p13 =       0	ST4_Disable flag
//p14 =       0	ST4_Type 1 = analog, 0 = digital
//p15 =       2	Wifi Type 0 = RN131, 1 = 8266, 2 = ESP32

	cPMC8.BaudRate				=	parameters[0];
	cPMC8.MotorEnable			=	parameters[1];
	cPMC8.TCP_UDP				=	parameters[2];
	cPMC8.TrackAlways			=	parameters[4];
	cPMC8.TrackOnBoot			=	parameters[5];
	cPMC8.NorthernHemisphere	=	parameters[6];
	cPMC8.SiderealRateFrac_RA	=	parameters[7];
	cPMC8.SiderealRateFrac_DEC	=	parameters[8];

	cPMC8.MountType				=	(TYPE_PMC8mount)parameters[9];
	cPMC8.MotorCurrentSlew		=	parameters[10];
	cPMC8.MotorCUrrentTrack		=	parameters[11];
	cPMC8.WifiChannel			=	parameters[12];
	cPMC8.AutoGuiderEnabled		=	parameters[13];
	cPMC8.ST4_Type				=	parameters[14];
	cPMC8.WifiType				=	(TYPE_Wifi)parameters[15];
	switch(cPMC8.WifiType)
	{
		case 	kPMC8wifi_RN131:	strcpy(cPMC8.WifiTypeStr,	"RN131");	break;
		case 	kPMC8wifi_8266:		strcpy(cPMC8.WifiTypeStr,	"8266");	break;
		case 	kPMC8wifi_ESP32:	strcpy(cPMC8.WifiTypeStr,	"ESP32");	break;
		default:					strcpy(cPMC8.WifiTypeStr,	"unknown");	break;
	}

	CONSOLE_DEBUG_W_STR("ESGi\t=",	esgiString);
	printf("%s\r\n", esgiString);
	for (iii=0; iii<16; iii++)
	{
		printf("p%02d = %7d\t%s\r\n", iii, parameters[iii], names[iii]);
	}


	CONSOLE_DEBUG_W_STR("cPMC8.ModelName\t=", cPMC8.ModelName);

	//*	set the mount type if it is not already set
	if (cPMC8mountType < 0)
	{
		if ((parameters[9] >= 0) && (parameters[9] < kPMC8mount_Last))
		{
			SetPMC8parameters((TYPE_PMC8mount)parameters[9]);
		}
		else
		{
			CONSOLE_ABORT(__FUNCTION__);
		}
	}


//	CONSOLE_ABORT(__FUNCTION__);
}


//**************************************************************************************
bool	TelescopeDriverExpSci::ProcessPMC8response(char *pmc8Response)
{
bool		wasProcessed;
uint32_t	cmdPrefix;
int			parsedValueInt;
char		axisArg;
char		myPMC8response[64];
char		*esPrefixPtr;
int			iii;
int			ccc;
char		cmdChar1;
char		cmdChar2;
char		cmdChar3;
char		cmdChar4;

	wasProcessed	=	true;
	//*	for some strange reason, the response to the version number
	//*	has a 0x07 as the first character, this fixes that problem
	if (pmc8Response[0] != 'E')
	{
		cFormatErrCnt++;
	}
	esPrefixPtr		=	strstr(pmc8Response, "ES");
	if (esPrefixPtr != NULL)
	{
		strcpy(myPMC8response, esPrefixPtr);
	}
	else
	{
		//*	if we get here, the data is probably bad.
		strcpy(myPMC8response, pmc8Response);
	}
	ccc				=	0;
	cmdChar1		=	myPMC8response[ccc++];
	cmdChar2		=	myPMC8response[ccc++];
	cmdChar3		=	myPMC8response[ccc++];
	cmdChar4		=	myPMC8response[ccc++];
	cmdPrefix		=	(cmdChar1 << 24)  +
						(cmdChar2 << 16)  +
						(cmdChar3 << 8)  +
						cmdChar4;
	//===============================================================
	//*	the case statements are in alphabetical order to make it easier to find
	switch(cmdPrefix)
	{
		//-------------------------------------------------------------
		//“ESGd” GET AXIS CURRENT DIRECTION VALUE
		//REQUEST: “ESGdA!”
		//RESPONSE: “ESGdAD!”
		//Example: Get Current RA Axis Direction Value
		//REQUEST: “ESGd0!”
		//RESPONSE: “ESGd01!”
		case 'ESGd':
			axisArg			=	myPMC8response[4];
			if (axisArg == '0')
			{
				cPMC8.Direction_RA	=	myPMC8response[45];
			}
			else if (axisArg == '1')
			{
				cPMC8.Direction_DEC	=	myPMC8response[45];
			}
			break;

		case 'ESGi':
			ProcessESGI(myPMC8response);
			break;

		//-------------------------------------------------------------
		//“ESGp” GET AXIS CURRENT POSITION VALUE
		//REQUEST: “ESGpA!”
		//RESPONSE: “ESGpAYYYYYY!”
		//		Example: Get Current RA Axis Position Value
		//		REQUEST: “ESGp0!”
		//		RESPONSE: “ESGp0FF37DA!”
		case 'ESGp':
			axisArg			=	myPMC8response[4];
			parsedValueInt	=	hextoi(&myPMC8response[5]);
			UpdateRaDec(axisArg, parsedValueInt, 2, 0);	//*	2 means rate data is invalid

			break;

		//-------------------------------------------------------------
		//“ESGr” GET AXIS CURRENT RATE VALUE
		//REQUEST: “ESGrA!”
		//RESPONSE: “ESGrAXXXX!”
		//	Example: Get Current RA Axis Rate Value
		//	REQUEST: “ESGr0!”
		//	RESPONSE: “ESGr037DA!”
		case 'ESGr':
//			CONSOLE_DEBUG_W_STR("ESGr             \t=",	myPMC8response);
			axisArg			=	myPMC8response[4];
			parsedValueInt	=	hextoi(&myPMC8response[5]);
			if (parsedValueInt > 50)
			{
				cTelescopeProp.Slewing	=	true;
			}
			if (axisArg == '0')
			{
				ProcessAcceleration_RA(parsedValueInt);
			}
			else if (axisArg == '1')
			{
				ProcessAcceleration_DEC(parsedValueInt);
			}
			else
			{
				CONSOLE_DEBUG("Invalid axis!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
			}
			//*	now figure out what type of operation we are doing
			if ((cAxisRate_RA == 0) && (cAxisRate_DEC == 0))
			{
				cTelescopeProp.Slewing	=	false;
				cTelescopeProp.Tracking	=	false;
			}
			else if ((cAxisRate_RA > 100) || (cAxisRate_DEC > 100))
			{
				cTelescopeProp.Slewing	=	true;
				cTelescopeProp.Tracking	=	false;
			}
			else
			{
				cTelescopeProp.Slewing	=	false;
				cTelescopeProp.Tracking	=	true;
			}
			break;

		//-----------------------------------------------
		//“ESGt” GET AXIS CURRENT TARGET VALUE
		//REQUEST: “ESGtA!”
		//RESPONSE: “ESGtAYYYYYY!”
		//Example: Get Current RA Axis Target Value
		//REQUEST: “ESGt0!”
		//RESPONSE: “ESGt062E4D7!”
		case 'ESGt':
			CONSOLE_DEBUG_W_STR("myPMC8response \t=", myPMC8response);
//			cTelescopeProp.Slewing	=	true;
			break;


		//-----------------------------------------------
		//“ESGv” GET CURRENT FIRMWARE VERSION VALUE
		//REQUEST: “ESGv!”
		//RESPONSE: “ESGvZZZZZZZZ!”
		//		ESGvES20A01.2 Release 2022.06.17
		//		: ES1A02AC20A01, ESP32, DST4!
		case 'ESGv':
			CONSOLE_DEBUG(myPMC8response);
			ccc	=	0;
			iii	=	4;
			while ((myPMC8response[iii] >= 0x20) && (ccc < 60))
			{
				cDeviceFirmwareVersStr[ccc]	=	myPMC8response[iii];
				ccc++;
				iii++;
			}
			cDeviceFirmwareVersStr[ccc]	=	0;

//			CONSOLE_DEBUG(cDeviceFirmwareVersStr);
			break;

		//	ESGw! 0x09     get WiFi channel                      ESGw!
		case 'ESGw':
			cPMC8.WirelessChannel	=	hextoi(&myPMC8response[4]);
			CONSOLE_DEBUG_W_STR("WirelessChannel\t=", myPMC8response);
			CONSOLE_DEBUG_W_NUM("WirelessChannel\t=", cPMC8.WirelessChannel);
			break;



		//-----------------------------------------------
		//Example: Get Current Precision Tracking Rate Value
		//REQUEST: “ESGx!”
		//RESPONSE: “ESGx0535!”
		case 'ESGx':
			CONSOLE_DEBUG(myPMC8response);
			parsedValueInt	=	hextoi(&myPMC8response[4]);
			cTrackingRate	=	parsedValueInt;
			break;

		//ESV!  0xF8     Request motor state vector
		//ESV!  will send state vector [m1D][M1P][M1V][M2D][M2P][M2V] in format [H][HHHHHH][HHHH] for both axes
		//	0123456789 123456789
		//	ESV1000AEA003010000000000!
 		case 'ESV0':
		case 'ESV1':
			{
			int	motorDir;
			int	motorPos;
			int	motorVel;
//				CONSOLE_DEBUG_W_STR("ESV\t=", myPMC8response);

				motorDir	=	myPMC8response[3] & 0x0f;
				motorPos	=	hextoi(&myPMC8response[4], 6);
				motorVel	=	hextoi(&myPMC8response[10], 4);
				UpdateRaDec(0, motorPos, motorDir, motorVel);

				motorDir	=	myPMC8response[14] & 0x0f;
				motorPos	=	hextoi(&myPMC8response[15], 6);
				motorVel	=	hextoi(&myPMC8response[21], 4);
				UpdateRaDec(1, motorPos, motorDir, motorVel);
			}
			break;

		default:
			CONSOLE_DEBUG_W_STR("Not processed\t=", myPMC8response);
			CONSOLE_DEBUG_W_HEX("cmdPrefix    \t=", cmdPrefix);
//			char	tempString[16];
//			int		ccc;
//			ccc	=	0;
//			tempString[ccc++]	=	(cmdPrefix >> 24) & 0x007f;
//			tempString[ccc++]	=	(cmdPrefix >> 16) & 0x007f;
//			tempString[ccc++]	=	(cmdPrefix >> 8) & 0x007f;
//			tempString[ccc++]	=	(cmdPrefix) & 0x007f;
//			tempString[ccc++]	=	0x3f;
//			tempString[ccc++]	=	0;
//			CONSOLE_DEBUG_W_STR("tempString\t=", tempString);
			wasProcessed	=	false;
//			CONSOLE_ABORT(__FUNCTION__);
			break;
	}
	return(wasProcessed);
}

//**************************************************************************************
bool	TelescopeDriverExpSci::SendPMC8command(const char *pmc8command)
{
int		bytesSent;
int		returnByteCNt;
char	returnBuffer[500];
bool	isValid;

	isValid		=	true;
	bytesSent	=	USB_SendCommand(cDeviceConnFileDesc, pmc8command);
	if (bytesSent > 0)
	{
		returnByteCNt	=	ReadUntilChar(cDeviceConnFileDesc, returnBuffer, 100, '!');
		if (returnByteCNt > 0)
		{
//			CONSOLE_DEBUG_W_STR("Return string\t=", returnBuffer);
			ProcessPMC8response(returnBuffer);
		}
		else
		{
			isValid	=	false;
		}
	}
	else
	{
		cUSBxmitErrCnt++;
		isValid	=	false;
	}
	return(isValid);
}

//**************************************************************************************
bool	TelescopeDriverExpSci::SendCmdsPeriodic(void)
{
bool	isValid;

#ifdef _DEBUG_PMC8_
	CONSOLE_DEBUG("=============================================================");
	CONSOLE_DEBUG(__FUNCTION__);
#endif
	isValid	=	true;

	//--------------------------------------------------------------------------
	//*	this gets everything in one command
	SendPMC8command("ESV!");

	//--------------------------------------------------------------------------
	//*	Right Ascension
//	SendPMC8command("ESGp0!");

	//--------------------------------------------------------------------------
	//*	Declination
//	SendPMC8command("ESGp1!");

	//--------------------------------------------------------------------------
	//*	TrackingRates
	cThreadLoopDelay_usec	=	500000;
	if (cMoveAxisMode_RA != kMoveAxisMode_idle)
	{
		//*	if move axis is enabled, we want to get the value and check the limit
//		CONSOLE_DEBUG("sending ESGr0!");
		SendPMC8command("ESGr0!");
		cThreadLoopDelay_usec	=	kAccelerationInterval_microSecs;
	}

	if (cMoveAxisMode_DEC != kMoveAxisMode_idle)
	{
		//*	if move axis is enabled, we want to get the value and check the limit
//		CONSOLE_DEBUG("sending ESGr1!");
		SendPMC8command("ESGr1!");
		cThreadLoopDelay_usec	=	kAccelerationInterval_microSecs;
	}

//	if (cTelescopeProp.Slewing)
//	{
//		DumpPMC8data();
//		CONSOLE_DEBUG_W_NUM("cPMC8.MovementLimit_RA_East\t=",	cPMC8.MovementLimit_RA_East);
//		CONSOLE_DEBUG_W_NUM("cPMC8.MovementLimit_RA_West\t=",	cPMC8.MovementLimit_RA_West);
//	}
	//-----------------------------------------------------------
	//*	are movement limits enabled
	if (cPMC8.EnableMovementLimits)
	{
		if ((cPosition_RA >= cPMC8.MovementLimit_RA_West) || (cPosition_RA <= cPMC8.MovementLimit_RA_East))
		{
			if (cTelescopeProp.Slewing)
			{
				CONSOLE_DEBUG("we have exceeded the limit, stopping slew movement");
				//*	are we in slewing mode
				if (cMoveAxisMode_RA != kMoveAxisMode_idle)
				{
					//*	we have exceeded the limit
					cMoveAxisMode_RA	=	kMoveAxisMode_Decel;
				}
			}
			else if (cAxisRate_RA > 0)
			{
			char	alpacaErrMsg[128];

				CONSOLE_DEBUG("we have exceeded the limit, stopping tracking");
				Telescope_TrackingOnOff(false, alpacaErrMsg);
			}
		}
	}

	return(isValid);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverExpSci::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);

	AddCmdToQueue("ESSr00000!");
	AddCmdToQueue("ESSr10000!");
	cTelescopeProp.Slewing	=	false;
	cMoveAxisMode_RA		=	kMoveAxisMode_idle;
	cMoveAxisMode_DEC		=	kMoveAxisMode_idle;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverExpSci::Telescope_FindHome(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	AddCmdToQueue("ESPt0000000!");
	AddCmdToQueue("ESPt1000000!");
	cTelescopeProp.Slewing	=	true;
	alpacaErrCode			=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
static void	Format6digitHex(int value, char *hexString)
{
char	myHexString[64];
int		sLen;
int		iii;

	sprintf(myHexString, "%06X", value);
	sLen	=	strlen(myHexString);
	while (sLen > 6)
	{
		for (iii=0; iii<sLen; iii++)
		{
			myHexString[iii]	=	myHexString[iii+1];
		}
		sLen	=	strlen(myHexString);
	}
	strcpy(hexString, myHexString);
}

//*****************************************************************************
//*	axisNum
//			axisPrimary		0	Primary axis (e.g., Right Ascension or Azimuth).
//			axisSecondary	1	Secondary axis (e.g., Declination or Altitude).
//			axisTertiary	2	Tertiary axis (e.g. imager rotator/de-rotator).
//
//*	moveRate_degPerSec	The rate of motion (deg/sec) about the specified axis
//*
//*	https://ascom-standards.org/Help/Platform/html/M_ASCOM_DeviceInterface_ITelescopeV3_MoveAxis.htm
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverExpSci::Telescope_MoveAxis(	const int		axisNum,
																const double	moveRate_degPerSec,
																char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				esCmdString[32];
double				arcSecondsPerSec;
double				microStepRate;
int					microStepRateValueLimit;
int					microStepRateValueStart;

	CONSOLE_DEBUG_W_BOOL("On start, cTelescopeProp.Slewing\t=",	cTelescopeProp.Slewing);

	arcSecondsPerSec		=	fabs(moveRate_degPerSec) * 3600.0;
	microStepRate			=	arcSecondsPerSec / cPMC8.ArcSecPerMicroStep;
	microStepRateValueLimit	=	microStepRate;
	microStepRateValueStart	=	kMaxStepRate / 10;
	CONSOLE_DEBUG_W_DBL("moveRate_degPerSec     \t=",	moveRate_degPerSec)
	CONSOLE_DEBUG_W_DBL("arcSecondsPerSec       \t=",	arcSecondsPerSec)
	CONSOLE_DEBUG_W_DBL("microStepRate          \t=",	microStepRate)
	CONSOLE_DEBUG_W_NUM("microStepRateValueLimit\t=",	microStepRateValueLimit)
	if (microStepRateValueLimit > kMaxStepRate)
	{
		microStepRateValueLimit	=	kMaxStepRate;
		CONSOLE_DEBUG_W_NUM("microStepRateValueLimit\t=",	microStepRateValueLimit)
	}

	switch(axisNum)
	{
		case 0:	//	0	= RA Axis
			cTelescopeProp.Slewing	=	true;
			cMoveAxisLimit_RA		=	microStepRateValueLimit;
			if (moveRate_degPerSec > 0.0)
			{
				cMoveAxisMode_RA		=	kMoveAxisMode_Accel;
				AddCmdToQueue("ESSd01!");
			}
			else if (moveRate_degPerSec < 0.0)
			{
				cMoveAxisMode_RA		=	kMoveAxisMode_Accel;
				AddCmdToQueue("ESSd00!");
			}
			else if (cMoveAxisMode_RA != kMoveAxisMode_idle)
			{
				CONSOLE_DEBUG("Setting to kMoveAxisMode_Decel ");
				cMoveAxisMode_RA		=	kMoveAxisMode_Decel;
				cMoveAxisLimit_RA		=	0;
				microStepRateValueStart	=   cAxisRate_RA - kAccelerationAmount;
			}
			sprintf(esCmdString, "ESSr0%04X!", microStepRateValueStart);
			cMoveAxisCurrentStepRate_RA	=	microStepRateValueStart;
			cMoveAxisLastMilliSecs_RA	=	millis();
			AddCmdToQueue(esCmdString);
			break;

		case 1:
			cTelescopeProp.Slewing	=	true;
			cMoveAxisLimit_DEC		=	microStepRateValueLimit;
			if (moveRate_degPerSec > 0.0)
			{
				cMoveAxisMode_DEC		=	kMoveAxisMode_Accel;
				AddCmdToQueue("ESSd11!");
			}
			else if (moveRate_degPerSec < 0.0)
			{
				cMoveAxisMode_DEC		=	kMoveAxisMode_Accel;
				AddCmdToQueue("ESSd10!");
			}
			else if (cMoveAxisMode_DEC != kMoveAxisMode_idle)
			{
				CONSOLE_DEBUG("Setting to kMoveAxisMode_Decel ");
				cMoveAxisMode_DEC		=	kMoveAxisMode_Decel;
				cMoveAxisLimit_DEC		=	0;
				microStepRateValueStart	=   cAxisRate_DEC - kAccelerationAmount;
			}
			sprintf(esCmdString, "ESSr1%04X!", microStepRateValueStart);
			cMoveAxisCurrentStepRate_DEC	=	microStepRateValueStart;
			cMoveAxisLastMilliSecs_DEC		=	millis();
			AddCmdToQueue(esCmdString);
			break;

		default:
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
			break;
	}
	CONSOLE_DEBUG_W_BOOL("On exit, cTelescopeProp.Slewing\t=",	cTelescopeProp.Slewing);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverExpSci::Telescope_Park(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	AddCmdToQueue("ESPt0000000!");
	AddCmdToQueue("ESPt1000000!");
	cTelescopeProp.Slewing	=	true;
	cTelescopeProp.AtPark	=	true;

	alpacaErrCode			=	kASCOM_Err_Success;
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverExpSci::Telescope_SetPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	//				“ESSpAYYYYYY!”
	AddCmdToQueue(	"ESSp0000000!");
	AddCmdToQueue(	"ESSp1000000!");
	alpacaErrCode		=	kASCOM_Err_Success;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverExpSci::Telescope_SlewToRA_DEC(	const double	newRtAscen_Hours,
																	const double	newDeclination_Degrees,
																	char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
//char				timeString[32];
//char				commandString[48];
int					motorCount_DEC;
int					motorCount_RA;
char				hexString[64];
char				esCmdString[64];

	CONSOLE_DEBUG(__FUNCTION__);
	cTelescopeProp.SideOfPier	=	Telescope_GetPhysicalSideOfPier();
	motorCount_RA	=	RA_to_MotorCounts(newRtAscen_Hours,			cTelescopeProp.SideOfPier);
	motorCount_DEC	=	DEC_to_MotorCounts(newDeclination_Degrees,	cTelescopeProp.SideOfPier);

	CONSOLE_DEBUG_W_STR("SideOfPier            \t=",	((cTelescopeProp.SideOfPier == kPierSide_pierEast) ? "East" : "West") );
	CONSOLE_DEBUG_W_DBL("newRtAscen_Hours      \t=",	newRtAscen_Hours);
	CONSOLE_DEBUG_W_DBL("newDeclination_Degrees\t=",	newDeclination_Degrees);
	CONSOLE_DEBUG_W_NUM("motorCount_RA         \t=",	motorCount_RA);
	CONSOLE_DEBUG_W_NUM("motorCount_DEC        \t=",	motorCount_DEC);

	//REQUEST: “ESPtAYYYYYY!”
	Format6digitHex(motorCount_RA, hexString);
	sprintf(esCmdString, "ESPt0%s!", hexString);
	if (strlen(esCmdString) != 12)
	{
		CONSOLE_DEBUG(esCmdString);
		CONSOLE_DEBUG_W_SIZE("Strlen=\t", strlen(esCmdString));
		CONSOLE_ABORT(__FUNCTION__);
	}
	AddCmdToQueue(esCmdString);

	Format6digitHex(motorCount_DEC, hexString);
	sprintf(esCmdString, "ESPt1%s!", hexString);
	if (strlen(esCmdString) != 12)
	{
		CONSOLE_DEBUG(esCmdString);
		CONSOLE_DEBUG_W_SIZE("Strlen=\t", strlen(esCmdString));
		CONSOLE_ABORT(__FUNCTION__);
	}
	AddCmdToQueue(esCmdString);

//	CONSOLE_ABORT(__FUNCTION__);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverExpSci::Telescope_SyncToRA_DEC(	const double	newRtAscen_Hours,
																	const double	newDeclination_Degrees,
																	char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
//char				timeString[32];
//char				commandString[48];

//	FormatHHMMSS(newRtAscen_Hours, timeString, false);
//


	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverExpSci::Telescope_TrackingOnOff(	const bool	newTrackingState,
																	char		*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
TYPE_DriveRates		myTrackingRateIdx;

	CONSOLE_DEBUG(__FUNCTION__);
	if (newTrackingState)
	{
		myTrackingRateIdx		=	cTelescopeProp.TrackingRate;
		if ((myTrackingRateIdx < 0) || (myTrackingRateIdx >= kDriveRate_Count))
		{
			myTrackingRateIdx	=   kDriveRate_driveSidereal;
		}
		cTelescopeProp.Tracking	=	true;
		alpacaErrCode			=	Telescope_TrackingRate(myTrackingRateIdx, alpacaErrMsg);;
	}
	else
	{
		//*	Set Precision Tracking Rate
		AddCmdToQueue("ESTr0000!");
		cTelescopeProp.Slewing	=	false;
		cTelescopeProp.Tracking	=	false;
		alpacaErrCode			=	kASCOM_Err_Success;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverExpSci::Telescope_TrackingRate(	TYPE_DriveRates	newTrackingRate,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
char				esCmdString[32];
int					esTrackingValue;
TYPE_DriveRates		myTrackingRateIdx;

//	CONSOLE_DEBUG(__FUNCTION__);
	myTrackingRateIdx		=	cTelescopeProp.TrackingRate;
	if ((myTrackingRateIdx < 0) || (myTrackingRateIdx >= kDriveRate_Count))
	{
		//*	default to sidereal
		myTrackingRateIdx	=   kDriveRate_driveSidereal;
	}
	cTelescopeProp.TrackingRate	=	myTrackingRateIdx;
	esTrackingValue				=	cPMC8.TrackingRate_value[myTrackingRateIdx];
	alpacaErrCode				=	kASCOM_Err_Success;

	sprintf(esCmdString, "ESTr%04X!", esTrackingValue);
	AddCmdToQueue(esCmdString);

//	CONSOLE_DEBUG_W_STR("esCmdString\t=", esCmdString);

	return(alpacaErrCode);
}

//**************************************************************************************
//*	from Driver.vb dated 29-MAR-2024  WEM
//**************************************************************************************
//Public Property SideOfPier() As PierSide Implements ITelescopeV3.SideOfPier
//
//        Get
//            Dim DECMP As Int32
//            Dim SOP As PierSide
//
//            If Telescope.SiteLatitudeValue >= 0 Then
//                DECMP = GetDECMotorPosition()
//                If DECMP > 0 Then       'used to be >= changed to try to pass conformu.
//                    SOP = PierSide.pierWest
//                    MountPierSide = PierSide.pierWest
//                ElseIf DECMP <= 0 Then     'used to be < ony changed to pass conform U
//                    SOP = PierSide.pierEast
//                    MountPierSide = PierSide.pierEast
//                End If
//            ElseIf Telescope.SiteLatitudeValue < 0 Then
//                DECMP = GetDECMotorPosition()
//                If DECMP >= 0 Then
//                    SOP = PierSide.pierEast
//                    MountPierSide = PierSide.pierEast
//                ElseIf DECMP < 0 Then
//                    SOP = PierSide.pierWest
//                    MountPierSide = PierSide.pierWest
//                End If
//            End If
//            'DECMP = GetDECMotorPosition()
//
//            Return SOP
//        End Get
//**************************************************************************************
TYPE_PierSide	TelescopeDriverExpSci::Telescope_CalculateSideOfPier(void)
{
TYPE_PierSide	sop;

	if (gObseratorySettings.Latitude_deg >= 0.0)
	{
		if (cPosition_DEC > 0)		//	used to be >= changed to try to pass conformu.
		{
			sop	=   kPierSide_pierWest;
		}
//		else if (cPosition_DEC <= 0)	//	used to be < ony changed to pass conform U
		else
		{
			sop	=   kPierSide_pierEast;
		}
	}
	else if (gObseratorySettings.Latitude_deg < 0.0)
	{
		if (cPosition_DEC >= 0)
		{
			sop	=   kPierSide_pierEast;
		}
//		else if (cPosition_DEC < 0)
		else
		{
			sop	=   kPierSide_pierWest;
		}
	}

	return(sop);
}

//*****************************************************************************
TYPE_PierSide	TelescopeDriverExpSci::Telescope_GetPhysicalSideOfPier(void)
{
TYPE_PierSide	physicalSideOfPier;

	if (cPosition_RA >= 0)
	{
		physicalSideOfPier	=	kPierSide_pierWest;
	}
	else
	{
		physicalSideOfPier	=	kPierSide_pierEast;
	}
	return(physicalSideOfPier);
}

//*****************************************************************************
void	TelescopeDriverExpSci::DumpPMC8data(void)
{
	CONSOLE_DEBUG("-----------------------------------------");

	CONSOLE_DEBUG_W_LONG(	"ThreadLoopCnt   \t=",	cDriverThreadLoopCnt);
	CONSOLE_DEBUG_W_STR(	"FirmWare Version\t=",	cDeviceFirmwareVersStr);
	CONSOLE_DEBUG_W_NUM(	"cPosition_RA    \t=",	cPosition_RA);
	CONSOLE_DEBUG_W_NUM(	"cPosition_DEC   \t=",	cPosition_DEC);
	CONSOLE_DEBUG_W_NUM(	"cAxisRate_RA    \t=",	cAxisRate_RA);
	CONSOLE_DEBUG_W_NUM(	"cAxisRate_DEC   \t=",	cAxisRate_DEC);
	CONSOLE_DEBUG_W_HEX(	"cTrackingRate   \t=",	cTrackingRate);
	CONSOLE_DEBUG_W_NUM(	"cFormatErrCnt   \t=",	cFormatErrCnt);
	CONSOLE_DEBUG_W_NUM(	"cUSBxmitErrCnt  \t=",	cUSBxmitErrCnt);

}

//*****************************************************************************
void	TelescopeDriverExpSci::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
char		lineBuff[512];
char		valueBuff[64];
int			iii;
const char	*trackRateNames[]	=	{"Sidereal", "Lunar", "Solar", "King"};


	//	CONSOLE_DEBUG(__FUNCTION__);


	SocketWriteData(reqData->socket,	"<HR><CENTER>\r\n");
	SocketWriteData(reqData->socket,	"<H2>Explore Scientific mount parameters</H2><BR>\r\n");
	SocketWriteData(reqData->socket,	"<TABLE BORDER=1>\r\n");

	SocketWriteData(reqData->socket,	"\t<TR><TD>Mount Type</TD>\r\n");
	strcpy(lineBuff, "\t<TD>");

	strcat(lineBuff, cPMC8.ModelName);

	strcat(lineBuff, "</TD></TR>\r\n");
	SocketWriteData(reqData->socket,	lineBuff);


	sprintf(lineBuff,	"\t<TR><TD>Arc Secs / Micro Step</TD><TD>%6.5f</TD></TR>\r\n",	cPMC8.ArcSecPerMicroStep);
	SocketWriteData(reqData->socket,	lineBuff);

	sprintf(lineBuff,	"\t<TR><TD>Steps per 360<SUP>&deg;</SUP> RA</TD><TD>%d</TD></TR>\r\n",	cPMC8.StepsPer360_RA);
	SocketWriteData(reqData->socket,	lineBuff);

	sprintf(lineBuff,	"\t<TR><TD>Steps per 360<SUP>&deg;</SUP> DEC</TD><TD>%d</TD></TR>\r\n",	cPMC8.StepsPer360_DEC);
	SocketWriteData(reqData->socket,	lineBuff);
	SocketWriteData(reqData->socket,	"</TABLE>\r\n");

	//=========================================================================================
	SocketWriteData(reqData->socket,	"<TABLE BORDER=1>\r\n");

	SocketWriteData(reqData->socket,	"<TR><TH>Rate</TH>");
	SocketWriteData(reqData->socket,	"<TH>ASCOM</TH></TD>");
	SocketWriteData(reqData->socket,	"<TH>Sidereal</TH>");
	SocketWriteData(reqData->socket,	"<TH>Micro Steps/Sec</TH>");
	SocketWriteData(reqData->socket,	"<TH>Value</TH>");
	SocketWriteData(reqData->socket,	"</TR>\r\n");
	SocketWriteData(reqData->socket,	"<TR><TH></TH><TH COLSPAN=2>arc-seconds / second</TH><TH></TH><TH></TH></TR>\r\n");
	for (iii=0; iii<kDriveRate_Count; iii++)
	{
		strcpy(lineBuff,	"\t<TR>");
		sprintf(valueBuff,	"<TD><CENTER>%s</TD>",		trackRateNames[iii]);
		strcat(lineBuff, valueBuff);

		sprintf(valueBuff,	"<TD><CENTER>%5.4f</TD>",	cPMC8.TrackingRate_ASCOM[iii]);
		strcat(lineBuff, valueBuff);

		sprintf(valueBuff,	"<TD><CENTER>%5.4f</TD>",	cPMC8.TrackingRate_Sidereal[iii]);
		strcat(lineBuff, valueBuff);

		sprintf(valueBuff,	"<TD><CENTER>%5.4f</TD>",	cPMC8.TrackingRate_MicroStep[iii]);
		strcat(lineBuff, valueBuff);

		sprintf(valueBuff,	"<TD><CENTER>0x%04X</TD>",	cPMC8.TrackingRate_value[iii]);
		strcat(lineBuff, valueBuff);
		strcat(lineBuff,	"</TR>\r\n");
		SocketWriteData(reqData->socket,	lineBuff);
	}
	SocketWriteData(reqData->socket,	"<TR><TD COLSPAN=5><CENTER>Note: PMC8 using Sidereal time internally</TD></TR>\r\n");
	SocketWriteData(reqData->socket,	"</TABLE>\r\n");
	//=========================================================================================
	//#define		kAccelerationSeconds			2
	//#define		kAccelerationSteps				100
	//
	//#define		kAccelerationInterval_milliSecs	((kAccelerationSeconds * 1000) / kAccelerationSteps)
	//#define		kAccelerationInterval_microSecs	((kAccelerationSeconds * 1000000) / kAccelerationSteps)
	//#define		kMaxStepRate					40000
	//#define		kAccelerationAmount				(kMaxStepRate / kAccelerationSteps)
	SocketWriteData(reqData->socket,	"<P>\r\n");
	SocketWriteData(reqData->socket,	"<TABLE BORDER=1>\r\n");
	SocketWriteData(reqData->socket,	"<TR><TH COLSPAN=2>MoveAxis Acceleration parameters</TH></TR>\r\n");

	SocketWriteData(reqData->socket,	"<TR><TD>Seconds for ramp up/down</TD>\r\n");
	sprintf(lineBuff,	"<TD><CENTER>%d</TD>",	kAccelerationSeconds);
	strcat(lineBuff,	"</TR>\r\n");
	SocketWriteData(reqData->socket,	lineBuff);

	SocketWriteData(reqData->socket,	"<TR><TD>Steps for ramp up/down</TD>\r\n");
	sprintf(lineBuff,	"<TD><CENTER>%d</TD>",	kAccelerationSteps);
	strcat(lineBuff,	"</TR>\r\n");
	SocketWriteData(reqData->socket,	lineBuff);


	SocketWriteData(reqData->socket,	"</TABLE>\r\n");

	SocketWriteData(reqData->socket,	"</CENTER>\r\n");
}

//*****************************************************************************
//*	https://www.w3schools.com/html/html_forms.asp
//*****************************************************************************
bool	TelescopeDriverExpSci::Setup_OutputForm(TYPE_GetPutRequestData *reqData, const char *formActionString)
{
int			mySocketFD;
int			iii;
char		lineBuff[512];
char		valueBuff[256];
const char	webPageTitle[]		=	"Explore Scientific Mount setup";
const char	*trackRateNames[]	=	{"Sidereal", "Lunar", "Solar", "King"};

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("The Action that will be preformed when save is pressed:", formActionString);

	mySocketFD	=	reqData->socket;


	SocketWriteData(mySocketFD,	gHtmlHeader_html);

	SocketWriteData(mySocketFD,	"<!DOCTYPE html>\r\n");
	SocketWriteData(mySocketFD,	"<HTML lang=\"en\">\r\n");
	sprintf(lineBuff,			"<TITLE>%s</TITLE>\r\n", webPageTitle);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	sprintf(lineBuff,			"<H1>%s</H1>\r\n", webPageTitle);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");

	sprintf(lineBuff, "<form action=\"%s\">\r\n", formActionString);
	SocketWriteData(mySocketFD,	lineBuff);

//
//
//	SocketWriteData(mySocketFD,	"<TR>\r\n");
//	SocketWriteData(mySocketFD,	"<TD>\r\n");
//
//	SocketWriteData(mySocketFD,	"<label for=\"name\">Name:</label><br>\r\n");
//	SocketWriteData(mySocketFD,	"<input type=\"text\" id=\"name\" name=\"name\" value=\"John Doe\"><br>\r\n");
//	SocketWriteData(mySocketFD,	"</TD>\r\n");

	//*	outside table
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
	SocketWriteData(mySocketFD,	"\t<TR><TD><CENTER>\r\n");

	//*	inside table, 1st of several
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

	//------------------------------------------------------------------
	SocketWriteData(mySocketFD,	"\t<TR><TD>Mount Type</TD>\r\n");
	strcpy(lineBuff, "\t<TD>");
	strcat(lineBuff, cPMC8.ModelName);
	strcat(lineBuff, "</TD></TR>\r\n");
	SocketWriteData(mySocketFD,	lineBuff);

	sprintf(lineBuff,	"\t<TR><TD>COM port</TD><TD>%s</TD></TR>",	cDeviceConnPath);
	SocketWriteData(mySocketFD,	lineBuff);

	sprintf(lineBuff,	"\t<TR><TD>Baud rate</TD><TD>%d</TD></TR>",	cPMC8.BaudRate);
	SocketWriteData(mySocketFD,	lineBuff);

	sprintf(lineBuff,	"\t<TR><TD>Wifi Type</TD><TD>%s</TD></TR>",	cPMC8.WifiTypeStr);
	SocketWriteData(mySocketFD,	lineBuff);


	//------------------------------------------------------------------
	sprintf(lineBuff,	"\t<TR><TD>Arc Secs / Micro Step</TD><TD>%6.5f</TD></TR>\r\n",	cPMC8.ArcSecPerMicroStep);
	SocketWriteData(mySocketFD,	lineBuff);

	sprintf(lineBuff,	"\t<TR><TD>Steps per 360<SUP>&deg;</SUP> RA</TD><TD>%d</TD></TR>\r\n",	cPMC8.StepsPer360_RA);
	SocketWriteData(mySocketFD,	lineBuff);

	sprintf(lineBuff,	"\t<TR><TD>Steps per 360<SUP>&deg;</SUP> DEC</TD><TD>%d</TD></TR>\r\n",	cPMC8.StepsPer360_DEC);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"</TABLE>\r\n");


	//=========================================================================================
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

	SocketWriteData(mySocketFD,	"<TR><TD>\r\n");

	Setup_OutputCheckBox(mySocketFD,	"wifienable",		"ESP WiFiMoudle UDP/IP enabled",	cPMC8.TCP_UDP);
	Setup_OutputCheckBox(mySocketFD,	"trackcomm",		"TRACK on Loss of COMMS",		cPMC8.TrackAlways);
	Setup_OutputCheckBox(mySocketFD,	"trackboot",		"TRACK on Boot",				cPMC8.TrackOnBoot);
	Setup_OutputCheckBox(mySocketFD,	"norhthemisphere",	"Northern Hemisphere",			cPMC8.NorthernHemisphere);
	Setup_OutputCheckBox(mySocketFD,	"autoguid",			"Auto Guider Enabled",			cPMC8.AutoGuiderEnabled);
	SocketWriteData(mySocketFD,	"</TD></TR>\r\n");


	//=========================================================================================
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TD>\r\n");

	Setup_OutputNumberBox(mySocketFD,	"wifichan",			"WiFi Channel:",						cPMC8.WifiChannel);
	Setup_OutputNumberBox(mySocketFD,	"siderealfracRA",	"RA Sidereal Rate Fraction (1-100)",	cPMC8.SiderealRateFrac_RA);
	Setup_OutputNumberBox(mySocketFD,	"siderealfracDEC",	"DEC Sidereal Rate Fraction (1-100)",	cPMC8.SiderealRateFrac_DEC);
	Setup_OutputNumberBox(mySocketFD,	"slewcurrent",		"SLEW Motor Current Value (mA)",		cPMC8.MotorCurrentSlew);
	Setup_OutputNumberBox(mySocketFD,	"trackcurrent",		"TRACK Motor Current Value (mA)",		cPMC8.MotorCUrrentTrack);

	SocketWriteData(mySocketFD,	"</TD>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");
	SocketWriteData(mySocketFD,	"</TABLE>\r\n");

	//=========================================================================================
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
	SocketWriteData(mySocketFD,	"<TR><TD COLSPAN=2><CENTER>\r\n");
	Setup_OutputCheckBox(mySocketFD,	"travelLimit",		"Enable Travel Limits",		cPMC8.EnableMovementLimits);
	SocketWriteData(mySocketFD,	"</TD></TR>\r\n");

	SocketWriteData(mySocketFD,	"<TR>\r\n");


	SocketWriteData(mySocketFD,	"<TD><CENTER>\r\n");
	Setup_OutputNumberBox(mySocketFD,	"limitWest",		"West Limit",		cPMC8.MovementLimit_RA_West, 1);
	SocketWriteData(mySocketFD,	"</TD>\r\n");

	SocketWriteData(mySocketFD,	"<TD><CENTER>\r\n");
	Setup_OutputNumberBox(mySocketFD,	"limitEast",		"East Limit",		cPMC8.MovementLimit_RA_East, 1);
	SocketWriteData(mySocketFD,	"</TD>\r\n");

	SocketWriteData(mySocketFD,	"</TR>\r\n");


	SocketWriteData(mySocketFD,	"</TABLE>\r\n");


	//=========================================================================================
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

	SocketWriteData(mySocketFD,	"<TR><TH>Rate</TH>");
	SocketWriteData(mySocketFD,	"<TH>ASCOM</TH></TD>");
	SocketWriteData(mySocketFD,	"<TH>Sidereal</TH>");
	SocketWriteData(mySocketFD,	"<TH>Micro Steps/Sec</TH>");
	SocketWriteData(mySocketFD,	"<TH>Value</TH>");
	SocketWriteData(mySocketFD,	"</TR>\r\n");
	SocketWriteData(mySocketFD,	"<TR><TH></TH><TH COLSPAN=2>arc-seconds / second</TH><TH></TH><TH></TH></TR>\r\n");
	for (iii=0; iii<kDriveRate_Count; iii++)
	{
		strcpy(lineBuff,	"\t<TR>");
		sprintf(valueBuff,	"<TD><CENTER>%s</TD>",		trackRateNames[iii]);
		strcat(lineBuff, valueBuff);

		sprintf(valueBuff,	"<TD><CENTER>%5.4f</TD>",	cPMC8.TrackingRate_ASCOM[iii]);
		strcat(lineBuff, valueBuff);

		sprintf(valueBuff,	"<TD><CENTER>%5.4f</TD>",	cPMC8.TrackingRate_Sidereal[iii]);
		strcat(lineBuff, valueBuff);

		sprintf(valueBuff,	"<TD><CENTER>%5.4f</TD>",	cPMC8.TrackingRate_MicroStep[iii]);
		strcat(lineBuff, valueBuff);

		sprintf(valueBuff,	"<TD><CENTER>0x%04X</TD>",	cPMC8.TrackingRate_value[iii]);
		strcat(lineBuff, valueBuff);
		strcat(lineBuff,	"</TR>\r\n");
		SocketWriteData(mySocketFD,	lineBuff);
	}
	SocketWriteData(mySocketFD,	"<TR><TD COLSPAN=5><CENTER>Note: PMC8 using Sidereal time internally</TD></TR>\r\n");
	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	//=========================================================================================
	//#define		kAccelerationSeconds			2
	//#define		kAccelerationSteps				100
	//
	//#define		kAccelerationInterval_milliSecs	((kAccelerationSeconds * 1000) / kAccelerationSteps)
	//#define		kAccelerationInterval_microSecs	((kAccelerationSeconds * 1000000) / kAccelerationSteps)
	//#define		kMaxStepRate					40000
	//#define		kAccelerationAmount				(kMaxStepRate / kAccelerationSteps)
	SocketWriteData(mySocketFD,	"<P>\r\n");
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
	SocketWriteData(mySocketFD,	"<TR><TH COLSPAN=2>MoveAxis Acceleration parameters</TH></TR>\r\n");

	SocketWriteData(mySocketFD,	"<TR><TD>Seconds for ramp up/down</TD>\r\n");
	sprintf(lineBuff,	"<TD><CENTER>%d</TD>",	kAccelerationSeconds);
	strcat(lineBuff,	"</TR>\r\n");
	SocketWriteData(mySocketFD,	lineBuff);

	SocketWriteData(mySocketFD,	"<TR><TD>Steps for ramp up/down</TD>\r\n");
	sprintf(lineBuff,	"<TD><CENTER>%d</TD>",	kAccelerationSteps);
	strcat(lineBuff,	"</TR>\r\n");
	SocketWriteData(mySocketFD,	lineBuff);


	SocketWriteData(mySocketFD,	"</TABLE>\r\n");

	//*	close outer table
	SocketWriteData(mySocketFD,	"</CENTER></TD>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TD><CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<input type=\"reset\" value=\"Reset\">\r\n");
	SocketWriteData(mySocketFD,	"<input type=\"submit\" value=\"Save\">\r\n");
	SocketWriteData(mySocketFD,	"</TD>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");
	SocketWriteData(mySocketFD,	"</TABLE>\r\n");

	SocketWriteData(mySocketFD,	"</CENTER>\r\n");


	SocketWriteData(mySocketFD,	"</form>\r\n");
	return(true);
}

//*****************************************************************************
void	TelescopeDriverExpSci::Setup_SaveInit(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	//*	set all of the filename options to false
//	memset(&cSetupFileNameOptions, 0, sizeof(TYPE_FilenameOptions));
}

//*****************************************************************************
void	TelescopeDriverExpSci::Setup_SaveFinish(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
//wifienable, true
//trackcomm, true
//norhthemisphere, true
//wifichan, 6
//siderealfacRA, 40
//siderealfacDEC, 40
//slewcurrent, 900
//trackcurrent, 600
//travelLimit, true
//limitWest, 1036800
//limitEast, -1036800


//*****************************************************************************
bool	TelescopeDriverExpSci::Setup_ProcessKeyword(const char *keyword, const char *valueString)
{

	CONSOLE_DEBUG_W_2STR("kw:value", keyword, valueString);


	return(true);
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
static int	hextoi(const char *hexString, int length)
{
char	myHexString[32];
int		intValue;

	strcpy(myHexString, hexString);
	myHexString[length]	=	0;
	intValue		=	hextoi(myHexString);
	return(intValue);
}


#endif // _ENABLE_TELESCOPE_EXP_SCI_
