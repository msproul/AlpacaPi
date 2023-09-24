//**************************************************************************
//*	Name:			filterwheeldriver_ATIK.cpp
//*
//*	Author:			Mark Sproul (C) 2021
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
//*	Redistributions of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*	Usage notes:	Remember to install the rules and reboot,
//*					see EFW_linux_mac_SDK/lib/README.txt
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Apr 26,	2021	<MLS> Created filterwheeldriver_ATIK.cpp
//*	Apr 26,	2021	<MLS> asl547 on Cloudy nights offered to loan me an ATIK filterwheel
//*	Apr 28,	2021	<MLS> Received ATIK EFW2 from asl547
//*	Apr 29,	2021	<MLS> ATIK filter wheel working
//*	Apr 30,	2021	<MLS> CONFORM-filterwheel/atik -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*	Apr 30,	2021	<MLS> SUPPORTED: ATIK filter wheel
//*****************************************************************************

#ifdef _ENABLE_FILTERWHEEL_ATIK_

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>

#include	"eventlogging.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"AtikCameras.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"filterwheeldriver.h"
#include	"filterwheeldriver_ATIK.h"


//**************************************************************************************
//*	this will get moved to the individual implentations later
void	CreateFilterWheelObjects_ATIK(void)
{
int		devNum;
int		atikDeviceCount;
bool	rulesOK;
char	driverVersionString[64];
int		atikVersionNum;
char	rulesFileName[]	=	"99-atik.rules";
bool	deviceIsEFW;

	CONSOLE_DEBUG(__FUNCTION__);

	atikVersionNum	=	ArtemisAPIVersion();
	sprintf(driverVersionString, "%d", atikVersionNum);
	LogEvent(	"filterwheel",
				"Library version (ATIK)",
				NULL,
				kASCOM_Err_Success,
				driverVersionString);

	AddLibraryVersion("camera", "ATIK", driverVersionString);
	CONSOLE_DEBUG_W_STR("ATIK driver version:", driverVersionString);

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

	atikDeviceCount	=	ArtemisDeviceCount();
	CONSOLE_DEBUG_W_NUM("atikDeviceCount\t=", atikDeviceCount);
	atikDeviceCount	=	4;
	for (devNum=0; devNum < atikDeviceCount; devNum++)
	{
	//	deviceIsEFW	=	ArtemisDeviceHasFilterWheel(devNum);
		deviceIsEFW	=	ArtemisEFWIsPresent(devNum);
		if (deviceIsEFW)
		{
			CONSOLE_DEBUG_W_NUM("devNum\t=", devNum);
			new FilterwheelATIK(devNum);
		}

	}
}


//**************************************************************************************
FilterwheelATIK::FilterwheelATIK(const int argAtikDeviceNumber)
	:FilterwheelDriver(argAtikDeviceNumber)
{
bool	rulesFileOK;
char	rulesFileName[]	=	"99-atik.rules";


	CONSOLE_DEBUG(__FUNCTION__);
	cFilterWheelDevNum	=	argAtikDeviceNumber;
	cNumberOfPositions	=	0;
	cForceReadPosition	=	true;
	cActualPosReadCout	=	0;
	cAtikEFWHandle		=	NULL;
	cArtimisErrorCnt	=	0;

	strcpy(cCommonProp.Name, "ATIK Filterwheel");	//*	put something there in case of failure to open
	strcpy(cCommonProp.Description, "ATIK filterwheel");
//-	strcpy(cDeviceVersion,	EFWGetSDKVersion());

	rulesFileOK	=	Check_udev_rulesFile(rulesFileName);
	if (rulesFileOK == false)
	{
		CONSOLE_DEBUG("ATIK Rules file does not appear to be installed properly");
	}
	ReadFilterWheelInfo();

	//*	on startup we need to know the current position
	Read_CurrentFilterPositon(NULL);	//*	we dont care about a return value
}

//**************************************************************************************
// Destructor
//**************************************************************************************
FilterwheelATIK::~FilterwheelATIK(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

}

//**************************************************************************************
//*	this opens the connection
bool	FilterwheelATIK::AlpacaConnect(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	return(false);
}

//**************************************************************************************
void	FilterwheelATIK::ReadFilterWheelInfo(void)
{
char			serialNumber[64]	=	"---";
ARTEMISEFWTYPE	efwType;
int				artimisRetCode;
int				numFilters	=	0;
int				moving		=	0;
int				currentPos	=	0;
int				targetPos	=	0;
bool			isMoving;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("cFilterWheelDevNum\t=",	cFilterWheelDevNum);

	efwType			=	(ARTEMISEFWTYPE)0;
	artimisRetCode	=	ArtemisEFWGetDeviceDetails(cFilterWheelDevNum, &efwType, serialNumber);
	if (artimisRetCode == ARTEMIS_OK)
	{
		CONSOLE_DEBUG_W_NUM("efwType\t\t=", efwType);
		CONSOLE_DEBUG_W_STR("serialNumber\t=", serialNumber);
	}
	else
	{
		CONSOLE_DEBUG("ArtemisEFWGetDeviceDetails returned ERROR");
		CONSOLE_DEBUG_W_NUM("artimisRetCode\t=",	artimisRetCode);
	}

	cAtikEFWHandle	=	ArtemisEFWConnect(cFilterWheelDevNum);
	if (cAtikEFWHandle != NULL)
	{
		cFilterWheelConnected		=	ArtemisEFWIsConnected(cAtikEFWHandle);
		CONSOLE_DEBUG_W_NUM("cFilterWheelConnected\t=",	cFilterWheelConnected);

		//-----------------------------------------------------------------------------------
		artimisRetCode	=	ArtemisEFWGetDetails( cAtikEFWHandle, &efwType, serialNumber);
		if (artimisRetCode == ARTEMIS_OK)
		{
			CONSOLE_DEBUG_W_NUM("efwType\t\t=", efwType);
			CONSOLE_DEBUG_W_STR("serialNumber\t=", serialNumber);
		}
		else
		{
			CONSOLE_DEBUG("ArtemisEFWGetDetails returned ERROR");
			CONSOLE_DEBUG_W_NUM("artimisRetCode\t=",	artimisRetCode);
			cArtimisErrorCnt++;
		}


		//-----------------------------------------------------------------------------------
		artimisRetCode	=	ArtemisFilterWheelInfo(	cAtikEFWHandle,
													&numFilters,
													&moving,
													&currentPos,
													&targetPos);

		if (artimisRetCode == ARTEMIS_OK)
		{
			CONSOLE_DEBUG_W_NUM("numFilters    \t=",	numFilters);
			CONSOLE_DEBUG_W_NUM("moving        \t=",	moving);
			CONSOLE_DEBUG_W_NUM("currentPos    \t=",	currentPos);
			CONSOLE_DEBUG_W_NUM("targetPos     \t=",	targetPos);
		}
		else
		{
			CONSOLE_DEBUG("ArtemisFilterWheelInfo returned ERROR");
			CONSOLE_DEBUG_W_NUM("artimisRetCode\t=",	artimisRetCode);
			cArtimisErrorCnt++;
		}

		//-----------------------------------------------------------------------------------
		artimisRetCode	=	ArtemisEFWNmrPosition(	cAtikEFWHandle, &numFilters);

		if (artimisRetCode == ARTEMIS_OK)
		{
			CONSOLE_DEBUG_W_NUM("numFilters    \t=",	numFilters);

			cNumberOfPositions	=	numFilters;
		}
		else
		{
			CONSOLE_DEBUG("ArtemisEFWNmrPosition returned ERROR");
			CONSOLE_DEBUG_W_NUM("artimisRetCode\t=",	artimisRetCode);
			cArtimisErrorCnt++;
		}

		//-----------------------------------------------------------------------------------
		artimisRetCode	=	ArtemisEFWGetPosition(	cAtikEFWHandle, &currentPos, &isMoving);

		if (artimisRetCode == ARTEMIS_OK)
		{
			CONSOLE_DEBUG_W_NUM("currentPos    \t=",	currentPos);
			CONSOLE_DEBUG_W_NUM("isMoving      \t=",	isMoving);

			cFilterWheelProp.Position	=	currentPos;
		}
		else
		{
			CONSOLE_DEBUG("ArtemisEFWGetPosition returned ERROR");
			CONSOLE_DEBUG_W_NUM("artimisRetCode\t=",	artimisRetCode);
			cArtimisErrorCnt++;
		}
		//	int ArtemisEFWSetPosition(ArtemisHandle handle, int   iPosition);
	}
	else
	{
		CONSOLE_DEBUG("Failed to connect!!!!!!!!!!!!");
	}
}

//*****************************************************************************
int	FilterwheelATIK::Read_CurrentFWstate(void)
{

	return(kFilterWheelState_OK);
}

//*****************************************************************************
//*	this returns a position starting with 0
//*	return -1 if unable to determine position
//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelATIK::Read_CurrentFilterPositon(int *rtnCurrentPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					artimisRetCode;
int					currentPos	=	0;
bool				isMoving;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cAtikEFWHandle != NULL)
	{
		//-----------------------------------------------------------------------------------
		artimisRetCode	=	ArtemisEFWGetPosition(	cAtikEFWHandle, &currentPos, &isMoving);

		if (artimisRetCode == ARTEMIS_OK)
		{
			CONSOLE_DEBUG_W_NUM("currentPos    \t=",	currentPos);
			CONSOLE_DEBUG_W_NUM("isMoving      \t=",	isMoving);
			if (isMoving)
			{
				cFilterWheelProp.Position	=	-1;
			}
			else
			{
				cFilterWheelProp.Position	=	currentPos;
			}
		}
		else
		{
			CONSOLE_DEBUG("ArtemisEFWGetPosition returned ERROR");
			CONSOLE_DEBUG_W_NUM("artimisRetCode\t=",	artimisRetCode);
			cArtimisErrorCnt++;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	//*	if the caller wants the value returned
	if (rtnCurrentPosition != NULL)
	{
		*rtnCurrentPosition	=	cFilterWheelProp.Position;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
//*	this accepts a position starting with 0
//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelATIK::Set_CurrentFilterPositon(const int newPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int					artimisRetCode;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (cAtikEFWHandle != NULL)
	{
		//-----------------------------------------------------------------------------------
		artimisRetCode	=	ArtemisEFWSetPosition(	cAtikEFWHandle, newPosition);

		if (artimisRetCode == ARTEMIS_OK)
		{

			cFilterWheelProp.Position	=	newPosition;
		}
		else
		{
			CONSOLE_DEBUG("ArtemisEFWGetPosition returned ERROR");
			CONSOLE_DEBUG_W_NUM("artimisRetCode\t=",	artimisRetCode);
			cArtimisErrorCnt++;
		}
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	return(alpacaErrCode);
}



#endif // _ENABLE_FILTERWHEEL_ATIK_
