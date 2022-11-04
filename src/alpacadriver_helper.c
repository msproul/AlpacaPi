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
//*	Sep 25,	2022	<MLS> Added (DumpObservingconditionsProp)
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





//*****************************************************************************
void	GetDeviceTypeFromEnum(const int deviceEnum, char *deviceTypeString)
{
int		ii;
	ii	=	0;
	while ((gDeviceTable[ii].deviceType[0] > 0x20 ) && (gDeviceTable[ii].enumValue >= 0))
	{
		if (deviceEnum == gDeviceTable[ii].enumValue)
		{
			strcpy(deviceTypeString, gDeviceTable[ii].deviceType);
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
	while ((gDeviceTable[ii].deviceType[0] != 0) && (enumValue < 0))
	{
		if (strcasecmp(deviceTypeStr, gDeviceTable[ii].deviceType) == 0)
		{
			enumValue	=	gDeviceTable[ii].enumValue;
		}
		ii++;
	}
	return(enumValue);
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
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->Cloudcover         \t=",	obsCondProp->Cloudcover.Value);
	CONSOLE_DEBUG_W_DBL(	"obsCondProp->Dewpoint           \t=",	obsCondProp->Dewpoint.Value);
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
