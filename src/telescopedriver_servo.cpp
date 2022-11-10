//**************************************************************************
//*	Name:			telescopedriver_servo.cpp
//*
//*	Author:			Mark Sproul and Ron Story (C) 2021
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*					This driver implements an Alpaca Telescope
//*					talking to an LX200 compatible mount
//*					via ethernet, USB or serial port
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
//*	Mar 25,	2022	<MLS> Created telescopedriver_Servo.cpp
//*	Mar 25,	2022	<MLS> Servo version is being created for Ron S <RNS>
//*	Apr  6,	2022	<MLS> Switched to using generic config reading routines
//*	May 11,	2022	<MLS> Now using servo config processing routines by <RNS>
//*	May 11,	2022	<MLS> Added OutputHTML_Part2() to telescope servo driver
//*	May 19,	2022	<MLS> Updated #include to reflect RNS filename changes
//*	May 21,	2022	<RNS> First pass of adding the functions needed from servo libs
//*	May 22,	2022	<RNS> ID'd the missing functionality added to servo libs
//*	May 22,	2022	<RNS> Adding the last of the functions, first pass review looks OK
//*	May 26,	2022	<MLS> Added html output of current configuration
//*	Jun 17,	2022	<RNS> Updated Servo_stop and start_tracking to use new args
//*	Jun 17,	2022	<RNS> cleaned up the #warning notes
//*	Jun 19,	2022	<RNS> Fixed curr position getting set to HA and not RA=LST-HA
//*	Jun 19,	2022	<RNS> Added .AtPark assignments to support unpark
//*	Jun 19,	2022	<RNS> Toggled RA direction for _MoveAxis
//*	Jun 20,	2022	<MLS> Added UpdateSlewingProperty()
//*	Jul 16,	2022	<RNS> Reviews MLS edits and corrected as needed
//*	Jul 17,	2022	<RNS> Simplified LST calcs using Servo_get_lst()
//*	Jul 18,	2022	<RNS> Fixed the mount settings move-by-button functionality
//*	Jul 20,	2022	<RNS> Fixed a hour/degs conversion error in SlewToRA_DEC
//*	Jul 20,	2022	<RNS> Removed LST calcs, not needed afterall
//*	Nov  9,	2022	<RNS> Fixed typos in some comments
//*****************************************************************************
//*	Roboclaw MC servo support
//*****************************************************************************

#ifdef _ENABLE_TELESCOPE_SERVO_

#include	<ctype.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<math.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"


#include	"servo_mount_cfg.h"
#include	"servo_time.h"
#include	"servo_std_defs.h"
#include	"servo_motion.h"
#include	"servo_motion_cfg.h"
#include	"servo_observ_cfg.h"

//#define	_DEBUG_WITHOUT_ROBOCLAW_
#ifdef _DEBUG_WITHOUT_ROBOCLAW_
	#define	Servo_set_pos(x,y)				(kSTATUS_OK)
	#define	Servo_move_axis_by_vel(x,y)		(kSTATUS_OK)
	#define	Servo_get_park_coordins(x,y)	(kSTATUS_OK)
	#define	Servo_get_pos(x,y)				(kSTATUS_OK)
	#define	Servo_unpark()					(kSTATUS_OK)
	#define	Servo_state()					(STOPPED)
	#define	Servo_move_to_park()			(kSTATUS_OK)
	#define	Servo_get_pier_side()			(kSTATUS_OK)
	#define	Servo_init(x,y)					(kSTATUS_OK)

	#define	Servo_start_tracking(x)			(kSTATUS_OK)
	#define	Servo_stop(x)					(kSTATUS_OK)
	#define	Servo_move_to_coordins(x,y,z,q)	(kSTATUS_OK)

//****************************************************************************
typedef enum
{
	PARKED 	=	0,
	PARKING,
//	HOMED,
//	HOMING,
	STOPPED,
	MOVING,
	TRACKING
}	TYPE_MOVE;

#else
	//*	servo controller routines using RoboClaws controller
	#include	"servo_mount.h"
#endif // _DEBUG_WITHOUT_ROBOCLAW_

#include	"telescopedriver.h"
#include	"telescopedriver_servo.h"


//**************************************************************************************
TelescopeDriverServo::TelescopeDriverServo(void)
	: TelescopeDriver()
{
int				cfgStatus;
int8_t			servoSide;
double			parkRa, parkDec;

	CONSOLE_DEBUG(__FUNCTION__);

	cVerboseDebug	=	true;

	strcpy(cCommonProp.Name, "AlpacaPi-Mount-Servo");
	strcpy(cCommonProp.Description, "Mount using RoboClaw servo controller");

	//*	setup the options for this driver
	cTelescopeProp.AlginmentMode			=	kAlignmentMode_algGermanPolar;
	cTelescopeProp.CanSlewAsync				=	true;
	cTelescopeProp.CanSync					=	true;
	cTelescopeProp.CanSetTracking			=	true;
	cTelescopeProp.CanMoveAxis[kAxis_RA]	=	true;
	cTelescopeProp.CanMoveAxis[kAxis_DEC]	=	true;
	cTelescopeProp.CanUnpark				=	true;
	cTelescopeProp.CanPark					=	true;
	cTelescopeProp.CanFindHome				=	false;

	cServoConfigIsValid	=	false;
	cfgStatus			=	Servo_init(kOBSERV_CFG_FILE, kMOUNT_CFG_FILE, kMOTION_CFG_FILE);
	if (cfgStatus == kSTATUS_OK)
	{
		cServoConfigIsValid	=	true;
	}
	CONSOLE_DEBUG_W_NUM("cfgStatus\t=", cfgStatus);
	CONSOLE_DEBUG_W_BOOL("cServoConfigIsValid\t=", cServoConfigIsValid);

	//	<RNS> additions
	//	Update the side of pier
	servoSide	=	Servo_get_pier_side();
	switch (servoSide)
	{
		case kEAST:
			cTelescopeProp.SideOfPier	=	kPierSide_pierEast;
			break;

		case kWEST:
			cTelescopeProp.SideOfPier	=	kPierSide_pierWest;
			break;

		default:
			cTelescopeProp.SideOfPier	=	kPierSide_pierUnknown;
			break;
	}
	cTelescopeProp.TrackingRate		=	kDriveRate_driveSidereal;

	//*	check to see if the observatory settings is valid
	if (gObservatorySettingsOK)
	{
		//*	we have settings from observatorysettings.txt config file
		//*	these override the "servo_location.cfg" file
		Servo_set_lat(cTelescopeProp.SiteLatitude);
		Servo_set_lon(cTelescopeProp.SiteLongitude);
		Servo_set_elev(cTelescopeProp.SiteElevation);
	}
	else
	{
		cTelescopeProp.SiteLatitude		=	Servo_get_lat();
		cTelescopeProp.SiteLongitude	=	Servo_get_lon();
		cTelescopeProp.SiteElevation	=	Servo_get_elev();
	}

	//	Servo_init() always sets the mount to the park position and _get_pos returns RA/De (not HA)
	Servo_get_pos(&parkRa, &parkDec);
	if (__isnan(parkRa))
	{
		parkRa	=	0.0;
		CONSOLE_DEBUG("parkRa is NAN");
	}
	if (__isnan(parkDec))
	{
		parkDec	=	0.0;
		CONSOLE_DEBUG("parkDec is NAN");
	}
	// Set ASCOM properties to current RA/Dec
	cTelescopeProp.RightAscension	=	parkRa;
	cTelescopeProp.Declination		=	parkDec;
	cTelescopeProp.AtPark 			= 	true;

	AlpacaConnect();

	CONSOLE_DEBUG_W_NUM("cTelescopeProp.CanUnpark\t=", cTelescopeProp.CanUnpark);
}

//**************************************************************************************
//	Destructor
//**************************************************************************************
TelescopeDriverServo::~TelescopeDriverServo(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	AlpacaDisConnect();
}

//**************************************************************************************
static void	WriteLocationParam_Dbl(const int socket, const int cfgEnum, const double value)
{
char	lineBuff[512];

	if ((cfgEnum >= 0) && (cfgEnum < OBS_CFG_LAST))
	{
		sprintf(lineBuff,	"<TR>\r\n<TD>%s</TD><TD>%5.4f</TD>\r\n<TR>\r\n",
									gObservConfigArray[cfgEnum].parameter,
									value);
		SocketWriteData(socket,	lineBuff);
	}
}

//**************************************************************************************
static void	WriteLocationParam_Str(const int socket, const int cfgEnum, const char *value)
{
char	lineBuff[512];

	if ((cfgEnum >= 0) && (cfgEnum < OBS_CFG_LAST))
	{
		sprintf(lineBuff,	"<TR>\r\n<TD>%s</TD><TD>%s</TD>\r\n<TR>\r\n",
									gObservConfigArray[cfgEnum].parameter,
									value);
		SocketWriteData(socket,	lineBuff);
	}
}

//**************************************************************************************
static void	WriteMotionConfigParam_Dbl(const int socket, const int cfgEnum, const double value)
{
char	lineBuff[512];

	if ((cfgEnum >= 0) && (cfgEnum < MOTION_CFG_LAST))
	{
		sprintf(lineBuff,	"<TR>\r\n<TD>%s</TD><TD>%5.4f</TD>\r\n<TR>\r\n",
									gMotionConfigArray[cfgEnum].parameter,
									value);
		SocketWriteData(socket,	lineBuff);
	}
}

//**************************************************************************************
static void	WriteMotionConfigParam_Str(const int socket, const int cfgEnum, const char *value)
{
char	lineBuff[512];

	if ((cfgEnum >= 0) && (cfgEnum < MOTION_CFG_LAST))
	{
		sprintf(lineBuff,	"<TR>\r\n<TD>%s</TD><TD>%s</TD>\r\n<TR>\r\n",
									gMotionConfigArray[cfgEnum].parameter,
									value);
		SocketWriteData(socket,	lineBuff);
	}
}

//**************************************************************************************
static void	WriteMotionConfigParam_Int(const int socket, const int cfgEnum, const int value)
{
char	lineBuff[512];

	if ((cfgEnum >= 0) && (cfgEnum < MOTION_CFG_LAST))
	{
		sprintf(lineBuff,	"<TR>\r\n<TD>%s</TD><TD>%d</TD>\r\n<TR>\r\n",
									gMotionConfigArray[cfgEnum].parameter,
									value);
		SocketWriteData(socket,	lineBuff);
	}
}


//**************************************************************************************
static void	WriteMountConfigParam_Dbl(const int socket, const int cfgEnum, const double value)
{
char	lineBuff[512];

	if ((cfgEnum >= 0) && (cfgEnum < MOUNT_CFG_LAST))
	{
		sprintf(lineBuff,	"<TR>\r\n<TD>%s</TD><TD>%5.4f</TD>\r\n<TR>\r\n",
									gMountConfigArray[cfgEnum].parameter,
									value);
		SocketWriteData(socket,	lineBuff);
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("cfgEnum\t=", cfgEnum);
		sprintf(lineBuff,	"<TR>\r\n<TD>ERROR!!!</TD><TD>%5.4f</TD>\r\n<TR>\r\n",
									value);
		SocketWriteData(socket,	lineBuff);
	}
}

//**************************************************************************************
static void	WriteMountConfigParam_Str(const int socket, const int cfgEnum, const char *value)
{
char	lineBuff[512];

	if ((cfgEnum >= 0) && (cfgEnum < MOUNT_CFG_LAST))
	{
		sprintf(lineBuff,	"<TR>\r\n<TD>%s</TD><TD>%s</TD>\r\n<TR>\r\n",
									gMountConfigArray[cfgEnum].parameter,
									value);
		SocketWriteData(socket,	lineBuff);
	}
}

//**************************************************************************************
static void	WriteMountConfigParam_Int(const int socket, const int cfgEnum, const int value)
{
char	lineBuff[512];

	if ((cfgEnum >= 0) && (cfgEnum < MOUNT_CFG_LAST))
	{
		sprintf(lineBuff,	"<TR>\r\n<TD>%s</TD><TD>%d</TD>\r\n<TR>\r\n",
									gMountConfigArray[cfgEnum].parameter,
									value);
		SocketWriteData(socket,	lineBuff);
	}
}

//*****************************************************************************
static void	OutputConfigFileHTML(	const int	theSocket,
									const char	*fileName,
									const char	*title,
									const bool	configFileOK)
{
char	lineBuff[512];
FILE	*filePointer;

	filePointer	=	fopen(fileName, "r");
	if (filePointer != NULL)
	{
		SocketWriteData(theSocket, "<HR>\r\n");
		sprintf(lineBuff, "<P><CENTER>%s Config File</CENTER>\r\n", title);

		SocketWriteData(theSocket, lineBuff);

		if (configFileOK)
		{
			SocketWriteData(theSocket, "<FONT COLOR=green>\r\n");
		}
		else
		{
			SocketWriteData(theSocket, "<FONT COLOR=red>\r\n");
		}

		sprintf(lineBuff, "Config file:%s is %s<P>\r\n", fileName,
							(configFileOK ? "valid" : "NOT valid"));

		SocketWriteData(theSocket, lineBuff);
		SocketWriteData(theSocket, "</FONT COLOR>\r\n");

		SocketWriteData(theSocket, "<PRE>\r\n");
		while (fgets(lineBuff, 500, filePointer) != NULL)
		{
			SocketWriteData(theSocket, lineBuff);
		}
		SocketWriteData(theSocket, "</PRE>\r\n");

		fclose(filePointer);
	}
}

//*****************************************************************************
void	TelescopeDriverServo::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
char	lineBuff[512];
bool	configFileOK;

	//	CONSOLE_DEBUG(__FUNCTION__);

	//*	print the config file to the web page,
	//*	this makes it much easier to see what the config is on a remote system

	SocketWriteData(reqData->socket,	"<HR><CENTER>\r\n");
	SocketWriteData(reqData->socket,	"<H2>Location configuration parameters</H2><BR>\r\n");
	SocketWriteData(reqData->socket,	"<TABLE BORDER=1>\r\n");

		WriteLocationParam_Dbl(reqData->socket,	EPOCH,			gServoObservCfg.baseEpoch);
		WriteLocationParam_Dbl(reqData->socket,	EPOCH_JD,		gServoObservCfg.baseJd);
		WriteLocationParam_Dbl(reqData->socket,	LATITUDE,		gServoObservCfg.lat);
		WriteLocationParam_Dbl(reqData->socket,	LONGITUDE,		gServoObservCfg.lon);
		WriteLocationParam_Dbl(reqData->socket,	ELEVATION,		gServoObservCfg.elev);
		WriteLocationParam_Dbl(reqData->socket,	TEMPERATURE,	gServoObservCfg.temp);
		WriteLocationParam_Dbl(reqData->socket,	PRESSURE,		gServoObservCfg.press);
		WriteLocationParam_Str(reqData->socket,	SITE,			gServoObservCfg.site);

	SocketWriteData(reqData->socket,	"</TABLE>\r\n");

	SocketWriteData(reqData->socket,	"</CENTER>\r\n");
	SocketWriteData(reqData->socket,	"<P>Note: Lat, Lon & Elev from observatorysettings.txt will override what is listed below\r\n");

	//--------------------------------------------------------------------------------
	configFileOK	=	Servo_check_observ_cfg();
	OutputConfigFileHTML(reqData->socket, kOBSERV_CFG_FILE, "Observatory", true);

	//---------------------------------------------------------------
	SocketWriteData(reqData->socket,	"<HR><CENTER>\r\n");
	SocketWriteData(reqData->socket,	"<H2>Servo Mount configuration parameters</H2><BR>\r\n");
	SocketWriteData(reqData->socket,	"<TABLE BORDER=1>\r\n");
		switch(gMountConfig.mount)
		{
			case kFORK:		strcpy(lineBuff,	"FORK");	break;
			case kGERMAN:	strcpy(lineBuff,	"GERMAN");	break;
			default:		strcpy(lineBuff,	"unknown");	break;
		}
		WriteMountConfigParam_Str(reqData->socket,	MOUNT,			lineBuff);

		switch(gMountConfig.side)
		{
			case kEAST:		strcpy(lineBuff,	"EAST");	break;
			case kWEST:		strcpy(lineBuff,	"WEST");	break;
			case kNONE:		strcpy(lineBuff,	"NONE");	break;
			default:		strcpy(lineBuff,	"unknown");	break;
		}
		WriteMountConfigParam_Str(reqData->socket,	PARK_SIDE,		lineBuff);
		WriteMountConfigParam_Dbl(reqData->socket,	ROLLOVER_WIN,	gMountConfig.flipWin);
//		WriteMountConfigParam_Dbl(reqData->socket,	OFF_TARGET_TOL,	gMountConfig.offTarget);

		//--------------------------------------------------------------------------------
		SocketWriteData(reqData->socket,	"<TR><TH COLSPAN=2>Right Assention</TH></TR>\r\n");
		WriteMountConfigParam_Dbl(reqData->socket,	RA_MOTOR_GEAR,		gMountConfig.ra.motorGear);
		WriteMountConfigParam_Dbl(reqData->socket,	RA_MAIN_GEAR,		gMountConfig.ra.mainGear);
//		WriteMountConfigParam_Dbl(reqData->socket,	RA_MOTOR_MAX_RPM,	gMountConfig.ra.motorMaxRPM);
		WriteMountConfigParam_Dbl(reqData->socket,	RA_ENCODER,			gMountConfig.ra.encoder);
		WriteMountConfigParam_Dbl(reqData->socket,	RA_MAX_ACC,			gMountConfig.ra.realAcc);
		WriteMountConfigParam_Dbl(reqData->socket,	RA_MAX_VEL,			gMountConfig.ra.realVel);
		WriteMountConfigParam_Dbl(reqData->socket,	RA_ADJ_VEL,			gMountConfig.ra.realAdj);
		WriteMountConfigParam_Dbl(reqData->socket,	RA_SLEW_VEL,		gMountConfig.ra.realSlew);
		WriteMountConfigParam_Dbl(reqData->socket,	RA_CONFIG,			gMountConfig.ra.config);
		WriteMountConfigParam_Dbl(reqData->socket,	RA_PRECESSION,		gMountConfig.ra.prec);
		WriteMountConfigParam_Dbl(reqData->socket,	RA_PARK,			gMountConfig.ra.park);

//		WriteMountConfigParam_Int(reqData->socket,	RA_SI_CON,			gMountConfig.ra.si);
		WriteMountConfigParam_Dbl(reqData->socket,	RA_GEAR_LASH,		gMountConfig.ra.gearLash);
		WriteMountConfigParam_Dbl(reqData->socket,	RA_SENSOR,			gMountConfig.ra.sync);
		WriteMountConfigParam_Int(reqData->socket,	RA_PARK_SENSOR,		gMountConfig.ra.syncValue);

		//--------------------------------------------------------------------------------
		SocketWriteData(reqData->socket,	"<TR><TH COLSPAN=2>Declination</TH></TR>\r\n");
		WriteMountConfigParam_Dbl(reqData->socket,	DEC_MOTOR_GEAR,		gMountConfig.dec.motorGear);
		WriteMountConfigParam_Dbl(reqData->socket,	DEC_MAIN_GEAR,		gMountConfig.dec.mainGear);
//		WriteMountConfigParam_Dbl(reqData->socket,	DEC_MOTOR_MAX_RPM,	gMountConfig.dec.motorMaxRPM);
		WriteMountConfigParam_Dbl(reqData->socket,	DEC_ENCODER,		gMountConfig.dec.encoder);
		WriteMountConfigParam_Dbl(reqData->socket,	DEC_MAX_ACC,		gMountConfig.dec.realAcc);
		WriteMountConfigParam_Dbl(reqData->socket,	DEC_MAX_VEL,		gMountConfig.dec.realVel);
		WriteMountConfigParam_Dbl(reqData->socket,	DEC_ADJ_VEL,		gMountConfig.dec.realAdj);
		WriteMountConfigParam_Dbl(reqData->socket,	DEC_SLEW_VEL,		gMountConfig.dec.realSlew);
		WriteMountConfigParam_Dbl(reqData->socket,	DEC_CONFIG,			gMountConfig.dec.config);
		WriteMountConfigParam_Dbl(reqData->socket,	DEC_PRECESSION,		gMountConfig.dec.prec);
		WriteMountConfigParam_Dbl(reqData->socket,	DEC_PARK,			gMountConfig.dec.park);

//		WriteMountConfigParam_Int(reqData->socket,	DEC_SI_CON,			gMountConfig.dec.si);
		WriteMountConfigParam_Dbl(reqData->socket,	DEC_GEAR_LASH,		gMountConfig.dec.gearLash);
		WriteMountConfigParam_Dbl(reqData->socket,	DEC_SENSOR,			gMountConfig.dec.sync);
		WriteMountConfigParam_Int(reqData->socket,	DEC_PARK_SENSOR,	gMountConfig.dec.syncValue);

	SocketWriteData(reqData->socket,	"</TABLE>\r\n");
	SocketWriteData(reqData->socket,	"</CENTER>\r\n");

	configFileOK	=	Servo_check_mount_cfg();
	OutputConfigFileHTML(reqData->socket, kMOUNT_CFG_FILE, "Mount", configFileOK);

	//--------------------------------------------------------------------------------
	SocketWriteData(reqData->socket,	"<HR><CENTER>\r\n");
	SocketWriteData(reqData->socket,	"<H2>Servo Motion configuration parameters</H2><BR>\r\n");
	SocketWriteData(reqData->socket,	"<TABLE BORDER=1>\r\n");
		WriteMotionConfigParam_Str(reqData->socket,	COMM_PORT,			gMotionConfig.port);
		WriteMotionConfigParam_Int(reqData->socket,	MC_ADDR,			gMotionConfig.motor0.addr);
		WriteMotionConfigParam_Int(reqData->socket,	BAUD,				gMotionConfig.baud);

		SocketWriteData(reqData->socket,	"<TR><TH COLSPAN=2>Right Assention</TH></TR>\r\n");
		WriteMotionConfigParam_Dbl(reqData->socket,	RA_KP_CON,			gMotionConfig.motor0.kp);
		WriteMotionConfigParam_Dbl(reqData->socket,	RA_KI_CON,			gMotionConfig.motor0.ki);
		WriteMotionConfigParam_Dbl(reqData->socket,	RA_KD_CON,			gMotionConfig.motor0.kd);
		WriteMotionConfigParam_Dbl(reqData->socket,	RA_IL_CON,			gMotionConfig.motor0.il);

		SocketWriteData(reqData->socket,	"<TR><TH COLSPAN=2>Declination</TH></TR>\r\n");
		WriteMotionConfigParam_Dbl(reqData->socket,	DEC_KP_CON,			gMotionConfig.motor1.kp);
		WriteMotionConfigParam_Dbl(reqData->socket,	DEC_KI_CON,			gMotionConfig.motor1.ki);
		WriteMotionConfigParam_Dbl(reqData->socket,	DEC_KD_CON,			gMotionConfig.motor1.kd);
		WriteMotionConfigParam_Dbl(reqData->socket,	DEC_IL_CON,			gMotionConfig.motor1.il);

	SocketWriteData(reqData->socket,	"</TABLE>\r\n");
	SocketWriteData(reqData->socket,	"</CENTER>\r\n");

	configFileOK	=	Servo_check_motion_cfg();
	OutputConfigFileHTML(reqData->socket, kMOTION_CFG_FILE, "Motion", configFileOK);
}

//*****************************************************************************
//*	returns delay time in micro-seconds
//*****************************************************************************
int32_t	TelescopeDriverServo::RunStateMachine(void)
{
double 		currRA_hours;
double 		currDec_degrees;
int8_t 		servoSide;
uint32_t	currentMilliSecs;
uint32_t	deltaMilliSecs;

	currentMilliSecs	=	millis();
	deltaMilliSecs		=	currentMilliSecs - cLastUpdate_milliSecs;

//	CONSOLE_DEBUG_W_LONG("deltaMilliSecs\t=", deltaMilliSecs);
	if (deltaMilliSecs > (5 * 1000))
	{
//		CONSOLE_DEBUG(__FUNCTION__);

		//	Update mount state and if "moving" update the .Slewing flag
		UpdateSlewingProperty();

		//	If moving, get current RA & Dec
		Servo_get_pos(&currRA_hours, &currDec_degrees);

//		CONSOLE_DEBUG_W_DBL("currRA_hours   \t=", currDec_degrees);
//		CONSOLE_DEBUG_W_DBL("currDec_degrees\t=", currRA_hours);

		if (__isnan(currRA_hours))
		{
//			currRA_hours	=	0.0;
//			CONSOLE_DEBUG("currRA_hours is NAN");
		}
		if (__isnan(currDec_degrees))
		{
//			currDec_degrees	=	0.0;
//			CONSOLE_DEBUG("currDec_degrees is NAN");
		}

		while (currRA_hours > 24.0)
		{
			currRA_hours	-=	24.0;
		}
		cTelescopeProp.RightAscension	=	currRA_hours;
		cTelescopeProp.Declination		=	currDec_degrees;

		//	Update the side of pier
		servoSide	=	Servo_get_pier_side();
		switch (servoSide)
		{
			case kEAST:
				cTelescopeProp.SideOfPier	=	kPierSide_pierEast;
				break;

			case kWEST:
				cTelescopeProp.SideOfPier	=	kPierSide_pierWest;
				break;

			default:
				cTelescopeProp.SideOfPier	=	kPierSide_pierUnknown;
				break;
		}
		cLastUpdate_milliSecs	=	currentMilliSecs;
	}
	//*	2 * 1000 * 1000 means you might not get called again for 2 seconds
	//*	you might get called earlier
	return(2 * 1000 * 1000);
}

//*****************************************************************************
void TelescopeDriverServo::UpdateSlewingProperty(void)
{
TYPE_MOVE 	currentMoveState;

	//	Update mount state and if "moving" update the .Slewing flag
	currentMoveState	=	Servo_state();
	if (currentMoveState == MOVING || currentMoveState == PARKING)
	{
		CONSOLE_DEBUG("Setting cTelescopeProp.Slewing to TRUE");
		cTelescopeProp.Slewing	=	true;
	}
	else
	{
		cTelescopeProp.Slewing	=	false;
	}
}


//*****************************************************************************
TYPE_ASCOM_STATUS TelescopeDriverServo::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

	alpacaErrCode	=	kASCOM_Err_Success;

	Servo_stop_axes(SERVO_BOTH_AXES);

	cTelescopeProp.Slewing	=	false;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS TelescopeDriverServo::Telescope_FindHome(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return (alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS TelescopeDriverServo::Telescope_MoveAxis(	const int		axisNum,
															const double	moveRate_degPerSec,
															char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
int					servoStatus;

	CONSOLE_DEBUG(__FUNCTION__);

	//	RA axis - positive degsPerSec means decreasing RA direction (West)
	//	RA axis - negative degsPerSec means increasing RA direction (East)
	//	Dec axis - positive degsPerSec means increasing Dec direction (North)
	//	Dec axis - negative degsPerSec means decreasing Dec direction (South)
	//	axisNum = 0 -> SERVO_RA_AXIS- maps correclty to "servo_std_defs.h"
	//	axisNum = 1 -> SERVO_DEC_AXIS - maps to correctly "servo_std_defs.h"

	CONSOLE_DEBUG_W_DBL("moveRate_degPerSec\t=", moveRate_degPerSec);

	// if a non-zero velocity is requested
	if (moveRate_degPerSec != 0.0)
	{
		// Toggle the direction for RA due to ASCOM sign difference with reality
		if (axisNum == SERVO_RA_AXIS)
		{
			// Use the inverse of supplied moveRate arg
			servoStatus	=	Servo_move_axis_by_vel(axisNum, -moveRate_degPerSec);

		}
		else
		{
			// For Dec, no correction needed
			servoStatus	=	Servo_move_axis_by_vel(axisNum, moveRate_degPerSec);
		}
		if (servoStatus == kSTATUS_OK)
		{
			CONSOLE_DEBUG("Setting cTelescopeProp.Slewing to TRUE");
			cTelescopeProp.Slewing	=	true;
			alpacaErrCode			=	kASCOM_Err_Success;
		}
		else
		{
			// Only ways to get an error from move_axis_by_vel is to have a bad axisNum
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			cTelescopeProp.Slewing	=	false;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_move_axis_by_vel() failed");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
	}
	else
	{
		//	Restore tracking rate, if set on axis
		servoStatus	=	Servo_start_axes_tracking(axisNum);
		if (servoStatus == kSTATUS_OK)
		{
			alpacaErrCode			=	kASCOM_Err_Success;
		}
		else
		{
			// Only ways to get an error from start_tracking is to have a bad axisNum
			alpacaErrCode	=	kASCOM_Err_InvalidValue;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_start_tracking() failed");
			CONSOLE_DEBUG(alpacaErrMsg);
		}
		UpdateSlewingProperty();
	}	// of if-else non-zero velocity
	if (alpacaErrCode != kASCOM_Err_Success)
	{
		CONSOLE_DEBUG_W_NUM("alpacaErrCode\t=", alpacaErrCode);
		CONSOLE_DEBUG(alpacaErrMsg);
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS TelescopeDriverServo::Telescope_Park(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					servoStatus;

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	kASCOM_Err_Success;
	servoStatus		=	Servo_move_to_park();
	if (servoStatus == kSTATUS_OK)
	{
		//	Mount is moving to the park position
		CONSOLE_DEBUG("Setting cTelescopeProp.Slewing to TRUE");
		cTelescopeProp.Slewing			=	true;

		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else
	{
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_move_to_park failed");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	//*RNS TODO: ...for now, force it parked regardless
	cTelescopeProp.AtPark 		= 	true;

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS TelescopeDriverServo::Telescope_SetPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS TelescopeDriverServo::Telescope_SlewToAltAz(const double newAlt_Degrees, const double newAz_Degrees, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS TelescopeDriverServo::Telescope_SlewToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
int					servoStatus;

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	kASCOM_Err_Success;

	servoStatus	=	Servo_move_to_coordins(newRA, newDec, Servo_get_lat(), Servo_get_lon());
	if (servoStatus != kSTATUS_OK)
	{
		//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_move_to_coordins failed");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not connected");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else
	{
		//	Mount is moving to the RA/Dec coordinates
	}
	UpdateSlewingProperty();
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_SyncToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
// double				hoursRA;

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	kASCOM_Err_Success;
	// hoursRA			=	newRA;

	//	Convert RA to hours
	//Time_deci_deg_to_hours(&hoursRA);

	// Set the new value to the current position
	Servo_set_pos(newRA, newDec);

	return (alpacaErrCode);
}

//*****************************************************************************
//* TODO: Will need to modify from RA only to BOTH for alt-azi mount
//*****************************************************************************
TYPE_ASCOM_STATUS TelescopeDriverServo::Telescope_TrackingOnOff(const bool newTrackingState,
																char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
int					servoStatus;

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	kASCOM_Err_Success;
	if (newTrackingState)
	{

		servoStatus	=	Servo_start_axes_tracking(SERVO_RA_AXIS);
		if (servoStatus != kSTATUS_OK)
		{
			//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_start_tracking failed");
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not connected");
			alpacaErrCode	=	kASCOM_Err_NotConnected;
		}
	}
	else
	{
		servoStatus	=	Servo_stop_axes(SERVO_RA_AXIS);
		if (servoStatus != kSTATUS_OK)
		{
			//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_stop failed");
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not connected");
			alpacaErrCode	=	kASCOM_Err_NotConnected;
		}
	}

	if (servoStatus == kSTATUS_OK)
	{
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_UnPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
int					servoStatus;

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	kASCOM_Err_Success;

	//	Attempt to unpark the mount, only works if it's current state is parked
	servoStatus		=	Servo_unpark();

	CONSOLE_DEBUG_W_NUM("@@@ Telescope_UnPark() servoStatus\t=", servoStatus);
	if (servoStatus == kSTATUS_OK)
	{
		//	Mount is now unparked and ready for movement
		cTelescopeProp.AtPark 		= 	false;
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	else
	{
		//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_unpark failed, mount was not parked");
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not connected");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	//*	for now, force it unparked regardless
	cTelescopeProp.AtPark 		= 	false;


	return(alpacaErrCode);
}

#endif // _ENABLE_TELESCOPE_SERVO_
