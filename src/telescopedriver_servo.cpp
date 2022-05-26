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
//#include	"helper_functions.h"

//*	servo controller routines using RoboClaws controller
#include	"servo_mount_cfg.h"
#include	"servo_mount.h"
#include	"servo_std_defs.h"
#include	"servo_time.h"

#include	"telescopedriver.h"
#include	"telescopedriver_servo.h"


//**************************************************************************************
TelescopeDriverServo::TelescopeDriverServo(void)
	: TelescopeDriver()
{
int		cfgStatus;
int8_t	servoSide;
double	parkRA, parkDec;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name, "AlpacaPi-Mount-Servo");
	strcpy(cCommonProp.Description, "Mount using RoboClaw servo controller");

	//*	setup the options for this driver
	cTelescopeProp.AlginmentMode	=	kAlignmentMode_algGermanPolar;
	cTelescopeProp.CanSlewAsync		=	true;
	cTelescopeProp.CanSync			=	false;
	cTelescopeProp.CanSetTracking	=	true;
	cTelescopeProp.CanMoveAxis		=	true;
	cTelescopeProp.CanUnpark		=	true;
	cTelescopeProp.CanPark			=	true;
	cTelescopeProp.CanFindHome		=	false;

#warning <RNS> No sure of your invocation location but we likely need complete pathnames or soft paths
#define kLOCAL_CFG_FILE "servo_location.cfg"

	cServoConfigIsValid	=	false;
	cfgStatus			=	Servo_init(kSCOPE_CFG_FILE, kLOCAL_CFG_FILE);
	if (cfgStatus == kSTATUS_OK)
	{
		cServoConfigIsValid	=	true;
	}
	CONSOLE_DEBUG_W_NUM("cfgStatus\t=", cfgStatus);
	CONSOLE_DEBUG_W_NUM("kSTATUS_OK\t=", kSTATUS_OK);
	CONSOLE_DEBUG_W_NUM("cServoConfigIsValid\t=", cServoConfigIsValid);
//	CONSOLE_ABORT(__FUNCTION__);

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
	cTelescopeProp.SiteLatitude		=	Time_get_lat();
	cTelescopeProp.SiteLongitude	=	Time_get_lon();

	//	We know that Servo_init() always sets the scope to the park position
	Servo_get_park_coordins(&parkRA, &parkDec);
	//	Convert park RA from hours to degs
	Time_deci_hours_to_deg(&parkRA);

	cTelescopeProp.RightAscension	=	parkRA;
	cTelescopeProp.Declination		=	parkDec;

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
static void	WriteConfigParam_Dbl(const int socket, const int cfgEnum, const double value)
{
char	lineBuff[512];

	if ((cfgEnum >= 0))
	{
		sprintf(lineBuff,	"<TR>\r\n<TD>%s</TD><TD>%5.4f</TD>\r\n<TR>\r\n",
									gMountConfigArray[cfgEnum].parameter,
									value);
		SocketWriteData(socket,	lineBuff);
	}
}

//**************************************************************************************
static void	WriteConfigParam_Str(const int socket, const int cfgEnum, const char *value)
{
char	lineBuff[512];

	if ((cfgEnum >= 0))
	{
		sprintf(lineBuff,	"<TR>\r\n<TD>%s</TD><TD>%s</TD>\r\n<TR>\r\n",
									gMountConfigArray[cfgEnum].parameter,
									value);
		SocketWriteData(socket,	lineBuff);
	}
}

//**************************************************************************************
static void	WriteConfigParam_Int(const int socket, const int cfgEnum, const int value)
{
char	lineBuff[512];

	if ((cfgEnum >= 0))
	{
		sprintf(lineBuff,	"<TR>\r\n<TD>%s</TD><TD>%d</TD>\r\n<TR>\r\n",
									gMountConfigArray[cfgEnum].parameter,
									value);
		SocketWriteData(socket,	lineBuff);
	}
}

//*****************************************************************************
void	TelescopeDriverServo::OutputHTML_Part2(TYPE_GetPutRequestData *reqData)
{
char	lineBuff[512];
FILE	*filePointer;

	//	CONSOLE_DEBUG(__FUNCTION__);

	//*	print the config file to the web page,
	//*	this makes it much easier to see what the config is on a remote system

#warning <RNS> Need to use the output text from Servo_read_*_file cal.  Dumping parsed data
#warning <RNS> is more valuable than the input file. Perhaps dump to log file in Servo_init()?

	filePointer	=	fopen(kLOCAL_CFG_FILE, "r");
	if (filePointer != NULL)
	{
		SocketWriteData(reqData->socket, "<HR>\r\n");
		SocketWriteData(reqData->socket, "<P><CENTER>Location Config File</CENTER>\r\n");

		sprintf(lineBuff, "Config file:%s<P>\r\n", kLOCAL_CFG_FILE);
		SocketWriteData(reqData->socket, lineBuff);

		SocketWriteData(reqData->socket, "<PRE>\r\n");
		while (fgets(lineBuff, 500, filePointer) != NULL)
		{
			SocketWriteData(reqData->socket, lineBuff);
		}
		SocketWriteData(reqData->socket, "</PRE>\r\n");

		fclose(filePointer);
	}

//	filePointer	=	fopen(kSCOPE_CFG_FILE, "r");
//	if (filePointer != NULL)
//	{
//		SocketWriteData(reqData->socket, "<HR>\r\n");
//		SocketWriteData(reqData->socket, "<P><CENTER>Servo Config File</CENTER>\r\n");
//
//		sprintf(lineBuff, "Config file:%s is %s<P>\r\n", kSCOPE_CFG_FILE,
//							(cServoConfigIsValid ? "valid" : "NOT valid"));
//
//		SocketWriteData(reqData->socket, lineBuff);
//
//		SocketWriteData(reqData->socket, "<PRE>\r\n");
//		while (fgets(lineBuff, 500, filePointer) != NULL)
//		{
//			SocketWriteData(reqData->socket, lineBuff);
//		}
//		SocketWriteData(reqData->socket, "</PRE>\r\n");
//
//		fclose(filePointer);
//	}

	//---------------------------------------------------------------
	SocketWriteData(reqData->socket,	"<HR><CENTER>\r\n");
	SocketWriteData(reqData->socket,	"<TABLE BORDER=1>\r\n");
		WriteConfigParam_Dbl(reqData->socket,	MC_FREQ,			gScopeConfig.freq);
		WriteConfigParam_Str(reqData->socket,	COMM_PORT,			gScopeConfig.port);
		WriteConfigParam_Int(reqData->socket,	BAUD,				gScopeConfig.baud);
		WriteConfigParam_Int(reqData->socket,	MC_ADDR,			gScopeConfig.addr);
		switch(gScopeConfig.mount)
		{
			case kFORK:		strcpy(lineBuff,	"FORK");	break;
			case kGERMAN:	strcpy(lineBuff,	"GERMAN");	break;
			default:		strcpy(lineBuff,	"unknown");	break;
		}
		WriteConfigParam_Str(reqData->socket,	MOUNT,			lineBuff);

		switch(gScopeConfig.side)
		{
			case kWEST:		strcpy(lineBuff,	"WEST");	break;
			case kEAST:		strcpy(lineBuff,	"EAST");	break;
			case kNONE:		strcpy(lineBuff,	"NONE");	break;
			default:		strcpy(lineBuff,	"unknown");	break;
		}
		WriteConfigParam_Str(reqData->socket,	PARK_SIDE,		lineBuff);

//	ROLLOVER_WIN,
//	OFF_TARGET_TOL

		SocketWriteData(reqData->socket,	"<TR><TH COLSPAN=2>Right Assention</TH></TR>\r\n");
		WriteConfigParam_Dbl(reqData->socket,	RA_MOTOR_GEAR,		gScopeConfig.ra.motorGear);
		WriteConfigParam_Dbl(reqData->socket,	RA_MAIN_GEAR,		gScopeConfig.ra.mainGear);
		WriteConfigParam_Dbl(reqData->socket,	RA_MOTOR_MAX_RPM,	gScopeConfig.ra.motorMaxRPM);
		WriteConfigParam_Dbl(reqData->socket,	RA_ENCODER,			gScopeConfig.ra.encoder);
		WriteConfigParam_Dbl(reqData->socket,	RA_MAX_ACC,			gScopeConfig.ra.realAcc);
		WriteConfigParam_Dbl(reqData->socket,	RA_MAX_VEL,			gScopeConfig.ra.realVel);
		WriteConfigParam_Dbl(reqData->socket,	RA_ADJ_VEL,			gScopeConfig.ra.realAdj);
		WriteConfigParam_Dbl(reqData->socket,	RA_SLEW_VEL,		gScopeConfig.ra.realSlew);
		WriteConfigParam_Dbl(reqData->socket,	RA_CONFIG,			gScopeConfig.ra.config);
		WriteConfigParam_Dbl(reqData->socket,	RA_PRECESSION,		gScopeConfig.ra.prec);
		WriteConfigParam_Dbl(reqData->socket,	RA_PARK,			gScopeConfig.ra.park);
//	RA_SI_CON,
//	RA_KP_CON,
//	RA_KI_CON,
//	RA_KD_CON,
//	RA_IL_CON,
		WriteConfigParam_Dbl(reqData->socket,	RA_GEAR_LASH,		gScopeConfig.ra.gearLash);
//	RA_SENSOR,
//	RA_PARK_SENSOR,

		//--------------------------------------------------------------------------------
		SocketWriteData(reqData->socket,	"<TR><TH COLSPAN=2>Declination</TH></TR>\r\n");
		WriteConfigParam_Dbl(reqData->socket,	DEC_MOTOR_GEAR,		gScopeConfig.dec.motorGear);
		WriteConfigParam_Dbl(reqData->socket,	DEC_MAIN_GEAR,		gScopeConfig.dec.mainGear);
		WriteConfigParam_Dbl(reqData->socket,	DEC_MOTOR_MAX_RPM,	gScopeConfig.dec.motorMaxRPM);
		WriteConfigParam_Dbl(reqData->socket,	DEC_ENCODER,		gScopeConfig.dec.encoder);
		WriteConfigParam_Dbl(reqData->socket,	DEC_MAX_ACC,		gScopeConfig.dec.realAcc);
		WriteConfigParam_Dbl(reqData->socket,	DEC_MAX_VEL,		gScopeConfig.dec.realVel);
		WriteConfigParam_Dbl(reqData->socket,	DEC_ADJ_VEL,		gScopeConfig.dec.realAdj);
		WriteConfigParam_Dbl(reqData->socket,	DEC_SLEW_VEL,		gScopeConfig.dec.realSlew);
		WriteConfigParam_Dbl(reqData->socket,	DEC_CONFIG,			gScopeConfig.dec.config);
		WriteConfigParam_Dbl(reqData->socket,	DEC_PRECESSION,		gScopeConfig.dec.prec);
		WriteConfigParam_Dbl(reqData->socket,	DEC_PARK,			gScopeConfig.dec.park);
//	DEC_SI_CON,
//	DEC_KP_CON,
//	DEC_KI_CON,
//	DEC_KD_CON,
//	DEC_IL_CON,
		WriteConfigParam_Dbl(reqData->socket,	DEC_GEAR_LASH,		gScopeConfig.dec.gearLash);
//	DEC_SENSOR,
//	DEC_PARK_SENSOR,



//	uint16_t	encoderMaxSpeed;	//	here and below are the int value for the MC, calc'd from the real-world values above
//	int32_t		pos;
//	uint32_t	maxAcc;
//	uint32_t	acc;
//	uint32_t	maxVel;
//	uint32_t	vel;
//	uint32_t	adj;
//	uint32_t	slew;
//	uint8_t		si;
//	uint16_t	kp;
//	uint16_t	ki;
//	uint16_t	kd;
//	uint16_t	il;
//	uint16_t	status;
//	uint8_t		cmdQueue;
//	int32_t		track;
//	double		direction;
//	double		park;
//	int8_t		parkInfo;		// on RA holds the original park_side of mount, on dec it hold the original direction before any flip
//	long double	time;
//	// double	lockDown;
//	double		zero;			// Axis real value at zero steps in decimal degrees, including RA
//	double		standby;
//	double		sync;
//	uint16_t	syncValue;
//	uint16_t	syncError;

	SocketWriteData(reqData->socket,	"</TABLE>\r\n");
	SocketWriteData(reqData->socket,	"</CENTER>\r\n");
}

//*****************************************************************************
//*	returns delay time in micro-seconds
//*****************************************************************************
int32_t	TelescopeDriverServo::RunStateMachine(void)
{
TYPE_MOVE 	currentMoveState;
double 		currRA, currDec;
int8_t 		servoSide;


	//*	this is where your periodic code goes
	//*	update cTelescopeProp values here
#warning <RNS>  Not sure I did this right but here is an attempt, please code review

	CONSOLE_DEBUG(__FUNCTION__);

	//	Update mount state and if "moving" update the .Slewing flag
	currentMoveState	=	Servo_state();
	if (currentMoveState == MOVING || currentMoveState == PARKING)
	{
		cTelescopeProp.Slewing	=	true;
	}
	else
	{
		cTelescopeProp.Slewing	=	false;
	}

#warning <RNS>  Also not positive I did this right but here is an attempt
	//	If moving, get current RA & Dec
	Servo_get_pos(&currRA, &currDec);
	//	Convert park RA from hours to degs
	Time_deci_hours_to_deg(&currRA);
	cTelescopeProp.RightAscension	=	currRA;
	cTelescopeProp.Declination		=	currDec;

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

	//*	5 * 1000 * 1000 means you might not get called again for 5 seconds
	//*	you might get called earlier
	return(5 * 1000 * 1000);
}

//*****************************************************************************
TYPE_ASCOM_STATUS TelescopeDriverServo::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

	alpacaErrCode	=	kASCOM_Err_Success;
	Servo_stop_all();

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
TYPE_ASCOM_STATUS TelescopeDriverServo::Telescope_MoveAxis(const int axisNum, const double moveRate_degPerSec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
int					servoStatus;

#warning <RNS> This might work, needs a your code review please ;^)
	CONSOLE_DEBUG(__FUNCTION__);

	//	RA axis - positive degsPerSec means decreasing RA direction (West)
	//	RA axis - negative degsPerSec means increasing RA direction (West)
	//	Dec axis - positive degsPerSec means increasing Dec direction (North)
	//	Dec axis - negative degsPerSec means decreasing Dec direction (Sout)
	//	axisNum = 0 -> RA - maps correclty to "servo_std_defs.h"
	//	axisNum = 1 -> Dec - maps to correctly "servo_std_defs.h"

	// if a non-zero velocity is requested
	if (moveRate_degPerSec != 0.0)
	{
		servoStatus	=	Servo_move_axis_by_vel(axisNum, moveRate_degPerSec);
		if (servoStatus == kSTATUS_OK)
		{
			cTelescopeProp.Slewing	=	true;
		}
		else
		{
			// Only ways to get an error from move_axis_by_vel is to have a bad axisNum
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
			alpacaErrCode			=	kASCOM_Err_NotImplemented;
			cTelescopeProp.Slewing	=	false;
		}
	}
	else
	{
		//	Restore tracking rate, if set on axis
		servoStatus	=	Servo_start_axis_tracking(axisNum);
		if (servoStatus != kSTATUS_OK)
		{
			// Only ways to get an error from start_axis_tracking is to have a bad axisNum
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
		}
		cTelescopeProp.Slewing	=	false;
	}	// of if-else non-zero velocity

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS TelescopeDriverServo::Telescope_Park(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
int					servoStatus;
double				parkRA, parkDec;

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	kASCOM_Err_Success;
	servoStatus		=	Servo_move_to_park();
	if (servoStatus != kSTATUS_OK)
	{
		//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_move_to_park failed");
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not connected");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	else
	{
		//	Mount is moving to the park position

		//	Update target RA/DEC using park position and slew state
		Servo_get_park_coordins(&parkRA, &parkDec);
		// Convert RA from hours to degs
		Time_deci_hours_to_deg(&parkRA);
		cTelescopeProp.TargetRightAscension	=	parkRA;
		cTelescopeProp.TargetDeclination	=	parkDec;
		cTelescopeProp.Slewing				=	true;

		CONSOLE_DEBUG(alpacaErrMsg);
	}

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
double				hoursRA;

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	kASCOM_Err_Success;
	hoursRA			=	(double)newRA;

	// Convert RA to hours
	Time_deci_deg_to_hours(&hoursRA);
	servoStatus	=	Servo_move_to_coordins(hoursRA, newDec, Time_get_lat(), Time_get_lon());

	if (servoStatus != kSTATUS_OK)
	{
		//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_more_to_coordins failed");
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not connected");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	else
	{
		//	Mount is moving to the RA/Dec coordinates

		CONSOLE_DEBUG(alpacaErrMsg);
	}

	servoStatus	=	Servo_state();
	if (servoStatus == MOVING || servoStatus == TRACKING)
	{
		cTelescopeProp.Slewing	=	true;
	}
	else
	{
		cTelescopeProp.Slewing	=	false;
	}

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_SyncToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;
double				hoursRA;

	CONSOLE_DEBUG(__FUNCTION__);

	alpacaErrCode	=	kASCOM_Err_Success;
	hoursRA			=	newRA;

	//	Convert RA to hours
	Time_deci_deg_to_hours(&hoursRA);

	// Set the new value to the current position
	Servo_set_pos(hoursRA, newDec);

	return (alpacaErrCode);
}

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
		servoStatus	=	Servo_start_axis_tracking(SERVO_RA_AXIS);
		if (servoStatus != kSTATUS_OK)
		{
			//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_start_tracking failed");
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not connected");
			alpacaErrCode	=	kASCOM_Err_NotConnected;
		}
	}
	else
	{
		servoStatus	=	Servo_stop_axis_tracking(SERVO_RA_AXIS);
		if (servoStatus != kSTATUS_OK)
		{
			//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_stop_tracking failed");
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
	servoStatus	=	Servo_unpark();
	if (servoStatus != kSTATUS_OK)
	{
		//	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Servo_unpark failed, mount was not parked");
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not connected");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	else
	{
		//	Mount is now unparked and ready for movement
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

#endif // _ENABLE_TELESCOPE_SERVO_
