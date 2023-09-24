//*****************************************************************************
//*	command table for Alpaca switch
//*		This file is used by both the driver and the controller
//*****************************************************************************
//*	Jul  1,	2023	<MLS> Created switch_AlpacaCmds.cpp
//*****************************************************************************


#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

#ifndef _SWITCH_ALPACA_CMDS_H_
	#include	"switch_AlpacaCmds.h"
#endif

//*****************************************************************************
TYPE_CmdEntry	gSwitchCmdTable[]	=
{
	{	"maxswitch",			kCmd_Switch_maxswitch,				kCmdType_GET	},
	{	"canwrite",				kCmd_Switch_canwrite,				kCmdType_GET	},
	{	"getswitch",			kCmd_Switch_getswitch,				kCmdType_GET	},
	{	"getswitchdescription",	kCmd_Switch_getswitchdescription,	kCmdType_GET	},
	{	"getswitchname",		kCmd_Switch_getswitchname,			kCmdType_GET	},
	{	"getswitchvalue",		kCmd_Switch_getswitchvalue,			kCmdType_GET	},
	{	"minswitchvalue",		kCmd_Switch_minswitchvalue,			kCmdType_GET	},
	{	"maxswitchvalue",		kCmd_Switch_maxswitchvalue,			kCmdType_GET	},
	{	"setswitch",			kCmd_Switch_setswitch,				kCmdType_PUT	},
	{	"setswitchname",		kCmd_Switch_setswitchname,			kCmdType_PUT	},
	{	"setswitchvalue",		kCmd_Switch_setswitchvalue,			kCmdType_PUT	},
	{	"switchstep",			kCmd_Switch_switchstep,				kCmdType_GET	},

	//*	added by MLS
	{	"--extras",				kCmd_Switch_Extras,					kCmdType_GET	},
	{	"setswitchdescription",	kCmd_Switch_setswitchdescription,	kCmdType_PUT	},
	{	"readall",				kCmd_Switch_readall,				kCmdType_GET	},

	{	"",						-1,	0x00	}
};

