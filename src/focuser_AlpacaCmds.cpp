//*****************************************************************************
//*	command table for Alpaca focuser
//*		This file is used by both the driver and the controller
//*****************************************************************************
//*	Jul  1,	2023	<MLS> Created focuser_AlpacaCmds.cpp
//*****************************************************************************


#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

#ifndef _FOCUSER_ALPACA_CMDS_H_
	#include	"focuser_AlpacaCmds.h"
#endif

//*****************************************************************************
TYPE_CmdEntry	gFocuserCmdTable[]	=
{

	{	"absolute",				kCmd_Focuser_absolute,			kCmdType_GET	},
	{	"ismoving",				kCmd_Focuser_ismoving,			kCmdType_GET	},
	{	"maxincrement",			kCmd_Focuser_maxincrement,		kCmdType_GET	},
	{	"maxstep",				kCmd_Focuser_maxstep,			kCmdType_GET	},
	{	"position",				kCmd_Focuser_position,			kCmdType_GET	},
	{	"stepsize",				kCmd_Focuser_stepsize,			kCmdType_GET	},
	{	"tempcomp",				kCmd_Focuser_tempcomp,			kCmdType_BOTH	},
	{	"tempcompavailable",	kCmd_Focuser_tempcompavailable,	kCmdType_GET	},
	{	"temperature",			kCmd_Focuser_temperature,		kCmdType_GET	},
	{	"halt",					kCmd_Focuser_halt,				kCmdType_PUT	},
	{	"move",					kCmd_Focuser_move,				kCmdType_PUT	},

	//*	items added by MLS
	{	"--extras",				kCmd_Focuser_Extras,			kCmdType_GET	},
	{	"moverelative",			kCmd_Focuser_moverelative,		kCmdType_PUT	},
	{	"readall",				kCmd_Focuser_readall,			kCmdType_GET	},


	{	"",						-1,	0x00	}
};

//*****************************************************************************
//*	extra stuff in read all that are not actual commands
//*****************************************************************************
TYPE_CmdEntry	gFocuserExtrasTable[]	=
{
	{	"AuxPosition",			kCmd_Focuser_AuxPosition,		kCmdType_GET	},
	{	"AuxIsMoving",			kCmd_Focuser_AuxIsMoving,		kCmdType_GET	},
	{	"Degrees-F",			kCmd_Focuser_DegreesF,			kCmdType_GET	},
	{	"Model",				kCmd_Focuser_Model,				kCmdType_GET	},
	{	"RotatorIsMoving",		kCmd_Focuser_RotatorIsMoving,	kCmdType_GET	},
	{	"RotatorPosition",		kCmd_Focuser_RotatorPosition,	kCmdType_GET	},
	{	"SerialNum",			kCmd_Focuser_SerialNum,			kCmdType_GET	},
	{	"SwitchAux1",			kCmd_Focuser_SwitchAux1,		kCmdType_GET	},
	{	"SwitchAux2",			kCmd_Focuser_SwitchAux2,		kCmdType_GET	},
	{	"SwitchIn",				kCmd_Focuser_SwitchIn,			kCmdType_GET	},
	{	"SwitchOut",			kCmd_Focuser_SwitchOut,			kCmdType_GET	},
	{	"SwitchRot",			kCmd_Focuser_SwitchRot,			kCmdType_GET	},
	{	"Voltage",				kCmd_Focuser_Voltage,			kCmdType_GET	},
	{	"",						-1,								0x00	}

};
