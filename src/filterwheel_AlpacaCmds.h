//*****************************************************************************
//Filter wheel commands
//*****************************************************************************
//*	Jun 26,	2023	<MLS> Created filterwheel_AlpacaCmds.h
//*****************************************************************************
//#include	"filterwheel_AlpacaCmds.h"

#ifndef _FILTERWHEEL_ALPACA_CMDS_H
#define _FILTERWHEEL_ALPACA_CMDS_H

extern	TYPE_CmdEntry	gFilterwheelCmdTable[];

//*****************************************************************************
enum
{
	kCmd_Filterwheel_focusoffsets	=	0,	//*	Filter focus offsets
	kCmd_Filterwheel_names,					//*	Filter wheel filter names
	kCmd_Filterwheel_position,				//*	GET-Returns the current filter wheel position
											//*	PUT-Sets the filter wheel position
	kCmd_Filterwheel_Extras,
	kCmd_Filterwheel_readall
};

#endif // _FILTERWHEEL_ALPACA_CMDS_H
