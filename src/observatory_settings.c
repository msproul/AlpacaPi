//**************************************************************************
//*	Name:			observatory_settings.c
//*
//*	Author:			Mark Sproul (C) 2019, 2020
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
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
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Jun 10,	2019	<MLS> Started on observatory_settings.c
//*	Jun 10,	2019	<MLS> Added FindKeywordFromTable()
//*	Nov  4,	2019	<MLS> Added LatString / LonString to structure
//*	Nov  9,	2019	<MLS> Added EvaluateLatLonString()
//*	Nov 10,	2019	<MLS> Added Concept of telescope settings group
//*	Nov 10,	2019	<MLS> Added GetTelescopeSettingsByRefID()
//*	Nov 11,	2019	<MLS> Added Comments to telescope info block
//*	Nov 12,	2019	<MLS> Added website to observatory info
//*	Nov 16,	2019	<MLS> Added hasFilterwheel flag
//*	Nov 17,	2019	<MLS> Added filter, used when filter wheel is NOT used
//*	Dec  9.	2019	<MLS> Added secondary diameter to telescope settings
//*	Dec 16,	2019	<MLS> Added email to observatory settings
//*	Dec 16,	2019	<MLS> Added ObservatorySettings_CreateTemplateFile()
//*	Mar 18,	2021	<MLS> Added better error message if create template fails
//*	Apr  6,	2022	<MLS> Switched to using generic config file read routines
//*	Mar  2,	2023	<MLS> Added DumpObservatorySettings()
//*	Mar  2,	2023	<MLS> Made sure RefID had something in it if available
//*****************************************************************************

#include	<errno.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"eventlogging.h"
#include	"linuxerrors.h"
#include	"readconfigfile.h"
#include	"helper_functions.h"
#include	"controller_startup.h"

#include	"alpacadriver_helper.h"
#include	"observatory_settings.h"


TYPE_OBSERVATORY_SETTINGS	gObseratorySettings;


//*****************************************************************************
enum
{
	kObservatory_aaa	=	0,
	//*	order does not matter, using alphabetic for readability
	kObservatory_AAVSO_ObserverID,		//*	assigned from AAVSO (text string)
	kObservatory_Configuration,			//*	dome, roll off, etc (text string)
	kObservatory_Elevation,				//*	feet above sea level
	kObservatory_Location,
	kObservatory_Latitude,
	kObservatory_Longitude,
	kObservatory_Name,
	kObservatory_Owner,
	kObservatory_Email,
	kObservatory_Observer,
	kObservatory_TimeZone,
	kObservatory_Website,
	kObservatory_UTCoffset,

	//*	alphabetical order not required
	kObservatory_Aperture,
	kObservatory_Comment,
	kObservatory_FilterName,
	kObservatory_Filterwheel,
	kObservatory_FocalLength,
	kObservatory_Focuser,
	kObservatory_Instrument,
	kObservatory_Manufacturer,
	kObservatory_Model,
	kObservatory_RefID,
	kObservatory_Secondary,
	kObservatory_TelescopeNum,

	kObservatory_ignored,

	kObservatory_last
};


//**************************************************************************
const TYPE_KEYWORDS	gSettingsList[]	=
{
	//*	the table is all lower case, case does not mater in the file
	{	"name",				kObservatory_Name				},
	{	"owner",			kObservatory_Owner				},
	{	"email",			kObservatory_Email				},
	{	"observer",			kObservatory_Observer			},
	{	"configuration",	kObservatory_Configuration		},
	{	"location",			kObservatory_Location			},
	{	"latitude",			kObservatory_Latitude			},
	{	"longitude",		kObservatory_Longitude			},
	{	"elevation",		kObservatory_Elevation			},
	{	"website",			kObservatory_Website			},
	{	"timezone",			kObservatory_TimeZone			},
	{	"utcoffset",		kObservatory_UTCoffset			},
	{	"aavso_observerid",	kObservatory_AAVSO_ObserverID	},

	//*	these are telescope specific

	{	"####################",						kObservatory_ignored			},
	{	"#telescope specific",						kObservatory_ignored			},
	{	"#up to N entries, numbered 1-N",			kObservatory_ignored			},
	{	"####################",						kObservatory_ignored			},
	{	"telescopenum",								kObservatory_TelescopeNum		},
	{	"refid",									kObservatory_RefID				},
	{	"focallength",								kObservatory_FocalLength		},
	{	"aperture",									kObservatory_Aperture			},
	{	"secondary",								kObservatory_Secondary			},
	{	"filter",									kObservatory_FilterName			},
	{	"filterwheel",								kObservatory_Filterwheel		},
	{	"focuser",									kObservatory_Focuser			},
	{	"instrument",								kObservatory_Instrument			},
	{	"manufacturer",								kObservatory_Manufacturer		},
	{	"model",									kObservatory_Model				},
	{	"#up to N comments per telescope entry",	kObservatory_ignored			},
	{	"comment",									kObservatory_Comment			},


	{	"",					-1								}

};

static int	gCurrTelescopeNum	=	0;
static int	gCurrCommentIdx		=	0;

//*****************************************************************************
void	ObservatorySettings_Init(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	memset(&gObseratorySettings, 0, sizeof(TYPE_OBSERVATORY_SETTINGS));

}

//*****************************************************************************
void	ObservatorySettings_CreateTemplateFile(void)
{
int		ii;
FILE	*filePointer;
char	separaterLine[]	=	"###############################################################################\n";
char	linuxErrStr[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	filePointer	=	fopen("observatorysettings_template.txt", "w");
	if (filePointer != NULL)
	{
		fprintf(filePointer, "%s",	separaterLine);
		fprintf(filePointer, "#	Modify this file with your information\n");
		fprintf(filePointer, "#	Rename this file to observatorysettings.txt\n");
		fprintf(filePointer, "#	configuration is meant to be DOME/Roll off roof/Portable/etc\n");
		fprintf(filePointer, "#	\n");
		fprintf(filePointer, "#	Up to %d telescope configurations (1 -> %d)\n", kMaxTelescopes, kMaxTelescopes);
		fprintf(filePointer, "#	Up to %d comments for each telescope configuration\n", kMaxComments);
		fprintf(filePointer, "#	This information is used primarily in the generation of FITS files\n");
		fprintf(filePointer, "#	All of these fields are optional\n");
		fprintf(filePointer, "#	Elevation is in feet above sea level\n");
		fprintf(filePointer, "#	The telescopenum value can be '+' to bump to the next index\n");
		fprintf(filePointer, "#	\n");
		fprintf(filePointer, "#	Note: this template file gets regenerated every time you run this program\n");
		fprintf(filePointer, "%s",	separaterLine);
		ii			=	0;
		while (gSettingsList[ii].keyword[0] != 0)
		{
			if (strncmp(gSettingsList[ii].keyword, "###", 3) == 0)
			{
				//*	print out a separator line
				fprintf(filePointer, "%s",	separaterLine);
			}
			else if (gSettingsList[ii].keyword[0] == '#')
			{
				//*	its a comment line, print it out as it is
				fprintf(filePointer, "%s\n", gSettingsList[ii].keyword);
			}
			else
			{
				//*	format the keyword value pair
				fprintf(filePointer, "%-30s\t=\t\n", gSettingsList[ii].keyword);
			}
			ii++;
		}
		fclose(filePointer);
	}
	else
	{
		//*	something went wrong, we failed to create the file
		GetLinuxErrorString(errno, linuxErrStr);
		CONSOLE_DEBUG_W_STR("Failed to create template file:", linuxErrStr);
	}
}

//*****************************************************************************
static double	EvaluateLatLonString(const char *latlonString)
{
double	latLonValue;
char	colonCnt;
char	dotCnt;
char	spaceCnt;
int		sLen;
int		ii;

	latLonValue	=	0.0;
	colonCnt	=	0;
	dotCnt		=	0;
	spaceCnt	=	0;
	sLen		=	strlen(latlonString);
	for (ii=0; ii<sLen; ii++)
	{
		switch(latlonString[ii])
		{
			case ':':		colonCnt++;	break;
			case '.':		dotCnt++;	break;
			case 0x20:		spaceCnt++;	break;
		}
	}
	if ((dotCnt == 1) && (colonCnt == 0))
	{
		latLonValue	=	AsciiToDouble(latlonString);
	}
	else if (colonCnt == 2)
	{

	}
	return(latLonValue);
}

//*****************************************************************************
static void	ProcessObsSettingsConfig(const char *keyword, const char *valueString, void *userDataPtr)
{
int					keywordEnumVal;
TYPE_TELESCOPE_INFO	*ts_infoPtr;

//	CONSOLE_DEBUG_W_STR(keyword, valueString);

	//*	this is kind of redundant to do it EVERY time, but it is the easiest to insure
	//*	the telescope index is not out of bounds
	if ((gCurrTelescopeNum < 0) || (gCurrTelescopeNum >= kMaxTelescopes))
	{
		CONSOLE_DEBUG("Resetting gCurrTelescopeNum to 0");
		gCurrTelescopeNum	=	0;
	}
	//*	this is for the telescope specific information
	ts_infoPtr	=	&gObseratorySettings.TS_info[gCurrTelescopeNum];

	//*	look up the keyword in the table
	keywordEnumVal	=	FindKeywordFromTable(keyword, gSettingsList);
	if (keywordEnumVal >= 0)
	{
		gObseratorySettings.KeyWordCount++;
	}
	//*	use the keyword enum value to figure out what to do with the data
	switch(keywordEnumVal)
	{
		case kObservatory_AAVSO_ObserverID:
			strcpy(gObseratorySettings.AAVSO_ObserverID,	valueString);
			break;

		case kObservatory_Configuration:
			strcpy(gObseratorySettings.Configuration,		valueString);
			break;

		case kObservatory_Email:
			strcpy(gObseratorySettings.Email,				valueString);
			break;

		case kObservatory_Location:
			strcpy(gObseratorySettings.Location,			valueString);
			break;

		case kObservatory_Elevation:
			strcpy(gObseratorySettings.ElevationString,			valueString);
			gObseratorySettings.Elevation_ft	=	AsciiToDouble(valueString);
			gObseratorySettings.Elevation_m		=	(gObseratorySettings.Elevation_ft * 12.0) * 0.0254;
			break;

		case kObservatory_Latitude:
			strcpy(gObseratorySettings.LatString,			valueString);
			gObseratorySettings.Latitude_deg	=	EvaluateLatLonString(valueString);
			gObseratorySettings.ValidLatLon		=	true;
			break;

		case kObservatory_Longitude:
			strcpy(gObseratorySettings.LonString,			valueString);
			gObseratorySettings.Longitude_deg	=	EvaluateLatLonString(valueString);
			gObseratorySettings.ValidLatLon		=	true;
			break;

		case kObservatory_Name:
			strcpy(gObseratorySettings.Name,		valueString);
			break;

		case kObservatory_Website:
			strcpy(gObseratorySettings.Website,		valueString);
			break;

		case kObservatory_Owner:
			strcpy(gObseratorySettings.Owner,		valueString);
			break;

		case kObservatory_Observer:
			strcpy(gObseratorySettings.Observer,	valueString);
			break;

		case kObservatory_TimeZone:
			strcpy(gObseratorySettings.TimeZone,	valueString);
			break;

		case kObservatory_UTCoffset:
			gObseratorySettings.UTCoffset		=	atoi(valueString);
			break;

		//--------------------------------------------------------------
		//*	these are telescope specific attributes
		case kObservatory_Aperture:
			ts_infoPtr->aperature_mm			=	AsciiToDouble(valueString);
			break;

		case kObservatory_Secondary:
			ts_infoPtr->secondary_mm			=	AsciiToDouble(valueString);
			break;


		case kObservatory_Comment:
			if (gCurrCommentIdx < kMaxComments)
			{
				//*	insure the string is not to long
	//			valueString[kMaxCommentLen - 2]	=	0;
	//			strncpyZero(ts_infoPtr->comments[gCurrCommentIdx].text,	valueString, (kMaxCommentLen - 2));
				strncpy(ts_infoPtr->comments[gCurrCommentIdx].text,	valueString, kMaxCommentLen);
				ts_infoPtr->comments[gCurrCommentIdx].text[kMaxCommentLen - 2]	=	0;

				gCurrCommentIdx++;
			}
			else
			{
				CONSOLE_DEBUG_W_STR("Exceeded comment limit", valueString)
			}
			break;

		case kObservatory_Filterwheel:
			strcpy(ts_infoPtr->filterwheel,			valueString);
			if (strlen(ts_infoPtr->filterwheel) > 0)
			{
				ts_infoPtr->hasFilterwheel	=	true;
			}
			break;

		case kObservatory_FilterName:
			strcpy(ts_infoPtr->filterName,				valueString);
			break;

		case kObservatory_FocalLength:
			ts_infoPtr->focalLen_mm				=	AsciiToDouble(valueString);
			break;

		case kObservatory_Focuser:
			strcpy(ts_infoPtr->focuser,					valueString);
			break;

		case kObservatory_Instrument:
			strcpy(ts_infoPtr->instrument,				valueString);
			break;

		case kObservatory_Manufacturer:
			strcpy(ts_infoPtr->telescp_manufacturer,	valueString);
			break;

		case kObservatory_Model:
			strcpy(ts_infoPtr->telescp_model,			valueString);
			break;

		case kObservatory_RefID:
			strcpy(ts_infoPtr->refID,					valueString);
			if (strlen(gObseratorySettings.RefID) == 0)
			{
				//*	this will set the RefID to the first one found in the file.
				//*	it can be reset later by GetTelescopeSettingsByRefID()
				strcpy(gObseratorySettings.RefID, valueString);
			}
			break;

		case kObservatory_TelescopeNum:
			if (valueString[0] == '+')
			{
				//*	just increment to the next one
				gCurrTelescopeNum++;
			}
			else
			{
				gCurrTelescopeNum		=	atoi(valueString);
				gCurrTelescopeNum--;	//*	adjust for zero base
			}
			if (gCurrTelescopeNum < 0)
			{
				gCurrTelescopeNum	=	0;
			}
			gCurrCommentIdx	=	0;
			break;

		case kObservatory_ignored:
			//*	ignored
			break;
	}
}

#ifndef _ENABLE_SKYTRAVEL_
//*****************************************************************************
int		SetStartupText(const char *textMsg)	{ return(1);}
void	SetStartupTextStatus(const int idx, const char *textMsg)	{ }
#endif // _ENABLE_SKYTRAVEL_

//*****************************************************************************
bool	ObservatorySettings_ReadFile(void)
{
bool	configFileOK;
int		linesRead;
int		startupWidgetIdx;

//	CONSOLE_DEBUG(__FUNCTION__);

	startupWidgetIdx	=	SetStartupText("Observatory settings:");

	ObservatorySettings_Init();
	configFileOK	=	false;
	linesRead		=	ReadGenericConfigFile("observatorysettings.txt", '=', &ProcessObsSettingsConfig, NULL);
	if (linesRead > 5)
	{
		configFileOK	=	true;
		SetStartupTextStatus(startupWidgetIdx, "OK");
	}
	else
	{
		ObservatorySettings_CreateTemplateFile();
		SetStartupTextStatus(startupWidgetIdx, "Failed");
	}

	if (gObseratorySettings.KeyWordCount > 10)
	{
		gObseratorySettings.ValidInfo	=	true;
	}
	else
	{
		ObservatorySettings_CreateTemplateFile();
		SetStartupTextStatus(startupWidgetIdx, "Not enough data");
	}

//	DumpObservatorySettings();
//	CONSOLE_ABORT(__FUNCTION__);

	return(configFileOK);
}

//*****************************************************************************
void	DumpObservatorySettings(void)
{
	CONSOLE_DEBUG(		"*************************************************************");
	CONSOLE_DEBUG(		"*******************  Obseratory Settings ********************");
//	sprintf(titleLine,	"************* Called from: %-20s *************", callingFunctionName);
//	CONSOLE_DEBUG(titleLine);
	CONSOLE_DEBUG(		"*************************************************************");

	CONSOLE_DEBUG_W_BOOL(	"ValidInfo       \t=",	gObseratorySettings.ValidInfo);
	CONSOLE_DEBUG_W_NUM(	"KeyWordCount    \t=",	gObseratorySettings.KeyWordCount);
	CONSOLE_DEBUG_W_STR(	"RefID           \t=",	gObseratorySettings.RefID);
	CONSOLE_DEBUG_W_STR(	"Name            \t=",	gObseratorySettings.Name);
	CONSOLE_DEBUG_W_STR(	"Location        \t=",	gObseratorySettings.Location);
	CONSOLE_DEBUG_W_STR(	"Website         \t=",	gObseratorySettings.Website);
	CONSOLE_DEBUG_W_STR(	"AAVSO_ObserverID\t=",	gObseratorySettings.AAVSO_ObserverID);
	CONSOLE_DEBUG_W_STR(	"Configuration   \t=",	gObseratorySettings.Configuration);
	CONSOLE_DEBUG_W_STR(	"Owner           \t=",	gObseratorySettings.Owner);
	CONSOLE_DEBUG_W_STR(	"Email           \t=",	gObseratorySettings.Email);
	CONSOLE_DEBUG_W_STR(	"Observer        \t=",	gObseratorySettings.Observer);
	CONSOLE_DEBUG_W_STR(	"TimeZone        \t=",	gObseratorySettings.TimeZone);
//	int					UTCoffset;

//	bool				ValidLatLon;
	CONSOLE_DEBUG_W_STR(	"ElevationString \t=",	gObseratorySettings.ElevationString);
	CONSOLE_DEBUG_W_STR(	"LatString       \t=",	gObseratorySettings.LatString);
	CONSOLE_DEBUG_W_STR(	"LonString       \t=",	gObseratorySettings.LonString);
//	double				Elevation_ft;				//*	feet above sea level
//	double				Elevation_m;				//*	meters above sea level
//	double				Latitude_deg;				//*	degrees
//	double				Longitude_deg;				//*	degrees

	DumpTelescopeInfo(gObseratorySettings.TS_info);
}

//*****************************************************************************
void	DumpTelescopeInfo(TYPE_TELESCOPE_INFO *ts_info)
{
	CONSOLE_DEBUG(		"*************************************************************");
	CONSOLE_DEBUG_W_STR("refID\t\t\t=",					ts_info->refID);
	CONSOLE_DEBUG_W_STR("telescp_manufacturer\t=",		ts_info->telescp_manufacturer);
	CONSOLE_DEBUG_W_STR("telescp_model\t\t=",			ts_info->telescp_model);
	CONSOLE_DEBUG_W_DBL("aperature_mm\t\t=",			ts_info->aperature_mm);
	CONSOLE_DEBUG_W_DBL("secondary_mm\t\t=",			ts_info->secondary_mm);
	CONSOLE_DEBUG_W_DBL("focalLen_mm\t\t=",				ts_info->focalLen_mm);
	CONSOLE_DEBUG_W_STR("filterwheel\t\t=",				ts_info->filterwheel);
}

//*****************************************************************************
//*	refID is a string that was specified in the settings file.
//*	refID can be NULL and if it is not found, settings for scope 0 are set
//*	Optionally you can specify the index, in which case refID must be NULL
//*****************************************************************************
void	GetTelescopeSettingsByRefID(	const char			*refID,		//*	can be NULL
										int					argIndex,	//*	ignored if refID is not null
										TYPE_TELESCOPE_INFO *ts_info)
{
int		telescopeIdx;
int		ii;

	CONSOLE_DEBUG(__FUNCTION__);

	telescopeIdx	=	-1;	//*	set the default
	if (refID != NULL)
	{
		CONSOLE_DEBUG_W_STR("refID\t\t=",	refID);
		for (ii=0; ii<kMaxTelescopes; ii++)
		{
			if (strcasecmp(refID, gObseratorySettings.TS_info[ii].refID) == 0)
			{
				telescopeIdx	=	ii;
				break;
			}
		}
		if (telescopeIdx < 0)
		{
			CONSOLE_DEBUG_W_STR("Telescope refID not found", refID);
		}
	}
	else
	{
		telescopeIdx	=	argIndex;
	}

	//================================================================
	if ((telescopeIdx < 0) || (telescopeIdx >= kMaxTelescopes))
	{
		telescopeIdx	=	0;
	}
//	CONSOLE_DEBUG_W_NUM("telescopeIdx\t=",	telescopeIdx);

	//*	set the global RefID
	strcpy(gObseratorySettings.RefID, gObseratorySettings.TS_info[telescopeIdx].refID);

	if (ts_info != NULL)
	{
		//*	copy the entire block over
		*ts_info	=	gObseratorySettings.TS_info[telescopeIdx];
		DumpTelescopeInfo(ts_info);
//		CONSOLE_ABORT(__FUNCTION__);

	#ifndef _ENABLE_SKYTRAVEL_
		if (strlen(ts_info->telescp_manufacturer) > 0)
		{
			strcpy(gWebTitle, ts_info->telescp_manufacturer);
			strcat(gWebTitle, " ");
			strcat(gWebTitle, ts_info->telescp_model);
		}
	#endif
	}
}

#ifndef _ENABLE_SKYTRAVEL_
//*****************************************************************************
void OutPutObservatoryInfoHTML(int socketFD)
{
	if (gObseratorySettings.ValidInfo)
	{
		SocketWriteData(socketFD,	"<CENTER>\r\n");
		if (strlen(gObseratorySettings.Name) > 0)
		{
			SocketWriteData(socketFD,	"<H1>");
			SocketWriteData(socketFD,	gObseratorySettings.Name);
			SocketWriteData(socketFD,	" Observatory</H1>\r\n");
		}
		if (strlen(gObseratorySettings.Owner) > 0)
		{
			SocketWriteData(socketFD,	gObseratorySettings.Owner);
			SocketWriteData(socketFD,	"<BR>\r\n");
			SocketWriteData(socketFD,	"<BR>\r\n");
		}
		if (strlen(gObseratorySettings.Location) > 0)
		{
			SocketWriteData(socketFD,	gObseratorySettings.Location);
			SocketWriteData(socketFD,	"<BR>\r\n");
		}
		if (strlen(gObseratorySettings.Website) > 0)
		{
			SocketWriteData(socketFD,	"<A HREF=");
			SocketWriteData(socketFD,	gObseratorySettings.Website);
			SocketWriteData(socketFD,	">");
			SocketWriteData(socketFD,	gObseratorySettings.Website);
			SocketWriteData(socketFD,	"</A>\r\n");
		}

		SocketWriteData(socketFD,	"</CENTER>\r\n");
	}
}
#endif // _ENABLE_SKYTRAVEL_


