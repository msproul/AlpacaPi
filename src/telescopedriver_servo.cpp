//**************************************************************************
//*	Name:			telescopedriver_Servo.cpp
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
//*	Mar 25,	2022	<MLS> Created telescopedriver_Servo.cpp
//*	Mar 25,	2022	<MLS> Servo version is being created for Ron S <RNS>
//*****************************************************************************


#ifdef _ENABLE_TELESCOPE_SERVO_

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


#include	"telescopedriver.h"
#include	"telescopedriver_servo.h"


static bool	ReadServoConfigFile(void);

//**************************************************************************************
TelescopeDriverServo::TelescopeDriverServo(void)
	:TelescopeDriver()
{

	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(cCommonProp.Name,		"Telescope-Servo");
	strcpy(cCommonProp.Description,	"Telescope control using Servo protocol");

	//*	setup the options for this driver
	cTelescopeProp.AlginmentMode	=	kAlignmentMode_algGermanPolar;
	cTelescopeProp.CanFindHome		=	false;
	cTelescopeProp.CanMoveAxis		=	false;
	cTelescopeProp.CanSetTracking	=	false;
	cTelescopeProp.CanSlewAsync		=	false;
	cTelescopeProp.CanSync			=	false;
	cTelescopeProp.CanUnpark		=	false;

	ReadServoConfigFile();

	AlpacaConnect();

	CONSOLE_DEBUG_W_NUM("cTelescopeProp.CanUnpark\t=", cTelescopeProp.CanUnpark);
}

//**************************************************************************************
// Destructor
//**************************************************************************************
TelescopeDriverServo::~TelescopeDriverServo(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	AlpacaDisConnect();
}

//*****************************************************************************
//*	returns delay time in micro-seconds
//*****************************************************************************
int32_t	TelescopeDriverServo::RunStateMachine(void)
{
	//*	this is where your periodic code goes
	//*	update cTelescopeProp values here


	//*	5 * 1000 * 1000 means you might not get called again for 5 seconds
	//*	you might get called earlier
	return(5 * 1000 * 1000);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_AbortSlew(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);

}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_FindHome(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_MoveAxis(const int axisNum, const double moveRate_degPerSec, char *alpacaErrMsg)
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
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_Park(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_SetPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_SlewToAltAz(const double newAlt_Degrees, const double newAz_Degrees, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_SlewToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}


//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_SyncToRA_DEC(const double newRA, const double newDec, char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);
}

//*****************************************************************************
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_TrackingOnOff(const bool newTrackingState, char *alpacaErrMsg)
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
TYPE_ASCOM_STATUS	TelescopeDriverServo::Telescope_UnPark(char *alpacaErrMsg)
{
TYPE_ASCOM_STATUS		alpacaErrCode	=	kASCOM_Err_NotImplemented;

	GENERATE_ALPACAPI_ERRMSG(alpacaErrMsg, "Not implemented");
	return(alpacaErrCode);

}

//*****************************************************************************
enum
{
	kServo_MC_FREQ	=	0,
	kServo_COMM_PORT,
	kServo_BAUD,
	kServo_MOUNT,
	kServo_RA_CONFIG,
	kServo_RA_MOTOR_GEAR,
	kServo_RA_MAIN_GEAR,
	kServo_RA_ENCODER,
	kServo_RA_MAX_VEL,
	kServo_RA_MAX_ACC,
	kServo_RA_ADJ_VEL,
	kServo_RA_SLEW_VEL,
	kServo_RA_SI_CON,
	kServo_RA_KP_CON,
	kServo_RA_KI_CON,
	kServo_RA_KD_CON,
	kServo_RA_IL_CON,
	kServo_DEC_CONFIG,
	kServo_DEC_MOTOR_GEAR,
	kServo_DEC_MAIN_GEAR,
	kServo_DEC_ENCODER,
	kServo_DEC_MAX_VEL,
	kServo_DEC_MAX_ACC,
	kServo_DEC_ADJ_VEL,
	kServo_DEC_SLEW_VEL,
	kServo_DEC_SI_CON,
	kServo_DEC_KP_CON,
	kServo_DEC_KI_CON,
	kServo_DEC_KD_CON,
	kServo_DEC_IL_CON,
	//	Note:  RA does not have a lock down position since LST is used
	kServo_DEC_LOCK_DOWN,
	kServo_RA_LOCK_DOWN,
	kServo_RA_GEAR_LASH,
	kServo_DEC_GEAR_LASH,
	kServo_ROLLOVER_WIN,
	kServo_RA_PRECESSION,
	kServo_DEC_PRECESSION,
	kServo_RA_STANDBY,
	kServo_DEC_STANDBY,
	kServo_RA_HOME,
	kServo_DEC_HOME,
	kServo_RA_HOME_FLAG,
	kServo_DEC_HOME_FLAG,
	kServo_OFF_TARGET_TOL,
	kServo_FAST_ON_TARGET,

	kServo_last

};

//*****************************************************************************
typedef struct
{
	char	keyword[16];
	int		enumValue;
} TYPE_KEYWORDS;

//*****************************************************************************
static TYPE_KEYWORDS	gServoCfgTable[]	=
{

	{	"MC_FREQ",			kServo_MC_FREQ			},
	{	"COMM_PORT",		kServo_COMM_PORT		},
	{	"BAUD",				kServo_BAUD				},
	{	"MOUNT",			kServo_MOUNT			},
	{	"RA_CONFIG",		kServo_RA_CONFIG		},
	{	"RA_MOTOR_GEAR",	kServo_RA_MOTOR_GEAR	},
	{	"RA_MAIN_GEAR",		kServo_RA_MAIN_GEAR		},
	{	"RA_ENCODER",		kServo_RA_ENCODER		},
	{	"RA_MAX_VEL",		kServo_RA_MAX_VEL		},
	{	"RA_MAX_ACC",		kServo_RA_MAX_ACC		},
	{	"RA_ADJ_VEL",		kServo_RA_ADJ_VEL		},
	{	"RA_SLEW_VEL",		kServo_RA_SLEW_VEL		},
	{	"RA_SI_CON",		kServo_RA_SI_CON		},
	{	"RA_KP_CON",		kServo_RA_KP_CON		},
	{	"RA_KI_CON",		kServo_RA_KI_CON		},
	{	"RA_KD_CON",		kServo_RA_KD_CON		},
	{	"RA_IL_CON",		kServo_RA_IL_CON		},
	{	"DEC_CONFIG",		kServo_DEC_CONFIG		},
	{	"DEC_MOTOR_GEAR",	kServo_DEC_MOTOR_GEAR	},
	{	"DEC_MAIN_GEAR",	kServo_DEC_MAIN_GEAR	},
	{	"DEC_ENCODER",		kServo_DEC_ENCODER		},
	{	"DEC_MAX_VEL",		kServo_DEC_MAX_VEL		},
	{	"DEC_MAX_ACC",		kServo_DEC_MAX_ACC		},
	{	"DEC_ADJ_VEL",		kServo_DEC_ADJ_VEL		},
	{	"DEC_SLEW_VEL",		kServo_DEC_SLEW_VEL		},
	{	"DEC_SI_CON",		kServo_DEC_SI_CON		},
	{	"DEC_KP_CON",		kServo_DEC_KP_CON		},
	{	"DEC_KI_CON",		kServo_DEC_KI_CON		},
	{	"DEC_KD_CON",		kServo_DEC_KD_CON		},
	{	"DEC_IL_CON",		kServo_DEC_IL_CON		},
	//	Note:  RA does not have a lock down position since LST is used
	{	"DEC_LOCK_DOWN",	kServo_DEC_LOCK_DOWN	},
	{	"RA_LOCK_DOWN",		kServo_RA_LOCK_DOWN		},
	{	"RA_GEAR_LASH",		kServo_RA_GEAR_LASH		},
	{	"DEC_GEAR_LASH",	kServo_DEC_GEAR_LASH	},
	{	"ROLLOVER_WIN",		kServo_ROLLOVER_WIN		},
	{	"RA_PRECESSION",	kServo_RA_PRECESSION	},
	{	"DEC_PRECESSION",	kServo_DEC_PRECESSION	},
	{	"RA_STANDBY",		kServo_RA_STANDBY		},
	{	"DEC_STANDBY",		kServo_DEC_STANDBY		},
	{	"RA_HOME",			kServo_RA_HOME			},
	{	"DEC_HOME",			kServo_DEC_HOME			},
	{	"RA_HOME_FLAG",		kServo_RA_HOME_FLAG		},
	{	"DEC_HOME_FLAG",	kServo_DEC_HOME_FLAG	},
	{	"OFF_TARGET_TOL",	kServo_OFF_TARGET_TOL	},
	{	"FAST_ON_TARGET",	kServo_FAST_ON_TARGET	},

	{	"",	-1	}

};

#if 0
//*****************************************************************************
static void	CheckDuplicates(void)
{
int		iii;
int		jjj;
int		dupCnt;

	CONSOLE_DEBUG(__FUNCTION__);
	iii			=	0;
	dupCnt		=	0;
	while (gServoCfgTable[iii].enumValue >= 0)
	{
		jjj	=	iii + 1;
		while (gServoCfgTable[jjj].enumValue >= 0)
		{
			if (strcasecmp(gServoCfgTable[jjj].keyword, gServoCfgTable[iii].keyword) == 0)
			{
				CONSOLE_DEBUG_W_STR("duplicate:", gServoCfgTable[jjj].keyword);
				dupCnt++;
			}
			jjj++;
		}
		iii++;
	}
	CONSOLE_DEBUG_W_NUM("dupCnt=", dupCnt);

}
#endif

//*****************************************************************************
//*	returns false if the keyword was not valid
//*****************************************************************************
static	bool ProcessServoConfigLine(char *lineBuff)
{
char	keyword[32];
int		iii;
int		ccc;
int		slen;
char	*valueStrPtr;
bool	validEntry;
int		keywordEnum;

	iii			=	0;
	ccc			=	0;
	validEntry	=	true;
	slen		=	strlen(lineBuff);
	while ((iii<slen) && (ccc < 31) && (lineBuff[iii] != ':') && (lineBuff[iii] > 0x20))
	{
		keyword[ccc]	=	lineBuff[iii];
		ccc++;
		iii++;
	}
	keyword[ccc]	=	0;

	valueStrPtr	=	strchr(lineBuff, ':');
	if (valueStrPtr != NULL)
	{
		valueStrPtr++;		//*	skip over the ":"
		while (*valueStrPtr <= 0x20)
		{
			valueStrPtr++;
		}

		//*	now find the keyword in the table
		keywordEnum	=	-1;
		iii			=	0;
		while ((gServoCfgTable[iii].enumValue >= 0) && (keywordEnum < 0))
		{
			if (strcasecmp(keyword, gServoCfgTable[iii].keyword) == 0)
			{
				keywordEnum	=	gServoCfgTable[iii].enumValue;
			}
			iii++;
		}

		if (keywordEnum >= 0)
		{
			CONSOLE_DEBUG_W_2STR("kw:val", keyword, valueStrPtr);
			//*	we have a valid keyword, do something with it
			switch (keywordEnum)
			{
				case kServo_MC_FREQ:
				case kServo_COMM_PORT:
				case kServo_BAUD:
				case kServo_MOUNT:
				case kServo_RA_CONFIG:
				case kServo_RA_MOTOR_GEAR:
				case kServo_RA_MAIN_GEAR:
				case kServo_RA_ENCODER:
				case kServo_RA_MAX_VEL:
				case kServo_RA_MAX_ACC:
				case kServo_RA_ADJ_VEL:
				case kServo_RA_SLEW_VEL:
				case kServo_RA_SI_CON:
				case kServo_RA_KP_CON:
				case kServo_RA_KI_CON:
				case kServo_RA_KD_CON:
				case kServo_RA_IL_CON:
				case kServo_DEC_CONFIG:
				case kServo_DEC_MOTOR_GEAR:
				case kServo_DEC_MAIN_GEAR:
				case kServo_DEC_ENCODER:
				case kServo_DEC_MAX_VEL:
				case kServo_DEC_MAX_ACC:
				case kServo_DEC_ADJ_VEL:
				case kServo_DEC_SLEW_VEL:
				case kServo_DEC_SI_CON:
				case kServo_DEC_KP_CON:
				case kServo_DEC_KI_CON:
				case kServo_DEC_KD_CON:
				case kServo_DEC_IL_CON:
				case kServo_DEC_LOCK_DOWN:
				case kServo_RA_LOCK_DOWN:
				case kServo_RA_GEAR_LASH:
				case kServo_DEC_GEAR_LASH:
				case kServo_ROLLOVER_WIN:
				case kServo_RA_PRECESSION:
				case kServo_DEC_PRECESSION:
				case kServo_RA_STANDBY:
				case kServo_DEC_STANDBY:
				case kServo_RA_HOME:
				case kServo_DEC_HOME:
				case kServo_RA_HOME_FLAG:
				case kServo_DEC_HOME_FLAG:
				case kServo_OFF_TARGET_TOL:
				case kServo_FAST_ON_TARGET:
					break;

				default:
					validEntry	=	false;
					break;
			}
		}
	}
	return(	validEntry);
}

//*****************************************************************************
static bool	ReadServoConfigFile(void)
{
FILE	*filePointer;
char	lineBuff[256];
int		iii;
int		slen;
bool	configOK;
bool	validEntry;
char	configFileName[]	=	"ss_scope.cfg";

	CONSOLE_DEBUG(__FUNCTION__);

//	CheckDuplicates();

	configOK	=	false;
	//*	check for the sql server settings file
	filePointer	=	fopen(configFileName, "r");
	if (filePointer != NULL)
	{
		configOK	=	true;
		while (fgets(lineBuff, 200, filePointer))
		{
			//*	get rid of the trailing CR/LF
			slen	=	strlen(lineBuff);
			for (iii=0; iii<slen; iii++)
			{
				if ((lineBuff[iii] == 0x0d) || (lineBuff[iii] == 0x0a))
				{
					lineBuff[iii]	=	0;
					break;
				}
			}
			slen	=	strlen(lineBuff);
			if ((slen > 3) && (lineBuff[0] != '#'))
			{
				validEntry	=	ProcessServoConfigLine(lineBuff);
				if (validEntry == false)
				{
					CONSOLE_DEBUG_W_STR("Servo Config file contains invalid data:", lineBuff);
				}
			}
		}
		fclose(filePointer);
	}
	return(configOK);
}

#endif // _ENABLE_TELESCOPE_SERVO_
