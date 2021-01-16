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
//*****************************************************************************

#include	<string.h>


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
	{	"SlitTracker",			kDeviceType_SlitTracker			},
	{	"Shutter",				kDeviceType_Shutter				},

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

