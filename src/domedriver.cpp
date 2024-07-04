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
//*	Apr  6,	2023	<MLS> Added GetCommandArgumentString() to domedriver
//*	Jun 18,	2023	<MLS> Added DeviceState_Add_Content() to domedriver
//*	Jul 16,	2023	<MLS> Added Setup web page for domedriver
//*	Jul 16,	2023	<MLS> Added cRORrelayDelay_secs into Setup for ChrisA
//*	Jul 17,	2023	<MLS> Added better logic for remote shutter check when shutter is moving
//*	Jul 17,	2023	<MLS> Added watchdog timer enable/disable to dome setup web page
//*	May 17,	2024	<MLS> Added http error 400 processing to dome driver
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

#include	"dome_AlpacaCmds.h"
#include	"dome_AlpacaCmds.cpp"



#if defined(__arm__) && !defined(_ENABLE_PI_HAT_SESNSOR_BOARD_)
//	#include <wiringPi.h>
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
	cDriverSupportsSetup			=	true;			//*	enable setup web page

	cTimeOfLastMoveCmd				=	time(NULL);		//*	these need to be set or it will do a shutdown before it even starts
	cTimeOfLastMoveCheck			=	time(NULL);
	cEnableIdleMoveTimeout			=	true;
	cIdleMoveTimeoutMinutes			=	2 * 60;
	cRORrelayDelay_secs				=	20;				//*	used by Roll Off Roof ONLY

	strcpy(cWatchDogTimeOutAction, "Close shutter");

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

	if ((strcmp(reqData->deviceCommand, "readall") != 0) && (strcmp(reqData->deviceCommand, "devicestate") != 0))
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

	if (reqData->clientIs_ConformU == false)
	{
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
	}
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
				CONSOLE_DEBUG(alpacaErrMsg);
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
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported for 'openshutter'");
				CONSOLE_DEBUG(alpacaErrMsg);
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
				CONSOLE_DEBUG(alpacaErrMsg);
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
				CONSOLE_DEBUG(alpacaErrMsg);
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
				CONSOLE_DEBUG(alpacaErrMsg);
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
				CONSOLE_DEBUG(alpacaErrMsg);
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
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported for synctoazimuth");
				CONSOLE_DEBUG(alpacaErrMsg);
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
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Get not supported for poweron");
				CONSOLE_DEBUG(alpacaErrMsg);
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
				CONSOLE_DEBUG(alpacaErrMsg);
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
				CONSOLE_DEBUG(alpacaErrMsg);
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
				CONSOLE_DEBUG(alpacaErrMsg);
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
				CONSOLE_DEBUG(alpacaErrMsg);
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
				CONSOLE_DEBUG(alpacaErrMsg);
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
				CONSOLE_DEBUG(alpacaErrMsg);
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
				CONSOLE_DEBUG(alpacaErrMsg);
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
				CONSOLE_DEBUG(alpacaErrMsg);
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

	if (cSendJSONresponse)	//*	False for setupdialog and camera binary data
	{
		//*	send the response information
		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Uint32(	mySocket,
																reqData->jsonTextBuffer,
																kMaxJsonBuffLen,
																"ClientTransactionID",
																reqData->ClientTransactionID,
																INCLUDE_COMMA);

		cBytesWrittenForThisCmd	+=	JsonResponse_Add_Uint32(	mySocket,
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
																reqData->httpRetCode,
																reqData->jsonTextBuffer,
																(cHttpHeaderSent == false));
	}
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
	bool		timeToCheckShutter;
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
		timeToCheckShutter		=	false;
		currentMilliSecs		=	millis();
		timeSinceLastWhatever	=	currentMilliSecs - cTimeOfLastShutterUpdate;
		if (timeSinceLastWhatever > (15 * 1000))
		{
			timeToCheckShutter		=	true;
		}
		else if ((cDomeProp.ShutterStatus >= kShutterStatus_Opening) && (timeSinceLastWhatever > (5 * 1000)))
		{
			//*	if the shutter is moving, update more often
			timeToCheckShutter		=	true;
		}
		if (timeToCheckShutter)
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

//		CONSOLE_DEBUG("Idle move is ENABLED");

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
	CONSOLE_DEBUG_W_STR(__FUNCTION__, deviceType);


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

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Double(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															responseString,
															cDomeProp.Altitude,
															INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Athome(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.AtHome,
														INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Atpark(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.AtPark,
														INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Azimuth(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Double(reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.Azimuth,
														INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Canfindhome(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.CanFindHome,
														INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Indicates whether the dome can be parked.
TYPE_ASCOM_STATUS	DomeDriver::Get_Canpark(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.CanPark,
														INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Indicates whether the dome altitude can be set
TYPE_ASCOM_STATUS	DomeDriver::Get_Cansetaltitude(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.CanSetAltitude,
														INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Indicates whether the dome park position can be set
TYPE_ASCOM_STATUS	DomeDriver::Get_Cansetazimuth(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.CanSetAzimuth,
														INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Indicates whether the dome azimuth can be set
TYPE_ASCOM_STATUS	DomeDriver::Get_Cansetpark(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.CanSetPark,
														INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
//*	Indicates whether the dome shutter can be opened
TYPE_ASCOM_STATUS	DomeDriver::Get_Cansetshutter(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.CanSetShutter,
														INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Canslave(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.CanSlave,
														INCLUDE_COMMA);
	return(alpacaErrCode);
}

//*****************************************************************************
//*	Indicates whether the dome azimuth position can be synced
TYPE_ASCOM_STATUS	DomeDriver::Get_Cansyncazimuth(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.CanSyncAzimuth,
														INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
//*	Indicates whether the any part of the dome is moving
TYPE_ASCOM_STATUS	DomeDriver::Get_Slewing(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.Slewing,
														INCLUDE_COMMA);
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Shutterstatus(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				statusString[32];

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
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Slaved(		TYPE_GetPutRequestData *reqData, char *alpacaErrMsg, const char *responseString)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);
	alpacaErrCode	=	kASCOM_Err_Success;
	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Bool(	reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														responseString,
														cDomeProp.Slaved,
														INCLUDE_COMMA);
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
	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Slaved",
											argumentString,
											(sizeof(argumentString) -1));

	if (cDomeProp.CanSlave)
	{
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
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid Value");
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not supported");
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
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_SetPark(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Set park not supported");
	CONSOLE_DEBUG(alpacaErrMsg);
	return(kASCOM_Err_NotImplemented);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Put_SlewToAltitude(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_InternalError;

	alpacaErrCode	=	kASCOM_Err_NotImplemented;
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "SlewToAltitude not finished");
	CONSOLE_DEBUG(alpacaErrMsg);
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
	//*	look for Azimuth
	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Azimuth",
											argumentString,
											(sizeof(argumentString) -1),
											kArgumentIsNumeric);

	if (cDomeProp.CanSetAzimuth)
	{
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
					alpacaErrCode			=	kASCOM_Err_InvalidValue;
					reqData->httpRetCode	=	400;
					GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid Value");
				}
			}
			else
			{
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid Value 'Azimuth=' was not found");
				CONSOLE_DEBUG(alpacaErrMsg);
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
		CONSOLE_DEBUG(alpacaErrMsg);
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
	//*	look for Azimuth
	foundKeyWord	=	GetKeyWordArgument(	reqData->contentData,
											"Azimuth",
											argumentString,
											(sizeof(argumentString) -1),
											kArgumentIsNumeric);

	cTimeOfLastMoveCmd		=	time(NULL);
	if (cDomeProp.CanSyncAzimuth)
	{
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
				alpacaErrCode			=	kASCOM_Err_InvalidValue;
				reqData->httpRetCode	=	400;
				GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid Value");
				CONSOLE_DEBUG(alpacaErrMsg);
			}
		}
		else
		{
			alpacaErrCode			=	kASCOM_Err_InvalidValue;
			reqData->httpRetCode	=	400;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Invalid Value 'Azimuth=' was not found");
			CONSOLE_DEBUG(alpacaErrMsg);
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
		CONSOLE_DEBUG(alpacaErrMsg);
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
		alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "ROR doesnt support dome rotation");
		CONSOLE_DEBUG(alpacaErrMsg);
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
		alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "ROR doesnt support dome rotation");
		CONSOLE_DEBUG(alpacaErrMsg);
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
		if (cDomeProp.Slewing)
		{
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Dome already in motion, command ignored");
			CONSOLE_DEBUG(alpacaErrMsg);
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
		alpacaErrCode	=	kASCOM_Err_ActionNotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "ROR doesnt support dome rotation");
		CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Currentstate(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				stateString[48];

	GetStateString(cDomeState, stateString);
	cBytesWrittenForThisCmd	+=	JsonResponse_Add_String(	reqData->socket,
															reqData->jsonTextBuffer,
															kMaxJsonBuffLen,
															"CurrentState",
															stateString,
															INCLUDE_COMMA);
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
//			CONSOLE_DEBUG(alpacaErrMsg);
	}
	else if (alpacaErrCode != kASCOM_Err_Success)
	{
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Power state unknown");
//			CONSOLE_DEBUG(alpacaErrMsg);
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
//			CONSOLE_DEBUG(alpacaErrMsg);
	}
	else if (alpacaErrCode != kASCOM_Err_Success)
	{
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Auxiliary state unknown");
//			CONSOLE_DEBUG(alpacaErrMsg);
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//Altitude
//AtHome
//AtPark
//Azimuth
//ShutterStatus
//Slewing
//*****************************************************************************
bool	DomeDriver::DeviceState_Add_Content(const int socketFD, char *jsonTextBuffer, const int maxLen)
{
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"Altitude",		cDomeProp.Altitude);
	DeviceState_Add_Bool(socketFD,	jsonTextBuffer, maxLen,	"AtHome",		cDomeProp.AtHome);
	DeviceState_Add_Bool(socketFD,	jsonTextBuffer, maxLen,	"AtPark",		cDomeProp.AtPark);
	DeviceState_Add_Dbl(socketFD,	jsonTextBuffer, maxLen,	"Azimuth",		cDomeProp.Azimuth);
	DeviceState_Add_Int(socketFD,	jsonTextBuffer, maxLen,	"ShutterStatus",cDomeProp.ShutterStatus);
	DeviceState_Add_Bool(socketFD,	jsonTextBuffer, maxLen,	"Slewing",		cDomeProp.Slewing);

	return(true);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	DomeDriver::Get_Readall(	TYPE_GetPutRequestData *reqData, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				stateString[48];

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
														"IdleTimeoutEnabled",
														cEnableIdleMoveTimeout,
														INCLUDE_COMMA);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Int32(reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														"IdleTimeout_minutes",
														cIdleMoveTimeoutMinutes,
														INCLUDE_COMMA);

	cBytesWrittenForThisCmd	+=	JsonResponse_Add_Int32(reqData->socket,
														reqData->jsonTextBuffer,
														kMaxJsonBuffLen,
														"ROR_relay_delay_secs",
														cRORrelayDelay_secs,
														INCLUDE_COMMA);
	alpacaErrCode	=	kASCOM_Err_Success;
	strcpy(alpacaErrMsg, "");
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
		CONSOLE_DEBUG(alpacaErrMsg);
	}
#else
	alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Open shutter not implemented");
	CONSOLE_DEBUG(alpacaErrMsg);

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
		CONSOLE_DEBUG(alpacaErrMsg);
	}
#else
	alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;
	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Close shutter not implemented");
	CONSOLE_DEBUG(alpacaErrMsg);
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

	CONSOLE_DEBUG("exit");
}

//*****************************************************************************
bool	DomeDriver::GetCmdNameFromMyCmdTable(const int cmdNumber, char *comandName, char *getPut)
{
bool	foundIt;

	foundIt	=	GetCmdNameFromTable(cmdNumber, comandName, gDomeCmdTable, getPut);
	return(foundIt);
}

//*****************************************************************************
bool	DomeDriver::GetCommandArgumentString(const int cmdENum, char *agumentString, char *commentString)
{
bool	foundFlag	=	true;

	switch(cmdENum)
	{
		case kCmd_Dome_altitude:		//*	The dome altitude
		case kCmd_Dome_athome:			//*	Indicates whether the dome is in the home position.
		case kCmd_Dome_atpark:			//*	Indicates whether the telescope is at the park position
		case kCmd_Dome_azimuth:			//*	The dome azimuth
		case kCmd_Dome_canfindhome:		//*	Indicates whether the dome can find the home position.
		case kCmd_Dome_canpark:			//*	Indicates whether the dome can be parked.
		case kCmd_Dome_cansetaltitude:	//*	Indicates whether the dome altitude can be set
		case kCmd_Dome_cansetazimuth:	//*	Indicates whether the dome azimuth can be set
		case kCmd_Dome_cansetpark:		//*	Indicates whether the dome park position can be set
		case kCmd_Dome_cansetshutter:	//*	Indicates whether the dome shutter can be opened
		case kCmd_Dome_canslave:		//*	Indicates whether the dome supports slaving to a telescope
		case kCmd_Dome_cansyncazimuth:	//*	Indicates whether the dome azimuth position can be synched
		case kCmd_Dome_shutterstatus:	//*	Status of the dome shutter or roll-off roof
		case kCmd_Dome_slaved:			//*	GET--Indicates whether the dome is slaved to the telescope
										//*	SET--Sets whether the dome is slaved to the telescope
		case kCmd_Dome_slewing:			//*	Indicates whether the any part of the dome is moving
		case kCmd_Dome_abortslew:		//*	Immediately cancel current dome operation.
		case kCmd_Dome_closeshutter:	//*	Close the shutter or otherwise shield telescope from the sky.
		case kCmd_Dome_findhome:		//*	Start operation to search for the dome home position.
		case kCmd_Dome_openshutter:		//*	Open shutter or otherwise expose telescope to the sky.
		case kCmd_Dome_park:			//*	Rotate dome in azimuth to park position.
		case kCmd_Dome_setpark:			//*	Set the current azimuth, altitude position of dome to be the park position
		case kCmd_Dome_slewtoaltitude:	//*	Slew the dome to the given altitude position.
		case kCmd_Dome_slewtoazimuth:	//*	Slew the dome to the given azimuth position.
		case kCmd_Dome_synctoazimuth:	//*	Synchronize the current position of the dome to the given azimuth.
			strcpy(agumentString, "");
			break;

//==============================================================
//*	extra commands added by MLS
		case kCmd_Dome_Extras:

		case kCmd_Dome_poweron:			//*	Turn dome power on
		case kCmd_Dome_poweroff:		//*	Turn dome power off
		case kCmd_Dome_powerstatus:		//*	Return power status

		case kCmd_Dome_auxiliaryon:		//*	Turn auxiliary on
		case kCmd_Dome_auxiliaryoff:	//*	Turn auxiliary off
		case kCmd_Dome_auxiliarystatus:	//*	Return auxiliary status
		case kCmd_Dome_goleft:			//*	Move the dome left (CCW)
		case kCmd_Dome_goright:			//*	Move the dome right (CW)
		case kCmd_Dome_bumpleft:		//*	Move the dome left (CCW)
		case kCmd_Dome_bumpright:		//*	Move the dome right (CW)
		case kCmd_Dome_slowleft:		//*	Move the dome left (CCW)
		case kCmd_Dome_slowright:		//*	Move the dome right (CW)
		case kCmd_Dome_currentstate:	//*	What is the current state of the state machine
			strcpy(agumentString, "-none-");
			break;


		//*	make this one last for consistency
		case kCmd_Dome_readall:			//*	Read all parameters
			strcpy(agumentString, "-none-");
			break;


		default:
			strcpy(agumentString, "");
			foundFlag	=	false;
			break;

	}
	return(foundFlag);
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
		CONSOLE_DEBUG("Closing dome shutter due to Timeout");
		LogEvent("dome",	"Closing due to Timeout",	NULL,	kASCOM_Err_Success,	"");
		cDomeProp.ShutterStatus	=	kShutterStatus_Closing;
		alpacaErrCode			=	CloseShutter(alpacaErrMsg);
		if (alpacaErrCode != kASCOM_Err_Success)
		{
			LogEvent("dome", "Error closing dome",	NULL, alpacaErrCode,	alpacaErrMsg);
		}
	}
}


//*****************************************************************************
//*	documentation
//*****************************************************************************
const char	gDomeDriverDocs[]	=
	"The AlpacaPi dome driver can support the following configurations:\r\n"
	"<UL>\r\n"
	"<LI>Dome with rotation and single shutter\r\n"
	"<LI>Roll Off Roof with roof acting as the shutter\r\n"
	"</UL>\r\n"
	"<P>\r\n"
	"A watchdog timer is also supported to close the dome if communications is losts\r\n"

	;

const char	gDomeDriverNotes[]	=
	"<UL>\r\n"
	"<LI><B>Note 1:</B>"
	"The watchdog timeout occurs if no commands have been received for the specified time.\r\n"
	"(default is 5 minutes).\r\n"
	"If SkyTravel is being used, it queries the dome driver every few seconds for status\r\n"
	"thus resetting the timeout.\r\n"
	"The Dome watchdog timer controls the dome shutter. If the timeout occurs, the shutter will close.\r\n"
	"<P><LI><B>Note 2:</B>"
	"The Dome movement timeout occurs if no MOVE command is received in the specified time\r\n"
	"(default is 2 hours).\r\n"
	"The Dome movement timer controls the dome position.\r\n"
	"If the timeout occurs, the dome will be sent back to the PARK position\r\n"
	"</UL>\r\n";

//*****************************************************************************
//*	Remote shutter documentation
//*****************************************************************************
#ifdef _ENABLE_REMOTE_SHUTTER_
const char	gRemoteShutterDocs[]	=
	"The AlpacaPi dome driver supports the option of a separate drive that controls the door.\r\n"
	"This allows one device driver to be used for the dome since most client applications can only deal with one.\r\n"
	"While in reality there are 2 drivers. The primary DOME driver talks to the second driver (SHUTTER).\r\n"
	"<P>\r\n"
	"The SHUTTER devices is a non-standard Alpaca device. It will not be recognized by other Alpaca clients.\r\n"
	"It responds to the same door/shutter commands as the DOME device.\r\n"
	"\r\n"
	"";

#endif // _ENABLE_REMOTE_SHUTTER_

//*****************************************************************************
//*	https://www.w3schools.com/html/html_forms.asp
//*****************************************************************************
bool	DomeDriver::Setup_OutputForm(TYPE_GetPutRequestData *reqData, const char *formActionString)
{
int			mySocketFD;
char		lineBuff[256];
const char	domeTitle[]	=	"AlpacaPi Dome Driver setup";
#ifdef _ENABLE_REMOTE_SHUTTER_
	char	shutterIPaddrStr[INET_ADDRSTRLEN];
#endif // _ENABLE_REMOTE_SHUTTER_

	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("The Action that will be preformed when save is pressed:", formActionString);

	mySocketFD	=	reqData->socket;


	SocketWriteData(mySocketFD,	gHtmlHeader_html);

	SocketWriteData(mySocketFD,	"<!DOCTYPE html>\r\n");
	SocketWriteData(mySocketFD,	"<HTML lang=\"en\">\r\n");
	sprintf(lineBuff,			"<TITLE>%s</TITLE>\r\n", domeTitle);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	sprintf(lineBuff,			"<H1>%s</H1>\r\n", domeTitle);
	SocketWriteData(mySocketFD,	lineBuff);
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");


//	SocketWriteData(mySocketFD,	"<form action=\"/setup/common\" target=\"_blank\">\r\n");
//	sprintf(lineBuff, "<form action=\"%s\" target=\"_blank\">\r\n", formActionString);
	sprintf(lineBuff, "<form action=\"%s\">\r\n", formActionString);
	SocketWriteData(mySocketFD,	lineBuff);



	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
#ifdef _ENABLE_DOME_RPI_
	SocketWriteData(mySocketFD,	"<H2>Dome with Raspberry Pi and DC motor controller</H2>\r\n");
#endif
#ifdef	_ENABLE_DOME_ROR_
	SocketWriteData(mySocketFD,	"<H2>Roll Off Roof with Raspberry Pi and relays</H2>\r\n");
#endif
#ifdef _ENABLE_DOME_SIMULATOR_
	SocketWriteData(mySocketFD,	"<H2>Dome Simulator</H2>\r\n");
#endif
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");

	SocketWriteData(mySocketFD,	gDomeDriverDocs);

	//----------------------------------------------------
	//----------------------------------------------------
	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");
	//----------------------------------------------------
	//*	table header
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TH>Property</TH>\r\n");
	SocketWriteData(mySocketFD,	"<TH>Value</TH>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");


	//-----------------------------------
	//*	first row
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TD>Watchdog Timeout<SUP>(*1)</SUP></TD>\r\n");

	SocketWriteData(mySocketFD,	"<TD>\r\n");
	Setup_OutputRadioBtn(mySocketFD,	"wdtimeout",	"enabled",	"enabled",	cWatchDogEnabled);
	Setup_OutputRadioBtn(mySocketFD,	"wdtimeout",	"disabled",	"disabled",	!cWatchDogEnabled);
	SocketWriteData(mySocketFD,	"</TD>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	//-----------------------------------
	//*	2nd row
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TD>\r\n");

	SocketWriteData(mySocketFD,	"<label for=\"watchdogtime\">Watchdog Timeout (mins):</label>\r\n");
	SocketWriteData(mySocketFD,	"</TD>\r\n");

	SocketWriteData(mySocketFD,	"<TD>\r\n");

	sprintf(lineBuff,	"<input type=\"text\" id=\"watchdogtime\" name=\"watchdogtime\" value=\"%d\">\r\n", cWatchDogTimeOut_Minutes);
	SocketWriteData(mySocketFD,	lineBuff);

	SocketWriteData(mySocketFD,	"</TD>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	//-----------------------------------
	//*	3rd row
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TD>Dome Movement Timeout<SUP>(*2)</SUP></TD>\r\n");

	SocketWriteData(mySocketFD,	"<TD>\r\n");
	Setup_OutputRadioBtn(mySocketFD,	"dometimeout",	"enabled",	"enabled",	cEnableIdleMoveTimeout);
	Setup_OutputRadioBtn(mySocketFD,	"dometimeout",	"disabled",	"disabled",	!cEnableIdleMoveTimeout);
	SocketWriteData(mySocketFD,	"</TD>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	//-----------------------------------
	//*	4th row

	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TD>\r\n");

	SocketWriteData(mySocketFD,	"<label for=\"length\">Dome Movement Timeout (mins):</label>\r\n");
	SocketWriteData(mySocketFD,	"</TD>\r\n");

	SocketWriteData(mySocketFD,	"<TD>\r\n");

	sprintf(lineBuff,	"<input type=\"text\" id=\"length\" name=\"idletimeout\" value=\"%d\">\r\n", cIdleMoveTimeoutMinutes);
	SocketWriteData(mySocketFD,	lineBuff);

	SocketWriteData(mySocketFD,	"</TD>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

#if defined(_ENABLE_DOME_ROR_) || defined(_ENABLE_DOME_SIMULATOR_)
	//-----------------------------------
	//*	5th row
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TD>\r\n");

	SocketWriteData(mySocketFD,	"<label for=\"length\">ROR relay delay (secs):<SUP>(*3)</SUP></label>\r\n");
	SocketWriteData(mySocketFD,	"</TD>\r\n");

	SocketWriteData(mySocketFD,	"<TD>\r\n");
	sprintf(lineBuff,	"<input type=\"text\" id=\"length\" name=\"rordelay\" value=\"%d\">\r\n", cRORrelayDelay_secs);
	SocketWriteData(mySocketFD,	lineBuff);

	SocketWriteData(mySocketFD,	"</TD>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");
#endif // _ENABLE_DOME_ROR_ || _ENABLE_DOME_SIMULATOR_

	//-----------------------------------
	//*	SAVE row
	SocketWriteData(mySocketFD,	"<TR>\r\n");
	SocketWriteData(mySocketFD,	"<TD COLSPAN=3><CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<input type=\"submit\" value=\"Save\">\r\n");
	SocketWriteData(mySocketFD,	"</TD>\r\n");
	SocketWriteData(mySocketFD,	"</TR>\r\n");

	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");


	SocketWriteData(mySocketFD,	"</form>\r\n");

	//---------------------------------------------------------------------------
	//	Documentation
	SocketWriteData(mySocketFD,	gDomeDriverNotes);

#if defined(_ENABLE_DOME_ROR_) || defined(_ENABLE_DOME_SIMULATOR_)
	SocketWriteData(mySocketFD,	"<P><LI><B>Note 3:</B>");
	SocketWriteData(mySocketFD,	"\tThe ROR delay applies to the ROR controller only, this is the delay for the relay timing.\r\n");
	SocketWriteData(mySocketFD,	"<P><LI>");
	SocketWriteData(mySocketFD,	"\tThese settings are NOT persistent and revert back to the default when the program is restarted.\r\n");
	SocketWriteData(mySocketFD,	"<P><LI>");
	SocketWriteData(mySocketFD,	"\tFor the Roll Off Roof, the dome movement is not defined and the 'shutter' is the 'roof'.\r\n");
	SocketWriteData(mySocketFD,	"\tTherefore they both default to FALSE.\r\n");
#endif // _ENABLE_DOME_ROR_

	SocketWriteData(mySocketFD,	"</UL>\r\n");

	SocketWriteData(mySocketFD,	"<P>\r\n");

	//----------------------------------------------------------------------
	//----------------------------------------------------------------------
	SocketWriteData(mySocketFD,	"<HR SIZE=3>\r\n");

	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<H2>Remote Shutter</H2>\r\n");
	SocketWriteData(mySocketFD,	"<P>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");

#ifdef _ENABLE_REMOTE_SHUTTER_
	SocketWriteData(mySocketFD,	gRemoteShutterDocs);

	SocketWriteData(mySocketFD,	"<P>\r\n");

	SocketWriteData(mySocketFD,	"<CENTER>\r\n");
	SocketWriteData(mySocketFD,	"<TABLE BORDER=1>\r\n");

	inet_ntop(AF_INET, &(cShutterDeviceAddress.sin_addr), shutterIPaddrStr, INET_ADDRSTRLEN);
	sprintf(lineBuff,	"<TR><TD>Shutter IP address is</TD><TD>%s</TD></TR>\r\n", shutterIPaddrStr);
	SocketWriteData(mySocketFD,	lineBuff);

	sprintf(lineBuff,	"<TR><TD>Port</TD><TD>%d</TD></TR>\r\n", cShutterPort);
	SocketWriteData(mySocketFD,	lineBuff);

	sprintf(lineBuff,	"<TR><TD>Alpaca dev num</TD><TD>%d</TD></TR>\r\n", cShutterAlpacaDevNum);
	SocketWriteData(mySocketFD,	lineBuff);

	SocketWriteData(mySocketFD,	"</TABLE>\r\n");
	SocketWriteData(mySocketFD,	"</CENTER>\r\n");

#endif // _ENABLE_REMOTE_SHUTTER_
	return(true);
}

//*****************************************************************************
void	DomeDriver::Setup_SaveInit(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

}

//*****************************************************************************
void	DomeDriver::Setup_SaveFinish(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	//*	copy the filename options over
}

//*****************************************************************************
bool	DomeDriver::Setup_ProcessKeyword(const char *keyword, const char *valueString)
{
//	CONSOLE_DEBUG_W_2STR("kw:value", keyword, valueString);
//[Setup_ProcessKeyword] kw:value timeout, enabled
//[Setup_ProcessKeyword] kw:value length, 223
//[Setup_ProcessKeyword] kw:value rordelay, 44

	if (strcasecmp(keyword, "wdtimeout") == 0)
	{
		if (strcasecmp(valueString, "enabled") == 0)
		{
			cWatchDogEnabled	=	true;
		}
		else if (strcasecmp(valueString, "disabled") == 0)
		{
			cWatchDogEnabled	=	false;
		}
		else
		{
			CONSOLE_DEBUG_W_STR("invalid valueString:", valueString);
		}
		CONSOLE_DEBUG_W_BOOL("cEnableIdleMoveTimeout", cEnableIdleMoveTimeout);
	}
	else if (strcasecmp(keyword, "dometimeout") == 0)
	{
		if (strcasecmp(valueString, "enabled") == 0)
		{
			cEnableIdleMoveTimeout	=	true;
		}
		else if (strcasecmp(valueString, "disabled") == 0)
		{
			cEnableIdleMoveTimeout	=	false;
		}
		else
		{
			CONSOLE_DEBUG_W_STR("invalid valueString:", valueString);
		}
		CONSOLE_DEBUG_W_BOOL("cEnableIdleMoveTimeout", cEnableIdleMoveTimeout);
	}
	else if (strcmp(keyword, "idletimeout") == 0)
	{
		if (isdigit(valueString[0]))
		{
			cIdleMoveTimeoutMinutes	=	atoi(valueString);
		}
		else
		{
			CONSOLE_DEBUG_W_STR("invalid valueString:", valueString);
		}
		CONSOLE_DEBUG_W_NUM("cIdleMoveTimeoutMinutes", cIdleMoveTimeoutMinutes);
	}
	else if (strcmp(keyword, "rordelay") == 0)
	{
		if (isdigit(valueString[0]))
		{
			cRORrelayDelay_secs	=	atoi(valueString);
		}
		else
		{
			CONSOLE_DEBUG_W_STR("invalid valueString:", valueString);
		}
		CONSOLE_DEBUG_W_NUM("cRORrelayDelay_secs", cRORrelayDelay_secs);
	}
	else if (strcmp(keyword, "watchdogtime") == 0)
	{
		if (isdigit(valueString[0]))
		{
			cWatchDogTimeOut_Minutes	=	atoi(valueString);
		}
		else
		{
			CONSOLE_DEBUG_W_STR("invalid valueString:", valueString);
		}
		CONSOLE_DEBUG_W_NUM("cRORrelayDelay_secs", cRORrelayDelay_secs);
	}
	else
	{
		CONSOLE_DEBUG("Unhandled data");
		CONSOLE_DEBUG_W_2STR("kw:value", keyword, valueString);
	}
	return(true);
}

#endif	//	defined(_ENABLE_DOME_) || defined(_ENABLE_DOME_ROR_)


