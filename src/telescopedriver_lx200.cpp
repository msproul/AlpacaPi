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
//*	Feb 15,	2021	<MLS> Working on telescope commands Telescope_*()
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
#include	"helper_functions.h"

#include	"lx200_com.h"


#include	"telescopedriver.h"
#include	"telescopedriver_lx200.h"


//**************************************************************************************
void	CreateTelescopeLX200Objects(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	new TelescopeDriverLX200(kDevCon_Ethernet, "192.168.1.104:49152");
}

//**************************************************************************************
//*	the device path is one of these options
//*		192.168.1.104:49152
//*		/dev/ttyUSB0
//*		/dev/ttyS0
//**************************************************************************************
TelescopeDriverLX200::TelescopeDriverLX200(DeviceConnectionType connectionType, const char *devicePath)
	:TelescopeDriverComm(connectionType, devicePath)
{

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name,		"Telescope-LX200");
	strcpy(cCommonProp.Description,	"Telescope control using LX200 protocol");

	//*	setup the options for this driver
	cTelescopeProp.AlginmentMode	=	kAlignmentMode_algGermanPolar;
	cTelescopeProp.CanSlewAsync		=	true;
	cTelescopeProp.CanSync			=	true;
	cTelescopeProp.CanSetTracking	=	true;
	cTelescopeProp.CanMoveAxis		=	true;
	cTelescopeProp.CanUnpark		=	true;


	cTelescopeInfoValid				=	false;
	cLX200_SocketErrCnt				=	0;
	cLX200_OutOfBoundsCnt			=	0;
	cTelescopeRA_String[0]			=	0;
	cTelescopeDecl_String[0]		=	0;

	cQueuedCmdCnt					=	0;


	AlpacaConnect();

	CONSOLE_DEBUG_W_NUM("cTelescopeProp.CanUnpark\t=", cTelescopeProp.CanUnpark);

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
bool	TelescopeDriverLX200::SendCmdsFromQueue(void)
{
int		returnByteCNt;
char	returnBuffer[500];
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
	while (cQueuedCmdCnt > 0)
	{
		CONSOLE_DEBUG_W_STR("Sending", cCmdQueue[0].cmdString);
		returnByteCNt	=	LX200_SendCommand(	cSocket_desc,
												cCmdQueue[0].cmdString,
												returnBuffer,
												400);
		if (returnByteCNt > 0)
		{
			CONSOLE_DEBUG_W_STR("returnBuffer\t=", returnBuffer);
		}
		for (iii=0; iii<cQueuedCmdCnt; iii++)
		{
			cCmdQueue[iii]	=	cCmdQueue[iii + 1];
		}
		cQueuedCmdCnt--;
		if (cQueuedCmdCnt > 0)
		{
			usleep(500);
		}
	}
	return(false);
}


//**************************************************************************************
bool	TelescopeDriverLX200::SendCmdsPeriodic(void)
{
int		returnByteCNt;
char	dataBuffer[500];
bool	isValid;

//	CONSOLE_DEBUG(__FUNCTION__);
	isValid	=	false;
	//--------------------------------------------------------------------------
	//*	Right Ascension
	returnByteCNt	=	LX200_SendCommand(cSocket_desc, "GR", dataBuffer, 400);
	if (returnByteCNt > 0)
	{
		isValid			=	Process_GR_RtAsc(dataBuffer);
		if (isValid)
		{
			cTelescopeInfoValid	=	true;
		}
		else
		{
			strcpy(cTelescopeRA_String, "RA failed");
			cLX200_SocketErrCnt++;
			cTelescopeInfoValid	=	false;
			CONSOLE_DEBUG_W_NUM("cLX200_SocketErrCnt\t=", cLX200_SocketErrCnt);
		}
		usleep(1000);
	}
	else
	{
		cLX200_SocketErrCnt++;
		CONSOLE_DEBUG_W_NUM("cLX200_SocketErrCnt\t=", cLX200_SocketErrCnt);
	}

	//--------------------------------------------------------------------------
	//*	Declination
	returnByteCNt	=	LX200_SendCommand(cSocket_desc, "GD", dataBuffer, 400);
	if (returnByteCNt > 0)
	{
		isValid			=	Process_GD(dataBuffer);
		if (isValid)
		{
			gTelescopeInfoValid	=	true;
		}
		else
		{
			strcpy(gTelescopeDecl_String, "DEC failed");
			cLX200_SocketErrCnt++;
			cTelescopeInfoValid	=	false;
		}
		usleep(1000);
	}
	else
	{
		cLX200_SocketErrCnt++;
		CONSOLE_DEBUG_W_NUM("cLX200_SocketErrCnt\t=", cLX200_SocketErrCnt);
	}

	//--------------------------------------------------------------------------
	//*	TrackingRate
//	returnByteCNt	=	LX200_SendCommand(cSocket_desc, "GT", dataBuffer, 400);
//	if (returnByteCNt > 0)
//	{
//		isValid			=	Process_GT(dataBuffer);
//		if (isValid)
//		{
//			cTelescopeInfoValid	=	true;
//		}
//		else
//		{
//		}
//		usleep(1000);
//	}
//	else
//	{
//		cLX200_SocketErrCnt++;
//		CONSOLE_DEBUG_W_NUM("cLX200_SocketErrCnt\t=", cLX200_SocketErrCnt);
//	}

	return(isValid);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverLX200::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	//*	because this is ABORT, we are going to wipe out all pending commands
	cQueuedCmdCnt	=	0;
	AddCmdToQueue("Q");
	cTelescopeProp.Slewing	=	false;

	return(alpacaErrCode);

}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverLX200::Telescope_MoveAxis(const int axisNum, const double moveRate_degPerSec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_Success;

	switch(axisNum)
	{
		case 0:
			if (moveRate_degPerSec > 0)
			{
				AddCmdToQueue("Mw");
			}
			else
			{
				AddCmdToQueue("Me");
			}
			cTelescopeProp.Slewing	=	true;
			break;

		case 1:
			if (moveRate_degPerSec > 0)
			{
				AddCmdToQueue("Mn");
			}
			else
			{
				AddCmdToQueue("Ms");
			}
			cTelescopeProp.Slewing	=	true;
			break;

		default:
			alpacaErrCode	=	kASCOM_Err_NotImplemented;
			GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
			break;

	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverLX200::Telescope_SlewToRA_DEC(	const double	newRtAscen_Hours,
																	const double	newDeclination_Degrees,
																	char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
bool				successFlg;
char				timeString[32];
char				commandString[48];

	CONSOLE_DEBUG(__FUNCTION__);
	successFlg		=	false;

	FormatHHMMSS(newRtAscen_Hours, timeString, false);

	//*	create the LX200 command SrHH:MM:SS
	strcpy(commandString, "Sr");
	strcat(commandString, timeString);
	AddCmdToQueue(commandString);

	//-------------------------------------------------

	FormatHHMMSS(newDeclination_Degrees, timeString, true);

	//*	create the LX200 command SdsDD*MM
	strcpy(commandString, "Sd");
	strcat(commandString, timeString);
	AddCmdToQueue(commandString);

	//*	Slew command
	//*	:MS# Slew to Target Object
	//		Returns:
	//		0				Slew is Possible
	//		1<string>#		Object Below Horizon w/string message
	//		2<string>#		Object Below Higher w/string message
	AddCmdToQueue("MS");

	return(alpacaErrCode);
}

//*****************************************************************************
//*	:SrHH:MM.T#
//*	:SrHH:MM:SS#
//*	Set target object RA to HH:MM.T or HH:MM:SS depending on the current precision setting.
//*	Returns:
//*	0 – Invalid
//*	1 - Valid
//*
//*	:SdsDD*MM#
//*	Set target object declination to sDD*MM or sDD*MM:SS depending on the current precision setting
//*	Returns:
//*	1 - Dec Accepted
//*	0 – Dec invalid
//*
//*	:CM#
//*	Synchronizes the telescope's position with the currently selected database object's coordinates.
//*	Returns:
//*	LX200's - a "#" terminated string with the name of the object that was synced.
//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverLX200::Telescope_SyncToRA_DEC(	const double	newRtAscen_Hours,
																	const double	newDeclination_Degrees,
																	char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				timeString[32];
char				commandString[48];

	FormatHHMMSS(newRtAscen_Hours, timeString, false);

	//*	create the LX200 command SrHH:MM:SS
	strcpy(commandString, "Sr");
	strcat(commandString, timeString);
	AddCmdToQueue(commandString);

	//-------------------------------------------------

	FormatHHMMSS(newDeclination_Degrees, timeString, true);

	//*	create the LX200 command SdsDD*MM
	strcpy(commandString, "Sd");
	strcat(commandString, timeString);
	AddCmdToQueue(commandString);

	//*	syncCommand
	//*	:CM# Synchronizes the telescope's position with the currently selected database object's coordinates
	AddCmdToQueue("CM");


	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverLX200::Telescope_TrackingOnOff(const bool newTrackingState, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	if (newTrackingState)
	{
		AddCmdToQueue("TQ");
		alpacaErrCode	=	kASCOM_Err_Success;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");

	}
	return(alpacaErrCode);
}

#if 0
//*****************************************************************************
void	TelescopeDriverLX200::AddCmdToLX200queue(const char *cmdString)
{
	CONSOLE_DEBUG_W_STR("cmdString\t\t=", cmdString);
	if (cQueuedCmdCnt < kMaxTelescopeCmds)
	{
		strcpy(cCmdQueue[cQueuedCmdCnt].cmdString, cmdString);
		cQueuedCmdCnt++;
	}
}
#endif // 0


//*****************************************************************************
static bool	CheckForValidResponse(const char *lx200ResponseString)
{
bool	isValid;
int		strLen;

	isValid	=	false;
	strLen		=	strlen(lx200ResponseString);
	if (strLen > 3)
	{
		//*	check for valid data
		if (lx200ResponseString[strLen-1] == '#')
		{
			//*	looking good
			isValid	=	true;
		}
	}
	return(isValid);
}

//*****************************************************************************
//	dataBuffer	= +40*58:56#
//*****************************************************************************
static double	LX200_ParseDegMinSec(char *dataBuffer)
{
int		degrees;
int		minutes;
int		seconds;
int		plusMinus;
char	*charPtr;
double	degrees_Dbl;
bool	isValid;

//	CONSOLE_DEBUG_W_STR("dataBuffer\t=",		dataBuffer);
	degrees		=	0;
	minutes		=	0;
	seconds		=	0;
	degrees_Dbl	=	0.0;
	plusMinus	=	1;
	isValid		=	CheckForValidResponse(dataBuffer);
	if (isValid)
	{
		//*	looking good
		charPtr	=	dataBuffer;
		if (*charPtr == '+')
		{
			plusMinus	=	1;
			charPtr++;
		}
		if (*charPtr == '-')
		{
			plusMinus	=	-1;
			charPtr++;
		}
		degrees	=	atoi(charPtr);
		while (isdigit(*charPtr))
		{
			charPtr++;
		}
		charPtr++;
		minutes	=	atoi(charPtr);
		while (isdigit(*charPtr))
		{
			charPtr++;
		}
		charPtr++;
		seconds	=	atoi(charPtr);

//			CONSOLE_DEBUG_W_NUM("degrees\t=",		degrees);
//			CONSOLE_DEBUG_W_NUM("minutes\t=",		minutes);
//			CONSOLE_DEBUG_W_NUM("seconds\t=",		seconds);

		degrees_Dbl	=	degrees;
		degrees_Dbl	+=	(1.0 * minutes) / 60.0;
		degrees_Dbl	+=	(1.0 * seconds) / 3600.0;

		degrees_Dbl	=	plusMinus * degrees_Dbl;
	}
	return(degrees_Dbl);
}

//*****************************************************************************
//*	process Right Ascension
//*****************************************************************************
bool	TelescopeDriverLX200::Process_GR_RtAsc(char *dataBuffer)
{
double	degrees_Dbl;
bool	isValid;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, dataBuffer);
	isValid		=	CheckForValidResponse(dataBuffer);
	if (isValid)
	{
		degrees_Dbl	=	LX200_ParseDegMinSec(dataBuffer);
	//	if ((degrees_Dbl >= 0.0) && (degrees_Dbl <= 24.0))
		if ((degrees_Dbl >= 0.0))
		{
			if (strlen(dataBuffer) < 32)
			{
				strcpy(cTelescopeRA_String, dataBuffer);
			}

			while (degrees_Dbl > 24.0)
			{
				degrees_Dbl	-=	24.0;
			}
			//*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//*	Important note:
			//*	rtAscension is 0->24 not 0->360 / 0->2_pi
			//*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			cTelescopeProp.RightAscension	=	degrees_Dbl;

//+			gTelescopeRA_Hours		=	degrees_Dbl;
//+			gTelescopeRA_Radians	=	(degrees_Dbl / 24.0) * (2.0 * M_PI);
		}
		else
		{
//+			gLX200_OutOfBoundsCnt++;
		}
	}
	return(isValid);
}

//*****************************************************************************
//*	process Declination
//*****************************************************************************
bool	TelescopeDriverLX200::Process_GD(char *dataBuffer)
{
double	degrees_Dbl;

bool	isValid;

	isValid		=	CheckForValidResponse(dataBuffer);
	if (isValid)
	{
		degrees_Dbl	=	LX200_ParseDegMinSec(dataBuffer);
		if ((degrees_Dbl >= -90.0) && (degrees_Dbl <= 90.0))
		{
			if (strlen(dataBuffer) < 32)
			{
				strcpy(cTelescopeDecl_String, dataBuffer);
			}
			cTelescopeProp.Declination	=	degrees_Dbl;
		}
		else
		{
			cLX200_OutOfBoundsCnt++;
		}
	}
	return(isValid);
}

//*****************************************************************************
//*	process Tracking
//*****************************************************************************
bool	TelescopeDriverLX200::Process_GT(char *dataBuffer)
{
bool	isValid;

	isValid		=	CheckForValidResponse(dataBuffer);
	if (isValid)
	{
//		CONSOLE_DEBUG_W_STR(__FUNCTION__, dataBuffer);
	//	gTelescopeTrackingRate	=	atof(dataBuffer);
	}
	return(isValid);
}

#endif // _ENABLE_TELESCOPE_LX200_
