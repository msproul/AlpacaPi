//*****************************************************************************
//*	SAO_stardata.c
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Aug  2,	2022	<MLS> Created SAO_stardata.c
//*	Dec 27,	2022	<MLS> Keith found a different version of the SAO catalog
//*	Dec 27,	2022	<MLS> https://cdsarc.cds.unistra.fr/ftp/cats/I/131A/
//*	Dec 29,	2022	<MLS> Renamed SAO_ReadFile() to SAO_ReadFile_heasarc()
//*	Dec 29,	2022	<MLS> Added SAO_ReadFile_sao_dat()
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
#include	<sys/stat.h>

//*	MLS Libraries
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"SkyStruc.h"
#include	"SAO_stardata.h"
#include	"helper_functions.h"
#include	"controller_startup.h"


#define		kSkyTravelDataDirectory	"skytravel_data"
#define	kMaxSAOcount	259000

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
				case kSAOarg_proper_motion_ra_fk5:		// Annual RA Proper Motion (FK5 System)
//					CONSOLE_DEBUG_W_STR("kSAOarg_proper_motion_ra_fk5\t=",	argBuff);
					if (strlen(argBuff) > 0)
					{
						objectStruct->propMotion_RA_mas_yr		=	atof(argBuff);
					}
					break;

				case kSAOarg_proper_motion_dec_fk5:		// Annual Declination Proper Motion (FK5 System)
//					CONSOLE_DEBUG_W_STR("kSAOarg_proper_motion_dec_fk5\t=",	argBuff);
					if (strlen(argBuff) > 0)
					{
						objectStruct->propMotion_DEC_mas_yr		=	atof(argBuff);
						if ((fabs(objectStruct->propMotion_RA_mas_yr) > 0.0) || (fabs(objectStruct->propMotion_DEC_mas_yr) > 0.0))
						{
							objectStruct->propMotionValid	=	true;
//							CONSOLE_DEBUG_W_DBL("propMotion_RA_mas_yr \t=",	objectStruct->propMotion_RA_mas_yr);
//							CONSOLE_DEBUG_W_DBL("propMotion_DEC_mas_yr\t=",	objectStruct->propMotion_DEC_mas_yr);
						}
					}
					break;
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

////************************************************************************
//static void	StripLeadingSpaces(char *theString)
//{
//int		iii;
//int		ccc;
//
//	ccc		=	0;
//	iii		=	0;
//	while (theString[iii] != 0)
//	{
//		if ((ccc == 0) && (theString[iii] == 0x20))
//		{
//			//*	do nothing
//		}
//		else
//		{
//			theString[ccc++]	=	theString[iii];
//		}
//		iii++;
//	}
//	theString[ccc]	=	0;
//}

//**************************************************************************
static void	ExtractTextField(	const char	*theLine,
								const int	startIdx,
								const int	fieldLen,
								char		*outputText,
								bool		removeLeadingSpaces)
{
int		iii;
int		ccc;

	iii	=	startIdx;
	ccc	=	0;
	while (ccc < fieldLen)
	{
		outputText[ccc]	=	theLine[iii];
		ccc++;
		iii++;
	}
	outputText[ccc]	=	0;

//	CONSOLE_DEBUG_W_STR("outputText:", outputText);

	if (removeLeadingSpaces)
	{
		StripLeadingSpaces(outputText);
	}
}

//*****************************************************************************
static TYPE_CelestData	*SAO_ReadFile_heasarc(long *starCount)
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
int				startupWidgetIdx;

//	CONSOLE_DEBUG(__FUNCTION__);

	startupWidgetIdx	=	SetStartupText("SAO-heasarc catalog:");
	saoStarData			=	NULL;

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
		SetStartupTextStatus(startupWidgetIdx, "OK");
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read star data:", filePath);
		SetStartupTextStatus(startupWidgetIdx, "Failed");
	}

//	CONSOLE_DEBUG_W_LONG("starCount\t=", *starCount);

	return(saoStarData);
}

//*****************************************************************************
//I/131A      SAO Star Catalog J2000     (SAO Staff 1966; USNO, ADC 1990)
//================================================================================
//Smithsonian Astrophysical Observatory Star Catalog
//    SAO Staff
//   <Smithsonian Astrophysical Observatory (1966)>
//================================================================================
//ADC_Keywords: Positional data ; Proper motions
//
//Description:
//    This machine-readable SAO catalog from the Astronomical Data Center is
//    based on an original binary version of the Smithsonian Astrophysical
//    Observatory Star Catalog (SAO, SAO Staff 1966). Subsequent
//    improvements by T. A. Nagy (1979) included the addition of equatorial
//    coordinates in radians and cross-identifications from the Table of
//    Correspondences SAO/HD/DM/GC (Morin 1973). As a prelude to creation of
//    the 1984 version of the SAO, a new version of the SAO-HD-GC-DM Cross
//    Index was prepared (Roman, Warren, and Schofield 1983). The 1984
//    version of the SAO contained the corrected and extended cross
//    identifications, all errata published up to January 1984 and known to
//    the ADC, numerous errors forwarded to the ADC by colleagues, and
//    errors discovered at the ADC during the course of this work. Clayton
//    A. Smith of the U. S. Naval Observatory provided J2000.0 positions and
//    proper motions for the SAO stars. Published and unpublished errors
//    discovered in the previous version (1984) have been corrected (up to
//    May 1991). The catalog contains SAO number; the right ascension and
//    declination with a B1950.0 equinox and epoch; annual proper motion and
//    its standard deviation, photographic and visual magnitudes; spectral
//    type; references to sources; the Durchmusterung (DM) identifier if the
//    star is listed in the Bonner DM (BD), Cordoba DM (CD), or Cape
//    Photographic DM (CP); component identification; The Henry Draper
//    (Extension) (HD or HDE) number; and J2000 positions and proper
//    motions. Multiple-star component identifications have been added to
//    stars where more than one SAO entry has the same DM number. The Henry
//    Draper Extension (HDE) numbers have been added for stars found in both
//    volumes of the extension. Data for duplicate SAO entries (those
//    referring to the same star) have been flagged. J2000 positions in
//    usual units and in radians have been added.
//
//File Summary:
//--------------------------------------------------------------------------------
// FileName  Lrecl   Records    Explanations
//--------------------------------------------------------------------------------
//ReadMe        80         .    This file
//sao.dat      204    258997    The SAO J2000.0 catalogue
//doc.txt       85      1479    Complete Documentation (extracted from the
//                              CD-ROM "Selected Astronomical Catalogs",
//                              Vol. 1, NASA 1991)
//--------------------------------------------------------------------------------
//
//Byte-by-byte Description of file: sao.dat
//--------------------------------------------------------------------------------
//   Bytes Format  Units   Label    Explanations
//--------------------------------------------------------------------------------
//   1-  6  I6     ---     SAO      [1/258997]+ SAO Catalog number
//       7  A1     ---     delFlag  [D] if star deleted (ignore all fields)
//   8-  9  I2     h       RAh      Hours RA, Equinox=B1950, Epoch=1950.0
//  10- 11  I2     min     RAm      Minutes RA, equinox B1950, Epoch=1950.0
//  12- 17  F6.3   s       RAs      Seconds RA, equinox B1950, Epoch=1950.0
//  18- 24  F7.4   s/a     pmRA     Annual proper motion in RA, FK4 system
//  25- 26  I2     mas/a e_pmRA     Standard deviation in pmRA
//      27  A1     ---     RA2mf    [+-] '+', add 1, or '-', substract 1,
//                                    RA minute: indication that the minutes
//                                    of time associated with the seconds
//                                    portion RA2 must be increased or
//                                    decreased by 1
//  28- 33  F6.3   s       RA2s     Seconds portion of RA, original epoch,
//                                    precessed to B1950
//  34- 35  I2     10mas e_RA2s     Standard deviation of RA2
//  36- 41  F6.1   a       EpRA2    Epoch of RA2 (RA original epoch)
//      42  A1     ---     DE-      Sign Dec, equinox B1950, Epoch=1950.0
//  43- 44  I2     deg     DEd      Degrees Dec, equinox B1950, Epoch=1950.0
//  45- 46  I2     arcmin  DEm      Minutes Dec, equinox B1950, Epoch=1950.0
//  47- 51  F5.2   arcsec  DEs      Seconds Dec, equinox B1950, Epoch=1950.0
//  52- 57  F6.3  arcsec/a pmDE     ? Annual proper motion in Dec, FK4 system (10)
//  58- 59  I2     mas/a e_pmDE     Standard deviation of Dec proper motion
//      60  A1     ---     DE2mf    [+-] '+', add 1, or '-', substract 1:
//                                    Indication that the arcminutes
//                                    associated with DE2 must be increased or
//                                    decreased by 1
//  61- 65  F5.2   arcsec  DE2s     Seconds of Declination, original epoch,
//                                    precessed to B1950
//  66- 67  I2     10mas e_DE2s     Standard deviation of DE2
//  68- 73  F6.1   a       EpDE2    Epoch of DE2 (Declinaation original epoch)
//  74- 76  I3     10mas e_Pos      Standard deviation of position at epoch 1950.0
//  77- 80  F4.1   mag     Pmag     []?=99.9 Photographic magnitude
//  81- 84  F4.1   mag     Vmag     []?=99.9 Visual magnitude
//  85- 87  A3     ---     SpType   Spectral type, '+++' for composite spectra
//  88- 89  I2     ---   r_Vmag     Coded source of visual magnitude (1)
//  90- 91  I2     ---   r_Num      Coded source of star number and footnotes (2)
//      92  I1     ---   r_Pmag     Coded source of photographic magnitude (3)
//      93  I1     ---   r_pmRA     Coded source of proper motions (4)
//      94  I1     ---   r_SpType   Coded source of spectral type (5)
//      95  I1     ---     Rem      Coded remarks duplicity and variability (6)
//      96  I1     ---   a_Vmag     Accuracy of V: 0 = 2 decimals, 1=1 decimal
//      97  I1     ---   a_Pmag     Accuracy of Ptg: 0 = 2 decimals, 1=1 decimal
//  98- 99  I2     ---   r_Cat      Code for source catalog (7)
// 100-104  I5     ---     CatNum   Number in source catalog
// 105-117  A13    ---     DM       Durchmusterung identification (9)
// 118-123  A6     ---     HD       Henry Draper Catalog (HD or HDE) number (A6)
//                                    (Catalogue <III/135>)
//     124  A1     ---   m_HD       HD component and multiple code (8)
// 125-129  A5     ---     GC       Boss General Catalog (GC) number
//                                    (Catalogue <I/113>)
// 130-139  D10.8  rad     RArad    Right ascension, 1950.0, in radians
// 140-150  D11.8  rad     DErad    Declination, 1950.0, in radians
// 151-152  I2     h       RA2000h  Hours RA, equinox, epoch J2000.0
// 153-154  I2     min     RA2000m  Minutes RA, equinox, epoch J2000.0
// 155-160  F6.3   s       RA2000s  Seconds RA, equinox, epoch J2000.0
// 161-167  F7.4   s/a     pmRA2000 Annual proper motion in FK5 system
//     168  A1     ---     DE2000-  Sign Dec, equinox, epoch J2000.0
// 169-170  I2     deg     DE2000d  Degrees Dec, equinox, epoch J2000.0
// 171-172  I2     arcmin  DE2000m  Minutes Dec, equinox, epoch J2000.0
// 173-177  F5.2   arcsec  DE2000s  Seconds Dec, equinox, epoch J2000.0
// 178-183  F6.3  arcsec/a pmDE2000 ? Annual proper motion in FK5 system (10)
// 184-193  D10.8  rad    RA2000rad Right ascension, J2000.0, in radians
// 194-204  D11.8  rad    DE2000rad Declination, J2000.0, in radians
//--------------------------------------------------------------------------------
//Note (1): Visual Magnitude Sources:
//  -------------------------------------------------------------------------
//         Photo-
//  Visual visual Magnitude source
//  -------------------------------------------------------------------------
//    0           Does not appear in source catalog
//    1      21   Determined by source catalog
//    2      22   Determined by source catalog or by authority in footnote
//    3      23   Source cited in source catalog introduction
//           24   Source unspecified
//    5           Taken from "Bonner Durchmusterung"
//    8           Based on Durchmusterung magnitudes and visual estimates
//    9           Taken from AGK1
//   10           Taken from Cordoba Zones (Resultados)
//   12           Taken from CGA (Perrine 1911a, b) or Cordoba Zones
//   13           Taken from Harvard publications
//   14           Taken from Harvard or San Luis photometry
//   15           Taken from the "Henry Draper Catalogue"
//   16           Combined magnitude of component stars
//   17           Arithmetic mean of maximum and minimum magnitudes of a
//                  variable star
//  -------------------------------------------------------------------------
//   Always check the duplicity / variability code (Note 6) when using
//   magnitudes. When blank, code = 0 and field = 99.9.
//
//Note (2): Star-Number Sources and Footnotes:
//  --------------------------------------------------
//    Footnote
//  without with  Star Number
//  --------------------------------------------------
//    0      16   Source catalog only
//    1      17   Source catalog and BD
//    2      18   Source catalog and CD
//    3      19   Source catalog and CPD
//    4      20   Cordoba B (Resultados) and CD
//    5      21   Cordoba A (Resultados) and CD
//    6      22   AGK1 and BD
//    7           GC and BD
//    8      24   Cordoba B (Resultados) and CPD
//    9           Cordoba A (Resultados) and CPD
//  --------------------------------------------------
//  When blank, the code for DM is 0 or 16, footnote is 0 through 9,
//  and field is all zeros. Footnotes and star numbers are those
//  appearing in the source catalogs.
//
//Note (3): Photographic Magnitude Sources:
//  ---------------------------------------------------
//  Code  Source
//  ---------------------------------------------------
//    0   Does not appear in source catalog
//    1   Determined by source catalog
//    4   Taken from magnitudes of the CPD and diameters
//        of the  Cape Astrographic Catalogue
//    8   Source cited in source catalog introduction
//    9   Columbia Contributions Numbers 30 and 31
//        (Schilt and Hill 1937, 1938)
//  ---------------------------------------------------
//  When blank, code is 0 and field is all zeros.
//
//Note (4): Proper-Motion Sources:
//  ---------------------------------------------------------
//  Code  Source
//  ---------------------------------------------------------
//    1   Determined by source catalog
//    3   Determined by comparison of catalog and Greenwich AC
//    5   Determined by comparison of catalog and AGK1
//    6   Determined by comparison of catalog and Greenwich AC
//        on the basis of the smallest difference in positions
//        (see page xiii of source reference)
//    8   Determined by comparison of catalog and AGK1 on the
//        basis of the smallest difference in positions
//        (see page xiii of source reference)
//  ---------------------------------------------------------
//
//Note (5): Spectral-Type Sources:
//  ----------------------------------------------------------
//  Code  Source
//  ----------------------------------------------------------
//    0   Taken from the Henry Draper Catalogue or no
//        spectrum in source catalog
//    1   Taken from the HD with M stars reclassified by Miss Cannon
//    2   Classified by G. G. Cillie
//    3   Classified by Goedicke
//    4   Classified by D. Hoffleit
//    5   Classified by M. W. Mayall
//    6   Classified at Leander McCormick Observatory
//    7   Classified by Nassau and Seyfert
//  ----------------------------------------------------------
//  If the spectrum is composite, "+++" is stored in the field
//  and the code is 0.
//
//Note (6):  Miscellaneous Coded Remarks for Duplicity and Variability:
//  -----------------------------------------------------
//  Code  Meaning
//  -----------------------------------------------------
//    0   No additional information
//    1   Double star - see source catalog for source
//    2   Double star in Aitken's Double Star Catalogue
//        (Aitken 1932)
//    3   Double star in Burnham's Double Star Catalogue
//        (Burnham 1906)
//    4   Variable star in visual magnitude in source catalog
//    5   Variable star in photographic magnitude in source catalog
//    6   Variable star in both magnitudes
//    7   Both double and variable, in either visual or
//        photographic magnitudes
//  -----------------------------------------------------
//  When blank, code is 0, no field is involved.
//
//Note (7): References for the Source Catalogs:
//  -----------------------------------------------------
//         No.      Abbreviated Title
//  -----------------------------------------------------
//         01       AGK2, Volume 1
//         02       AGK2, Volume 2
//         03       AGK2, Volume 5
//         04       AGK2, Volume 6
//         05       AGK2, Volume 7
//         06       AGK2, Volume 8
//         20       Yale Transactions 11
//         21       Yale Transactions 12   Part I
//         22       Yale Transactions 12   Part II
//         23       Yale Transactions 13   Part I
//         24       Yale Transactions 13   Part II
//         25       Yale Transactions 14
//         26       Yale Transactions 16
//         27       Yale Transactions 17
//         28       Yale Transactions 18
//         29       Yale Transactions 19
//         30       Yale Transactions 20
//         31       Yale Transactions 21
//         32       Yale Transactions 22   Part I
//         33       Yale Transactions 22   Part II
//         34       Yale Transactions 24
//         35       Yale Transactions 25
//         36       Yale Transactions 26   Part I
//         37       Yale Transactions 26   Part II
//         38       Yale Transactions 27
//         40       Cape Annals 17
//         41       Cape Annals 18
//         42       Cape Annals 19
//         43       Cape Annals 20
//         48       Cape Zone
//         60       Melbourne 3
//         61       Melbourne 4
//         70       GC
//         71       FK3
//         74       FK4
//  -----------------------------------------------------
//
//Note (8):  Explanation of HD Codes:
//  -------------------------------------------------------------
//  Code   Meaning
//  -------------------------------------------------------------
//    0    Single star or primary with a companion > 0.3 mag
//         (visual) fainter.
//    1    Brighter component with a companion <= 0.3 mag fainter.
//    2    Fainter component with a companion <= 0.3 mag brighter.
//    9    The "SAO Catalog" entry refers to two consecutive
//         HD numbers, the lower of which is given.
//  -------------------------------------------------------------
//
//Note (9): The DM designation is coded as:
//    catalog 'BD', 'CD', 'CP' (A2)
//    zone and number (A8), and
//    component identification (A2) if there are two or more SAO stars
//       having the same DM number
//    supplement letter (A1) for BD (Warren and Kress, Catalogue <I/71>)
//
//Note (10): There are 3 blank values for the Declination proper motions for:
//  SAO 208759     (pmDE=-0.180 in printed catalogue)
//  SAO 208795     (pmDE=-0.042 in printed catalogue)
//  SAO 212470     (pmDE=-0.072 in printed catalogue)
//--------------------------------------------------------------------------------
//
//See also:
//  A more recent catalogue with better position and proper motion data,
//  the so-called "PPM Catalogue" (Positions and Proper Motions)
//  has been prepared at ARI (Astronomisches Rechen-Institut at Heidelberg,
//  Germany); is is made of two parts:
//  I/146 : PPM North Star Catalogue, 181731 stars;
//  I/193 : PPM South Star Catalogue, 197179 stars.
//
//References:
//   Morin, D. 1973, Table of Correspondences SAO/HD/DM/GC, Obs. de Meudon,
//      unpublished                                       =IV/12
//   Nagy, T.A. 1979, Documentation for the Machine-Readable Version of the
//      Smithsonian Astrophysical Observatory Catalog(ue) (EBCDIC Version),
//      Systems and Applied Sciences Corporation R-SAW-7/79-34
//   Roman, N.G., Warren Jr., W.H., and Schofield Jr., N.J. 1983, Documentation
//      for the Machine-Readable Version of the SAO-HD-GC-DM Cross Index,
//      Version 1983, NSSDC/WDC-A-R&S 83-17
//
//Historical Notes:
// * Changes since 1984 Version: see in file "sao.doc", section 4.
//   by Nancy G. Roman and Wayne H. Warren.
// * October 1991: CD-ROM ""Selected Astronomical Catalogs" Vol. 1,
//   directory:  /astrom/saoj2000
// * 24-Jun-1993: First standardized document at CDS
// * 25-Oct-1995: Standardized document revisited.
//================================================================================
//(End)                                     Francois Ochsenbein [CDS]  27-Oct-1995
//*****************************************************************************

//*****************************************************************************
static void	SAO_ParseDataEntry_SaoDat(const char *lineBuff, TYPE_CelestData *objectStruct)
{
char	argString[64];

	objectStruct->dataSrc	=	kDataSrc_SAO;

	//   1-  6  I6     ---     SAO      [1/258997]+ SAO Catalog number
	ExtractTextField(lineBuff,	(1 - 1),	6,	argString, true);
	strcpy(objectStruct->longName, "SAO ");
	strcat(objectStruct->longName, argString);
	objectStruct->id	=	atoi(argString);

	//       7  A1     ---     delFlag  [D] if star deleted (ignore all fields)
	//   8-  9  I2     h       RAh      Hours RA, Equinox=B1950, Epoch=1950.0
	//  10- 11  I2     min     RAm      Minutes RA, equinox B1950, Epoch=1950.0
	//  12- 17  F6.3   s       RAs      Seconds RA, equinox B1950, Epoch=1950.0
	//  18- 24  F7.4   s/a     pmRA     Annual proper motion in RA, FK4 system
	//  25- 26  I2     mas/a e_pmRA     Standard deviation in pmRA
	//      27  A1     ---     RA2mf    [+-] '+', add 1, or '-', substract 1,
	//                                    RA minute: indication that the minutes
	//                                    of time associated with the seconds
	//                                    portion RA2 must be increased or
	//                                    decreased by 1
	//  28- 33  F6.3   s       RA2s     Seconds portion of RA, original epoch,
	//                                    precessed to B1950
	//  34- 35  I2     10mas e_RA2s     Standard deviation of RA2
	//  36- 41  F6.1   a       EpRA2    Epoch of RA2 (RA original epoch)
	//      42  A1     ---     DE-      Sign Dec, equinox B1950, Epoch=1950.0
	//  43- 44  I2     deg     DEd      Degrees Dec, equinox B1950, Epoch=1950.0
	//  45- 46  I2     arcmin  DEm      Minutes Dec, equinox B1950, Epoch=1950.0
	//  47- 51  F5.2   arcsec  DEs      Seconds Dec, equinox B1950, Epoch=1950.0
	//  52- 57  F6.3  arcsec/a pmDE     ? Annual proper motion in Dec, FK4 system (10)
	//  58- 59  I2     mas/a e_pmDE     Standard deviation of Dec proper motion
	//      60  A1     ---     DE2mf    [+-] '+', add 1, or '-', substract 1:
	//                                    Indication that the arcminutes
	//                                    associated with DE2 must be increased or
	//                                    decreased by 1
	//  61- 65  F5.2   arcsec  DE2s     Seconds of Declination, original epoch,
	//                                    precessed to B1950
	//  66- 67  I2     10mas e_DE2s     Standard deviation of DE2
	//  68- 73  F6.1   a       EpDE2    Epoch of DE2 (Declinaation original epoch)
	//  74- 76  I3     10mas e_Pos      Standard deviation of position at epoch 1950.0
	//  77- 80  F4.1   mag     Pmag     []?=99.9 Photographic magnitude

	//  81- 84  F4.1   mag     Vmag     []?=99.9 Visual magnitude
	ExtractTextField(lineBuff,	(81 - 1),	5,	argString, true);
	objectStruct->realMagnitude	=	atof(argString);

	//  85- 87  A3     ---     SpType   Spectral type, '+++' for composite spectra
	ExtractTextField(lineBuff,	(85 - 1),	3,	argString, true);
	objectStruct->spectralClass	=	argString[0];

	//  88- 89  I2     ---   r_Vmag     Coded source of visual magnitude (1)
	//  90- 91  I2     ---   r_Num      Coded source of star number and footnotes (2)
	//      92  I1     ---   r_Pmag     Coded source of photographic magnitude (3)
	//      93  I1     ---   r_pmRA     Coded source of proper motions (4)
	//      94  I1     ---   r_SpType   Coded source of spectral type (5)
	//      95  I1     ---     Rem      Coded remarks duplicity and variability (6)
	//      96  I1     ---   a_Vmag     Accuracy of V: 0 = 2 decimals, 1=1 decimal
	//      97  I1     ---   a_Pmag     Accuracy of Ptg: 0 = 2 decimals, 1=1 decimal
	//  98- 99  I2     ---   r_Cat      Code for source catalog (7)
	// 100-104  I5     ---     CatNum   Number in source catalog
	// 105-117  A13    ---     DM       Durchmusterung identification (9)

	// 118-123  A6     ---     HD       Henry Draper Catalog (HD or HDE) number (A6)
	ExtractTextField(lineBuff,	(118 - 1),	6,	argString, true);
	if (strlen(argString) > 0)
	{
		strcat(objectStruct->longName, "/HD");
		strcat(objectStruct->longName, argString);
	}
	//                                    (Catalogue <III/135>)
	//     124  A1     ---   m_HD       HD component and multiple code (8)
	// 125-129  A5     ---     GC       Boss General Catalog (GC) number
	//                                    (Catalogue <I/113>)
	// 130-139  D10.8  rad     RArad    Right ascension, 1950.0, in radians
	// 140-150  D11.8  rad     DErad    Declination, 1950.0, in radians
	// 151-152  I2     h       RA2000h  Hours RA, equinox, epoch J2000.0
	// 153-154  I2     min     RA2000m  Minutes RA, equinox, epoch J2000.0
	// 155-160  F6.3   s       RA2000s  Seconds RA, equinox, epoch J2000.0

	// 161-167  F7.4   s/a     pmRA2000 Annual proper motion in FK5 system
	ExtractTextField(lineBuff,	(161 - 1),	7,	argString, true);
	objectStruct->propMotion_RA_mas_yr		=	atof(argString) * 1000.0;

	//     168  A1     ---     DE2000-  Sign Dec, equinox, epoch J2000.0
	// 169-170  I2     deg     DE2000d  Degrees Dec, equinox, epoch J2000.0
	// 171-172  I2     arcmin  DE2000m  Minutes Dec, equinox, epoch J2000.0
	// 173-177  F5.2   arcsec  DE2000s  Seconds Dec, equinox, epoch J2000.0

	// 178-183  F6.3  arcsec/a pmDE2000 ? Annual proper motion in FK5 system (10)
	ExtractTextField(lineBuff,	(178 - 1),	7,	argString, true);
	objectStruct->propMotion_DEC_mas_yr		=	atof(argString) * 1000.0;
	if ((fabs(objectStruct->propMotion_RA_mas_yr) > 0.0) || (fabs(objectStruct->propMotion_DEC_mas_yr) > 0.0))
	{
		objectStruct->propMotionValid	=	true;
//		CONSOLE_DEBUG_W_DBL("propMotion_RA_mas_yr \t=",	objectStruct->propMotion_RA_mas_yr);
//		CONSOLE_DEBUG_W_DBL("propMotion_DEC_mas_yr\t=",	objectStruct->propMotion_DEC_mas_yr);
	}

	// 184-193  D10.8  rad    RA2000rad Right ascension, J2000.0, in radians
	ExtractTextField(lineBuff,	(184 - 1),	10,	argString, true);
	objectStruct->org_ra	=	atof(argString);
	objectStruct->ra		=	objectStruct->org_ra;


	// 194-204  D11.8  rad    DE2000rad Declination, J2000.0, in radians
	ExtractTextField(lineBuff,	(194 - 1),	10,	argString, true);
	objectStruct->org_decl	=	atof(argString);
	objectStruct->decl		=	objectStruct->org_ra;

}

//*****************************************************************************
TYPE_CelestData	*SAO_ReadFile_sao_dat(const char *filePath, long *starCount)
{
FILE			*filePointer;
char			lineBuff[300];
bool			keepReading;
int				linesRead;
int				recordCount;
int				deletedCount;
TYPE_CelestData	*saoStarData;
int				startupWidgetIdx;

//	CONSOLE_DEBUG(__FUNCTION__);

	startupWidgetIdx	=	SetStartupText("SAO-dat catalog:");

	deletedCount	=	0;
	linesRead		=	0;
	filePointer		=	fopen(filePath, "r");
	if (filePointer != NULL)
	{

		//*	allocate the array for the star data
		saoStarData	=	(TYPE_CelestData *)calloc(kMaxSAOcount, sizeof(TYPE_CelestData));

		if (saoStarData != NULL)
		{
			keepReading	=	true;
			recordCount	=	0;
			while (keepReading && (recordCount < kMaxSAOcount))
			{
				if (fread(lineBuff, 205, 1, filePointer))
				{
					linesRead++;
					if ((lineBuff[0] == 0x20) || isdigit(lineBuff[0]))
					{
						//*	check for deleted records
						if (lineBuff[6] != 'D')
						{
							SAO_ParseDataEntry_SaoDat(lineBuff, &saoStarData[recordCount]);
							recordCount++;
						}
						else
						{
							deletedCount++;
//							CONSOLE_DEBUG(lineBuff);
						}
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
		SetStartupTextStatus(startupWidgetIdx, "OK");
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to read star data:", filePath);
		SetStartupTextStatus(startupWidgetIdx, "Failed");
	}
//
//	CONSOLE_DEBUG_W_NUM(	"deletedCount\t=", deletedCount);
//	CONSOLE_DEBUG_W_LONG(	"starCount   \t=", *starCount);

	return(saoStarData);
}

//*****************************************************************************
TYPE_CelestData	*SAO_ReadFile(long *starCount)
{
struct stat		fileStatus;
int				returnCode;
char			filePath[128];
TYPE_CelestData	*saoStarData;

//	CONSOLE_DEBUG(__FUNCTION__);
	strcpy(filePath, kSkyTravelDataDirectory);
	strcat(filePath, "/sao/sao.dat");

	//*	fstat - check for existence of file
//	CONSOLE_DEBUG_W_STR("Checking on:", filePath);
	returnCode	=	stat(filePath, &fileStatus);
	if (returnCode == 0)
	{
//		CONSOLE_DEBUG_W_STR("file is present:", filePath);
		saoStarData	=	SAO_ReadFile_sao_dat(filePath, starCount);
	}
	else
	{
		saoStarData	=	SAO_ReadFile_heasarc(starCount);
	}
	if (saoStarData == NULL)
	{
		SetStartupText("SAO catalog:\tNot found");
	}
	return(saoStarData);
}

#ifdef _INCLUDE_SAO_MAIN_


//*****************************************************************************
int main(int argc, char *argv[])
{
long	starCount;

//	CONSOLE_DEBUG("SAO read test");
	starCount	=	0;

	SAO_ReadFileEitherFile(&starCount);
	CONSOLE_DEBUG_W_LONG("starCount\t=", starCount);

}
#endif // _INCLUDE_SAO_MAIN_
