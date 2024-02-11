//*****************************************************************************
//*	Name:			obscond_AlpacaCmds.cpp
//*
//*	Author:			Mark Sproul (C) 2023
//*
//*	Description:	Command table for obscond
//*					This file is used by both the driver and the controller
//*****************************************************************************
//*	Jul  1,	2023	<MLS> Created obscond_AlpacaCmds.cpp
//*****************************************************************************


#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif

#ifndef _OBSCOND_ALPACA_CMDS_H_
	#include	"obscond_AlpacaCmds.h"
#endif



//*****************************************************************************
TYPE_CmdEntry	gObsCondCmdTable[]	=
{
	{	"averageperiod",		kCmd_ObservCond_averageperiod,			kCmdType_BOTH	},
	{	"cloudcover",			kCmd_ObservCond_cloudcover,				kCmdType_GET	},
	{	"dewpoint",				kCmd_ObservCond_dewpoint,				kCmdType_GET	},
	{	"humidity",				kCmd_ObservCond_humidity,				kCmdType_GET	},
	{	"pressure",				kCmd_ObservCond_pressure,				kCmdType_GET	},
	{	"rainrate",				kCmd_ObservCond_rainrate,				kCmdType_GET	},
	{	"skybrightness",		kCmd_ObservCond_skybrightness,			kCmdType_GET	},
	{	"skyquality",			kCmd_ObservCond_skyquality,				kCmdType_GET	},
	{	"skytemperature",		kCmd_ObservCond_skytemperature,			kCmdType_GET	},
	{	"starfwhm",				kCmd_ObservCond_starfwhm,				kCmdType_GET	},
	{	"temperature",			kCmd_ObservCond_temperature,			kCmdType_GET	},
	{	"winddirection",		kCmd_ObservCond_winddirection,			kCmdType_GET	},
	{	"windgust",				kCmd_ObservCond_windgust,				kCmdType_GET	},
	{	"windspeed",			kCmd_ObservCond_windspeed,				kCmdType_GET	},
	{	"refresh",				kCmd_ObservCond_refresh,				kCmdType_PUT	},
	{	"sensordescription",	kCmd_ObservCond_sensordescription,		kCmdType_GET	},
	{	"timesincelastupdate",	kCmd_ObservCond_timesincelastupdate,	kCmdType_GET	},

	//*	added by MLS
	{	"--extras",				kCmd_ObservCond_Extras,					kCmdType_GET	},
	{	"readall",				kCmd_ObservCond_readall,				kCmdType_GET	},


	{	"",						-1,	0x00	}
};

