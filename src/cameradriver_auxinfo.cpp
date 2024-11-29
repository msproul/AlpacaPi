//**************************************************************************
//*	Name:			cameradriver_auxinfo.cpp
//*
//*	Author:			Mark Sproul (C) 2024
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
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Nov 18,	2024	<MLS> Created cameradriver_auxinfo.cpp
//*	Nov 21,	2024	<MLS> Added AuxInfo_CreateConfigTemplate()
//*****************************************************************************

#if defined(_ENABLE_CAMERA_) && defined(_ENABLE_FITS_)

#include	<errno.h>
#include	<math.h>
#include	<gnu/libc-version.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<pthread.h>



#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"linuxerrors.h"
#include	"alpaca_defs.h"
#include	"alpacadriver.h"
#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"cameradriver.h"
#include	"readconfigfile.h"
#include	"cameradriver_auxinfo.h"

#define	kAuxInfoConfigFile	"auxinfoconfig.txt"

static bool		gAuxInfo_TelescopeEnabled	=	false;
static bool		gAuxInfo_DomeEnabled		=	false;
static char		gAuxInfo_TelescopeIPstr[32]	=	"";
static int		gAuxInfo_TelescopePort		=	kAlpacaPiDefaultPORT;

//*****************************************************************************

static char	gAuxInfoHeader[]	=
{
		"###################################################################################\n"
		"#\tThis file contains configuration info for auxiliary info to be saved in the FITS header\n"
		"#\tThe purpose is to be able to get the information from the mount (Alpaca telescope)\n"
		"#\tand save it in the FITS header\n"
		"#\n"
		"###################################################################################\n"
};



//*****************************************************************************
enum
{
	kAuxInfo_aaa	=	0,
	//*	order does not matter, using alphabetic for readability

	kAuxInfo_MountIPaddr,
	kAuxInfo_MountPort,


	kAuxInfo_last
};


//**************************************************************************
const TYPE_KEYWORDS	gAuxInfoSettingsList[]	=
{
	//*	the table is all lower case, case does not mater in the file
	{	"mount_ip",			kAuxInfo_MountIPaddr		},
	{	"mount_port",		kAuxInfo_MountPort			},

	{	"",					-1							}
};


//*****************************************************************************
static void	ProcessAuxInfoConfig(const char *keyword, const char *valueString, void *userDataPtr)
{
int		keywordEnumVal;
int		valueStrLen;

	CONSOLE_DEBUG_W_STR(keyword, valueString);


	//*	look up the keyword in the table
	keywordEnumVal	=	FindKeywordFromTable(keyword, gAuxInfoSettingsList);
	valueStrLen		=	strlen(valueString);
	//*	use the keyword enum value to figure out what to do with the data
	switch(keywordEnumVal)
	{
		case kAuxInfo_MountIPaddr:
			if (valueStrLen > 0)
			{
				strcpy(gAuxInfo_TelescopeIPstr,	valueString);
				gAuxInfo_TelescopeEnabled	=	true;
			}
			break;

		case kAuxInfo_MountPort:
			if (valueStrLen > 0)
			{
				gAuxInfo_TelescopePort	=	atoi(valueString);
			}
			break;
		default:
			CONSOLE_DEBUG_W_STR(keyword, valueString);
			break;
	}
}

//*****************************************************************************
static void	AuxInfo_CreateConfigTemplate(void)
{
int		iii;
FILE	*filePointer;
char	linuxErrStr[64];
char	separaterLine[]	=	"###############################################################################\n";
char	fileName[64];

	CONSOLE_DEBUG(__FUNCTION__);

	strcpy(fileName, kAuxInfoConfigFile);
	strcat(fileName, ".template");
	filePointer	=	fopen(fileName, "w");
	if (filePointer != NULL)
	{
		fprintf(filePointer, "%s",	gAuxInfoHeader);
		iii			=	0;
		while (gAuxInfoSettingsList[iii].keyword[0] != 0)
		{
			if (strncmp(gAuxInfoSettingsList[iii].keyword, "###", 3) == 0)
			{
				//*	print out a separator line
				fprintf(filePointer, "%s",	separaterLine);
			}
			else if (gAuxInfoSettingsList[iii].keyword[0] == '#')
			{
				//*	its a comment line, print it out as it is
				fprintf(filePointer, "%s\n", gAuxInfoSettingsList[iii].keyword);
			}
			else
			{
				//*	format the keyword value pair
				fprintf(filePointer, "%-30s\t=\t\n", gAuxInfoSettingsList[iii].keyword);
			}
			iii++;
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
void	AuxInfo_StartThread(void)
{
int		linesRead;

	CONSOLE_DEBUG(__FUNCTION__);

	linesRead		=	ReadGenericConfigFile(kAuxInfoConfigFile, '=', &ProcessAuxInfoConfig, NULL);
//	if (linesRead <= 0)
	{

		AuxInfo_CreateConfigTemplate();
	}


	//------------------------------------------------------------------------
	//*	if anything is enabled, start the auxiliary info thread
	CONSOLE_DEBUG_W_BOOL("gAuxInfo_TelescopeEnabled\t=",	gAuxInfo_TelescopeEnabled);
	CONSOLE_DEBUG_W_BOOL("gAuxInfo_DomeEnabled     \t=",	gAuxInfo_DomeEnabled);
	if (gAuxInfo_TelescopeEnabled || gAuxInfo_DomeEnabled)
	{
	}

}


#endif // defined(_ENABLE_CAMERA_) && defined(_ENABLE_FITS_)

