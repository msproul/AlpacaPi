//*****************************************************************************
//*	Name:			rotator_AlpacaCmds.cpp
//*
//*	Author:			Mark Sproul (C) 2023
//*
//*	Description:	Command table for rotator
//*					This file is used by both the driver and the controller
//*****************************************************************************
//*	Jul  1,	2023	<MLS> Created rotator_AlpacaCmds.cpp
//*****************************************************************************


#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

#ifndef _ROTATOR_ALPACA_CMDS_H_
	#include	"rotator_AlpacaCmds.h"
#endif

//*****************************************************************************
TYPE_CmdEntry	gRotatorCmdTable[]	=
{

	{	"canreverse",			kCmd_Rotator_canreverse,		kCmdType_GET	},
	{	"ismoving",				kCmd_Rotator_ismoving,			kCmdType_GET	},
	{	"mechanicalposition",	kCmd_Rotator_mechanicalposition,kCmdType_GET	},
	{	"position",				kCmd_Rotator_position,			kCmdType_GET	},
	{	"reverse",				kCmd_Rotator_reverse,			kCmdType_BOTH	},
	{	"stepsize",				kCmd_Rotator_stepsize,			kCmdType_GET	},
	{	"targetposition",		kCmd_Rotator_targetposition,	kCmdType_GET	},

	{	"halt",					kCmd_Rotator_halt,				kCmdType_PUT	},
	{	"move",					kCmd_Rotator_move,				kCmdType_PUT	},
	{	"moveabsolute",			kCmd_Rotator_moveabsolute,		kCmdType_PUT	},
	{	"movemechanical",		kCmd_Rotator_movemechanical,	kCmdType_PUT	},
	{	"sync",					kCmd_Rotator_sync,				kCmdType_PUT	},


	//*	added by MLS
	{	"--extras",				kCmd_Rotator_Extras,			kCmdType_GET	},
	{	"step",					kCmd_Rotator_step,				kCmdType_PUT	},
	{	"stepabsolute",			kCmd_Rotator_stepabsolute,		kCmdType_PUT	},
	{	"readall",				kCmd_Rotator_readall,			kCmdType_GET	},

	{	"",						-1,	0x00	}
};
