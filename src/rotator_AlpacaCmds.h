//*****************************************************************************
//*	Jun 30,	2023	<MLS> Created rotator_AlpacaCmds.h
//*****************************************************************************
//#include	"rotator_AlpacaCmds.h"

#ifndef _ROTATOR_ALPACA_CMDS_H_
#define	_ROTATOR_ALPACA_CMDS_H_

#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

extern	TYPE_CmdEntry		gRotatorCmdTable[];


//*****************************************************************************
//*	Rotator Specific Methods
//*****************************************************************************
enum
{
	kCmd_Rotator_canreverse=	0,		//*	Indicates whether the Rotator supports the Reverse method.
	kCmd_Rotator_ismoving,				//*	Indicates whether the rotator is currently moving.
	kCmd_Rotator_mechanicalposition,	//*	Returns the rotator's mechanical current position.
	kCmd_Rotator_position,				//*	Returns the rotator's current position.
	kCmd_Rotator_reverse,				//*	Returns the rotator's Reverse state.
										//*	Sets the rotator's Reverse state.
	kCmd_Rotator_stepsize,				//*	Returns the minimum StepSize
	kCmd_Rotator_targetposition,		//*	Returns the destination position angle.
	kCmd_Rotator_halt,					//*	Immediately stops rotator motion.
	kCmd_Rotator_move,					//*	Moves the rotator to a new relative position.
	kCmd_Rotator_moveabsolute,			//*	Moves the rotator to a new absolute position.
	kCmd_Rotator_movemechanical,		//*	Moves the rotator to a new raw mechanical position.
	kCmd_Rotator_sync,					//*	Syncs the rotator to the specified position angle without moving it.

	//*	added by MLS
	kCmd_Rotator_Extras,
	kCmd_Rotator_step,					//*	Moves the rotator to a new relative position.
	kCmd_Rotator_stepabsolute,			//*	Moves the rotator to a new absolute position.

	kCmd_Rotator_readall,				//*
};


#endif // _ROTATOR_ALPACA_CMDS_H_
