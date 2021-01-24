//**************************************************************************
//*	Name:			rotatordriver_nc.cpp
//*
//*	Author:			Mark Sproul (C) 2019, 2020
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
//*	Dec 14,	2019	<MLS> Started on NiteCrawler rotator driver
//*****************************************************************************

#ifdef _ENABLE_ROTATOR_

#include	<string.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"JsonResponse.h"

#include	"alpacadriver.h"
#include	"focuserdriver.h"
#include	"rotatordriver.h"
#include	"rotatordriver_nc.h"


//**************************************************************************************
RotatorDriver_NiteCrawler::RotatorDriver_NiteCrawler(const int argDevNum, FocuserNiteCrawler *focuserObject)
	:RotatorDriver(argDevNum)
{

	CONSOLE_DEBUG(__FUNCTION__);

	//*	save the object pointer to the focuser
	cFocuserObject		=	focuserObject;

	strcpy(cDeviceName, "NiteCrawler Rotator");
	strcpy(cRotatorManufacturer,	"Moonlite");

	cRotatorCanReverse	=	false;

	if (cFocuserObject != NULL)
	{
		cFocuserObject->GetFocuserModel(cRotatorModel);
		cFocuserObject->GetFocuserSerialNumber(cRotatorSerialNum);

		cRotatorStepsPerRev	=	cFocuserObject->GetRotatorStepsPerRev();
		if (cRotatorStepsPerRev> 0)
		{
			cRotatorStepSize	=	360.0 / cRotatorStepsPerRev;
		}
	}
}

//**************************************************************************************
// Destructor
//**************************************************************************************
RotatorDriver_NiteCrawler::~RotatorDriver_NiteCrawler(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
int32_t	RotatorDriver_NiteCrawler::ReadCurrentPoisiton_steps(void)
{
int32_t		myRotatorPosition;

//	CONSOLE_DEBUG(__FUNCTION__);

	myRotatorPosition	=	0;
	if (cFocuserObject != NULL)
	{
		myRotatorPosition	=	cFocuserObject->GetRotatorPosition();
	}
	else
	{
		CONSOLE_DEBUG("No attached focuser");
	}

	return(myRotatorPosition);
}

//*****************************************************************************
double	RotatorDriver_NiteCrawler::ReadCurrentPoisiton_degs(void)
{
int		myRotatorPosition;
double	myPosDegrees;

//	CONSOLE_DEBUG(__FUNCTION__);
	myPosDegrees	=	0;

	if (cFocuserObject != NULL)
	{
		myRotatorPosition	=	cFocuserObject->GetRotatorPosition();
		if (cRotatorStepsPerRev > 0)
		{
			myPosDegrees		=	(360.0 * myRotatorPosition) / cRotatorStepsPerRev;
		}
		else
		{
			myPosDegrees		=	(360.0 * myRotatorPosition) / kStepsPerRev_WR30;
		}
	}
	else
	{
		CONSOLE_DEBUG("No attached focuser");
	}

	return(myPosDegrees);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver_NiteCrawler::SetCurrentPoisiton_steps(const int32_t newPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_INT32("newPosition\t=", newPosition);
	if (cFocuserObject != NULL)
	{
		alpacaErrCode		=	cFocuserObject->SetStepperPosition(2, newPosition);
		cRotatorIsMoving	=	true;
	}
	else
	{
		CONSOLE_DEBUG("No attached focuser");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver_NiteCrawler::SetCurrentPoisiton_degs(const double newPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int32_t				newPosition_Steps;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_DBL("newPosition\t\t=", newPosition);
	CONSOLE_DEBUG_W_INT32("cRotatorStepsPerRev\t=", cRotatorStepsPerRev);
	//*	we have to calculate the new position based on steps
	if (cRotatorStepsPerRev > 0)
	{
		newPosition_Steps	=	(newPosition / 360.0) * cRotatorStepsPerRev;
		CONSOLE_DEBUG_W_INT32("newPosition_Steps\t=", newPosition_Steps);
		alpacaErrCode		=	SetCurrentPoisiton_steps(newPosition_Steps);
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver_NiteCrawler::HaltMovement(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cFocuserObject != NULL)
	{
		alpacaErrCode	=	cFocuserObject->HaltStepper(2);
	}
	else
	{
		CONSOLE_DEBUG("No attached focuser");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
	return(alpacaErrCode);
}

//*****************************************************************************
bool	RotatorDriver_NiteCrawler::IsRotatorMoving(void)
{
bool	isMoving;

	CONSOLE_DEBUG(__FUNCTION__);
	if (cFocuserObject != NULL)
	{
		isMoving	=	cFocuserObject->GetRotatorIsMoving();
	}
	else
	{
		CONSOLE_DEBUG("No attached focuser");
		isMoving	=	false;
	}
	return(isMoving);
}


#endif // _ENABLE_ROTATOR_
