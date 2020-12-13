//**************************************************************************************
//*	Read image file
//*		(c) by Mark Sproul
//*		msproul@skychariot.com
//**************************************************************************************
//*	Edit History
//**************************************************************************************
//*	May 20,	2019	<MLS> Started on Read image file
//*	May 22,	2019	<MLS> Image displayed in an openCV window
//**************************************************************************************

#include	<stdio.h>
#include	<string.h>
#include	<stdlib.h>
#include	<errno.h>
#include	<stdbool.h>
#include	<ctype.h>

#include	"opencv/highgui.h"

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


#ifndef true
	#define	true	1
	#define	false	0
#endif

//*****************************************************************************
//*	Json dictionary table structure
//*	enum values should be positive
//*****************************************************************************
#define	kSJP_MaxKeyLen	32
typedef struct
{
	char	keyword[kSJP_MaxKeyLen];
	int		kw_enumValue;


} SJP_Dictionary_t;


//********************************************************************
enum
{
	//*	the order of these does not matter
	kImage_command	=	0,
	kImage_imagetype,
	kImage_xsize,
	kImage_ysize,
	kImage_value,
		
	kImage_last
};

//********************************************************************
SJP_Dictionary_t	gImageDictionary[]	=
{
	//*	the order of these does not matter
	{	"command",		kImage_command		},
	{	"imagetype",	kImage_imagetype	},
	{	"xsize",		kImage_xsize		},
	{	"ysize",		kImage_ysize		},
	{	"value",		kImage_value		},


	//*	must be last
	{	"",			-1	}

};

//*****************************************************************************
//*	find a token in the table,
//*	stops when the there is an empty element in the table
//*	hard coded limit of 1000 entries in a table, just for code safety
//*	returns -1 if not found
//*****************************************************************************
int	SJP_FindKeyWordFromDictionary(	const char				*theKeyword,
									const SJP_Dictionary_t	*vocabList)
{
int		ii;
int		keyWordEnumValue;

	keyWordEnumValue	=	-1;
	
	if (vocabList != NULL)
	{
		ii	=	0;
		while ((keyWordEnumValue < 0) && (ii < 1000) && (strlen(vocabList[ii].keyword) > 0))
		{
			if (strcmp(theKeyword, vocabList[ii].keyword) == 0)
			{
				keyWordEnumValue	=	vocabList[ii].kw_enumValue;
			}
			ii++;
		}
	}	
	return(keyWordEnumValue);
}


//********************************************************************
typedef struct
{
	int		xSize;
	int		ySize;
	char	imageType[16];
	
} TYPE_ImageInfo;

//********************************************************************
//*&	returns true if it found something
bool	RemoveCRLF(char *theString)
{
int		sLen;
bool	eolFound;

	sLen		=	strlen(theString);
	eolFound	=	false;
	
	while (sLen > 0)
	{
		if ((theString[sLen - 1] == 0x0d) || (theString[sLen - 1] == 0x0a))
		{
			eolFound			=	true;
			theString[sLen - 1]	=	0;
		}
		else
		{
			break;
		}
		sLen	=	strlen(theString);
	}
	return(eolFound);
}



//**************************************************************************************
static void	ProcessCmdLineOptions(const char *argV0, const char *cmdLineOption)
{
	//*	process option
	switch(cmdLineOption[1])
	{
		case 'n':
			break;

	}
}

//**************************************************************************************
static void	PrintHelp(const char *appName)
{
	printf("Usage\n");
	printf("%s [-options] files\n", appName);
	printf("     -n Newline only (default is cr/lf)\n");

}

//**************************************************************************************
static void	ParseJsonLine(const char* jsonData, char *keyword, char *value)
{
int		sLen;
int		ii;
int		cc;

//	printf("%s\r\n", jsonData);

	keyword[0]	=	0;
	value[0]	=	0;
	sLen		=	strlen(jsonData);
	//*	first we are going to extract the keyword
	ii			=	0;
	cc			=	0;
	while ((jsonData[ii] > 0) && (jsonData[ii] != ':') && (ii<sLen) && (cc < 48))
	{
		if ((jsonData[ii] != '"') && (jsonData[ii] != 0x20) && (jsonData[ii] != 0x09))
		{
			keyword[cc++]	=	jsonData[ii];
			keyword[cc]		=	0;
		}
		ii++;
	}
	ii++;	//*	skip the ':'
	//*	now put the rest of it into the value
	cc			=	0;
	while ((jsonData[ii] > 0) && (ii<sLen) && (cc < 48))
	{
		if ((jsonData[ii] != '"') && (jsonData[ii] != ','))
		{
			value[cc++]	=	jsonData[ii];
			value[cc]	=	0;
		}
		ii++;
	}
	//*	make the keyword all lower case
	sLen		=	strlen(keyword);
	for (ii=0; ii<sLen; ii++)
	{
		keyword[ii]	=	tolower(keyword[ii]);
	}
//	printf("k=%-20s\tv=%-20s\r\n", keyword, value);
}

#define	kLineBuffSize	2048

//**************************************************************************************
static void	ReadImageValues(TYPE_ImageInfo *imageInfo, FILE	 *inputFileFP)
{
long		pixelCount;
long		ii;
char		lineBuff[kLineBuffSize];
long		pixelValue;
IplImage	*openCV_Image;

	printf("%s\r\n", __FUNCTION__);

	pixelCount	=	imageInfo->xSize * imageInfo->ySize;
	if (pixelCount > 0)
	{
		openCV_Image	=	cvCreateImage(cvSize(imageInfo->xSize, imageInfo->ySize), IPL_DEPTH_8U, 1);
		for (ii=0; ii<pixelCount; ii++)
		{
			fgets(lineBuff, (kLineBuffSize - 2), inputFileFP);
			pixelValue	=	atoi(lineBuff);
			if (openCV_Image != NULL)
			{
				openCV_Image->imageData[ii]	=	pixelValue;
			}
		}
		
		if (openCV_Image != NULL)
		{
			cvShowImage("camera", openCV_Image);
			cvWaitKey(0);
		}
	}
	else
	{
		printf("Pixel count is zero\r\n");
	}
}

//**************************************************************************************
static void	ReadImageArray(const char *fileName)
{
FILE			*inputFileFP;
char			lineBuff[kLineBuffSize];
char			keywordString[kLineBuffSize];
char			valueString[kLineBuffSize];
int				cc;
int				ii;
int				slen;
long			linesProcessed;
bool			eolFound;
TYPE_ImageInfo	myImageInfo;
int				keywordEnum;

	linesProcessed		=	0;
	inputFileFP	=	fopen(fileName, "r");
	if (inputFileFP != NULL)
	{
		printf("Working on %s\t", fileName);
		
		memset(&myImageInfo, 0,sizeof(TYPE_ImageInfo));
		
		while (fgets(lineBuff, (kLineBuffSize - 2), inputFileFP))
		{
			linesProcessed++;
			
			RemoveCRLF(lineBuff);
//			printf("%s\r\n", lineBuff);
			ParseJsonLine(lineBuff, keywordString, valueString);
			printf("k=%-20s\tv=%-20s\r\n", keywordString, valueString);
			keywordEnum	=	SJP_FindKeyWordFromDictionary(keywordString, gImageDictionary);
			switch(keywordEnum)
			{
				case kImage_imagetype:
					break;

				case kImage_xsize:
					myImageInfo.xSize	=	atoi(valueString);
					break;

				case kImage_ysize:
					myImageInfo.ySize	=	atoi(valueString);
					break;
				
				case kImage_value:
					ReadImageValues(&myImageInfo, inputFileFP);
					break;
				
				default:
//					printf("Did not find keyword\r\n");
					break;
			}
		}
		fclose(inputFileFP);
	}
	else
	{
		printf("Erro %d Failed to open file = %s\n", errno, fileName);
	}
}

//**************************************************************************************
int	main(int argc, char *argv[])
{
int		argIdx;
int		fileCount;

	if (argc <= 1)
	{
		PrintHelp(argv[0]);
		exit(0);
	}

	
	//*	lets look for options
	for (argIdx = 1; argIdx < argc; argIdx++)
	{
		if (argv[argIdx][0] == '-')
		{
			fileCount++;
			//*	process option
			ProcessCmdLineOptions(argv[0], argv[argIdx]);

		}
	}
	//*	now go back thru the list and look for file names
	for (argIdx = 1; argIdx < argc; argIdx++)
	{
		if (argv[argIdx][0] == '-')
		{
			fileCount++;
			//*	process option
			ProcessCmdLineOptions(argv[0], argv[argIdx]);

		}
		else
		{
			ReadImageArray(argv[argIdx]);
		}
	}
}
