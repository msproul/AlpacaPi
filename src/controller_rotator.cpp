//*****************************************************************************
//*		controller_rotator.cpp		(c) 2022 by Mark Sproul
//*
//*
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
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Oct  7,	2022	<MLS> Created controller_rotator.cpp
//*****************************************************************************


#ifdef _ENABLE_CTRL_ROTATOR_


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"




#include	"alpaca_defs.h"
#include	"alpacadriver_helper.h"
#include	"windowtab_rotator.h"
#include	"windowtab_capabilities.h"
#include	"windowtab_drvrInfo.h"
#include	"windowtab_about.h"

#include	"sendrequest_lib.h"
#include	"helper_functions.h"

#include	"controller.h"
#include	"controller_rotator.h"

#define	kWindowWidth	450
#define	kWindowHeight	725


//**************************************************************************************
enum
{
	kTab_Rotator	=	1,
	kTab_Capabilities,
	kTab_DriverInfo,
	kTab_About,

	kTab_Count

};

//**************************************************************************************
ControllerRotator::ControllerRotator(	const char			*argWindowName,
										struct sockaddr_in	*deviceAddress,
										const int			port,
										const int			deviceNum)

	:Controller(argWindowName, kWindowWidth,  kWindowHeight)
{

	cAlpacaDevNum			=	deviceNum;
	cFirstDataRead			=	true;

	memset((void *)&cRotatorProp, 0, sizeof(TYPE_RotatorProperties));

	cLastUpdate_milliSecs	=	millis();


	strcpy(cAlpacaDeviceTypeStr,	"rotator");

	if (deviceAddress != NULL)
	{
		cDeviceAddress	=	*deviceAddress;
		cPort			=	port;
		cValidIPaddr	=	true;
	}
	SetupWindowControls();

	//*	display the IPaddres/port
	if (cValidIPaddr)
	{
	char	lineBuff[128];
	char	ipString[32];

		PrintIPaddressToString(cDeviceAddress.sin_addr.s_addr, ipString);
		sprintf(lineBuff, "%s:%d/%d", ipString, cPort, cAlpacaDevNum);
		SetWindowIPaddrInfo(lineBuff, true);

		CheckConnectedState();		//*	check connected and connect if not already connected
	}

#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_

}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerRotator::~ControllerRotator(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	DELETE_OBJ_IF_VALID(cRotatorTabObjPtr);
	DELETE_OBJ_IF_VALID(cCapabilitiesTabObjPtr);
	DELETE_OBJ_IF_VALID(cDriverInfoTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
}

//**************************************************************************************
void	ControllerRotator::SetupWindowControls(void)
{

	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_Count);
	SetTabText(kTab_Rotator,		"Rotator");
	SetTabText(kTab_Capabilities,	"Capabilities");
	SetTabText(kTab_DriverInfo,		"Driver Info");
	SetTabText(kTab_About,			"About");


	cRotatorTabObjPtr	=	new WindowTabRotator(cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cRotatorTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Rotator,	cRotatorTabObjPtr);
		cRotatorTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cCapabilitiesTabObjPtr		=	new WindowTabCapabilities(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cCapabilitiesTabObjPtr != NULL)
	{
		SetTabWindow(kTab_Capabilities,	cCapabilitiesTabObjPtr);
		cCapabilitiesTabObjPtr->SetParentObjectPtr(this);
	}

	//--------------------------------------------
	cDriverInfoTabObjPtr		=	new WindowTabDriverInfo(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cDriverInfoTabObjPtr != NULL)
	{
		SetTabWindow(kTab_DriverInfo,	cDriverInfoTabObjPtr);
		cDriverInfoTabObjPtr->SetParentObjectPtr(this);
	}

	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, cWindowName);
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
	}
}

//**************************************************************************************
void	ControllerRotator::RunBackgroundTasks(const char *callingFunction, bool enableDebug)
{
uint32_t	currentMillis;
uint32_t	deltaSeconds;
bool		validData;
bool		needToUpdate;

	if (cReadStartup)
	{
		CONSOLE_DEBUG_W_STR("cAlpacaDeviceTypeStr\t=", cAlpacaDeviceTypeStr);
		AlpacaGetStartupData();
		AlpacaGetCommonProperties_OneAAT(cAlpacaDeviceTypeStr);
		UpdateAboutBoxRemoteDevice(kTab_About, kAboutBox_CPUinfo);

		cReadStartup	=	false;
	}


	needToUpdate	=	false;
	currentMillis	=	millis();
	deltaSeconds	=	(currentMillis - cLastUpdate_milliSecs) / 1000;

	if (cFirstDataRead || (deltaSeconds >= 5))
	{
		needToUpdate	=	true;
	}
	if (cRotatorProp.IsMoving && (deltaSeconds >= 1))
	{
		needToUpdate	=	true;
	}
	if (cForceAlpacaUpdate)
	{
		needToUpdate		=	true;
		cForceAlpacaUpdate	=	false;
	}

	if (needToUpdate)
	{
		//*	is the IP address valid
		if (cValidIPaddr)
		{
			validData	=	AlpacaGetStatus();
			if (validData == false)
			{
				CONSOLE_DEBUG("Failed to get data");
			}

			UpdateConnectedIndicator(kTab_Rotator,		kRotatorCtrl_Connected);
		}
	}
}

//*****************************************************************************
void	ControllerRotator::AlpacaProcessSupportedActions(const char	*deviceTypeStr,
														const int	deviveNum,
														const char	*valueString)
{

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
//*	Get data One At A Time
//*****************************************************************************
bool	ControllerRotator::AlpacaGetStartupData_OneAAT(void)
{
//SJP_Parser_t	jsonParser;
bool			validData;
char			alpacaString[128];
//char			dataString[128];
//int				jjj;
//double			myDoubleValue;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("cAlpacaDeviceTypeStr\t=", cAlpacaDeviceTypeStr);


	validData	=	AlpacaGetStringValue(cAlpacaDeviceTypeStr, "description",	NULL,	alpacaString);
	if (validData)
	{
		CONSOLE_DEBUG_W_STR("description\t=", alpacaString);
		strcpy(cAlpacaVersionString, alpacaString);
	}

	ReadOneDriverCapability("rotator", "canreverse", "CanReverse", &cRotatorProp.CanReverse);


	return(validData);
}

//*****************************************************************************
bool	ControllerRotator::AlpacaGetStartupData(void)
{
bool			validData;
//int				jjj;

	CONSOLE_DEBUG(__FUNCTION__);
	//===============================================================
	//*	get supportedactions
	validData	=	AlpacaGetSupportedActions(cAlpacaDeviceTypeStr, cAlpacaDevNum);

	if (validData)
	{
		SetWidgetValid(kTab_Rotator,	kRotatorCtrl_Readall,		cHas_readall);
	}
	else
	{
		CONSOLE_DEBUG("Read failure - supportedactions");
		cReadFailureCnt++;
	}

	if (cHas_readall)
	{
		//*	use readall to get the startup data
		validData	=	AlpacaGetStatus_ReadAll(cAlpacaDeviceTypeStr, cAlpacaDevNum);
	}
//-	else
	{
		validData	=	AlpacaGetStartupData_OneAAT();
	}

	cLastUpdate_milliSecs	=	millis();

	return(validData);
}

//*****************************************************************************
//*	Get Status, One At A Time
//*****************************************************************************
bool	ControllerRotator::AlpacaGetStatus_OneAAT(void)
{
bool			validData;
int				myFailureCount;
double			argDouble;
bool			returnValueIsValid;
bool			argBoolean;

//	CONSOLE_DEBUG(__FUNCTION__);

	myFailureCount	=	0;
	//	bool	CanReverse;			//*	Indicates whether the Rotator supports the Reverse method.

	//----------------------------------------------------------------------
	//	bool	IsMoving;			//*	Indicates whether the rotator is currently moving
	validData	=	AlpacaGetBooleanValue(	cAlpacaDeviceTypeStr,
											"ismoving",
											NULL,
											&argBoolean,
											&returnValueIsValid,
											false);
	if (validData && returnValueIsValid)
	{
//		CONSOLE_DEBUG_W_BOOL("'IsMoving' property is", argBoolean);
		cRotatorProp.IsMoving	=	argBoolean;
	}
	else
	{
		CONSOLE_DEBUG("failed to get 'IsMoving' property");
//		CONSOLE_DEBUG_W_BOOL("validData\t\t=",			validData);
//		CONSOLE_DEBUG_W_BOOL("returnValueIsValid\t=",	returnValueIsValid);
		myFailureCount++;
	}

	//----------------------------------------------------------------------
	//	double	MechanicalPosition;	//*	This returns the raw mechanical position of the rotator in degrees.
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"mechanicalposition",		NULL,	&argDouble, &returnValueIsValid);
	if (validData && returnValueIsValid)
	{
		cRotatorProp.MechanicalPosition	=	argDouble;
	}
	else
	{
	//	CONSOLE_DEBUG("failed to get 'MechanicalPosition' property");
		myFailureCount++;
	}

	//----------------------------------------------------------------------
	//	double	Position;			//*	Current instantaneous Rotator position, allowing for any sync offset, in degrees.
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"position",		NULL,	&argDouble, &returnValueIsValid);
	if (validData && returnValueIsValid)
	{
		cRotatorProp.Position	=	argDouble;
	}
	else
	{
		CONSOLE_DEBUG("failed to get 'Position' property");
		myFailureCount++;
	}


	//----------------------------------------------------------------------
	//	bool	Reverse;			//*	Sets or Returns the rotator’s Reverse state.

	//----------------------------------------------------------------------
	//	double	StepSize;			//*	The minimum StepSize, in degrees.
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"stepsize",		NULL,	&argDouble, &returnValueIsValid);
	if (validData && returnValueIsValid)
	{
		cRotatorProp.StepSize	=	argDouble;
	}
	else
	{
		CONSOLE_DEBUG("failed to get 'StepSize' property");
		myFailureCount++;
	}

	//----------------------------------------------------------------------
	//	double	TargetPosition;		//*	The destination position angle for Move() and MoveAbsolute().
	validData	=	AlpacaGetDoubleValue(cAlpacaDeviceTypeStr,	"targetposition",		NULL,	&argDouble, &returnValueIsValid);
	if (validData && returnValueIsValid)
	{
		cRotatorProp.TargetPosition	=	argDouble;
	}
	else
	{
		CONSOLE_DEBUG("failed to get 'StepSize' property");
		myFailureCount++;
	}

	//*	all done, how many errors
	if (myFailureCount < 2)
	{
		validData	=	true;
	}
	else
	{
		validData	=	false;
	}
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "exit");
	return(validData);
}

//*****************************************************************************
bool	ControllerRotator::AlpacaGetStatus(void)
{
bool	validData;
bool	previousOnLineState;

//	CONSOLE_DEBUG(__FUNCTION__);

	previousOnLineState	=	cOnLine;
	cHas_readall	=	false;
	if (cHas_readall)
	{
		validData	=	AlpacaGetStatus_ReadAll(cAlpacaDeviceTypeStr, cAlpacaDevNum);
	}
	else
	{
		validData	=	AlpacaGetStatus_OneAAT();
		AlpacaGetCommonConnectedState(cAlpacaDeviceTypeStr);
	}

	if (validData)
	{
		if (cOnLine == false)
		{
			//*	if we were previously off line, force reading startup again
			cReadStartup	=	true;
		}

		cOnLine	=	true;
		if (cRotatorTabObjPtr != NULL)
		{
			cRotatorTabObjPtr->UpdateProperties_Rotator(&cRotatorProp);
		}

	}
	else
	{
		cOnLine	=	false;
	}

	if (cOnLine != previousOnLineState)
	{
		SetWindowIPaddrInfo(NULL, cOnLine);
	}

	cLastUpdate_milliSecs	=	millis();
	cFirstDataRead			=	false;
	return(validData);
}

//*****************************************************************************
void	ControllerRotator::AlpacaProcessReadAll(	const char	*deviceTypeStr,
													const int	deviceNum,
													const char	*keywordString,
													const char	*valueString)
{

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR(keywordString, valueString);
	if (strcasecmp(keywordString, "connected") == 0)
	{
		cCommonProp.Connected	=  IsTrueFalse(valueString);
	}
	else if (strcasecmp(keywordString, "version") == 0)
	{
		//*	"version": "AlpacaPi - V0.2.2-beta build #32",
		strcpy(cAlpacaVersionString, valueString);
	}
	else
	{
		AlpacaProcessReadAll_Common(	deviceTypeStr,
										deviceNum,
										keywordString,
										valueString);
	}
}

//*****************************************************************************
void	ControllerRotator::UpdateCommonProperties(void)
{
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Name,				cCommonProp.Name);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_Description,		cCommonProp.Description);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverInfo,			cCommonProp.DriverInfo);
	SetWidgetText(kTab_DriverInfo,		kDriverInfo_DriverVersion,		cCommonProp.DriverVersion);
	SetWidgetNumber(kTab_DriverInfo,	kDriverInfo_InterfaceVersion,	cCommonProp.InterfaceVersion);

}

//**************************************************************************************
void	ControllerRotator::UpdateCapabilityList(void)
{
	UpdateCapabilityListID(kTab_Capabilities, kCapabilities_TextBox1, kCapabilities_TextBoxN);
}

//*****************************************************************************
void	ControllerRotator::SendMoveAbsoluteCommand(double newPosition_Deg)
{
char			dataString[128];
bool			validData;
double			newDesiredPosition;

	cRotatorProp.TargetPosition	=	newPosition_Deg;

	if (cValidIPaddr)
	{
		newDesiredPosition	=	newPosition_Deg;
		if (newDesiredPosition < 0.0)
		{
			newDesiredPosition	+=	360.0;
		}
		if (newDesiredPosition >= 360.0)
		{
			newDesiredPosition	-=	360.0;
		}

		sprintf(dataString, "Position=%1.5f",		newDesiredPosition);
		CONSOLE_DEBUG_W_DBL("newDesiredPosition\t=",	newDesiredPosition);
		CONSOLE_DEBUG_W_STR("dataString\t=",		dataString);

		validData	=	AlpacaSendPutCmd(	"rotator",	"moveabsolute",		dataString);
		if (validData == false)
		{
			CONSOLE_DEBUG("AlpacaSendPutCmd(moveabsolute) failed!!!");
		}
		cForceAlpacaUpdate	=	true;
	}
	else
	{
		CONSOLE_DEBUG("We do not have a valid IP address to query");
	}
}

//*****************************************************************************
void	ControllerRotator::SendMoveRelativeCommand(double positionChange_Deg)
{
char			dataString[128];
bool			validData;

//	cRotatorProp.TargetPosition	=	newDesiredPosition;

	if (cValidIPaddr)
	{
		sprintf(dataString, "Position=%1.5f",			positionChange_Deg);
		CONSOLE_DEBUG_W_DBL("positionChange_Deg\t=",	positionChange_Deg);
		CONSOLE_DEBUG_W_STR("dataString\t=",			dataString);

		validData	=	AlpacaSendPutCmd(	"rotator",	"move",		dataString);
		if (validData == false)
		{
			CONSOLE_DEBUG("AlpacaSendPutCmd(move) failed!!!");
		}

		cForceAlpacaUpdate	=	true;
	}
	else
	{
		CONSOLE_DEBUG("We do not have a valid IP address to query");
	}
}

//*****************************************************************************
void	ControllerRotator::SendHaltCommand(void)
{
bool			validData;

	validData	=	AlpacaSendPutCmd(	"rotator",	"halt",		"");
	if (validData == false)
	{
		CONSOLE_DEBUG("AlpacaSendPutCmd(halt) failed!!!");
	}
}
#endif // _ENABLE_CTRL_ROTATOR_
