//**************************************************************************
//*	Name:			filterwheeldriver_SIM.cpp
//*
//*	Author:			Mark Sproul (C) 2023
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
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Mar  1,	2023	<MLS> Created filterwheeldriver_sim.cpp
//*	Mar  3,	2023	<MLS> CONFORMU-filterwheel/simulator -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*****************************************************************************

#ifdef _ENABLE_FILTERWHEEL_SIMULATOR_

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>

#include	"eventlogging.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"filterwheeldriver.h"
#include	"filterwheeldriver_sim.h"


//**************************************************************************************
//*	this will get moved to the individual implentations later
void	CreateFilterWheelObjects_SIM(void)
{
	new FilterwheelSIM(0);
}


//**************************************************************************************
FilterwheelSIM::FilterwheelSIM(const int deviceNumber)
	:FilterwheelDriver(deviceNumber)
{
int		filterIndex;

	CONSOLE_DEBUG(__FUNCTION__);
	cFilterWheelDevNum		=	deviceNumber;
	cNumberOfPositions		=	9;
	cForceReadPosition		=	true;
	cActualPosReadCout		=	0;
	cFilterWheelIsOpen		=	true;
	cFilterWheelConnected	=	true;

	strcpy(cCommonProp.Name,		"AlpacaPi FilterWheel Simulator");	//*	put something there in case of failure to open
	strcpy(cCommonProp.Description, "AlpacaPi Simulator");
	ReadFilterWheelInfo();

//	DumpCommonProperties(__FUNCTION__);

	//*	this is a simulator, simulate the names
	filterIndex	=	0;
	strcpy(cFilterWheelProp.Names[filterIndex++].FilterName, "None");
	strcpy(cFilterWheelProp.Names[filterIndex++].FilterName, "Red");
	strcpy(cFilterWheelProp.Names[filterIndex++].FilterName, "Green");
	strcpy(cFilterWheelProp.Names[filterIndex++].FilterName, "Blue");
	strcpy(cFilterWheelProp.Names[filterIndex++].FilterName, "Sii");
	strcpy(cFilterWheelProp.Names[filterIndex++].FilterName, "HA");
	strcpy(cFilterWheelProp.Names[filterIndex++].FilterName, "Oiii");
	strcpy(cFilterWheelProp.Names[filterIndex++].FilterName, "IR");
	strcpy(cFilterWheelProp.Names[filterIndex++].FilterName, "Dark");
	CONSOLE_DEBUG_W_NUM("filterIndex\t=", filterIndex);
	if (filterIndex >= kMaxFiltersPerWheel)
	{
		CONSOLE_ABORT("To many filter names")
	}
//	for (filterIndex=0; filterIndex<kMaxFiltersPerWheel; filterIndex++)
//	{
//		CONSOLE_DEBUG_W_STR("FilterName\t=", cFilterWheelProp.Names[filterIndex].FilterName);
//	}
}

//**************************************************************************************
// Destructor
//**************************************************************************************
FilterwheelSIM::~FilterwheelSIM(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

}

//**************************************************************************************
//*	this opens the connection
//**************************************************************************************
bool	FilterwheelSIM::AlpacaConnect(void)
{
	CONSOLE_DEBUG(__FUNCTION__);

	return(true);
}

//**************************************************************************************
void	FilterwheelSIM::ReadFilterWheelInfo(void)
{

}

//*****************************************************************************
int	FilterwheelSIM::Read_CurrentFWstate(void)
{

	return(kFilterWheelState_OK);
}

//*****************************************************************************
//*	this returns a position starting with 0
//*	return -1 if unable to determine position
//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelSIM::Read_CurrentFilterPositon(int *rtnCurrentPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);

	return(alpacaErrCode);
}

//*****************************************************************************
//*	this accepts a position starting with 0
//*****************************************************************************
TYPE_ASCOM_STATUS	FilterwheelSIM::Set_CurrentFilterPositon(const int newPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);
	if (true)
	{
		cFilterWheelProp.Position	=	newPosition;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	return(alpacaErrCode);
}

#endif // _ENABLE_FILTERWHEEL_SIMULATOR_
