//************************************************************************
//*	Hipparcos data
//************************************************************************
//*	http://adc.gsfc.nasa.gov/adc-cgi/cat.pl?/catalogs/1/1239/
//************************************************************************



/*
Byte-by-byte Description of file: hip_main.dat
--------------------------------------------------------------------------------
   Bytes Format Units   Label     Explanations
--------------------------------------------------------------------------------
       1  A1    ---     Catalog   [H] Catalogue (H=Hipparcos)               (H0)
   9- 14  I6    ---     HIP       Identifier (HIP number)                   (H1)
      16  A1    ---     Proxy     [HT] Proximity flag                       (H2)
  18- 28  A11   ---     RAhms    *Right ascension in h m s, ICRS (Eq=J2000) (H3)
  30- 40  A11   ---     DEdms    *Declination in deg ' ", ICRS (Eq=J2000)   (H4)
  42- 46  F5.2  mag     Vmag      ? Magnitude in Johnson V                  (H5)
      48  I1    ---     VarFlag  *[1,3]? Coarse variability flag            (H6)
      50  A1    ---   r_Vmag     *[GHT] Source of magnitude                 (H7)
  52- 63  F12.8 deg     RAdeg    *? alpha, degrees (ICRS, Eq=J2000)         (H8)
  65- 76  F12.8 deg     DEdeg    *? delta, degrees (ICRS, Eq=J2000)         (H9)
      78  A1    ---     AstroRef *[*+A-Z] Reference flag for astrometry    (H10)
  80- 86  F7.2  mas     Plx       ? Trigonometric parallax                 (H11)
  88- 95  F8.2 mas/yr   pmRA      ? Proper motion mu_alpha.cos(delta), ICRS(H12)
  97-104  F8.2 mas/yr   pmDE      ? Proper motion mu_delta, ICRS           (H13)
 106-111  F6.2  mas   e_RAdeg     ? Standard error in RA*cos(DEdeg)        (H14)
 113-118  F6.2  mas   e_DEdeg     ? Standard error in DE                   (H15)
 120-125  F6.2  mas   e_Plx       ? Standard error in Plx                  (H16)
 127-132  F6.2 mas/yr e_pmRA      ? Standard error in pmRA                 (H17)
 134-139  F6.2 mas/yr e_pmDE      ? Standard error in pmDE                 (H18)
 141-145  F5.2  ---     DE:RA     [-1/1]? Correlation, DE/RA*cos(delta)    (H19)
 147-151  F5.2  ---     Plx:RA    [-1/1]? Correlation, Plx/RA*cos(delta)   (H20)
 153-157  F5.2  ---     Plx:DE    [-1/1]? Correlation, Plx/DE              (H21)
 159-163  F5.2  ---     pmRA:RA   [-1/1]? Correlation, pmRA/RA*cos(delta)  (H22)
 165-169  F5.2  ---     pmRA:DE   [-1/1]? Correlation, pmRA/DE             (H23)
 171-175  F5.2  ---     pmRA:Plx  [-1/1]? Correlation, pmRA/Plx            (H24)
 177-181  F5.2  ---     pmDE:RA   [-1/1]? Correlation, pmDE/RA*cos(delta)  (H25)
 183-187  F5.2  ---     pmDE:DE   [-1/1]? Correlation, pmDE/DE             (H26)
 189-193  F5.2  ---     pmDE:Plx  [-1/1]? Correlation, pmDE/Plx            (H27)
 195-199  F5.2  ---     pmDE:pmRA [-1/1]? Correlation, pmDE/pmRA           (H28)
 201-203  I3    %       F1        ? Percentage of rejected data            (H29)
 205-209  F5.2  ---     F2       *? Goodness-of-fit parameter              (H30)
 211-216  I6    ---     ---       HIP number (repetition)                  (H31)
 218-223  F6.3  mag     BTmag     ? Mean BT magnitude                      (H32)
 225-229  F5.3  mag   e_BTmag     ? Standard error on BTmag                (H33)
 231-236  F6.3  mag     VTmag     ? Mean VT magnitude                      (H34)
 238-242  F5.3  mag   e_VTmag     ? Standard error on VTmag                (H35)
     244  A1    ---   m_BTmag    *[A-Z*-] Reference flag for BT and VTmag  (H36)
 246-251  F6.3  mag     B-V       ? Johnson B-V colour                     (H37)
 253-257  F5.3  mag   e_B-V       ? Standard error on B-V                  (H38)
     259  A1    ---   r_B-V       [GT] Source of B-V from Ground or Tycho  (H39)
 261-264  F4.2  mag     V-I       ? Colour index in Cousins' system        (H40)
 266-269  F4.2  mag   e_V-I       ? Standard error on V-I                  (H41)
     271  A1    ---   r_V-I      *[A-T] Source of V-I                      (H42)
     273  A1    ---     CombMag   [*] Flag for combined Vmag, B-V, V-I     (H43)
 275-281  F7.4  mag     Hpmag    *? Median magnitude in Hipparcos system   (H44)
 283-288  F6.4  mag   e_Hpmag    *? Standard error on Hpmag                (H45)
 290-294  F5.3  mag     Hpscat    ? Scatter on Hpmag                       (H46)
 296-298  I3    ---   o_Hpmag     ? Number of observations for Hpmag       (H47)
     300  A1    ---   m_Hpmag    *[A-Z*-] Reference flag for Hpmag         (H48)
 302-306  F5.2  mag     Hpmax     ? Hpmag at maximum (5th percentile)      (H49)
 308-312  F5.2  mag     HPmin     ? Hpmag at minimum (95th percentile)     (H50)
 314-320  F7.2  d       Period    ? Variability period (days)              (H51)
     322  A1    ---     HvarType *[CDMPRU]? variability type               (H52)
     324  A1    ---     moreVar  *[12] Additional data about variability   (H53)
     326  A1    ---     morePhoto [ABC] Light curve Annex                  (H54)
 328-337  A10   ---     CCDM      CCDM identifier                          (H55)
     339  A1    ---   n_CCDM     *[HIM] Historical status flag             (H56)
 341-342  I2    ---     Nsys      ? Number of entries with same CCDM       (H57)
 344-345  I2    ---     Ncomp     ? Number of components in this entry     (H58)
     347  A1    ---     MultFlag *[CGOVX] Double/Multiple Systems flag     (H59)
     349  A1    ---     Source   *[PFILS] Astrometric source flag          (H60)
     351  A1    ---     Qual     *[ABCDS] Solution quality                 (H61)
 353-354  A2    ---   m_HIP       Component identifiers                    (H62)
 356-358  I3    deg     theta     ? Position angle between components      (H63)
 360-366  F7.3  arcsec  rho       ? Angular separation between components  (H64)
 368-372  F5.3  arcsec  e_rho     ? Standard error on rho                  (H65)
 374-378  F5.2  mag     dHp       ? Magnitude difference of components     (H66)
 380-383  F4.2  mag   e_dHp       ? Standard error on dHp                  (H67)
     385  A1    ---     Survey    [S] Flag indicating a Survey Star        (H68)
     387  A1    ---     Chart    *[DG] Identification Chart                (H69)
     389  A1    ---     Notes    *[DGPWXYZ] Existence of notes             (H70)
 391-396  I6    ---     HD        [1/359083]? HD number <III/135>          (H71)
 398-407  A10   ---     BD        Bonner DM <I/119>, <I/122>               (H72)
 409-418  A10   ---     CoD       Cordoba Durchmusterung (DM) <I/114>      (H73)
 420-429  A10   ---     CPD       Cape Photographic DM <I/108>             (H74)
 431-434  F4.2  mag     (V-I)red  V-I used for reductions                  (H75)
 436-447  A12   ---     SpType    Spectral type                            (H76)
     449  A1    ---   r_SpType   *[1234GKSX]? Source of spectral type      (H77)
--------------------------------------------------------------------------------
Note on RAhms, DEdms, RAdeg, DEdeg: right ascension and declination are
     expressed for epoch J1991.25 (JD2448349.0625 (TT)) in the
     ICRS (International Celestial Reference System, consistent with
     J2000) reference system.
     There are 263 cases where these fields are missing (no astrometric
     solution could be found)
Note on VarFlag: the values are
     1: < 0.06mag ; 2: 0.06-0.6mag ; 3: >0.6mag
Note on r_Vmag: the source is
     G = ground-based, H=HIP, T=Tycho
Note on AstroRef: this flag indicates that the astrometric parameters in H3-4
     and H8-30 refer to:
     A to Z: the letter indicates the component of a double or multiple system
     *: the photocentre of a double or multiple system
     +: the centre of mass
Note on F2: values exceeding +3 indicate a bad fit to the data.
Note on m_BTmag: this flag indicates the component or combined photometry:
     A to Z : the letter indicates the component measured in Tycho
              (non-single star)
     * : the photometry refers to all components of the Hipparcos entry
     - : single-pointing triple or quadruple system
Note on r_V-I: the origin of the V-I colour, in summary:
     'A'        for an observation of V-I in Cousins' system;
     'B' to 'K' when V-I derived from measurements in other
                bands/photoelectric systems
     'L' to 'P' when V-I derived from Hipparcos and Star Mapper photometry
     'Q'        for long-period variables
     'R' to 'T' when colours are unknown
Note on Hpmag, e_Hpmag:
     the Hipparcos magnitude could not be determined for 14 stars.
Note on m_Hpmag: this flag indicates for double or multiple entries:
     A to Z : the letter indicates the specified component measured
     * : combined Hpmag of a double system, corrected for attenuation
     - : combined Hpmag of a multiple system, not corrected for attenuation
Note on HvarType: Hipparcos-defined type of variability (a blank entry
     signifies that the entry could not be classified as variable or constant):
     C : no variability detected ("constant")
     D : duplicity-induced variability
     M : possibly micro-variable (amplitude < 0.03mag)
     P : periodic variable
     R : V-I colour index was revised due to variability analysis
     U : unsolved variable which does not fall in the other categories
Note on moreVar: more data about periodic variability are provided
Note on n_CCDM: the flag takes the following values:
     H : determined multiple by Hipparcos, previously unknown
     I : system previously identified as multiple in HIC <I/196> (annex1)
     M : miscellaneous (system identified after publication of HIC)
Note on MultFlag: indicates that further details are given in the Double
     and Multiple Systems Annex:
     C : solutions for the components
     G : acceleration or higher order terms
     O : orbital solutions
     V : variability-induced movers (apparent motion arises from variability)
     X : stochastic solution (probably astrometric binaries with short period)
Note on Source: qualifies the source of the astrometric parameters H8-30
        with a 'C' in MultFlag:
     P : primary target of a 2- or 3-pointing system
     F : secondary or tertiary of a 2- or 3-pointing 'fixed' system
         (common parallax and proper motions)
     I : secondary or tertiary of a 2- or 3-pointing 'independent' system
         (no constraints on parallax or proper motions)
     L : secondary or tertiary of a 2- or 3-pointing 'linear' system
         (common parallax)
     S : astrometric parameters from 'single-star merging' process.
Note on Qual: Reliability of the double or multiple star solution:
        A=good, B=fair, C=poor, D=uncertain, S=suspected non-single
Note on Chart: the chart was produced:
     D : from the STScI Digitized Sky Survey
     G : from the Guide Star Catalog
Note on Notes: the flag has the following meaning:
     D : double and multiple systems note only (note in hd_notes.dat file)
     G : general note only (note in hg_notes.dat file)
     P : photometric notes only (note in hp_notes.dat file)
     W : D + P
     X : D + G
     Y : G + P
     Z : D + G + P
Note on r_SpType: the flag indicates the source, as:
     1 : Michigan catalogue for the HD stars, vol. 1 (Houk+, 1975) <III/31>
     2 : Michigan catalogue for the HD stars, vol. 2 (Houk, 1978)  <III/51>
     3 : Michigan Catalogue for the HD stars, vol. 3 (Houk, 1982)  <III/80>
     4 : Michigan Catalogue for the HD stars, vol. 4 (Houk+, 1988) <III/133>
     G : updated after publication of the HIC <I/196>
     K : General Catalog of Variable Stars, 4th Ed. (Kholopov+ 1988) <II/139>
     S : SIMBAD data-base <http://cdsweb.u-strasbg.fr/Simbad.html>
     X : Miscellaneous
     A blank entry has no corresponding information.
--------------------------------------------------------------------------------
*/

#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<stdio.h>

//*	MLS Libraries
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"SkyStruc.h"
#include	"SkyTravelConstants.h"
#include	"StarData.h"

#include	"StarCatalogHelper.h"


#include	"HipparcosCatalog.h"

double	gSmallestMag	=	100.0;
double	gLargestMag		=	0.0;
int		g99Count		=	0;

//************************************************************************
static bool	ParseOneLineOfHipparcosStarCatalog(char *lineBuff, TYPE_CelestData *starRec)
{
bool		validObject;
long		raHour;
long		raMin;
double		raSec;
double		raHours;
double		raDegrees;
long		deDeg;
long		deMin;
double		deSec;
double		declDegrees;
double		raRadians, declRadians;
short		deSign;
char		icSourceDesignator;
char		raString[32];
char		deString[32];

//	CONSOLE_DEBUG(lineBuff);

	memset(starRec, 0, sizeof(TYPE_CelestData));

	//*	the reason for the "-1" is so that you can reference the above documentation easier
	validObject					=	true;
	icSourceDesignator			=	lineBuff[0];
	if (icSourceDesignator != 'H')
	{
		CONSOLE_DEBUG("invalid data")
	}
	starRec->id					=	ParseLongFromString(lineBuff,		 9-1, 6);

	strncpyZero(raString, &lineBuff[18-1], 11);	//*	a11
	strncpyZero(deString, &lineBuff[30-1], 11);


	raHour						=	ParseLongFromString(lineBuff,		18-1, 2);
	raMin						=	ParseFloatFromString(lineBuff,		21-1, 2);
	raSec						=	ParseFloatFromString(lineBuff,		24-1, 5);

	deSign						=	lineBuff[30-1];
	deDeg						=	ParseLongFromString(lineBuff,		31-1, 2);
	deMin						=	ParseLongFromString(lineBuff,		34-1, 2);
	deSec						=	ParseFloatFromString(lineBuff,		37-1, 5);

//	starRec->realMagnitude		=	ParseFloatFromString(lineBuff,		42-1, 5);

// 275-281  F7.4  mag     Hpmag    *? Median magnitude in Hipparcos system   (H44)
	starRec->realMagnitude		=	ParseFloatFromString(lineBuff,		275-1, 7);

	if (starRec->realMagnitude > gLargestMag)
	{
		gLargestMag	=	starRec->realMagnitude;
	}
	if (starRec->realMagnitude < gSmallestMag)
	{
		gSmallestMag	=	starRec->realMagnitude;
	}

	if (starRec->realMagnitude == 0.0)
	{
		starRec->realMagnitude	=	99.0;
		g99Count++;
	}

//  80- 86  F7.2  mas     Plx       ? Trigonometric parallax                 (H11)
//	starRec->parallax			=	ParseFloatFromString(lineBuff,		80-1, 7);


	raHours			=	raHour + (raMin / 60.0) + (raSec / 3600.0);
	raDegrees		=	raHours * 15;
	declDegrees		=	deDeg + (deMin / 60.0) + (deSec / 3600.0);

	raRadians		=	(raDegrees * PI) / 180.0;
	declRadians		=	(declDegrees * PI) / 180.0;

	if (deSign == '-')
	{
		declRadians	=	-declRadians;
	}

	starRec->ra			=	raRadians;
	starRec->decl		=	declRadians;
	starRec->org_ra		=	raRadians;
	starRec->org_decl	=	declRadians;
	starRec->dataSrc	=	kDataSrc_Hipparcos;

	return(validObject);
}

#define	kHIPrecordSize	451

//************************************************************************
TYPE_CelestData	*ReadHipparcosStarCatalog(long *starCount)
{
FILE			*filePointer;
long			hipFileSize;
long			hipLineCount;
long			recordCount;
TYPE_CelestData	*hipStarData;
TYPE_CelestData	starObject;
bool			validObject;
char			lineBuff[512];
char			filePath[128];
size_t			bufferSize;

	CONSOLE_DEBUG(__FUNCTION__);

	hipStarData	=	NULL;

	strcpy(filePath, kSkyTravelDataDirectory);
	strcat(filePath, "/hip_main.dat");

	filePointer	=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
		//*	get the file size
		hipFileSize		=	GetFileSize(filePath);
		hipLineCount	=	hipFileSize / kHIPrecordSize;

		CONSOLE_DEBUG_W_LONG("hipFileSize\t=", hipFileSize);
		CONSOLE_DEBUG_W_LONG("hipLineCount\t=", hipLineCount);


		bufferSize	=	hipLineCount * sizeof(TYPE_CelestData);
		hipStarData	=	(TYPE_CelestData *)malloc(bufferSize);

		if (hipStarData != NULL)
		{
			memset(hipStarData, 0, bufferSize);

	//		hipLineCount	=	10;
			recordCount		=	0;
		//	while (fgets(lineBuff, kHIPrecordSize, filePointer) && (recordCount < hipLineCount))
			while (fread(lineBuff, kHIPrecordSize, 1, filePointer) && (recordCount < hipLineCount))
			{
				lineBuff[kHIPrecordSize]	=	0;
				validObject	=	ParseOneLineOfHipparcosStarCatalog(lineBuff, &starObject);
				if (validObject)
				{
					hipStarData[recordCount]	=	starObject;
					recordCount++;
				}
			}
			*starCount	=	recordCount;
		}
		fclose(filePointer);
	}

	CONSOLE_DEBUG_W_DBL("gLargestMag\t=", gLargestMag);
	CONSOLE_DEBUG_W_DBL("gSmallestMag\t=", gSmallestMag);
	CONSOLE_DEBUG_W_NUM("g99Count\t=", g99Count);

	return(hipStarData);
}


