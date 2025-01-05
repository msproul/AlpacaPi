//**************************************************************************
//*	Name:			usbmanager.cpp
//*
//*	Author:			Mark Sproul (C) 2023
//*
//*	Description:	C++ Driver for Alpaca protocol
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.  You
//*	must determine the suitability of this source code for your use.
//*
//*	Re-distributions of this source code must retain this copyright notice.
//*****************************************************************************
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Jun 10,	2023	<MLS> Created usbmanager.cpp
//*	Jun 10,	2023	<MLS> Added USB_BuildTable()
//*	Jun 10,	2023	<MLS> Added USB_GetPathFromID()
//*	Jun 14,	2023	<MLS> Added event logging if errors with usbquerry.sh
//*	Sep 20,	2023	<MLS> Added USB_DumpTable()
//*	May  9,	2024	<MLS> Added auto init to USB_GetPathFromID()
//*	Jun  1,	2024	<MLS> Added ttyACM to scanned ports
//*****************************************************************************

#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>
#include	<dirent.h>
#include	<errno.h>
#include	<sys/stat.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"eventlogging.h"
#include	"usbmanager.h"

//*****************************************************************************
typedef struct
{
	char	usbPath[48];
	char	usbIDstring[64];
	bool	hasBeenClaimed;
} TYPE_USBentry;

#define	kMaxUSBdeviceCnt	10

static	TYPE_USBentry	gUSBtable[kMaxUSBdeviceCnt];
static	int				gUSBcount	=	-1;

static int	USB_BuildTable(void);
static int	USBpathSort(const void *e1, const void* e2);
static void	RunUSBprofile(void);



//*****************************************************************************
int	USB_InitTable(void)
{
//	CONSOLE_DEBUG(__FUNCTION__);

	if (gUSBcount <= 0)
	{
		gUSBcount	=	USB_BuildTable();
		if (gUSBcount > 0)
		{
			RunUSBprofile();
		}
	}
//	CONSOLE_DEBUG_W_NUM("gUSBcount\t=", gUSBcount);

	return(gUSBcount);
}

//*****************************************************************************
void	USB_DumpTable(void)
{
int		iii;

//	CONSOLE_DEBUG_W_NUM("gUSBcount\t=", gUSBcount);
	for (iii=0; iii<gUSBcount; iii++)
	{
		CONSOLE_DEBUG_W_STR(gUSBtable[iii].usbPath, gUSBtable[iii].usbIDstring);
	}
}


//*****************************************************************************
//*	Build list of USB serial devices and determine the type
//*	Returns the number found
//*****************************************************************************
static int	USB_BuildTable(void)
{
int				usbDeviceCnt;
DIR				*directory;
struct dirent	*dir;
bool			keepGoing;
int				errorCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	memset(gUSBtable, 0, sizeof(gUSBtable));
	usbDeviceCnt	=	0;

	directory	=	opendir("/dev/");
	if (directory != NULL)
	{
		keepGoing	=	true;
		while (keepGoing)
		{
			dir	=	readdir(directory);
			if (dir != NULL)
			{
				if ((strncmp(dir->d_name, "ttyUSB", 6) == 0) ||
					(strncmp(dir->d_name, "ttyACM", 6) == 0) ||
					(strncmp(dir->d_name, "ttyAMA", 6) == 0))
				{
//					CONSOLE_DEBUG_W_STR("USB serial device found->", dir->d_name);

					if (usbDeviceCnt < kMaxUSBdeviceCnt)
					{
						strcpy(gUSBtable[usbDeviceCnt].usbPath,	"/dev/");
						strcat(gUSBtable[usbDeviceCnt].usbPath,	dir->d_name);
						usbDeviceCnt++;
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
		if  (usbDeviceCnt > 1)
		{
			qsort(gUSBtable, usbDeviceCnt, sizeof(TYPE_USBentry), USBpathSort);
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
		CONSOLE_DEBUG("Failed to open /dev");
		CONSOLE_ABORT(__FUNCTION__);
	}

	return(usbDeviceCnt);
}

//*****************************************************************************
static int	USBpathSort(const void *e1, const void* e2)
{
int				retValue;
TYPE_USBentry	*entry1;
TYPE_USBentry	*entry2;

	entry1		=	(TYPE_USBentry *)e1;
	entry2		=	(TYPE_USBentry *)e2;
	retValue	=	strcmp(entry1->usbPath, entry2->usbPath);
	return(retValue);
}

//*****************************************************************************
static void	UpdateUSBidString(char *deviceName, const char *idString)
{
int		iii;

	for (iii=0; iii<gUSBcount; iii++)
	{
		if (strcasecmp(deviceName, gUSBtable[iii].usbPath) == 0)
		{
			strcpy(gUSBtable[iii].usbIDstring, idString);
			break;
		}
	}
}

//*****************************************************************************
bool	USB_GetPathFromID(const char *idString, char *usbPath)
{
int		iii;
bool	foundIt;

	//*	auto init
	if (gUSBcount <= 0)
	{
		USB_InitTable();
	}

//	USB_DumpTable();

//	CONSOLE_DEBUG_W_STR("Looking for:", idString);

	foundIt	=	false;
	iii		=	0;
	while ((foundIt == false) && (iii < gUSBcount))
	{
		//*	check for a match
		if (strstr(gUSBtable[iii].usbIDstring, idString) != NULL)
		{
//			CONSOLE_DEBUG_W_STR("Found:", gUSBtable[iii].usbIDstring);
			//*	make sure it has not been claimed already
			if (gUSBtable[iii].hasBeenClaimed == false)
			{
				strcpy(usbPath, gUSBtable[iii].usbPath);
				gUSBtable[iii].hasBeenClaimed	=	true;
				foundIt							=	true;
				break;
			}
		}
		iii++;
	}
	return(foundIt);
}

//*****************************************************************************
//*	run the script "usbquerry.sh" and parse the results
//*	https://unix.stackexchange.com/questions/144029/command-to-determine-ports-of-a-device-like-dev-ttyusb0
//*****************************************************************************
static void	RunUSBprofile(void)
{
int			systemRetCode;
char		commandString[64];
FILE		*filePointer;
char		lineBuff[128];
char		deviceName[128];
char		deviceIDstring[128];
char		*eolPtr;
char		*spcPtr;
int			returnCode;
struct stat	fileStatus;
char		usbQuerryName[]	=	"usbquerry.sh";
char		usbIDfileName[]	=	"usb_id.txt";

//	CONSOLE_DEBUG(__FUNCTION__);
	returnCode	=	stat(usbQuerryName, &fileStatus);		//*	fstat - check for existence of file
	if (returnCode == 0)
	{
		LogEvent(	__FUNCTION__,
					usbQuerryName,
					NULL,
					kASCOM_Err_Success,
					"Is present");

		strcpy(commandString,	"bash ");
		strcat(commandString,	usbQuerryName);
		strcat(commandString,	" > ");
		strcat(commandString,	usbIDfileName);

		systemRetCode	=	system(commandString);
		if (systemRetCode == 0)
		{
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("ERROR system() returned", systemRetCode);
		}

		//------------------------------------------------
		//*	check to make sure the output file got created
		returnCode	=	stat(usbIDfileName, &fileStatus);		//*	fstat - check for existence of file
		if (returnCode == 0)
		{
//			CONSOLE_DEBUG_W_SIZE("fileStatus.st_size\t=", fileStatus.st_size);
			if (fileStatus.st_size > 10)
			{
				//------------------------------------------------
				//*	now read in the file
				filePointer	=	fopen(usbIDfileName, "r");
				if (filePointer != NULL)
				{
					memset(lineBuff, 0, sizeof(lineBuff));
					while (fgets(lineBuff, 80, filePointer))
					{
						//*	get rid of CR or LF at the end of the line
						eolPtr	=	strchr(lineBuff, 0x0d);
						if (eolPtr != NULL)
						{
							*eolPtr	=	0;
						}
						eolPtr	=	strchr(lineBuff, 0x0a);
						if (eolPtr != NULL)
						{
							*eolPtr	=	0;
						}
//						CONSOLE_DEBUG(lineBuff);
						if (strlen(lineBuff) > 10)
						{
							//*	make sure its a valid tty device
							if (strncasecmp(lineBuff, "/dev/tty", 8) == 0)
							{
								strcpy(deviceName, lineBuff);
								spcPtr	=	strchr(deviceName, 0x20);
								if (spcPtr != NULL)
								{
									*spcPtr	=	0;

									//*	copy the device ID
									spcPtr	+=	3;
									strcpy(deviceIDstring, spcPtr);
									UpdateUSBidString(deviceName, deviceIDstring);
								}
							}
						}
					}
					fclose(filePointer);
				}
			}
		}
		else
		{
			CONSOLE_DEBUG_W_STR("Failed to open:", usbIDfileName);
			LogEvent(	__FUNCTION__,
						usbIDfileName,
						NULL,
						kASCOM_Err_Success,
						"Not created");
		}
	}
	else
	{
		LogEvent(	__FUNCTION__,
					usbQuerryName,
					NULL,
					kASCOM_Err_Success,
					"Missing file");
	}
//	CONSOLE_DEBUG(__FUNCTION__);
}

