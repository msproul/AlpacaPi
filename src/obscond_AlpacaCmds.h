//*****************************************************************************
//*	Jun 28,	2023	<MLS> Created obscond_AlpacaCmds.h
//*****************************************************************************
//#include	"obscond_AlpacaCmds.h"

#ifndef _OBSCOND_ALPACA_CMDS_H_
#define _OBSCOND_ALPACA_CMDS_H_

extern	TYPE_CmdEntry	gObsCondCmdTable[];

//*****************************************************************************
//*	ObservingConditions
//*****************************************************************************
enum
{

	kCmd_ObservCond_averageperiod	=	0,	//*	Returns the time period over which observations will be averaged
											//*	Sets the time period over which observations will be averaged
	kCmd_ObservCond_cloudcover,				//*	Returns the amount of sky obscured by cloud
	kCmd_ObservCond_dewpoint,				//*	Returns the atmospheric dew point at the observatory
	kCmd_ObservCond_humidity,				//*	Returns the atmospheric humidity at the observatory
	kCmd_ObservCond_pressure,				//*	Returns the atmospheric pressure at the observatory.
	kCmd_ObservCond_rainrate,				//*	Returns the rain rate at the observatory.
	kCmd_ObservCond_skybrightness,			//*	Returns the sky brightness at the observatory
	kCmd_ObservCond_skyquality,				//*	Returns the sky quality at the observatory
	kCmd_ObservCond_skytemperature,			//*	Returns the sky temperature at the observatory
	kCmd_ObservCond_starfwhm,				//*	Returns the seeing at the observatory
	kCmd_ObservCond_temperature,			//*	Returns the temperature at the observatory
	kCmd_ObservCond_winddirection,			//*	Returns the wind direction at the observatory
	kCmd_ObservCond_windgust,				//*	Returns the peak 3 second wind gust at the observatory over the last 2 minutes
	kCmd_ObservCond_windspeed,				//*	Returns the wind speed at the observatory.
	kCmd_ObservCond_refresh,				//*	Refreshes sensor values from hardware.
	kCmd_ObservCond_sensordescription,		//*	Return a sensor description
	kCmd_ObservCond_timesincelastupdate,	//*	Return the time since the sensor value was last updated

	//=================================================================
	//*	commands added that are not part of Alpaca
	//*	added by MLS
	kCmd_ObservCond_Extras,
	kCmd_ObservCond_readall
};

#endif // _OBSCOND_ALPACA_CMDS_H_
