//**************************************************************************
//*	Name:			switchdriver_sim.cpp
//*
//*	Author:			Mark Sproul
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.  You
//*	must determine the suitability of this source code for your use.
//*
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar  2,	2023	<MLS> Created switchdriver_sim.cpp
//*	Mar  3,	2023	<MLS> CONFORMU-switch/simulator -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*****************************************************************************

#ifdef _ENABLE_SWITCH_SIMULATOR_

#include	<string.h>
#include	<unistd.h>


#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"switchdriver.h"
#include	"switchdriver_sim.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


//*****************************************************************************
void	CreateSwitchObjects_SIM(void)
{
	new SwitchDriverSIM();
}


//**************************************************************************************
SwitchDriverSIM::SwitchDriverSIM(void)
	:SwitchDriver()
{
int		iii;
//	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(cCommonProp.Name,		"AlpacaPi Switch Simulator");
	strcpy(cCommonProp.Description, "AlpacaPi Switch Simulator");

	cSwitchProp.MaxSwitch	=	0;

	for (iii = 0; iii < kMaxSwitchCnt; iii++)
	{
		cSwitchValues[iii]	=	false;
	}

	Init_Hardware();
}

//**************************************************************************************
SwitchDriverSIM::~SwitchDriverSIM( void )
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
void	SwitchDriverSIM::Init_Hardware(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	NOTE: cSwitchProp.MaxSwitch gets incremented in ConfigureSwitch()
	ConfigureSwitch(cSwitchProp.MaxSwitch, kSwitchType_Relay, 1);
	ConfigureSwitch(cSwitchProp.MaxSwitch, kSwitchType_Relay, 1);
	ConfigureSwitch(cSwitchProp.MaxSwitch, kSwitchType_Relay, 1);
	ConfigureSwitch(cSwitchProp.MaxSwitch, kSwitchType_Relay, 1);
	ConfigureSwitch(cSwitchProp.MaxSwitch, kSwitchType_Relay, 1);
	ConfigureSwitch(cSwitchProp.MaxSwitch, kSwitchType_Relay, 1);
	ConfigureSwitch(cSwitchProp.MaxSwitch, kSwitchType_Status, 1);
	ConfigureSwitch(cSwitchProp.MaxSwitch, kSwitchType_Analog, 1);

}

//*****************************************************************************
bool	SwitchDriverSIM::GetSwitchState(const int switchNumber)
{
bool	switchState;

//	CONSOLE_DEBUG(__FUNCTION__);
	switchState	=	cSwitchValues[switchNumber];

	return(switchState);
}

//*****************************************************************************
bool	SwitchDriverSIM::SetSwitchState(const int switchNumber, bool on_off)
{

//	CONSOLE_DEBUG(__FUNCTION__);
	if ((switchNumber >= 0) && (switchNumber < cSwitchProp.MaxSwitch))
	{
		cSwitchValues[switchNumber]	=	on_off;
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Switch number out of bounds:", switchNumber);
	}
	return(true);
}

//*****************************************************************************
bool	SwitchDriverSIM::SetSwitchValue(const int switchNumber, double switchValue)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	if ((switchNumber >= 0) && (switchNumber < cSwitchProp.MaxSwitch))
	{

	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Switch number out of bounds:", switchNumber);
	}
	return(true);
}

#endif // _ENABLE_SWITCH_SIMULATOR_


