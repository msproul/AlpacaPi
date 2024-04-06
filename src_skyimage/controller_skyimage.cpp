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


bool	gKeepRunning			=	true;
bool	gVerbose				=	false;
char	gFullVersionString[128]	=	"V0.0.0";




TYPE_ImageFile	gImageList[kMaxImageCnt];
char			gDirectoryPath[256]	=	"";
int				gImageCount			=	0;
int				gCurrentImageIndex	=	0;

static void	ReadFitsInfoForImage(TYPE_ImageFile *imageFileData);
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
//	CONSOLE_DEBUG(__FUNCTION__);
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

//**************************************************************************************
void	ControllerSkyImage::RunBackgroundTasks(const char *callingFunction, bool enableDebug)
{
unsigned int	startMilliSecs;
unsigned int	deltaMilliSecs;

	if (cBackGroundImgIdx < gImageCount)
	{
		startMilliSecs	=	millis();
		deltaMilliSecs	=	0;
		while ((deltaMilliSecs < 50) && (cBackGroundImgIdx < gImageCount))
		{
	//		CONSOLE_DEBUG_W_NUM("cBackGroundImgIdx\t=", cBackGroundImgIdx);
	//		CONSOLE_DEBUG_W_NUM("gImageCount      \t=", gImageCount);
			if (gImageList[cBackGroundImgIdx].FitsProcessed == false)
			{
				if (gImageList[cBackGroundImgIdx].ImageFileType == kImageFileType_FITS)
				{
					ReadFitsInfoForImage(&gImageList[cBackGroundImgIdx]);
				}
				else
				{
	//				CONSOLE_DEBUG_W_STR("File is PDS\t=", gImageList[cBackGroundImgIdx].FileName);
					gImageList[cBackGroundImgIdx].FitsProcessed	=	true;
				}
				cFitsProcessCntr++;
			}
			cBackGroundImgIdx++;

			//*	update the window but not every time
			if ((cBackGroundImgIdx % 10) == 0)
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
		//*	if the sort order gets changed while we are reading the fits info,
		//*	this will make sure everything gets read properly
		cBackGroundImgIdx	=	0;
		cFitsProcessCntr	=	0;
	}

	//---------------------------------------------
	//*	Moon Phase window
	if (cMoonPhaseTabObjPtr != NULL)
	{
		cMoonPhaseTabObjPtr->RunWindowBackgroundTasks();
		cUpdateWindow	=	true;
	}
}

//*****************************************************************************
int	ControllerSkyImage::BuildFileList(const char *directoryPath)
{
DIR				*directory;
struct dirent	*dir;
bool			keepGoing;
char			curFileName[512];
char			fileExtension[16];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, directoryPath);
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
						CONSOLE_DEBUG_W_STR(dir->d_name, "Is a directory");
						BuildFileList(subDirectoryName);
					}
				}

				if ((strcasecmp(fileExtension, ".fits") == 0) ||
					(strcasecmp(fileExtension, ".fit") == 0) ||
					(strcasecmp(fileExtension, ".img") == 0) ||
					(strcasecmp(fileExtension, ".imq") == 0))
				{
					if (cFileIndex < kMaxImageCnt)
					{
						gImageList[cFileIndex].validEntry	=	true;
						strcpy(gImageList[cFileIndex].DirectoryPath,	directoryPath);
						strcpy(gImageList[cFileIndex].FileName,		curFileName);
						strcpy(gImageList[cFileIndex].FilePath,		directoryPath);
						strcat(gImageList[cFileIndex].FilePath,		curFileName);


						//*	check file extension to determine file type
						SetImageFileType(&gImageList[cFileIndex],fileExtension);

						cFileIndex++;
					}
					else
					{
						CONSOLE_DEBUG_W_NUM("Ran out of room, cnt\t=",	cFileIndex);
						return(cFileIndex);
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
		CONSOLE_DEBUG_W_STR("Failed to open directory\t=",	directoryPath);
	}
//	CONSOLE_ABORT(__FUNCTION__);
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

//	CONSOLE_DEBUG(__FUNCTION__);
	if (imageController != NULL)
	{
		gCurrentImageIndex++;
		if ((gCurrentImageIndex >= 0) && (gCurrentImageIndex < gImageCount))
		{
			successFlag	=	imageController->LoadImage(gImageList[gCurrentImageIndex].FilePath);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("gCurrentImageIndex out of range", gCurrentImageIndex);
			gCurrentImageIndex	=	gImageCount - 1;
			successFlag	=	false;
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

//	CONSOLE_DEBUG(__FUNCTION__);

	iii	=	1;
	while (iii<argc)
	{
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
	while ((*dataPtr == 0x20) && (*dataPtr > 0x20))
	{
		dataPtr++;
	}
	strcpy(valueString, dataPtr);
}

//*****************************************************************************
static void	ReadFitsInfoForImage(TYPE_ImageFile *imageFileData)
{
fitsfile	*fptr;
char		card[FLEN_CARD];
char		valueString[FLEN_CARD];
int			status;
int			nkeys;
int			iii;

//	CONSOLE_DEBUG(__FUNCTION__);
	status	=	0;	///* MUST initialize status
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
int main(int argc, char *argv[])
{
int					iii;
int					objectsCreated;
int					activeObjCnt;
int					keyPressed;
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
	if (strlen(myDirectoryPath) > 0)
	{
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

			#if (CV_MAJOR_VERSION >= 3)
				keyPressed	=	cv::waitKeyEx(5);
			#else
				keyPressed	=	cv::waitKey(5);
			#endif
				if (keyPressed > 0)
				{
					Controller_HandleKeyDown(keyPressed);
				}

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

