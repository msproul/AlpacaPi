//**************************************************************************
//*	Name:			readconfigfile.c
//*
//*	Author:			Mark Sproul (C) 2022
//*
//*	Description:	This is a general purpose config file reader
//*					developed as part of the AlpacaPi project
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
//*	Apr  5,	2022	<MLS> Created readconfigfile.c
//*	Apr  6,	2022	<MLS> Added FindConfigKeywordFromTable()
//*	Sep 20,	2022	<MLS> Fixed bug in ProcessConfigLine() when argument was empty
//*****************************************************************************

#include	<stdbool.h>
#include	<string.h>
#include	<stdio.h>
#include	<stdlib.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"readconfigfile.h"

////*****************************************************************************
//static void	CallBackFunction(const char *keyword, const char *value)
//{
//	CONSOLE_DEBUG_W_STR(keyword, value);
//}

//*****************************************************************************
//*	returns false if the keyword was not valid
//*****************************************************************************
static	bool ProcessConfigLine(	const char			*lineBuff,
								const char			separterChar,
								ProcessConfigEntry	*configCallBack)
{
char	keyword[kMaxKeyWordLen];
char	valueString[kMaxVakyeStrLen];
int		iii;
int		ccc;
int		slen;
char	*valueStrPtr;
bool	validEntry;

	iii			=	0;
	ccc			=	0;
	validEntry	=	true;
	slen		=	strlen(lineBuff);
	//*	find the keyword
	while ((iii<slen) && (ccc < (kMaxKeyWordLen - 1)) &&
			(lineBuff[iii] != separterChar) && (lineBuff[iii] > 0x20))
	{
		keyword[ccc]	=	lineBuff[iii];
		ccc++;
		iii++;
	}
	keyword[ccc]	=	0;

	valueStrPtr	=	strchr(lineBuff, separterChar);
	if (valueStrPtr != NULL)
	{
		valueStrPtr++;		//*	skip over the separterChar
		while ((*valueStrPtr == 0x20) || (*valueStrPtr == 0x09))
		{
			valueStrPtr++;
		}
		strncpy(valueString, valueStrPtr, (kMaxVakyeStrLen-2));
		valueString[kMaxVakyeStrLen-2]	=	0;

		//*	call the supplied callback function
		if (configCallBack != NULL)
		{
			configCallBack(keyword, valueString);
		}
//		CallBackFunction(keyword, valueStrPtr);

	}
	return(	validEntry);
}

//*****************************************************************************
//*	returns # of processed lines
//*	-1 means failed to open config file
//*****************************************************************************
int	ReadGenericConfigFile(	const char	*configFilePath,
							const char	separterChar,
							ProcessConfigEntry *configCallBack
							)
{
FILE	*filePointer;
char	lineBuff[256];
int		iii;
int		slen;
int		valideLineCnt;
bool	validEntry;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, configFilePath);

//	CheckDuplicates();

	valideLineCnt	=	-1;
	//*	open the file specified
	filePointer	=	fopen(configFilePath, "r");
	if (filePointer != NULL)
	{
		valideLineCnt	=	0;
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
				validEntry	=	ProcessConfigLine(lineBuff, separterChar, configCallBack);
				if (validEntry == false)
				{
					CONSOLE_DEBUG_W_STR("Servo Config file contains invalid data:", lineBuff);
				}
				valideLineCnt++;
			}
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open:", configFilePath);
	}
	return(valideLineCnt);
}

//*****************************************************************************
int		FindKeywordFromTable(const char *keyword, const TYPE_KEYWORDS *keywordTable)
{
int		keywordEnumValue;
int		iii;
	//*	Find the keyword in the table
	keywordEnumValue	=	-1;
	iii			=	0;
	while ((keywordTable[iii].enumValue >= 0) && (keywordEnumValue < 0))
	{
		if (strcasecmp(keyword, keywordTable[iii].keyword) == 0)
		{
			keywordEnumValue	=	keywordTable[iii].enumValue;
		}
		iii++;
	}
	return(keywordEnumValue);
}
