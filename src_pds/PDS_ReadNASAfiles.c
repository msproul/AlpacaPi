//*****************************************************************************
//*	File: PDS_ReadNASAfiles.c
//*	Voyager Image Decompression Program
//*	Adapted by Mark Sproul
//*	January 1992
//*	for using in my graphics programs
//*
//*	The orginal source code was obtained from the CD-ROMs
//*		"NASA Voyagers to the Outer Planets"
//*		"USA_NASA_PDS_VG_001"
//*
//*	Routine "OpenVoyagerPDSfile" (C) by Mark Sproul, 1992
//*		"OpenVoyagerPDSfile" opens a file and reads it into
//*		a Macintosh OffScreen bit map.
//*
//*	Routine	"OpenPDSdatabaseFile" (C) by Mark Sproul, 1992
//*		"OpenPDSdatabaseFile" opens a database file from the CD-ROM
//*		about the PDS data files
//*
//*	Many other support routines are also (C) by Mark Sproul, 1992
//*
//*	Some of the other code is from the CD-ROM and is believed to be
//*	in the public domain.
//*
//*****************************************************************************
//*	Edit history
//*****************************************************************************
//*	Jan 26,	1992	Reading of PDS files into Mac program from CD working
//*	Mar 14,	1992	Starting on more detailed interpretation of label (header)
//*	Mar 16,	1992	Changed number of displayed bytes from 836 to 800
//*	Apr  4,	1992	Working on text window again.
//*	Apr  4,	1992	Text from labels and headers reading into text windows nicely
//*	Apr 19,	1992	Working on database window etc.
//*	Apr 20,	1992	PDS database file working fantastically
//*	Apr 21,	1992	Cleaning up and bullet proofing the database window stuff
//*	Jun 22,	1992	Received 60 CD-ROMs from NASA
//*						Vol	9-12 of Voyager
//*						Vol 1-8  of Viking Orbiter of Mars
//*						Vol 1-48 of MaMagellan Mission to Venus
//*	Jun 22,	1992	Reading Viking Browse files (300 x 264)
//*	Jun 22,	1992	Reading Magellan "F" files (uncompressed)
//*	Jun 25,	1992	Working real good with Viking and Magellan CD-ROMs
//*	Jun 25,	1992	Crash occasionally when reading .TAB files
//*	Jun 25,	1992	Working on more .TAB handling (data base stuff)
//*	Jul  9,	1992	Feature database working
//*	Jul 10,	1992	Multiple databases open at the same time working
//*	Jul 11,	1992	Added code to close window routine to dispose of the database ptrs
//*	Sep  7,	1992	Started implemented"Open PDS Database" menu,	auto dim working
//*	Nov  6,	1992	Minor bugs fixed in voyager browse stuff
//*	Apr 15,	1993	Received 47 CD-ROMs from NASA
//*						Vol	49-67     of Magellan Mission to Venus
//*						Vol	2001-2013 of Magellan Mission to Venus
//*						Vol 2001-2013 of Viking Orbiter of Mars
//*	Nov 17,	2022	<MLS> Resurrected  PDS file code to run on Linux
//*	Nov 17,	2022	<MLS> Added PDS_ReadImage()
//*	Nov 18,	2022	<MLS> Magellan MG_0xxx (.img) images working
//*	Nov 18,	2022	<MLS> Viking VO_2xxx (.img) images working
//*	Nov 19,	2022	<MLS> Voyager VG_00xx (.img) images working
//*	Nov 20,	2022	<MLS> Galileo GO_00xx (.img) images working
//*****************************************************************************


#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<stdint.h>
#include	<stdbool.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	nil	NULL
#include	"PDS.typedefs.h"
#include	"PDS_decompress.h"

#include	"PDS_ReadNASAfiles.h"



//#include	<TextEdit.h>
//#include	"PICTprocessor.h"
//#include	"PICTprocessor.menus.h"
//#include	"PICTprocessor.typedefs.h"
//#include	"PICTprocessor.dialogs.h"
//#include	"PICTprocessor.globals.h"
//#include	"PP.prototypes.h"
//#include	"TextWindow.h"
//#include	"PDS.prototypes.h"

int	gImagesRead	=	0;

//*****************************************************************************
//*	C-language Considerations
//*	-------------------------
//*
//*	The C-language versions of the decompression subroutines are more
//*	portable than the equivalent FORTRAN versions. These routines have
//*	been tested on a VAX 750 running under version 4.6 of VMS, a
//*	Micro-VAX running under version 2.2 of ULTRIX, a SUN workstation
//*	running under version 4.2 (release 3.4) of UNIX, and an IBM PC
//*	running under MSDOS using versions 4.0 and 3.0 of MICROSOFT C.
//*
//*	The C-language version of the decompression software resides in the
//*	DECOMP.C file.  This file contains the decmpinit and decompress
//*	routines, and the working routines dcmprs, huff_tree, and sort_freq.
//*	Also, there is a main program, located in the DETEST.C file, which
//*	tests the performance of the decompression software.  If you are
//*	adapting the software to your paricular hardware and operating system
//*	environment, then this program will be useful for testing the
//*	software.
//*
//*****************************************************************************
//*****************************************************************************
//*
//*	hist   - Buffer to contain 511 elements of the encoding histogram.
//*	         The encoding histogram is extracted from the image area.
//*	nsi    - Number of bytes obtained from the read of a compressed
//*	         line.
//*	nso    - Number of output samples after decompression. For Voyager
//*	         images, this value is 836.
//*	linei  - Buffer containing the input compressed line.
//*	lineo  - Buffer to contain the restored line after decompression.
//*	nl     - Number of lines in the image array.  For Voyager images,
//*	         this value is 800.
//*	il     - Loop counter for processing image lines.
//******************************************************************
#define		VOYAGER_LINES			800
#define		VOYAGER_PIXELS			800
#define		VOYAGER_RECORD_BYTES	836

//*	PDSLabelTextHeight is for the line of text at the top of the image
#define		PDSLabelTextHeight	15



long		gPDS_Info_BytesPerRecord;
short		gPDScdROMtype;
short		gPDSdBASEtype;
//char		gCurrentVolName[64];		//*	current Volumne Name
PDS_header_data	PDSheader;

//*****************************************************************************
static int PDS_GetCDROMtype(const char *filePath, char *volumeName)
{
int		cdROMtype;
int		volChars;
char	*slashPtr;
char	myVolName[512];
char	tempString[512];

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("filePath=", filePath);

	strcpy(volumeName, "");
	strcpy(myVolName, filePath);
	slashPtr	=	strchr(myVolName, '/');
	cdROMtype	=	-1;
	while (slashPtr != NULL)
	{
		slashPtr++;
		strcpy(tempString, slashPtr);	//*	strcpy does not like to copy into itself
		strcpy(myVolName, tempString);

//		CONSOLE_DEBUG_W_STR("myVolName\t=", myVolName);
		volChars	=	(myVolName[0] << 24);
		volChars	+=	(myVolName[1] << 16);
		volChars	+=	(myVolName[2] << 8);
		volChars	+=	(myVolName[3]);

		if (volChars == 'VG_0')	cdROMtype	=	VOYAGER;
		if (volChars == 'VO_1')	cdROMtype	=	VIKING_1;
		if (volChars == 'VO_2')	cdROMtype	=	VIKING_2;
		if (volChars == 'MG_0')	cdROMtype	=	MAGELLAN_0;
		if (volChars == 'MG_1')	cdROMtype	=	MAGELLAN_1;
		if (volChars == 'MG_2')	cdROMtype	=	MAGELLAN_2;
		if (volChars == 'MG_3')	cdROMtype	=	MAGELLAN_3;
		if (volChars == 'GO_0')	cdROMtype	=	GALILEO;
		if (volChars == 'JEDI')	cdROMtype	=	JEDI;

		if (cdROMtype > 0)
		{
			slashPtr	=	strchr(myVolName, '/');
			if (slashPtr != NULL)
			{
				*slashPtr	=	0;
			}
			strcpy(volumeName, myVolName);
//			CONSOLE_DEBUG_W_STR("volumeName\t=", volumeName);
//			CONSOLE_DEBUG_W_NUM("cdROMtype \t=", cdROMtype);
			break;
		}
		slashPtr	=	strchr(myVolName, '/');
	}
	return(cdROMtype);
}

#if 0
//*****************************************************************************
static int PDS_GetDBASEtype(const char *filePath, char *volumeName)
{
int		dbaseType;
//int		volChars;
char	theFileName[32];
char	*slashPtr;
int		iii;
int		myCDROMtype;

//	CONSOLE_DEBUG(__FUNCTION__);

	dbaseType	=	0;

	myCDROMtype	=	PDS_GetCDROMtype(filePath, volumeName);
	CONSOLE_DEBUG_W_STR("volumeName \t=", volumeName);
	CONSOLE_DEBUG_W_NUM("myCDROMtype\t=", myCDROMtype);


	//*	check for long path names and pull out the file name only
	while ((slashPtr = strchr(theFileName, '/')) != nil)
	{
		iii	=	0;
		while ((iii<32) && (slashPtr[iii + 1] != 0x00))
		{
			theFileName[iii]	=	slashPtr[iii + 1];
			iii++;
		}
		theFileName[iii]	=	0;
	}


//	CONSOLE_DEBUG_W_NUM("myCDROMtype\t=", myCDROMtype);
	switch (myCDROMtype)
	{
		case VOYAGER:
			dbaseType					=	Voyager_db_ImageDBase;	//*	IMGINDEX.TAB;1
			gPDS_Info_BytesPerRecord	=	512;
			break;

		case VIKING_1:
		case VIKING_2:
			dbaseType					=	Viking_db_ImageDBase;	//*	IMGINDEX.TAB;1
			gPDS_Info_BytesPerRecord	=	512;
			break;

		case MAGELLAN_0:
		case MAGELLAN_1:
		case MAGELLAN_2:
		case MAGELLAN_3:
			if (strncmp(theFileName, "GEOM", 4) ==0)
			{
				dbaseType					=	Magellan_db_MDIR_Geometry;
				gPDS_Info_BytesPerRecord	=	90;
			}
			else if (strncmp(theFileName, "GEO", 3) ==0)	//*	note, this is DIFFERENT the "GEOM"
			{
				dbaseType					=	Magellan_db_Venus_Features;
				gPDS_Info_BytesPerRecord	=	284;
			}
			else if (strncmp(theFileName, "CONTENTS", 8) ==0)
			{
				dbaseType					=	Magellan_db_CD_Contents;
				gPDS_Info_BytesPerRecord	=	80;
			}
			else if (strncmp(theFileName, "MCUMDIR", 7) ==0)
			{
				dbaseType					=	Magellan_db_MDIR_Products_list;
				gPDS_Info_BytesPerRecord	=	80;
			}
			else if (strncmp(theFileName, "FRAME", 5) ==0)
			{
				dbaseType					=	Magellan_db_FrameLatLong_List;
				gPDS_Info_BytesPerRecord	=	80;
			}

			break;

		default:
			gPDS_Info_BytesPerRecord	=	512;
			break;
	}
	return(dbaseType);
}
#endif // 0

//*****************************************************************************
//*	set default values for PDS header info
//*****************************************************************************
static void	PDS_InitHeader(PDS_header_data *pdsHeaderPtr)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	memset(pdsHeaderPtr, 0, sizeof(PDS_header_data));
	pdsHeaderPtr->record_Type		=	0;						//*	type of record	: Variable, Fixed
	pdsHeaderPtr->record_Bytes		=	-1;						//*	number of bytes per record (after uncompressing)
	pdsHeaderPtr->file_records		=	0;						//*	total number of records in file
	pdsHeaderPtr->label_records		=	0;						//*	number of records in label (header)
	pdsHeaderPtr->spaceCraft_name[0]=	0;						//*	name of space craft
	pdsHeaderPtr->scanLines			=	-1;						//*	number of scan lines in image
	pdsHeaderPtr->lineSamples		=	-1;						//*	number of samples per line
	pdsHeaderPtr->histogramOffset	=	-1;						//*	offset record for histogram
	pdsHeaderPtr->imageLocationFlag	=	0;						//*	location of image 0 = means this file, 1 = other file
	pdsHeaderPtr->imageOffset		=	0;						//*	offset record for image
	pdsHeaderPtr->imageFileName[0]	=	0;						//*	file name if the image is a seperate file
}

//*****************************************************************************
static void	PDS_DumpHeader(PDS_header_data *pdsHeaderPtr)
{
	CONSOLE_DEBUG("--------------------------------------");

	CONSOLE_DEBUG_W_NUM(	"cdROMtype        \t=",	pdsHeaderPtr->cdROMtype);
	CONSOLE_DEBUG_W_STR(	"volumeName       \t=",	pdsHeaderPtr->volumeName);
	CONSOLE_DEBUG_W_STR(	"imageFileName    \t=",	pdsHeaderPtr->imageFileName);
	CONSOLE_DEBUG_W_STR(	"spaceCraft_name  \t=",	pdsHeaderPtr->spaceCraft_name);
	CONSOLE_DEBUG_W_STR(	"targetBody       \t=",	pdsHeaderPtr->targetBody);
	CONSOLE_DEBUG_W_NUM(	"record_Type      \t=",	pdsHeaderPtr->record_Type);
	CONSOLE_DEBUG_W_NUM(	"record_Bytes     \t=",	pdsHeaderPtr->record_Bytes);
	CONSOLE_DEBUG_W_NUM(	"file_records     \t=",	pdsHeaderPtr->file_records);
	CONSOLE_DEBUG_W_NUM(	"label_records    \t=",	pdsHeaderPtr->label_records);
	CONSOLE_DEBUG_W_NUM(	"scanLines        \t=",	pdsHeaderPtr->scanLines	);
	CONSOLE_DEBUG_W_NUM(	"lineSamples      \t=",	pdsHeaderPtr->lineSamples);
	CONSOLE_DEBUG_W_NUM(	"imageLocationFlag\t=",	pdsHeaderPtr->imageLocationFlag);
	CONSOLE_DEBUG_W_LONG(	"labelSize        \t=",	pdsHeaderPtr->labelSize);
	CONSOLE_DEBUG_W_NUM(	"lineSuffixBytes  \t=",	pdsHeaderPtr->lineSuffixBytes);
	CONSOLE_DEBUG_W_NUM(	"histogramOffset  \t=",	pdsHeaderPtr->histogramOffset);
	CONSOLE_DEBUG_W_NUM(	"encodeHistOffset \t=",	pdsHeaderPtr->encodeHistOffset);
	CONSOLE_DEBUG_W_NUM(	"imageOffset      \t=",	pdsHeaderPtr->imageOffset);
	CONSOLE_DEBUG_W_BOOL(	"imageIsCompressed\t=",	pdsHeaderPtr->imageIsCompressed);
	CONSOLE_DEBUG_W_NUM(	"gImagesRead      \t=",	gImagesRead);
	CONSOLE_DEBUG("--------------------------------------");
}

//*****************************************************************************
//*	subroutines to Read PDS Labels - read and parse the PDS label
//*	returns TRUE for EOF
//*****************************************************************************
static bool PDS_ProcessLabelOneLine(const char *lineBuff, PDS_header_data *pdsHeaderPtr)
{
bool		eofFlag;
int			iii;
char		*equalsPtr;
char		*argPtr;
char		argString[80];
int			ccc;
char		keyWord[80];

//	CONSOLE_DEBUG_W_STR("lineBuff:", lineBuff);
	eofFlag	=	false;

	//----------------------------------------------------------
	//*	extract the keyword
	iii	=	0;
	//*	fist skip leading spaces
	while (lineBuff[iii] == 0x20)
	{
		iii++;
	}
	ccc	=	0;
	while ((lineBuff[iii] > 0x20) && (lineBuff[iii] != '='))
	{
		keyWord[ccc++]	=	lineBuff[iii];
		iii++;
	}
	keyWord[ccc]	=	0;

//	CONSOLE_DEBUG_W_STR("keyWord\t=", keyWord);

	strcpy(argString, "");
	equalsPtr	=	strchr(lineBuff, '=');
	if (equalsPtr != NULL)
	{
		argPtr	=	equalsPtr + 1;
		while ((*argPtr <= 0x20) && (*argPtr > 0x0))
		{
			argPtr++;
		}
		strcpy(argString, argPtr);
	}

	//----------------------------------------------------------
	if (strcmp(keyWord,	"RECORD_TYPE") == 0)
	{
		if (strncmp(argString, "VARIABLE_LENGTH", 15) == 0)
		{
			pdsHeaderPtr->record_Type =	VARIABLE_LENGTH;
		}
		else
		{
			pdsHeaderPtr->record_Type	=	FIXED_LENGTH;
		}
	}
	else if (strcmp(keyWord,	"RECORD_BYTES") == 0)
	{
		pdsHeaderPtr->record_Bytes	=	atoi(argString);
	}
	else if (strcmp(keyWord,	"FILE_RECORDS") == 0)
	{
		pdsHeaderPtr->file_records	=	atoi(argString);
	}
	else if (strcmp(keyWord,	"LABEL_RECORDS") == 0)
	{
		pdsHeaderPtr->label_records	=	atoi(argString);
		if (pdsHeaderPtr->imageOffset <= 0)
		{
			pdsHeaderPtr->imageOffset	=	pdsHeaderPtr->label_records;
		}
	}
	else if (strcmp(keyWord,	"SPACECRAFT_NAME") == 0)
	{
		strcpy(pdsHeaderPtr->spaceCraft_name, argString);
	}
	else if (strcmp(keyWord,	"LINES") == 0)
	{
		pdsHeaderPtr->scanLines	=	atoi(argString);
	}
	else if (strcmp(keyWord,	"LINE_SAMPLES") == 0)
	{
		pdsHeaderPtr->lineSamples	=	atoi(argString);
	}
	else if (strcmp(keyWord,	"IMAGE_ID") == 0)
	{
		strcpy(pdsHeaderPtr->imageFileName, argString);
	}
	else if (strcmp(keyWord,	"^IMAGE_HISTOGRAM") == 0)
	{
		pdsHeaderPtr->histogramOffset	=	atoi(argString);
	}
	else if (strcmp(keyWord,	"^IMAGE") == 0)
	{
		if (argString[0] == 0x28)		// left parren
		{
			argPtr	=	argString;
			pdsHeaderPtr->imageLocationFlag	= 1;	//*	location of image, 1 = other file
			argPtr++;								// skip left parren
			if (argPtr[0] == '"')	argPtr++;		// skip any quotes
			iii	=	0;
			while ((argPtr[0] != '"') && (argPtr[0] != ','))
			{
				pdsHeaderPtr->imageFileName[iii++]	= argPtr[0];	//*	file name if the image is a seperate file
				argPtr++;
			}
			if (argPtr[0] == '"')	argPtr++;	// skip any quotes
			if (argPtr[0] == ',')	argPtr++;	// skip any commas
		}
		else
		{
			pdsHeaderPtr->imageLocationFlag	= 0;	//*	location of image, 1 = other file
		}
		pdsHeaderPtr->imageOffset	=	atoi(argString);
	}
	else if ((strcmp(keyWord,	"END")) == 0)
	{
		eofFlag	=	true;
	}

	else if ((strcmp(keyWord,	"LBLSIZE")) == 0)
	{
		pdsHeaderPtr->labelSize	=	atoi(argString);
	}
	//========================================================
	//*	these are for MG_0
	else if ((strcmp(keyWord,	"LBLSIZE")) == 0)
	{
		pdsHeaderPtr->labelSize	=	atoi(argString);
	}
	else if ((strcmp(keyWord,	"NL")) == 0)
	{
		pdsHeaderPtr->scanLines	=	atoi(argString);
	}
	else if ((strcmp(keyWord,	"NS")) == 0)
	{
		pdsHeaderPtr->lineSamples	=	atoi(argString);
		if (pdsHeaderPtr->record_Bytes > pdsHeaderPtr->lineSamples)
		{
			pdsHeaderPtr->lineSuffixBytes	=	pdsHeaderPtr->record_Bytes - pdsHeaderPtr->lineSamples;
		}
	}
	else if ((strcmp(keyWord,	"RECSIZE")) == 0)
	{
		pdsHeaderPtr->record_Bytes	=	atoi(argString);
		pdsHeaderPtr->imageOffset	=	pdsHeaderPtr->labelSize / pdsHeaderPtr->record_Bytes;

	}
	//========================================================
	//*	these are for voyager
	else if ((strcmp(keyWord,	"IMAGE_LINES")) == 0)
	{
		pdsHeaderPtr->scanLines	=	atoi(argString);
	}
	else if ((strcmp(keyWord,	"LINE_SUFFIX_BYTES")) == 0)
	{
		pdsHeaderPtr->lineSuffixBytes	=	atoi(argString);
//		CONSOLE_DEBUG_W_NUM("imageOffset\t=", pdsHeaderPtr->imageOffset);
	}
	else if ((strcmp(keyWord,	"TARGET_BODY")) == 0)
	{
		strcpy(pdsHeaderPtr->targetBody, argString);
	}
	else if ((strcmp(keyWord,	"TARGET_NAME")) == 0)
	{
		strcpy(pdsHeaderPtr->targetBody, argString);
	}
	else if ((strcmp(keyWord,	"^ENCODING_HISTOGRAM")) == 0)
	{
		pdsHeaderPtr->encodeHistOffset	=	atoi(argString);
	}
	else if ((strcmp(keyWord,	"OBJECT")) == 0)
	{
		if (strcmp(argString, "ENCODING_HISTOGRAM") ==  0)
		{
			pdsHeaderPtr->imageIsCompressed	=	true;
		}
	}

//./src_pds/PDS_ReadNASAfiles.c           : 770 [PDS_ReadHeaderAndImage] OBJECT                           = ENCODING_HISTOGRAM

	//========================================================
	//*	these are for Galileo
	else if ((strcmp(keyWord,	"NBB")) == 0)
	{
		pdsHeaderPtr->linePrefixBytes	=	atoi(argString);
	}
	else if ((strcmp(keyWord,	"MISSION")) == 0)
	{
		strcpy(pdsHeaderPtr->spaceCraft_name, argString);
	}
	else if ((strcmp(keyWord,	"TARGET")) == 0)
	{
		strcpy(pdsHeaderPtr->targetBody, argString);
	}




//	//******** This part to be replaced or deleted later
//	if (	(strncmp(lineBuff,"SPACECRAFT",10) == 0)
//		||	(strncmp(lineBuff,"MISSION",7) == 0)
//		||	(strncmp(lineBuff,"TARGET",6) == 0)
//		||	(strncmp(lineBuff,"IMAGE_NUMBER",12) == 0))
//	{
//
//		strncat(PDSlabelData, lineBuff, length);
//		strncat(PDSlabelData, " *\r", 3);
//	}

	return(eofFlag);
}

//*********************************************************************
//*	Read variable length records from input file
//*********************************************************************
static int	PDS_ReadVariableRecord(FILE *filePointer, uint8_t *dataBuffer, bool verbose)
{
int		dataLength;
int		bytesRead;
int		charsRead;

	//*	variable length record
	charsRead	=	-1;
	bytesRead	=	fread(dataBuffer, 1, 2, filePointer);
	if (bytesRead == 2)
	{
		if (verbose)
		{
			CONSOLE_DEBUG_W_HEX("dataBuffer[0]\t=", dataBuffer[0]);
			CONSOLE_DEBUG_W_HEX("dataBuffer[1]\t=", dataBuffer[1]);
		}
		dataLength	=	dataBuffer[1] << 8;
		dataLength	+=	dataBuffer[0];
		//*	the dataLength has to be an even number
		if ((dataLength % 2) == 1)
		{
			dataLength	+=	1;
		}
		if (verbose)
		{
			CONSOLE_DEBUG_W_NUM("dataLength\t=", dataLength);
		}
		charsRead				=	fread(dataBuffer, 1, dataLength, filePointer);
		dataBuffer[charsRead]	=	0;
	}
	return(charsRead);
}

//*****************************************************************************
//*	returns the number of chars read, -1 if error
//*****************************************************************************
static int	PDS_ReadOneLineFromHeader(FILE *filePointer, const int cdROMtype, char *lineBuff)
{
int		charsRead;
uint8_t	dataBuffer[16];
size_t	bytesRead;
int		ccc;
int		singleQuoteCtr;
bool	keepGoing;
char	theChar;

//	CONSOLE_DEBUG_W_NUM("cdROMtype\t=", cdROMtype);

	charsRead	=	-1;
	switch(cdROMtype)
	{
		case VOYAGER:
		case VIKING_1:
			//*	variable length record
			charsRead	=	PDS_ReadVariableRecord(filePointer, (uint8_t *)lineBuff, false);
			break;

//		case -1:
		case MAGELLAN_0:
//		case MAGELLAN_1:
		case MAGELLAN_2:
		case MAGELLAN_3:
		case GALILEO:
			singleQuoteCtr	=	0;
			ccc				=	0;
			keepGoing		=	true;
			charsRead		=	0;
			while (keepGoing)
			{
				//*	read 1 byte at a time
				bytesRead	=	fread(dataBuffer, 1, 1, filePointer);
				charsRead	+=	bytesRead;
				theChar		=	dataBuffer[0];
				if (theChar == '\'')
				{
					singleQuoteCtr++;
				}
				if ((theChar == 0x20) && (ccc == 0))
				{
					//*	do nothing
				}
				else if ((theChar != 0x20) || ((theChar == 0x20) && ((singleQuoteCtr % 2) == 1)))
				{
					if (ccc < 80)
					{
						lineBuff[ccc]		=	theChar;
						lineBuff[ccc + 1]	=	0;
						ccc++;
					}
					else
					{
						CONSOLE_DEBUG("Buffer overflow");
						keepGoing	=	false;
					}
				}
				else if (theChar == 0x20)
				{
					keepGoing	=	false;
				}
			}
			break;

		case MAGELLAN_1:
		default:
			if (fgets(lineBuff, 100, filePointer))
			{
				charsRead	=	strlen(lineBuff);
			}
			break;

	}
	return(charsRead);
}

//*****************************************************************************
static bool	PDS_ReadUncompressedImage(FILE *filePointer, PDS_header_data *pdsHeaderPtr)
{
bool			returnFlag;
//int				histogramBytesRead;
int				imageBytesRead;
long			currentOffset;
size_t			filePositionOffset;
size_t			recordsRead;

//	CONSOLE_DEBUG(__FUNCTION__);

	filePositionOffset	=	(pdsHeaderPtr->imageOffset -1) * pdsHeaderPtr->record_Bytes;
	if (pdsHeaderPtr->labelSize > filePositionOffset)
	{
		filePositionOffset	=	pdsHeaderPtr->labelSize;
	}
	if (pdsHeaderPtr->linePrefixBytes > 0)
	{
		filePositionOffset	+=	pdsHeaderPtr->linePrefixBytes;
	}
	currentOffset		=	fseek(filePointer, filePositionOffset, SEEK_SET);
	currentOffset		=	ftell(filePointer);
//	CONSOLE_DEBUG_W_LONG(	"currentOffset     \t=", currentOffset);
//	CONSOLE_DEBUG_W_LONG(	"filePositionOffset\t=", filePositionOffset);
//	CONSOLE_DEBUG_W_NUM(	"imageOffset       \t=", pdsHeaderPtr->imageOffset);
//	CONSOLE_DEBUG_W_NUM(	"record_Bytes      \t=", pdsHeaderPtr->record_Bytes);
//	CONSOLE_DEBUG_W_LONG(	"labelSize         \t=", pdsHeaderPtr->labelSize);

//	CONSOLE_DEBUG_W_NUM("lineSamples\t=", pdsHeaderPtr->lineSamples);
//	CONSOLE_DEBUG_W_NUM("scanLines  \t=", pdsHeaderPtr->scanLines);

	if (pdsHeaderPtr->lineSuffixBytes > 0)
	{
	uint8_t		*pixelRowPtr;
	int			yyy;
	uint8_t		suffixBuffer[256];

		//*	we have to read the image in one line at a time
		imageBytesRead	=	0;
		pixelRowPtr		=	pdsHeaderPtr->imageData;
		for (yyy=0; yyy<pdsHeaderPtr->scanLines; yyy++)
		{
			imageBytesRead	+=	fread(pixelRowPtr, 1, pdsHeaderPtr->lineSamples, filePointer);
			imageBytesRead	+=	fread(suffixBuffer, 1, pdsHeaderPtr->lineSuffixBytes, filePointer);

			pixelRowPtr		+=	pdsHeaderPtr->lineSamples;
		}
	}
	else
	{
		recordsRead		=	fread(pdsHeaderPtr->imageData, pdsHeaderPtr->lineSamples, pdsHeaderPtr->scanLines, filePointer);
		imageBytesRead	=	recordsRead * pdsHeaderPtr->lineSamples;
	}
//	CONSOLE_DEBUG_W_LONG("filePositionOffset\t=", filePositionOffset);
//	CONSOLE_DEBUG_W_LONG("currentOffset     \t=", currentOffset);
//	CONSOLE_DEBUG_W_LONG("image recordsRead \t=", recordsRead);

	//-------------------------------------------------------
	//*	lets try adjusting the image
	//int	pixelCount;
	//int	pixelValue;
	//	pixelCount	=	pdsHeaderPtr->lineSamples * pdsHeaderPtr->scanLines;
	//	for (iii=0; iii<pixelCount; iii++)
	//	{
	//		pixelValue	=	pdsHeaderPtr->imageData[iii] & 0x00ff;
	//		pixelValue	=	pixelValue + (pixelValue/ 2);
	//		if (pixelValue > 255)
	//		{
	//			pixelValue	=	255;
	//		}
	//		pdsHeaderPtr->imageData[iii]	=	pixelValue;
	//	}

	returnFlag	=	true;

	return(returnFlag);
}

//*****************************************************************************
static void	StepThroughAllVariableRecords(FILE *filePointer)
{
int		recordCount;
int		dataLength;
long	currentOffset;
long	totalBytesRead;
bool	keepGoing;
char	dataBuffer[4096];

	currentOffset	=	fseek(filePointer, 0, SEEK_SET);
	totalBytesRead	=	0;
	recordCount		=	1;
	keepGoing		=	true;

	while (keepGoing && (recordCount < 75))
	{
		dataLength		=	PDS_ReadVariableRecord(filePointer, (uint8_t *)dataBuffer, false);
		if (dataLength < 0)
		{
			keepGoing		=	false;
		}

		if (recordCount < 60)
		{
			printf("Record #%4d Length=%4d data=%s\r\n", recordCount, dataLength, dataBuffer);
		}
		else
		{
			printf("Record #%4d Length=%4d\r\n", recordCount, dataLength);
		}

//		CONSOLE_DEBUG_W_NUM("dataLength\t=", dataLength);


		totalBytesRead	+=	dataLength;
		recordCount++;
	}
	CONSOLE_DEBUG_W_NUM("recordCount\t=", recordCount);

	currentOffset	=	fseek(filePointer, 0, SEEK_SET);
	currentOffset		=	ftell(filePointer);
	CONSOLE_DEBUG_W_LONG("currentOffset\t=", currentOffset);
}

//*****************************************************************************
static void	SetFilePositionVariableRec(FILE *filePointer, int desiredRecordNumber)
{
int		recordCount;
int		dataLength;
char	dataBuffer[4096];
long	currentOffset;

//	CONSOLE_DEBUG_W_NUM(__FUNCTION__, desiredRecordNumber);

	currentOffset	=	fseek(filePointer, 0, SEEK_SET);
	if (currentOffset != 0)
	{
		CONSOLE_DEBUG("fseek() failed");
	}
	recordCount		=	1;
	dataLength		=	0;
	while ((recordCount < desiredRecordNumber) && (dataLength >= 0))
	{
		dataLength		=	PDS_ReadVariableRecord(filePointer, (uint8_t *)dataBuffer, false);
		recordCount++;
	}
}

//*****************************************************************************
static bool	PDS_ReadCompressedImage(FILE *filePointer, PDS_header_data *pdsHeaderPtr)
{
bool		returnFlag;
int			scanLineIdx;
int			dataLength;
uint8_t		compressedDataBuff[4096];
char		imagebuff[4096];
int			out_bytes;
uint8_t		*pdsPixelPtr;
uint8_t		*encodingHistPtr;

//	CONSOLE_DEBUG(__FUNCTION__);

//	StepThroughAllVariableRecords(filePointer);

	//-----------------------------------------------------------------------------
	//*	check for compressed image
	if (pdsHeaderPtr->encodeHistOffset > 0)
	{
		SetFilePositionVariableRec(filePointer, pdsHeaderPtr->encodeHistOffset);
		memset(pdsHeaderPtr->encodingHistogram, 0, sizeof(pdsHeaderPtr->encodingHistogram));

		if (pdsHeaderPtr->record_Bytes == 836)
		{
		//	CONSOLE_DEBUG("Reading compression histogram (836) 3 records");
			encodingHistPtr	=	(uint8_t *)pdsHeaderPtr->encodingHistogram;
			dataLength		=	PDS_ReadVariableRecord(filePointer, (encodingHistPtr),			false);
			dataLength		=	PDS_ReadVariableRecord(filePointer, (encodingHistPtr + 836),	false);
			dataLength		=	PDS_ReadVariableRecord(filePointer, (encodingHistPtr + 1672),	false);
//			length	=	read_variableRecord(fRefNum, (char *)hist);
//			length	=	read_variableRecord(fRefNum, (char *)hist+836);
//			length	=	read_variableRecord(fRefNum, (char *)hist+1672);
		}
		else
		{
			dataLength	=	PDS_ReadVariableRecord(filePointer, (uint8_t *)pdsHeaderPtr->encodingHistogram,		false);
			dataLength	=	PDS_ReadVariableRecord(filePointer, (uint8_t *)pdsHeaderPtr->encodingHistogram+1204,	false);
//			length	=	read_variableRecord(fRefNum, (char *)hist);
//			length	=	read_variableRecord(fRefNum, (char *)hist+1204);
		}
	}
	else
	{
		CONSOLE_DEBUG("Did not read image incoding histogram");
	}

	decmpinit(pdsHeaderPtr->encodingHistogram);

	//-----------------------------------------------------------------------------
	//*	position to start of image
	SetFilePositionVariableRec(filePointer, pdsHeaderPtr->imageOffset);

	pdsPixelPtr	=	pdsHeaderPtr->imageData;	//*	pointer to the image data
	for (scanLineIdx=1; scanLineIdx < pdsHeaderPtr->scanLines; scanLineIdx++)
	{
		dataLength	=	PDS_ReadVariableRecord(filePointer, compressedDataBuff, false);
		if (dataLength <= 0)
		{
			break;
		}

		out_bytes	=	VOYAGER_RECORD_BYTES;
		out_bytes	=	pdsHeaderPtr->record_Bytes;

		decompress(compressedDataBuff, imagebuff, &dataLength, &out_bytes);

		//*	now copy it over to the PDS image buffer
		memcpy(pdsPixelPtr, imagebuff, pdsHeaderPtr->lineSamples);

		pdsPixelPtr	+=	pdsHeaderPtr->lineSamples;
	}
	DecompressFreeMemory();
	returnFlag	=	true;

//	CONSOLE_DEBUG(__FUNCTION__);
	return(returnFlag);
}

//*****************************************************************************
static bool	PDS_ReadHeaderAndImage(const char *filePath, PDS_header_data *pdsHeaderPtr)
{
bool			returnFlag;
FILE			*filePointer;
int				linesRead;
char			lineBuff[512];
bool			keepReading;
bool			eofFlag;
int				slen;
int				totalBytesRead;
int				headerBytesRead;
int				histogramBytesRead;
int				imageBytesRead;
int				charsRead;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__, filePath);
//	CONSOLE_DEBUG_W_NUM("cdROMtype\t=", pdsHeaderPtr->cdROMtype);
	returnFlag	=	false;
	filePointer	=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
		totalBytesRead		=	0;
		headerBytesRead		=	0;
		histogramBytesRead	=	0;
		imageBytesRead		=	0;
		linesRead			=	0;
		keepReading			=	true;

		//*	read the header
		while (keepReading && (linesRead < 100))
		{
			charsRead	=	PDS_ReadOneLineFromHeader(filePointer, pdsHeaderPtr->cdROMtype, lineBuff);
			if (charsRead >= 0)
			{
				pdsHeaderPtr->headerBytesRead	+=	charsRead;

				//*	get rid of any CR/LF at the end
				slen			=	strlen(lineBuff);
				headerBytesRead	+=	slen;
				while ((slen > 0) && (lineBuff[slen - 1] < 0x20))
				{
					lineBuff[slen - 1]	=	0;
					slen				=	strlen(lineBuff);
				}
//				CONSOLE_DEBUG(lineBuff);

				//*	save a copy of the header line
				strcpy(pdsHeaderPtr->HeaderData[linesRead].headerLine, lineBuff);

				eofFlag	=	PDS_ProcessLabelOneLine(lineBuff, pdsHeaderPtr);
				if (eofFlag)
				{
					keepReading	=	false;
				}

				//*	now check for other type
				if (pdsHeaderPtr->labelSize > 0)
				{
//					CONSOLE_DEBUG_W_NUM("headerBytesRead\t=", pdsHeaderPtr->headerBytesRead);
					if (pdsHeaderPtr->headerBytesRead >= pdsHeaderPtr->labelSize)
					{
						CONSOLE_DEBUG("Done with header");
						keepReading	=	false;
					}
				}
				linesRead++;
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("End of header, charsRead=", charsRead);
				keepReading	=	false;
			}
		}
		pdsHeaderPtr->HeaderLineCnt	=	linesRead;
//		CONSOLE_DEBUG_W_NUM("header lines read\t=", linesRead);
//		CONSOLE_DEBUG_W_NUM("header bytes read\t=", headerBytesRead);
//		//--------------------------------------------------------
//		if (pdsHeaderPtr->histogramOffset > 0)
//		{
//		int				iii;
//			//*	read the histogram
//			filePositionOffset	=	(pdsHeaderPtr->histogramOffset -1) * pdsHeaderPtr->record_Bytes;
//			currentOffset		=	fseek(filePointer, filePositionOffset, SEEK_SET);
//			currentOffset		=	ftell(filePointer);
//			recordsRead			=	fread(pdsHeaderPtr->histogram, (kHistogramEntries * sizeof(uint32_t)), 1, filePointer);
//			histogramBytesRead	=	recordsRead * (kHistogramEntries * sizeof(uint32_t));
//
////			for (iii=0; iii<kHistogramEntries; iii++)
////			{
////				printf("%3d\t%6d\r\n", iii, pdsHeaderPtr->histogram[iii]);
////			}
//		}


//		PDS_DumpHeader(pdsHeaderPtr);

		//--------------------------------------------------------
		//*	see if we have enough data to describe the image
		if ((pdsHeaderPtr->record_Bytes > 0) &&
			(pdsHeaderPtr->scanLines > 0) &&
			(pdsHeaderPtr->lineSamples > 0) &&
			(pdsHeaderPtr->imageOffset > 0))

		{
			pdsHeaderPtr->imageData	=	calloc(pdsHeaderPtr->scanLines, pdsHeaderPtr->lineSamples);
			if (pdsHeaderPtr->imageData != NULL)
			{
				if (pdsHeaderPtr->imageIsCompressed)
				{
					returnFlag	=	PDS_ReadCompressedImage(filePointer, pdsHeaderPtr);
				}
				else
				{
					returnFlag	=	PDS_ReadUncompressedImage(filePointer, pdsHeaderPtr);
				}
			}
			else
			{
				CONSOLE_DEBUG_W_STR("Not enough info to open image:", filePath);
			}
		}
		fclose(filePointer);
//		totalBytesRead	=	headerBytesRead + histogramBytesRead + imageBytesRead;
//		CONSOLE_DEBUG_W_NUM("header bytes read   \t=", headerBytesRead);
//		CONSOLE_DEBUG_W_NUM("histogram bytes read\t=", histogramBytesRead);
//		CONSOLE_DEBUG_W_NUM("image bytes read    \t=", imageBytesRead);
//		CONSOLE_DEBUG_W_NUM("total bytes read    \t=", totalBytesRead);
	}
	else
	{
		returnFlag	=	false;
		CONSOLE_DEBUG("Failed to open file");
	}

	gImagesRead++;
	return(returnFlag);
}

//*****************************************************************************
bool	PDS_ReadImage(const char *filePath, PDS_header_data	*pdsHeaderPtr)
{
bool		returnFlag;
char		volumeName[32];

	PDS_InitHeader(pdsHeaderPtr);

	pdsHeaderPtr->cdROMtype	=	PDS_GetCDROMtype(filePath, volumeName);
//	pdsHeaderPtr->dBaseType	=	PDS_GetDBASEtype(filePath, volumeName);
	strcpy(pdsHeaderPtr->volumeName, volumeName);

//	CONSOLE_DEBUG_W_STR("volumeName\t=", volumeName);
//	CONSOLE_DEBUG_W_NUM("cdROMtype \t=", pdsHeaderPtr->cdROMtype);
//	CONSOLE_DEBUG_W_NUM("dBaseType \t=", dBaseType);

	returnFlag	=	PDS_ReadHeaderAndImage(filePath, pdsHeaderPtr);

	return(returnFlag);
}

#ifdef _INCLUDE_MAIN_

//*****************************************************************************
int main(int argc, char *argv[])
{
int				iii;
PDS_header_data	pdsHeader;

	CONSOLE_DEBUG("NASA CD-ROM read");
	CONSOLE_DEBUG_W_NUM("argc\t=", argc);

	for (iii=1; iii<argc; iii++)
	{
		CONSOLE_DEBUG_W_STR("Arg=", argv[iii]);

		PDS_ReadImage(argv[iii], &pdsHeader);
		PDS_DumpHeader(&pdsHeader);
	}
	return(0);
}
#endif // _INCLUDE_MAIN_
