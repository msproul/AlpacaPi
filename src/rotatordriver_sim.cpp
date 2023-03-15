//**************************************************************************
//*	Name:			rotatordriver_sim.cpp
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
//*	Mar  1,	2023	<MLS> Created rotatordriver_sim.cpp
//*	Mar  2,	2023	<MLS> CONFORMU-rotatordriver_sim -> PASSED!!!!!!!!!!!!!!!!!!!!!
//*****************************************************************************

#ifdef _ENABLE_ROTATOR_SIMULATOR_

#include	<string.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"JsonResponse.h"

#include	"alpacadriver.h"
#include	"rotatordriver_sim.h"

//**************************************************************************************
void	CreateRotatorObjects_SIM(void)
{
	new	RotatorDriver_Sim(0);
}

//**************************************************************************************
RotatorDriver_Sim::RotatorDriver_Sim(const int argDevNum)
	:RotatorDriver(argDevNum)
{

	CONSOLE_DEBUG(__FUNCTION__);

	//*	save the object pointer to the focuser

	strcpy(cCommonProp.Name,		"AlpacaPi Rotator Simulator");
	strcpy(cCommonProp.Description,	"Rotator Simulator");
	strcpy(cRotatorManufacturer,	"AlpacaPi");

	//*	for IRotatorV3, this must be TRUE, for now we are using version 2
	cRotatorProp.CanReverse	=	false;

	cRotatorStepsPerRev		=	1000;
	cRotatorProp.StepSize	=	360.0 / cRotatorStepsPerRev;
}

//**************************************************************************************
// Destructor
//**************************************************************************************
RotatorDriver_Sim::~RotatorDriver_Sim(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
}


//*****************************************************************************
int32_t	RotatorDriver_Sim::ReadCurrentPoisiton_steps(void)
{
int32_t		myRotatorPosition;

//	CONSOLE_DEBUG(__FUNCTION__);

	myRotatorPosition	=	cRotatorProp.Position * cRotatorProp.StepSize;

	return(myRotatorPosition);
}

//*****************************************************************************
double	RotatorDriver_Sim::ReadCurrentPoisiton_degs(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	return(cRotatorProp.Position);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver_Sim::SetCurrentPoisiton_steps(const int32_t newPosition)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_INT32("newPosition\t=", newPosition);
	cRotatorProp.IsMoving			=	false;

	DumpRotatorProperties(__FUNCTION__);
	CONSOLE_ABORT(__FUNCTION__);

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver_Sim::SetCurrentPoisiton_degs(const double newPosition_Degs)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
int32_t				newPosition_Steps;

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_DBL("newPosition_Degs     \t=", newPosition_Degs);
	CONSOLE_DEBUG_W_INT32("cRotatorStepsPerRev\t=", cRotatorStepsPerRev);
	//*	we have to calculate the new position based on steps

	cRotatorProp.MechanicalPosition	=	newPosition_Degs;
	cRotatorProp.Position			=	newPosition_Degs;

	DumpRotatorProperties(__FUNCTION__);

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	RotatorDriver_Sim::HaltMovement(void)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;

//	CONSOLE_DEBUG(__FUNCTION__);

	return(alpacaErrCode);
}

//*****************************************************************************
bool	RotatorDriver_Sim::IsRotatorMoving(void)
{
bool	isMoving;

//	CONSOLE_DEBUG(__FUNCTION__);
	isMoving	=	false;
	return(isMoving);
}


#endif // _ENABLE_ROTATOR_SIMULATOR_
