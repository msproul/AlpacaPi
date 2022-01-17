//*****************************************************************************
//*	StarData.c
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	??				Original Written by Frank Covits
//*	May  2,	1996	<MLS> (Mark Sproul) Starting on SkyTravel for Frank and Clif
//*	Nov 18,	1999	<MLS> restarting efforts
//*	Nov 20,	1999	<MLS> major re-writing and re-organization
//*	Dec 29,	2020	<MLS> Starting to integrate SkyTravel into AlpacaPi
//*	Jan  3,	2021	<MLS> Added ReadTSCfile() & ParseOneLineOfTSCdata()
//*	Jan  6,	2021	<MLS> Added DumpCelestDataStruct() for debugging
//*	Mar  3,	2021	<MLS> Added ReadMessierData()
//*	Mar  3,	2021	<MLS> Added ReadMessierCatalog() & ParseOneLineMessierCatalog()
//*	Mar  4,	2021	<MLS> Added ReadHYGdata() & ParseOneLineHYGdata()
//*	Mar  5,	2021	<MLS> Added ReadHenryDraperCatalog() & ParseOneLineHenryDraperData()
//*	Mar  7,	2021	<MLS> Added ReadSpecialData()
//*	Apr 17,	2021	<MLS> Jim H. Found bug in ReadHYGdata(), short name overflow
//*	Oct 23,	2021	<MLS> Added magnitude processing to ParseOneLineHYGdata()
//*	Oct 24,	2021	<MLS> Added parsing of spectral class to Henry Draper catalog
//*****************************************************************************
//*	Messier data
//	https://starlust.org/messier-catalog/
//	https://docs.google.com/spreadsheets/d/11keXJH6XIeJh6N90yRQ-9X_Pdg9vpq34vWZY8RA9I5c/edit#gid=0
//
//
//	https://github.com/astronexus/HYG-Database
//
//	https://heasarc.gsfc.nasa.gov/FTP/heasarc/dbase/dump/
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
int			status;
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


//*****************************************************************************
void	DumpCelestDataStruct(const char *functionName, TYPE_CelestData *objectStruct)
{
int	myHexData;

	CONSOLE_DEBUG("---------------------------------------------------------");
	CONSOLE_DEBUG_W_STR("Called from  \t=",	functionName);
	CONSOLE_DEBUG_W_LONG("sizeof(obj) \t=",	sizeof(TYPE_CelestData));
	CONSOLE_DEBUG_W_LONG("id          \t=",	objectStruct->id);
	CONSOLE_DEBUG_W_NUM("magn         \t=",	objectStruct->magn);
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
				if (bytesRead > 0)
				{
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
					CONSOLE_ABORT("_BIG_ENDIAN_");
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
				else
				{
					CONSOLE_DEBUG("Failed to read star data");
				}
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
TYPE_CelestData	*ReadDefaultStarData(long *objectCount, TYPE_Time *timePtr)
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
				specifiedLnCnt	+=	3;	//*	leave some extra room
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
						raMin	=	atof(delimPtr);
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
						deMin	=	atof(delimPtr);
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

	CONSOLE_DEBUG_W_STR(__FUNCTION__, filePath);

	messierData	=	NULL;

	filePointer	=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
		specifiedLnCnt	=	120;
		bufferSize		=	specifiedLnCnt * sizeof(TYPE_CelestData);
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
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read:", filePath);
	}
//	CONSOLE_DEBUG_W_NUM("gMaxArgLen\t=", gMaxArgLen);

	return(messierData);
}



//************************************************************************
TYPE_CelestData	*ReadMessierData(const char *folderPath, int dataSource, long *objectCount)
{
TYPE_CelestData	*messierData;
char			myFilePath[256];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, folderPath);

	strcpy(myFilePath, folderPath);
//	strcat(myFilePath, "MessierCatalog.csv");
	strcat(myFilePath, "MessierCatalog.tab");

	messierData	=	ReadMessierCatalog(myFilePath, dataSource, objectCount);
	if (messierData == NULL)
	{
		CONSOLE_DEBUG("ReadMessierCatalog() failed, trying TSC file");
		//*	failed, try the TSC version
		strcpy(myFilePath, folderPath);
		strcat(myFilePath, "Messier.tsc");
		messierData	=	ReadTSCfile(myFilePath, dataSource, objectCount);
	}
	return(messierData);
}

//************************************************************************
//	1,1,224700,,,,,0.000060,1.089009,219.7802,-5.20,-1.88,0.0,9.100,2.390,F5,0.482,219.740502,0.003449,4.177065,0.00000004,-0.00000554,-0.00000200,0.0000
//	15693409775347223,0.01900678824815125,-0.000000025210311388888885,-0.000000009114497,,,Psc,1,1,,9.638290236239703,,,
//************************************************************************
//*
//*	1. id: The database primary key.
//*	2. hip: The star's ID in the Hipparcos catalog, if known.
//*	3. hd: The star's ID in the Henry Draper catalog, if known.
//*	4. hr: The star's ID in the Harvard Revised catalog, which is the same as its number in the Yale Bright Star Catalog.
//*	5. gl: The star's ID in the third edition of the Gliese Catalog of Nearby Stars.
//*	6. bf: The Bayer / Flamsteed designation, primarily from the Fifth Edition of the Yale Bright Star Catalog. This is a combination of the two designations. The Flamsteed number, if present, is given first; then a three-letter abbreviation for the Bayer Greek letter; the Bayer superscript number, if present; and finally, the three-letter constellation abbreviation. Thus Alpha Andromedae has the field value "21Alp And", and Kappa1 Sculptoris (no Flamsteed number) has "Kap1Scl".
//*	7. ra, dec: The star's right ascension and declination, for epoch and equinox 2000.0.
//*	8. proper: A common name for the star, such as "Barnard's Star" or "Sirius". I have taken these names primarily from the Hipparcos project's web site, which lists representative names for the 150 brightest stars and many of the 150 closest stars. I have added a few names to this list. Most of the additions are designations from catalogs mostly now forgotten (e.g., Lalande, Groombridge, and Gould ["G."]) except for certain nearby stars which are still best known by these designations.
//*	9. dist: The star's distance in parsecs, the most common unit in astrometry. To convert parsecs to light years, multiply by 3.262. A value >= 100000 indicates missing or dubious (e.g., negative) parallax data in Hipparcos.
//*	10. pmra, pmdec:  The star's proper motion in right ascension and declination, in milliarcseconds per year.
//*	11. rv:  The star's radial velocity in km/sec, where known.
//*	12. mag: The star's apparent visual magnitude.
//*	13. absmag: The star's absolute visual magnitude (its apparent magnitude from a distance of 10 parsecs).
//*	14. spect: The star's spectral type, if known.
//*	15. ci: The star's color index (blue magnitude - visual magnitude), where known.
//*	16. x,y,z: The Cartesian coordinates of the star, in a system based on the equatorial coordinates as seen from Earth. +X is in the direction of the vernal equinox (at epoch 2000), +Z towards the north celestial pole, and +Y in the direction of R.A. 6 hours, declination 0 degrees.
//*	17. vx,vy,vz: The Cartesian velocity components of the star, in the same coordinate system described immediately above. They are determined from the proper motion and the radial velocity (when known). The velocity unit is parsecs per year; these are small values (around 1 millionth of a parsec per year), but they enormously simplify calculations using parsecs as base units for celestial mapping.
//*	18. rarad, decrad, pmrarad, prdecrad:  The positions in radians, and proper motions in radians per year.
//*	19. bayer:  The Bayer designation as a distinct value
//*	20. flam:  The Flamsteed number as a distinct value
//*	21. con:  The standard constellation abbreviation
//*	22. comp, comp\_primary, base:  Identifies a star in a multiple star system.  comp = ID of companion star, comp\_primary = ID of primary star for this component, and base = catalog ID or name for this multi-star system.  Currently only used for Gliese stars.
//*	23. lum:  Star's luminosity as a multiple of Solar luminosity.
//*	24. var:  Star's standard variable star designation, when known.
//*	25. var\_min, var\_max:  Star's approximate magnitude range, for variables.  This value is based on the Hp magnitudes for the range in the original Hipparcos catalog, adjusted to the V magnitude scale to match the "mag" field.
//*
//************************************************************************
enum
{
	//*	note, this does not match the description above, in the data, PROPER is before RA.

	kHYG_id		=	1,	// The database primary key.
	kHYG_hip,			//*	The star's ID in the Hipparcos catalog, if known.
	kHYG_hd,			//*	The star's ID in the Henry Draper catalog, if known.
	kHYG_hr,			//*	The star's ID in the Harvard Revised catalog, which is the same as its number in the Yale Bright Star Catalog.
	kHYG_gl,			//*	The star's ID in the third edition of the Gliese Catalog of Nearby Stars.
	kHYG_bf,			//*	The Bayer / Flamsteed designation, primarily from the Fifth Edition of the Yale Bright Star Catalog. This is a combination of the two designations. The Flamsteed number, if present, is given first; then a three-letter abbreviation for the Bayer Greek letter; the Bayer superscript number, if present; and finally, the three-letter constellation abbreviation. Thus Alpha Andromedae has the field value "21Alp And", and Kappa1 Sculptoris (no Flamsteed number) has "Kap1Scl".
	kHYG_proper,		//*	A common name for the star, such as "Barnard's Star" or "Sirius". I have taken these names primarily from the Hipparcos project's web site, which lists representative names for the 150 brightest stars and many of the 150 closest stars. I have added a few names to this list. Most of the additions are designations from catalogs mostly now forgotten (e.g., Lalande, Groombridge, and Gould ["G."]) except for certain nearby stars which are still best known by these designations.

	kHYG_ra,
	kHYG_dec,			//*	The star's right ascension and declination, for epoch and equinox 2000.0.
	kHYG_dist,			//*	The star's distance in parsecs, the most common unit in astrometry. To convert parsecs to light years, multiply by 3.262. A value >= 100000 indicates missing or dubious (e.g., negative) parallax data in Hipparcos.

	kHYG_pmra,
	kHYG_pmdec,			//*	The star's proper motion in right ascension and declination, in milliarcseconds per year.
	kHYG_rv,			//*	The star's radial velocity in km/sec, where known.
	kHYG_mag,			//*	The star's apparent visual magnitude.
	kHYG_absmag,		//*	The star's absolute visual magnitude (its apparent magnitude from a distance of 10 parsecs).
	kHYG_spect,			//*	The star's spectral type, if known.
	kHYG_ci,			//*	The star's color index (blue magnitude - visual magnitude), where known.

	kHYG_x,
	kHYG_y,
	kHYG_z,				//*	The Cartesian coordinates of the star, in a system based on the equatorial coordinates as seen from Earth. +X is in the direction of the vernal equinox (at epoch 2000), +Z towards the north celestial pole, and +Y in the direction of R.A. 6 hours, declination 0 degrees.

	kHYG_vx,
	kHYG_vy,
	kHYG_vz,			//*	The Cartesian velocity components of the star, in the same coordinate system described immediately above. They are determined from the proper motion and the radial velocity (when known). The velocity unit is parsecs per year; these are small values (around 1 millionth of a parsec per year), but they enormously simplify calculations using parsecs as base units for celestial mapping.

	kHYG_rarad,
	kHYG_decrad,
	kHYG_pmrarad,
	kHYG_prdecrad,		//*	The positions in radians, and proper motions in radians per year.
	kHYG_bayer,			//*	The Bayer designation as a distinct value
	kHYG_flam,			//*	The Flamsteed number as a distinct value
	kHYG_con,			//*	The standard constellation abbreviation

	kHYG_comp,
	kHYG_comp_primary,
	kHYG_base,			//*	Identifies a star in a multiple star system.  comp = ID of companion star, comp\_primary = ID of primary star for this component, and base = catalog ID or name for this multi-star system.  Currently only used for Gliese stars.
	kHYG_lum,			//*	Star's luminosity as a multiple of Solar luminosity.
	kHYG_var,			//*	Star's standard variable star designation, when known.

	kHYG_var_min,
	kHYG_var_max,		//*	Star's approximate magnitude range, for variables.  This value is based on the Hp magnitudes for the range in the original Hipparcos catalog, adjusted to the V magnitude scale to match the "mag" field.


	kHYG_Last
};

//************************************************************************
static void	ParseOneLineHYGdata(char *lineBuff, TYPE_CelestData *starRec)
{
int		sLen;
int		argLen;
double	raDegrees	=	0.0;
double	raRadians	=	0.0;
double	declDegrees	=	0.0;
double	declRadians	=	0.0;
double	magnitude	=	0.0;
int		ii;
int		argNum;
char	argString[64];
char	theChar;
char	spectralClass	=	0;
int		ccc;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, lineBuff);

	//*	step thru the line looking for TABS.
	sLen		=	strlen(lineBuff);
	argNum		=	1;		//*	start with 1 to match the above docs
	ccc			=	0;
	for (ii=0; ii <= sLen; ii++)
	{
		theChar	=	lineBuff[ii];
		if ((theChar == ',') || (theChar == 0))
		{
			argLen	=	strlen(argString);
			//*	end of argument, do something with it
			switch(argNum)
			{
				case kHYG_id:	//*		1. id: The database primary key.
					starRec->id	=	atoi(argString);
					strcpy(starRec->shortName, "HYG");
					strcat(starRec->shortName, argString);
					break;

				case kHYG_hip:		//*		2. hip: The star's ID in the Hipparcos catalog, if known.
					if (argLen > 0)
					{
						starRec->id	=	atoi(argString);
						strcpy(starRec->shortName, "HIP");
						strcat(starRec->shortName, argString);
					}
					break;

				//*		3. hd: The star's ID in the Henry Draper catalog, if known.
				case kHYG_hd:
//					CONSOLE_DEBUG_W_NUM("kHYG_hd: argNum=", argNum);
					if (argLen > 0)
					{
						if (argLen < (kShortNameMax - 2))
						{
							starRec->id	=	atoi(argString);
							strcpy(starRec->shortName, "HD");
							strcat(starRec->shortName, argString);
						}
						else
						{
							CONSOLE_DEBUG_W_STR("Name too long:", argString);
						//	CONSOLE_ABORT(__FUNCTION__);
						}
					}
					break;

				//*		8. proper: A common name for the star, such as "Barnard's Star" or "Sirius". I have taken these names primarily from the Hipparcos project's web site, which lists representative names for the 150 brightest stars and many of the 150 closest stars. I have added a few names to this list. Most of the additions are designations from catalogs mostly now forgotten (e.g., Lalande, Groombridge, and Gould ["G."]) except for certain nearby stars which are still best known by these designations.
				case kHYG_proper:
					if (argLen < kLongNameMax)
					{
						strcat(starRec->longName, argString);
//						if (argLen > 0)
//						{
//							CONSOLE_DEBUG_W_STR("longName", argString);
//						}
					}
					else
					{
						CONSOLE_DEBUG_W_STR("Name too long", argString);
						strncpy(starRec->longName, argString, (kLongNameMax -1));
						starRec->longName[kLongNameMax -1]	=	0;
						CONSOLE_ABORT(__FUNCTION__);
					}
					break;


				//*	The star's right ascension and declination, for epoch and equinox 2000.0.
				case kHYG_ra:	//*	right ascension
					raDegrees		=	atof(argString);
					break;

				case kHYG_dec:		//*	declination
					declDegrees		=	atof(argString);
					break;

				case kHYG_mag:	//*	magnitude
					magnitude		=	atof(argString);
					break;

				case kHYG_spect:
//					CONSOLE_DEBUG_W_NUM("kHYG_spect: argNum=", argNum);
					spectralClass	=	argString[0];
					break;

				//*		9. dist: The star's distance in parsecs, the most common unit in astrometry. To convert parsecs to light years, multiply by 3.262. A value >= 100000 indicates missing or dubious (e.g., negative) parallax data in Hipparcos.
				//*		10. pmra, pmdec:  The star's proper motion in right ascension and declination, in milliarcseconds per year.
				//*		11. rv:  The star's radial velocity in km/sec, where known.
				//*		12. mag: The star's apparent visual magnitude.
				//*		13. absmag: The star's absolute visual magnitude (its apparent magnitude from a distance of 10 parsecs).
				//*		14. spect: The star's spectral type, if known.
				//*		15. ci: The star's color index (blue magnitude - visual magnitude), where known.
				//*		16. x,y,z: The Cartesian coordinates of the star, in a system based on the equatorial coordinates as seen from Earth. +X is in the direction of the vernal equinox (at epoch 2000), +Z towards the north celestial pole, and +Y in the direction of R.A. 6 hours, declination 0 degrees.
				//*		17. vx,vy,vz: The Cartesian velocity components of the star, in the same coordinate system described immediately above. They are determined from the proper motion and the radial velocity (when known). The velocity unit is parsecs per year; these are small values (around 1 millionth of a parsec per year), but they enormously simplify calculations using parsecs as base units for celestial mapping.
				//*		18. rarad, decrad, pmrarad, prdecrad:  The positions in radians, and proper motions in radians per year.
				//*		19. bayer:  The Bayer designation as a distinct value
				//*		20. flam:  The Flamsteed number as a distinct value
				//*		21. con:  The standard constellation abbreviation
				//*		22. comp, comp\_primary, base:  Identifies a star in a multiple star system.  comp = ID of companion star, comp\_primary = ID of primary star for this component, and base = catalog ID or name for this multi-star system.  Currently only used for Gliese stars.
				//*		23. lum:  Star's luminosity as a multiple of Solar luminosity.
				//*		24. var:  Star's standard variable star designation, when known.
				//*		25. var\_min, var\_max:  Star's approximate magnitude range, for variables.  This value is based on the Hp magnitudes for the range in the original Hipparcos catalog, adjusted to the V magnitude scale to match the "mag" field.

			}
			argNum++;
			ccc				=	0;
			argString[0]	=	0;
		}
		else
		{
			if (ccc < 60)
			{
				argString[ccc++]	=	theChar;
				argString[ccc]		=	0;
			}
			else
			{
				CONSOLE_DEBUG_W_STR("Arg string overflow", argString);
			}
		}
	}

	//*	now we have all the data parsed, put it into the record
	raRadians	=	RADIANS(raDegrees * 15.0);
	declRadians	=	RADIANS(declDegrees);


	starRec->ra				=	raRadians;
	starRec->decl			=	declRadians;
	starRec->org_ra			=	raRadians;
	starRec->org_decl		=	declRadians;
	starRec->realMagnitude	=	magnitude;
	starRec->spectralClass	=	spectralClass;

	starRec->dataSrc		=	kDataSrc_Unknown;

	starRec->magn			=	ST_STAR;

	if (magnitude > 30.0)
	{
		CONSOLE_DEBUG_W_DBL("magnitude", magnitude);
		fprintf(stderr, "%s\r\n", lineBuff);
	}
//	CONSOLE_ABORT(__FUNCTION__);
}

//************************************************************************
TYPE_CelestData	*ReadHYGdata(const char *folderPath, int dataSource, long *objectCount)
{
TYPE_CelestData	*hygData;
FILE			*filePointer;
int				specifiedLnCnt;
int				recordCount;
char			lineBuff[2048];
size_t			bufferSize;
char			myFilePath[256];
int				linesRead;
int				skippedCount;
int				validMagCount;
int				zeroMagCount;
double			hightestMagValue;

	hygData				=	NULL;
	recordCount			=	0;
	skippedCount		=	0;
	validMagCount		=	0;
	zeroMagCount		=	0;
	hightestMagValue	=	0.0;
	strcpy(myFilePath, folderPath);
	strcat(myFilePath, "hygdata_v3.csv");

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, myFilePath);
	CONSOLE_DEBUG_W_NUM("kHYG_Last\t=", kHYG_Last);

	filePointer	=	fopen(myFilePath, "r");
	if (filePointer != NULL)
	{
//		CONSOLE_DEBUG("File Opened OK");
		specifiedLnCnt	=	119650;
		bufferSize		=	(specifiedLnCnt + 2) * sizeof(TYPE_CelestData);
		hygData			=	(TYPE_CelestData *)malloc(bufferSize);

		if (hygData != NULL)
		{
//			CONSOLE_DEBUG("Memory allocated OK");
			memset(hygData, 0, bufferSize);
			linesRead	=	0;
			while (fgets(lineBuff, 2000, filePointer) && (recordCount < specifiedLnCnt))
			{
				linesRead++;
				//*	we want to skip the 1st 2 lines
				if ((lineBuff[0] != '#') && (linesRead > 2))
				{
					ParseOneLineHYGdata(lineBuff, &hygData[recordCount]);
					hygData[recordCount].dataSrc	=	dataSource;
					if (hygData[recordCount].realMagnitude > hightestMagValue)
					{
						hightestMagValue	=	hygData[recordCount].realMagnitude;
//						CONSOLE_DEBUG_W_NUM("New high value at recordCount\t=", recordCount);
//						CONSOLE_DEBUG_W_DBL("hightestMagValue\t\t=", hightestMagValue);
					}
					if (hygData[recordCount].realMagnitude > 0.0)
					{
						validMagCount++;
					}
					else
					{
						zeroMagCount++;
					}

				//	if (recordCount < 3)
				//	{
				//		DumpCelestDataStruct(__FUNCTION__, &hygData[recordCount]);
				//		CONSOLE_DEBUG(lineBuff);
				//	}

					if (hygData[recordCount].id > 0)
					{
						recordCount++;

					//	if ((recordCount % 5000) == 0)
					//	{
					//		CONSOLE_DEBUG_W_NUM("HYG records read\t=", recordCount);
					//	}
					}
					else
					{
						DumpCelestDataStruct(__FUNCTION__, &hygData[recordCount]);
						skippedCount++;
					}
				}
			}
			*objectCount	=	recordCount;
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read:", myFilePath);
	}
	CONSOLE_DEBUG_W_NUM("HYG records read\t=", recordCount);
	CONSOLE_DEBUG_W_NUM("skippedCount    \t=", skippedCount);
	CONSOLE_DEBUG_W_NUM("Total lines     \t=", (recordCount + skippedCount));
	CONSOLE_DEBUG_W_NUM("validMagCount   \t=", validMagCount);
	CONSOLE_DEBUG_W_NUM("zeroMagCount    \t=", zeroMagCount);
	CONSOLE_DEBUG_W_DBL("hightestMagValue\t=", hightestMagValue);

//	CONSOLE_ABORT(__FUNCTION__);
	return(hygData);
}

static int	gPhotMagSubsitutionCnt	=	0;
static int	gHDstarsWithoutMag		=	0;

//************************************************************************
//*	#
//*	# Table Parameters
//*	#
//*	0	field[bii]				=	float4:.4f_degree		// Galactic Latitude
//*	1	field[class]			=	int2  (index)			// Browse Object Classification
//*	2	field[dec]				=	float8:.4f_degree (key) // Declination
//*	3	field[dircos1]			=	float8					// 1st Directional Cosine
//*	4	field[dircos2]			=	float8					// 2nd Directional Cosine
//*	5	field[dircos3]			=	float8					// 3rd Directional Cosine
//*	6	field[hd_number]		=	int4  (index)			// HD Catalog Number
//*	7	field[lii]				=	float4:.4f_degree		// Galactic Longitude
//*	8	field[multiplicity_flag]=	int2  (index)			// 0=single stars, 1=multiple systems
//*	9	field[name]				=	char20  (index)			// Object Designation
//*	10	field[pgmag]			=	float4  (index)			// Photographic Magnitude
//*	11	field[ra]				=	float8:.4f_degree (key) // Right Ascension
//*	12	field[spectral_type]	=	char4  (index)			// Spectral Type
//*	13	field[variability_flag]	=	int2  (index)			// Variability Flag: 0=not variable, 1=variable, 2=uncertain
//*	14	field[vmag]				=	float4:5.1f  (index)	// V Magnitude
//
//	line[1] = bii class dec dircos1 dircos2 dircos3 hd_number lii multiplicity_flag name pgmag ra spectral_type variability_flag vmag
//
//	-27.0404|2606|-89.833011026304|0|0|0|98784|302.772|0|HD98784|9.7|134.582368152346|K 0|0|8.7|
//	-26.9715|2306|-89.7836787384847|0|0|0|99685|302.765|0|HD99685|7.6|149.273661887492|A 0|0|7.6|
//	-26.9227|2706|-89.7713071321891|0|0|0|110994|303.044|0|HD110994|8.6|218.823086521289|M A|0|6.56|
//	22.0649|2656|77.0904069280486|0|0|0|193467|109.86|0|HD193467|-99.9|303.103701319968|K 5|0|-99.9|
//	19.0907|2556|81.9622539153399|0|0|0|4499|122.885|0|HD4499|-99.9|12.542015537301|G 5|1|-99.9|
//	photo_Magnitude	= -899136720
//************************************************************************
static void	ParseOneLineHenryDraperData(char *lineBuff, TYPE_CelestData *starRec)
{
int		sLen;
double	raDegrees	=	0;
double	raRadians	=	0;
double	declDegrees	=	0;
double	declRadians	=	0;
int		ii;
int		argNum;
char	argString[64];
char	theChar;
int		ccc;
char	spectralClass;
double	photo_Magnitude;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, lineBuff);

	spectralClass	=	0;
	photo_Magnitude	=	0.0;
	//*	step thru the line looking for "|" separator char.
	sLen		=	strlen(lineBuff);
	argNum		=	0;
	ccc			=	0;
	for (ii=0; ii <= sLen; ii++)
	{
		theChar	=	lineBuff[ii];
		if ((theChar == '|') || (theChar == 0))
		{
			//*	end of argument, do something with it
			switch(argNum)
			{
				case 2:				//*	declination
					declDegrees		=	atof(argString);
					break;

				case 6:				// HD Catalog Number
					starRec->id		=	atoi(argString);
					break;

				case 9:				// Object Designation
					strcpy(starRec->longName, argString);
					break;

				case 10:			// Photographic Magnitude
					photo_Magnitude		=	atof(argString);
					break;

				case 11:			//*	right ascension
					raDegrees		=	atof(argString);
					break;

				case 12:			//*	Spectral class
					spectralClass	=	argString[0];
					break;

				case 13:			// Variability Flag: 0=not variable, 1=variable, 2=uncertain
					starRec->variability		=	atoi(argString);
					break;

				case 14:			//*	V Magnitude
					starRec->realMagnitude		=	atof(argString);
					break;

			}
			argNum++;
			ccc				=	0;
			argString[0]	=	0;
		}
		else
		{
			if (ccc < 60)
			{
				argString[ccc++]	=	theChar;
				argString[ccc]		=	0;
			}
		}
	}
	//*	now we have all the data parsed, put it into the record
	raRadians				=	RADIANS(raDegrees);
	declRadians				=	RADIANS(declDegrees);

	starRec->ra				=	raRadians;
	starRec->decl			=	declRadians;
	starRec->org_ra			=	raRadians;
	starRec->org_decl		=	declRadians;
	starRec->dataSrc		=	kDataSrc_Unknown;
	starRec->spectralClass	=	spectralClass;

	starRec->magn			=	ST_STAR;


	if ((starRec->realMagnitude < 0.0001) && (photo_Magnitude > 0.0))
	{
		starRec->realMagnitude	=	photo_Magnitude;
		gPhotMagSubsitutionCnt++;
	}
	//*	debugging
	if (starRec->realMagnitude < 0.001)
	{
		starRec->realMagnitude	=	17.777;

//		CONSOLE_DEBUG(lineBuff);
//		CONSOLE_DEBUG_W_DBL("photo_Magnitude\t=", photo_Magnitude);
		gHDstarsWithoutMag++;
	}
}



//************************************************************************
TYPE_CelestData	*ReadHenryDraperCatalog(	const char	*folderPath,
											int			dataSource,
											long		*objectCount)
{
TYPE_CelestData	*draperData;
FILE			*filePointer;
int				specifiedLnCnt;
int				recordCount;
char			lineBuff[2048];
size_t			bufferSize;
char			myFilePath[256];
int				linesRead;
int				headerLineCnt;

	draperData	=	NULL;
	recordCount	=	0;

	strcpy(myFilePath, folderPath);
	strcat(myFilePath, "heasarc_hd.tdat");

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, myFilePath);

	filePointer	=	fopen(myFilePath, "r");
	if (filePointer != NULL)
	{
		specifiedLnCnt	=	CountLinesInFile(filePointer);
//		CONSOLE_DEBUG_W_NUM("Lines in file", specifiedLnCnt);

		bufferSize		=	specifiedLnCnt * sizeof(TYPE_CelestData);
		draperData		=	(TYPE_CelestData *)malloc(bufferSize);

		if (draperData != NULL)
		{
			//*	skip the header stuff
			headerLineCnt	=	0;
			while (fgets(lineBuff, 2000, filePointer) && (recordCount < specifiedLnCnt))
			{
				headerLineCnt++;
				if (strncmp(lineBuff, "<DATA>", 6) == 0)
				{
					break;
				}
			}
//			CONSOLE_DEBUG_W_NUM("headerLineCnt", headerLineCnt);

			memset(draperData, 0, bufferSize);
			linesRead	=	0;
			while (fgets(lineBuff, 2000, filePointer) && (recordCount < specifiedLnCnt))
			{
				linesRead++;
				if ((lineBuff[0] != '<') && (recordCount < specifiedLnCnt))
				{
					ParseOneLineHenryDraperData(lineBuff, &draperData[recordCount]);
					draperData[recordCount].dataSrc	=	dataSource;

				//	if (recordCount < 3)
				//	{
				//		DumpCelestDataStruct(__FUNCTION__, &draperData[recordCount]);
				//		CONSOLE_DEBUG(lineBuff);
				//	}

					recordCount++;
				}
			}

			*objectCount	=	recordCount;
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read:", myFilePath);
	}
	CONSOLE_DEBUG_W_NUM("HGC records read\t\t=", recordCount);
	CONSOLE_DEBUG_W_NUM("gPhotMagSubsitutionCnt\t=", gPhotMagSubsitutionCnt);
	CONSOLE_DEBUG_W_DBL("percent substitutions\t=", (gPhotMagSubsitutionCnt * 100.0) / recordCount);
	CONSOLE_DEBUG_W_DBL("percent without mag\t=", (gHDstarsWithoutMag * 100.0) / recordCount);

//	CONSOLE_ABORT(__FUNCTION__);
	return(draperData);
}

//************************************************************************
//	#Name<tab>RA<TAB>Dec
//	#J2000
//	Aphopis-1800	09	19	17.62	-6	16	06.7
//************************************************************************
static void	ParseOneLineSpecialData(char *lineBuff, TYPE_CelestData *starRec)
{
int		sLen;
int		raHour		=	0;
int		raMin		=	0;
double	raSec		=	0;
double	raDegrees	=	0;
double	raRadians	=	0;

char	deSign		=	0;
int		deDeg		=	0;
int		deMin		=	0;
double	deSec		=	0;
double	declDegrees	=	0;
double	declRadians	=	0;

int		ii;
int		argNum;
char	argString[32];
char	theChar;
int		ccc;
int		argLen;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, lineBuff);

	//*	step thru the line looking for commas.
	sLen		=	strlen(lineBuff);
	argNum		=	0;
	ccc			=	0;
	for (ii=0; ii <= sLen; ii++)
	{
		theChar	=	lineBuff[ii];
		if ((theChar == 0x09) || (theChar == 0))
		{
//			CONSOLE_DEBUG_W_STR("argString=", argString);

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


				case 1:	//*	raHour
					raHour	=	atoi(argString);
					break;

				case 2:	//*	raMin
					raMin	=	atoi(argString);
					break;

				case 3:	//*	raSec
					raSec	=	atof(argString);
					break;

				//--------------
				case 4:	//*	degrees sign
					deSign	=	argString[0];
					deDeg	=	fabs(atoi(argString));
					break;

				case 5:	//*	deMin
					deMin	=	atoi(argString);
					break;

				case 6:	//*	deSec
					deSec	=	atof(argString);
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


	if (deSign == '-')
	{
		declRadians		=	-declRadians;
	}

	starRec->ra				=	raRadians;
	starRec->decl			=	declRadians;
	starRec->org_ra			=	raRadians;
	starRec->org_decl		=	declRadians;
	starRec->dataSrc		=	kDataSrc_Unknown;
	starRec->realMagnitude	=	0.0;
	starRec->spectralClass	=	0;

	starRec->magn			=	ST_STAR;

#if 0
	CONSOLE_DEBUG_W_STR("Name  \t=", starRec->longName);
	CONSOLE_DEBUG_W_NUM("raHour\t=", raHour);
	CONSOLE_DEBUG_W_NUM("raMin \t=", raMin);
	CONSOLE_DEBUG_W_DBL("raSec \t=", raSec);

	CONSOLE_DEBUG_W_HEX("deSign\t=", deSign);

	CONSOLE_DEBUG_W_NUM("deDeg \t=", deDeg);
	CONSOLE_DEBUG_W_NUM("deMin \t=", deMin);
	CONSOLE_DEBUG_W_DBL("deSec \t=", deSec);
#endif
}


//************************************************************************
static void	ParseOneLineJPLhorizons(char *lineBuff, TYPE_CelestData *starRec)
{
int		raHour		=	0;
int		raMin		=	0;
double	raSec		=	0;
double	raHoursDBL	=	0;
double	raRadians	=	0;

char	deSign		=	0;
int		deDeg		=	0;
int		deMin		=	0;
double	deSec		=	0;
double	declDegrees	=	0;
double	declRadians	=	0;


	CONSOLE_DEBUG_W_STR(__FUNCTION__, lineBuff);
//#	0			1		2			3		4			5
//#123456789 123456789 123456789 123456789 123456789 123456789
// 2021-Mar-07 00:00 A   09 22 31.49 -06 52 17.0  16.207   3.981 0.11268507492183  -0.1037108 150.5830 /T  26.5111

	strncpy(starRec->longName, &lineBuff[1], 17);
	lineBuff[17]	=	0;
	raHour			=	atoi(&lineBuff[23]);
	raMin			=	atoi(&lineBuff[26]);
	raSec			=	atof(&lineBuff[29]);
	deSign			=	lineBuff[35];
	deDeg			=	atoi(&lineBuff[36]);
	deMin			=	atoi(&lineBuff[39]);
	deSec			=	atof(&lineBuff[42]);

	CONSOLE_DEBUG_W_NUM("raHour\t=", raHour);
	CONSOLE_DEBUG_W_NUM("raMin \t=", raMin);
	CONSOLE_DEBUG_W_DBL("raSec \t=", raSec);

	CONSOLE_DEBUG_W_HEX("deSign\t=", deSign);

	CONSOLE_DEBUG_W_NUM("deDeg \t=", deDeg);
	CONSOLE_DEBUG_W_NUM("deMin \t=", deMin);
	CONSOLE_DEBUG_W_DBL("deSec \t=", deSec);

	//*	now we have all the data parsed, put it into the record
	raHoursDBL	=	raHour;
	raHoursDBL	+=	raMin / 60.0;
	raHoursDBL	+=	raSec / 3600.0;
//	raRadians	=	(raHoursDBL / 24.0) * (2.0 * M_PI);	//*	0->24 not 0->360
	raRadians	=	RADIANS(raHoursDBL * 15.0);	//*	0->24 not 0->360

	declDegrees	=	deDeg;
	declDegrees	+=	deMin / 60.0;
	declDegrees	+=	deSec / 3600.0;

	if (deSign == '-')
	{
		declDegrees		=	-declDegrees;
//		CONSOLE_DEBUG_W_DBL("declDegrees\t=", declDegrees);
	}
	declRadians	=	RADIANS(declDegrees);

	starRec->ra			=	raRadians;
	starRec->decl		=	declRadians;
	starRec->org_ra		=	raRadians;
	starRec->org_decl	=	declRadians;
	starRec->dataSrc	=	kDataSrc_Unknown;

	starRec->magn		=	ST_STAR;

}

#ifdef _DEBUG_STAR_DATA_
//************************************************************************
static void	DumpCelestralDataArray(TYPE_CelestData *starData, long recordCount)
{
int		iii;
double	hoursValue;
double	degreesValue;
char	raString[64];
char	decString[64];

	CONSOLE_DEBUG(__FUNCTION__);

	for (iii=0; iii<recordCount; iii++)
	{
		hoursValue		=	DEGREES(starData[iii].ra) / 15.0;
		degreesValue	=	DEGREES(starData[iii].decl);
		FormatHHMMSS(hoursValue,	raString, false);
		FormatHHMMSS(degreesValue,	decString, true);

		printf("%s\t%s\t%s\r\n",	starData[iii].longName,
									raString,
									decString);
	}
}
#endif // _DEBUG_STAR_DATA_

//************************************************************************
TYPE_CelestData	*ReadSpecialData(int dataSource, long *objectCount)
{
TYPE_CelestData	*specialData;
FILE			*filePointer;
int				specifiedLnCnt;
int				recordCount;
char			lineBuff[256];
size_t			bufferSize;
char			myFilePath[256];
int				linesRead;

	specialData	=	NULL;
	recordCount	=	0;

	strcpy(myFilePath, "special.txt");

	CONSOLE_DEBUG_W_STR(__FUNCTION__, myFilePath);

	filePointer	=	fopen(myFilePath, "r");
	if (filePointer != NULL)
	{
		specifiedLnCnt	=	CountLinesInFile(filePointer);
		CONSOLE_DEBUG_W_NUM("Lines in file", specifiedLnCnt);

		bufferSize		=	specifiedLnCnt * sizeof(TYPE_CelestData);
		specialData		=	(TYPE_CelestData *)malloc(bufferSize);

		if (specialData != NULL)
		{
			memset(specialData, 0, bufferSize);
			linesRead	=	0;
			while (fgets(lineBuff, 200, filePointer) && (recordCount < specifiedLnCnt))
			{
				linesRead++;
				if (strncasecmp(lineBuff, "!EXIT", 5) == 0)
				{
					break;
				}
				else if ((strlen(lineBuff) > 50) && (lineBuff[0] == 0x20))
				{
					CONSOLE_DEBUG_W_STR("lineBuff\t=", lineBuff);
					ParseOneLineJPLhorizons(lineBuff, &specialData[recordCount]);
					specialData[recordCount].dataSrc	=	dataSource;

					recordCount++;
				}
				else if ((lineBuff[0] != '#') && (lineBuff[0] != 0x09)
						&& (recordCount < specifiedLnCnt))
				{
					if (strlen(lineBuff) > 20)
					{
						ParseOneLineSpecialData(lineBuff, &specialData[recordCount]);
						specialData[recordCount].dataSrc	=	dataSource;

					//	if (recordCount < 3)
					//	{
					//		DumpCelestDataStruct(__FUNCTION__, &specialData[recordCount]);
					//		CONSOLE_DEBUG(lineBuff);
					//	}
						recordCount++;
					}
				}
			}
			*objectCount	=	recordCount;
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read:", myFilePath);
	}
//	DumpCelestralDataArray(specialData, recordCount);
//	CONSOLE_DEBUG_W_NUM("Special records read\t=", recordCount);
//	CONSOLE_ABORT(__FUNCTION__);

	return(specialData);
}
