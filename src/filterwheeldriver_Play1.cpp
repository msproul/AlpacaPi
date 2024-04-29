//**************************************************************************
//*	Name:			filterwheeldriver_Play1.cpp
//*
//*	Author:			Mark Sproul (C) 2024
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
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 18,	2024	<MLS> Borrowed Player-One filter wheel from Vanessa Zhang
//*	Apr 18,	2024	<MLS> Created filterwheeldriver_Play1.cpp
//*	Apr 18,	2024	<MLS> PlayerOne filterwheel working
//*****************************************************************************

#ifdef _ENABLE_FILTERWHEEL_PLAYERONE_

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>

#include	"eventlogging.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"PlayerOnePW.h"

#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"filterwheeldriver.h"
#include	"filterwheeldriver_Play1.h"


//**************************************************************************************
//**************************************************************************************
void	CreateFilterWheelObjects_PlayerOne(void)
{
int		devNum;
int		payerOneFilterWheelCount;
bool	rulesOK;
char	driverVersionString[64];
int		playerOneAPIversionNum;
char	rulesFileName[]	=	"99-player_one_astronomy.rules";

	CONSOLE_DEBUG(__FUNCTION__);

	playerOneAPIversionNum		=	POAGetPWAPIVer();
	CONSOLE_DEBUG_W_NUM("playerOneAPIversionNum\t=", playerOneAPIversionNum);
	strcpy(driverVersionString, POAGetPWSDKVer());
	LogEvent(	"filterwheel",
				"Library version (PlayerOne)",
				NULL,
				kASCOM_Err_Success,
				driverVersionString);

	AddLibraryVersion("camera", "PlayerOne", driverVersionString);
	CONSOLE_DEBUG_W_STR("PlayerOne driver version:", driverVersionString);

	//*	check to make sure the rules file is present
	//	/lib/udev/rules.d/99-atik.rules
	rulesOK	=	Check_udev_rulesFile(rulesFileName);
	if (rulesOK != true)
	{
		CONSOLE_DEBUG("Problem with ATIK rules");
		LogEvent(	"filterwheel",
					"Problem with ATIK rules",
					NULL,
					kASCOM_Err_Success,
					rulesFileName);
	}

	payerOneFilterWheelCount	=	POAGetPWCount();
	CONSOLE_DEBUG_W_NUM("payerOneFilterWheelCount\t=", payerOneFilterWheelCount);
	for (devNum=0; devNum < payerOneFilterWheelCount; devNum++)
	{
		new FilterwheelPlayerOne(devNum);
	}
}


//**************************************************************************************
FilterwheelPlayerOne::FilterwheelPlayerOne(const int argPlayerDeviceNumber)
	:FilterwheelDriver(argPlayerDeviceNumber)
{
bool	rulesFileOK;
char	rulesFileName[]	=	"99-player_one_astronomy.rules";
int		playerOneAPIversionNum;

	CONSOLE_DEBUG(__FUNCTION__);
	cFilterWheelDevNum	=	argPlayerDeviceNumber;
	cNumberOfPositions	=	0;
	cForceReadPosition	=	true;
	cActualPosReadCout	=	0;
	cPWindex			=	argPlayerDeviceNumber;

	strcpy(cDeviceManufacturer,		"PlayerOne");
//	strcpy(cDeviceManufAbrev,		"PlayerOne");
//	strcpy(cDeviceVersion,			"PlayerOne");
	playerOneAPIversionNum		=	POAGetPWAPIVer();
	sprintf(cDeviceFirmwareVersStr,	"API Version#%d", playerOneAPIversionNum);

	strcpy(cCommonProp.Name, "PlayerOne Filterwheel");	//*	put something there in case of failure to open
	strcpy(cCommonProp.Description, "PlayerOne filterwheel");

	rulesFileOK	=	Check_udev_rulesFile(rulesFileName);
	if (rulesFileOK == false)
	{
		CONSOLE_DEBUG("PlayerOne Rules file does not appear to be installed properly");
	}
	ReadFilterWheelInfo();

	//*	on startup we need to know the current position
	Read_CurrentFilterPositon(NULL);	//*	we dont care about a return value
}

//**************************************************************************************
// Destructor
//**************************************************************************************
FilterwheelPlayerOne::~FilterwheelPlayerOne(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

}

//**************************************************************************************
//*	this opens the connection
bool	FilterwheelPlayerOne::AlpacaConnect(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	return(false);
}

//**************************************************************************************
void	FilterwheelPlayerOne::ReadFilterWheelInfo(void)
{
PWErrors	pwError;

	CONSOLE_DEBUG_W_NUM("cPWindex     \t=", cPWindex);
	pwError	=	POAGetPWProperties(cPWindex, &cPWProperties);
	CONSOLE_DEBUG_W_NUM("pwError      \t=", pwError);
	if (pwError == PW_OK)
	{
		CONSOLE_DEBUG_W_STR("Name         \t=", cPWProperties.Name);
		CONSOLE_DEBUG_W_NUM("PositionCount\t=", cPWProperties.PositionCount);
		CONSOLE_DEBUG_W_STR("Serial Number\t=", cPWProperties.SN);

		//*	save the info where it belongs
		cNumberOfPositions		=	cPWProperties.PositionCount;
		strcpy(cDeviceModel,		cPWProperties.Name);
		strcpy(cCommonProp.Name,	cPWProperties.Name);
		strcpy(cDeviceSerialNum,	cPWProperties.SN);
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("POAGetPWProperties returned ", pwError);
		CONSOLE_DEBUG(POAGetPWErrorString(pwError));
		CONSOLE_DEBUG_W_NUM("POAGetPWProperties() returned error\t=", pwError);
	}
	pwError	=	POAOpenPW(cPWProperties.Handle);
	if (pwError == PW_OK)
	{
		CONSOLE_DEBUG("POAOpenPW() worked");
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("POAGetPWState returned ", pwError);
		CONSOLE_DEBUG(POAGetPWErrorString(pwError));
	}

//	//-------------------------------------------
//	//*	get the various names
//char		customName[64];
//char		aliasName[64];
//	pwError	=   POAGetPWCustomName(cPWProperties.Handle, customName, sizeof(customName));
//	CONSOLE_DEBUG_W_STR("customName\t=",	customName);
//	for (iii=0; iii< cNumberOfPositions; iii++)
//	{
//		pwError	=   POAGetPWFilterAlias(cPWProperties.Handle, iii, aliasName, sizeof(aliasName));
//		CONSOLE_DEBUG_W_STR("aliasName \t=",	aliasName);
//	}
}

//*****************************************************************************
int	FilterwheelPlayerOne::Read_CurrentFWstate(void)
{

	return(kFilterWheelState_OK);
}

//*****************************************************************************
//*	this returns a position starting with 0
//*	return -1 if unable to determine position
//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelPlayerOne::Read_CurrentFilterPositon(int *rtnCurrentPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					currentPos	=	0;
bool				isMoving;
PWErrors			pwError;

//	CONSOLE_DEBUG(__FUNCTION__);

	isMoving	=	false;
	pwError		=	POAGetPWState(cPWProperties.Handle, &cPWstate);
	if (pwError == PW_OK)
	{
		switch(cPWstate)
		{
			case PW_STATE_CLOSED:		///< PW was closed
//				CONSOLE_DEBUG("State is closed");
				pwError	=	POAOpenPW(cPWProperties.Handle);
				if (pwError == PW_OK)
				{
					CONSOLE_DEBUG("POAOpenPW() worked");
				}
				else
				{
					CONSOLE_DEBUG_W_NUM("POAGetPWState returned ", pwError);
					CONSOLE_DEBUG(POAGetPWErrorString(pwError));
				}
				break;

			case PW_STATE_OPENED:		///< PW was opened, but not moving(Idle)
//				CONSOLE_DEBUG("State is open");
				break;

			case PW_STATE_MOVING:		///< PW is moving
				CONSOLE_DEBUG("PW_STATE_MOVING");
				isMoving		=	true;
				break;

		}
		//*	alpaca specs say to do it this way, return -1 when moving
		if (isMoving)
		{
			cFilterWheelProp.Position	=	-1;
			alpacaErrCode	=	kASCOM_Err_Success;
		}
		else
		{
			currentPos	=	0;
			pwError	=	POAGetCurrentPosition(cPWProperties.Handle, &currentPos);
			if (pwError == PW_OK)
			{
				cFilterWheelProp.Position	=	currentPos;
				alpacaErrCode	=	kASCOM_Err_Success;
			}
		}
		//*	if the caller wants the value returned
		if (rtnCurrentPosition != NULL)
		{
			*rtnCurrentPosition	=	cFilterWheelProp.Position;
		}
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("POAGetPWState returned ", pwError);
		CONSOLE_DEBUG(POAGetPWErrorString(pwError));
		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	this accepts a position starting with 0
//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelPlayerOne::Set_CurrentFilterPositon(const int newPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
PWErrors			pwError;

//	CONSOLE_DEBUG(__FUNCTION__);
	pwError	=	POAGotoPosition(cPWProperties.Handle, newPosition);
	if (pwError == PW_OK)
	{
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("POAGotoPosition returned ", pwError);
		CONSOLE_DEBUG(POAGetPWErrorString(pwError));
		alpacaErrCode	=	kASCOM_Err_UnspecifiedError;
	}
	return(alpacaErrCode);
}
#endif // _ENABLE_FILTERWHEEL_PLAYERONE_

#ifdef _INCLUDE_PLAYERONE_MAIN_
//*****************************************************************************
int main(int argc, char *argv[])
{
	int PW_count = POAGetPWCount();

	printf("PW count: %d\n", PW_count);
	if(PW_count <= 0)
	{
		printf("there is no Player One filter wheel! \n");

		getchar();

		return 0;
	}

	//malloc pointer memory first
	PWProperties **ppPOA_PW_Prop = (PWProperties **)malloc(sizeof(PWProperties *) * PW_count);

	int i;
	for(i = 0; i < PW_count; i++)
	{
		ppPOA_PW_Prop[i] = (PWProperties *)malloc(sizeof (PWProperties)); //malloc the PWProperties memory

		PWErrors error = POAGetPWProperties(i, ppPOA_PW_Prop[i]);

		if(error == PW_OK)
		{
			//print PW Handle and Name
			printf("PW Handle: %d, PW name: %s \n", ppPOA_PW_Prop[i]->Handle, ppPOA_PW_Prop[i]->Name);
			//print PW SN and PW position count
			printf("PW SN: %s \n PW Position Count: %d \n", ppPOA_PW_Prop[i]->SN, ppPOA_PW_Prop[i]->PositionCount);
		}
		else
		{
			free(ppPOA_PW_Prop[i]);
			ppPOA_PW_Prop[i] = NULL;
			printf("get PW properties failed, index: %d, error code: %s \n", i, POAGetPWErrorString(error));
		}
	}

	//operate the first PW

	int handle, position_count, goto_position;

	handle = ppPOA_PW_Prop[0]->Handle;

	position_count = ppPOA_PW_Prop[0]->PositionCount;

	// open PW
	PWErrors error;
	error = POAOpenPW(handle);

	if(error != PW_OK)
	{
		printf("Open PW Failed, error code: %s \n", POAGetPWErrorString(error));

		//free the memory
		for(i = 0; i < PW_count; i++)
		{
			if(ppPOA_PW_Prop[i])
			{
				free(ppPOA_PW_Prop[i]);
				ppPOA_PW_Prop[i] = NULL;
			}
		}

		free(ppPOA_PW_Prop);
		ppPOA_PW_Prop = NULL;

		return 0;
	}

	//NOTE: When opened successfully, phoenix filter wheel will goto 1st position,menas: index == 0;

	//Detect PW is moving
	//You can ignore this step in your APP, in this demo, just waiting for PW stop moving
	PWState pw_state = PW_STATE_OPENED;
	do
	{
		POAGetPWState(handle, &pw_state);
	}while(pw_state == PW_STATE_MOVING);

	//get the current position
	goto_position = 0;
	error = POAGetCurrentPosition(handle, &goto_position);
	printf("get PW current position, pos: %d, error code: %s \n \n", goto_position, POAGetPWErrorString(error));

	printf("Goto Poistion Testing: \n");

	do
	{
		printf("--> Please input a number in the range[0 - %d], if the number < 0, this Goto Poistion Testing will quit: \n", position_count-1);
		scanf("%d", &goto_position);

		if(goto_position < 0)
		{
			printf("goto poistion testing quit.... \n");
			break;
		}

		if(goto_position >= position_count)
		{
			printf("The number you inputed is out of range.... \n");
			continue;
		}

		error = POAGotoPosition(handle, goto_position); // goto position

		printf("got position: %d, error string: %s \n", goto_position, POAGetPWErrorString(error));

		if(error == PW_OK) //waiting for PW stop moving
		{
			PWState pw_state = PW_STATE_OPENED;

			do
			{
				POAGetPWState(handle, &pw_state);
			}while(pw_state == PW_STATE_MOVING);
		}

	}while(1);


	//Close PW
	POAClosePW(handle);


	//free the memory
	for(i = 0; i < PW_count; i++)
	{
		if(ppPOA_PW_Prop[i])
		{
			free(ppPOA_PW_Prop[i]);
			ppPOA_PW_Prop[i] = NULL;
		}
	}

	free(ppPOA_PW_Prop);
	ppPOA_PW_Prop = NULL;

	return 0;
}
#endif // _INCLUDE_PLAYERONE_MAIN_


