//*****************************************************************************
//*		controller_focus.cpp		(c) 2020 by Mark Sproul
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
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Feb 20,	2020	<MLS> Created controller_focus.cpp
//*	Feb 22,	2020	<MLS> Communication with Alpaca device working
//*	Apr 24,	2020	<MLS> controller_focus.cpp is now an intermediate class
//*	Apr 24,	2020	<MLS> controller_focus.cpp will be used for MoonLite focusers
//*	Apr 24,	2020	<MLS> Added long list of UpdateWindowTabs_xxx() for updating class
//*	Apr 25,	2020	<MLS> Major re-write of focuser class structure complete
//*	Apr 26,	2020	<MLS> Added ReadNiteCrawlerColors()
//*	Apr 26,	2020	<MLS> Added UpdateWindowTabs_SwitchState()
//*	May  8,	2020	<MLS> Added UpdateWindowTabs_DesiredAuxPos()
//*	Jun 19,	2020	<MLS> USB port now gets closed by destructor
//*	Dec 28,	2020	<MLS> Added ZeroMotorValues()
//*	Jan 26,	2021	<MLS> Updated several routine names
//*	Jun 19,	2023	<MLS> Updated constructor to use TYPE_REMOTE_DEV
//*	Jul  1,	2023	<MLS> Added cFocuserProp to focus controller
//*	Jul  8,	2023	<MLS> Changing the way property updates are handled
//*****************************************************************************
//*	From the Nitecrawler web site
//*	Rotating drawtube .001 degree resolution
//*	WR25 374,920 steps per revolution
//*	WR30 444,080 steps per revolution
//*	WR35 505,960 steps per revolution
//*	Approx. .9  travel, 94,580 steps per inch resolution
//*	.0000105 inch per step or .2667 Microns per step
//*****************************************************************************

#ifdef _ENABLE_CTRL_FOCUSERS_

#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>

#include	"discovery_lib.h"
#include	"helper_functions.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"

#include	"moonlite_com.h"
#include	"controller.h"
#include	"controller_focus.h"
#include	"controller_focus_ml_nc.h"
#include	"controller_focus_ml_hr.h"
#include	"controller_focus_generic.h"
#include	"focuser_common.h"
#include	"nitecrawler_colors.h"

#include	"focuser_AlpacaCmds.h"
#include	"focuser_AlpacaCmds.cpp"


#pragma mark -


//**************************************************************************************
ControllerFocus::ControllerFocus(	const char			*argWindowName,
									TYPE_REMOTE_DEV		*alpacaDevice,
									const int			focuserType)
	:Controller(argWindowName, kFocuserBoxWidth,  kFocuserBoxHeight, true, alpacaDevice)
{


	strcpy(cAlpacaDeviceTypeStr,	"focuser");

	//*	moved all init stuff to separate routine so we can have multiple constructors
	ControllerFocusInit(kComMode_Alpaca, focuserType);

	SetupWindowControls();

#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_
}

//**************************************************************************************
ControllerFocus::ControllerFocus(	const char			*argWindowName,
									const char			*usbPortPath,
									int					focuserType)
	:Controller(argWindowName, kFocuserBoxWidth,  kFocuserBoxHeight)
{
	CONSOLE_DEBUG(__FUNCTION__);

	cValidUSB		=	true;
	cValidIPaddr	=	false;
	cOnLine			=	false;

	//*	moved all init stuff to separate routine so we can have multiple constructors
	ControllerFocusInit(kComMode_USB, focuserType);

	strcpy(cUSBpath, usbPortPath);

	SetupWindowControls();

	SetWindowIPaddrInfo(cUSBpath, true);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerFocus::~ControllerFocus(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	if (cUSBportOpen)
	{
		CONSOLE_DEBUG("Closing USB port");
		CloseUSBport();
	}
}

//**************************************************************************************
void	ControllerFocus::ControllerFocusInit(const int comMode, const int focuserType)
{
	CONSOLE_DEBUG(__FUNCTION__);

	SetCommandLookupTable(gFocuserCmdTable);
	SetAlternateLookupTable(gFocuserExtrasTable);

	memset(&cFocuserProp, 0, sizeof(TYPE_FocuserProperties));
	cFocuserProp.IsMoving	=	false;
	cFocuserProp.Position	=	0;

	cCommMode			=	comMode;
	cFocuserType		=	focuserType;

	cOnLine				=	true;		//*	assume its online, if it wasn't, we wouldn't be here
	cReadStartup		=	true;
	cRotatorPosition	=	0;
//	cFocuserPosition	=	0;
	cFocuserDesiredPos	=	0;
	cRotatorDesiredPos	=	0;
	cAuxMotorPosition	=	0;
	cAuxMotorDesiredPos	=	0;
	cReadFailureCnt		=	0;
	cValidUSB			=	false;
	cUSBportOpen		=	false;
	cFirstDataRead		=	true;
	cModelName[0]		=	0;
	cSerialNumber[0]	=	0;
	cUnitVersion[0]		=	0;
	cStepsPerRev		=	kStepsPerRev_WR30;

	cLastUpdate_milliSecs		=	millis();
	cLastTimeSecs_Temperature	=	0;

	memset(&cMoonliteCom, 0, sizeof(TYPE_MOONLITECOM));
}


//**************************************************************************************
void	ControllerFocus::CreateWindowTabs(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::GetStartUpData_SubClass(void)
{
bool	validData;

	CONSOLE_DEBUG(__FUNCTION__);

	if (cUSBportOpen)
	{
		//*	the usb port is open, get data that way
		validData	=	USBGetStatus();
		if (validData != true)
		{
			CONSOLE_DEBUG("USBGetStatus() failed!!");
		}
	}

	CONSOLE_DEBUG_W_NUM("cFocuserProp.Position\t=",	cFocuserProp.Position);
	CONSOLE_DEBUG_W_NUM("cRotatorPosition\t=",		cRotatorPosition);
	CONSOLE_DEBUG_W_NUM("cAuxMotorPosition\t=",		cAuxMotorPosition);

	cFocuserDesiredPos	=	cFocuserProp.Position;
	cRotatorDesiredPos	=	cRotatorPosition;
	cAuxMotorDesiredPos	=	cAuxMotorPosition;

	CONSOLE_DEBUG_W_NUM("cFocuserDesiredPos\t=",	cFocuserDesiredPos);
	CONSOLE_DEBUG_W_NUM("cRotatorDesiredPos\t=",	cRotatorDesiredPos);
	CONSOLE_DEBUG_W_NUM("cAuxMotorDesiredPos\t=",	cAuxMotorDesiredPos);
}

//*****************************************************************************
void	ControllerFocus::GetStatus_SubClass(void)
{
bool	validData;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_BOOL("cUSBportOpen\t=", cUSBportOpen);
	if (cFocuserProp.IsMoving)
	{
		cUpdateDelta_secs	=	1;
	}
	else
	{
		cUpdateDelta_secs	=	5;
	}
	if (cUSBportOpen)
	{
		//*	the usb port is open, get data that way
		validData	=	USBGetStatus();
		if (validData != true)
		{
			CONSOLE_DEBUG("USBGetStatus() failed!!");
		}
	}
}

//*****************************************************************************
void	ControllerFocus::AlpacaProcessSupportedActions(const char *deviceTypeStr, const int deviveNum, const char *valueString)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	if (strcasecmp(valueString, "readall") == 0)
	{
		cHas_readall	=	true;
	}
	else if (strcasecmp(valueString, "foo") == 0)
	{
		//*	you get the idea
	}
}

//*****************************************************************************
void	ControllerFocus::AlpacaDisplayErrorMessage(const char *errorMsgString)
{
	CONSOLE_DEBUG_W_STR("Alpaca error\t=",	errorMsgString);
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
//	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateRotatorPosition(void)
{
	//*	This function should be overloaded
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
//	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
//	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateVoltage(const double newVoltage)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateStepsPerRev(const int newStepsPerRev)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateStartupData(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	CONSOLE_DEBUG_W_NUM("cFocuserProp.Position\t=",	cFocuserProp.Position);
	CONSOLE_DEBUG_W_NUM("cRotatorPosition\t=",		cRotatorPosition);
	CONSOLE_DEBUG_W_NUM("cAuxMotorPosition\t=",		cAuxMotorPosition);

	cFocuserDesiredPos	=	cFocuserProp.Position;
	cRotatorDesiredPos	=	cRotatorPosition;
	cAuxMotorDesiredPos	=	cAuxMotorPosition;

	CONSOLE_DEBUG_W_NUM("cFocuserDesiredPos\t=",	cFocuserDesiredPos);
	CONSOLE_DEBUG_W_NUM("cRotatorDesiredPos\t=",	cRotatorDesiredPos);
	CONSOLE_DEBUG_W_NUM("cAuxMotorDesiredPos\t=",	cAuxMotorDesiredPos);

	UpdateWindowTabs_Everything();

	cFirstDataRead		=	false;
}

//**************************************************************************************
void	ControllerFocus::UpdateStatusData(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be over-ridden");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//**************************************************************************************
void	ControllerFocus::UpdateOnlineStatus(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be over-ridden");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
//	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_Everything(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be over-ridden");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_SwitchBits(unsigned char switchBits)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be over-ridden");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_AuxSwitchBits(unsigned char auxSwitchBits)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be over-ridden");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_DesiredFocusPos(const int newDesiredPoistion)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be over-ridden");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_DesiredRotatorPos(const int newDesiredPoistion)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be over-ridden");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_DesiredAuxPos(const int newDesiredPoistion)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be over-ridden");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_ConnectState(bool connectedState)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be over-ridden");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_SwitchState(int switchId, bool onOffState)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be over-ridden");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateSupportedActions(void)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be over-ridden");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_ReadAll(bool hasReadAll)
{
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "needs to be over-ridden");
	CONSOLE_DEBUG_W_STR("cWindowName \t=",	cWindowName);
	CONSOLE_ABORT(__FUNCTION__);
}

//*****************************************************************************
bool	ControllerFocus::AlpacaProcessReadAllIdx(	const char	*deviceTypeStr,
													const int	deviceNum,
													const int	keywordEnum,
													const char	*valueString)
{
double		argDouble;
bool		switchStatus;
bool		dataWasHandled	=	true;

	switch(keywordEnum)
	{
		case kCmd_Focuser_absolute:				//*	Indicates whether the focuser is capable of absolute position.
			cFocuserProp.Absolute		=	IsTrueFalse(valueString);
			break;

		case kCmd_Focuser_ismoving:				//*	Indicates whether the focuser is currently moving.
			cFocuserProp.IsMoving		=	IsTrueFalse(valueString);
			break;

		case kCmd_Focuser_maxincrement:			//*	Returns the focuser's maximum increment size.
			cFocuserProp.MaxIncrement	=	atoi(valueString);
			break;

		case kCmd_Focuser_maxstep:				//*	Returns the focuser's maximum step size.
			cFocuserProp.MaxStep		=	atoi(valueString);
			break;

		case kCmd_Focuser_position:				//*	Returns the focuser's current position.
			cFocuserProp.Position		=	atoi(valueString);
			break;

		case kCmd_Focuser_stepsize:				//*	Returns the focuser's step size.
			cFocuserProp.StepSize		=	atoi(valueString);
			break;

		case kCmd_Focuser_tempcomp:				//*	Retrieves the state of temperature compensation mode
			cFocuserProp.TempComp		=	IsTrueFalse(valueString);
			break;

		case kCmd_Focuser_tempcompavailable:	//*	Indicates whether the focuser has temperature compensation.
			cFocuserProp.TempCompAvailable	=	IsTrueFalse(valueString);
			break;

		case kCmd_Focuser_temperature:			//*	Returns the focuser's current temperature.
			cFocuserProp.Temperature_DegC	=	AsciiToDouble(valueString);
			break;

		//-------------------------------------------------
		//*	extras stuff from readall
		case kCmd_Focuser_AuxPosition:
			cAuxMotorPosition	=	atoi(valueString);
			break;

		case kCmd_Focuser_AuxIsMoving:
			cFocuserProp.IsMoving	=	IsTrueFalse(valueString);
			break;

		case kCmd_Focuser_DegreesF:
			break;

		case kCmd_Focuser_Model:
			strcpy(cModelName, valueString);
			//*	set the steps per revolution based on the model
			if (strstr(cModelName, "2.5") != NULL)
			{
				UpdateStepsPerRev(kStepsPerRev_WR25);
			}
			else if (strstr(cModelName, "3.0") != NULL)
			{
				UpdateStepsPerRev(kStepsPerRev_WR30);
			}
			else if (strstr(cModelName, "3.5") != NULL)
			{
				UpdateStepsPerRev(kStepsPerRev_WR35);
			}
			break;

		case kCmd_Focuser_RotatorIsMoving:
			cFocuserProp.IsMoving	=	IsTrueFalse(valueString);
			break;

		case kCmd_Focuser_RotatorPosition:
			cRotatorPosition	=	atoi(valueString);
			UpdateRotatorPosition();
			break;

		case kCmd_Focuser_SerialNum:
			strcpy(cSerialNumber, valueString);
			break;

		case kCmd_Focuser_SwitchAux1:
			break;

		case kCmd_Focuser_SwitchAux2:
			break;

		case kCmd_Focuser_SwitchIn:
			switchStatus	=	IsTrueFalse(valueString);
			UpdateWindowTabs_SwitchState(kSwitch_In, switchStatus);
			break;

		case kCmd_Focuser_SwitchOut:
			switchStatus	=	IsTrueFalse(valueString);
			UpdateWindowTabs_SwitchState(kSwitch_Out, switchStatus);
			break;

		case kCmd_Focuser_SwitchRot:
			switchStatus	=	IsTrueFalse(valueString);
			UpdateWindowTabs_SwitchState(kSwitch_Rot, switchStatus);
			break;

		case kCmd_Focuser_Voltage:
			argDouble	=	AsciiToDouble(valueString);
			UpdateVoltage(argDouble);
			break;

		default:
			dataWasHandled	=	false;
			break;
	}
	return(dataWasHandled);
}

//*****************************************************************************
bool	ControllerFocus::AlpacaGetStartupData_OneAAT(void)
{
bool		validData;
int			argInt;

	//========================================================
	validData	=	AlpacaGetIntegerValue(	"focuser", "position",	NULL,	&argInt);
	if (validData)
	{
		CONSOLE_DEBUG_W_NUM("new focuser position\t=",	argInt);
		cFocuserProp.Position		=	argInt;
	}
	else
	{
		cReadFailureCnt++;
	}

	return(validData);
}


//*****************************************************************************
//*	Get Status, One At A Time
//*****************************************************************************
bool	ControllerFocus::AlpacaGetStatus_OneAAT(void)
{
bool		validData;
int			myFailureCount;
double		argDouble;
int			argInt;
bool		argBool;

	CONSOLE_DEBUG(__FUNCTION__);

	myFailureCount	=	0;
	//========================================================
	validData	=	AlpacaGetBooleanValue(	"focuser", "ismoving",	NULL,	&argBool);
	if (validData)
	{
		cFocuserProp.IsMoving	=	argBool;
		CONSOLE_DEBUG_W_BOOL("cFocuserProp.IsMoving\t=",	cFocuserProp.IsMoving);
	}
	else
	{
		cReadFailureCnt++;
		myFailureCount++;
	}
	//========================================================
	validData	=	AlpacaGetIntegerValue(	"focuser", "position",	NULL,	&argInt);
	if (validData)
	{
		CONSOLE_DEBUG_W_NUM("new focuser position\t=",	argInt);
		cFocuserProp.Position		=	argInt;
	}
	else
	{
		cReadFailureCnt++;
		myFailureCount++;
	}
	//========================================================
	validData	=	AlpacaGetDoubleValue(	"focuser", "temperature",	NULL,	&argDouble);
	if (validData)
	{
		CONSOLE_DEBUG_W_DBL("new focuser temp\t=",	argDouble);
		cFocuserProp.Temperature_DegC		=	argDouble;
	}
	else
	{
		cReadFailureCnt++;
		myFailureCount++;
	}

	if (myFailureCount < 2)
	{
		validData	=	true;
	}
	else
	{
		validData	=	false;
	}
	return(validData);
}

//*****************************************************************************
void	ControllerFocus::AlpacaGetCapabilities(void)
{
	//*	Focuser does not have any "Capabilities"
	//*	required for base class
}

//**************************************************************************************
void	ControllerFocus::UpdateConnectedStatusIndicator(void)
{
	//*	needs to be over-ridden by subclass
}

//*****************************************************************************
bool	ControllerFocus::AlpacaGetStatus(void)
{
bool	validData;
char	lineBuff[128];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	cFocuserProp.IsMoving	=	false;
	validData				=	false;
	if (cHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll("focuser", cAlpacaDevNum);
		sprintf(lineBuff, "%s-%s", cModelName, cSerialNumber);
//-		SetWidgetText(kTab_Focuser, kFocusTab_Model, lineBuff);

		SetWindowIPaddrInfo(NULL, true);

		if (cFocuserProp.Position != cFocuserDesiredPos)
		{
			cFocuserProp.IsMoving	=	true;
		}
		if (cRotatorPosition != cRotatorDesiredPos)
		{
			cFocuserProp.IsMoving	=	true;
		}
		cLastUpdate_milliSecs	=	millis();
	}
	else
	{
		validData	=	AlpacaGetStatus_OneAAT();	//*	One At A Time
	}

	if (validData)
	{
		//*	check to see if we were one line before
		if (cOnLine == false)
		{
			//*	if we go from offline back to online, re-do the startup info
			cReadStartup	=	true;
		}
		cOnLine	=	true;
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Offline-", cWindowName);
		cOnLine	=	false;
	}
//	CONSOLE_DEBUG(__FUNCTION__);
	SetWindowIPaddrInfo(NULL, cOnLine);


	cLastUpdate_milliSecs	=	millis();
	return(validData);
}


//*****************************************************************************
bool	ControllerFocus::USBGetStatus(void)
{
bool			validUSBdata;
int32_t			newPositionValue;
uint32_t		currentMillis;
uint32_t		currentSeconds;
double			newValeDbl;
unsigned char	switchBits;
unsigned char	auxSwitchBits;
unsigned char	previousSwitchBits;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);

	cFocuserProp.IsMoving	=	false;
	//=================================================================
	validUSBdata	=	MoonLite_GetPosition(	&cMoonliteCom,
												1,
												&newPositionValue);
	if (validUSBdata)
	{
		if (newPositionValue != cFocuserProp.Position)
		{
			cFocuserProp.IsMoving	=	true;
		}
//		CONSOLE_DEBUG_W_NUM("newPositionValue\t=",		newPositionValue);
		cFocuserProp.Position		=	newPositionValue;
	}
	else
	{
		CONSOLE_DEBUG("Error getting data");
	}

	if (cMoonliteCom.model == kMoonLite_NiteCrawler)
	{
		//=================================================================
		validUSBdata	=	MoonLite_GetPosition(	&cMoonliteCom,
													2,
													&newPositionValue);
		if (validUSBdata)
		{
			if (newPositionValue != cRotatorPosition)
			{
				cFocuserProp.IsMoving	=	true;
			}
			cRotatorPosition	=	newPositionValue;
			UpdateRotatorPosition();
		}
		else
		{
			CONSOLE_DEBUG("Error getting data");
		}

		//=================================================================
		validUSBdata	=	MoonLite_GetPosition(	&cMoonliteCom,
													3,
													&newPositionValue);
		if (validUSBdata)
		{
			if (newPositionValue != cAuxMotorPosition)
			{
				cFocuserProp.IsMoving	=	true;
			}
			cAuxMotorPosition	=	newPositionValue;
		}
		else
		{
			CONSOLE_DEBUG("Error getting data, MoonLite_GetPosition");
		}

		//=================================================================
		previousSwitchBits	=	cMoonliteCom.switchBits;
		switchBits			=	0;
		validUSBdata		=	MoonLite_GetSwiches(&cMoonliteCom, &switchBits);
		if (validUSBdata)
		{
			if (switchBits != 0)
			{
//				CONSOLE_DEBUG_W_HEX("switchBits\t=", switchBits);
			}
			//*	The GS query the switch status for the limit and rotation home switches:
			//*	b0=Rotation switch
			//*	b1=Out limit switch
			//*	b2= In limit switch
			if (switchBits != previousSwitchBits)
			{
				//*	we have a change, deal with it
				UpdateWindowTabs_SwitchBits(switchBits);
			}
		}
		else
		{
			CONSOLE_DEBUG("Error getting from MoonLite_GetSwiches");
		}

		//=================================================================
		previousSwitchBits	=	cMoonliteCom.auxSwitchBits;
		validUSBdata		=	MoonLite_GetAuxSwiches(&cMoonliteCom, &auxSwitchBits);
		if (validUSBdata)
		{
			//*	GA is the AUX channel switch status:
			//*	b0= Out limit
			//*	b1= In lmit
			if (auxSwitchBits != previousSwitchBits)
			{
				UpdateWindowTabs_AuxSwitchBits(auxSwitchBits);
			}
		}
		else
		{
			CONSOLE_DEBUG("Error getting from MoonLite_GetAuxSwiches");
		}
	}


	currentMillis	=	millis();
	currentSeconds	=	currentMillis / 1000;

//	CONSOLE_DEBUG(__FUNCTION__);
	//*	Do NOT check the temperature if its moving
	if (cFocuserProp.IsMoving == false)
	{
//		CONSOLE_DEBUG("cFocuserProp.IsMoving is false");
		//*	check the temperature every 10 seconds
		if (cFirstDataRead ||  ((currentSeconds - cLastTimeSecs_Temperature) > 5))
		{
//			CONSOLE_DEBUG("getting ready to check temperature");
			validUSBdata	=	MoonLite_GetTemperature(&cMoonliteCom, &newValeDbl);
			if (validUSBdata)
			{
				cFocuserProp.Temperature_DegC		=	newValeDbl;
			}
			//*	only the NiteCrawler has voltage
			if (cMoonliteCom.model == kMoonLite_NiteCrawler)
			{
				validUSBdata	=	MoonLite_GetVoltage(&cMoonliteCom, &newValeDbl);
				if (validUSBdata)
				{
					UpdateVoltage(newValeDbl);
				}
			}
			cLastTimeSecs_Temperature	=	currentSeconds;
		}
	}
	cLastUpdate_milliSecs	=	millis();
	return(validUSBdata);
}

//*****************************************************************************
void	ControllerFocus::SendMoveFocuserCommand(int newDesiredPosition)
{
char		dataString[128];
bool		validData;

	CONSOLE_DEBUG(__FUNCTION__);

	cFocuserDesiredPos	=	newDesiredPosition;

	UpdateWindowTabs_DesiredFocusPos(cFocuserDesiredPos);

	if (cValidIPaddr)
	{

		sprintf(dataString, "Position=%d", cFocuserDesiredPos);
		validData	=	AlpacaSendPutCmd(	"focuser",	"move",		dataString);
		if (!validData)
		{
			CONSOLE_DEBUG("Error sending move command");
		}
	}
	else if (cUSBportOpen)
	{
		CONSOLE_DEBUG("Sending move command to USB port");
		MoonLite_SetPosition(	&cMoonliteCom,
								1,
								cFocuserDesiredPos);
	}
	else
	{
		CONSOLE_DEBUG("We do not have a valid IP address to query");
	}

	cFocuserProp.IsMoving	=	true;
}


//*****************************************************************************
void	ControllerFocus::SendMoveRotatorCommand(int newDesiredPosition)
{
char			dataString[128];
bool			validData;
double			newDegreesValue;

	cRotatorDesiredPos	=	newDesiredPosition;
	UpdateWindowTabs_DesiredRotatorPos(cRotatorDesiredPos);
	cFocuserProp.IsMoving	=	true;

	if (cValidIPaddr)
	{
		if (cHas_readall)
		{
			//================================================
			//*	stepabsolute is an added command by MLS
			//*	it deals in step units instead of degrees
			sprintf(dataString, "Position=%d", cRotatorDesiredPos);
			validData	=	AlpacaSendPutCmd(	"rotator",	"stepabsolute",		dataString);
			if (!validData)
			{
				CONSOLE_DEBUG("Error rotator/stepabsolute");
			}
		}
		else
		{
			if (cStepsPerRev > 0)
			{
				newDegreesValue	=	((newDesiredPosition * 360.0)/ cStepsPerRev);

				sprintf(dataString, "Position=%1.5f", newDegreesValue);
				CONSOLE_DEBUG_W_DBL("newDegreesValue\t=",	newDegreesValue);
				CONSOLE_DEBUG_W_STR("dataString\t=",		dataString);

				validData	=	AlpacaSendPutCmd(	"rotator",	"moveabsolute",		dataString);
			}
			else
			{
				CONSOLE_DEBUG("cStepsPerRev is zero, we cannot determine a new angle");
			}
		}
	}
	else if (cUSBportOpen)
	{
		MoonLite_SetPosition(	&cMoonliteCom,
								2,
								cRotatorDesiredPos);
	}
	else
	{
		cFocuserProp.IsMoving	=	false;
		CONSOLE_DEBUG("We do not have a valid IP address to query");
	}
}

//*****************************************************************************
void	ControllerFocus::SendMoveAuxMotorCommand(int newDesiredPosition)
{
char			dataString[128];
bool			validData;

	CONSOLE_DEBUG(__FUNCTION__);

	cAuxMotorDesiredPos	=	newDesiredPosition;
	UpdateWindowTabs_DesiredAuxPos(cAuxMotorDesiredPos);

	cFocuserProp.IsMoving	=	true;
	if (cValidIPaddr)
	{
		sprintf(dataString, "Position=%d", cRotatorDesiredPos);

		validData	=	AlpacaSendPutCmd(	"auxmotor",	"moveabsolute",		dataString);
		if (!validData)
		{
			CONSOLE_DEBUG("Error auxmotor");
		}
	}
	else if (cUSBportOpen)
	{
		MoonLite_SetPosition(	&cMoonliteCom,
								3,
								cAuxMotorDesiredPos);
	}
	else
	{
		cFocuserProp.IsMoving	=	false;
		CONSOLE_DEBUG("We do not have a valid IP address to query");
	}
}

//*****************************************************************************
void	ControllerFocus::SendStopMotorsCommand(void)
{
bool		validData;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cValidIPaddr)
	{
		validData	=	AlpacaSendPutCmd(	"focuser",	"halt",		"");

		if (cFocuserType == kMoonLite_NiteCrawler)
		{
			validData	=	AlpacaSendPutCmd(	"rotator",	"halt",		"");
		}
		if (!validData)
		{
			CONSOLE_DEBUG("Error halting motors");
		}
	}
	else if (cUSBportOpen)
	{
		MoonLite_StopMotors(&cMoonliteCom);
	}
	else
	{
		CONSOLE_DEBUG("We do not have a valid IP/usb port address to query");
	}
	UpdateStartupData();
}

//*****************************************************************************
void	ControllerFocus::ZeroMotorValues(void)
{
bool			validUSBdata;
unsigned char	switchBits;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cValidIPaddr)
	{
		CONSOLE_DEBUG("Not implemented via Alpaca")
	}
	else if (cUSBportOpen)
	{
		CONSOLE_DEBUG("USB port is open");
		//*	this is going to set the current rotation position to zero
		//*	IF it is at the rotational HOME position
		validUSBdata		=	MoonLite_GetSwiches(&cMoonliteCom, &switchBits);
		if (validUSBdata && (switchBits & 0x01))
		{
			CONSOLE_DEBUG("Rotator is in the HOME position");
			CONSOLE_DEBUG(__FUNCTION__);
			validUSBdata	=	MoonLite_SetCurrentPosition(&cMoonliteCom,
															2,
															0);
			if (validUSBdata)
			{
				CONSOLE_DEBUG("Rotator appears to have been reset to zero");
			}
			else
			{
				CONSOLE_DEBUG("Command Failure");
			}
		}
		else
		{
			CONSOLE_DEBUG("Rotator NOT at Home");
		}

		//===========================================================
		//*	set the AUX to zero regardless of switches
		validUSBdata	=	MoonLite_SetCurrentPosition(	&cMoonliteCom,
														3,
														0);
		if (validUSBdata)
		{
			CONSOLE_DEBUG("Aux appears to have been reset to zero");
		}
		else
		{
			CONSOLE_DEBUG("Command Failure");
		}
	}
	else
	{
		CONSOLE_DEBUG("We do not have a valid IP/usb port address to query");
	}
	UpdateStartupData();
}

//*****************************************************************************
void	ControllerFocus::ReadNiteCrawlerColors(void)
{
bool		validData;
int			iii;
uint16_t	colorValue16Bit;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cValidIPaddr)
	{
	}
	else if (cUSBportOpen)
	{
		if (cMoonliteCom.model == kMoonLite_NiteCrawler)
		{
			for (iii=0; iii<12; iii++)
			{
				//*	reset the color value
				gNiteCrawlerColors[iii].color16bit	=	0;

				validData	=	MoonLite_GetNC_Color(	&cMoonliteCom,
														gNiteCrawlerColors[iii].colorNumber,
														&colorValue16Bit);
				if (validData)
				{
					gNiteCrawlerColors[iii].color16bit	=	colorValue16Bit;
//					printf("Color %2d=\t%04X\tDef=%04X\t%s",	gNiteCrawlerColors[iii].colorNumber,
//																gNiteCrawlerColors[iii].color16bit,
//																gNiteCrawlerColors[iii].color16bitDefault,
//																gNiteCrawlerColors[iii].name);
//
//					if (gNiteCrawlerColors[iii].color16bit != gNiteCrawlerColors[iii].color16bitDefault)
//					{
//						printf("\t<--- error");
//					}
//					printf("\r\n");
				}
				else
				{
					CONSOLE_DEBUG("Failed to read color");
				}
			}
			MoonLite_FlushReadBuffer(&cMoonliteCom);
		}
	}
	else
	{
		CONSOLE_DEBUG("We do not have a valid IP address to query");
	}
}


#pragma mark -
//*****************************************************************************
void	ControllerFocus::OpenUSBport(void)
{
bool	openOk;
char	lineBuff[128];

	CONSOLE_DEBUG(__FUNCTION__);

	memset(&cMoonliteCom, 0, sizeof(TYPE_MOONLITECOM));

	strcpy(cMoonliteCom.usbPortPath, cUSBpath);
	openOk	=	MoonLite_OpenFocuserConnection(&cMoonliteCom, (cFocuserType == kMoonLite_NiteCrawler));
	if (openOk)
	{
		gDebugBackgroundThread	=	true;
		cUSBportOpen		=	true;
		cFirstDataRead		=	true;
		cReadStartup		=	true;
		cFocuserType		=	cMoonliteCom.model;
		strcpy(cModelName,		cMoonliteCom.deviceModelString);
		strcpy(cUnitVersion,	cMoonliteCom.deviceVersion);
		strcpy(cSerialNumber,	cMoonliteCom.deviceSerialNum);
		if (strlen(cSerialNumber) > 0)
		{
			sprintf(lineBuff, "%s-%s", cModelName, cSerialNumber);
		}
		else
		{
			strcpy(lineBuff,		cModelName);
		}
		UpdateWindowTabs_ConnectState(true);


		if (cMoonliteCom.model == kMoonLite_NiteCrawler)
		{
			if (strstr(cModelName, "2.5") != NULL)
			{
				UpdateStepsPerRev(kStepsPerRev_WR25);
			}
			else if (strstr(cModelName, "3.0") != NULL)
			{
				UpdateStepsPerRev(kStepsPerRev_WR30);
			}
			else if (strstr(cModelName, "3.5") != NULL)
			{
				UpdateStepsPerRev(kStepsPerRev_WR35);
			}
		}
		ReadNiteCrawlerColors();
	}
	else
	{
		CONSOLE_DEBUG("Failed to get moonlite data");
	}
	CONSOLE_DEBUG_W_BOOL("cReadStartup          \t=", cReadStartup);
	CONSOLE_DEBUG_W_BOOL("gDebugBackgroundThread\t=", gDebugBackgroundThread);
}


//*****************************************************************************
void	ControllerFocus::CloseUSBport(void)
{
bool	closeOK;

	closeOK	=	MoonLite_CloseFocuserConnection(&cMoonliteCom);
	if (closeOK == false)
	{
		CONSOLE_DEBUG("Failed to closing usb port");
	}
	cUSBportOpen	=	false;

	UpdateWindowTabs_ConnectState(false);
}



//*****************************************************************************
//*	returns focuser type as per enum
//*	figure out a window name
//*****************************************************************************
int	GenerateFocuserWindowName(TYPE_REMOTE_DEV *device, int focuserNum, char *windowName)
{
int	myFocuserTYpe;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG(device->deviceNameStr);

	//*	is "NiteCrawler" in the name???
	if (strcasestr(device->deviceNameStr, "NiteCrawler") != NULL)
	{
		myFocuserTYpe	=	kFocuserType_NiteCrawler;
		if (strlen(device->hostName) > 0)
		{
			//*	if we have a host name, use it
			if (device->alpacaDeviceNum > 0)
			{
				//*	if there are more than one device on this host, we need to make the window name unique
				sprintf(windowName, "NiteCrawler-%s-%d",
									device->hostName,
									device->alpacaDeviceNum);
			}
			else
			{
				sprintf(windowName, "NiteCrawler-%s",
									device->hostName);
			}
		}
		else
		{
			sprintf(windowName, "NiteCrawler -%d", focuserNum);
		}

	}
	else if (strncasecmp(device->deviceNameStr, "Moonlite", 8) == 0)
	{
		myFocuserTYpe	=	kFocuserType_MoonliteSingle;
		sprintf(windowName, "Moonlite -%d", focuserNum);
		if (strlen(device->hostName) > 0)
		{
			//*	if we have a host name, use it
			if (device->alpacaDeviceNum > 0)
			{
				//*	if there are more than one device on this host, we need to make the window name unique
				sprintf(windowName, "Moonlite-%s-%d",
									device->hostName,
									device->alpacaDeviceNum);
			}
			else
			{
				sprintf(windowName, "Moonlite-%s",
									device->hostName);
			}
		}
		else
		{
			sprintf(windowName, "Moonlite -%d", focuserNum);
		}
	}
	else if (strcasestr(device->deviceNameStr, "Shelyak") != NULL)
	{
		myFocuserTYpe	=	kFocuserType_Shelyak;
		sprintf(windowName, "%s -%d", device->deviceNameStr, focuserNum);
	}
	else
	{
		myFocuserTYpe	=	kFocuserType_Other;
		sprintf(windowName, "Focuser -%d", focuserNum);

	}
	CONSOLE_DEBUG(windowName);
	return(myFocuserTYpe);
}

int		gFocuserNum		=	1;

//*****************************************************************************
int	CheckForFocuser(TYPE_REMOTE_DEV *remoteDevice)
{
Controller		*myController;
char			windowName[128]	=	"Moonlite NiteCrawler";
int				objectsCreated;
int				myFocuserType;

	objectsCreated	=	0;
	//*	is it a focuser?
	if (strcasecmp(remoteDevice->deviceTypeStr, "focuser") == 0)
	{
		//*	figure out a window name
		myFocuserType	=	GenerateFocuserWindowName(remoteDevice, gFocuserNum, windowName);
		CONSOLE_DEBUG_W_NUM("myFocuserType\t=", myFocuserType);

		//*	create the controller window object
		if (myFocuserType == kFocuserType_NiteCrawler)
		{
			CONSOLE_DEBUG("Focuser is Moonlite NiteCrawler");
//			myController	=	new ControllerNiteCrawler(	windowName,
//															&remoteDevice->deviceAddress,
//															remoteDevice->port,
//															remoteDevice->alpacaDeviceNum);
			myController	=	new ControllerNiteCrawler(	windowName,
															remoteDevice);

		}
		else if (myFocuserType == kFocuserType_MoonliteSingle)
		{
			CONSOLE_DEBUG("Focuser is Moonlite Single");
			myController	=	new ControllerMLsingle(		windowName,
															remoteDevice);

//			myController	=	new ControllerMLsingle(		windowName,
//															&remoteDevice->deviceAddress,
//															remoteDevice->port,
//															remoteDevice->alpacaDeviceNum);
		}
		else
		{
			CONSOLE_DEBUG("Focuser is Generic");
			myController	=	new ControllerFocusGeneric(	windowName,
															remoteDevice);
//			myController	=	new ControllerFocusGeneric(	windowName,
//															&remoteDevice->deviceAddress,
//															remoteDevice->port,
//															remoteDevice->alpacaDeviceNum);

		}
		if (myController != NULL)
		{
			//*	force window update
			myController->HandleWindowUpdate();
			cv::waitKey(100);
		}

		gFocuserNum++;
		objectsCreated++;
	}
	return(objectsCreated);
}

#endif	//	_ENABLE_CTRL_FOCUSERS_
