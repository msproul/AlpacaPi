//*****************************************************************************
//*	StarData.c
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	??				Written by Frank Covits
//*	May  2,	1996	<MLS> (Mark Sproul) Starting on Sky Travel for Frank and Cliff
//*	Nov 18,	1999	<MLS> restarting efforts
//*	Nov 20,	1999	<MLS> major re-writing and re-orginization
//*	Dec 29,	2020	<MLS> Starting to integrate SkyTravel into AlpacaPi
//*	Jan  3,	2021	<MLS> Added ReadTSCfile() & ParseOneLineOfTSCdata()
//*	Jan  5,	2021	<MLS> Added ReadConstellationOutlines()
//*	Jan  5,	2021	<MLS> Reading data from https://www.iau.org/public/themes/constellations/
//*	Jan  6,	2021	<MLS> Added DumpCelestDataStruct() for debugging
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



#include	"SkyStruc.h"
#include	"SkyTravelConstants.h"
#include	"StarData.h"
#include	"ConstellationData.h"



static	TYPE_CelestData	*gDefaultStarCatalog	=	NULL;
static	long			gDefaultStarCatCount	=	0;


//**************************************************************************
long	GetFileSize(const char *filePath)
{
struct stat fileStatBuffer;
int         status;
long		myFileSize;

//	CONSOLE_DEBUG(__FUNCTION__);

	myFileSize	=	-1;
	status		=	stat(filePath, &fileStatBuffer);
	if(status == 0)
	{
		// size of file is in member buffer.st_size;
		myFileSize	=	fileStatBuffer.st_size;
	}
	return(myFileSize);
}


//**************************************************************************
//* read DEFAULT star binary data from disk
//*	returns number of objects
//**************************************************************************
static TYPE_CelestData *ReadDefaultStarDataDisk(long *objectCount)
{
FILE				*filePointer;
long				fileSize;
long				ii;
long				myDiskObjectCount;
TYPE_CelestDataDisk	*myDiskObjectPtr;
TYPE_CelestData		*myObjectPtr;
char				filePath[128];
int					bytesRead;

//	CONSOLE_DEBUG(__FUNCTION__);

	myObjectPtr		=	NULL;
	*objectCount	=	0;

	strcpy(filePath, kSkyTravelDataDirectory);
	strcat(filePath, "/OBJALL.B");

	fileSize		=	GetFileSize(filePath);
//	CONSOLE_DEBUG_W_STR("filePath\t=", filePath);
//	CONSOLE_DEBUG_W_LONG("fileSize\t=", fileSize);

	if (fileSize > 0)
	{
		myDiskObjectPtr	=	(TYPE_CelestDataDisk *)malloc(fileSize);
		if (myDiskObjectPtr != NULL)
		{
			filePointer	=	fopen(filePath, "r");
			if (filePointer != NULL)
			{
				bytesRead	=	fread(myDiskObjectPtr, 1, fileSize, filePointer);

//				CONSOLE_DEBUG_W_NUM("bytesRead\t=", bytesRead);

				myDiskObjectCount	=	fileSize / sizeof(TYPE_CelestDataDisk);

//				CONSOLE_DEBUG_W_LONG("TYPE_CelestDataDisk\t=", sizeof(TYPE_CelestDataDisk));
//				CONSOLE_DEBUG_W_LONG("myDiskObjectCount\t=", myDiskObjectCount);

			#ifdef _BIG_ENDIAN_
				CONSOLE_DEBUG("_BIG_ENDIAN_");
				for (ii=0; ii<myDiskObjectCount; ii++)
				{
					Swap4Bytes((long *)	&(myDiskObjectPtr[ii].decl));
					Swap4Bytes((long *)	&(myDiskObjectPtr[ii].ra));
					Swap2Bytes((short *)&(myDiskObjectPtr[ii].magn));
					Swap2Bytes((short *)&(myDiskObjectPtr[ii].id));
				}
			#endif

				myObjectPtr	=	(TYPE_CelestData *)malloc((myDiskObjectCount + 10) * sizeof(TYPE_CelestData));
				if (myObjectPtr != NULL)
				{
					for (ii=0; ii<myDiskObjectCount; ii++)
					{
						myObjectPtr[ii].id			=	myDiskObjectPtr[ii].id;
						myObjectPtr[ii].magn		=	myDiskObjectPtr[ii].magn;
						myObjectPtr[ii].dataSrc		=	kDataSrc_Orginal;
						myObjectPtr[ii].ra			=	myDiskObjectPtr[ii].ra;
						myObjectPtr[ii].decl		=	myDiskObjectPtr[ii].decl;
						myObjectPtr[ii].org_ra		=	myDiskObjectPtr[ii].ra;
						myObjectPtr[ii].org_decl	=	myDiskObjectPtr[ii].decl;
	//					CONSOLE_DEBUG_W_NUM("myObjectPtr[ii].magn\t=", myObjectPtr[ii].magn);
					}
				}

				*objectCount	=	myDiskObjectCount;
				fclose(filePointer);
			}
			free(myDiskObjectPtr);
		}
	}
	else
	{
		CONSOLE_DEBUG("Failed to get file size")
	}
	return(myObjectPtr);
}


//**************************************************************************
//* read star binary data
//*	returns number of objects
//*	each window needs its own copy, so this routine creates a new copy of the data
//**************************************************************************
TYPE_CelestData	*GetDefaultStarData(long *objectCount, TYPE_Time *timePtr)
{
long			ii;
TYPE_CelestData	*myObjectPtr;

//	CONSOLE_DEBUG(__FUNCTION__);
	myObjectPtr	=	NULL;
	if (gDefaultStarCatalog == NULL)
	{
		gDefaultStarCatalog	=	ReadDefaultStarDataDisk(&gDefaultStarCatCount);
	}


	if ((gDefaultStarCatalog != NULL) && (gDefaultStarCatCount > 0))
	{
		myObjectPtr	=	(TYPE_CelestData *)malloc((gDefaultStarCatCount + 10) * sizeof(TYPE_CelestData));
		for (ii=0; ii<gDefaultStarCatCount; ii++)
		{
			myObjectPtr[ii]	=	gDefaultStarCatalog[ii];
		}

		*objectCount	=	gDefaultStarCatCount;

		timePtr->timeOfLastPrec		=	JD2000;	//*	set last prec = julian day 2000
		timePtr->strflag			=	true;	//*	say 2000 data present
		timePtr->starDataModified	=	false;

		//*	debugging
//		for (ii=0; ii<50; ii++)
//		{
//			DumpCelestDataStruct(__FUNCTION__, &gDefaultStarCatalog[ii]);
//		}
//		CONSOLE_ABORT(__FUNCTION__);

	}
	return(myObjectPtr);
}


int		gMaxArgLen	=	0;


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
	starRec->dataSrc	=	kDataSrc_Unkown;

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

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, filePath);

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

				bufferSize	=	specifiedLnCnt * sizeof(TYPE_CelestData);
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
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read:", filePath);
	}
//	CONSOLE_DEBUG_W_NUM("gMaxArgLen\t=", gMaxArgLen);

	return(tscStarData);
}


//*****************************************************************************
void	DumpCelestDataStruct(const char *functionName, TYPE_CelestData *objectStruct)
{
int	myHexData;

	CONSOLE_DEBUG_W_STR("Called from  \t=",	functionName);
	CONSOLE_DEBUG_W_LONG("sizeof(obj) \t=",	sizeof(TYPE_CelestData));
	CONSOLE_DEBUG_W_LONG("id          \t=",	objectStruct->id);
	CONSOLE_DEBUG_W_NUM("magn        \t=",	objectStruct->magn);
	CONSOLE_DEBUG_W_NUM("dataSrc      \t=",	objectStruct->dataSrc);
	myHexData	=	objectStruct->type;
	CONSOLE_DEBUG_W_HEX("type         \t=",	myHexData);
//	CONSOLE_DEBUG_W_LONG("curXX       \t=",	objectStruct->curXX);
//	CONSOLE_DEBUG_W_NUM("curYY        \t=",	objectStruct->curYY);
	CONSOLE_DEBUG_W_DBL("realMagnitude\t=",	objectStruct->realMagnitude);
	CONSOLE_DEBUG_W_DBL("ra           \t=",	DEGREES(objectStruct->ra / 15.0));
	CONSOLE_DEBUG_W_DBL("decl         \t=",	DEGREES(objectStruct->decl));
	CONSOLE_DEBUG_W_DBL("org_ra       \t=",	DEGREES(objectStruct->org_ra / 15.0));
	CONSOLE_DEBUG_W_DBL("org_decl     \t=",	DEGREES(objectStruct->org_decl));
//	CONSOLE_DEBUG_W_DBL("maxSizeArcMinutes\t=",	objectStruct->maxSizeArcMinutes);
	CONSOLE_DEBUG_W_DBL("parallax     \t=",	objectStruct->parallax);
	CONSOLE_DEBUG_W_STR("shortName    \t=",	objectStruct->shortName);
	CONSOLE_DEBUG_W_STR("longName     \t=",	objectStruct->longName);

}
