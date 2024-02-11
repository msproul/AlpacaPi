//*****************************************************************************
//*	Name:			filterwheel_AlpacaCmds.cpp
//*
//*	Author:			Mark Sproul (C) 2023
//*
//*	Description:	Command table for FilterWheel
//*					This file is used by both the driver and the controller
//*****************************************************************************
//*	Jul  1,	2023	<MLS> Created filterwheel_AlpacaCmds.cpp
//*****************************************************************************


#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

#ifndef _FILTERWHEEL_ALPACA_CMDS_H
	#include	"filterwheel_AlpacaCmds.h"
#endif

//*****************************************************************************
TYPE_CmdEntry	gFilterwheelCmdTable[]	=
{
	{	"focusoffsets",			kCmd_Filterwheel_focusoffsets,	kCmdType_GET	},
	{	"names",				kCmd_Filterwheel_names,			kCmdType_GET	},
	{	"position",				kCmd_Filterwheel_position,		kCmdType_BOTH	},
	//*	items added by MLS
	{	"--extras",				kCmd_Filterwheel_Extras,		kCmdType_GET	},
	{	"readall",				kCmd_Filterwheel_readall,		kCmdType_GET	},

	{	"",						-1,	0x00	}
};
