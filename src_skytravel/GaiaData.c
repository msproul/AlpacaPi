//*****************************************************************************
//*	GaiaData.c
//*		https://gea.esac.esa.int/archive/
//*		https://gea.esac.esa.int/archive/documentation/GEDR3/Gaia_archive/chap_datamodel/sec_dm_main_tables/ssec_dm_gaia_source.html
//*		https://www.gaia.ac.uk/data/gaia-data-release-2
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Nov  9,	2021	<MLS> Created GaiaData.c
//*	Nov 11,	2021	<MLS> Working on extraction
//*	Nov 12,	2021	<MLS> Added ReadGaiaExtractedList()
//*	Dec  8,	2021	<MLS> Added CalcSpectralClassFrom_BP_RP()
//*****************************************************************************

#ifdef _ENABLE_GAIA_
#error "Not used any more"

#include	<stdbool.h>
#include	<string.h>
#include	<stdlib.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<math.h>
#include	<ctype.h>
#include	<dirent.h>
#include	<errno.h>

#define	RADIANS(degrees)	((degrees) * (M_PI / 180.0))

//*	MLS Libraries
#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"SkyStruc.h"
#include	"helper_functions.h"

#include	"GaiaData.h"

#define	kGaiaStarCntMax	300000

//**************************************************************************
//*	https://gea.esac.esa.int/archive/documentation/GEDR3/Gaia_archive/chap_datamodel/sec_dm_main_tables/ssec_dm_gaia_source.html
//**************************************************************************
enum
{
	kGAIA_solution_id	=	0,
	kGAIA_designation,
	kGAIA_source_id,
	kGAIA_random_index,
	kGAIA_ref_epoch,
	kGAIA_ra,
	kGAIA_ra_error,
	kGAIA_dec,
	kGAIA_dec_error,
	kGAIA_parallax,
	kGAIA_parallax_error,
	kGAIA_parallax_over_error,
	kGAIA_pm,
	kGAIA_pmra,
	kGAIA_pmra_error,
	kGAIA_pmdec,
	kGAIA_pmdec_error,
	kGAIA_ra_dec_corr,
	kGAIA_ra_parallax_corr,
	kGAIA_ra_pmra_corr,
	kGAIA_ra_pmdec_corr,
	kGAIA_dec_parallax_corr,
	kGAIA_dec_pmra_corr,
	kGAIA_dec_pmdec_corr,
	kGAIA_parallax_pmra_corr,
	kGAIA_parallax_pmdec_corr,
	kGAIA_pmra_pmdec_corr,
	kGAIA_astrometric_n_obs_al,
	kGAIA_astrometric_n_obs_ac,
	kGAIA_astrometric_n_good_obs_al,
	kGAIA_astrometric_n_bad_obs_al,
	kGAIA_astrometric_gof_al,
	kGAIA_astrometric_chi2_al,
	kGAIA_astrometric_excess_noise,
	kGAIA_astrometric_excess_noise_sig,
	kGAIA_astrometric_params_solved,
	kGAIA_astrometric_primary_flag,
	kGAIA_nu_eff_used_in_astrometry,
	kGAIA_pseudocolour,
	kGAIA_pseudocolour_error,
	kGAIA_ra_pseudocolour_corr,
	kGAIA_dec_pseudocolour_corr,
	kGAIA_parallax_pseudocolour_corr,
	kGAIA_pmra_pseudocolour_corr,
	kGAIA_pmdec_pseudocolour_corr,
	kGAIA_astrometric_matched_transits,
	kGAIA_visibility_periods_used,
	kGAIA_astrometric_sigma5d_max,
	kGAIA_matched_transits,
	kGAIA_new_matched_transits,
	kGAIA_matched_transits_removed,
	kGAIA_ipd_gof_harmonic_amplitude,
	kGAIA_ipd_gof_harmonic_phase,
	kGAIA_ipd_frac_multi_peak,
	kGAIA_ipd_frac_odd_win,
	kGAIA_ruwe,
	kGAIA_scan_direction_strength_k1,
	kGAIA_scan_direction_strength_k2,
	kGAIA_scan_direction_strength_k3,
	kGAIA_scan_direction_strength_k4,
	kGAIA_scan_direction_mean_k1,
	kGAIA_scan_direction_mean_k2,
	kGAIA_scan_direction_mean_k3,
	kGAIA_scan_direction_mean_k4,
	kGAIA_duplicated_source,
	kGAIA_phot_g_n_obs,
	kGAIA_phot_g_mean_flux,
	kGAIA_phot_g_mean_flux_error,
	kGAIA_phot_g_mean_flux_over_error,
	kGAIA_phot_g_mean_mag,
	kGAIA_phot_bp_n_obs,
	kGAIA_phot_bp_mean_flux,
	kGAIA_phot_bp_mean_flux_error,
	kGAIA_phot_bp_mean_flux_over_error,
	kGAIA_phot_bp_mean_mag,
	kGAIA_phot_rp_n_obs,
	kGAIA_phot_rp_mean_flux,
	kGAIA_phot_rp_mean_flux_error,
	kGAIA_phot_rp_mean_flux_over_error,
	kGAIA_phot_rp_mean_mag,
	kGAIA_phot_bp_n_contaminated_transits,
	kGAIA_phot_bp_n_blended_transits,
	kGAIA_phot_rp_n_contaminated_transits,
	kGAIA_phot_rp_n_blended_transits,
	kGAIA_phot_proc_mode,
	kGAIA_phot_bp_rp_excess_factor,
	kGAIA_bp_rp,
	kGAIA_bp_g,
	kGAIA_g_rp,
	kGAIA_dr2_radial_velocity,
	kGAIA_dr2_radial_velocity_error,
	kGAIA_dr2_rv_nb_transits,
	kGAIA_dr2_rv_template_teff,
	kGAIA_dr2_rv_template_logg,
	kGAIA_dr2_rv_template_fe_h,
	kGAIA_l,
	kGAIA_b,
	kGAIA_ecl_lon,
	kGAIA_ecl_lat,

	kGAIA_last
};

double	gBP_RP_min	=	99;
double	gBP_RP_max	=	0;

#ifdef _INCLUDE_GAIA_MAIN_
double	gLowestMagnitude	=	999.0;
int		gLowMagCnt			=	0;
int		gFileCount			=	0;
int		gMetLimitsCount		=	0;
int		gMagnitudeDistro[10];




FILE	*gOutputFilePtr		=	NULL;

#endif // _INCLUDE_GAIA_MAIN_

//**************************************************************************
static char	CalcSpectralClassFrom_BP_RP(double bp_rp)
{
char	spectralClass;
double	bpLimits[]	=	{0.435,	0.870, 1.304, 1.729, 2.174, 2.609 };

	if (bp_rp < bpLimits[0])
	{
		spectralClass	=	'O';
	}
	else if (bp_rp < bpLimits[1])
	{
		spectralClass	=	'B';
	}
	else if (bp_rp < bpLimits[2])
	{
		spectralClass	=	'A';
	}
	else if (bp_rp < bpLimits[3])
	{
		spectralClass	=	'F';
	}
	else if (bp_rp < bpLimits[4])
	{
		spectralClass	=	'G';
	}
	else if (bp_rp < bpLimits[5])
	{
		spectralClass	=	'k';
	}
	else
	{
		spectralClass	=	'M';
	}
	return(spectralClass);
}


//**************************************************************************
static bool	ParseOneLineGaiaData(const char *lineBuff, TYPE_CelestData *gaiaStar)
{
bool	validData;
int		iii;
int		ccc;
int		sLen;
int		argNum;
char	theChar;
char	argString[64];
double	magnitude;
double	rightAscen;
double	declination;
char	designation[64];
bool	foundOccultation	=	false;
int		designation_sLen;
double	bp_rp;
#ifdef _INCLUDE_GAIA_MAIN_
	int		magnitudeBinValue;
#endif // _INCLUDE_GAIA_MAIN_


	validData	=	false;
	sLen		=	strlen(lineBuff);
	argNum		=	0;
	ccc			=	0;
	magnitude	=	99;
	rightAscen	=	0.0;
	declination	=	0.0;
	strcpy(designation, "GAIA-unknown");
	for (iii=0; iii <= sLen; iii++)
	{
		theChar	=	lineBuff[iii];
		if ((theChar == ',') || (theChar == 0))
		{
			//*	end of argument, do something with it
			switch(argNum)
			{
				case kGAIA_designation:
													//*	example Gaia ER3 133768513079427328
					//*	take care of double quotes around the designation name
					if (argString[0] == '"')
					{
						strcpy(designation, &argString[1]);
					}
					else
					{
						strcpy(designation, argString);
					}
					designation_sLen	=	strlen(designation);
					if (designation[designation_sLen -1] == '"')
					{
						designation[designation_sLen -1]	=	0;
						designation_sLen--;
					}
					if (designation_sLen >= kLongNameMax)
					{
						CONSOLE_DEBUG_W_STR("Name too long:", designation);
						designation[kLongNameMax - 1]	=	0;
					}
				//	CONSOLE_DEBUG_W_STR("kGAIA_designation:", argString);
					if (strstr(designation, "133768513079427328") != NULL)
					{
						foundOccultation	=	true;
					}
					break;

				case kGAIA_ra:
				//	CONSOLE_DEBUG_W_STR("kGAIA_ra:", argString);
					rightAscen	=	AsciiToDouble(argString);
				//	CONSOLE_DEBUG_W_DBL("rightAscen:", rightAscen);
					break;

				case kGAIA_dec:
				//	CONSOLE_DEBUG_W_STR("kGAIA_dec:", argString);
					declination	=	AsciiToDouble(argString);
				//	CONSOLE_DEBUG_W_DBL("declination:", declination);
					break;

				case kGAIA_phot_g_mean_mag:
					if (isdigit(argString[0]))
					{
						magnitude	=	AsciiToDouble(argString);
					#ifdef _INCLUDE_GAIA_MAIN_
						if (magnitude < gLowestMagnitude)
						{
							gLowestMagnitude	=	magnitude;
						}
						if (magnitude < 12.0)
						{
							gLowMagCnt++;
						}
						magnitudeBinValue		=	magnitude / 5;
						gMagnitudeDistro[magnitudeBinValue]++;
					#endif // _INCLUDE_GAIA_MAIN_
						if (foundOccultation)
						{
//
//							CONSOLE_DEBUG_W_STR("designation    \t=", designation);
//							CONSOLE_DEBUG_W_STR("phot_g_mean_mag\t=", argString);
//							CONSOLE_DEBUG_W_DBL("magnitude      \t=", magnitude);
							magnitude				=	2.0;
							gaiaStar->spectralClass	=	'M';
						}
					}
					break;

				case kGAIA_bp_rp:
				//	CONSOLE_DEBUG_W_STR("kGAIA_bp_rp:", argString);
					if (isdigit(argString[0]))
					{
						bp_rp	=	AsciiToDouble(argString);
						if (bp_rp < gBP_RP_min)
						{
							gBP_RP_min	=	bp_rp;
						}
						if (bp_rp > gBP_RP_max)
						{
							gBP_RP_max	=	bp_rp;
						}
						//*	figure out the spectral class
						gaiaStar->spectralClass	=	CalcSpectralClassFrom_BP_RP(bp_rp);
					}
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
	if (argNum >= kGAIA_last)
	{
		strcpy(gaiaStar->longName, designation);
		gaiaStar->dataSrc		=	kDataSrc_GAIA_gedr3;
		gaiaStar->decl			=	RADIANS(declination);
		gaiaStar->org_decl		=	RADIANS(declination);
		gaiaStar->org_ra		=	RADIANS(rightAscen);
		gaiaStar->ra			=	RADIANS(rightAscen);
		gaiaStar->realMagnitude	=	magnitude;

		validData	=	true;
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}

	return(validData);
}

//**************************************************************************
//*	returns the number of stars in the array
//**************************************************************************
static int	ReadGaiaDataFile(	const char		*filePath,
								TYPE_CelestData	*gaiaDataPtr,
								const int		startIndex,
								const int		maxStarCount,
								const double	magnitudeLimit)
{
FILE				*filePointer;
TYPE_CelestData		currentStar;
long				gaiaStarIdx;
char				lineBuff[2048];
char				currentName[128];
int					lineLength;
int					linesRead;
bool				validData;
int					maxLineLength;
int					recordsAccepted;

//	CONSOLE_DEBUG_W_STR("Reading:", filePath);
//	CONSOLE_DEBUG_W_DBL("magnitudeLimit\t=", magnitudeLimit);

	linesRead		=	0;
	gaiaStarIdx		=	0;
	maxLineLength	=	0;
	recordsAccepted	=	0;
	filePointer		=	fopen(filePath, "r");
	if (filePointer != NULL)
	{
//		CONSOLE_DEBUG("File Open");
	#ifdef _INCLUDE_GAIA_MAIN_
		gFileCount++;
	#endif

		strcpy(currentName, "");

		linesRead		=	0;
		gaiaStarIdx		=	startIndex;
		while (fgets(lineBuff, 2000, filePointer) && (gaiaStarIdx < maxStarCount))
		{
			linesRead++;
			lineLength	=	strlen(lineBuff);
			if (lineLength > maxLineLength)
			{
				maxLineLength	=	lineLength;
			}
			//*	get rid of trailing CR/LF and spaces
			while ((lineBuff[lineLength - 1] <= 0x20) && (lineLength > 0))
			{
				lineBuff[lineLength - 1]	=	0;
				lineLength	=	strlen(lineBuff);
			}
			if (lineLength > 100)
			{
				memset(&currentStar, 0, sizeof(TYPE_CelestData));
				validData	=	ParseOneLineGaiaData(lineBuff, &currentStar);

				if (validData)
				{
					//*	we have a valid star, check to see if its within the limits
					if (currentStar.realMagnitude < magnitudeLimit)
					{
						recordsAccepted++;
					#ifdef _INCLUDE_GAIA_MAIN_
						gMetLimitsCount++;
						//*	output the line
						if (gOutputFilePtr != NULL)
						{
							fprintf(gOutputFilePtr, "%s\n", lineBuff);
						}
					#else
						if (gaiaDataPtr != NULL)
						{
							gaiaDataPtr[gaiaStarIdx]	=	currentStar;
							gaiaStarIdx++;
						}
					#endif // _INCLUDE_GAIA_MAIN_
					}
				}
			}
		}
		fclose(filePointer);
	}
//	CONSOLE_DEBUG_W_NUM("maxLineLength   \t=", maxLineLength);
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open file:", filePath);
	}
//	CONSOLE_DEBUG_W_NUM("linesRead\t=", linesRead);
	CONSOLE_DEBUG_W_NUM("recordsAccepted\t=", recordsAccepted);

	CONSOLE_DEBUG_W_DBL("gBP_RP_min\t=",	gBP_RP_min);
	CONSOLE_DEBUG_W_DBL("gBP_RP_max\t=",	gBP_RP_max);
//	CONSOLE_ABORT(__FUNCTION__);

	return(gaiaStarIdx);
}

#ifndef _INCLUDE_GAIA_MAIN_
//**************************************************************************************
TYPE_CelestData	*ReadGaiaExtractedList(long *gaiaObjCnt)
{
TYPE_CelestData	*gaiaDataPtr;
char			gaiaFilePath[]	=	"Gaia_data/Gaia.csv";
FILE			*filePointer;
int				linesInFile;
long			myGaiaObjCnt;
double			magnitudeLimit;

	CONSOLE_DEBUG(__FUNCTION__);
	gaiaDataPtr		=	NULL;
	filePointer		=	fopen(gaiaFilePath, "r");
	if (filePointer != NULL)
	{
		linesInFile	=	CountLinesInFile(filePointer);
		fclose(filePointer);
		if (linesInFile > 0)
		{
			CONSOLE_DEBUG_W_NUM("linesInFile", linesInFile);
			gaiaDataPtr		=	(TYPE_CelestData *)calloc((linesInFile + 10), sizeof(TYPE_CelestData));
			if (gaiaDataPtr != NULL)
			{
				magnitudeLimit	=	100.0;	//*	we want everything in this file
				myGaiaObjCnt	=	ReadGaiaDataFile(	gaiaFilePath,
														gaiaDataPtr,
														0,
														linesInFile + 2,
														magnitudeLimit);

				CONSOLE_DEBUG_W_LONG("myGaiaObjCnt", myGaiaObjCnt);
				*gaiaObjCnt	=	myGaiaObjCnt;
			}
			else
			{
				CONSOLE_DEBUG("Failed to allocate memory for GAIA array");
			}
		}
		else
		{
			CONSOLE_DEBUG_W_STR("File is empty", gaiaFilePath);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open file:", gaiaFilePath);
	}
	return(gaiaDataPtr);
}
#endif // _INCLUDE_GAIA_MAIN_

//**************************************************************************************
static void	ExtractFileExtension(const char *fileName, char *extension)
{
int		fnLength;
int		ccc;

	fnLength	=	strlen(fileName);
	ccc			=	fnLength;
	while ((fileName[ccc] != '.') && (ccc > 1))
	{
		ccc--;
	}
	strcpy(extension, &fileName[ccc]);
}

//**************************************************************************************
TYPE_CelestData	*ReadGaiaDataDirectory(	const char	*dirName,
										double		magnitudeLimit,
										long		*objectCount)
{
DIR				*directory;
struct dirent	*dir;
int				errorCode;
bool			keepGoing;
char			fileNameExtension[64];
int				filesRead;
TYPE_CelestData	*gaiaDataPtr;
int				gaiaStarCount;
size_t			bufferSize;
char			filePath[256];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, dirName);
	gaiaStarCount	=	0;
	bufferSize		=	kGaiaStarCntMax * sizeof(TYPE_CelestData);
	gaiaDataPtr		=	(TYPE_CelestData *)malloc(bufferSize);
	if (gaiaDataPtr != NULL)
	{
		directory	=	opendir(dirName);
		if (directory != NULL)
		{
			keepGoing		=	true;
			while (keepGoing)
			{
				dir	=	readdir(directory);
				if (dir != NULL)
				{
					if (dir->d_name[0] == '.')
					{
						//*	ignore . and ..
					}
					else if (dir->d_type == DT_DIR)
					{
						//*	ignore directories
					}
					else if (strlen(dir->d_name) < 20)
					{
						//*	ignore directories
						CONSOLE_DEBUG_W_STR("Ignoring", dir->d_name);
					}
					else
					{
						ExtractFileExtension(dir->d_name, fileNameExtension);
						//*	make sure its a .csv file
						if (strcasecmp(fileNameExtension, ".csv") == 0)
						{
							CONSOLE_DEBUG_W_STR("Processing", dir->d_name);
							strcpy(filePath, dirName);
							strcat(filePath, "/");
							strcat(filePath, dir->d_name);
							gaiaStarCount	=	ReadGaiaDataFile(	filePath,
																	gaiaDataPtr,
																	gaiaStarCount,
																	kGaiaStarCntMax,
																	magnitudeLimit);
							filesRead++;

							if (gaiaStarCount >= kGaiaStarCntMax)
							{
								CONSOLE_DEBUG("Exceeded star limit");
								keepGoing		=	false;
							}
						}
						else
						{
	//							CONSOLE_DEBUG_W_STR("Ignoring", dir->d_name);
						}
					}
				}
				else
				{
					keepGoing	=	false;
				}
			}

			errorCode		=	closedir(directory);

			if (errorCode != 0)
			{
				CONSOLE_DEBUG_W_NUM("closedir errorCode\t=", errorCode);
				CONSOLE_DEBUG_W_NUM("errno\t=", errno);
			}
		}
		else
		{
			CONSOLE_DEBUG_W_STR("Failed to open", dirName);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}
		*objectCount	=	gaiaStarCount;
	}
	else
	{
		CONSOLE_DEBUG("Failed to allocate memory");
	}
	return(gaiaDataPtr);
}

#ifdef _INCLUDE_GAIA_MAIN_
//*****************************************************************************
int main(int argc, char *argv[])
{
int				iii;
int				filesRead;
TYPE_CelestData	*gaiaDataPtr;
long			gaiaStarCount;
size_t			bufferSize;
double			magnitudeLimit;
char			gaia_directory[]	=	"Gaia_data";
char			filePath[64];

	CONSOLE_DEBUG_W_NUM("kGAIA_last\t=", kGAIA_last);
	for (iii=0; iii<10; iii++)
	{
		gMagnitudeDistro[iii]	=	0;
	}

	strcpy(filePath, gaia_directory);
	strcat(filePath, "/");
	strcat(filePath, "Gaia.csv");
	gOutputFilePtr		=	fopen(filePath, "w");
	if (gOutputFilePtr != NULL)
	{
		magnitudeLimit	=	12.0;
		gaiaDataPtr		=	ReadGaiaDataDirectory(gaia_directory, magnitudeLimit, &gaiaStarCount);

		CONSOLE_DEBUG_W_NUM("Files processed \t=", gFileCount);
		CONSOLE_DEBUG_W_NUM("Records accepted\t=", gMetLimitsCount);
		CONSOLE_DEBUG_W_DBL("gLowestMagnitude\t=", gLowestMagnitude);
		CONSOLE_DEBUG_W_NUM("gLowMagCnt      \t=", gLowMagCnt);
		CONSOLE_DEBUG_W_LONG("gaiaStarCount   \t=", gaiaStarCount);

		for (iii=0; iii<10; iii++)
		{
			printf("Magnitude %2d-%2d has %7d entries\r\n", (iii * 5), ((iii+1) * 5),
								gMagnitudeDistro[iii]);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open ", filePath);
	}

	return(0);
}

#endif


#endif // _ENABLE_GAIA_
