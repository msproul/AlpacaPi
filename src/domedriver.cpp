//**************************************************************************
//*	Name:			domedriver.cpp
//*
//*	Author:			Mark Sproul (C) 2019-2020
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
//*	Apr 12,	2019	<MLS> Created domedriver.c
//*	Apr 12,	2019	<MLS> Started on Dome control
//*	Apr 14,	2019	<MLS> Moved dome specific code to separate file
//*	May  6,	2019	<MLS> Working on uniform implementation7
//*	May  6,	2019	<MLS> Added Init_Dome() & Dome_RunStateMachine()
//*	May  9,	2019	<MLS> Added Dome_OutputHTML()
//*	May 16,	2019	<MLS> WiringPi PWM working
//*	May 19,	2019	<MLS> Open/Close of shutter timing logic added
//*	May 20,	2019	<MLS> Added CheckDomeButtons()
//*	May 21,	2019	<MLS> Added DomeControl_BumpMove()
//*	May 25,	2019	<MLS> Dome controller installed and working
//*	May 27,	2019	<MLS> Home and Park sensors hooked up and working
//*	May 28,	2019	<MLS> Added Reversing logic, working
//*	May 28,	2019	<MLS> Added currentstate command
//*	May 29,	2019	<MLS> Added GetStateString()
//*	May 29,	2019	<MLS> Added logevent when state changes
//*	Jun 13,	2019	<MLS> Added DomeControl_SlowMove()
//*	Jun 14,	2019	<MLS> Auto start on R-Pi working by adding line to /etc/rc.local
//*	Jun 22,	2019	<MLS> Manual move now stops at park or at home
//*	Jun 24,	2019	<MLS> Added 2 second delay for auto stop on manual move
//*	Sep  4,	2019	<MLS> Converted dome driver to C++
//*	Oct  9,	2019	<MLS> More work on dome driver C++
//*	Oct 14,	2019	<MLS> Changed routine names for consistency
//*	Oct 25,	2019	<MLS> Bump move now uses slow movement
//*	Oct 25,	2019	<MLS> Added Get_NormalMove()
//*	Mar 15,	2020	<MLS> Added Get_Readall()
//*	Apr 14,	2020	<MLS> Major work on dome driver, adding functions even if not implemented
//*	Apr 14,	2020	<MLS> CONFORM-dome -> Passes most of CONFORM testing
//*	Apr 30,	2020	<MLS> Added connection to shutter device for status of shutter
//*	May  3,	2020	<MLS> Working on slaved mode
//*	May  3,	2020	<MLS> Added GetSlitTrackerData()
//*	May  3,	2020	<MLS> Added ProcessDiscovery() to dome driver
//*	Dec  2,	2020	<MLS> Added OpenShutter() and CloseShutter() virtual routines
//*	Dec  2,	2020	<MLS> Added support for Roll Off Roof
//*	Dec  7,	2020	<MLS> Added _ENABLE_SLIT_TRACKER_REMOTE_
//*	Dec  7,	2020	<MLS> Added _ENABLE_REMOTE_SHUTTER_
//*	Dec  7,	2020	<MLS> CONFORM-dome -> 4 errors, 0 warnings and 0 issues
//*	Jan 10,	2021	<MLS> Added UpdateDomePosition()
//*	Jan 10,	2021	<MLS> Added Put_SyncToAzimuth()
//*	Jan 10,	2021	<MLS> Put_Park() can now can figure out which way to the parking lot ;)
//*	Jan 10,	2021	<MLS> Put_SlewToAzimuth() now working
//*	Jan 10,	2021	<MLS> Added Put_SlewToAltitude() (not finished)
//*	Jan 10,	2021	<MLS> Put_FindHome() can now can figure out which way to go home ;)
//*	Jan 12,	2021	<MLS> Added RunStateMachine_Dome() & RunStateMachine_ROR()
//*****************************************************************************
//*	cd /home/pi/dev-mark/alpaca
//*	LOGFILE=logfile.txt
//*	DATE=`date`
//*
//*	if [ -f domecontroller ]
//*	then
//*		./domecontroller >/dev/null &
//*		echo "$DATE - Dome controller started" >> $LOGFILE
//*	else
//*		pwd
//*		echo "Dome controller executable not found"
//*		echo "$DATE - Dome controller executable not found" >> $LOGFILE
//*	fi
//*****************************************************************************

#if defined(_ENABLE_DOME_) || defined(_ENABLE_ROR_)

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>
#include	<math.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"RequestData.h"
#include	"JsonResponse.h"
#include	"eventlogging.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"domedriver.h"

#include	"json_parse.h"
#include	"sendrequest_lib.h"

#if defined(__arm__) && !defined(_ENABLE_PI_HAT_SESNSOR_BOARD_)
	#include <wiringPi.h>
#else
	#define	LOW		0
	#define	HIGH	1
#endif


#define	kRotateDome_CW		0
#define	kRotateDome_CCW		1



#define	kStopRightNow	true
#define	kStopNormal		false





static void		GetStateString(DOME_STATE_TYPE domeState, char *stateString);
static void		DomeControl_GetStatusString(const int status, char *statusString);

//*****************************************************************************
void	CreateDomeObjects(void)
{

	new DomeDriver(0);
}


//*****************************************************************************
//*	dome commands
enum
{
	kCmd_Dome_altitude	=	0,	//*	The dome altitude
	kCmd_Dome_athome,			//*	Indicates whether the dome is in the home position.
	kCmd_Dome_atpark,			//*	Indicates whether the telescope is at the park position
	kCmd_Dome_azimuth,			//*	The dome azimuth
	kCmd_Dome_canfindhome,		//*	Indicates whether the dome can find the home position.
	kCmd_Dome_canpark,			//*	Indicates whether the dome can be parked.
	kCmd_Dome_cansetaltitude,	//*	Indicates whether the dome altitude can be set
	kCmd_Dome_cansetazimuth,	//*	Indicates whether the dome azimuth can be set
	kCmd_Dome_cansetpark,		//*	Indicates whether the dome park position can be set
	kCmd_Dome_cansetshutter,	//*	Indicates whether the dome shutter can be opened
	kCmd_Dome_canslave,			//*	Indicates whether the dome supports slaving to a telescope
	kCmd_Dome_cansyncazimuth,	//*	Indicates whether the dome azimuth position can be synched
	kCmd_Dome_shutterstatus,	//*	Status of the dome shutter or roll-off roof
	kCmd_Dome_slaved,			//*	GET--Indicates whether the dome is slaved to the telescope
								//*	SET--Sets whether the dome is slaved to the telescope
	kCmd_Dome_slewing,			//*	Indicates whether the any part of the dome is moving
	kCmd_Dome_abortslew,		//*	Immediately cancel current dome operation.
	kCmd_Dome_closeshutter,		//*	Close the shutter or otherwise shield telescope from the sky.
	kCmd_Dome_findhome,			//*	Start operation to search for the dome home position.
	kCmd_Dome_openshutter,		//*	Open shutter or otherwise expose telescope to the sky.
	kCmd_Dome_park,				//*	Rotate dome in azimuth to park position.
	kCmd_Dome_setpark,			//*	Set the current azimuth, altitude position of dome to be the park position
	kCmd_Dome_slewtoaltitude,	//*	Slew the dome to the given altitude position.
	kCmd_Dome_slewtoazimuth,	//*	Slew the dome to the given azimuth position.
	kCmd_Dome_synctoazimuth,	//*	Synchronize the current position of the dome to the given azimuth.

	//==============================================================
	//*	extra commands added by MLS
	kCmd_Dome_Extras,
	kCmd_Dome_goleft,			//*	Move the dome left (CCW)
	kCmd_Dome_goright,			//*	Move the dome right (CW)

	kCmd_Dome_bumpleft,			//*	Move the dome left (CCW)
	kCmd_Dome_bumpright,		//*	Move the dome right (CW)

	kCmd_Dome_slowleft,			//*	Move the dome left (CCW)
	kCmd_Dome_slowright,		//*	Move the dome right (CW)

	kCmd_Dome_currentstate,		//*	What is the current state of the state machine

	//*	make this one last for consistency
	kCmd_Dome_readall,			//*	Read all parameters


	kCmd_Dome_last
};



//*****************************************************************************
static TYPE_CmdEntry	gDomeCmdTable[]	=
{

	{	"altitude",			kCmd_Dome_altitude,			kCmdType_GET	},
	{	"athome",			kCmd_Dome_athome,			kCmdType_GET	},
	{	"atpark",			kCmd_Dome_atpark,			kCmdType_GET	},
	{	"azimuth",			kCmd_Dome_azimuth,			kCmdType_GET	},
	{	"canfindhome",		kCmd_Dome_canfindhome,		kCmdType_GET	},
	{	"canpark",			kCmd_Dome_canpark,			kCmdType_GET	},
	{	"cansetaltitude",	kCmd_Dome_cansetaltitude,	kCmdType_GET	},
	{	"cansetazimuth",	kCmd_Dome_cansetazimuth,	kCmdType_GET	},
	{	"cansetpark",		kCmd_Dome_cansetpark,		kCmdType_GET	},
	{	"cansetshutter",	kCmd_Dome_cansetshutter,	kCmdType_GET	},
	{	"canslave",			kCmd_Dome_canslave,			kCmdType_GET	},
	{	"cansyncazimuth",	kCmd_Dome_cansyncazimuth,	kCmdType_GET	},
	{	"shutterstatus",	kCmd_Dome_shutterstatus,	kCmdType_GET	},
	{	"slaved",			kCmd_Dome_slaved,			kCmdType_BOTH	},
	{	"slewing",			kCmd_Dome_slewing,			kCmdType_GET	},
	{	"abortslew",		kCmd_Dome_abortslew,		kCmdType_BOTH	},
	{	"closeshutter",		kCmd_Dome_closeshutter,		kCmdType_PUT	},
	{	"findhome",			kCmd_Dome_findhome,			kCmdType_PUT	},
	{	"openshutter",		kCmd_Dome_openshutter,		kCmdType_PUT	},
	{	"park",				kCmd_Dome_park,				kCmdType_PUT	},
	{	"setpark",			kCmd_Dome_setpark,			kCmdType_PUT	},
	{	"slewtoaltitude",	kCmd_Dome_slewtoaltitude,	kCmdType_PUT	},
	{	"slewtoazimuth",	kCmd_Dome_slewtoazimuth,	kCmdType_PUT	},
	{	"synctoazimuth",	kCmd_Dome_synctoazimuth,	kCmdType_PUT	},

	//==============================================================
	//*	extra commands added by MLS
	{	"--extras",			kCmd_Dome_Extras,			kCmdType_GET	},

#ifndef _ENABLE_ROR_
	{	"goleft",			kCmd_Dome_goleft,			kCmdType_PUT	},
	{	"goright",			kCmd_Dome_goright,			kCmdType_PUT	},

	{	"bumpleft",			kCmd_Dome_bumpleft,			kCmdType_PUT	},
	{	"bumpright",		kCmd_Dome_bumpright,		kCmdType_PUT	},

	{	"slowleft",			kCmd_Dome_slowleft,			kCmdType_PUT	},
	{	"slowright",		kCmd_Dome_slowright,		kCmdType_PUT	},
#endif



	{	"currentstate",		kCmd_Dome_currentstate,		kCmdType_GET	},
	{	"readall",			kCmd_Dome_readall,			kCmdType_GET	},


	{	"",					-1,		0	}
};

//*****************************************************************************
DomeDriver::DomeDriver(const int argDevNum)
	:AlpacaDriver(kDeviceType_Dome)
{

	CONSOLE_DEBUG(__FUNCTION__);

	cShutterstatus			=	kShutterStatus_Closed;
	cAltitude_Degrees		=	0.0;
	cAzimuth_Degrees		=	0.0;
	cAzimuth_Destination	=	-1.0;		//*	must be >= to 0 to be valid
	cParkAzimuth			=	0.0;
	cHomeAzimuth			=	0.0;
	cCurrentPWM				=	0;
	cCurrentDirection		=	kRotateDome_CW;
	cBumpSpeedAmount		=	1;
	cTimeOfLastSpeedChange	=	0;
	cTimeOfMovingStart		=	0;

	cDomeConfig				=	kIsDome;
	cAtHome					=	false;
	cAtPark					=	false;
	cCanSlave				=	false;
	cCanFindHome			=	false;
	cCanPark				=	false;
	cGoingHome				=	false;
	cCanSetAltitude			=	false;
	cCanSetAzimuth			=	false;
	cCanSetPark				=	false;
	cCanSetShutter			=	false;
	cCanSyncAzimuth			=	false;
	cGoingPark				=	false;
	cGoingBump				=	false;
	cManualMove				=	false;
	cSlaved					=	false;
	cSlewing				=	false;

	cDomeState				=	kDomeState_Idle;
	cPreviousDomeState		=	kDomeState_Idle;

	strcpy(cDeviceName,			"Dome");
	strcpy(cDeviceDescription,	"Dome");

	Init_Hardware();

#ifdef _ENABLE_SLIT_TRACKER_REMOTE_
int	iii;

	cSlitTrackerInfoValid			=	false;
	cTimeOfLastSlitTrackerUpdate	=	0;

	for (iii=0; iii<kSensorValueCnt; iii++)
	{
		cSlitDistance[iii].distanceInches	=	0;
		cSlitDistance[iii].validData		=	false;
		cSlitDistance[iii].updated			=	false;
		cSlitDistance[iii].readCount		=	0;
	}

#endif // _ENABLE_SLIT_TRACKER_REMOTE_
#ifdef _ENABLE_REMOTE_SHUTTER_
	cShutterInfoValid				=	false;
	cShutterPort					=	0;
	cTimeOfLastShutterUpdate		=	0;
#endif // _ENABLE_REMOTE_SHUTTER_


	SendDiscoveryQuery();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
DomeDriver::~DomeDriver( void )
{
	StopDomeMoving(kStopRightNow);
}

//*****************************************************************************
void	DomeDriver::Init_Hardware(void)
{

	CONSOLE_DEBUG(__FUNCTION__);
}



//*****************************************************************************
//*	valid commands
//*		http://127.0.0.1:6800/api/v1/dome/altitude
//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::ProcessCommand(TYPE_GetPutRequestData *reqData)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					cmdEnumValue;
int					cmdType;
char				alpacaErrMsg[256];
//int				myDeviceNum;
int					mySocket;


	CONSOLE_DEBUG_W_STR("htmlData\t=",	reqData->htmlData);

	if (strcmp(reqData->deviceCommand, "readall") != 0)
	{
		CONSOLE_DEBUG_W_STR("deviceCommand\t=",	reqData->deviceCommand);
	}

	CheckSensors();

	//*	make local copies of the data structure to make the code easier to read
	mySocket	=	reqData->socket;
//	myDeviceNum	=	reqData->deviceNumber;

	strcpy(alpacaErrMsg, "");

	//*	set up the json response
	JsonResponse_CreateHeader(reqData->jsonTextBuffer, kMaxJsonBuffLen);

	//*	this is not part of the protocol, I am using it for testing
	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Device",
								cDeviceName,
								INCLUDE_COMMA);

	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"Command",
								reqData->deviceCommand,
								INCLUDE_COMMA);

//	JsonResponse_Add_Int32(		mySocket,
//								reqData->jsonTextBuffer,
//								kMaxJsonBuffLen,
//								"PreviousState",
//								domeState,
//								INCLUDE_COMMA);

	//*	look up the command
	cmdEnumValue	=	FindCmdFromTable(reqData->deviceCommand, gDomeCmdTable, &cmdType);
	switch(cmdEnumValue)
	{
		//----------------------------------------------------------------------------------------
		//*	Common commands that we want to over ride
		//----------------------------------------------------------------------------------------
		case kCmd_Common_supportedactions:	//*	Returns the list of action names supported by this driver.
			alpacaErrCode	=	Get_SupportedActions(reqData, gDomeCmdTable);
			break;

		//----------------------------------------------------------------------------------------
		//*	Dome specific commands
		//----------------------------------------------------------------------------------------
		case kCmd_Dome_altitude:			//*	The dome altitude
			alpacaErrCode	=	Get_Altitude(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_athome:				//*	Indicates whether the dome is in the home position.
			alpacaErrCode	=	Get_Athome(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_atpark:				//*	Indicates whether the telescope is at the park position
			alpacaErrCode	=	Get_Atpark(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_azimuth:				//*	The dome azimuth
			alpacaErrCode	=	Get_Azimuth(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_canfindhome:			//*	Indicates whether the dome can find the home position.
			alpacaErrCode	=	Get_Canfindhome(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_canpark:				//*	Indicates whether the dome can be parked.
			alpacaErrCode	=	Get_Canpark(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_cansetaltitude:		//*	Indicates whether the dome altitude can be set
			alpacaErrCode	=	Get_Cansetaltitude(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_cansetazimuth:		//*	Indicates whether the dome azimuth can be set
			alpacaErrCode	=	Get_Cansetazimuth(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_cansetpark:			//*	Indicates whether the dome park position can be set
			alpacaErrCode	=	Get_Cansetpark(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_cansetshutter:		//*	Indicates whether the dome shutter can be opened
			alpacaErrCode	=	Get_Cansetshutter(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_canslave:			//*	Indicates whether the dome supports slaving to a telescope
			alpacaErrCode	=	Get_Canslave(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_cansyncazimuth:		//*	Indicates whether the dome azimuth position can be synched
			alpacaErrCode	=	Get_Cansyncazimuth(reqData, alpacaErrMsg, gValueString);
			break;

		//*	Returns the status of the dome shutter or roll-off roof. 0 = Open, 1 = Closed, 2 = Opening, 3 = Closing, 4 = Shutter status error
		case kCmd_Dome_shutterstatus:		//*	Status of the dome shutter or roll-off roof
			alpacaErrCode	=	Get_Shutterstatus(reqData, alpacaErrMsg, gValueString);
			break;


		case kCmd_Dome_slaved:				//*	GET--Indicates whether the dome is slaved to the telescope
											//*	SET--Sets whether the dome is slaved to the telescope
			if (reqData->get_putIndicator == 'G')
			{
				alpacaErrCode	=	Get_Slaved(reqData, alpacaErrMsg, gValueString);
			}
			else if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_Slaved(reqData, alpacaErrMsg);
			}
			break;

		case kCmd_Dome_slewing:				//*	Indicates whether the any part of the dome is moving
			alpacaErrCode	=	Get_Slewing(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_abortslew:			//*	Immediately cancel current dome operation.
			//*	I want this one to go even if it is a GET command
			alpacaErrCode	=	Put_AbortSlew(reqData, alpacaErrMsg);
			break;

		case kCmd_Dome_closeshutter:		//*	Close the shutter or otherwise shield telescope from the sky.
			alpacaErrCode	=	Put_CloseShutter(reqData, alpacaErrMsg);
			break;

		case kCmd_Dome_findhome:			//*	Start operation to search for the dome home position.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_FindHome(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_openshutter:			//*	Open shutter or otherwise expose telescope to the sky.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_OpenShutter(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_park:				//*	Rotate dome in azimuth to park position.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_Park(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_setpark:				//*	Set the current azimuth, altitude position of dome to be the park position
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SetPark(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_slewtoaltitude:		//*	Slew the dome to the given altitude position.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SlewToAltitude(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_slewtoazimuth:		//*	Slew the dome to the given azimuth position.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SlewToAzimuth(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_synctoazimuth:		//*	Synchronize the current position of the dome to the given azimuth.
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SyncToAzimuth(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_goleft:
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_NormalMove(reqData, alpacaErrMsg, kRotateDome_CCW);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_goright:
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_NormalMove(reqData, alpacaErrMsg, kRotateDome_CW);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_bumpleft:
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_BumpMove(reqData, alpacaErrMsg, kRotateDome_CCW);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_bumpright:
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_BumpMove(reqData, alpacaErrMsg, kRotateDome_CW);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_slowleft:
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SlowMove(reqData, alpacaErrMsg, kRotateDome_CCW);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_slowright:
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_SlowMove(reqData, alpacaErrMsg, kRotateDome_CW);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;


		case kCmd_Dome_currentstate:
			alpacaErrCode	=	Get_Currentstate(reqData, alpacaErrMsg);
			break;

		case kCmd_Dome_readall:
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
	JsonResponse_Add_Int32(		mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ClientTransactionID",
								gClientTransactionID,
								INCLUDE_COMMA);

	JsonResponse_Add_Int32(		mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ServerTransactionID",
								gServerTransactionID,
								INCLUDE_COMMA);

	JsonResponse_Add_Int32(		mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ErrorNumber",
								alpacaErrCode,
								INCLUDE_COMMA);

//	JsonResponse_Add_Int32(		mySocket,
//								reqData->jsonTextBuffer,
//								kMaxJsonBuffLen,
//								"NewState",
//								domeState,
//								INCLUDE_COMMA);

	JsonResponse_Add_String(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"ErrorMessage",
								alpacaErrMsg,
								NO_COMMA);

	JsonResponse_Add_Finish(	mySocket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								kInclude_HTTP_Header);

	//*	this is for the logging function
	strcpy(reqData->alpacaErrMsg, alpacaErrMsg);
	return(alpacaErrCode);
}

//*****************************************************************************
int32_t	DomeDriver::RunStateMachine_Dome(void)
{
int32_t		minDealy_microSecs;
uint32_t	currentMilliSecs;
uint32_t	timeSinceLastWhatever;
int			isAtMax;

	UpdateDomePosition();

	minDealy_microSecs		=	1000;		//*	default to 1 millisecond
	currentMilliSecs		=	millis();
	timeSinceLastWhatever	=	currentMilliSecs - cTimeOfLastSpeedChange;
	switch(cDomeState)
	{
		case kDomeState_Idle:
			break;

		case kDomeState_SpeedingUp:
			if (timeSinceLastWhatever >= 2)
			{
				isAtMax	=	BumpDomeSpeed(cBumpSpeedAmount);
				if (isAtMax)
				{
					cDomeState	=	kDomeState_Moving;
				}
			}
			minDealy_microSecs		=	250;
			break;

		case kDomeState_Moving:
			CheckMoving();
			minDealy_microSecs		=	250;
			break;

		case kDomeState_SlowingDown:
			if (timeSinceLastWhatever >= 1)
			{
				isAtMax	=	BumpDomeSpeed(-cBumpSpeedAmount);
				if (isAtMax)
				{
					cDomeState	=	kDomeState_Stopped;
				}
			}
			minDealy_microSecs		=	250;
			break;

		case kDomeState_Stopped:
			cDomeState				=	kDomeState_Idle;
			minDealy_microSecs		=	250;
			break;

		case kDomeState_Reversing_Slowing:
			CheckSensors();
			if (timeSinceLastWhatever >= 1)
			{
				isAtMax	=	BumpDomeSpeed(-cBumpSpeedAmount);
				if (isAtMax)
				{
					CONSOLE_DEBUG("Changing to kDomeState_Reversing_Waiting");
					cDomeState	=	kDomeState_Reversing_Waiting;
				}
			}
			minDealy_microSecs		=	250;
			break;

		case kDomeState_Reversing_Waiting:
			CheckSensors();
			if (timeSinceLastWhatever >= 30)
			{
				if (cCurrentDirection == kRotateDome_CW)
				{
					CONSOLE_DEBUG("Currently running CW");
					CONSOLE_DEBUG_W_NUM("cDomeState\t=", cDomeState);
					StartDomeMoving(kRotateDome_CCW);
					CONSOLE_DEBUG_W_NUM("cDomeState\t=", cDomeState);
				}
				else
				{
				//	CONSOLE_DEBUG("Currently running CCW");
				}
			}
			minDealy_microSecs		=	250;
			break;

		case kDomeState_last:
		default:
			break;
	}
	return(minDealy_microSecs);
}

//*****************************************************************************
//*	this needs to be over ridden by the sub class
//*****************************************************************************
int32_t	DomeDriver::RunStateMachine_ROR(void)
{
int32_t		minDealy_microSecs;

	CONSOLE_ABORT(__FUNCTION__);
	minDealy_microSecs		=	1000;		//*	default to 1 millisecond

	return(minDealy_microSecs);

}

//*****************************************************************************
//*	return number of microseconds allowed for delay
//*****************************************************************************
int32_t	DomeDriver::RunStateMachine(void)
{
int32_t		minDealy_microSecs;
uint32_t	currentMilliSecs;
uint32_t	timeSinceLastWhatever;
char		stateString[48];

//	CONSOLE_DEBUG(__FUNCTION__);

	CheckDomeButtons();

	if (cDomeConfig == kIsDome)
	{
		minDealy_microSecs	=	RunStateMachine_Dome();
	}
	else
	{
		minDealy_microSecs	=	RunStateMachine_ROR();
	}

	currentMilliSecs		=	millis();

#ifdef _ENABLE_REMOTE_SHUTTER_
	//====================================================================
	//*	check to see if its time to update the shutter status
	timeSinceLastWhatever	=	currentMilliSecs - cTimeOfLastShutterUpdate;
	if (timeSinceLastWhatever > (15 * 1000))
	{
		GetRemoteShutterStatus();
		cTimeOfLastShutterUpdate	=	millis();
	}
#endif // _ENABLE_REMOTE_SHUTTER_

#ifdef _ENABLE_SLIT_TRACKER_REMOTE_
	//====================================================================
	//*	check to see if its time to update the slit tracker
	if (cSlitTrackerInfoValid)
	{
		timeSinceLastWhatever	=	currentMilliSecs - cTimeOfLastSlitTrackerUpdate;
		if (timeSinceLastWhatever > (60 * 1000))
		{
			GetSlitTrackerData();
			cTimeOfLastSlitTrackerUpdate	=	millis();
		}
	}
	else
	{
		//*	if we don't have slit tracker data, check for it every 5 minutes
		timeSinceLastWhatever	=	currentMilliSecs - cTimeOfLastSlitTrackerUpdate;
		if (timeSinceLastWhatever > (5 * 60 * 1000))
		{
			CONSOLE_DEBUG("Calling SendDiscoveryQuery() !!!!!!!!!!!!!!!!!!");
			SendDiscoveryQuery();
			cTimeOfLastSlitTrackerUpdate	=	millis();
		}

	}
#endif // _ENABLE_SLIT_TRACKER_REMOTE_

	//*	has the state changed.
	if (cDomeState != cPreviousDomeState)
	{
		GetStateString(cDomeState, stateString);
		LogEvent("dome", "StateChanged", NULL, kASCOM_Err_Success, stateString);
	}
	cPreviousDomeState	=	cDomeState;
	return(minDealy_microSecs);
}

//**************************************************************************************
void	DomeDriver::ProcessDiscovery(	struct sockaddr_in	*deviceAddress,
										const int			ipPortNumber,
										const char			*deviceType,
										const int			deviceNumber)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceType);
#ifdef _ENABLE_SLIT_TRACKER_REMOTE_
	if (strcasecmp(deviceType, "slittracker") == 0)
	{
		//*	yeah!  we have a slit tracker
		CONSOLE_DEBUG("slittracker!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		cSlitTrackerInfoValid		=	true;
		cSlitTrackerDeviceAddress	=	*deviceAddress;
		cSlitTrackerPort			=	ipPortNumber;
		cSlitTrackerAlpacaDevNum	=	deviceNumber;

//		cCanSlave					=	true;
		LogEvent("dome", "slittracker", NULL, kASCOM_Err_Success, "Discovered");
	}
#endif // _ENABLE_SLIT_TRACKER_REMOTE_


#ifdef _ENABLE_REMOTE_SHUTTER_
	if (strcasecmp(deviceType, "shutter") == 0)
	{
		CONSOLE_DEBUG("Shutter!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		cShutterInfoValid			=	true;
		cShutterDeviceAddress		=	*deviceAddress;
		cShutterPort				=	ipPortNumber;
		cShutterAlpacaDevNum		=	deviceNumber;
		LogEvent("dome", "shutter", NULL, kASCOM_Err_Success, "Discovered");
	}
#endif // _ENABLE_REMOTE_SHUTTER_
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Altitude(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Double(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									responseString,
									cAltitude_Degrees,
									INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Athome(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cAtHome,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Atpark(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cAtPark,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Azimuth(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Double(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cAzimuth_Degrees,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Canfindhome(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCanFindHome,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Indicates whether the dome can be parked.
TYPE_ASCOM_STATUS	DomeDriver::Get_Canpark(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCanPark,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Indicates whether the dome altitude can be set
TYPE_ASCOM_STATUS	DomeDriver::Get_Cansetaltitude(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCanSetAltitude,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Indicates whether the dome park position can be set
TYPE_ASCOM_STATUS	DomeDriver::Get_Cansetazimuth(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCanSetAzimuth,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Indicates whether the dome azimuth can be set
TYPE_ASCOM_STATUS	DomeDriver::Get_Cansetpark(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCanSetPark,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
//*	Indicates whether the dome shutter can be opened
TYPE_ASCOM_STATUS	DomeDriver::Get_Cansetshutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCanSetShutter,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Canslave(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCanSlave,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Indicates whether the dome azimuth position can be synced
TYPE_ASCOM_STATUS	DomeDriver::Get_Cansyncazimuth(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cCanSyncAzimuth,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
//*	Indicates whether the any part of the dome is moving
TYPE_ASCOM_STATUS	DomeDriver::Get_Slewing(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (reqData != NULL)
	{
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cSlewing,
								INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Shutterstatus(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				statusString[32];

	if (reqData != NULL)
	{
		//*	shutter state is an enum defined by Alpaca/ASCOM, defined in alpaca_defs.h
		JsonResponse_Add_Int32(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cShutterstatus,
								INCLUDE_COMMA);

		DomeControl_GetStatusString(cShutterstatus, statusString);

		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"shutterstatus-str",
									statusString,
									INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Slaved(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		alpacaErrCode	=	kASCOM_Err_Success;
		JsonResponse_Add_Bool(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								responseString,
								cSlaved,
								INCLUDE_COMMA);

	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_Slaved(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				argumentString[32];
bool				foundKeyWord;
bool				newSlavedValue;

	CONSOLE_DEBUG(__FUNCTION__);
	if (reqData != NULL)
	{
		if (cCanSlave)
		{
			foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
													"Slaved",
													argumentString,
													(sizeof(argumentString) -1));
			if (foundKeyWord)
			{
				newSlavedValue	=	IsTrueFalse(argumentString);
				if (newSlavedValue)
				{
					if (cCanSlave)
					{
						//*	we can only slave if the shutter is open
						if (cShutterstatus == kShutterStatus_Open)
						{
							cSlaved			=	true;
							alpacaErrCode	=	kASCOM_Err_Success;
						}
						else
						{
							alpacaErrCode	=	kASCOM_Err_InvalidOperation;
							GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Shutter must be open");
							CONSOLE_DEBUG(alpacaErrMsg);
						}
					}
					else
					{
						alpacaErrCode	=	kASCOM_Err_InvalidOperation;
						GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Slave mode not permitted");
						CONSOLE_DEBUG(alpacaErrMsg);
					}
				}
				else
				{
					cSlaved			=	false;
					alpacaErrCode	=	kASCOM_Err_Success;
				}
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid Value");
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not supported");
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}



//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_AbortSlew(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	StopDomeMoving(kStopRightNow);

	if (reqData != NULL)
	{

	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_Park(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				deltaDegrees;
int 				direction;

	if (reqData != NULL)
	{
		CheckSensors();

		cGoingBump	=	false;
		cGoingHome	=	false;

		if (cCanPark)
		{
			if (cAtPark)
			{
				CONSOLE_DEBUG("Already at park, command ignored");
			}
			else if (cSlewing)
			{
				cGoingPark	=	true;
			}
			else
			{
				//*	lets try and figure out which way to go.
				direction		=	kRotateDome_CW;	//*	set a default
				deltaDegrees	=	cAzimuth_Degrees - cParkAzimuth;
				CONSOLE_DEBUG_W_DBL("Distance from park\t=", deltaDegrees);

				if ((deltaDegrees > 0.0) && (deltaDegrees < 180.0))
				{
					direction		=	kRotateDome_CCW;	//*	set a default
				}
				cGoingPark		=	true;

				if (cAtHome)
				{
					StartDomeMoving(kRotateDome_CCW);
				}
				else
				{
					StartDomeMoving(direction);
				}
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Park Not supported");
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_OpenShutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	alpacaErrCode	=	OpenShutter(alpacaErrMsg);

	cShutterstatus	=	kShutterStatus_Opening;

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_CloseShutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cShutterstatus	=	kShutterStatus_Closing;

	alpacaErrCode	=	CloseShutter(alpacaErrMsg);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_FindHome(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					direction;
double				deltaDegrees;

	if (cCanFindHome)
	{
		cGoingBump	=	false;
		cGoingPark	=	false;

		if (cAtHome)
		{
			CONSOLE_DEBUG("Already at home, command ignored");
		}
		else if (cSlewing)
		{
			cGoingHome	=	true;
		}
		else
		{
			cGoingHome	=	true;
			//*	lets try and figure out which way to go.
			direction		=	kRotateDome_CW;	//*	set a default
			deltaDegrees	=	cAzimuth_Degrees - cHomeAzimuth;
			CONSOLE_DEBUG_W_DBL("Distance from home\t=", deltaDegrees);

			if ((deltaDegrees > 0.0) && (deltaDegrees < 180.0))
			{
				direction		=	kRotateDome_CCW;	//*	set a default
			}

			if (cAtPark)
			{
				//*	because my HOME is CW from my PARK
				StartDomeMoving(kRotateDome_CW);
			}
			else
			{
				StartDomeMoving(direction);
			}
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "FindHome Not supported");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_SetPark(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Set park not supported");
	return(kASCOM_Err_NotImplemented);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_SlewToAltitude(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;

	alpacaErrCode	=	kASCOM_Err_NotImplemented;
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SlewToAltitude not finished");

	return(alpacaErrCode);
}


//*****************************************************************************
//*	http://127.0.0.1:6800/api/v1.0.0-oas3/dome/0/slewtoazimuth" -H  "accept: application/json" -H  "Content-Type: application/x-www-form-urlencoded" -d "Azimuth=27&ClientID=1&ClientTransactionID=223"
//*	curl -X PUT "https://virtserver.swaggerhub.com/ASCOMInitiative/ASCOMAlpacaAPI/1.0.0-oas3/dome/0/slewtoazimuth" -H  "accept: application/json" -H  "Content-Type: application/x-www-form-urlencoded" -d "Azimuth=27&ClientID=1&ClientTransactionID=223"
//*	curl -X PUT "http://127.0.0.1:6800/api/v1.0.0-oas3/dome/0/slewtoazimuth" -H  "accept: application/json" -H  "Content-Type: application/x-www-form-urlencoded" -d "Azimuth=27&ClientID=1&ClientTransactionID=223"
//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_SlewToAzimuth(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
double				newAzimuthValue;
double				deltaDegrees;
char				argumentString[64];
bool				foundKeyWord;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cCanSetAzimuth)
	{
		if (reqData != NULL)
		{
			//*	look for Azimuth
			foundKeyWord	=	GetKeyWordArgument(reqData->contentData, "Azimuth", argumentString, 31);
			if (foundKeyWord)
			{
				newAzimuthValue	=	atof(argumentString);
				CONSOLE_DEBUG_W_DBL("newAzimuthValue\t=", newAzimuthValue);
				if ((newAzimuthValue >= 0.0) && (newAzimuthValue <= 360.0))
				{
					cAzimuth_Destination	=	newAzimuthValue;
					deltaDegrees			=	cAzimuth_Destination - cAzimuth_Degrees;
					CONSOLE_DEBUG_W_DBL("deltaDegrees\t=", deltaDegrees);
					if ((deltaDegrees >= 1.0) && (deltaDegrees < 180.0))
					{
						CONSOLE_DEBUG("kRotateDome_CW");
						StartDomeMoving(kRotateDome_CW);
					}
					else if (deltaDegrees <= -1.0)
					{
						CONSOLE_DEBUG("kRotateDome_CCW");
						StartDomeMoving(kRotateDome_CCW);
					}
					else
					{
						CONSOLE_DEBUG("dont bother moving");
						//*	dont bother moving
					}
					alpacaErrCode			=	kASCOM_Err_Success;
				}
				else
				{
					alpacaErrCode	=	kASCOM_Err_InvalidValue;
					GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid Value");
				}
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid Value 'Azimuth=' was not found");
				CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InternalError;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		if (cDomeConfig == kIsRollOffRoof)
		{
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "ROR does not use slewtoazimuth");
		}
		else
		{
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "slewtoazimuth Not implemented");
		}
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_SyncToAzimuth(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;
double				newAzimuthValue;
char				argumentString[64];
bool				foundKeyWord;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cCanSyncAzimuth)
	{
		if (reqData != NULL)
		{
			//*	look for Azimuth
			foundKeyWord	=	GetKeyWordArgument(reqData->contentData, "Azimuth", argumentString, 31);
			if (foundKeyWord)
			{
				newAzimuthValue	=	atof(argumentString);
				CONSOLE_DEBUG_W_DBL("newAzimuthValue\t=", newAzimuthValue);
				if ((newAzimuthValue >= 0.0) && (newAzimuthValue <= 360.0))
				{
					cAzimuth_Degrees	=	newAzimuthValue;
					alpacaErrCode		=	kASCOM_Err_Success;
				}
				else
				{
					alpacaErrCode	=	kASCOM_Err_InvalidValue;
					GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid Value");
				}
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_InvalidValue;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid Value 'Azimuth=' was not found");
				CONSOLE_DEBUG_W_STR("contentData\t=", reqData->contentData);
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InternalError;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		if (cDomeConfig == kIsRollOffRoof)
		{
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "ROR does not use synctoazimuth");
		}
		else
		{
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "synctoazimuth Not implemented");
		}
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_BumpMove(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, int direction)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (cDomeConfig == kIsDome)
	{
		if (reqData != NULL)
		{
			if (cSlewing)
			{
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Dome already in motion, command ignored");
			}
			else
			{
				cGoingBump	=	true;
				StartDomeMoving(direction);
				//*	we started it moving at 50%, make the state machine think its already up to speed
				//*	so that it stays at 50%
				cDomeState	=	kDomeState_Moving;
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InternalError;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "ROR doesnt support dome rotation");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_NormalMove(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, int direction)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (cDomeConfig == kIsDome)
	{
		if (reqData != NULL)
		{
			if (cSlewing)
			{
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Dome already in motion, command ignored");
			}
			else
			{
				StartDomeMoving(direction);
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InternalError;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "ROR doesnt support dome rotation");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_SlowMove(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, int direction)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	if (cDomeConfig == kIsDome)
	{
		if (reqData != NULL)
		{
			if (cSlewing)
			{
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Dome already in motion, command ignored");
			}
			else
			{
				StartDomeMoving(direction);
				//*	we started it moving at 50%, make the state machine think its already up to speed
				//*	so that it stays at 50%
				cDomeState	=	kDomeState_Moving;
			}
		}
		else
		{
			alpacaErrCode	=	kASCOM_Err_InternalError;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "ROR doesnt support dome rotation");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Currentstate(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				stateString[48];

	if (reqData != NULL)
	{
		GetStateString(cDomeState, stateString);
		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"CurrentState",
									stateString,
									INCLUDE_COMMA);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Readall(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				stateString[48];

	if (reqData != NULL)
	{
		//*	do the common ones first
		Get_Readall_Common(	reqData,	alpacaErrMsg);

		Get_Altitude(		reqData,	alpacaErrMsg,	"altitude");
		Get_Athome(			reqData,	alpacaErrMsg,	"athome");
		Get_Atpark(			reqData,	alpacaErrMsg,	"atpark");
		Get_Azimuth(		reqData,	alpacaErrMsg,	"azimuth");
		Get_Canfindhome(	reqData,	alpacaErrMsg,	"canfindhome");
		Get_Canpark(		reqData,	alpacaErrMsg,	"canpark");
		Get_Cansetaltitude(	reqData,	alpacaErrMsg,	"cansetaltitude");
		Get_Cansetazimuth(	reqData,	alpacaErrMsg,	"cansetazimuth");
		Get_Cansetpark(		reqData,	alpacaErrMsg,	"cansetpark");
		Get_Cansetshutter(	reqData,	alpacaErrMsg,	"cansetshutter");
		Get_Canslave(		reqData,	alpacaErrMsg,	"canslave");
		Get_Cansyncazimuth(	reqData,	alpacaErrMsg,	"cansyncazimuth");
		Get_Shutterstatus(	reqData,	alpacaErrMsg,	"shutterstatus");
		Get_Slaved(			reqData,	alpacaErrMsg,	"slaved");
		Get_Slewing(		reqData,	alpacaErrMsg,	"slewing");

		JsonResponse_Add_Int32(	reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"SlewingRate-PWM",
								cCurrentPWM,
								INCLUDE_COMMA);


		//===============================================================
		JsonResponse_Add_String(reqData->socket,
								reqData->jsonTextBuffer,
								kMaxJsonBuffLen,
								"version",
								gFullVersionString,
								INCLUDE_COMMA);


		GetStateString(cDomeState, stateString);
		JsonResponse_Add_String(	reqData->socket,
									reqData->jsonTextBuffer,
									kMaxJsonBuffLen,
									"CurrentState",
									stateString,
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
//*	this can be over ridden
//*	for anything other than the remote shutter it must be over ridden
//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::OpenShutter(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _ENABLE_REMOTE_SHUTTER_
	if (cShutterInfoValid)
	{
		alpacaErrCode	=	OpenRemoteShutter(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Remote shutter not detected");
	}
#else
	CONSOLE_ABORT(__FUNCTION__);
	alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Open shutter not implemented");

#endif // _ENABLE_REMOTE_SHUTTER_
	return(alpacaErrCode);
}

//*****************************************************************************
//*	this should be over ridden
TYPE_ASCOM_STATUS	DomeDriver::CloseShutter(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
#ifdef _ENABLE_REMOTE_SHUTTER_
	if (cShutterInfoValid)
	{
		alpacaErrCode	=	CloseRemoteShutter(alpacaErrMsg);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Remote shutter not detected");
	}
#else
	alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Close shutter not implemented");
#endif // _ENABLE_REMOTE_SHUTTER_
	return(alpacaErrCode);
}

//*****************************************************************************
void	DomeDriver::OutputHTML(		TYPE_GetPutRequestData *reqData)
{
int			mySocketFD;
char		lineBuffer[128];
char		domeConfigStr[32];

	CONSOLE_DEBUG(__FUNCTION__);

	CheckSensors();

	mySocketFD	=	reqData->socket;

	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H2>Dome</H2>\r\n");

	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

	//*-----------------------------------------------------------
	switch(cDomeConfig)
	{
		case kIsDome:			strcpy(domeConfigStr, "Dome");			break;
		case kIsRollOffRoof:	strcpy(domeConfigStr, "Roll Off Roof");	break;
		default:				strcpy(domeConfigStr, "Invalid");		break;
	}
	sprintf(lineBuffer,	"\t<TR><TD>Configuration</TD><TD>%s</TD></TR>\r\n",		domeConfigStr);
	SocketWriteData(mySocketFD,	lineBuffer);

	//*-----------------------------------------------------------
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"\t<TD>Shutter status:</TD><TD>");
	DomeControl_GetStatusString(cShutterstatus, lineBuffer);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TD></TR>\r\n");


	//*-----------------------------------------------------------
	sprintf(lineBuffer,	"\t<TR><TD>At home</TD><TD>%s</TD></TR>\r\n",		(cAtHome)	? "Yes" : "No");
	SocketWriteData(mySocketFD,	lineBuffer);

	//*-----------------------------------------------------------
	sprintf(lineBuffer,	"\t<TR><TD>At park</TD><TD>%s</TD></TR>\r\n",		(cAtPark)	? "Yes" : "No");
	SocketWriteData(mySocketFD,	lineBuffer);

	//*-----------------------------------------------------------
	sprintf(lineBuffer,	"\t<TR><TD>Slewing</TD><TD>%s</TD></TR>\r\n",		(cSlewing)	? "Yes" : "No");
	SocketWriteData(mySocketFD,	lineBuffer);

	//*-----------------------------------------------------------
	sprintf(lineBuffer,	"\t<TR><TD>Altitude</TD><TD>%1.2f</TD></TR>\r\n",	cAltitude_Degrees);
	SocketWriteData(mySocketFD,	lineBuffer);

	//*-----------------------------------------------------------
	sprintf(lineBuffer,	"\t<TR><TD>Azimuth</TD><TD>%1.2f</TD></TR>\r\n",	cAzimuth_Degrees);
	SocketWriteData(mySocketFD,	lineBuffer);


	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	SocketWriteData(mySocketFD,	"<P>\r\n");


	SocketWriteData(mySocketFD,	"</CENTER>\r\n");

	//*	now generate links to all of the commands
	GenerateHTMLcmdLinkTable(mySocketFD, "dome", 0, gDomeCmdTable);

	CONSOLE_DEBUG("exit");

}

//*****************************************************************************
bool	DomeDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gDomeCmdTable, getPut);
	return(foundIt);
}

#pragma mark -


//*****************************************************************************
static void	GetStateString(DOME_STATE_TYPE domeState, char *stateString)
{
	switch(domeState)
	{
		case kDomeState_Idle:				strcpy(stateString,	"Idle");				break;
		case kDomeState_SpeedingUp:			strcpy(stateString,	"SpeedingUp");			break;
		case kDomeState_Moving:				strcpy(stateString,	"Moving");				break;
		case kDomeState_SlowingDown:		strcpy(stateString,	"SlowingDown");			break;
		case kDomeState_Stopped:			strcpy(stateString,	"Stopped");				break;
		case kDomeState_Reversing_Slowing:	strcpy(stateString,	"Reversing_Slowing");	break;
		case kDomeState_Reversing_Waiting:	strcpy(stateString,	"Reversing_Waiting");	break;
		case kDomeState_last:
		default:							strcpy(stateString,	"last");				break;
	}
}


//*****************************************************************************
static void	DomeControl_GetStatusString(const int status, char *statusString)
{
	switch(status)
	{
		case kShutterStatus_Open:		strcpy(statusString,	"Open");	break;
		case kShutterStatus_Closed:		strcpy(statusString,	"Closed");	break;
		case kShutterStatus_Opening:	strcpy(statusString,	"Opening");	break;
		case kShutterStatus_Closing:	strcpy(statusString,	"Closing");	break;
		case kShutterStatus_Error:		strcpy(statusString,	"Error");	break;
		default:						strcpy(statusString,	"unknown");	break;
	}
}


//*****************************************************************************
//*	returns true if at max or min
//*	returns false otherwise
//*****************************************************************************
bool	DomeDriver::BumpDomeSpeed(const int howMuch)
{
bool		returnState;

	returnState			=	false;

	return(returnState);
}



//*****************************************************************************
//*	this should be over ridden
//*****************************************************************************
void	DomeDriver::StartDomeMoving(const int direction)
{
	CONSOLE_DEBUG(__FUNCTION__);
	cTimeOfLastSpeedChange	=	millis();
	cTimeOfMovingStart		=	millis();
}


//*****************************************************************************
//*	this should be over ridden
//*****************************************************************************
void	DomeDriver::StopDomeMoving(bool rightNow)
{
	cAzimuth_Destination	=	-1;
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
//*	this should be over ridden
void	DomeDriver::CheckSensors(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
//*	this should be over ridden
void	DomeDriver::CheckDomeButtons(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

}

//*****************************************************************************
void	DomeDriver::ProcessButtonPressed(const int pressedButton)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
//*	this should be over ridden
void	DomeDriver::UpdateDomePosition(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

}


//*****************************************************************************
void	DomeDriver::CheckMoving(void)
{
uint32_t	currentMilliSecs;
uint32_t	movingTime_millisecs;
uint32_t	movingTime_seconds;

//	CONSOLE_DEBUG(__FUNCTION__);

	currentMilliSecs		=	millis();
	movingTime_millisecs	=	currentMilliSecs - cTimeOfMovingStart;
	movingTime_seconds		=	movingTime_millisecs / 1000;

//	CONSOLE_DEBUG_W_NUM("currentMilliSecs\t=",		currentMilliSecs);
//	CONSOLE_DEBUG_W_NUM("timeOfMovingStart\t=",		timeOfMovingStart);
//	CONSOLE_DEBUG_W_NUM("movingTime_millisecs\t=",	movingTime_millisecs);

	CheckSensors();

	//****************************************************
	//*	are we going home?
	if (cGoingHome && cAtHome)
	{
		CONSOLE_DEBUG("Stopping at Home");
		StopDomeMoving(kStopNormal);
		cGoingHome	=	false;
	}

	//****************************************************
	//*	are going to the park position?
	if (cGoingPark)
	{
		if (cAtPark)
		{
			CONSOLE_DEBUG("Stopping at Park");
			StopDomeMoving(kStopNormal);
			cGoingPark	=	false;
		}
		else if (cAtHome && (cCurrentDirection == kRotateDome_CW))
		{
			//*	the home sensor is to the right (CW) from the park sensor
			//*	if we are trying to go to park, and we get to home, we are going
			//*	the wrong direction
			CONSOLE_DEBUG("Changing to kDomeState_Reversing_Slowing");
			cDomeState	=	kDomeState_Reversing_Slowing;
		}
	}

	//****************************************************
	if (cGoingBump && (movingTime_millisecs >= 2500))
	{
		CONSOLE_DEBUG("Stop Bumping");
		StopDomeMoving(kStopNormal);
		cGoingBump	=	false;
	}

	//*	check if we are slewing to a destination
	if (cAzimuth_Destination >= 0.0)
	{
    double	deltaDegrees;

		deltaDegrees	=	fabs(cAzimuth_Destination - cAzimuth_Degrees);


		if (deltaDegrees < 0.10)
		{
			CONSOLE_DEBUG("kStopRightNow");
			StopDomeMoving(kStopRightNow);
		}
		else if (deltaDegrees < 0.25)
		{
			StopDomeMoving(kStopNormal);
		}
		else if (deltaDegrees < 5.0)
		{
			CONSOLE_DEBUG_W_DBL("Slewing, left to go\t=", deltaDegrees);
			CONSOLE_DEBUG_W_NUM("Slowing down", cCurrentPWM);
			if (cCurrentPWM > 500)
			{
				BumpDomeSpeed(-10);
				usleep(500);
			}
		}
		//*	we might have already stopped
		if (cSlewing)
		{
			//*	now check to see if we have gone too far
			if (cCurrentDirection == kRotateDome_CW)
			{
				if (cAzimuth_Degrees > cAzimuth_Destination)
				{
					CONSOLE_DEBUG("Went too far");
					StopDomeMoving(kStopRightNow);
				}
			}
			else if (cCurrentDirection == kRotateDome_CCW)
			{
				if (cAzimuth_Degrees < cAzimuth_Destination)
				{
					CONSOLE_DEBUG("Went too far");
					StopDomeMoving(kStopRightNow);
				}
			}
			else
			{
				CONSOLE_DEBUG("We dont know which way we are going!!!!!!!!!!");
			}
		}
	}

	//*	if we are in manual move and we have been moving for at least 2 seconds,
	//*	then stop at the next stop point.
	if (cManualMove && (movingTime_seconds >= 2))
	{
		//*	manual move will stop at the next stop point
		if (cAtHome || cAtPark)
		{
			StopDomeMoving(kStopNormal);
			CONSOLE_DEBUG_W_NUM("cAtHome\t=",	cAtHome);
			CONSOLE_DEBUG_W_NUM("cAtPark\t=",	cAtPark);
			CONSOLE_DEBUG("Stopping at stop point, move time > 2 secs");
			LogEvent(	"dome",
						"Stopping",
						NULL,
						kASCOM_Err_Success,
						"Stopping at stop point, move time > 2 secs");
		}
	}


	//****************************************************
	//*	as a last resort, if the dome has been moving for 2 minutes and 10 seconds, STOP
	//*	it takes about 2 minutes 4 seconds do a complete revolution
	if (movingTime_seconds >= ((2 * 60) + 10))
	{
		CONSOLE_DEBUG("2 minute waring");
		CONSOLE_DEBUG_W_NUM("movingTime_seconds\t=", movingTime_seconds);
		StopDomeMoving(kStopNormal);
	}
}

#ifdef _ENABLE_SLIT_TRACKER_REMOTE_
//*****************************************************************************
void	DomeDriver::GetSlitTrackerData(void)
{
SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
int				jjj;
int				clockValue;
char			clockString[32];
double			inchValue;

	CONSOLE_DEBUG(__FUNCTION__);

	SJP_Init(&jsonParser);
	sprintf(alpacaString,	"/api/v1/%s/%d/%s", "slittracker", cSlitTrackerAlpacaDevNum, "readall");

	validData	=	GetJsonResponse(	&cSlitTrackerDeviceAddress,
										cSlitTrackerPort,
										alpacaString,
										NULL,
										&jsonParser);
	if (validData)
	{
		if (cCanSlave == false)
		{
			//*	change of state from off line to on line
			LogEvent("dome",	"slittracker",	NULL,	kASCOM_Err_Success,	"Online");
		}

		cCanSlave	=	true;
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
//			CONSOLE_DEBUG_W_STR("keyword\t=",	jsonParser.dataList[jjj].keyword);
//			CONSOLE_DEBUG_W_STR(	jsonParser.dataList[jjj].valueString)
			//	"sensor-0":28.310000,

			if (strncasecmp(jsonParser.dataList[jjj].keyword, "sensor-", 6) == 0)
			{
				strcpy(clockString, &jsonParser.dataList[jjj].keyword[7]);
				clockValue	=	atoi(clockString);
				inchValue	=	atof(jsonParser.dataList[jjj].valueString);
				if ((clockValue >= 0) && (clockValue < kSensorValueCnt))
				{
					cSlitDistance[clockValue].distanceInches	=	inchValue;
					cSlitDistance[clockValue].validData			=	true;
					cSlitDistance[clockValue].updated			=	true;
					cSlitDistance[clockValue].readCount++;

//					CONSOLE_DEBUG_W_DBL("distanceInches\t=",	inchValue);
				}
				else
				{
					CONSOLE_DEBUG_W_NUM("clockValue out of bounds\t=",	clockValue);
				}
			}
		}
	}
	else
	{
		if (cCanSlave)
		{
			//*	change of state from on line to off line
			LogEvent("dome",	"slittracker",	NULL,	kASCOM_Err_Success,	"Offline");
		}
		cCanSlave	=	false;
		CONSOLE_DEBUG("Failed to get data from slit tracker");
	}


}
#endif // _ENABLE_SLIT_TRACKER_REMOTE_


#endif	//	defined(_ENABLE_DOME_) || defined(_ENABLE_ROR_)


