//*****************************************************************************
//*	ConstellationData.c
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jan  5,	2021	<MLS> Added ReadConstellationOutlines()
//*	Jan  7,	2021	<MLS> Created ConstellationData.c
//*	Jan  7,	2021	<MLS> Downloaded much better constellation data
//*	Jan  7,	2021	<MLS> From https://github.com/dcf21/constellation-stick-figures
//*****************************************************************************


#include	<string.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<math.h>
#include	<ctype.h>

#include	"alpacadriver_helper.h"

//*	MLS Libraries
#define _ENABLE_CONSOLE_DEBUG_
#define	_DEBUG_TIMING_
#include	"ConsoleDebug.h"

#include	"ConstellationData.h"
#include	"HipparcosCatalog.h"


#define	RADIANS(degrees)	(degrees * M_PI / 180.0)
#define	DEGREES(radians)	(radians * 180.0 / M_PI)



static	TYPE_CelestData	*gHippData	=	NULL;
static	long			gHippCount	=	0;
static	int				gFailedToFindCnt	=	0;
//************************************************************************
static int	FindHipparcosIndexFromID(int hippId)
{
int		hippIndex;
int		iii;

	hippIndex	=	-1;
	if (gHippData != NULL)
	{
		for (iii=0; iii<gHippCount; iii++)
		{
			if (hippId == gHippData[iii].id)
			{
				hippIndex	=	iii;
				break;
			}
		}
	}
	return(hippIndex);
}

//************************************************************************
static void	UpdateOneConstellation(TYPE_ConstVector *constellation)
{
int		iii;
int		hippIndex;

//	CONSOLE_DEBUG(__FUNCTION__);

	for (iii=0; iii<constellation->starCount; iii++)
	{
		hippIndex	=	FindHipparcosIndexFromID(constellation->hippStars[iii].hippIdNumber);
		if (hippIndex >= 0)
		{
			constellation->hippStars[iii].rtAscension	=	gHippData[hippIndex].org_ra;
			constellation->hippStars[iii].declination	=	gHippData[hippIndex].org_decl;
		}
		else
		{
			//*	since we couldnt find the star, make it a move
			constellation->hippStars[iii].moveFlag	=	true;
			gFailedToFindCnt++;
			CONSOLE_DEBUG_W_STR("-----", constellation->longName);
		}
	}
}

//************************************************************************
static void	UpdateFromHipparcos(TYPE_ConstVector *constelVectors, int vectorCnt)
{
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	//*	first we have to read in the hipparcos catalog
	gHippData	=	ReadHipparcosStarCatalog(&gHippCount);
	if (gHippData != NULL)
	{
		for (iii=0; iii<vectorCnt; iii++)
		{
			UpdateOneConstellation(&constelVectors[iii]);
		}
		free(gHippData);
	}
//	CONSOLE_DEBUG_W_NUM("gFailedToFindCnt\t=", gFailedToFindCnt);
}


//************************************************************************
static int	ParseIntegerList(const char *lineBuff, int *intergerArray, int maxIntgers)
{
int			lineLength;
int			iii;
int			ccc;
int			integerCount;
char		newLineBuffer[128];
char		*argPtr;
int			currentInt;

	//*	this is what we are dealing with, go thru the line and clean it up
	//["62322", "61585", "61199", "63613", "61585", "59929", "57363"]
	//["677*", "1067", "113963", "113881", "677*"]
	//*	not sure what the "*" means
	integerCount	=	0;
	lineLength		=	strlen(lineBuff);
	ccc				=	0;
	for (iii=0; iii<lineLength; iii++)
	{
		if (isdigit(lineBuff[iii]) || (lineBuff[iii] == ',') || (lineBuff[iii] == '*'))
		{
			newLineBuffer[ccc++]	=	lineBuff[iii];
			newLineBuffer[ccc]		=	0;
		}
	}

//	CONSOLE_DEBUG_W_STR("newLineBuffer\t=", newLineBuffer);
	argPtr		=	newLineBuffer;
	while ((argPtr != NULL) && (integerCount < maxIntgers))
	{
		currentInt						=	atoi(argPtr);
		intergerArray[integerCount++]	=	currentInt;
//		CONSOLE_DEBUG_W_NUM("currentInt\t=", currentInt);

		//*	advance to the next one in the list
		argPtr		=	strchr(argPtr, ',');
		if (argPtr != NULL)
		{
			argPtr++;	//*	skip the comma
		}
	}

	return(integerCount);
}

//************************************************************************
TYPE_ConstVector	*ReadConstellationVectorsFile(const char *filePath, int *objectCount)
{
FILE				*filePointer;
int					iii;
int					constelIdx;
int					starIdx;
TYPE_ConstVector	*constelVectorData;
char				lineBuff[128];
char				currentName[128];
size_t				bufferSize;
int					lineLength;
int					linesRead;
char				*argPtr;
int					intgerArray[20];
int					integerCount;
bool				moveFlag;

	CONSOLE_DEBUG_W_STR("Reading:", filePath);

	constelVectorData	=	NULL;
	*objectCount		=	0;
	filePointer			=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
		bufferSize			=	kConstOutlineCnt * sizeof(TYPE_ConstVector);
		constelVectorData	=	(TYPE_ConstVector *)malloc(bufferSize);

		if (constelVectorData != NULL)
		{
			memset(constelVectorData, 0, bufferSize);
			strcpy(currentName, "");

			linesRead	=	0;
			constelIdx	=	-1;
			starIdx		=	0;
			while (fgets(lineBuff, 100, filePointer) && (constelIdx < kConstOutlineCnt))
			{
				linesRead++;
				lineLength	=	strlen(lineBuff);
				//*	get rid of trailing CR/LF and spaces
				while ((lineBuff[lineLength - 1] <= 0x20) && (lineLength > 0))
				{
					lineBuff[lineLength - 1]	=	0;
					lineLength	=	strlen(lineBuff);
				}


				if ((lineBuff[0] == '#') || (lineLength == 0))
				{
					//*	ignore the line, its a comment
				}
				else if (lineBuff[0] == '*')
				{
					//*	we have a new constellation
					constelIdx++;
					argPtr	=	lineBuff;
					argPtr++;				//*	skip the "*"
					while (*argPtr == 0x20)
					{
						argPtr++;
					}
					strcpy(currentName, argPtr);
					starIdx		=	0;
//					CONSOLE_DEBUG_W_STR("currentName\t=", currentName);

					//*	put the data into the array
					if (constelIdx < kConstOutlineCnt)
					{
						strcpy(constelVectorData[constelIdx].longName, currentName);
					}
				}
				else if (lineBuff[0] == '[')
				{
					integerCount	=	ParseIntegerList(lineBuff, intgerArray, 20);
					if (constelIdx < kConstOutlineCnt)
					{
						//*	copy the integers to the array
						moveFlag	=	true;
						for (iii=0; iii<integerCount; iii++)
						{
							if (starIdx < kMaxConstVecotrPts)
							{
								constelVectorData[constelIdx].hippStars[starIdx].moveFlag		=	moveFlag;
								constelVectorData[constelIdx].hippStars[starIdx].hippIdNumber	=	intgerArray[iii];

								starIdx++;
							}
							else
							{
								CONSOLE_DEBUG_W_NUM("Exceeded star limit\t=", starIdx);
							//	CONSOLE_ABORT(__FUNCTION__);
							}
							constelVectorData[constelIdx].starCount	=	starIdx;
							moveFlag	=	false;
						}
//						CONSOLE_DEBUG_W_NUM("Star count\t=", starIdx);
					}

				}
			}
			*objectCount	=	constelIdx;
//			CONSOLE_DEBUG_W_NUM("total constellations read\t=", constelIdx);

		}
		fclose(filePointer);

	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read:", filePath);
	}
//	CONSOLE_DEBUG_W_NUM("linesRead\t=", linesRead);

	return(constelVectorData);
}

//************************************************************************
static int	FindConstVectIdxByName(TYPE_ConstVector *vectorList, int objectCount, const char *searchName)
{
int		foundIdx;
int		iii;

	foundIdx	=	-1;
	iii			=	0;
	while ((foundIdx < 0) && (iii < objectCount))
	{
		if (strcasecmp(searchName, vectorList[iii].longName) == 0)
		{
			foundIdx	=	iii;
		}
		iii++;
	}
	return(foundIdx);
}

//************************************************************************
//*	returns 1 if replace occurred,
//*	0 if not
//************************************************************************
static int	ReplaceVectorDef(	TYPE_ConstVector *destVetorLst, int destVectorCnt,
								TYPE_ConstVector *srcVetorLst, int srcVectorCnt,
								const char *objectName)
{
int		srcIdx;
int		destIdx;
int		substituteCnt;

	substituteCnt	=	0;
	destIdx			=	FindConstVectIdxByName(destVetorLst, destVectorCnt, objectName);
	srcIdx			=	FindConstVectIdxByName(srcVetorLst, srcVectorCnt, objectName);
	if ((destIdx > 0) && (srcIdx > 0))
	{
		destVetorLst[destIdx]	=	srcVetorLst[srcIdx];
		substituteCnt++;
	}
	return(substituteCnt);
}
//************************************************************************
//*	we are going to read the simplified list,
//*	then read the full list and replace some of the definitions from one to the other
//*	if ether is missing, then the one is present will be used
//*	if both are missing, NULL is returned
//************************************************************************
TYPE_ConstVector	*ReadConstellationVectors(const char *directoryPath, int *objectCount)
{
char				fullFilePath[128];
TYPE_ConstVector	*constVectors_simp		=	NULL;
TYPE_ConstVector	*constVectors_rey		=	NULL;
TYPE_ConstVector	*constVectors_return	=	NULL;
int					objectCount_simp;
int					objectCount_rey;
int					objectCount_return;
int					subCnt;

	CONSOLE_DEBUG_W_STR(__FUNCTION__, directoryPath);

	SETUP_TIMING();

	*objectCount	=	0;

	strcpy(fullFilePath, directoryPath);
	strcat(fullFilePath, "/");
	strcat(fullFilePath, "constellation_lines_simplified.dat");

	constVectors_simp	=	ReadConstellationVectorsFile(fullFilePath, &objectCount_simp);

	strcpy(fullFilePath, directoryPath);
	strcat(fullFilePath, "/");
	strcat(fullFilePath, "constellation_lines_rey.dat");
	constVectors_rey	=	ReadConstellationVectorsFile(fullFilePath, &objectCount_rey);

	if ((constVectors_simp != NULL) && (constVectors_rey != NULL))
	{
//		CONSOLE_DEBUG("We have both files, starting with substitution");
		//*	ok, do the substitutions
		//------------------------------------------
		subCnt	=	0;

		subCnt	+=	ReplaceVectorDef(constVectors_simp, objectCount_simp, constVectors_rey, objectCount_rey, "ComaBerenices");
		subCnt	+=	ReplaceVectorDef(constVectors_simp, objectCount_simp, constVectors_rey, objectCount_rey, "Draco");
		subCnt	+=	ReplaceVectorDef(constVectors_simp, objectCount_simp, constVectors_rey, objectCount_rey, "Gemini");
		subCnt	+=	ReplaceVectorDef(constVectors_simp, objectCount_simp, constVectors_rey, objectCount_rey, "Leo");
		subCnt	+=	ReplaceVectorDef(constVectors_simp, objectCount_simp, constVectors_rey, objectCount_rey, "Ophiuchus");
		subCnt	+=	ReplaceVectorDef(constVectors_simp, objectCount_simp, constVectors_rey, objectCount_rey, "Perseus");
		subCnt	+=	ReplaceVectorDef(constVectors_simp, objectCount_simp, constVectors_rey, objectCount_rey, "Vela");

//		CONSOLE_DEBUG_W_NUM("Definitions substituted\t=", subCnt);
		free(constVectors_rey);
		constVectors_rey	=	NULL;
		constVectors_return	=	constVectors_simp;
		objectCount_return	=	objectCount_simp;
	}
	else if (constVectors_simp != NULL)
	{
		constVectors_return	=	constVectors_simp;
		objectCount_return	=	objectCount_simp;
	}
	else if (constVectors_rey != NULL)
	{
		constVectors_return	=	constVectors_rey;
		objectCount_return	=	objectCount_rey;
	}

	//-------------------------------------------------------------
	if ((constVectors_return != NULL) && (objectCount_return > 0))
	{
		UpdateFromHipparcos(constVectors_return, objectCount_return);
		*objectCount	=	objectCount_return;
	}
	else
	{
		CONSOLE_DEBUG("out of luck..............");
	}

	DEBUG_TIMING("Elapsed millisconds=");
	return(constVectors_return);

}




//************************************************************************
static int	FindConstOutlineIdx(const char *theShortName, TYPE_ConstOutline	*constelOutlineData)
{
int		recordIdx;
int		iii;

	recordIdx	=	-1;
	iii			=	0;
	while ((recordIdx < 0) && (strlen(constelOutlineData[iii].shortName) > 0) && (iii < kConstOutlineCnt))
	{
		if (strcmp(theShortName, constelOutlineData[iii].shortName) == 0)
		{
			recordIdx	=	iii;
		}
		iii++;
	}
	return(recordIdx);
}

//************************************************************************
static void	ParseConstOutlineLongName(char *lineBuff, TYPE_ConstOutline	*constelOutlineData)
{
int		recordIdx;
int		jjj;
int		ccc;
char	myShortName[8];
char	myLongName[64];

//	CONSOLE_DEBUG_W_STR("lineBuff\t=", lineBuff);
	if (lineBuff[0] == '+')
	{
		//*	truncate the line at column 22
		lineBuff[22]	=	0;
		ccc	=	21;
		while ((lineBuff[ccc] == 0x20) && (ccc > 4))
		{
			//*	get rid of trailing spaces
			lineBuff[ccc]	=	0;
			ccc--;
		}
		jjj	=	1;
		ccc	=	0;
		while ((lineBuff[jjj] != '=') && (ccc < 4))
		{
			myShortName[ccc++]	=	lineBuff[jjj];
			myShortName[ccc]	=	0;
			jjj++;
		}
		jjj++;	//*	increment past the '='
		ccc	=	0;
		while ((lineBuff[jjj] > 0x20) && (ccc < 32))
		{
			myLongName[ccc++]	=	lineBuff[jjj];
			myLongName[ccc]	=	0;
			jjj++;
		}

		recordIdx	=	FindConstOutlineIdx(myShortName, constelOutlineData);
		//*	did we find it?
		if (recordIdx >= 0)
		{
			strcpy(constelOutlineData[recordIdx].longName, myLongName);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_STR("We should't be here\t=", lineBuff);
	}
}



//************************************************************************
static void	ParseConstOutlineCenter(const char *lineBuff, TYPE_ConstOutline	*constelOutlineData)
{
int		recordIdx;
int		jjj;
int		ccc;
char	myShortName[8];
double	myRtAscen;
double	myDeclina;

	if (lineBuff[0] == '=')
	{
//		CONSOLE_DEBUG_W_STR("lineBuff\t=", lineBuff);
		jjj	=	1;
		ccc	=	0;
		while ((lineBuff[jjj] != '=') && (ccc < 4))
		{
			myShortName[ccc++]	=	lineBuff[jjj];
			myShortName[ccc]	=	0;
			jjj++;
		}
//		CONSOLE_DEBUG_W_STR("myShortName\t=", myShortName);

		//	0123456789 123459789
		//	=PEG=23.5000:22.0000
		myRtAscen	=	atof(&lineBuff[5]);
		myDeclina	=	atof(&lineBuff[13]);
//		CONSOLE_DEBUG_W_DBL("myRtAscen\t=", myRtAscen);
//		CONSOLE_DEBUG_W_DBL("myDeclina\t=", myDeclina);

		recordIdx	=	FindConstOutlineIdx(myShortName, constelOutlineData);
		//*	did we find it?
		if (recordIdx >= 0)
		{
			constelOutlineData[recordIdx].rtAscension	=	RADIANS(myRtAscen * 15.0);
			constelOutlineData[recordIdx].declination	=	RADIANS(myDeclina);
		}
	}
}



//************************************************************************
double	DegreesFromHHMMSS(double hours, double minutes, double seconds)
{
double	myDegrees;

	myDegrees	=	hours;
	myDegrees	+=	minutes / 60.0;
	myDegrees	+=	seconds / 3600.0;
	return(myDegrees);
}


//************************************************************************
static void	FindCostellationCenter(TYPE_ConstOutline *constelObj)
{
int		iii;
double	min_rtAscn;
double	max_rtAscn;
double	min_decl;
double	max_decl;


	min_rtAscn	=	99.0;
	max_rtAscn	=	-99.0;
	min_decl	=	99.0;
	max_decl	=	-99.0;
	iii	=	0;
	while (constelObj->path[iii].rtAscension > 0.0)
	{

		if (constelObj->path[iii].rtAscension < min_rtAscn)
		{
			min_rtAscn	=	constelObj->path[iii].rtAscension;
		}
		if (constelObj->path[iii].rtAscension > max_rtAscn)
		{
			max_rtAscn	=	constelObj->path[iii].rtAscension;
		}
		if (constelObj->path[iii].declination < min_decl)
		{
			min_decl	=	constelObj->path[iii].declination;
		}
		if (constelObj->path[iii].declination > max_decl)
		{
			max_decl	=	constelObj->path[iii].declination;
		}
		iii++;
	}

	constelObj->rtAscension	=	min_rtAscn + ((max_rtAscn - min_rtAscn) / 2.0);
	constelObj->declination	=	min_decl + ((max_decl - min_decl) / 2.0);


//	CONSOLE_DEBUG_W_STR("name\       t=", constelObj->shortName);
//	CONSOLE_DEBUG_W_DBL("rtAscension\t=", constelObj->rtAscension);
//	CONSOLE_DEBUG_W_DBL("declination\t=", constelObj->declination);
}

//************************************************************************
//*	11 56 16.9843|-55.6957932|CRU
//*	11 56 13.7673|-64.6957855|CRU
//*	12 57 45.2113|-64.6769638|CRU
//*	12 57 20.2827|-55.6771049|CRU
//************************************************************************
TYPE_ConstOutline	*ReadConstellationOutlines(const char *filePath, int *objectCount)
{
FILE				*filePointer;
int					iii;
int					recordIdx;
int					coordIdx;
TYPE_ConstOutline	*constelOutlineData;
char				lineBuff[128];
size_t				bufferSize;
double				rtAscension_Deg;
double				declination_Deg;
double				hours;
double				minutes;
double				seconds;
char				name[32];
char				currentName[32];
TYPE_RaDec			linePoint;


//	CONSOLE_DEBUG_W_STR(__FUNCTION__, filePath);

	constelOutlineData	=	NULL;
	*objectCount		=	0;
	filePointer			=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
		bufferSize			=	kConstOutlineCnt * sizeof(TYPE_ConstOutline);
		constelOutlineData	=	(TYPE_ConstOutline *)malloc(bufferSize);

		if (constelOutlineData != NULL)
		{
			memset(constelOutlineData, 0, bufferSize);
			strcpy(currentName, "");

			recordIdx	=	0;
			coordIdx	=	0;
			while (fgets(lineBuff, 100, filePointer) && (recordIdx < kConstOutlineCnt))
			{
				if (isdigit(lineBuff[0]) && (lineBuff[13] == '|') && (lineBuff[25] == '|'))
				{
					lineBuff[30]	=	0;
					//*	0123456789 123456789 1234567890
					//*	10 20 43.5185|-29.7947845|ANT
					//*	12 57 20.2827|-55.6771049|CRU
					hours			=	atof(lineBuff);
					minutes			=	atof(&lineBuff[3]);
					seconds			=	atof(&lineBuff[6]);
					rtAscension_Deg	=	DegreesFromHHMMSS(hours, minutes, seconds);
					declination_Deg	=	atof(&lineBuff[14]);

					strncpy(name, &lineBuff[26], 4);
					name[4]	=	0;
					if (name[3] <= 0x20)
					{
						name[3]	=	0;
					}

					//*	convert to radians (rtAsce 0->24)
					linePoint.rtAscension	=	RADIANS(15.0 * rtAscension_Deg);
					linePoint.declination	=	RADIANS(declination_Deg);

					//*	is it different than before
					if (strcmp(name, currentName) != 0)
					{
					//	CONSOLE_DEBUG_W_STR("name\t=", name);
						if (strlen(currentName) > 0)
						{
							//*	we have a new record entry
							recordIdx++;
							coordIdx	=	0;
							strcpy(currentName, name);
						}
						else
						{
							//*	this will happen on the very first entry
							strcpy(currentName, name);
						}
					}
					//*	this should ALWAYS be true
					if (strcmp(name, currentName) == 0)
					{
						if (recordIdx < kConstOutlineCnt)
						{
							strcpy(constelOutlineData[recordIdx].shortName, name);
							if (coordIdx < kMaxConstPointCnt)
							{
								constelOutlineData[recordIdx].path[coordIdx]	=	linePoint;
								coordIdx++;
							}
							else
							{
								CONSOLE_ABORT("kMaxConstPointCnt exceeded");
							}
						}
						else
						{
							CONSOLE_ABORT("kConstOutlineCnt exceeded");
						}
					}
					else
					{
						CONSOLE_ABORT("screwed up some place");
					}
				}
				else if (lineBuff[0] == '+')
				{
					lineBuff[39]	=	0;
					ParseConstOutlineLongName(lineBuff, constelOutlineData);
				}
				else if (lineBuff[0] == '=')
				{
					ParseConstOutlineCenter(lineBuff, constelOutlineData);
				}
			}
			*objectCount	=	recordIdx;
//			CONSOLE_DEBUG_W_NUM("total constellations read\t=", recordIdx);

			//*	find centers
			for (iii=0; iii<recordIdx; iii++)
			{
				//*	check to see if its already det
				if (constelOutlineData[iii].rtAscension == 0.0)
				{
				//	CONSOLE_DEBUG_W_STR("FindCostellationCenter\t=", constelOutlineData[iii].shortName);
					FindCostellationCenter(&constelOutlineData[iii]);
				}
			}
		}
		fclose(filePointer);


	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read:", filePath);
	}
//	CONSOLE_ABORT("foo")
	return(constelOutlineData);
}

