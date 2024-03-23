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


#define	kWindowWidth	1200
//#define	kWindowHeight	820
#define	kWindowHeight	800

#include	"helper_functions.h"
#include	"windowtab_about.h"
#include	"windowtab_imageList.h"


#include	"controller.h"
#include	"controller_skyimage.h"
#include	"controller_image.h"
#include	"imagelist.h"


bool	gKeepRunning			=	true;
bool	gVerbose				=	false;
char	gFullVersionString[128]	=	"V0.0.0";


char			gDirectoryPath[256];
TYPE_ImageFile	*gImageList			=	NULL;
int				gImageCount			=	0;
int				gCurrentImageIndex	=	0;
int				gBackGroundImgIdx	=	0;

void	ReadFitsInfoForImage(TYPE_ImageFile *imageFileData);

//**************************************************************************************
ControllerSkyImage::ControllerSkyImage(	const char *argWindowName)
				:Controller(	argWindowName,
								kWindowWidth,
								kWindowHeight,
								false)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	cWindowType						=	'IMAG';
	cImageListTabObjPtr				=	NULL;
	cAboutBoxTabObjPtr				=	NULL;
	cFitsProcessCntr				=	0;

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
	DELETE_OBJ_IF_VALID(cAboutBoxTabObjPtr);
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
	if (gImageList != NULL)
	{
		if (gBackGroundImgIdx < gImageCount)
		{
//			CONSOLE_DEBUG_W_NUM("gBackGroundImgIdx\t=", gBackGroundImgIdx);
//			CONSOLE_DEBUG_W_NUM("gImageCount      \t=", gImageCount);
			if (gImageList[gBackGroundImgIdx].FitsProcessed == false)
			{
				if (gImageList[gBackGroundImgIdx].ImageFileType == kImageFileType_FITS)
				{
					ReadFitsInfoForImage(&gImageList[gBackGroundImgIdx]);
				}
				else
				{
					CONSOLE_DEBUG_W_STR("File is PDS\t=", gImageList[gBackGroundImgIdx].FileName);
					gImageList[gBackGroundImgIdx].FitsProcessed	=	true;
				}
				cFitsProcessCntr++;
			}
			gBackGroundImgIdx++;

			//*	update the window but not every time
			if ((gBackGroundImgIdx % 10) == 0)
			{
				if (cImageListTabObjPtr != NULL)
				{
					cImageListTabObjPtr->UpdateOnScreenWidgetList();
					HandleWindowUpdate();
				}
			}
		}
		else if (cFitsProcessCntr > 0)
		{
			//*	if the sort order gets changed while we are reading the fits info,
			//*	this will make sure everything gets read properly
			gBackGroundImgIdx	=	0;
			cFitsProcessCntr	=	0;
		}
	}
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
static int	FileNameSort(const void *e1, const void* e2)
{
int				retValue;
TYPE_ImageFile	*entry1;
TYPE_ImageFile	*entry2;

	entry1		=	(TYPE_ImageFile *)e1;
	entry2		=	(TYPE_ImageFile *)e2;
	retValue	=	strcmp(entry1->FileName, entry2->FileName);
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
static int	BuildFileList(const char *directoryPath, int maxFiles)
{
DIR				*directory;
struct dirent	*dir;
bool			keepGoing;
int				fileIndex;
char			curFileName[512];
char			fileExtension[16];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, directoryPath);
	directory	=	opendir(directoryPath);
	fileIndex	=	0;
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

				if ((strcasecmp(fileExtension, ".fits") == 0) ||
					(strcasecmp(fileExtension, ".fit") == 0) ||
					(strcasecmp(fileExtension, ".img") == 0) ||
					(strcasecmp(fileExtension, ".imq") == 0))
				{
					if (fileIndex < maxFiles)
					{
						gImageList[fileIndex].validEntry	=	true;
						strcpy(gImageList[fileIndex].DirectoryPath,	directoryPath);
						strcpy(gImageList[fileIndex].FileName,		curFileName);
						strcpy(gImageList[fileIndex].FilePath,		directoryPath);
						strcat(gImageList[fileIndex].FilePath,		curFileName);


						//*	check file extension to determine file type
						SetImageFileType(&gImageList[fileIndex],fileExtension);

						fileIndex++;
					}
					else
					{
						CONSOLE_DEBUG_W_NUM("Ran out of room, cnt\t=",	fileIndex);
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
		if  (fileIndex > 1)
		{
			qsort(gImageList, fileIndex, sizeof(TYPE_ImageFile), FileNameSort);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open directory\t=",	directoryPath);
	}
//	CONSOLE_ABORT(__FUNCTION__);
	return(fileIndex);
}

//*****************************************************************************
static int	ReadFileDirectory(const char *directoryPath)
{
int				totalFileCount;
int				fitsFileCount;
int				jpegFileCount;
int				pngFileCount;
DIR				*directory;
struct dirent	*dir;
bool			keepGoing;
int				errorCode;
char			curFileName[512];
char			fileExtension[16];

	CONSOLE_DEBUG_W_STR(__FUNCTION__, directoryPath);

//	memset(gUSBtable, 0, sizeof(gUSBtable));
	totalFileCount	=	0;
	fitsFileCount	=	0;
	jpegFileCount	=	0;
	pngFileCount	=	0;

	directory	=	opendir(directoryPath);
	if (directory != NULL)
	{
		keepGoing	=	true;
		while (keepGoing)
		{
			dir	=	readdir(directory);
			if (dir != NULL)
			{
				totalFileCount++;
				strcpy(curFileName, dir->d_name);
				ExtractFileExtension(curFileName, fileExtension);

				if ((strcasecmp(fileExtension, ".fits") == 0) ||
					(strcasecmp(fileExtension, ".fit") == 0) ||
					(strcasecmp(fileExtension, ".img") == 0) ||
					(strcasecmp(fileExtension, ".imq") == 0))
				{
					fitsFileCount++;
				}
				if ((strcasecmp(fileExtension, ".jpg") == 0) ||
					(strcasecmp(fileExtension, ".jpeg") == 0))
				{
					jpegFileCount++;
				}
				if (strcasecmp(fileExtension, ".png") == 0)
				{
					pngFileCount++;
				}
			}
			else
			{
				keepGoing	=	false;
			}
		}
		errorCode	=	closedir(directory);
		if (errorCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("closedir errorCode\t=", errorCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open:", directoryPath);
		CONSOLE_ABORT(__FUNCTION__);
	}
//	CONSOLE_DEBUG_W_NUM("fitsFileCount \t=",	fitsFileCount);
//	CONSOLE_DEBUG_W_NUM("jpegFileCount \t=",	jpegFileCount);
//	CONSOLE_DEBUG_W_NUM("pngFileCount  \t=",	pngFileCount);
//	CONSOLE_DEBUG_W_NUM("totalFileCount\t=",	totalFileCount);

	//==================================================================
	//*	now create the list of file names and populate it
	if (fitsFileCount > 0)
	{
		gImageList	=	(TYPE_ImageFile *)calloc(fitsFileCount + 5, sizeof(TYPE_ImageFile));
		if (gImageList != NULL)
		{
			gImageCount	=	BuildFileList(directoryPath, fitsFileCount);
		}
	}
	return(totalFileCount);
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


	CONSOLE_DEBUG(__FUNCTION__);
	sprintf(gFullVersionString,		"%s - %s build #%d", kApplicationName, kVersionString, kBuildNumber);
	strcpy(gDirectoryPath, "");

	objectsCreated		=	0;
	lastDebugMillis		=	millis();


	//*	deal with any options from the command line
	ProcessCmdLineArgs(argc, argv);

	//*	find the directory specified
	iii	=	1;
	while (iii<argc)
	{
		if (argv[iii][0] != '-')
		{
			//*	this should be a directory
			strcpy(gDirectoryPath, argv[iii]);
		}
		iii++;
	}
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, gDirectoryPath);
	if (strlen(gDirectoryPath) > 0)
	{
		ReadFileDirectory(gDirectoryPath);
	}
	else
	{
		exit(0);
	}
	skyImageCtrlObj	=	new ControllerSkyImage("SkyImage");
	objectsCreated++;

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
void	ReadFitsInfoForImage(TYPE_ImageFile *imageFileData)
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
void	LoadNextImageFromList(ControllerImage *imageController)
{
	CONSOLE_DEBUG(__FUNCTION__);
	if (imageController != NULL)
	{
		gCurrentImageIndex++;
		if ((gCurrentImageIndex >= 0) && (gCurrentImageIndex < gImageCount))
		{
			imageController->LoadImage(gImageList[gCurrentImageIndex].FilePath);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("gCurrentImageIndex out of range", gCurrentImageIndex);
			gCurrentImageIndex	=	gImageCount - 1;
		}
	}
}

//*****************************************************************************
void	LoadPreviousImageFromList(ControllerImage *imageController)
{
	if (imageController != NULL)
	{
		gCurrentImageIndex--;
		if (gCurrentImageIndex < 0)
		{
			gCurrentImageIndex	=	0;
		}
		if ((gCurrentImageIndex >= 0) && (gCurrentImageIndex < gImageCount))
		{
			imageController->LoadImage(gImageList[gCurrentImageIndex].FilePath);
		}
	}
}


