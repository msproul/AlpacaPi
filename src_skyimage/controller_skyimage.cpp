//*****************************************************************************
//*		controller_skyimage.cpp		(c) 2024 by Mark Sproul
//*
//*
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Mar  9,	2024	<MLS> Created controller_skyimage.cpp
//*	Mar 13,	2024	<MLS> Double click now opens image window
//*	Mar 18,	2024	<MLS> Added LoadNextImageFromList() & LoadPreviousImageFromList()
//*	Mar 24,	2024	<MLS> Added recursive directory reading
//*	Mar 24,	2024	<MLS> Fixed crash bug when image failed to load
//*	Mar 27,	2024	<MLS> Added NASA MoonPhase window ti SkyImage
//*	May  5,	2024	<MLS> Fixed bug in running background tasks
//*	May  7,	2024	<MLS> Modified file name parsing to display partial path
//*****************************************************************************



#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<dirent.h>
#include	<errno.h>
#include	<sys/stat.h>
#include	<stdio.h>
#include	<unistd.h>


#include	<fitsio.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#define	kWindowWidth	1300
//#define	kWindowHeight	820
#define	kWindowHeight	800

#include	"helper_functions.h"
#include	"NASA_moonphase.h"

#include	"windowtab_about.h"
#include	"windowtab_imageList.h"
#include	"controller.h"
#include	"controller_skyimage.h"
#include	"windowtab_MoonPhase.h"
#include	"controller_image.h"
#include	"imagelist.h"
#include	"linuxerrors.h"

#ifdef _ENABLE_NASA_PDS_
	#include	"PDS.typedefs.h"
	#include	"PDS_ReadNASAfiles.h"
#endif

bool	gKeepRunning			=	true;
bool	gVerbose				=	false;
char	gFullVersionString[128]	=	"V0.0.0";




TYPE_ImageFile	gImageList[kMaxImageCnt];
char			gDirectoryPath[256]	=	"";
int				gImageCount			=	0;
int				gCurrentImageIndex	=	0;

static void	ReadImageHeader_FITS(TYPE_ImageFile *imageFileData);
static void	ReadImageHeader_PDS(TYPE_ImageFile *imageFileData);

static void	ExtractFileExtension(const char *fileName, char *extension);
static void	SetImageFileType(TYPE_ImageFile *imageFileInfo, const char *fileExtension);
static int	FileNameQSort(const void *e1, const void* e2);

//**************************************************************************************
ControllerSkyImage::ControllerSkyImage(	const char *argWindowName, const char *argDirectoryPath)
				:Controller(	argWindowName,
								kWindowWidth,
								kWindowHeight,
								false)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cWindowType				=	'IMAG';
	cImageListTabObjPtr		=	NULL;
	cAboutBoxTabObjPtr		=	NULL;
	cFitsProcessCntr		=	0;
	cBackGroundImgIdx		=	0;

	strcpy(cDirectoryPath, argDirectoryPath);
	cDirectoryPathLen	=	strlen(cDirectoryPath);
	ReadFileDirectory(cDirectoryPath);

	SetupWindowControls();

#ifdef _USE_BACKGROUND_THREAD_
	StartBackgroundThread();
#endif // _USE_BACKGROUND_THREAD_
}

//**************************************************************************************
// Destructor
//**************************************************************************************
ControllerSkyImage::~ControllerSkyImage(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);


	DELETE_OBJ_IF_VALID(cImageListTabObjPtr);
	DELETE_OBJ_IF_VALID(cMoonPhaseTabObjPtr);
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
}

//**************************************************************************************
bool	ControllerSkyImage::RunFastBackgroundTasks(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_ABORT(__FUNCTION__);
	//---------------------------------------------
	//*	Moon Phase window
	if (cMoonPhaseTabObjPtr != NULL)
	{
		cMoonPhaseTabObjPtr->RunWindowBackgroundTasks();
		cUpdateWindow	=	true;
	}
	else
	{
		CONSOLE_ABORT(__FUNCTION__);
	}
	return(true);
}

//**************************************************************************************
void	ControllerSkyImage::SetupWindowControls(void)
{

//	CONSOLE_DEBUG(__FUNCTION__);

	SetTabCount(kTab_SI_Count);

	//=============================================================
	SetTabText(kTab_SI_ImgList,		"Image List");
	cImageListTabObjPtr		=	new WindowTabImageList(	cWidth, cHeight, cBackGrndColor, "Image List");
	if (cImageListTabObjPtr != NULL)
	{
		SetTabWindow(kTab_SI_ImgList,	cImageListTabObjPtr);
		cImageListTabObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	SetTabText(kTab_MoonPhase,	"Moon Phase");
	cMoonPhaseTabObjPtr		=	new WindowTabMoonPhase(	cWidth, cHeight, cBackGrndColor, "Moon Phase");
	if (cMoonPhaseTabObjPtr != NULL)
	{
		SetTabWindow(kTab_MoonPhase,	cMoonPhaseTabObjPtr);
		cMoonPhaseTabObjPtr->SetParentObjectPtr(this);
	}

	//=============================================================
	SetTabText(kTab_SI_About,		"About");
	cAboutBoxTabObjPtr		=	new WindowTabAbout(	cWidth, cHeight, cBackGrndColor, "About");
	if (cAboutBoxTabObjPtr != NULL)
	{
		SetTabWindow(kTab_SI_About,	cAboutBoxTabObjPtr);
		cAboutBoxTabObjPtr->SetParentObjectPtr(this);
		cAboutBoxTabObjPtr->SetLocalDeviceInfo();
	}
}

static bool	gBackGroundTaskActive	=	false;

//**************************************************************************************
void	ControllerSkyImage::RunBackgroundTasks(const char *callingFunction, bool enableDebug)
{
unsigned int	startMilliSecs;
unsigned int	deltaMilliSecs;

//	CONSOLE_DEBUG(__FUNCTION__);

	if (gBackGroundTaskActive)
	{
		CONSOLE_DEBUG_W_STR("Called from", callingFunction);
		CONSOLE_ABORT(__FUNCTION__);
	}
	gBackGroundTaskActive	=	true;

	if (cBackGroundImgIdx < gImageCount)
	{
		startMilliSecs	=	millis();
		deltaMilliSecs	=	0;
		while ((deltaMilliSecs < 100) && (cBackGroundImgIdx < gImageCount))
		{
//			CONSOLE_DEBUG_W_NUM("cBackGroundImgIdx\t=", cBackGroundImgIdx);
//			CONSOLE_DEBUG_W_NUM("gImageCount      \t=", gImageCount);
			if (gImageList[cBackGroundImgIdx].FitsProcessed == false)
			{
				if (gImageList[cBackGroundImgIdx].ImageFileType == kImageFileType_FITS)
				{
					ReadImageHeader_FITS(&gImageList[cBackGroundImgIdx]);
				}
				else
				{
//					CONSOLE_DEBUG_W_NUM("cBackGroundImgIdx\t=", cBackGroundImgIdx);
//					CONSOLE_DEBUG_W_STR("File is PDS      \t=", gImageList[cBackGroundImgIdx].FileName);
//					CONSOLE_DEBUG_W_STR("File is PDS      \t=", gImageList[cBackGroundImgIdx].FilePath);
					ReadImageHeader_PDS(&gImageList[cBackGroundImgIdx]);
					gImageList[cBackGroundImgIdx].FitsProcessed	=	true;
				}
				cFitsProcessCntr++;
			}
			cBackGroundImgIdx++;

			if ((cBackGroundImgIdx % 50) == 0)
			{
				SetWidgetNumber(kTab_SI_ImgList, kImageList_Btn_Scan, cBackGroundImgIdx);
			}
			//*	update the window but not every time
			if ((cBackGroundImgIdx % 25) == 0)
			{
				if (cImageListTabObjPtr != NULL)
				{
					cImageListTabObjPtr->UpdateOnScreenWidgetList();
					HandleWindowUpdate();
				}
			}
			deltaMilliSecs	=	millis() - startMilliSecs;
		}
	}
	else if (cFitsProcessCntr > 0)
	{
	int		unProcessedCount;
	int		iii;
		//*	go through the entire list and see how many have not been processed
		unProcessedCount	=	0;
		for (iii=0; iii<gImageCount; iii++)
		{
			if (gImageList[iii].FitsProcessed == false)
			{
				unProcessedCount++;
			}
		}
		CONSOLE_DEBUG_W_NUM("unProcessedCount\t=",	unProcessedCount);
		//*	if the sort order gets changed while we are reading the fits info,
		//*	this will make sure everything gets read properly
		if (unProcessedCount > 0)
		{
			CONSOLE_DEBUG("Reseting cBackGroundImgIdx to 0")
			cBackGroundImgIdx	=	0;
		}
		cFitsProcessCntr	=	0;
	}
	else
	{
		SetWidgetText(kTab_SI_ImgList, kImageList_Btn_Scan, "Scan");
	}

	//---------------------------------------------
	//*	Moon Phase window
	if (cMoonPhaseTabObjPtr != NULL)
	{
		cMoonPhaseTabObjPtr->RunWindowBackgroundTasks();
		cUpdateWindow	=	true;
	}
	gBackGroundTaskActive	=	false;
}

static int	gRecursionLevel	=	0;

//*****************************************************************************
int	ControllerSkyImage::BuildFileList(const char *directoryPath)
{
DIR				*directory;
struct dirent	*dir;
bool			keepGoing;
char			curFileName[512];
char			fileExtension[16];

	gRecursionLevel++;
	CONSOLE_DEBUG_W_STR(__FUNCTION__, directoryPath);
	CONSOLE_DEBUG_W_NUM("gRecursionLevel\t=", gRecursionLevel);
	directory	=	opendir(directoryPath);
	if (directory != NULL)
	{
		keepGoing	=	true;
		while (keepGoing)
		{
//			CONSOLE_DEBUG("----------------------------------");
			dir	=	readdir(directory);
			if (dir != NULL)
			{
				strcpy(curFileName, dir->d_name);
				ExtractFileExtension(curFileName, fileExtension);

//				CONSOLE_DEBUG_W_STR("curFileName\t=", 	curFileName);
//				CONSOLE_DEBUG_W_HEX("dir->d_type\t=", 	dir->d_type);
				if (dir->d_type == DT_DIR)
				{
					if (dir->d_name[0] != '.')
					{
					char	subDirectoryName[256];
						strcpy(subDirectoryName, directoryPath);
						strcat(subDirectoryName, dir->d_name);
						strcat(subDirectoryName, "/");
						BuildFileList(subDirectoryName);
					}
				}

				if ((strcasecmp(fileExtension, ".fits") == 0) ||
					(strcasecmp(fileExtension, ".fit") == 0) ||
//					(strcasecmp(fileExtension, ".ibg") == 0) ||
					(strcasecmp(fileExtension, ".img") == 0) ||
					(strcasecmp(fileExtension, ".imq") == 0))
				{
					if (cFileIndex < kMaxImageCnt)
					{
						gImageList[cFileIndex].validEntry	=	true;
						strcpy(gImageList[cFileIndex].DirectoryPath,	directoryPath);
						strcpy(gImageList[cFileIndex].FilePath,		directoryPath);
						strcat(gImageList[cFileIndex].FilePath,		curFileName);

						//*	we want to display the part of the path that was NOT specified on the command line
						strcpy(gImageList[cFileIndex].FileName,		&gImageList[cFileIndex].FilePath[cDirectoryPathLen]);
//						strcpy(gImageList[cFileIndex].FileName,		curFileName);


						//*	check file extension to determine file type
						SetImageFileType(&gImageList[cFileIndex], fileExtension);

						cFileIndex++;
					}
					else
					{
						CONSOLE_DEBUG_W_NUM("Ran out of room, cnt\t=",	cFileIndex);
						keepGoing	=	false;
					}
				}
			}
			else
			{
				keepGoing	=	false;
			}
		}
		//*	if there are more than one, sort them so there is consistency.
		//*	accessing the file path does not always guarantee the same order
		if  (cFileIndex > 1)
		{
			qsort(gImageList, cFileIndex, sizeof(TYPE_ImageFile), FileNameQSort);
		}
		closedir(directory);
	}
	else
	{
	char	errorString[256];

		CONSOLE_DEBUG_W_STR("Failed to open directory\t=",	directoryPath);
		CONSOLE_DEBUG_W_NUM("errno\t=",	errno);
		GetLinuxErrorString(errno, errorString);
		CONSOLE_DEBUG_W_STR("errno\t=",	errorString);
	}
//	CONSOLE_ABORT(__FUNCTION__);
	gRecursionLevel--;
	return(cFileIndex);
}


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

//*****************************************************************************
static int	FileNameQSort(const void *e1, const void* e2)
{
int				retValue;
TYPE_ImageFile	*entry1;
TYPE_ImageFile	*entry2;

	entry1		=	(TYPE_ImageFile *)e1;
	entry2		=	(TYPE_ImageFile *)e2;
	retValue	=	strcmp(entry1->FileName, entry2->FileName);
//	if (retValue == 0)
//	{
//		CONSOLE_DEBUG_W_2STR("duplicate:", entry1->FileName, entry2->FileName);
//		CONSOLE_ABORT(__FUNCTION__);
//	}
	return(retValue);
}

//*****************************************************************************
static void	SetImageFileType(TYPE_ImageFile *imageFileInfo, const char *fileExtension)
{
	if ((strcasecmp(fileExtension, ".fits") == 0) ||
		(strcasecmp(fileExtension, ".fit") == 0))
	{
//		CONSOLE_DEBUG("kImageFileType_FITS");
		imageFileInfo->ImageFileType	=	kImageFileType_FITS;
	}
	else if ((strcasecmp(fileExtension, ".IMG") == 0) ||
			(strcasecmp(fileExtension, ".IMQ") == 0))
	{
//		CONSOLE_DEBUG("kImageFileType_PDS");
		imageFileInfo->ImageFileType	=	kImageFileType_PDS;
	}
//	CONSOLE_DEBUG_W_NUM("ImageFileType\t=", imageFileInfo->ImageFileType);

}


//*****************************************************************************
void	ControllerSkyImage::ReadFileDirectory(const char *directoryPath)
{

	CONSOLE_DEBUG_W_STR(__FUNCTION__, directoryPath);

	cFileIndex	=	0;
	gImageCount	=	BuildFileList(directoryPath);
	CONSOLE_DEBUG_W_NUM("gImageCount\t=", gImageCount);
//	CONSOLE_ABORT(__FUNCTION__);
}


//*****************************************************************************
bool	LoadNextImageFromList(ControllerImage *imageController)
{
bool	successFlag	=	false;

	CONSOLE_DEBUG(__FUNCTION__);
	if (imageController != NULL)
	{
		gCurrentImageIndex++;
		if ((gCurrentImageIndex >= 0) && (gCurrentImageIndex < gImageCount))
		{
			CONSOLE_DEBUG_W_STR("Next FilePath:", gImageList[gCurrentImageIndex].FilePath);
			successFlag	=	imageController->LoadImage(gImageList[gCurrentImageIndex].FilePath);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("gCurrentImageIndex out of range", gCurrentImageIndex);
			gCurrentImageIndex	=	gImageCount - 1;
			successFlag			=	false;
		}
	}
//	CONSOLE_DEBUG_W_BOOL(__FUNCTION__, successFlag);
	return(successFlag);
}

//*****************************************************************************
bool	LoadPreviousImageFromList(ControllerImage *imageController)
{
bool	successFlag	=	false;

	if (imageController != NULL)
	{
		gCurrentImageIndex--;
		if (gCurrentImageIndex < 0)
		{
			gCurrentImageIndex	=	0;
			successFlag			=	false;
		}
		if ((gCurrentImageIndex >= 0) && (gCurrentImageIndex < gImageCount))
		{
			successFlag	=	imageController->LoadImage(gImageList[gCurrentImageIndex].FilePath);
		}
	}
//	CONSOLE_DEBUG_W_BOOL(__FUNCTION__, successFlag);
	return(successFlag);
}


//*****************************************************************************
static void	ProcessCmdLineArgs(int argc, char **argv)
{
int		iii;
char	theChar;

	CONSOLE_DEBUG(__FUNCTION__);

	iii	=	1;
	while (iii<argc)
	{
		CONSOLE_DEBUG(argv[iii]);
		if (argv[iii][0] == '-')
		{
			theChar	=	argv[iii][1];
			switch(theChar)
			{
				case 'c':
					break;

				case 'i':
					break;

				case 'v':
					gVerbose	=	true;
					break;

			}
		}
		iii++;
	}
}

//*****************************************************************************
static void	GetDataFromFitsLine(char *cardData, char *valueString)
{
char	*dataPtr;

//DATAMAX =                  236 / Maximum pixel value
	dataPtr	=	cardData;
	dataPtr	+=	9;
//	while ((*dataPtr == 0x20) && (*dataPtr > 0x20))
	while (*dataPtr == 0x20)
	{
		dataPtr++;
	}
	strcpy(valueString, dataPtr);
}

//*****************************************************************************
static void	ReadImageHeader_FITS(TYPE_ImageFile *imageFileData)
{
fitsfile	*fptr;
char		card[FLEN_CARD];
char		valueString[FLEN_CARD];
int			status;
int			nkeys;
int			iii;
int			jjj;
int			ccc;
int			sLen;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG(imageFileData->DirectoryPath);
//	CONSOLE_DEBUG(imageFileData->FileName);
//	CONSOLE_DEBUG(imageFileData->FilePath);
	status	=	0;			//* MUST initialize status
	fits_open_file(&fptr, imageFileData->FilePath, READONLY, &status);
	if (status == 0)
	{
		fits_get_hdrspace(fptr, &nkeys, NULL, &status);

		for (iii = 1; iii <= nkeys; iii++)
		{
			status		=	0;
			fits_read_record(fptr, iii, card, &status);	//* read keyword
			card[80]	=	0;
			if (strncmp(card, "DATAMAX", 7) == 0)
			{
				GetDataFromFitsLine(card, valueString);
				imageFileData->DATAMAX	=	atoi(valueString);
			}
			else if (strncmp(card, "DATAMIN", 7) == 0)
			{
				GetDataFromFitsLine(card, valueString);
				imageFileData->DATAMIN	=	atoi(valueString);
			}
			else if (strncmp(card, "EXPTIME", 7) == 0)
			{
				GetDataFromFitsLine(card, valueString);
				imageFileData->Exposure_secs	=	atof(valueString);
			}
			else if (strncmp(card, "SATUPRCT", 8) == 0)
			{
				GetDataFromFitsLine(card, valueString);
				imageFileData->SaturationPercent	=	atof(valueString);
			}
			else if (strncmp(card, "GAIN", 4) == 0)
			{
				GetDataFromFitsLine(card, valueString);
				imageFileData->Gain	=	atoi(valueString);
			}
			else if (strncmp(card, "OBJECT", 4) == 0)
			{
				GetDataFromFitsLine(card, valueString);
				jjj		=	0;
				ccc		=	0;
				sLen	=	strlen(valueString);
				if (valueString[jjj] == '\'')
				{
					jjj++;
				}
//				CONSOLE_DEBUG_W_NUM("jjj        \t=", jjj);
				while ((valueString[jjj] >= 0x20) && (valueString[jjj] != '\'') && (jjj < sLen))
				{
					imageFileData->Object[ccc]	=	valueString[jjj];
					ccc++;
					jjj++;
				}
				imageFileData->Object[ccc]	=	0;
			}
		}
	}
	else
	{
		fits_report_error(stderr, status);
		CONSOLE_DEBUG_W_STR("Failed to open", imageFileData->FileName);
	}
	status	=	0;	///* MUST initialize status
	fits_close_file(fptr, &status);

	imageFileData->FitsProcessed	=	true;
	if (status)				//* print any error messages
	{
		fits_report_error(stderr, status);
	}
}

//*****************************************************************************
static void	ExtractKeywordValue(char *line, char *keyword, char *valueString)
{
int		iii;
int		sLen;
char	*valuePtr;
char	*quotePtr;

	keyword[0]		=	0;
	valueString[0]	=	0;
	sLen			=	strlen(line);
	iii				=	0;
	while ((line[iii] > 0x20) && (line[iii] != '=') && (iii < sLen))
	{
		keyword[iii]	=	line[iii];
		iii++;
	}
	keyword[iii]	=	0;

	valuePtr	=	strchr(line, '=');
	if (valuePtr != NULL)
	{
		valuePtr++;
		while (*valuePtr == 0x20)
		{
			valuePtr++;
		}
		if (*valuePtr == '\'')
		{
			valuePtr++;
		}

		strcpy(valueString, valuePtr);
		quotePtr	=	strchr(valueString, '\'');
		if (quotePtr != NULL)
		{
			*quotePtr	=	0;
		}
	}
}

//*****************************************************************************
static void	ReadImageHeader_PDS(TYPE_ImageFile *imageFileData)
{
PDS_header_data	pdsHeader;
bool			readOK;
int				iii;
char			keyword[64];
char			valueString[128];

	CONSOLE_DEBUG(__FUNCTION__);

//	memset(&pdsHeader, 0, sizeof(PDS_header_data));
	readOK	=	PDS_ReadImage(imageFileData->FilePath, &pdsHeader, false);
	if (readOK)
	{
//		CONSOLE_DEBUG_W_NUM("pdsHeader.HeaderLineCnt\t=", pdsHeader.HeaderLineCnt);
		for (iii=0; iii < pdsHeader.HeaderLineCnt; iii++)
		{
//			printf("%s\r\n", pdsHeader.HeaderData[iii].headerLine);
			ExtractKeywordValue(pdsHeader.HeaderData[iii].headerLine, keyword, valueString);

			if (strcasecmp(keyword, "TARGET_NAME") == 0)
			{
				strcpy(imageFileData->Object, valueString);
			}
			else if (strcasecmp(keyword, "TARGET_BODY") == 0)
			{
				if (strlen(imageFileData->Object) < 2)
				{
					strcpy(imageFileData->Object, valueString);
				}
			}
//			else if (strcasecmp(keyword, "FILTER_NAME") == 0)
//			{
//				strcpy(imageFileData->Filter, valueString);
//			}
//			else if (strcasecmp(keyword, "INSTRUMENT_NAME") == 0)
//			{
//				strcpy(imageFileData->Camera, valueString);
//			}
//			else if (strcasecmp(keyword, "EXPOSURE_DURATION") == 0)
//			{
//				imageFileData->Exposure_Secs	=	atof(valueString);
//				spacePtr	=	strchr(valueString, 0x20);
//				if (spacePtr != NULL)
//				{
//					*spacePtr	=	0;
//				}
//			}
//			else if (strcasecmp(keyword, "IMAGE_TIME") == 0)
//			{
//				strcpy(imageFileData->Time_UTC, valueString);
//			}
//			else if (strcasecmp(keyword, "SPACECRAFT_NAME") == 0)
//			{
//				strcpy(imageFileData->Observatory, valueString);
//			}
			//--------------------------------------------------------------
			//*	these are from GALILEO
//			else if (strcasecmp(keyword, "MISSION") == 0)
//			{
//				strcpy(imageFileData->Observatory, valueString);
//			}
			else if (strcasecmp(keyword, "TARGET") == 0)
			{
				strcpy(imageFileData->Object, valueString);
			}
//			else if (strcasecmp(keyword, "DAT_TIM") == 0)
//			{
//				//*	if there is already a value, do not over-ride
//				if (strlen(imageFileData->Time_UTC) < 10)
//				{
//					strcpy(imageFileData->Time_UTC, valueString);
//				}
//			}
//			else if (strcasecmp(keyword, "EXP") == 0)
//			{
//				exposure_MilliSecs				=	atof(valueString);
//				imageFileData->Exposure_Secs	=	exposure_MilliSecs / 1000.0;
//				sprintf(statusString, "%2.5f", imageFileData->Exposure_Secs);
//			}
//			else if (strcasecmp(keyword, "SENSOR") == 0)
//			{
//				//	SENSOR='SSI'
//				strcpy(imageFileData->Camera, valueString);
//			}
//			else if (strcasecmp(keyword, "FILTER") == 0)
//			{
//				//FILTER=3(VIO)             Filter position: 0(CLR), 1(GRN), 2(RED),
//				//                            3(VLT), 4(756), 5(968), 6(727), 7(889)
//				if (isdigit(valueString[0]))
//				{
//				int	filterNumber;
//					filterNumber	=	atoi(valueString);
//					switch(filterNumber)
//					{
//						case 0:	strcpy(valueString,	"0-CLR");	break;
//						case 1:	strcpy(valueString,	"1-GRN");	break;
//						case 2:	strcpy(valueString,	"2-RED");	break;
//						case 3:	strcpy(valueString,	"3-VLT");	break;
//						case 4:	strcpy(valueString,	"4-756");	break;
//						case 5:	strcpy(valueString,	"5-968");	break;
//						case 6:	strcpy(valueString,	"6-727");	break;
//						case 7:	strcpy(valueString,	"7-889");	break;
//					}
//				}
//				strcpy(imageFileData->Filter, valueString);
//			}
		}
	}
//	if (pdsHeader.imageData != NULL)
//	{
//		free(pdsHeader.imageData);
//		pdsHeader.imageData	=	NULL;
//	}

}

//*****************************************************************************
int main(int argc, char *argv[])
{
int					iii;
int					objectsCreated;
int					activeObjCnt;
int					keyPressed;
int					sLen;
unsigned int		currentMillis;
unsigned int		lastDebugMillis;
unsigned int		deltaSecs;
//unsigned long		startNanoSecs;
//unsigned long		endNanoSecs;
//unsigned long		deltaNanoSecs;
ControllerSkyImage	*skyImageCtrlObj;
char				myDirectoryPath[256];

	CONSOLE_DEBUG(__FUNCTION__);
	sprintf(gFullVersionString,		"%s - %s build #%d", kApplicationName, kVersionString, kBuildNumber);
	strcpy(myDirectoryPath, "");

	objectsCreated		=	0;
	lastDebugMillis		=	millis();

	NASA_ReadMoonPhaseDirectory();
	NASA_ReadMoonPhaseData();

	//*	deal with any options from the command line
	ProcessCmdLineArgs(argc, argv);

	//*	find the directory specified
	iii	=	1;
	while (iii<argc)
	{
		if (argv[iii][0] != '-')
		{
			//*	this should be a directory
			strcpy(myDirectoryPath, argv[iii]);
		}
		iii++;
	}

	strcpy(gDirectoryPath, myDirectoryPath);


//	CONSOLE_DEBUG_W_STR(__FUNCTION__, myDirectoryPath);
	sLen	=	strlen(myDirectoryPath);
	if (sLen > 0)
	{
		if (myDirectoryPath[sLen -1] != '/')
		{
			strcat(myDirectoryPath, "/");
		}
		skyImageCtrlObj	=	new ControllerSkyImage("SkyImage", myDirectoryPath);
		objectsCreated++;
	}
	else
	{
		exit(0);
	}

	activeObjCnt	=	objectsCreated;


	skyImageCtrlObj->ShowWindow();
	while (gKeepRunning && (activeObjCnt > 0))
	{
		activeObjCnt	=	0;
		for (iii=0; iii<kMaxControllers; iii++)
		{
			if (gControllerList[iii] != NULL)
			{
				activeObjCnt++;
				gControllerList[iii]->HandleWindow();
//				gControllerList[iii]->RunBackgroundTasks(__FUNCTION__, false);

			#if (CV_MAJOR_VERSION >= 3)
				keyPressed	=	cv::waitKeyEx(5);
			#else
				keyPressed	=	cv::waitKey(5);
			#endif
				if (keyPressed > 0)
				{
					Controller_HandleKeyDown(keyPressed);
				}

				//--------------------------------------------------------------------
				//*	check for closed window (controller)
				if (gControllerList[iii]->cKeepRunning == false)
				{
				//	CONSOLE_DEBUG_W_NUM("Deleting controller #", iii);
				//	CONSOLE_DEBUG_W_STR("Deleting window", gControllerList[iii]->cWindowName);
					delete gControllerList[iii];
					if (gControllerList[iii] != NULL)
					{
						CONSOLE_DEBUG_W_STR("Delete had a problem", gControllerList[iii]->cWindowName);
					}
				}
			}
		}
		if (gVerbose)
		{
			currentMillis	=	millis();
			deltaSecs		=	(currentMillis - lastDebugMillis) / 1000;
			if (deltaSecs > 15)
			{
				DumpControllerBackGroundTaskStatus(__FUNCTION__);
				lastDebugMillis	=	millis();
			}
		}
	}
//	CONSOLE_DEBUG("Closing all windows");
	for (iii=0; iii<kMaxControllers; iii++)
	{
		if (gControllerList[iii] != NULL)
		{
//			CONSOLE_DEBUG_W_STR("Deleting window", gControllerList[iii]->cWindowName);
			delete gControllerList[iii];
			cv::waitKey(10);
		//	sleep(2);
		}
	}
	CONSOLE_DEBUG("Clean exit");
}

