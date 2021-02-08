//**************************************************************************
//*	Name:			telescopedriver_lx200.cpp
//*
//*	Author:			Mark Sproul (C) 2021
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*					This driver implements an Alpaca Telescope
//*					talking to an LX200 compatible mount
//*					via ethernet, USB or serial port
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
//*	Usage notes:	This driver does not implement any actual device,
//*					you must create a sub-class that does the actual control
//*
//*	References:		https://ascom-standards.org/api/
//*					https://ascom-standards.org/Help/Developer/html/N_ASCOM_DeviceInterface.htm
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Jan 13,	2021	<MLS> Created telescopedriver_lx200.cpp
//*	Jan 21,	2021	<MLS> Added  AlpacaConnect() & AlpacaDisConnect() to telescope
//*	Jan 21,	2021	<MLS> TelescopeLX200 now talking via ethernet to LX200 (TSC)
//*	Feb  7,	2021	<MLS> Started adding Telescope_comm support
//*	Feb  7,	2021	<MLS> Added _USE_TELESCOPE_COM_OBJECT_
//*****************************************************************************


#ifdef _ENABLE_TELESCOPE_LX200_

#include	<ctype.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<math.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
//#include	"JsonResponse.h"
//#include	"sidereal.h"

//#define	_USE_TELESCOPE_COM_OBJECT_

#ifdef _USE_TELESCOPE_COM_OBJECT_
	#include	"telescope_comm.h"
#else
	#include	"lx200_com.h"
#endif


#include	"telescopedriver.h"
#include	"telescopedriver_lx200.h"


//**************************************************************************************
void	CreateTelescopeLX200Objects(void)
{
	new TelescopeDriverLX200(kDevCon_Ethernet, "192.168.1.104:49152");
}

//**************************************************************************************
//*	the device path is one of these options
//*		192.168.1.104:49152
//*		/dev/ttyUSB0
//*		/dev/ttyS0
//**************************************************************************************
TelescopeDriverLX200::TelescopeDriverLX200(DeviceConnectionType connectionType, const char *devicePath)
	:TelescopeDriver()
{

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name,			"Telescope-LX200");
	strcpy(cCommonProp.Description,	"Telescope control using LX200 protocol");

	cDeviceConnType	=	connectionType;
	strcpy(cDeviceConnPath,	devicePath);

	//*	setup the options for this driver
	cTelescopeProp.AlginmentMode	=	kAlignmentMode_algGermanPolar;
	cTelescopeProp.CanSlewAsync		=	true;
	cTelescopeProp.CanSync			=	true;
	cTelescopeProp.CanSetTracking	=	true;


#ifdef _USE_TELESCOPE_COM_OBJECT_
	cTelescopeComm	=	NULL;
#endif
	AlpacaConnect();
}

//**************************************************************************************
// Destructor
//**************************************************************************************
TelescopeDriverLX200::~TelescopeDriverLX200(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	AlpacaDisConnect();
}

//**************************************************************************************
int32_t	TelescopeDriverLX200::RunStateMachine(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _USE_TELESCOPE_COM_OBJECT_
#else
	if (gTelescopeUpdated)
	{
//		CONSOLE_DEBUG("gTelescopeUpdated");
		cTelescopeProp.RightAscension	=	gTelescopeRA_Hours;
		cTelescopeProp.Declination		=	gTelescopeDecl_Degrees;
//		CONSOLE_DEBUG_W_DBL("cRightAscension\t=",	cRightAscension);
//		CONSOLE_DEBUG_W_DBL("cDeclination\t=",		cDeclination);

		if (gLX200_ThreadActive)
		{
			if (cDeviceConnected == false)
			{
				//*	if it was disconnected, print out a message
				CONSOLE_DEBUG("LX200 connection Established");
			}
			cDeviceConnected	=	true;
		}
		else
		{
			if (cDeviceConnected)
			{
				//*	if it was connected, print out a message
				CONSOLE_DEBUG("LX200 connection failed!!!!!!!!!!!!!!!!!!");
			}
			cDeviceConnected	=	false;
		}

		gTelescopeUpdated	=	false;

		cTelescopeProp.Slewing	=	false;
	}
#endif
	return(15 * 1000 * 1000);
}

//*****************************************************************************
bool	TelescopeDriverLX200::AlpacaConnect(void)
{
char	ipAddrString[128];
int		tcpPort;
char	*colonPtr;
int		lx200ErrCode;
char	lx200ErrMsg[128];
bool	okFlag;

	CONSOLE_DEBUG(__FUNCTION__);
	okFlag	=	false;
	switch(cDeviceConnType)
	{
		case kDevCon_Ethernet:
			strcpy(ipAddrString, cDeviceConnPath);
			colonPtr	=	strchr(ipAddrString, ':');
			if (colonPtr != NULL)
			{
				*colonPtr	=	0;
				colonPtr++;
				tcpPort		=	atoi(colonPtr);
				CONSOLE_DEBUG_W_STR("ipAddrString\t=",	ipAddrString);
				CONSOLE_DEBUG_W_NUM("tcpPort\t=",	tcpPort);
			#ifdef _USE_TELESCOPE_COM_OBJECT_
				CONSOLE_DEBUG("creating TelescopeComm object!!!!!!!!!!!!!!!!");
				if (cTelescopeComm == NULL)
				{
					cTelescopeComm	=	new TelescopeComm(kDevCon_Ethernet, ipAddrString, tcpPort);
				}

				cTelescopeComm->StartThread();
			#else
				lx200ErrCode	=	LX200_StartThread(ipAddrString, tcpPort, lx200ErrMsg);
				okFlag			=	true;
			#endif
			}
			break;

	}
	return(okFlag);
}

//*****************************************************************************
bool	TelescopeDriverLX200::AlpacaDisConnect(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
#ifdef _USE_TELESCOPE_COM_OBJECT_
	if (cTelescopeComm != NULL)
	{
		cTelescopeComm->StopThread();
	}
#else
	LX200_StopThread();
#endif
	return(true);
}

//*****************************************************************************
//*	needs to be over-ridden
TYPE_ASCOM_STATUS	TelescopeDriverLX200::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

#ifdef _USE_TELESCOPE_COM_OBJECT_
	if (cTelescopeComm != NULL)
	{
		cTelescopeComm->StopMovement();
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}
#else
	LX200_StopMovement();
	alpacaErrCode	=	kASCOM_Err_Success;
#endif


	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverLX200::Telescope_SlewToRA_DEC(	const double	newRA,
																	const double	newDec,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
bool				returnCode;

	CONSOLE_DEBUG(__FUNCTION__);

	returnCode	=	false;
#ifdef _USE_TELESCOPE_COM_OBJECT_
	if (cTelescopeComm != NULL)
	{
		returnCode	=	cTelescopeComm->SlewScopeDegrees(newRA, newDec, alpacaErrMsg);
	}
#else
	returnCode	=	LX200_SlewScopeDegrees(newRA, newDec, alpacaErrMsg);
#endif // _USE_TELESCOPE_COM_OBJECT_
	if (returnCode)
	{
		cTelescopeProp.Slewing	=	true;
		alpacaErrCode			=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverLX200::Telescope_SyncToRA_DEC(	const double	newRA,
																	const double	newDec,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_NotImplemented;
bool				returnCode;

	CONSOLE_DEBUG("-------------------------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);

	returnCode	=	false;
#ifdef _USE_TELESCOPE_COM_OBJECT_
	if (cTelescopeComm != NULL)
	{
		returnCode	=	cTelescopeComm->SyncScopeDegrees(newRA, newDec, alpacaErrMsg);
	}
#else
	returnCode	=	LX200_SyncScopeDegrees(newRA, newDec, alpacaErrMsg);
#endif // _USE_TELESCOPE_COM_OBJECT_
	if (returnCode)
	{
		CONSOLE_DEBUG("kASCOM_Err_Success");
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		CONSOLE_DEBUG("kASCOM_Err_NotConnected");
		alpacaErrCode	=	kASCOM_Err_NotConnected;
	}

	return(alpacaErrCode);
}

#endif // _ENABLE_TELESCOPE_LX200_
