//*****************************************************************************
//*	SAO_stardata.c
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Aug  2,	2022	<MLS> Created SAO_stardata.c
//*****************************************************************************
//*	http://tdc-www.harvard.edu/catalogs/sao.html
//*	http://www.stargazing.net/kepler/b1950.html
//*	https://heasarc.gsfc.nasa.gov/FTP/heasarc/dbase/dump/
//*	https://heasarc.gsfc.nasa.gov/W3Browse/star-catalog/sao.html
//*****************************************************************************


#include	<ctype.h>
#include	<string.h>
#include	<stdlib.h>
#include	<stdio.h>
#include	<stdbool.h>
#include	<math.h>

//*	MLS Libraries
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"SkyStruc.h"
#include	"SAO_stardata.h"


#define		kSkyTravelDataDirectory	"skytravel_data"

#define		kSAO_asciiFileName	"/sao/heasarc_sao.tdat"
#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))

//*****************************************************************************
enum
{
	kSAOarg_name	=	1,				// SAO Catalog Designation
	kSAOarg_ra,							// Right Ascension
	kSAOarg_proper_motion_ra,			// Annual RA Proper Motion (FK4 System)
	kSAOarg_proper_motion_ra_error,		// Standard Deviation in RA Proper Motion (FK4 System)
	kSAOarg_ra_epoch,					// RA Original Epoch
	kSAOarg_dec,						// Declination
	kSAOarg_proper_motion_dec,			// Annual Declination Proper Motion (FK4 System)
	kSAOarg_proper_motion_dec_error,	// Standard Deviation of Declination Proper Motion (FK4 System)
	kSAOarg_dec_epoch,					// Declination Original Epoch
	kSAOarg_position_error,				// Standard Deviation of Position at Epoch 1950.0
	kSAOarg_lii,						// Galactic Longitude
	kSAOarg_bii,						// Galactic Latitude
	kSAOarg_pg_mag,						// Photographic Magnitude
	kSAOarg_vmag,						// Visual Magnitude
	kSAOarg_spect_type,					// Spectral Type (+++ for Composite Spectra)
	kSAOarg_ref_vmag,					// Reference Code for Visual Magnitude
	kSAOarg_ref_star_number,			// Reference Code for Star Number and Footnotes
	kSAOarg_ref_pg_mag,					// Reference Code for Photographic Magnitude
	kSAOarg_ref_proper_motion,			// Reference Code for Proper Motions
	kSAOarg_ref_spect_type,				// Reference Code for Spectral Type
	kSAOarg_remarks,					// Coded Remarks for Duplicity and Variability
	kSAOarg_ref_source_cat,				// Reference Code for Source Catalog
	kSAOarg_num_source_cat,				// Number in Source Catalog
	kSAOarg_dm,							// Durchmusterung Identification (BD, CD, or CP)
	kSAOarg_hd,							// Henry Draper Catalog (HD or HDE) Number
	kSAOarg_hd_component,				// HD Component and Multiple Code
	kSAOarg_gc,							// Boss General Catalog (GC) Number
	kSAOarg_proper_motion_ra_fk5,		// Annual RA Proper Motion (FK5 System)
	kSAOarg_proper_motion_dec_fk5,		// Annual Declination Proper Motion (FK5 System)
	kSAOarg_class,						// Browse Object Classification


};

////*****************************************************************************
//static void	SAO_DumpRecord(TYPE_SAO_DATA *saoRecord)
//{
//int			iii;
//int16_t		theByte;
//char		*dataPtr;
//
//	printf("\r\n");
//	dataPtr	=	(char *)saoRecord;
//	for (iii=0; iii<sizeof(TYPE_SAO_DATA); iii++)
//	{
//		theByte	=	dataPtr[iii] & 0x00ff;
//		printf("%02X ", theByte);
//	}
//	printf("\r\n");
//}

//*****************************************************************************
static void	SAO_ParseDataEntry(const char *lineBuff, TYPE_CelestData *objectStruct)
{
char	argBuff[64];
int		iii;
int		ccc;
int		lineLen;
int		argNum;
char	theChar;
double	ra_Degrees;
double	dec_Degrees;

//	CONSOLE_DEBUG(__FUNCTION__);

//	printf("*");
//	fflush(stdout);

	memset((void *)objectStruct, 0, sizeof(TYPE_CelestData));

	lineLen	=	strlen(lineBuff);
	iii		=	0;
	ccc		=	0;
	argNum	=	1;
	while ((lineBuff[iii] != 0) && (iii < lineLen))
	{
		theChar	=	lineBuff[iii];
		iii++;
		if ((theChar == '|') || (theChar < 0x20))
		{
//SAO 258660|218.82525833|-0.1169|3|1903.6|-89.771625|-0.004|2|1903|0.13|303.04406295|-26.92288489||6.6|M0|14|3|0|1|0|0|70|17838|CP-89   37|110994|0|17838|-0.3021|-0.003|2700|
			switch(argNum)
			{
				case kSAOarg_name:		// SAO Catalog Designation
					strcpy(objectStruct->shortName, argBuff);
					strcpy(objectStruct->longName, argBuff);
					if ((argBuff[0] == 'S') && (argBuff[3] == ' ') && (isdigit(argBuff[4])))
					{
						objectStruct->id	=	atoi(&argBuff[4]);
					}
					break;

				case kSAOarg_ra:		// Right Ascension
					ra_Degrees				=	atof(argBuff);
					objectStruct->org_ra	=	RADIANS(ra_Degrees);
					objectStruct->ra		=	RADIANS(ra_Degrees);
					break;

//				case kSAOarg_proper_motion_ra:			// Annual RA Proper Motion (FK4 System)
//				case kSAOarg_proper_motion_ra_error:	// Standard Deviation in RA Proper Motion (FK4 System)
//				case kSAOarg_ra_epoch:					// RA Original Epoch
//					if (strlen(argBuff) > 0)
//					{
//						CONSOLE_DEBUG_W_STR("kSAOarg_ra_epoch\t=", argBuff);
//					}
//					break;

				case kSAOarg_dec:						// Declination
					dec_Degrees				=	atof(argBuff);
					objectStruct->org_decl	=	RADIANS(dec_Degrees);
					objectStruct->decl		=	RADIANS(dec_Degrees);
					break;

//				case kSAOarg_proper_motion_dec:			// Annual Declination Proper Motion (FK4 System)
//				case kSAOarg_proper_motion_dec_error:	// Standard Deviation of Declination Proper Motion (FK4 System)
//				case kSAOarg_dec_epoch:					// Declination Original Epoch
//				case kSAOarg_position_error:			// Standard Deviation of Position at Epoch 1950.0
//				case kSAOarg_lii:						// Galactic Longitude
//				case kSAOarg_bii:						// Galactic Latitude
//				case kSAOarg_pg_mag:					// Photographic Magnitude

				case kSAOarg_vmag:						// Visual Magnitude
					if (strlen(argBuff) > 0)
					{
						objectStruct->realMagnitude	=	atof(argBuff);
					}
					else
					{
					//	CONSOLE_DEBUG_W_STR("realMagnitude\t=", argBuff);
						objectStruct->realMagnitude	=	10;
					}
					break;

				case kSAOarg_spect_type:				// Spectral Type (+++ for Composite Spectra)
					objectStruct->spectralClass	=	argBuff[0];
					break;

//				case kSAOarg_ref_vmag:					// Reference Code for Visual Magnitude
//				case kSAOarg_ref_star_number:			// Reference Code for Star Number and Footnotes
//				case kSAOarg_ref_pg_mag:				// Reference Code for Photographic Magnitude
//				case kSAOarg_ref_proper_motion:			// Reference Code for Proper Motions
//				case kSAOarg_ref_spect_type:			// Reference Code for Spectral Type
//				case kSAOarg_remarks:					// Coded Remarks for Duplicity and Variability
//				case kSAOarg_ref_source_cat:			// Reference Code for Source Catalog
//				case kSAOarg_num_source_cat:			// Number in Source Catalog
//				case kSAOarg_dm:						// Durchmusterung Identification (BD, CD, or CP)
				case kSAOarg_hd:						// Henry Draper Catalog (HD or HDE) Number
					if (strlen(argBuff) > 0)
					{
						strcat(objectStruct->longName, "/HD");
						strcat(objectStruct->longName, argBuff);
					}
					break;

//				case kSAOarg_hd_component:				// HD Component and Multiple Code
//				case kSAOarg_gc:						// Boss General Catalog (GC) Number
//				case kSAOarg_proper_motion_ra_fk5:		// Annual RA Proper Motion (FK5 System)
//				case kSAOarg_proper_motion_dec_fk5:		// Annual Declination Proper Motion (FK5 System)
//				case kSAOarg_class:						// Browse Object Classification
			}

			ccc				=	0;
			argBuff[ccc]	=	0;

			argNum++;

		}
		else if (ccc < 60)
		{
			argBuff[ccc++]	=	theChar;
			argBuff[ccc]	=	0;
		}
	}
	objectStruct->dataSrc	=	kDataSrc_SAO;
}

//*****************************************************************************
TYPE_CelestData	*SAO_ReadFile(long *starCount)
{
FILE			*filePointer;
char			filePath[128];
char			lineBuff[1024];
bool			readHeader;
bool			keepReading;
int				linesRead;
int				lineLen;
int				maxLineLen;
int				recordCount;
TYPE_CelestData	*saoStarData;

	CONSOLE_DEBUG(__FUNCTION__);


	strcpy(filePath, kSkyTravelDataDirectory);
	strcat(filePath, kSAO_asciiFileName);

	CONSOLE_DEBUG(filePath);

	maxLineLen	=	0;
	filePointer	=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
		readHeader	=	true;
		linesRead	=	0;
		while (readHeader && (linesRead < 200))
		{
			if (fgets(lineBuff, 1000, filePointer))
			{
				lineLen	=	strlen(lineBuff);
				if (lineLen > maxLineLen)
				{
					maxLineLen	=	lineLen;
//					CONSOLE_DEBUG_W_NUM("maxLineLen\t=", maxLineLen);
				}
				linesRead++;
				if (strncmp(lineBuff, "<DATA>", 6) == 0)
				{
					readHeader	=	false;
				}
			}
		}
		CONSOLE_DEBUG_W_NUM("linesRead\t=", linesRead);

#define	kMaxSAOcount	259000

		//*	allocate the array for the star data
		saoStarData	=	(TYPE_CelestData *)calloc(kMaxSAOcount, sizeof(TYPE_CelestData));

		if (saoStarData != NULL)
		{
			keepReading	=	true;
			recordCount	=	0;
			while (keepReading && (recordCount < kMaxSAOcount))
			{
				if (fgets(lineBuff, 1000, filePointer))
				{
					if (strncmp(lineBuff, "SAO ", 4) == 0)
					{
						SAO_ParseDataEntry(lineBuff, &saoStarData[recordCount]);
						recordCount++;
					}
				}
				else
				{
					keepReading	=	false;
				}
			}
			*starCount	=	recordCount;
		}

		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read star data:", filePath);
	}

	CONSOLE_DEBUG_W_LONG("starCount\t=", *starCount);

	return(saoStarData);
}


#ifdef _INCLUDE_SAO_MAIN_


//*****************************************************************************
int main(int argc, char *argv[])
{
long	starCount;

	CONSOLE_DEBUG("SAO read test");


	SAO_ReadFile(&starCount);
	CONSOLE_DEBUG_W_LONG("starCount\t=", starCount);

}
#endif // _INCLUDE_SAO_MAIN_
