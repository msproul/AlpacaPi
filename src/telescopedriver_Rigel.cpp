//**************************************************************************
//*	Name:			telescopedriver_Rigel.cpp
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
//*	Apr 20,	2021	<MLS> Created telescopedriver_Rigel.cpp
//*	Apr 20,	2021	<MLS> Rigel version is being created for Jim H <JMH>
//*	Mar 25,	2022	<JMH> Started working on fifo support
//*	Mar 25,	2022	<MLS> Merged JMH fifo support into main code branch
//*****************************************************************************


#ifdef _ENABLE_TELESCOPE_RIGEL_

#include	<ctype.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<stdint.h>
#include	<unistd.h>
#include	<math.h>
#include	<unistd.h>
#include	<fcntl.h>
#include	<sys/shm.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
//#include "telstatshm.h"


#include	"telescopedriver.h"
#include	"telescopedriver_Rigel.h"

#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))

const char	*gRigelFifo	=	"/usr/local/telescope/comm/Tel.in";
int			tel_msg(char *msg);
// int open_telshm(TelStatShm **tpp);

//**************************************************************************************
TelescopeDriverRigel::TelescopeDriverRigel(void)
	:TelescopeDriver()
{

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name,		"Telescope-Rigel");
	strcpy(cCommonProp.Description,	"Telescope control using Rigel protocol");

	//*	setup the options for this driver
	cTelescopeProp.AlginmentMode	=	kAlignmentMode_algPolar;
	cTelescopeProp.CanSlewAsync		=	true;
	cTelescopeProp.CanSync			=	false;
	cTelescopeProp.CanSetTracking	=	false;
	cTelescopeProp.CanMoveAxis		=	false;
	cTelescopeProp.CanUnpark		=	false;
	cTelescopeProp.CanFindHome		=	true;
	cTelescopeProp.CanPark			=	true;
	cTelescopeProp.CanSlewAltAzAsync	=	true;
	cTelescopeProp.ApertureDiameter		=	0.370;		//*	meters
	cTelescopeProp.FocalLength			=	cTelescopeProp.ApertureDiameter * 14;	//*	meters
	cTelescopeProp.ApertureArea			=	M_PI * ((cTelescopeProp.ApertureDiameter / 2) * (cTelescopeProp.ApertureDiameter / 2)); // square meters

//	need to start Rigel daemons and connect to shared memory here

	AlpacaConnect();

	CONSOLE_DEBUG_W_NUM("cTelescopeProp.CanUnpark\t=", cTelescopeProp.CanUnpark);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
TelescopeDriverRigel::~TelescopeDriverRigel(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	AlpacaDisConnect();
}

//*****************************************************************************
//*	returns delay time in micro-seconds
//*****************************************************************************
int32_t	TelescopeDriverRigel::RunStateMachine(void)
{
	//*	this is where your periodic code goes
	//*	update cTelescopeProp values here

	//*	5 * 1000 * 1000 means you might not get called again for 5 seconds
	//*	you might get called earlier
	return(5 * 1000 * 1000);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				msg[80];

	sprintf(msg,"stop");
	tel_msg(msg);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_FindHome(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				msg[80];

	sprintf(msg,"home");
	tel_msg(msg);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_MoveAxis(	const int		axisNum,
																const double	moveRate_degPerSec,
																char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_MethodNotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);
	switch(axisNum)
	{
		case 0:
			if (moveRate_degPerSec > 0)
			{

			}
			else
			{

			}
			cTelescopeProp.Slewing	=	true;
			break;

		case 1:
			if (moveRate_degPerSec > 0)
			{

			}
			else
			{

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
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_Park(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
char				msg[80];

	sprintf(msg,"stow");
	tel_msg(msg);
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_SetPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_SlewToAltAz(const double	newAlt_Degrees,
																const double	newAz_Degrees,
																char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				newAlt_rad;
double				newAz_rad;
char				msg[80];

	newAlt_rad	=	RADIANS(newAlt_Degrees);
	newAz_rad	=	RADIANS(newAz_Degrees);

	sprintf(msg,"Alt:%.6f Az:%.6f", newAlt_rad, newAz_rad);
	tel_msg(msg);
	CONSOLE_DEBUG(msg);

	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_SlewToRA_DEC(	const double	newRA,
																	const double	newDec,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS	alpacaErrCode	=	kASCOM_Err_Success;
double				newRa_rad;
double				newDec_rad;
char				msg[80];

	newRa_rad	=	RADIANS(newRA * 15.0);	//*	newRA is hour angle not degrees
	newDec_rad	=	RADIANS(newDec);

	sprintf(msg,"RA:%.6f Dec:%.6f", newRa_rad, newDec_rad);
	tel_msg(msg);
	CONSOLE_DEBUG(msg);

	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_SyncToRA_DEC(	const double	newRA,
																	const double	newDec,
																	char			*alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_TrackingOnOff(	const bool	newTrackingState,
																	char		*alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	CONSOLE_DEBUG(__FUNCTION__);

	if (newTrackingState)
	{

		alpacaErrCode	=	kASCOM_Err_NotImplemented;
	}
	else
	{
		alpacaErrCode	=	kASCOM_Err_NotImplemented;
		GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");

	}
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverRigel::Telescope_UnPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);

}

//*****************************************************************************
int tel_msg(char *msg)
{
int fd;

	// Open FIFO for write only
	fd	=	open(gRigelFifo, O_WRONLY);
	if (fd >= 0)
	{
		//*	Write the input on FIFO
		write(fd, msg, strlen(msg)+1);

		//*	and close it
		close(fd);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Error opening fifo\t=", gRigelFifo);
//		CONSOLE_ABORT(__FUNCTION__);
	}
	return 0;
}

//* connect to the telstatshm shared memory segment.
//* return 0 and set *tpp if ok, else -1.

//int open_telshm(TelStatShm **tpp)
//{
//	int shmid;
//	long addr;
//
//	shmid = shmget (TELSTATSHMKEY, sizeof(TelStatShm), 0);
//	if (shmid < 0)
//		return (-1);
//
//	addr = (long) shmat (shmid, (void *)0, 0);
//	if (addr == -1)
//		return (-1);
//
//	*tpp = (TelStatShm *) addr;
//	return (0);
//}
#endif // _ENABLE_TELESCOPE_RIGEL_
