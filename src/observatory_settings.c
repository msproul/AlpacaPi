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
//*****************************************************************************

#include	<stdlib.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdbool.h>
#include	<ctype.h>
#include	<stdint.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"eventlogging.h"

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
//	kObservatory_,




	kObservatory_last
};

#define	kMaxKeyWordLen	64
#define	kMaxValueLen	80
//**************************************************************************
typedef struct
{
	char	keyWord[kMaxKeyWordLen];
	int		enumValue;
} TYPE_SETTINGS_DEF;

//**************************************************************************
const TYPE_SETTINGS_DEF	gSettingsList[]	=
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

int	FindKeywordFromTable(const char *theCmd, const TYPE_SETTINGS_DEF *theCmdTable);


//*****************************************************************************
void	ObservatorySettings_Init(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	memset(&gObseratorySettings, 0, sizeof(TYPE_OBSERVATORY_SETTINGS));

	ObservatorySettings_CreateTemplateFile();
}

//*****************************************************************************
void	ObservatorySettings_CreateTemplateFile(void)
{
int		ii;
FILE	*filePointer;
char	separaterLine[]	=	"###############################################################################\n";

	CONSOLE_DEBUG(__FUNCTION__);

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
		while (gSettingsList[ii].keyWord[0] != 0)
		{
			if (strncmp(gSettingsList[ii].keyWord, "###", 3) == 0)
			{
				//*	print out a separator line
				fprintf(filePointer, "%s",	separaterLine);
			}
			else if (gSettingsList[ii].keyWord[0] == '#')
			{
				//*	its a comment line, print it out as it is
				fprintf(filePointer, "%s\n", gSettingsList[ii].keyWord);
			}
			else
			{
				//*	format the keyword value pair
				fprintf(filePointer, "%-30s\t=\t\n", gSettingsList[ii].keyWord);
			}
			ii++;
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create template file");
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
		latLonValue	=	atof(latlonString);
	}
	else if (colonCnt == 2)
	{

	}
	return(latLonValue);
}

//*****************************************************************************
static void	ProcessObservatorySetting(const char *lineBuff)
{
char				keyword[kMaxKeyWordLen];
char				valueString[128];
int					keywordEnumVal;
int					ii;
int					cc;
TYPE_TELESCOPE_INFO	*ts_infoPtr;

	keyword[0]		=	0;
	valueString[0]	=	0;
	ii	=	0;
	cc	=	0;
	//*	pull out the keyword
	while ((lineBuff[ii] != 0) && (lineBuff[ii] != '=') && (ii<100))
	{
		if ((lineBuff[ii] > 0x20) && (cc < kMaxKeyWordLen))
		{
			keyword[cc++]	=	tolower(lineBuff[ii]);
			keyword[cc]	=	0;
		}
		ii++;
	}

	//*	skip white space
	while (((lineBuff[ii] == 0x20) || (lineBuff[ii] == 0x09) || (lineBuff[ii] == '=')) && (ii<100))
	{
		ii++;
	}
	//*	pull out the value string
	cc	=	0;
	while ((lineBuff[ii] != 0) && (ii<100))
	{
		if ((lineBuff[ii] >= 0x20) && (cc < kMaxValueLen))
		{
			valueString[cc++]	=	lineBuff[ii];
			valueString[cc]	=	0;
		}
		ii++;
	}

	keywordEnumVal	=	FindKeywordFromTable(keyword, gSettingsList);

	//*	this is kind of redundant to do it EVERY time, but it is the easiest to insure
	//*	the telescope index is not out of bounds
	if ((gCurrTelescopeNum < 0) || (gCurrTelescopeNum >= kMaxTelescopes))
	{
		CONSOLE_DEBUG("Reseting gCurrTelescopeNum to 0");
		gCurrTelescopeNum	=	0;
	}
	//*	this is for the telescope specific information
	ts_infoPtr	=	&gObseratorySettings.TS_info[gCurrTelescopeNum];

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
			gObseratorySettings.Elevation_ft	=	atof(valueString);
			gObseratorySettings.Elevation_m		=	(gObseratorySettings.Elevation_ft * 12.0) * 0.0254;
			break;

		case kObservatory_Latitude:
			strcpy(gObseratorySettings.LatString,			valueString);
			gObseratorySettings.Latitude		=	EvaluateLatLonString(valueString);
			gObseratorySettings.ValidLatLon		=	true;
			break;

		case kObservatory_Longitude:
			strcpy(gObseratorySettings.LonString,			valueString);
			gObseratorySettings.Longitude		=	EvaluateLatLonString(valueString);
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
			ts_infoPtr->aperature_mm			=	atof(valueString);
			break;

		case kObservatory_Secondary:
			ts_infoPtr->secondary_mm			=	atof(valueString);
			break;


		case kObservatory_Comment:
			if (gCurrCommentIdx < kMaxComments)
			{
				//*	insure the string is not to long
				valueString[kMaxCommentLen - 2]	=	0;
				strcpy(ts_infoPtr->comments[gCurrCommentIdx].text,	valueString);
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
			ts_infoPtr->focalLen_mm				=	atof(valueString);
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
			break;

		case kObservatory_TelescopeNum:
			if (valueString[0] == '+')
			{
				//*	just increment to the next one
				gCurrTelescopeNum++;
			}
			else
			{
				gCurrTelescopeNum			=	atoi(valueString);
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

//*****************************************************************************
void	ObservatorySettings_ReadFile(void)
{
FILE			*filePointer;
char			lineBuff[256];
int				ii;
int				slen;
char			fileName[]	=	"observatorysettings.txt";

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	check for the observatory settings file
	filePointer	=	fopen(fileName, "r");
	if (filePointer != NULL)
	{
#ifndef _ENABLE_SKYTRAVEL_
		LogEvent(	"AlpacaPi",
					"Obs settings file read",
					NULL,
					kASCOM_Err_Success,
					fileName);
#endif // _ENABLE_SKYTRAVEL_

		gObseratorySettings.ValidInfo	=	true;
		while (fgets(lineBuff, 200, filePointer))
		{
			//*	get rid of the trailing CR/LF
			slen	=	strlen(lineBuff);
			for (ii=0; ii<slen; ii++)
			{
				if ((lineBuff[ii] == 0x0d) || (lineBuff[ii] == 0x0a))
				{
					lineBuff[ii]	=	0;
					break;
				}
			}
			slen	=	strlen(lineBuff);
			if ((slen > 3) && (lineBuff[0] != '#'))
			{
				ProcessObservatorySetting(lineBuff);
			}

		}
		fclose(filePointer);
	}
#ifndef __arm__
	CONSOLE_DEBUG(__FUNCTION__);
	ii	=	0;
	while ((strlen(gObseratorySettings.TS_info[ii].refID) > 0) && (ii < kMaxTelescopes))
	{
		printf("%2d\t%s\r\n",	(ii+1), gObseratorySettings.TS_info[ii].refID);
		ii++;
	}
#endif
//	CONSOLE_DEBUG_W_STR("Observer\t\t=",		gObseratorySettings.Observer);
//	CONSOLE_DEBUG_W_STR("AAVSO_ObserverID\t=",	gObseratorySettings.AAVSO_ObserverID);
//	CONSOLE_DEBUG_W_STR("Name\t\t=",			gObseratorySettings.Name);

}

//*****************************************************************************
//*	refID is a string that was specified in the settings file.
//*	refID can be NULL and if it is not found, settings for scope 0 are set
//*	Optionally you can specify the index, in which case refID must be NULL
//*****************************************************************************
void	GetTelescopeSettingsByRefID(	const char			*refID,		//*	can be NULL
										int					argIndex,
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

	if ((telescopeIdx < 0) || (telescopeIdx >= kMaxTelescopes))
	{
		telescopeIdx	=	0;
	}
	CONSOLE_DEBUG_W_NUM("telescopeIdx\t=",	telescopeIdx);

	if (ts_info != NULL)
	{
		//*	copy the entire block over
		*ts_info	=	gObseratorySettings.TS_info[telescopeIdx];
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


//*****************************************************************************
int	FindKeywordFromTable(const char *theCmd, const TYPE_SETTINGS_DEF *theCmdTable)
{
int		ii;
int		keywordEnumValue;

	keywordEnumValue	=	-1;
	ii					=	0;
	while ((theCmdTable[ii].keyWord[0] != 0) && (keywordEnumValue < 0))
	{
		if (strcasecmp(theCmd, theCmdTable[ii].keyWord) == 0)
		{
			keywordEnumValue	=	theCmdTable[ii].enumValue;
		}
		ii++;
	}
	return(keywordEnumValue);
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


