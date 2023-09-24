//*****************************************************************************
//*	MessierData.c
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jul 23,	2023	<MLS> Created MessierData.c
//*	Mar  3,	2021	<MLS> Added ReadMessierData()
//*	Mar  3,	2021	<MLS> Added ReadMessierCatalog() & ParseOneLineMessierCatalog()
//*****************************************************************************
//*	Messier data
//	https://starlust.org/messier-catalog/
//	https://docs.google.com/spreadsheets/d/11keXJH6XIeJh6N90yRQ-9X_Pdg9vpq34vWZY8RA9I5c/edit#gid=0
//
//*****************************************************************************

#include	<string.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<math.h>
#include	<ctype.h>


//*	MLS Libraries
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"alpacadriver_helper.h"
#include	"helper_functions.h"
#include	"controller_startup.h"

#include	"SkyStruc.h"
#include	"SkyTravelConstants.h"
#include	"StarData.h"

static int		gMaxArgLen	=	0;

//************************************************************************
//*	sample data
//*		110,,,,,,,,
//*		2000,,,,,,,,
//*		Taurus,Messier 1,5,34,32,+,22,0,52
//*		Aquarius,Messier 2,21,33,27,-,0,49,22
//************************************************************************
//*	added by MLS 1/3/2021
//************************************************************************
static void	ParseOneLineOfTSCdata(char *lineBuff, TYPE_CelestData *starRec)
{
int		sLen;
int		raHour		=	0;
int		raMin		=	0;
int		raSec		=	0;
double	raDegrees	=	0;
double	raRadians	=	0;

char	deSign		=	0;
int		deDeg		=	0;
int		deMin		=	0;
int		deSec		=	0;
double	declDegrees	=	0;
double	declRadians	=	0;

int		ii;
int		argNum;
char	argString[32];
char	theChar;
int		ccc;
int		argLen;

	//*	step thru the line looking for commas.
	sLen		=	strlen(lineBuff);
	argNum		=	0;
	ccc			=	0;
	for (ii=0; ii <= sLen; ii++)
	{
		theChar	=	lineBuff[ii];
		if ((theChar == ',') || (theChar == 0))
		{
			argLen	=	strlen(argString);
			if (argLen > gMaxArgLen)
			{
				gMaxArgLen	=	argLen;
			}
			//*	end of argument, do something with it
			switch(argNum)
			{
				case 0:	//*	name 1
				//	CONSOLE_DEBUG_W_STR("Name1", argString);
					if (argLen < kLongNameMax)
					{
						strcpy(starRec->longName, argString);
					}
					break;

				case 1:	//*	name 2
				//	CONSOLE_DEBUG_W_STR("Name2", argString);
					if (strncasecmp(argString, "Messier", 7) == 0)
					{
						starRec->shortName[0]	=	'M';
						strcat(starRec->shortName, &argString[8]);
						starRec->id	=	atoi(&argString[8]);
					}
					break;

				case 2:	//*	raHour
					raHour	=	atoi(argString);
					break;

				case 3:	//*	raMin
					raMin	=	atoi(argString);
					break;

				case 4:	//*	raSec
					raSec	=	atoi(argString);
					break;

				//--------------
				case 5:	//*	degrees sign
					deSign	=	argString[0];
					break;

				case 6:	//*	deDeg
					deDeg	=	atoi(argString);
					break;

				case 7:	//*	deMin
					deMin	=	atoi(argString);

					break;
				case 8:	//*	deSec
					deSec	=	atoi(argString);
					break;

			}
			argNum++;
			ccc		=	0;
		}
		else
		{
			if (ccc < 30)
			{
				argString[ccc++]	=	theChar;
				argString[ccc]	=	0;
			}
		}
	}
	//*	now we have all the data parsed, put it into the record
	raDegrees	=	raHour;
	raDegrees	+=	raMin / 60.0;
	raDegrees	+=	raSec / 3600.0;
	raRadians	=	(raDegrees / 24.0) * (2.0 * M_PI);	//*	0->24 not 0->360

	declDegrees	=	deDeg;
	declDegrees	+=	deMin / 60.0;
	declDegrees	+=	deSec / 3600.0;
	declRadians	=	(declDegrees * PI) / 180.0;


//	starRec->id				=	ParseLongFromString(lineBuff,	 1-1, 4);


	if (deSign == '-')
	{
		declRadians		=	-declRadians;
	}

	starRec->ra			=	raRadians;
	starRec->decl		=	declRadians;
	starRec->org_ra		=	raRadians;
	starRec->org_decl	=	declRadians;
	starRec->dataSrc	=	kDataSrc_Unknown;

	starRec->magn		=	0x34;


}


//************************************************************************
//*	added by MLS 1/3/2021
//************************************************************************
TYPE_CelestData	*ReadTSCfile(const char *filePath, int dataSource, long *objectCount)
{
FILE			*filePointer;
int				specifiedLnCnt;
int				recordCount;
TYPE_CelestData	*tscStarData;
char			lineBuff[256];
size_t			bufferSize;
int				startupWidgetIdx;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, filePath);
	startupWidgetIdx	=	SetStartupText("Messier-TSC catalog:");

	tscStarData	=	NULL;
	filePointer	=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
		//*	the first line specifies the number of data entries
		//*	the 2nd line is the data catalog year (normally 2000)
		if (fgets(lineBuff, 100, filePointer))
		{
			specifiedLnCnt	=	atoi(lineBuff);
			if (specifiedLnCnt > 10)
			{
//				CONSOLE_DEBUG_W_NUM("specifiedLnCnt\t=", specifiedLnCnt);

				//*	get the dataset year
				if (fgets(lineBuff, 100, filePointer))
				{

				}

				bufferSize	=	(specifiedLnCnt + 2) * sizeof(TYPE_CelestData);
				tscStarData	=	(TYPE_CelestData *)malloc(bufferSize);

				if (tscStarData != NULL)
				{
					memset(tscStarData, 0, bufferSize);
					recordCount	=	0;
					while (fgets(lineBuff, 200, filePointer) && (recordCount < specifiedLnCnt))
					{
						ParseOneLineOfTSCdata(lineBuff, &tscStarData[recordCount]);
						tscStarData[recordCount].dataSrc	=	dataSource;

						recordCount++;
					}

					*objectCount	=	recordCount;
				}
			}
		}
		fclose(filePointer);
		SetStartupTextStatus(startupWidgetIdx, "OK");
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read:", filePath);
		SetStartupTextStatus(startupWidgetIdx, "Failed");
	}
//	CONSOLE_DEBUG_W_NUM("gMaxArgLen\t=", gMaxArgLen);

	return(tscStarData);
}

#ifdef _USE_ORIGNIAL_MESSIER_CATALOG_

//************************************************************************
//	#M	NGC	TYPE	CONS	RA	DEC	MAG	SIZE	DIST (ly)	VIEWING SEASON	VIEWING DIFFICULTY
//	M1	NGC 1952 Crab Nebula	Supernova Remnant	Taurus	5h 34.5m	22:1	8.4	6.0x4.0	6300	Winter	Moderate
//************************************************************************
static void	ParseOneLineMessierCatalog(char *lineBuff, TYPE_CelestData *starRec)
{
int		sLen;
int		raHour		=	0;
double	raMin		=	0;
//int		raSec		=	0;
double	raDegrees	=	0;
double	raRadians	=	0;
char	deSign		=	0;
int		deDeg		=	0;
int		deMin		=	0;
//int		deSec		=	0;
double	declDegrees	=	0;
double	declRadians	=	0;
int		ii;
int		argNum;
char	argString[64];
char	theChar;
int		ccc;
char	*delimPtr;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, lineBuff);

	if (strncasecmp(lineBuff, "M45", 3) == 0)
	{
		CONSOLE_DEBUG_W_STR(__FUNCTION__, lineBuff);
//		CONSOLE_ABORT(__FUNCTION__);
	}

	//*	step thru the line looking for TABS.
	argString[0]	=	0;
	sLen			=	strlen(lineBuff);
	argNum			=	0;
	ccc				=	0;
	for (ii=0; ii <= sLen; ii++)
	{
		theChar	=	lineBuff[ii];
		if ((theChar == 0x09) || (theChar == ',') || (theChar == 0))
		{

			//*	end of argument, do something with it
			switch(argNum)
			{
				case 0:	//*	M #
					strcpy(starRec->shortName, argString);
					break;

				case 1:	//*	long name
					strcpy(starRec->longName, argString);
					break;

				case 4:	//*	right ascension
						//*	5h 34.5m
					raHour		=	atoi(argString);
					delimPtr	=	strchr(argString, 0x20);
					if (delimPtr != NULL)
					{
						delimPtr++;
						raMin	=	AsciiToDouble(delimPtr);
					}
					break;

				case 5:	//*	declination
						//*	22:1
					deSign		=	argString[0];
					deDeg		=	abs(atoi(argString));
					delimPtr	=	strchr(argString, ':');
					if (delimPtr != NULL)
					{
						delimPtr++;
						deMin	=	AsciiToDouble(delimPtr);
					}
					break;

			}
			argNum++;
			ccc		=	0;
		}
		else
		{
			if (ccc < 60)
			{
				argString[ccc++]	=	theChar;
				argString[ccc]	=	0;
			}
		}
	}
	//*	now we have all the data parsed, put it into the record
	raDegrees	=	raHour;
	raDegrees	+=	raMin / 60.0;
//	raDegrees	+=	raSec / 3600.0;
	raRadians	=	(raDegrees / 24.0) * (2.0 * M_PI);	//*	0->24 not 0->360

	declDegrees	=	deDeg;
	declDegrees	+=	deMin / 60.0;
//	declDegrees	+=	deSec / 3600.0;
	declRadians	=	(declDegrees * PI) / 180.0;


	if (deSign == '-')
	{
		declRadians		=	-declRadians;
	}

	starRec->ra			=	raRadians;
	starRec->decl		=	declRadians;
	starRec->org_ra		=	raRadians;
	starRec->org_decl	=	declRadians;
	starRec->dataSrc	=	kDataSrc_Unknown;

	starRec->magn		=	0x34;
}



//************************************************************************
//*	Messier data
//	https://starlust.org/messier-catalog/
//	https://docs.google.com/spreadsheets/d/11keXJH6XIeJh6N90yRQ-9X_Pdg9vpq34vWZY8RA9I5c/edit#gid=0
//************************************************************************
static TYPE_CelestData	*ReadMessierCatalog(const char *filePath, int dataSource, long *objectCount)
{
FILE			*filePointer;
int				specifiedLnCnt;
int				recordCount;
TYPE_CelestData	*messierData;
char			lineBuff[256];
size_t			bufferSize;
int				startupWidgetIdx;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, filePath);
	startupWidgetIdx	=	SetStartupText("Messier catalog:");

	messierData	=	NULL;

	filePointer	=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
		specifiedLnCnt	=	120;
		bufferSize		=	(specifiedLnCnt + 2) * sizeof(TYPE_CelestData);
		messierData		=	(TYPE_CelestData *)malloc(bufferSize);

		if (messierData != NULL)
		{
			memset(messierData, 0, bufferSize);
			recordCount	=	0;
			while (fgets(lineBuff, 200, filePointer) && (recordCount < specifiedLnCnt))
			{
				if (lineBuff[0] != '#')
				{
					ParseOneLineMessierCatalog(lineBuff, &messierData[recordCount]);
					messierData[recordCount].dataSrc	=	dataSource;

					recordCount++;
				}
			}

			*objectCount	=	recordCount;
		}
		fclose(filePointer);
		SetStartupTextStatus(startupWidgetIdx, "OK");
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read:", filePath);
		SetStartupTextStatus(startupWidgetIdx, "Failed");
	}
//	CONSOLE_DEBUG_W_NUM("gMaxArgLen\t=", gMaxArgLen);

	return(messierData);
}
#endif // _USE_ORIGNIAL_MESSIER_CATALOG_



//************************************************************************
TYPE_CelestData	*ReadMessierData(const char *folderPath, int dataSource, long *objectCount)
{
TYPE_CelestData	*messierData;
char			myFilePath[256];

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, folderPath);

#ifdef _USE_ORIGNIAL_MESSIER_CATALOG_
//	strcpy(myFilePath, folderPath);
////	strcat(myFilePath, "MessierCatalog.csv");
//	strcat(myFilePath, "MessierCatalog.tab");
//
//	messierData	=	ReadMessierCatalog(myFilePath, dataSource, objectCount);
//	if (messierData == NULL)
#endif
	{
	//	CONSOLE_DEBUG("ReadMessierCatalog() failed, trying TSC file");
		//*	failed, try the TSC version
		strcpy(myFilePath, folderPath);
		strcat(myFilePath, "Messier.tsc");
		messierData	=	ReadTSCfile(myFilePath, dataSource, objectCount);
	}
//	CONSOLE_DEBUG_W_LONG("Messier object count\t=", *objectCount);
//	CONSOLE_ABORT(__FUNCTION__);
	return(messierData);
}

