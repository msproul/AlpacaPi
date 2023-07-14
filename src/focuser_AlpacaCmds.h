//*****************************************************************************
//*	Jul  1,	2023	<MLS> Created focuser_AlpacaCmds.h
//*****************************************************************************
//#include	"focuser_AlpacaCmds.h"

#ifndef _FOCUSER_ALPACA_CMDS_H_
#define	_FOCUSER_ALPACA_CMDS_H_

#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

extern	TYPE_CmdEntry	gFocuserCmdTable[];


//*****************************************************************************
//focuser
//*****************************************************************************
enum
{
	kCmd_Focuser_absolute	=	0,		//*	Indicates whether the focuser is capable of absolute position.
	kCmd_Focuser_ismoving,				//*	Indicates whether the focuser is currently moving.
	kCmd_Focuser_maxincrement,			//*	Returns the focuser's maximum increment size.
	kCmd_Focuser_maxstep,				//*	Returns the focuser's maximum step size.
	kCmd_Focuser_position,				//*	Returns the focuser's current position.
	kCmd_Focuser_stepsize,				//*	Returns the focuser's step size.
	kCmd_Focuser_tempcomp,				//*	Retrieves the state of temperature compensation mode
										//*	Sets the device's temperature compensation mode.
	kCmd_Focuser_tempcompavailable,		//*	Indicates whether the focuser has temperature compensation.
	kCmd_Focuser_temperature,			//*	Returns the focuser's current temperature.
	kCmd_Focuser_halt,					//*	Immediately stops focuser motion.
	kCmd_Focuser_move,					//*	Moves the focuser to a new position.

	//*	added by MLS
	kCmd_Focuser_Extras,
	kCmd_Focuser_moverelative,			//*	Moves a specific amount
	kCmd_Focuser_readall,				//*	Reads all of the values


	kCmd_Focuser_last
};


//*****************************************************************************
enum
{
	kCmd_Focuser_firstone	=	(kCmd_Focuser_last + 1),
	kCmd_Focuser_AuxPosition,
	kCmd_Focuser_AuxIsMoving,
	kCmd_Focuser_DegreesF,
	kCmd_Focuser_Model,
	kCmd_Focuser_RotatorIsMoving,
	kCmd_Focuser_RotatorPosition,
	kCmd_Focuser_SerialNum,
	kCmd_Focuser_SwitchAux1,
	kCmd_Focuser_SwitchAux2,
	kCmd_Focuser_SwitchIn,
	kCmd_Focuser_SwitchOut,
	kCmd_Focuser_SwitchRot,
	kCmd_Focuser_Voltage,

	kCmd_Focuser_ExtrasLast

};
#endif // _FOCUSER_ALPACA_CMDS_H_
