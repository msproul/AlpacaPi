//*****************************************************************************
//*	Jul  1,	2023	<MLS> Created switch_AlpacaCmds.h
//*****************************************************************************
//#include	"switch_AlpacaCmds.h"

#ifndef _SWITCH_CMDS_H_


#define	_SWITCH_CMDS_H_

#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

//extern	TYPE_LookupTable	gCmdTbl_Switch[];
extern	TYPE_CmdEntry		gSwitchCmdTable[];


//*****************************************************************************
//switch
//*****************************************************************************
enum
{
	kCmd_Switch_maxswitch	=	0,		//*	The number of switch devices managed by this driver
	kCmd_Switch_canwrite,				//*	Indicates whether the specified switch device can be written to
	kCmd_Switch_getswitch,				//*	Return the state of switch device id as a boolean
	kCmd_Switch_getswitchdescription,	//*	Gets the description of the specified switch device
	kCmd_Switch_getswitchname,			//*	Gets the name of the specified switch device
	kCmd_Switch_getswitchvalue,			//*	Gets the value of the specified switch device as a double
	kCmd_Switch_minswitchvalue,			//*	Gets the minimum value of the specified switch device as a double
	kCmd_Switch_maxswitchvalue,			//*	Gets the maximum value of the specified switch device as a double
	kCmd_Switch_setswitch,				//*	Sets a switch controller device to the specified state, true or false
	kCmd_Switch_setswitchname,			//*	Sets a switch device name to the specified value
	kCmd_Switch_setswitchvalue,			//*	Sets a switch device value to the specified value
	kCmd_Switch_switchstep,				//*	Returns the step size that this device supports (the difference between successive values of the device).

	//*	added by MLS
	kCmd_Switch_Extras,
	kCmd_Switch_setswitchdescription,
	kCmd_Switch_readall,

	kCmd_Switch_last
};

#endif // _SWITCH_CMDS_H_
