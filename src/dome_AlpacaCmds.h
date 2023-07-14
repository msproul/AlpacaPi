//*****************************************************************************
//*	Jun 27,	2023	<MLS> Created dome_AlpacaCmds.h
//*****************************************************************************
//#include	"dome_AlpacaCmds.h"

#ifndef _DOME_ALPACA_CMDS_H_
#define	_DOME_ALPACA_CMDS_H_

#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

extern	TYPE_CmdEntry	gDomeCmdTable[];
extern	TYPE_CmdEntry	gDomeExtrasTable[];

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
//*	dome extra commands
enum
{
	kCmd_Dome_SlewingRatePWM	=	kCmd_Dome_last + 1,
	kCmd_Dome_IdleTimeOutEnabled,
	kCmd_Dome_IdleTimeOutMinutes,

};

#endif // _DOME_ALPACA_CMDS_H_
