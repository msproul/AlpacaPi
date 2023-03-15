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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 12,	2019	<MLS> Created domedriver.c
//*	Apr 12,	2019	<MLS> Started on Dome control
//*	Apr 14,	2019	<MLS> Moved dome specific code to separate file
//*	May  6,	2019	<MLS> Working on uniform implementation
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
//*	Jan 24,	2021	<MLS> Converted Domedriver to use properties struct
//*	Jan 27,	2021	<MLS> Added Added Put_PowerOn(), Put_PowerOff() & SetPower()
//*	Jan 27,	2021	<MLS> Added Added Put_AuxiliaryOn(), Put_AuxiliaryOff() & SetAuxiliary
//*	Feb 25,	2021	<MLS> Added remote shutter support to Put_AbortSlew()
//*	Jun 15,	2021	<MLS> Added powerStatus and auxiliaryStatus commands
//*	Jun 15,	2021	<MLS> Added Get_PowerStatus() & Get_AuxiliaryStatus()
//*	Jun 23,	2021	<MLS> Updated dome driver cCommonProp.InterfaceVersion to 2
//*	Dec 13,	2021	<MLS> Added WatchDog_TimeOut() to domedriver
//*	May 24,	2022	<MLS> Updated dome driver to count output data
//*	Aug 29,	2022	<MLS> Added logic to keep track of last time any movement was made
//*	Mar  9,	2023	<MLS> Removed all slit tracker code from dome driver
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

#if defined(_ENABLE_DOME_) || defined(_ENABLE_DOME_ROR_)

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

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"domedriver.h"

#ifdef _ENABLE_DOME_RPI_
	#include	"domedriver_rpi.h"
#endif
#ifdef	_ENABLE_DOME_ROR_
	#include	"domedriver_ror_rpi.h"
#endif
#ifdef _ENABLE_DOME_SIMULATOR_
	#include	"domedriver_sim.h"
#endif


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

//*****************************************************************************
void	CreateDomeObjects(void)
{
#ifdef	_ENABLE_DOME_ROR_
	CreateDomeObjectsROR();
#endif
#ifdef _ENABLE_DOME_RPI_
	CreateDomeObjectsRPi();
#endif

#ifdef _ENABLE_DOME_SIMULATOR_
	CreateDomeObjectsSIM();
#endif

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

	kCmd_Dome_poweron,			//*	Turn dome power on
	kCmd_Dome_poweroff,			//*	Turn dome power off
	kCmd_Dome_powerstatus,		//*	Return power status

	kCmd_Dome_auxiliaryon,		//*	Turn auxiliary   on
	kCmd_Dome_auxiliaryoff,		//*	Turn auxiliary off
	kCmd_Dome_auxiliarystatus,	//*	Return auxiliary status


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


	{	"poweron",			kCmd_Dome_poweron,			kCmdType_PUT	},
	{	"poweroff",			kCmd_Dome_poweroff,			kCmdType_PUT	},
	{	"powerstatus",		kCmd_Dome_powerstatus,		kCmdType_GET	},

	{	"auxiliaryon",		kCmd_Dome_auxiliaryon,		kCmdType_PUT	},
	{	"auxiliaryoff",		kCmd_Dome_auxiliaryoff,		kCmdType_PUT	},
	{	"auxiliarystatus",  kCmd_Dome_auxiliarystatus,	kCmdType_GET	},


#ifndef _ENABLE_DOME_ROR_
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

	strcpy(cCommonProp.Name,		"Dome");
	strcpy(cCommonProp.Description,	"Generic Dome");
	cCommonProp.InterfaceVersion	=	2;
	cDriverCmdTablePtr				=	gDomeCmdTable;


	cDomeConfig						=	kIsDome;
	cAzimuth_Destination			=	-1.0;		//*	must be >= to 0 to be valid
	cParkAzimuth					=	0.0;
	cHomeAzimuth					=	0.0;
	cCurrentPWM						=	0;
	cCurrentDirection				=	kRotateDome_CW;
	cBumpSpeedAmount				=	1;
	cTimeOfLastSpeedChange			=	0;
	cTimeOfMovingStart				=	0;

	cTimeOfLastMoveCmd				=	time(NULL);		//*	these need to be set or it will do a shutdown before it even starts
	cTimeOfLastMoveCheck			=	time(NULL);
	cEnableIdleMoveTimeout			=	true;
	cIdleMoveTimeoutMinutes			=	2 * 60;

	//*	clear out all of the properties data
	memset(&cDomeProp, 0, sizeof(TYPE_DomeProperties));
	cDomeProp.ShutterStatus		=	kShutterStatus_Closed;

	cGoingHome					=	false;
	cGoingPark					=	false;
	cGoingBump					=	false;
	cManualMove					=	false;

	cDomeState					=	kDomeState_Idle;
	cPreviousDomeState			=	kDomeState_Idle;

	strcpy(cCommonProp.Name,		"Dome");
	strcpy(cCommonProp.Description,	"Dome");

	cUUID.part3					=	'DO';				//*	model number

	Init_Hardware();

#ifdef _ENABLE_REMOTE_SHUTTER_
	cShutterInfoValid				=	false;
	cShutterPort					=	0;
	cTimeOfLastShutterUpdate		=	0;
#endif // _ENABLE_REMOTE_SHUTTER_

#if defined(_ENABLE_REMOTE_SHUTTER_) || defined(_TEST_DISCOVERQUERY_)
	SendDiscoveryQuery();
#endif
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


//	CONSOLE_DEBUG_W_STR("htmlData\t=",	reqData->htmlData);

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
	JsonResponse_CreateHeader(reqData->jsonTextBuffer);

	//*	this is not part of the protocol, I am using it for testing
	cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(	mySocket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"Device",
															cCommonProp.Name,
															INCLUDE_COMMA);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(	mySocket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"Command",
															reqData->deviceCommand,
															INCLUDE_COMMA);

//	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Int32(		mySocket,
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

		case kCmd_Dome_poweron:
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_PowerOn(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_poweroff:
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_PowerOff(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_powerstatus:
			alpacaErrCode	=	Get_PowerStatus(reqData, alpacaErrMsg, gValueString);
			break;

		case kCmd_Dome_auxiliaryon:
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_AuxiliaryOn(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_auxiliaryoff:
			if (reqData->get_putIndicator == 'P')
			{
				alpacaErrCode	=	Put_AuxiliaryOff(reqData, alpacaErrMsg);
			}
			else
			{
				alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported");
			}
			break;

		case kCmd_Dome_auxiliarystatus:
			alpacaErrCode	=	Get_AuxiliaryStatus(reqData, alpacaErrMsg, gValueString);
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
	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Int32(		mySocket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"ClientTransactionID",
															gClientTransactionID,
															INCLUDE_COMMA);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Int32(		mySocket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"ServerTransactionID",
															gServerTransactionID,
															INCLUDE_COMMA);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Int32(		mySocket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"ErrorNumber",
															alpacaErrCode,
															INCLUDE_COMMA);

//	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Int32(		mySocket,
//								reqData->jsonTextBuffer,
//								kMaxJsonBuffLen,
//								"NewState",
//								domeState,
//								INCLUDE_COMMA);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(	mySocket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"ErrorMessage",
															alpacaErrMsg,
															NO_COMMA);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Finish(	mySocket,
															reqData->jsonTextBuffer,
															(cHttpHeaderSent == false));

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
char		stateString[48];
time_t		currentTimeEpoch;
time_t		deltaSeconds;

#if defined(_ENABLE_REMOTE_SHUTTER_)
	uint32_t	currentMilliSecs;
	uint32_t	timeSinceLastWhatever;
#endif


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


#ifdef _ENABLE_REMOTE_SHUTTER_
	//====================================================================
	//*	check to see if its time to update the shutter status
	if (cShutterInfoValid)
	{
		currentMilliSecs		=	millis();
		timeSinceLastWhatever	=	currentMilliSecs - cTimeOfLastShutterUpdate;
		if (timeSinceLastWhatever > (15 * 1000))
		{
			GetRemoteShutterStatus();
			cTimeOfLastShutterUpdate	=	millis();
		}
	}
#endif // _ENABLE_REMOTE_SHUTTER_


	//----------------------------------------------------------
	//*	check for dome activity, this is in addition to the watchdog stuff
	if (cEnableIdleMoveTimeout)
	{
	char	alpacaErrMsg[128];
	char	msgString[64];
	int		deltaMinutes;
	int		hours;
	int		minutes;
	int		seconds;

		currentTimeEpoch	=	time(NULL);
		deltaSeconds		=	currentTimeEpoch - cTimeOfLastMoveCheck;
		if (deltaSeconds >= 15)
		{
			//*	compute how long it has been since a move command was received,
			//*	this can be dome rotation OR shutter movement
			deltaSeconds		=	currentTimeEpoch - cTimeOfLastMoveCmd;
			deltaMinutes		=	deltaSeconds / 60;

			//*	are we past the set time out for dome movement
			if (deltaMinutes >= cIdleMoveTimeoutMinutes)
			{
				if ((cDomeProp.AtPark == false) || (cDomeProp.ShutterStatus != kShutterStatus_Closed))
				{
					minutes			=	deltaSeconds / 60;
					hours			=	minutes / 60;
					minutes			=	minutes % 60;
					seconds			=	deltaSeconds % 60;
					sprintf(msgString, "Time since last move command %d:%02d:%02d", hours, minutes, seconds);
					CONSOLE_DEBUG(msgString);
					LogEvent("dome",	"WatchDog:Timeout",				NULL,		kASCOM_Err_Success,	NULL);
					LogEvent("dome",	"WatchDog:Shutting down dome",	msgString,	kASCOM_Err_Success,	msgString);

					if (cDomeProp.AtPark == false)
					{
						LogEvent("dome",	"Shutdown - moving to park",	"",	kASCOM_Err_Success,	"");
						Put_Park(NULL, alpacaErrMsg);
					}
					if (cDomeProp.ShutterStatus != kShutterStatus_Closed)
					{
						WatchDog_TimeOut();
					}
				}
			}
			cTimeOfLastMoveCheck			=	time(NULL);	//*	reset the check timer
		}
	}

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
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceType);


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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Double(	reqData->socket,
																reqData->jsonTextBuffer,
																kMaxJsonBuffLen,
																responseString,
																cDomeProp.Altitude,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.AtHome,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.AtPark,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Double(reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.Azimuth,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.CanFindHome,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.CanPark,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.CanSetAltitude,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.CanSetAzimuth,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.CanSetPark,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.CanSetShutter,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.CanSlave,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.CanSyncAzimuth,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.Slewing,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Int32(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.ShutterStatus,
															INCLUDE_COMMA);

		GetDomeShutterStatusString(cDomeProp.ShutterStatus, statusString);

		cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(	reqData->socket,
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.Slaved,
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
		if (cDomeProp.CanSlave)
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
					if (cDomeProp.CanSlave)
					{
						//*	we can only slave if the shutter is open
						if (cDomeProp.ShutterStatus == kShutterStatus_Open)
						{
							cDomeProp.Slaved	=	true;
							alpacaErrCode		=	kASCOM_Err_Success;
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
					cDomeProp.Slaved	=	false;
					alpacaErrCode		=	kASCOM_Err_Success;
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

#ifdef _ENABLE_REMOTE_SHUTTER_
	if (cShutterInfoValid)
	{
		alpacaErrCode	=	StopRemoteShutter(alpacaErrMsg);
	}
#endif // _ENABLE_REMOTE_SHUTTER_

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_Park(		TYPE_GetPutRequestData *reqDataQQ, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				deltaDegrees;
int 				direction;

	CONSOLE_DEBUG(__FUNCTION__);
	cTimeOfLastMoveCmd	=	time(NULL);
	CheckSensors();

	cGoingBump	=	false;
	cGoingHome	=	false;

	if (cDomeProp.CanPark)
	{
		if (cDomeProp.AtPark)
		{
			CONSOLE_DEBUG("Already at park, command ignored");
		}
		else if (cDomeProp.Slewing)
		{
			cGoingPark	=	true;
		}
		else
		{
			//*	lets try and figure out which way to go.
			deltaDegrees	=	cDomeProp.Azimuth - cParkAzimuth;
			CONSOLE_DEBUG_W_DBL("Distance from park\t=", deltaDegrees);

			if ((deltaDegrees > 0.0) && (deltaDegrees < 180.0))
			{
				direction		=	kRotateDome_CCW;
			}
			else
			{
				direction		=	kRotateDome_CW;
			}
			cGoingPark			=	true;
			StartDomeMoving(direction);
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Park Not supported");
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_OpenShutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	cTimeOfLastMoveCmd		=	time(NULL);
	alpacaErrCode			=	OpenShutter(alpacaErrMsg);

	cDomeProp.ShutterStatus	=	kShutterStatus_Opening;

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_CloseShutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	cTimeOfLastMoveCmd		=	time(NULL);
	cDomeProp.ShutterStatus	=	kShutterStatus_Closing;

	alpacaErrCode			=	CloseShutter(alpacaErrMsg);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_FindHome(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					direction;
double				deltaDegrees;

	cTimeOfLastMoveCmd		=	time(NULL);
	if (cDomeProp.CanFindHome)
	{
		cGoingBump	=	false;
		cGoingPark	=	false;

		if (cDomeProp.AtHome)
		{
			CONSOLE_DEBUG("Already at home, command ignored");
		}
		else if (cDomeProp.Slewing)
		{
			cGoingHome	=	true;
		}
		else
		{
			cGoingHome	=	true;
			//*	lets try and figure out which way to go.
			direction		=	kRotateDome_CW;	//*	set a default
			deltaDegrees	=	cDomeProp.Azimuth - cHomeAzimuth;
			CONSOLE_DEBUG_W_DBL("Distance from home\t=", deltaDegrees);

			if ((deltaDegrees > 0.0) && (deltaDegrees < 180.0))
			{
				direction		=	kRotateDome_CCW;	//*	set a default
			}

			StartDomeMoving(direction);
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
	cTimeOfLastMoveCmd		=	time(NULL);
	if (cDomeProp.CanSetAzimuth)
	{
		if (reqData != NULL)
		{
			//*	look for Azimuth
			foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
													"Azimuth",
													argumentString,
													(sizeof(argumentString) -1),
													kArgumentIsNumeric);
			if (foundKeyWord)
			{
				newAzimuthValue	=	AsciiToDouble(argumentString);
				CONSOLE_DEBUG_W_DBL("newAzimuthValue\t=", newAzimuthValue);
				if ((newAzimuthValue >= 0.0) && (newAzimuthValue <= 360.0))
				{
					cAzimuth_Destination	=	newAzimuthValue;
					deltaDegrees			=	cAzimuth_Destination - cDomeProp.Azimuth;
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
	cTimeOfLastMoveCmd		=	time(NULL);
	if (cDomeProp.CanSyncAzimuth)
	{
		if (reqData != NULL)
		{
			//*	look for Azimuth
			foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
													"Azimuth",
													argumentString,
													(sizeof(argumentString) -1),
													kArgumentIsNumeric);
			if (foundKeyWord)
			{
				newAzimuthValue	=	AsciiToDouble(argumentString);
				CONSOLE_DEBUG_W_DBL("newAzimuthValue\t=", newAzimuthValue);
				if ((newAzimuthValue >= 0.0) && (newAzimuthValue <= 360.0))
				{
					cDomeProp.Azimuth	=	newAzimuthValue;
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

	cTimeOfLastMoveCmd		=	time(NULL);
	if (cDomeConfig == kIsDome)
	{
		if (reqData != NULL)
		{
			if (cDomeProp.Slewing)
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

	cTimeOfLastMoveCmd				=	time(NULL);

	if (cDomeConfig == kIsDome)
	{
		if (reqData != NULL)
		{
			if (cDomeProp.Slewing)
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

	cTimeOfLastMoveCmd				=	time(NULL);

	if (cDomeConfig == kIsDome)
	{
		if (reqData != NULL)
		{
			if (cDomeProp.Slewing)
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
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(	reqData->socket,
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
TYPE_ASCOM_STATUS	DomeDriver::Put_PowerOn(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

	alpacaErrCode	=	SetPower(true);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_PowerOff(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

	alpacaErrCode	=	SetPower(false);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_PowerStatus(TYPE_GetPutRequestData *reqData,
												char					*alpacaErrMsg,
												const char				*responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				powerState;

	if (reqData != NULL)
	{
		powerState		=	false;
		alpacaErrCode	=	GetPower(&powerState);
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(	reqData->socket,
																reqData->jsonTextBuffer,
																kMaxJsonBuffLen,
																responseString,
																(powerState ? "ON" : "OFF"),
																INCLUDE_COMMA);

		if (alpacaErrCode == kASCOM_Err_PropertyNotImplemented)
		{
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Property Not Implemented");
		}
		else if (alpacaErrCode != kASCOM_Err_Success)
		{
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Power state unknown");
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_InternalError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_AuxiliaryOn(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

	alpacaErrCode	=	SetAuxiliary(true);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_AuxiliaryOff(TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode;

	alpacaErrCode	=	SetAuxiliary(false);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_AuxiliaryStatus(TYPE_GetPutRequestData *reqData,
												char					*alpacaErrMsg,
												const char				*responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				auxiliaryState;

	if (reqData != NULL)
	{
		auxiliaryState	=	false;
		alpacaErrCode	=	GetAuxiliary(&auxiliaryState);
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(	reqData->socket,
																reqData->jsonTextBuffer,
																kMaxJsonBuffLen,
																responseString,
																(auxiliaryState ? "ON" : "OFF"),
																INCLUDE_COMMA);

		if (alpacaErrCode == kASCOM_Err_PropertyNotImplemented)
		{
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Property Not Implemented");
		}
		else if (alpacaErrCode != kASCOM_Err_Success)
		{
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Auxiliary state unknown");
		}
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
		Get_PowerStatus(	reqData,	alpacaErrMsg,	"powerstatus");
		Get_AuxiliaryStatus(reqData,	alpacaErrMsg,	"auxiliarystatus");

		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Int32(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"SlewingRate-PWM",
															cCurrentPWM,
															INCLUDE_COMMA);


		//===============================================================
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"version",
															gFullVersionString,
															INCLUDE_COMMA);


		GetStateString(cDomeState, stateString);
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"CurrentState",
															stateString,
															INCLUDE_COMMA);

		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"Idle Timeout Enabled",
															cEnableIdleMoveTimeout,
															INCLUDE_COMMA);

		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Int32(reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"Idle Timeout (minutes)",
															cIdleMoveTimeoutMinutes,
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
TYPE_ASCOM_STATUS	DomeDriver::SetPower(bool onOffFlag)
{
	//*	this routine is meant to be over-ridden
	return(kASCOM_Err_MethodNotImplemented);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::GetPower(bool *onOffFlag)
{
	//*	this routine is meant to be over-ridden
	return(kASCOM_Err_MethodNotImplemented);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::SetAuxiliary(bool onOffFlag)
{
	//*	this routine is meant to be over-ridden
	return(kASCOM_Err_MethodNotImplemented);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::GetAuxiliary(bool *onOffFlag)
{
	//*	this routine is meant to be over-ridden
	return(kASCOM_Err_MethodNotImplemented);
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
	alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Open shutter not implemented");

#endif // _ENABLE_REMOTE_SHUTTER_
	return(alpacaErrCode);
}

//*****************************************************************************
//*	this can be over ridden
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
	GetDomeShutterStatusString(cDomeProp.ShutterStatus, lineBuffer);
	SocketWriteData(mySocketFD,	lineBuffer);
	SocketWriteData(mySocketFD,	"</TD></TR>\r\n");

	//*-----------------------------------------------------------
	sprintf(lineBuffer,	"\t<TR><TD>At home</TD><TD>%s</TD></TR>\r\n",		(cDomeProp.AtHome)	? "Yes" : "No");
	SocketWriteData(mySocketFD,	lineBuffer);

	//*-----------------------------------------------------------
	sprintf(lineBuffer,	"\t<TR><TD>At park</TD><TD>%s</TD></TR>\r\n",		(cDomeProp.AtPark)	? "Yes" : "No");
	SocketWriteData(mySocketFD,	lineBuffer);

	//*-----------------------------------------------------------
	sprintf(lineBuffer,	"\t<TR><TD>Slewing</TD><TD>%s</TD></TR>\r\n",		(cDomeProp.Slewing)	? "Yes" : "No");
	SocketWriteData(mySocketFD,	lineBuffer);

	//*-----------------------------------------------------------
	sprintf(lineBuffer,	"\t<TR><TD>Altitude</TD><TD>%1.2f</TD></TR>\r\n",	cDomeProp.Altitude);
	SocketWriteData(mySocketFD,	lineBuffer);

	//*-----------------------------------------------------------
	sprintf(lineBuffer,	"\t<TR><TD>Azimuth</TD><TD>%1.2f</TD></TR>\r\n",	cDomeProp.Azimuth);
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
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This should be overridden");
	cTimeOfLastSpeedChange	=	millis();
	cTimeOfMovingStart		=	millis();
}


//*****************************************************************************
//*	this should be over ridden
//*****************************************************************************
void	DomeDriver::StopDomeMoving(bool rightNow)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This should be overridden");
	cAzimuth_Destination	=	-1;
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
	if (cGoingHome && cDomeProp.AtHome)
	{
		CONSOLE_DEBUG("Stopping at Home");
		StopDomeMoving(kStopNormal);
		cGoingHome	=	false;
	}

	//****************************************************
	//*	are going to the park position?
	if (cGoingPark)
	{
		if (cDomeProp.AtPark)
		{
			CONSOLE_DEBUG("Stopping at Park");
			StopDomeMoving(kStopNormal);
			cGoingPark	=	false;
		}
		else if (cDomeProp.AtHome && (cCurrentDirection == kRotateDome_CW))
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

		deltaDegrees	=	fabs(cAzimuth_Destination - cDomeProp.Azimuth);


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
		if (cDomeProp.Slewing)
		{
			//*	now check to see if we have gone too far
			if (cCurrentDirection == kRotateDome_CW)
			{
				if (cDomeProp.Azimuth > cAzimuth_Destination)
				{
					CONSOLE_DEBUG("Went too far");
					StopDomeMoving(kStopRightNow);
				}
			}
			else if (cCurrentDirection == kRotateDome_CCW)
			{
				if (cDomeProp.Azimuth < cAzimuth_Destination)
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
		if (cDomeProp.AtHome || cDomeProp.AtPark)
		{
			StopDomeMoving(kStopNormal);
			CONSOLE_DEBUG_W_NUM("AtHome\t=",	cDomeProp.AtHome);
			CONSOLE_DEBUG_W_NUM("AtPark\t=",	cDomeProp.AtPark);
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

//*****************************************************************************
void	DomeDriver::WatchDog_TimeOut(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				alpacaErrMsg[64];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cCommonProp.Name);

	if (cDomeProp.ShutterStatus != kShutterStatus_Closed)
	{
		LogEvent("dome",	"Closing due to Timeout",	NULL,	kASCOM_Err_Success,	"");
		cDomeProp.ShutterStatus	=	kShutterStatus_Closing;
		alpacaErrCode			=	CloseShutter(alpacaErrMsg);
		if (alpacaErrCode != kASCOM_Err_Success)
		{
			LogEvent("dome", "Error closing dome",	NULL, alpacaErrCode,	alpacaErrMsg);
		}
	}
}


#endif	//	defined(_ENABLE_DOME_) || defined(_ENABLE_DOME_ROR_)


