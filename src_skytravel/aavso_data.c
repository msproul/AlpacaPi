//**************************************************************************
//*	Name:			aavso_data.c
//*
//*	Author:			Mark Sproul (C) 20201
//*					msproul@skychariot.com
//*
//*	Description:	Read AAVSO Target Tool data file
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
//*	References:
//*		https://filtergraph.com/aavso/api/index#
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul msproul@skychariot.com
//*****************************************************************************
//*	Apr 10,	2021	<MLS> Created aavso_data.c
//*	Apr 10,	2021	<MLS> AAVSO TargetTool objects displayed
//*	Apr 11,	2021	<MLS> Working on AAVSO TargetTool read logic
//*****************************************************************************

#include	<string.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<math.h>
#include	<stdbool.h>
#include	<ctype.h>

//*	MLS Libraries
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#include	"SkyStruc.h"
#include	"aavso_data.h"

#define		kAAVSO_starCnt	300

#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))
#define	DEGREES(radians)	((radians) * (180.0 / M_PI))


//************************************************************************
//	#Getting https://filtergraph.com/aavso/api/v1/targets
//	#Data retrieved at:Sun Apr 11 11:48:39 EDT 2021
//
//		{
//			"targets": [
//		{
//			"max_mag": 5.6,
//			"star_name": "CH Cyg",
//			"max_mag_band": "V",
//			"obs_section": ["Alerts / Campaigns"],
//			"var_type": "ZAND+SR",
//			"obs_cadence": 5.0,
//			"last_data_point": 1617245641,
//			"solar_conjunction": false,
//			"other_info": "B and V especially needed
//	[[Alert Notice 639 https://www.aavso.org/aavso-alert-notice-639]]
//	[[Alert Notice 454 https://www.aavso.org/aavso-alert-notice-454]]
//	[[Special Notice #320 https://www.aavso.org/aavso-special-notice-320]]",
//			"period": null,
//			"obs_mode": "All",
//			"observability_times": [["TARGET_RISES",
//			1618202628]],
//			"ra": 291.13779,
//			"min_mag_band": "V",
//			"min_mag": 10.1,
//			"dec": 50.24142,
//			"constellation": "Cyg",
//			"filter": "B",
//			"priority": true},
//************************************************************************
static void ParseOneLineOfJson(char *jsonLine, char *keyWordStr, char *valueStr)
{
int		sLen;
int		iii;
int		ccc;
char	theChar;

	keyWordStr[0]	=	0;
	valueStr[0]		=	0;
	sLen	=	strlen(jsonLine);
	if (sLen > 5)
	{
		if (jsonLine[0] != '[')
		{
			iii			=	0;
			ccc			=	0;
			//*	parse the keyword
			while ((jsonLine[iii] != ':') && (iii < sLen))
			{
				if ((jsonLine[iii] != '{') && (jsonLine[iii] != '"') && (jsonLine[iii] > 0x20))
				{
					if (ccc < 100)
					{
						keyWordStr[ccc++]	=	jsonLine[iii];
						keyWordStr[ccc]		=	0;
					}
				}
				iii++;
			}
			iii++;
			//*	parse the value string
			ccc			=	0;
			while (iii < sLen)
			{
				theChar	=	jsonLine[iii];
			//	if ((theChar != '[') && (theChar != '"') && (theChar != ',') && (theChar >= 0x20))
				if ((theChar != '"') && (theChar != ',') && (theChar >= 0x20))
				{
					if ((ccc == 0) && (theChar == 0x20))
					{
						//*	ignore leading spaces
					}
					else if (ccc < 100)
					{
						valueStr[ccc++]	=	theChar;
						valueStr[ccc]	=	0;
					}
				}
				iii++;
			}
		}
		else
		{
			//*	this is going to be a continuation of the "other info" data
			strcpy(keyWordStr,	"other_info");
			//*	copy the entire line into the value string
			strcpy(valueStr,	jsonLine);
		}
	}
}

//************************************************************************
//*	find the entry in the table, if not found, add it, if there is room
//************************************************************************
static int	FindStarEntry(TYPE_CelestData *targetData, char *starName, int maxEntries)
{
int		foundIdx;
int		iii;
char	myStarName[kLongNameMax + 10];

	strncpy(myStarName, starName, (kLongNameMax - 1));
	myStarName[kLongNameMax -1]	=	0;

	foundIdx	=	-1;
	iii			=	0;
	while ((foundIdx < 0) && (iii < maxEntries) && (targetData[iii].longName[0] != 0))
	{
		if (strcmp(myStarName, targetData[iii].longName) == 0)
		{
			foundIdx	=	iii;
		}
		iii++;
	}
	if ((foundIdx < 0) && (iii < maxEntries))
	{
		foundIdx	=	iii;
	}
	if (foundIdx >= 0)
	{
		strcpy(targetData[foundIdx].longName, myStarName);

		targetData[foundIdx].dataSrc	=	kDataSrc_AAVSOalert;
	}
	else
	{
		CONSOLE_DEBUG("Out of room");
	}
	return(foundIdx);
}

//************************************************************************
static void	ProcessOneLine(char *lineBuff, TYPE_CelestData *currentStarEntry)
{
char		keyWordStr[128];
char		valueStr[128];
double		ra_Degrees;
double		dec_Degrees;
char		*noticePtr;
int			alertID;

	ParseOneLineOfJson(lineBuff, keyWordStr, valueStr);
	if (strcasecmp(keyWordStr, "star_name") == 0)
	{
//		CONSOLE_DEBUG_W_STR("starname\t=", valueStr);
		strncpy(currentStarEntry->longName, valueStr, (kLongNameMax - 1));
		currentStarEntry->longName[kLongNameMax -1]	=	0;
	}
	else if (strcasecmp(keyWordStr, "ra") == 0)
	{
		ra_Degrees	=	atof(valueStr);
		currentStarEntry->org_ra	=	RADIANS(ra_Degrees);
		currentStarEntry->ra		=	RADIANS(ra_Degrees);
	}
	else if (strcasecmp(keyWordStr, "dec") == 0)
	{
		dec_Degrees	=	atof(valueStr);
		currentStarEntry->org_decl	=	RADIANS(dec_Degrees);
		currentStarEntry->decl		=	RADIANS(dec_Degrees);
	}
	else if (strcasecmp(keyWordStr, "max_mag") == 0)
	{
		currentStarEntry->realMagnitude	=		atof(valueStr);
	}
	else if (strcasecmp(keyWordStr, "other_info") == 0)
	{
	//	CONSOLE_DEBUG_W_STR(keyWordStr, valueStr);
	//	printf("other_info=%s\r\n", valueStr);
		noticePtr	=	strstr(valueStr, "notice-");
		if (noticePtr != NULL)
		{
			while ((*noticePtr != '-') && ((*noticePtr != 0)))
			{
				noticePtr++;
			}
			if (*noticePtr == '-')
			{
				noticePtr++;
				alertID	=	atoi(noticePtr);
//				CONSOLE_DEBUG_W_NUM("alertID\t=", alertID);
				if (alertID > currentStarEntry->id)
				currentStarEntry->id	=	alertID;
			}
		}
	}
}


//************************************************************************
TYPE_CelestData	*ReadAAVSO_TargetData(long *objectCount)
{
FILE				*filePointer;
TYPE_CelestData		*targetData;
char				readBuff[128];
char				lineBuff[128];
size_t				bufferSize;
int					lineLength;
int					linesRead;
int					sLen;
int					ccc;
char				filePath[64];
int					alertIdx;
int					maxStarNameLen;
TYPE_CelestData		currentStarEntry;
int					linesForCurrEntry;

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(filePath,	"aavso/");
	strcat(filePath,	"alerts_json.txt");
	CONSOLE_DEBUG_W_STR("Reading:", filePath);

	targetData			=	NULL;
	*objectCount		=	0;
	filePointer			=	fopen(filePath, "r");
	maxStarNameLen		=	0;
	linesForCurrEntry	=	0;
	if (filePointer != NULL)
	{
		bufferSize	=	kAAVSO_starCnt * sizeof(TYPE_CelestData);
		targetData	=	(TYPE_CelestData *)malloc(bufferSize);

		if (targetData != NULL)
		{
			memset(targetData, 0, bufferSize);

			memset(&currentStarEntry, 0, sizeof(TYPE_CelestData));

			linesRead	=	0;
			alertIdx	=	-1;
			while (fgets(readBuff, 100, filePointer) && (alertIdx < kAAVSO_starCnt))
			{
				linesRead++;
				lineLength	=	strlen(readBuff);
				if (lineLength > 0)
				{

					//*	get rid of leading spaces
					ccc		=	0;
					sLen	=	strlen(readBuff);
					while (((readBuff[ccc] == 0x20) || (readBuff[ccc] == 0x09)) && (ccc < sLen))
					{
						ccc++;
					}
					strcpy(lineBuff, &readBuff[ccc]);

					//*	get rid of trailing cr/lf
					sLen	=	strlen(lineBuff);
					while ((sLen > 0) && (lineBuff[sLen -1] < 0x20))
					{
						lineBuff[sLen -1]	=	0;
						sLen--;
					}

					if (lineBuff[0] == '#')
					{
						//*	its a comment, ignore it
					}
					else if (lineBuff[0] == '{')
					{
						//*	we have the start of a new entry, take care of the old one
						if (linesForCurrEntry > 10)
						{
							alertIdx	=	FindStarEntry(targetData, currentStarEntry.longName, kAAVSO_starCnt);
							if (alertIdx >= 0)
							{
								//*	copy the data to our array
								targetData[alertIdx]			=	currentStarEntry;
								targetData[alertIdx].dataSrc	=	kDataSrc_AAVSOalert;
							}
							else
							{
								CONSOLE_DEBUG("No room in table");
							}
						}
						else
						{
							CONSOLE_DEBUG_W_STR("Too early:", lineBuff);
						}

						linesForCurrEntry	=	0;
						memset(&currentStarEntry, 0, sizeof(TYPE_CelestData));
					}
					else
					{
						ProcessOneLine(lineBuff, &currentStarEntry);
						linesForCurrEntry++;
					}

				}
			}
			*objectCount	=	alertIdx;
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read:", filePath);
	}
	CONSOLE_DEBUG_W_NUM("linesRead		\t=",	linesRead);
	CONSOLE_DEBUG_W_NUM("maxStarNameLen\t=",	maxStarNameLen);
	CONSOLE_DEBUG_W_NUM("total alerts found\t=",	alertIdx);

//	CONSOLE_ABORT(__FUNCTION__);

	return(targetData);
}
