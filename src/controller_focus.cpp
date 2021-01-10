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
//*	Redistributions of this source code must retain this copyright notice.
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
//*	Jun 19,	2020	<MLS> USB port now get closed by destructor
//*	Dec 28,	2020	<MLS> Added ZeroMotorValues()
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


#include "opencv/highgui.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"

#include	"discovery_lib.h"


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"moonlite_com.h"
#include	"controller.h"
#include	"controller_focus.h"
#include	"focuser_common.h"
#include	"nitecrawler_colors.h"

#define	kBoxWidth	302


#pragma mark -

//**************************************************************************************
//**************************************************************************************
//**************************************************************************************
//**************************************************************************************


//**************************************************************************************
ControllerFocus::ControllerFocus(	const char			*argWindowName,
									struct sockaddr_in	*deviceAddress,
									const int			port,
									const int			deviceNum,
									const int			focuserType)
	:Controller(argWindowName, kFocuserBoxWidth,  kFocuserBoxHeight)
{


	//*	moved all init stuff to separate routine so we can have multiple constructors
	ControllerFocusInit(kComMode_Alpaca, focuserType);


	cAlpacaDevNum	=	deviceNum;
	if (deviceAddress != NULL)
	{
		cDeviceAddress	=	*deviceAddress;
		cPort			=	port;
		cValidIPaddr	=	true;
	}
	SetupWindowControls();
}

//**************************************************************************************
ControllerFocus::ControllerFocus(	const char			*argWindowName,
									const char			*usbPortPath,
									int					focuserType)
	:Controller(argWindowName, kFocuserBoxWidth,  kFocuserBoxHeight)
{
	CONSOLE_DEBUG(__FUNCTION__);


	//*	moved all init stuff to separate routine so we can have multiple constructors
	ControllerFocusInit(kComMode_USB, focuserType);


	strcpy(cUSBpath, usbPortPath);
	cValidUSB	=	true;

	SetupWindowControls();

	SetWindowIPaddrInfo(cUSBpath, true);

}


//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerFocus::~ControllerFocus(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
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
	cCommMode			=	comMode;
	cOnLine				=	true;		//*	assume its online, if it wasnt, we wouldnt be here
	cReadStartup		=	true;
	cFocuserPosition	=	0;
	cRotatorPosition	=	0;
	cFocuserDesiredPos	=	0;
	cRotatorDesiredPos	=	0;
	cAuxMotorPosition	=	0;
	cAuxMotorDesiredPos	=	0;
	cReadFailureCnt		=	0;
	cValidIPaddr		=	false;
	cValidUSB			=	false;
	cUSBportOpen		=	false;
	cFirstDataRead		=	true;
	cModelName[0]		=	0;
	cSerialNumber[0]	=	0;
	cUnitVersion[0]		=	0;
	cStepsPerRev		=	kStepsPerRev_WR30;
	cAlpacaDevNum		=	0;

	cLastUpdate_milliSecs		=	millis();
	cLastTimeSecs_Temperature	=	0;

	memset(&cMoonliteCom, 0, sizeof(TYPE_MOONLITECOM));

//	cNiteCrawlerTabObjPtr	=	NULL;
//	cFocuserTabObjPtr		=	NULL;
//	cAuxTabObjPtr			=	NULL;
//	cConfigTabObjPtr		=	NULL;
//	cGraphTabObjPtr			=	NULL;

	//*	create the tab objects
	cFocuserType			=	focuserType;
}


//**************************************************************************************
void	ControllerFocus::CreateWindowTabs(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

}


//**************************************************************************************
void	ControllerFocus::RunBackgroundTasks(void)
{
uint32_t	currentMillis;
uint32_t	deltaSeconds;
bool		validData;
bool		needToUpdate;
uint32_t	updateDelta;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (cReadStartup)
	{
		if (cValidIPaddr)
		{
			AlpacaGetStartupData();
		}
		cReadStartup	=	false;
	}

	needToUpdate	=	false;
	currentMillis	=	millis();
	deltaSeconds	=	(currentMillis - cLastUpdate_milliSecs) / 1000;

	if (cFirstDataRead || (deltaSeconds > 5))
	{
//		CONSOLE_DEBUG(__FUNCTION__);
		needToUpdate	=	true;
	}
	if (cUSBportOpen)
	{
		//*	USB? only update once a second
		updateDelta	=	400;
//		updateDelta	=	5000;		//*	slow down for debugging
	}
	else
	{
		//*	if we are moving, then update 5 times a second
		updateDelta	=	200;
	}
	if (cIsMoving && ((currentMillis - cLastUpdate_milliSecs) > updateDelta))
	{
		needToUpdate	=	true;
	}
	if (needToUpdate)
	{
//		CONSOLE_DEBUG(__FUNCTION__);
		//*	is the IP address valid
		if (cValidIPaddr)
		{
			validData	=	AlpacaGetStatus();
		}
		else if (cUSBportOpen)
		{
			//*	the usb port is open, get data that way
			validData	=	USBGetStatus();
		}
		else
		{
			validData	=	false;
		}
		if (validData == false)
		{
			cIsMoving	=	false;
		}

		if (cFirstDataRead)
		{
			UpdateFromFirstRead();
		}
	}
}

//*****************************************************************************
void	ControllerFocus::UpdateFocuserPostion(const int newFocuserPostion)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");

	cFocuserPosition	=	newFocuserPostion;
}

//*****************************************************************************
void	ControllerFocus::UpdateRotatorPostion(const int newRotatorPostion)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	exit(0);
}

//*****************************************************************************
void	ControllerFocus::UpdateAuxMotorPostion(const int newAuxMotorPostion)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	exit(0);
}

//*****************************************************************************
void	ControllerFocus::UpdateTemperature(const double newTemperature)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	exit(0);
}

//*****************************************************************************
void	ControllerFocus::UpdateVoltage(const double newVoltage)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	exit(0);
}

//*****************************************************************************
void	ControllerFocus::UpdateStepsPerRev(const int newStepsPerRev)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	exit(0);
}

//*****************************************************************************
void	ControllerFocus::UpdateFromFirstRead(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

//	CONSOLE_DEBUG_W_NUM("cFocuserPosition\t=",		cFocuserPosition);
//	CONSOLE_DEBUG_W_NUM("cRotatorPosition\t=",		cRotatorPosition);
//	CONSOLE_DEBUG_W_NUM("cAuxMotorPosition\t=",		cAuxMotorPosition);

	cFocuserDesiredPos	=	cFocuserPosition;
	cRotatorDesiredPos	=	cRotatorPosition;
	cAuxMotorDesiredPos	=	cAuxMotorPosition;

//	CONSOLE_DEBUG_W_NUM("cFocuserDesiredPos\t=",	cFocuserDesiredPos);


	UpdateWindowTabs_Everything();

	cFirstDataRead		=	false;
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_Everything(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	exit(0);
	//*	This function should be overloaded
//-	SetWidgetNumber(kTab_Focuser,	kFocusTab_rotDesired,		cRotatorDesiredPos);
//-	SetWidgetNumber(kTab_Focuser,	kFocusTab_focDesired,		cFocuserDesiredPos);
//-	SetWidgetNumber(kTab_AuxMotor,	kAuxMotorBox_DesiredPos,	cAuxMotorDesiredPos);
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_SwitchBits(unsigned char switchBits)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	exit(0);

}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_AuxSwitchBits(unsigned char auxSwitchBits)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	exit(0);
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_Version(const char *versionString)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	exit(0);

}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_DesiredFocusPos(const int newDesiredPoistion)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	exit(0);

}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_DesiredRotatorPos(const int newDesiredPoistion)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	exit(0);
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_DesiredAuxPos(const int newDesiredPoistion)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	exit(0);
}


//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_ConnectState(bool connectedState)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
}

//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_SwitchState(int switchId, bool onOffState)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
}


//*****************************************************************************
bool	ControllerFocus::AlpacaGetStartupData(void)
{
bool			validData;

	validData	=	false;

	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions("focuser", cAlpacaDevNum);
	if (validData)
	{
		UpdateWindowTabs_ReadAll(cHasReadAll);
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}
	return(validData);
}


//*****************************************************************************
void	ControllerFocus::UpdateWindowTabs_ReadAll(bool hasReadAll)
{
	//*	This function should be overloaded
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "This function should be overloaded");
	exit(0);
}


//*****************************************************************************
void	ControllerFocus::AlpacaProcessReadAll(const char *deviceType, const char *keywordString, const char *valueString)
{
int			argValue;
double		argDouble;
bool		switchStatus;

	if (strcasecmp(keywordString, "ismoving") == 0)
	{
		cIsMoving	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "position") == 0)
	{
		argValue	=	atoi(valueString);
		UpdateFocuserPostion(argValue);
	}
	else if (strcasecmp(keywordString, "temperature") == 0)
	{
		argDouble	=	atof(valueString);
		UpdateTemperature(argDouble);
	}
	//===============================================================
	//*	the ones below here are not in the alpaca standard definition
	else if (strcasecmp(keywordString, "version") == 0)
	{
		//*	"version": "AlpacaPi - V0.2.2-beta build #32",
		strcpy(cAlpacaVersionString, valueString);
		UpdateWindowTabs_Version(cAlpacaVersionString);
	}
	else if (strcasecmp(keywordString, "Model") == 0)
	{
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
	}
	else if (strcasecmp(keywordString, "SerialNum") == 0)
	{
		strcpy(cSerialNumber, valueString);
	}
	else if (strcasecmp(keywordString, "RotatorPosition") == 0)
	{
		argValue	=	atoi(valueString);
		UpdateRotatorPostion(argValue);
	}
	else if (strcasecmp(keywordString, "AuxPosition") == 0)
	{
		argValue	=	atoi(valueString);
		UpdateAuxMotorPostion(argValue);
	}
	else if (strcasecmp(keywordString, "Voltage") == 0)
	{
		argDouble	=	atof(valueString);
		UpdateVoltage(argDouble);
	}
	else if (strcasecmp(keywordString, "RotatorIsMoving") == 0)
	{
		cIsMoving	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "AuxIsMoving") == 0)
	{
		cIsMoving	=	IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "SwitchOUT") == 0)
	{
		switchStatus	=	IsTrueFalse(valueString);
		UpdateWindowTabs_SwitchState(kSwitch_Out, switchStatus);
	}
	else if (strcasecmp(keywordString, "SwitchIN") == 0)
	{
		switchStatus	=	IsTrueFalse(valueString);
		UpdateWindowTabs_SwitchState(kSwitch_In, switchStatus);
	}
	else if (strcasecmp(keywordString, "SwitchROT") == 0)
	{
		switchStatus	=	IsTrueFalse(valueString);
		UpdateWindowTabs_SwitchState(kSwitch_Rot, switchStatus);
	}
}


//*****************************************************************************
bool	ControllerFocus::AlpacaGetStatus(void)
{
bool	validData;
char	lineBuff[128];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, cWindowName);
	cIsMoving	=	false;
	validData	=	false;
	if (cHasReadAll)
	{
		validData	=	AlpacaGetStatus_ReadAll("focuser", cAlpacaDevNum);
		sprintf(lineBuff, "%s-%s", cModelName, cSerialNumber);
//-		SetWidgetText(kTab_Focuser, kFocusTab_Model, lineBuff);

		SetWindowIPaddrInfo(NULL, true);

		if (cFocuserPosition != cFocuserDesiredPos)
		{
			cIsMoving	=	true;
		}
		if (cRotatorPosition != cRotatorDesiredPos)
		{
			cIsMoving	=	true;
		}
		cLastUpdate_milliSecs	=	millis();
	}
	else
	{
//		validData	=	AlpacaGetStatus_OneAAT();	//*	One At A Time
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

	cIsMoving	=	false;
	//=================================================================
	validUSBdata	=	MoonLite_GetPosition(	&cMoonliteCom,
												1,
												&newPositionValue);
	if (validUSBdata)
	{
		if (newPositionValue != cFocuserPosition)
		{
			cIsMoving	=	true;
		}
//		CONSOLE_DEBUG_W_NUM("newPositionValue\t=",		newPositionValue);
		UpdateFocuserPostion(newPositionValue);
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
				cIsMoving	=	true;
			}
			UpdateRotatorPostion(newPositionValue);
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
				cIsMoving	=	true;
			}
			UpdateAuxMotorPostion(newPositionValue);
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
	if (cIsMoving == false)
	{
//		CONSOLE_DEBUG("cIsMoving is false");
		//*	check the temperature every 10 seconds
		if (cFirstDataRead ||  ((currentSeconds - cLastTimeSecs_Temperature) > 5))
		{
//			CONSOLE_DEBUG("getting ready to check temperature");
			validUSBdata	=	MoonLite_GetTemperature(&cMoonliteCom, &newValeDbl);
			if (validUSBdata)
			{
				UpdateTemperature(newValeDbl);
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
		MoonLite_SetPostion(	&cMoonliteCom,
								1,
								cFocuserDesiredPos);
	}
	else
	{
		CONSOLE_DEBUG("We do not have a valid IP address to query");
	}

	cIsMoving	=	true;
}


//*****************************************************************************
void	ControllerFocus::SendMoveRotatorCommand(int newDesiredPosition)
{
char			dataString[128];
bool			validData;
double			newDegreesValue;

	cRotatorDesiredPos	=	newDesiredPosition;
	UpdateWindowTabs_DesiredRotatorPos(cRotatorDesiredPos);
	cIsMoving	=	true;

	if (cValidIPaddr)
	{
		if (cHasReadAll)
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
		MoonLite_SetPostion(	&cMoonliteCom,
								2,
								cRotatorDesiredPos);
	}
	else
	{
		cIsMoving	=	false;
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

	cIsMoving	=	true;
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
		MoonLite_SetPostion(	&cMoonliteCom,
								3,
								cAuxMotorDesiredPos);
	}
	else
	{
		cIsMoving	=	false;
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
	UpdateFromFirstRead();
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
			validUSBdata	=	MoonLite_SetCurrentPostion(	&cMoonliteCom,
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
		validUSBdata	=	MoonLite_SetCurrentPostion(	&cMoonliteCom,
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
	UpdateFromFirstRead();
}


//*****************************************************************************
void	ControllerFocus::ReadNiteCrawlerColors(void)
{
bool		validData;
int			iii;
uint16_t	colorValue16Bit;

	CONSOLE_DEBUG(__FUNCTION__);

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
					printf("Color %2d=\t%04X\tDef=%04X\t%s",	gNiteCrawlerColors[iii].colorNumber,
																gNiteCrawlerColors[iii].color16bit,
																gNiteCrawlerColors[iii].color16bitDefault,
																gNiteCrawlerColors[iii].name);

					if (gNiteCrawlerColors[iii].color16bit != gNiteCrawlerColors[iii].color16bitDefault)
					{
						printf("\t<--- error");
					}
					printf("\r\n");
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
		cUSBportOpen		=	true;
		cFirstDataRead		=	true;
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


#endif	//	_ENABLE_CTRL_FOCUSERS_
