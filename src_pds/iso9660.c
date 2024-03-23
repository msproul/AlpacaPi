//*****************************************************************************
//*	iso9660.c
//		https://en.wikipedia.org/wiki/ISO_9660
//		https://wiki.osdev.org/ISO_9660
//*****************************************************************************
//*	Edit history <MLS> Mark Sproul, msproul@skychariot.com
//*****************************************************************************
//*	Nov 20,	2022	<MLS> Started on ISO reader to read old style NASA CD-roms
//*	Nov 23,	2022	<MLS> Cleaned up directory reading code, one block at a time
//*****************************************************************************

#include	<ctype.h>
#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<stdint.h>
#include	<stdbool.h>
#include	<errno.h>
#include	<sys/stat.h>
#include	<sys/types.h>
#include	<unistd.h>
#include	<sys/sysmacros.h>
#include	<time.h>

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#pragma pack(1)
//*	LSBMSB  -> Least Significatn Byte first  / Most Significant Byte first
//*****************************************************************************
typedef struct
{
	uint32_t	LSB;
	uint32_t	MSB;
} TYPE_LSBMSB;

//*****************************************************************************
typedef struct
{
	uint16_t	LSB;
	uint16_t	MSB;
} TYPE_LSBMSB16;

//*****************************************************************************
typedef struct
{
	char	directoryName[64];
} TYPE_DirectoryPath;

//*****************************************************************************
typedef struct
{
	uint8_t			Length;
	uint8_t			ExtAttRecordLength;
	TYPE_LSBMSB		ExtentLocation;
	TYPE_LSBMSB		DataLength;
	char			RecordingDate[7];
	uint8_t			FileFlags;
	uint8_t			FileUnitSize;
	uint8_t			InterleaveGap;
	TYPE_LSBMSB16	VolumeSeqNum;
	uint8_t			FileIdentifierLength;
	char			FileIndetifier[1];
} DirectoryRecord;

//*****************************************************************************
typedef struct
{
	DirectoryRecord		DirRecord;
	char				filename[256];
	void				*SubDirectory;
} TYPE__DirectoryEntry;

//*****************************************************************************
typedef struct
{
	uint8_t			VolumeDescriptorType;
	char			StandardIdentifier[5];
	uint8_t			VolumeDescriptorVersion;
	char			UnusedField1;
	char			SystemIdentifier[32];
	char			VolumeIdentifier[32];
	char			UnusedField2[8];
	TYPE_LSBMSB		VolumeSpaceSize;
	char			UnusedField3[32];
	TYPE_LSBMSB16	VolumeSetSize;
	TYPE_LSBMSB16	VolumeSequenceNumber;
	TYPE_LSBMSB16	LogicalBlockSize;
	TYPE_LSBMSB		PathTableSize;
	uint32_t		TypeLPathTable;
	uint32_t		TypeLPathTable_O;
	uint32_t		TypeMPathTable;
	uint32_t		TypeMPathTable_O;
	DirectoryRecord	DirRecordRootDirectory;
	char			VolumeSetIdentifier[128];
	char			PublisherIdentifier[128];
	char			DataPreparerIdentifier[128];
	char			ApplicationIdentifier[128];
	char			CopyrightFileIdentifier[37];
	char			AbstractFileIdentifier[37];
	char			BibliographicFileIdentifier[37];


	char			VolumeCreationDateTime[17];
	char			VolumeModificationDateTime[17];
	char			VolumeExpirationDateTime[17];
	char			VolumeEffectiveDateTime[17];
	uint8_t			FileStructureVersion;
	uint8_t			Unused;
	char			ApplicationUsed[512];
	char			ReservedFutureStandardization[653];

//	char	filler[2048];
} PrimaryVolumeDescriptor;
#pragma pack(0)

PrimaryVolumeDescriptor	gPVD;

#define	kMaxDirEntries	600
#define	kMaxLevelsDeep	8
TYPE__DirectoryEntry	gRootDirectory[kMaxDirEntries];
TYPE_DirectoryPath		gDirectoryPathTree[kMaxLevelsDeep];
int						gTotalBytesWritten		=	0;
bool					gCreateFlag				=	false;
bool					gDisplayDirectoryFlag	=	false;
bool					gProcessRecursive		=	true;
bool					gVerboseFlag			=	false;


//*****************************************************************************
void	StripTrailingSpaces(char *theString, int maxSLen)
{
int		ccc;

//	CONSOLE_DEBUG_W_NUM("maxSLen\t=", maxSLen);
	ccc	=	maxSLen - 1;
	while ((ccc > 0) && (theString[ccc] == 0x20))
	{
		theString[ccc]	=	0;
		ccc--;
	}
}

//*****************************************************************************
static void	Swap4Bytes(int *value)
{
int		byte1;
int		byte2;
int		byte3;
int		byte4;
int		newValue;

	byte1		=	(*value >> 24) & 0x00ff;
	byte2		=	(*value >> 16) & 0x00ff;
	byte3		=	(*value >> 8) & 0x00ff;
	byte4		=	(*value) & 0x00ff;

	newValue	=	byte4 << 24;
	newValue	+=	byte3 << 16;
	newValue	+=	byte2 << 8;
	newValue	+=	byte1;

	*value		=	newValue;

}


//*****************************************************************************
void	PrintDirectoryRecord(DirectoryRecord *dirRec)
{
int		year;
int		month;
int		day;
char	dateString[32];
int		iii;
char	*dataPtr;

	year	=	1900 + (dirRec->RecordingDate[0] & 0x00ff);
	month	=	(dirRec->RecordingDate[1] & 0x00ff);
	day		=	(dirRec->RecordingDate[2] & 0x00ff);
	sprintf(dateString, "%04d-%02d-%02d", year, month, day);


	CONSOLE_DEBUG_W_NUM(	"Length              \t=",	dirRec->Length);
	CONSOLE_DEBUG_W_NUM(	"ExtAttRecordLength  \t=",	dirRec->ExtAttRecordLength);
	CONSOLE_DEBUG_W_NUM(	"ExtentLocation      \t=",	dirRec->ExtentLocation.LSB);
	CONSOLE_DEBUG_W_NUM(	"DataLength          \t=",	dirRec->DataLength.LSB);
	CONSOLE_DEBUG_W_STR(	"RecordingDate       \t=",	dateString);
	CONSOLE_DEBUG_W_HEX(	"FileFlags           \t=",	dirRec->FileFlags);
	CONSOLE_DEBUG_W_NUM(	"FileUnitSize        \t=",	dirRec->FileUnitSize);
	CONSOLE_DEBUG_W_NUM(	"InterleaveGap       \t=",	dirRec->InterleaveGap);
	CONSOLE_DEBUG_W_NUM(	"VolumeSeqNum        \t=",	dirRec->VolumeSeqNum.LSB);
	CONSOLE_DEBUG_W_HEX(	"VolumeSeqNum        \t=",	dirRec->VolumeSeqNum.LSB);
	CONSOLE_DEBUG_W_NUM(	"FileIdentifierLength\t=",	dirRec->FileIdentifierLength);

//	dataPtr	=	(char *)dirRec;
//	for (iii=0; iii<sizeof(DirectoryRecord); iii++)
//	{
//		CONSOLE_DEBUG_W_HEX(	"HEXvalue       \t=",	(dataPtr[iii] & 0x0ff));
//	}
}


//*****************************************************************************
void	DumpHex(uint8_t *dataBuffer, int length)
{
int		iii;
int		jjj;
char	theChar;

	iii	=	0;
	while (iii<length)
	{
		printf("%04X\t", iii);
		for (jjj=0; jjj<16; jjj++)
		{
			printf("%02X ", (dataBuffer[iii+jjj] & 0x00ff));
		}
		printf("\t[");
		for (jjj=0; jjj<16; jjj++)
		{
			theChar	=	(dataBuffer[iii+jjj] & 0x00ff);
			if ((theChar < 0x20) || (theChar >= 0x7f))
			{
				theChar	=	'.';
			}
			printf("%c", theChar);
		}
		printf("]\r\n");
		iii	+=	16;
	}
}

//*****************************************************************************
void SeekToBlock(FILE *rawDatafilePointer, int blockNumber)
{
size_t		filePositionOffset;
long		currentOffset;
int			errCode;

	filePositionOffset	=	blockNumber * gPVD.LogicalBlockSize.LSB;

	if (gVerboseFlag)
	{
		CONSOLE_DEBUG_W_NUM("Seeking to block number\t=", blockNumber);
		CONSOLE_DEBUG_W_LHEX("New file offset        \t=", (unsigned long)filePositionOffset);
	}
	errCode				=	fseek(rawDatafilePointer, filePositionOffset, SEEK_SET);
	if (errCode == 0)
	{
		currentOffset	=	ftell(rawDatafilePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("errno\t=", errno);
	}
	if (gVerboseFlag)
	{
		CONSOLE_DEBUG_W_LONG(	"filePositionOffset\t\t=",	filePositionOffset);
		CONSOLE_DEBUG_W_NUM(	"LogicalBlockSize  \t\t=",	gPVD.LogicalBlockSize.LSB);
		CONSOLE_DEBUG_W_LONG(	"currentOffset     \t\t=",	currentOffset);
	}
}

int	gLevelsdeep	=	0;

//*****************************************************************************
void	PrintDirectoryEntry(TYPE__DirectoryEntry *dirEntry)
{
int		year;
int		month;
int		day;
int		hour;
int		minute;
int		second;
int		gmtOffst;
char	dateString[32];
char	fileFlagsString[10]	=	"--------";
int		iii;

	year		=	1900 + (dirEntry->DirRecord.RecordingDate[0] & 0x00ff);
	month		=	(dirEntry->DirRecord.RecordingDate[1] & 0x00ff);
	day			=	(dirEntry->DirRecord.RecordingDate[2] & 0x00ff);
	hour		=	(dirEntry->DirRecord.RecordingDate[3] & 0x00ff);
	minute		=	(dirEntry->DirRecord.RecordingDate[4] & 0x00ff);
	second		=	(dirEntry->DirRecord.RecordingDate[5] & 0x00ff);
	gmtOffst	=	(dirEntry->DirRecord.RecordingDate[6] & 0x00ff);
	sprintf(dateString, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);

	if (dirEntry->DirRecord.FileFlags & 0x80)
	{
		fileFlagsString[0]	=	'+';
	}
	if (dirEntry->DirRecord.FileFlags & 0x02)
	{
		fileFlagsString[6]	=	'd';
	}

	for (iii=1; iii<gLevelsdeep; iii++)
	{
		printf("\t");
	}
	printf("%s ",		fileFlagsString);
	printf("%2d ",		(dirEntry->DirRecord.Length & 0x00ff));
	printf("%8d ",		dirEntry->DirRecord.DataLength.LSB);
	printf("%s ",		dateString);
	printf("%-20s\t",	dirEntry->filename);
	printf("->%4d ",	dirEntry->DirRecord.ExtentLocation.LSB);



	printf("\r\n");

}


//*****************************************************************************
void	DisplayFileStat(const char *pathName)
{
struct stat sb;

	if (lstat(pathName, &sb) == 0)
	{
		printf("ID of containing device:  [%jx,%jx]\n",
								(uintmax_t) major(sb.st_dev),
								(uintmax_t) minor(sb.st_dev));

		printf("File type:                ");

		switch (sb.st_mode & S_IFMT)
		{
			case S_IFBLK:  printf("block device\n");		break;
			case S_IFCHR:  printf("character device\n");	break;
			case S_IFDIR:  printf("directory\n");			break;
			case S_IFIFO:  printf("FIFO/pipe\n");			break;
			case S_IFLNK:  printf("symlink\n");				break;
			case S_IFREG:  printf("regular file\n");		break;
			case S_IFSOCK: printf("socket\n");				break;
			default:		printf("unknown?\n");			break;
		}

		printf("I-node number:            %ju\n", (uintmax_t) sb.st_ino);

		printf("Mode:                     %jo (octal)\n",
				(uintmax_t) sb.st_mode);

		printf("Link count:               %ju\n", (uintmax_t) sb.st_nlink);
		printf("Ownership:                UID=%ju   GID=%ju\n",
				(uintmax_t) sb.st_uid, (uintmax_t) sb.st_gid);

		printf("Preferred I/O block size: %jd bytes\n",
				(intmax_t) sb.st_blksize);
		printf("File size:                %jd bytes\n",
				(intmax_t) sb.st_size);
		printf("Blocks allocated:         %jd\n",
				(intmax_t) sb.st_blocks);

		printf("Last status change:       %s", ctime(&sb.st_ctime));
		printf("Last file access:         %s", ctime(&sb.st_atime));
		printf("Last file modification:   %s", ctime(&sb.st_mtime));
   }
}

//*****************************************************************************
void	BuildPath(char *fileSystemPath, const int level)
{
int			iii;

	fileSystemPath[0]	=	0;
	for (iii=0; iii<=level; iii++)
	{
		strcat(fileSystemPath, gDirectoryPathTree[iii].directoryName);
		strcat(fileSystemPath, "/");
	}
}

//*****************************************************************************
int		CreateDirectory(const int level)
{
struct stat	fileStatus;
char		myDirectoryPath[512];
int			iii;
int			returnCode;

//	CONSOLE_DEBUG(__FUNCTION__);
	BuildPath(myDirectoryPath, level);

	CONSOLE_DEBUG_W_STR("myDirectoryPath\t=", myDirectoryPath);

	returnCode	=	mkdir(myDirectoryPath, 0744);
	if (returnCode != 0)
	{
		if (errno != EEXIST)
		{
			CONSOLE_DEBUG_W_NUM("Error creating directory errno\t=", errno);
		}
	}
//	DisplayFileStat(myDirectoryPath);
}

//*****************************************************************************
//*	returns number of bytes copied
//*****************************************************************************
int	CreateAndCopyFile(FILE *rawDatafilePointer, TYPE__DirectoryEntry *dirEntry)
{
char	myFilePath[256];
char	dataBuffer[5000];
int		dataBytesRead;
size_t	myFileSize;
uint8_t	*fileBufferPtr;
FILE	*dataFilePointer;
size_t	bytesWritten;

//	CONSOLE_DEBUG_W_STR(__FUNCTION__,	dirEntry->filename);
	bytesWritten	=	0;
	BuildPath(myFilePath, gLevelsdeep-1);
	strcat(myFilePath, dirEntry->filename);
	CONSOLE_DEBUG_W_STR("myFilePath\t\t=",	myFilePath);

//	CONSOLE_DEBUG_W_NUM("ExtentLocation\t=",	dirEntry->DirRecord.ExtentLocation.LSB);
//	CONSOLE_DEBUG_W_NUM("DataLength    \t=",	dirEntry->DirRecord.DataLength.LSB);

	SeekToBlock(rawDatafilePointer, dirEntry->DirRecord.ExtentLocation.LSB + 1);

	myFileSize		=	dirEntry->DirRecord.DataLength.LSB;
	fileBufferPtr	=	calloc(1, (myFileSize + 100));
	if (fileBufferPtr != NULL)
	{
		dataBytesRead		=	fread(fileBufferPtr, 1, myFileSize, rawDatafilePointer);
//		CONSOLE_DEBUG_W_NUM("dataBytesRead    \t=",	dataBytesRead);
		if (dataBytesRead == myFileSize)
		{
			//*	write out the file
			dataFilePointer	=	fopen(myFilePath, "w");
			if (dataFilePointer != NULL)
			{
				bytesWritten	=	fwrite(fileBufferPtr, 1, myFileSize, dataFilePointer);
//				CONSOLE_DEBUG_W_LONG("bytesWritten    \t=",	bytesWritten);

				fclose(dataFilePointer);
			}
		}

//		DumpHex(fileBufferPtr, 0x400);

		free(fileBufferPtr);
	}
	return(bytesWritten);
}



//*****************************************************************************
//*	returns number of valid entries
//*****************************************************************************
int	BuildDirectory(	FILE					*rawDatafilePointer,
					DirectoryRecord			*currentDirectory,
					TYPE__DirectoryEntry	*dirArray)
{
DirectoryRecord			*dirRecordPtr;
int						dirRecordSize;
bool					keepGoing;
uint8_t					*myDataPtr;
uint8_t					*myBlockPtr;
int						fileNameLen;
int						dirEntryIdx;
TYPE__DirectoryEntry	localDirEntry;
size_t					bytesUsedInBlock;
size_t					bytesLeftInBlock;
int						blocksProcessed;
int						directoryByteCount;
int						blockCount;
int						iii;
uint8_t					directoryBuffer[2100];
int						blkNum;
int						dataBytesRead;

	//*	the directory is in 2048 byte blocks.
	//*	a directory entry cannot span across a block boundry
	//*	so at the end of each block, we have to skip a few bytes

	//*	figure out how many blocks we should be processing
	directoryByteCount	=	currentDirectory->DataLength.LSB;
	blockCount			=	directoryByteCount / 2048;
//	CONSOLE_DEBUG_W_NUM("blockCount\t\t\t=", blockCount);

	dirEntryIdx		=	0;
	blocksProcessed	=	0;
	for (blkNum=0; blkNum<blockCount; blkNum++)
	{
		dataBytesRead	=	fread(directoryBuffer, 1, 2048, rawDatafilePointer);
//		CONSOLE_DEBUG_W_NUM("dataBytesRead\t\t=", dataBytesRead);

		myDataPtr		=	directoryBuffer;
		keepGoing		=	true;
		while (keepGoing)
		{
			dirRecordPtr	=	(DirectoryRecord*)myDataPtr;
			dirRecordSize	=	dirRecordPtr->Length;
			if (dirRecordSize > 0)
			{
				if (gVerboseFlag)
				{
					CONSOLE_DEBUG_W_NUM("dirRecordSize   \t\t=", dirRecordSize);
					CONSOLE_DEBUG_W_LHEX("START:myDataPtr\t\t=", (unsigned long)(myDataPtr - directoryBuffer));
					DumpHex(myDataPtr, dirRecordSize);
				}
				localDirEntry.DirRecord	=	*dirRecordPtr;
				fileNameLen				=	dirRecordPtr->FileIdentifierLength & 0x00ff;
				strncpy(localDirEntry.filename, dirRecordPtr->FileIndetifier, fileNameLen);
				localDirEntry.filename[fileNameLen]	=	0;
				//*	lets make it lower case
				for (iii=0; iii<fileNameLen; iii++)
				{
					localDirEntry.filename[iii]	=	tolower(localDirEntry.filename[iii]);
				}
				//*	take care of "." and "..", current directory and parent directory
				if ((fileNameLen <= 1) && (dirEntryIdx == 0))
				{
					strcpy(localDirEntry.filename, ".");
				}
				if ((fileNameLen <= 1) && (dirEntryIdx == 1))
				{
					strcpy(localDirEntry.filename, "..");
				}
				//*	check to see if it is a file (not a directory)
				if ((dirRecordPtr->FileFlags & 0x02) == 0)
				{
					if (localDirEntry.filename[fileNameLen - 2] == ';')
					{
						localDirEntry.filename[fileNameLen - 2]	=	0;
					}
				}

	//			PrintDirectoryEntry(&localDirEntry);
				if (dirEntryIdx < kMaxDirEntries)
				{
					dirArray[dirEntryIdx]	=	localDirEntry;
					dirEntryIdx++;
				}
				else
				{
					CONSOLE_DEBUG("OUT OF MEMORY IN DIRECTORY TREE!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
					CONSOLE_ABORT(__FUNCTION__);
				}

				myDataPtr	+=	dirRecordSize;
			}
			else
			{
				keepGoing	=	false;
				if (gVerboseFlag)
				{
					CONSOLE_DEBUG("We are the end of the current block (should be empty)");
					DumpHex(myDataPtr, 48);
				}
			}
		}
		blocksProcessed++;
	}
	if (blocksProcessed < blockCount)
	{
		CONSOLE_DEBUG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
		CONSOLE_DEBUG_W_NUM("entries found  \t=", dirEntryIdx);
		CONSOLE_DEBUG_W_NUM("directoryByteCount\t=", directoryByteCount);
		CONSOLE_DEBUG_W_NUM("blockCount        \t=", blockCount);
		CONSOLE_DEBUG_W_NUM("blocksProcessed   \t=", blocksProcessed);
		CONSOLE_ABORT(__FUNCTION__);
	}
	if (gVerboseFlag)
	{
		CONSOLE_DEBUG_W_NUM("blocksProcessed\t=", blocksProcessed);
		CONSOLE_DEBUG_W_NUM("entries found  \t=", dirEntryIdx);
	}
	return(dirEntryIdx);
}


//*****************************************************************************
//*	LBA = Logical block addressing
//*****************************************************************************
void	ReadDirectoryRecursivly(FILE					*rawDatafilePointer,
								DirectoryRecord			*currentDirectory,
								const int				directoryLBA,
								TYPE__DirectoryEntry	*dirArray)
{
int		validEntries;
int		dataBytesRead;
int		iii;

	gLevelsdeep++;

	if (gVerboseFlag)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		CONSOLE_DEBUG_W_NUM("directoryLBA\t=", directoryLBA);
	}

	//*	go to the starting block of the directory
	SeekToBlock(rawDatafilePointer, directoryLBA);


	validEntries	=	BuildDirectory(rawDatafilePointer, currentDirectory, dirArray);
	if (gVerboseFlag)
	{
		CONSOLE_DEBUG_W_NUM("validEntries\t=", validEntries);
	}
	for (iii=2; iii<validEntries; iii++)
	{
		//*	do we want to display the directory entry
		if (gDisplayDirectoryFlag)
		{
			PrintDirectoryEntry(&dirArray[iii]);
		}
		if (dirArray[iii].DirRecord.FileFlags & 0x02)
		{
			//*	we have a directory, parse it
			//*	LBA = Logical block addressing
			if (dirArray[iii].DirRecord.ExtentLocation.LSB != directoryLBA)
			{
				strcpy(gDirectoryPathTree[gLevelsdeep].directoryName,	dirArray[iii].filename);
				if (gCreateFlag)
				{
					CreateDirectory(gLevelsdeep);
				}

				dirArray[iii].SubDirectory	=	malloc(kMaxDirEntries * sizeof(TYPE__DirectoryEntry));
				if (dirArray[iii].SubDirectory != NULL)
				{
					if (gProcessRecursive)
					{
						ReadDirectoryRecursivly(	rawDatafilePointer,
													&dirArray[iii].DirRecord,
													dirArray[iii].DirRecord.ExtentLocation.LSB + 4,
													dirArray[iii].SubDirectory);
					}
				}
				else
				{
					CONSOLE_DEBUG("Failed to allocate directory array")
					CONSOLE_ABORT(__FUNCTION__);
				}
			}
			else
			{
				CONSOLE_DEBUG("LBA is the same")
			}
		}
		else
		{
			if (gCreateFlag)
			{
				//*	we have a file, lets copy it
				gTotalBytesWritten	+=	CreateAndCopyFile(rawDatafilePointer, &dirArray[iii]);
			}
		}
	}
	if (gCreateFlag)
	{
		CONSOLE_DEBUG_W_NUM("gTotalBytesWritten (meg)\t=", gTotalBytesWritten / (1024 * 1024));
	}
	gLevelsdeep--;
}


//*****************************************************************************
void	ExtractVolumeName(void)
{
FILE					*rawDatafilePointer;
long					currentOffset;
size_t					filePositionOffset;
char					filePath[]	=	"/dev/sr0";
int						dataBytesRead;
int						nameLen;

	rawDatafilePointer	=	fopen(filePath, "r");
	if (rawDatafilePointer != NULL)
	{
		filePositionOffset	=	0x08000;
		currentOffset		=	fseek(rawDatafilePointer, filePositionOffset, SEEK_SET);
		dataBytesRead		=	fread(&gPVD, 1, 2048, rawDatafilePointer);

		StripTrailingSpaces(gPVD.VolumeIdentifier,				sizeof(gPVD.VolumeIdentifier));

		nameLen	=	strlen(gPVD.VolumeIdentifier);
//		CONSOLE_DEBUG_W_NUM("nameLen\t=", nameLen);
		printf(	"%s\n",	gPVD.VolumeIdentifier);
		fclose(rawDatafilePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open", filePath);
	}
}

//*****************************************************************************
void	PrintHelp(const char *appname)
{
	printf("Usage: %s\r\n", appname);
	printf(	"	-d\tDirectory listing\r\n");
	printf(	"	-h\tHelp (this message)\r\n");
	printf(	"	-n\tExtract volume Name\r\n");
	printf(	"	-t\tTop level directory listing only\r\n");
	printf(	"	-v\tVerbose, (for debugging)\r\n");
	printf(	"	-w\tWrite data\r\n");
}

//*****************************************************************************
int main(int argc, char *argv[])
{
int						iii;
FILE					*rawDatafilePointer;
long					currentOffset;
size_t					filePositionOffset;
char					dataBuffer[5000];
char					cdIndicator[32];
char					volumeName[32];
char					*dataPtr;
char					filePath[]	=	"/dev/sr0";
int						dataBytesRead;
size_t					pvdSize;
int						megaBytesOnDisk;

	for (iii=1; iii< argc; iii++)
	{
		if (argv[iii][0] == '-')
		{
			switch(argv[iii][1])
			{
				//	-d	Directory listin
				case 'd':
					gDisplayDirectoryFlag	=	true;
					break;

				//	-h
				case 'h':
					PrintHelp(argv[0]);
					exit(0);
					break;

				//	-n
				case 'n':
					ExtractVolumeName();
					exit(0);
					break;

				//*	-t  top level only
				case 't':
					gProcessRecursive		=	false;
					gDisplayDirectoryFlag	=	true;
					gCreateFlag				=	false;
					break;

				//	-v		Verbose flag
				case 'v':
					gVerboseFlag				=	true;
					break;

				//	-w
				case 'w':
					CONSOLE_DEBUG("Writing data enabled!!!!!!!!!!!!!");
					gCreateFlag	=	true;
					break;
			}
		}
	}
	CONSOLE_DEBUG("NASA CD-ROM reader");

	memset(gRootDirectory, 0, sizeof*(gRootDirectory));


	pvdSize	=	sizeof(PrimaryVolumeDescriptor);
	CONSOLE_DEBUG_W_SIZE("sizeof(PrimaryVolumeDescriptor) \t=",	pvdSize);
	CONSOLE_DEBUG_W_SIZE("sizeof(DirectoryRecord)         \t=",	sizeof(DirectoryRecord));


	if (pvdSize != 2048)
	{
		CONSOLE_ABORT("pvdSize != 2048");
	}

	rawDatafilePointer	=	fopen(filePath, "r");
	if (rawDatafilePointer != NULL)
	{
		filePositionOffset	=	0x08000;
		currentOffset		=	fseek(rawDatafilePointer, filePositionOffset, SEEK_SET);
		dataBytesRead		=	fread(&gPVD, 1, 2048, rawDatafilePointer);
		CONSOLE_DEBUG_W_NUM("dataBytesRead\t\t\t=", dataBytesRead);

		StripTrailingSpaces(gPVD.VolumeIdentifier,				sizeof(gPVD.VolumeIdentifier));
		StripTrailingSpaces(gPVD.VolumeSetIdentifier,			sizeof(gPVD.VolumeSetIdentifier));
		StripTrailingSpaces(gPVD.PublisherIdentifier,			sizeof(gPVD.PublisherIdentifier));
		StripTrailingSpaces(gPVD.DataPreparerIdentifier,		sizeof(gPVD.DataPreparerIdentifier));
		StripTrailingSpaces(gPVD.ApplicationIdentifier,			sizeof(gPVD.ApplicationIdentifier));
		StripTrailingSpaces(gPVD.CopyrightFileIdentifier,		sizeof(gPVD.CopyrightFileIdentifier));
		StripTrailingSpaces(gPVD.AbstractFileIdentifier,		sizeof(gPVD.AbstractFileIdentifier));
		StripTrailingSpaces(gPVD.BibliographicFileIdentifier,	sizeof(gPVD.BibliographicFileIdentifier));
		StripTrailingSpaces(gPVD.ApplicationUsed,				sizeof(gPVD.ApplicationUsed));


		Swap4Bytes(&gPVD.TypeMPathTable);
		Swap4Bytes(&gPVD.TypeMPathTable_O);

		CONSOLE_DEBUG_W_NUM(	"VolumeDescriptorType       \t=",	gPVD.VolumeDescriptorType);
		CONSOLE_DEBUG_W_STR(	"StandardIdentifier         \t=",	gPVD.StandardIdentifier);
		CONSOLE_DEBUG_W_NUM(	"VolumeDescriptorVersion    \t=",	gPVD.VolumeDescriptorVersion);
//		CONSOLE_DEBUG_W_STR(	"SystemIdentifier           \t=",	gPVD.SystemIdentifier);
		CONSOLE_DEBUG_W_STR(	"VolumeIdentifier           \t=",	gPVD.VolumeIdentifier);
		CONSOLE_DEBUG_W_NUM(	"VolumeSpaceSize            \t=",	gPVD.VolumeSpaceSize.LSB);
		CONSOLE_DEBUG_W_NUM(	"VolumeSetSize              \t=",	gPVD.VolumeSetSize.LSB);
		CONSOLE_DEBUG_W_NUM(	"VolumeSequenceNumber        \t=",	gPVD.VolumeSequenceNumber.LSB);
		CONSOLE_DEBUG_W_NUM(	"LogicalBlockSize           \t=",	gPVD.LogicalBlockSize.LSB);
		CONSOLE_DEBUG_W_HEX(	"LogicalBlockSize           \t=",	gPVD.LogicalBlockSize.LSB);

		CONSOLE_DEBUG_W_NUM(	"PathTableSize.LSB          \t=",	gPVD.PathTableSize.LSB);
		CONSOLE_DEBUG_W_NUM(	"TypeLPathTable             \t=",	gPVD.TypeLPathTable);
		CONSOLE_DEBUG_W_NUM(	"TypeLPathTable_O           \t=",	gPVD.TypeLPathTable_O);
		CONSOLE_DEBUG_W_NUM(	"TypeMPathTable             \t=",	gPVD.TypeMPathTable);
		CONSOLE_DEBUG_W_NUM(	"TypeMPathTable_O           \t=",	gPVD.TypeMPathTable_O);

		CONSOLE_DEBUG_W_STR(	"VolumeSetIdentifier        \t=",	gPVD.VolumeSetIdentifier);
		CONSOLE_DEBUG_W_STR(	"PublisherIdentifier        \t=",	gPVD.PublisherIdentifier);
		CONSOLE_DEBUG_W_STR(	"DataPreparerIdentifier     \t=",	gPVD.DataPreparerIdentifier);
		CONSOLE_DEBUG_W_STR(	"ApplicationIdentifier      \t=",	gPVD.ApplicationIdentifier);
		CONSOLE_DEBUG_W_STR(	"CopyrightFileIdentifier    \t=",	gPVD.CopyrightFileIdentifier);
		CONSOLE_DEBUG_W_STR(	"AbstractFileIdentifier     \t=",	gPVD.AbstractFileIdentifier);
		CONSOLE_DEBUG_W_STR(	"BibliographicFileIdentifier\t=",	gPVD.BibliographicFileIdentifier);

		CONSOLE_DEBUG_W_STR(	"VolumeCreationDateTime     \t=",	gPVD.VolumeCreationDateTime);
		CONSOLE_DEBUG_W_STR(	"VolumeModificationDateTime \t=",	gPVD.VolumeModificationDateTime);
		CONSOLE_DEBUG_W_STR(	"VolumeExpirationDateTime   \t=",	gPVD.VolumeExpirationDateTime);
		CONSOLE_DEBUG_W_STR(	"VolumeEffectiveDateTime    \t=",	gPVD.VolumeEffectiveDateTime);
		CONSOLE_DEBUG_W_NUM(	"FileStructureVersion (=1)  \t=",	gPVD.FileStructureVersion);
		CONSOLE_DEBUG_W_STR(	"ApplicationUsed            \t=",	gPVD.ApplicationUsed);
		CONSOLE_DEBUG_W_NUM(	"Unused (should be 0)       \t=",	gPVD.Unused);

		megaBytesOnDisk	=	(gPVD.VolumeSpaceSize.LSB * gPVD.LogicalBlockSize.LSB) / (1024 * 1024);
		CONSOLE_DEBUG_W_NUM(	"Megabytes on CDROM         \t=",	megaBytesOnDisk);

		PrintDirectoryRecord(&gPVD.DirRecordRootDirectory);

		//*	set up the directory tree
		strcpy(gDirectoryPathTree[0].directoryName,	gPVD.VolumeIdentifier);
		if (gCreateFlag)
		{
			CreateDirectory(0);
		}

		ReadDirectoryRecursivly(	rawDatafilePointer,
									&gPVD.DirRecordRootDirectory,
									gPVD.DirRecordRootDirectory.ExtentLocation.LSB,
									gRootDirectory);
//		DumpHex(dataBuffer, 0x880);

		fclose(rawDatafilePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("Failed to open", filePath);
	}
	CONSOLE_DEBUG_W_NUM(	"Megabytes on CDROM         \t=",	megaBytesOnDisk);
	if (gCreateFlag)
	{
		CONSOLE_DEBUG_W_NUM("gTotalBytesWritten (meg)\t=", gTotalBytesWritten / (1024 * 1024));
	}

	if (argc < 2)
	{
		PrintHelp(argv[0]);
	}

	return(0);
}

