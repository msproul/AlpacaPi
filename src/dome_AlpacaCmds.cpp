//*****************************************************************************
//*	command table for Alpaca dome
//*		This file is used by both the driver and the controller
//*****************************************************************************
//*	Jul  1,	2023	<MLS> Created dome_AlpacaCmds.cpp
//*	Jul  9,	2023	<MLS> Added gDomeExtrasTable
//*****************************************************************************

#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

#ifndef _DOME_ALPACA_CMDS_H_
	#include	"dome_AlpacaCmds.h"
#endif

//*****************************************************************************
TYPE_CmdEntry	gDomeCmdTable[]	=
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
	{	"abortslew",		kCmd_Dome_abortslew,		kCmdType_PUT	},
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

	{	"",					-1,							0				}
};

//*****************************************************************************
TYPE_CmdEntry	gDomeExtrasTable[]	=
{
	{	"SlewingRate-PWM",		kCmd_Dome_SlewingRatePWM,		kCmdType_GET	},
	{	"IDLETIMEOUTENABLED",	kCmd_Dome_IdleTimeOutEnabled,	kCmdType_GET	},
	{	"IDLE TIMEOUT ENABLED",	kCmd_Dome_IdleTimeOutEnabled,	kCmdType_GET	},
	{	"IDLETIMEOUT_MINUTES",	kCmd_Dome_IdleTimeOutMinutes,	kCmdType_GET	},
	{	"IDLE TIMEOUT (MINUTES)",	kCmd_Dome_IdleTimeOutMinutes,	kCmdType_GET	},

	{	"",						-1,								0				}
};
