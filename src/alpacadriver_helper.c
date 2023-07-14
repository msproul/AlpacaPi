//*****************************************************************************
//*	Alpaca driver helper
//*
//*	Used by both AlpacaPi drives and clients
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Jan 16,	2021	<MLS> Created alpacadriver_helper.c
//*	Mar 12,	2022	<MLS> Added GetBinaryElementTypeString()
//*	Sep 25,	2022	<MLS> Added DumpObservingconditionsProp()
//*	Mar  3,	2023	<MLS> Added FindDeviceTypeByStringLowerCase()
//*	Mar  9,	2023	<MLS> Added GetDomeShutterStatusString()
//*	Jun 18,	2023	<MLS> Added LookupStringInTable()
//*****************************************************************************

#include	<string.h>
#include	<stdio.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpaca_defs.h"
#include	"alpacadriver_helper.h"


//*******************************************************
TYPE_DeviceTable	gDeviceTable[]	=
{
	{	"Camera",				kDeviceType_Camera				},
	{	"Dome",					kDeviceType_Dome				},
	{	"Filterwheel",			kDeviceType_Filterwheel			},
	{	"Focuser",				kDeviceType_Focuser				},
	{	"Management",			kDeviceType_Management			},
	{	"Observingconditions",	kDeviceType_Observingconditions	},
	{	"Rotator",				kDeviceType_Rotator				},
	{	"SafetyMonitor",		kDeviceType_SafetyMonitor		},
	{	"Switch",				kDeviceType_Switch				},
	{	"Telescope",			kDeviceType_Telescope			},
	{	"CoverCalibrator",		kDeviceType_CoverCalibrator		},

	//*	extras defined by MLS
	{	"Multicam",				kDeviceType_Multicam			},
	{	"Shutter",				kDeviceType_Shutter				},
	{	"SlitTracker",			kDeviceType_SlitTracker			},
	{	"Shutter",				kDeviceType_Shutter				},
	{	"Spectrograph",			kDeviceType_Spectrograph		},

	{	"",						kDeviceType_undefined			}
};

//*******************************************************
TYPE_DeviceTable	gDeviceTableLowerCase[]	=
{
	{	"camera",				kDeviceType_Camera				},
	{	"dome",					kDeviceType_Dome				},
	{	"filterwheel",			kDeviceType_Filterwheel			},
	{	"focuser",				kDeviceType_Focuser				},
	{	"management",			kDeviceType_Management			},
	{	"observingconditions",	kDeviceType_Observingconditions	},
	{	"rotator",				kDeviceType_Rotator				},
	{	"safetyMonitor",		kDeviceType_SafetyMonitor		},
	{	"switch",				kDeviceType_Switch				},
	{	"telescope",			kDeviceType_Telescope			},
	{	"covercalibrator",		kDeviceType_CoverCalibrator		},

	//*	extras defined by MLS
	{	"multicam",				kDeviceType_Multicam			},
	{	"shutter",				kDeviceType_Shutter				},
	{	"slittracker",			kDeviceType_SlitTracker			},
	{	"shutter",				kDeviceType_Shutter				},
	{	"spectrograph",			kDeviceType_Spectrograph		},

	{	"",						kDeviceType_undefined			}
};

//*****************************************************************************
void	GetDeviceTypeFromEnum(const int deviceEnum, char *deviceTypeString)
{
int		ii;
	ii	=	0;
	while ((gDeviceTable[ii].deviceTypeStr[0] > 0x20 ) && (gDeviceTable[ii].enumValue >= 0))
	{
		if (deviceEnum == gDeviceTable[ii].enumValue)
		{
			strcpy(deviceTypeString, gDeviceTable[ii].deviceTypeStr);
		}
		ii++;
	}
}

//*****************************************************************************
TYPE_DEVICETYPE	FindDeviceTypeByString(const char *deviceTypeStr)
{
int					ii;
TYPE_DEVICETYPE		enumValue;

	enumValue	=	kDeviceType_undefined;
	ii				=	0;
	while ((gDeviceTable[ii].deviceTypeStr[0] != 0) && (enumValue < 0))
	{
		if (strcasecmp(deviceTypeStr, gDeviceTable[ii].deviceTypeStr) == 0)
		{
			enumValue	=	gDeviceTable[ii].enumValue;
		}
		ii++;
	}
	return(enumValue);
}

//*****************************************************************************
//*	this is strict lower case to meet alpaca standards
//*****************************************************************************
TYPE_DEVICETYPE	FindDeviceTypeByStringLowerCase(const char *deviceTypeStr)
{
int					ii;
TYPE_DEVICETYPE		enumValue;

	enumValue	=	kDeviceType_undefined;
	ii				=	0;
	while ((gDeviceTableLowerCase[ii].deviceTypeStr[0] != 0) && (enumValue < 0))
	{
		if (strcmp(deviceTypeStr, gDeviceTableLowerCase[ii].deviceTypeStr) == 0)
		{
			enumValue	=	gDeviceTableLowerCase[ii].enumValue;
		}
		ii++;
	}
	return(enumValue);
}

//*****************************************************************************
int	LookupStringInTable(const char *lookupString, TYPE_LookupTable *lookupTable)
{
int		iii;
int		enumValue;

//	CONSOLE_DEBUG_W_STR("lookupString\t=", lookupString);
	enumValue	=	-1;
	iii			=	0;
	while ((lookupTable[iii].keyword[0] != 0) && (enumValue < 0))
	{
		if (strcasecmp(lookupString, lookupTable[iii].keyword) == 0)
		{
			enumValue	=	lookupTable[iii].enumValue;
		}
		iii++;
	}
	return(enumValue);
}

//*****************************************************************************
int	LookupStringInCmdTable(const char *lookupString, TYPE_CmdEntry *commandTable)
{
int		iii;
int		enumValue;

//	CONSOLE_DEBUG_W_STR("lookupString\t=", lookupString);
	enumValue	=	-1;
	iii			=	0;
	while ((commandTable[iii].commandName[0] != 0) && (enumValue < 0))
	{
		if (strcasecmp(lookupString, commandTable[iii].commandName) == 0)
		{
			enumValue	=	commandTable[iii].enumValue;
		}
		iii++;
	}
	return(enumValue);
}

//*****************************************************************************
void	GetDomeShutterStatusString(const int status, char *statusString)
{
	switch(status)
	{
		case kShutterStatus_Open:		strcpy(statusString,	"Open");	break;
		case kShutterStatus_Closed:		strcpy(statusString,	"Closed");	break;
		case kShutterStatus_Opening:	strcpy(statusString,	"Opening");	break;
		case kShutterStatus_Closing:	strcpy(statusString,	"Closing");	break;
		case kShutterStatus_Error:		strcpy(statusString,	"Error");	break;

		case kShutterStatus_Unknown:
		default:						strcpy(statusString,	"unknown");	break;
	}
}


//*****************************************************************************
void	GetBinaryElementTypeString(const int elementType, char *typeString)
{
	switch(elementType)
	{
		case kAlpacaImageData_Unknown:	strcpy(typeString,	"Unknown");	break;
		case kAlpacaImageData_Int16:	strcpy(typeString,	"Int16");	break;
		case kAlpacaImageData_Int32:	strcpy(typeString,	"Int32");	break;
		case kAlpacaImageData_Double:	strcpy(typeString,	"Double");	break;
		case kAlpacaImageData_Single:	strcpy(typeString,	"Single");	break;
		case kAlpacaImageData_Decimal:	strcpy(typeString,	"Decimal");	break;
		case kAlpacaImageData_Byte:		strcpy(typeString,	"Byte");	break;
		case kAlpacaImageData_Int64:	strcpy(typeString,	"Int64");	break;
		case kAlpacaImageData_UInt16:	strcpy(typeString,	"UInt16");	break;
		default:						strcpy(typeString,	"Unknown");	break;
	}
}


//*****************************************************************************
void	DumpObservingconditionsProp(TYPE_ObsConditionProperties	*obsCondProp, const char *callingFunctionName)
{
char	titleLine[128];

	CONSOLE_DEBUG(		"*************************************************************");
	CONSOLE_DEBUG(		"************** Observingconditions properties ***************");
	sprintf(titleLine,	"************* Called from: %-20s *************", callingFunctionName);
	CONSOLE_DEBUG(titleLine);
	CONSOLE_DEBUG(		"*************************************************************");


	CONSOLE_DEBUG_W_DBL(	"obsCondProp->Averageperiod      \t=",	obsCondProp->Averageperiod.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->CloudCover         \t=",	obsCondProp->CloudCover.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->Dewpoint           \t=",	obsCondProp->DewPoint.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->Humidity           \t=",	obsCondProp->Humidity.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->Pressure_hPa       \t=",	obsCondProp->Pressure.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->RainRate           \t=",	obsCondProp->RainRate.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->SkyBrightness      \t=",	obsCondProp->SkyBrightness.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->SkyQuality         \t=",	obsCondProp->SkyQuality.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->SkyTemperature_DegC\t=",	obsCondProp->SkyTemperature.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->StarFWHM           \t=",	obsCondProp->StarFWHM.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->Temperature_DegC   \t=",	obsCondProp->Temperature.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->WindDirection      \t=",	obsCondProp->WindDirection.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->WindGust           \t=",	obsCondProp->WindGust.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->WindSpeed          \t=",	obsCondProp->WindSpeed.Value);

}
