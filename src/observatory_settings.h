//**************************************************************************
//#include	"observatory_settings.h"

#ifndef _OBSERVATORY_SETTINGS_H_
#define	_OBSERVATORY_SETTINGS_H_


#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif // _STDBOOL_H

#ifdef __cplusplus
	extern "C" {
#endif

#define	kMaxCommentLen	80
#define	kMaxComments	5
#define	kMaxTelescopes	15
//**************************************************************************
typedef struct
{
	char	text[kMaxCommentLen];
} TYPE_COMMENT;


#define	kTelescopeDefMaxStrLen	48
#define	kInstrumentNameMaxLen	64
#define	kMaxRefIDLen			16
//**************************************************************************
//*	Data about telescope in use, multiple configurations supported
typedef struct
{
	char			refID[kMaxRefIDLen];
	double			aperature_mm;
	double			secondary_mm;		//	Secondary is the diameter of the secondary mirror for area calculations in FITS header
	double			focalLen_mm;
	char			telescp_manufacturer[kTelescopeDefMaxStrLen];
	char			telescp_model[kTelescopeDefMaxStrLen];
	char			focuser[kTelescopeDefMaxStrLen];
	bool			hasFilterwheel;
	char			filterwheel[kTelescopeDefMaxStrLen];
	char			filterName[kTelescopeDefMaxStrLen];		//*	only used if filterwheel is NOT being controlled
	char			instrument[kInstrumentNameMaxLen];
	TYPE_COMMENT	comments[kMaxComments];

} TYPE_TELESCOPE_INFO;

//**************************************************************************
typedef struct
{
	bool				ValidInfo;
	char				RefID[kMaxRefIDLen];
	char				Name[64];
	char				Location[48];
	char				Website[64];
	char				AAVSO_ObserverID[16];		//*	assigned from AAVSO (text string)
	char				Configuration[32];			//*	dome, roll off, etc (text string)
	char				Owner[64];
	char				Email[64];
	char				Observer[64];
	char				TimeZone[16];
	int					UTCoffset;


	bool				ValidLatLon;
	char				ElevationString[48];		//*	feet above sea level
	char				LatString[48];
	char				LonString[48];
	double				Elevation_ft;				//*	feet above sea level
	double				Elevation_m;				//*	meters above sea level
	double				Latitude;					//*	degrees
	double				Longitude;					//*	degrees
	TYPE_TELESCOPE_INFO	TS_info[kMaxTelescopes];

} TYPE_OBSERVATORY_SETTINGS;

extern TYPE_OBSERVATORY_SETTINGS	gObseratorySettings;


void	ObservatorySettings_Init(void);
void	ObservatorySettings_CreateTemplateFile(void);
void	ObservatorySettings_ReadFile(void);
void	GetTelescopeSettingsByRefID(const char *refID, int argIndex, TYPE_TELESCOPE_INFO *ts_info);
void	OutPutObservatoryInfoHTML(int socketFD);


#ifdef __cplusplus
}
#endif


#endif // _OBSERVATORY_SETTINGS_H_
