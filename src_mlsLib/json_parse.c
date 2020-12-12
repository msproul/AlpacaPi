//*****************************************************************************
//*	json_parse.c
//*		simple json parser
//*		written by Mark Sproul mark.l.sproul.civ@mail.mil
//*		unlimited use rights, modify and use as you wish
//*		Please send updates and bug fixes to the above email address
//*
//*		Limitations:
//*			Does not differentiate nested constructs
//*			Limited error handling
//*			Keyword max length of 31 chars
//*			Value max length of 63 chars
//*
//*****************************************************************************
//*	Nov  8,	2018	<MLS> Started on json_parse library
//*	Nov 13,	2018	<MLS> Added support for escape chars (i.e. \t)
//*	Dec 11,	2018	<MLS> Added dictionary structure and lookup
//*	Feb  5,	2019	<MLS> Working on nested structure
//*	Feb  5,	2019	<MLS> Added "hdr", "data", and "error" block parsing
//*	Feb  6,	2019	<MLS> Changed structure to include "hdr", "data", and "error" blocks
//*	Feb  6,	2019	<MLS> Added SJP_FindKeyWordString()
//*	Feb  7,	2019	<MLS> Changed SJP_FindKeyWordString to force upper case
//*	Feb  7,	2019	<MLS> Added error SJP_NoHeader
//*	Feb  7,	2019	<MLS> Moved array data to valueString part of struct
//*	Feb 13,	2019	<MLS> Added version numbers
//*	Feb 13,	2019	<MLS> Added SJP_GetVersion()
//*	May 14,	2019	<MLS> Changed init routines to use memset, dramatic time improvement
//*	May 24,	2019	<MLS> Updated array parsing
//*	Jul  2,	2019	<MLS> Fixed handling of sub objects i.e. "TRACK_1" : {
//*	Aug  7,	2019	<MLS> Added SJP_DumpJsonData()
//*	Aug 10,	2019	<MLS> Got feedback from Rajeev@IAI, fixed some parsing issues
//*	Dec 31,	2019	<MLS> Fixed bug on first entry of an arry
//*	Mar  5,	2020	<MLS> Added _DEBUG_ARRAY_
//*	Mar  5,	2020	<MLS> Fixed bug when there is only one element in an array
//*	Mar  5,	2020	<MLS> At start of an array, there was a limit of 32 chars for 1st data element
//*****************************************************************************

//#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

//#define	_DEBUG_PARSER_


//#define	_ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#include	"json_parse.h"

//**************************************************************************************
void	SJP_Init(SJP_Parser_t *theParserDataStruct)
{
short		ii;

	if (theParserDataStruct != NULL)
	{
		theParserDataStruct->tokenCount_Hdr		=	0;
		theParserDataStruct->tokenCount_Data	=	0;
		theParserDataStruct->tokenCount_Errs	=	0;

		//*	set all the strings to null
		for (ii=0; ii<kSJP_MaxTokens_Hdr; ii++)
		{
			memset(&theParserDataStruct->headerList[ii], 0, sizeof(SJP_token_t));
		}

		for (ii=0; ii<kSJP_MaxTokens_Data; ii++)
		{
			memset(&theParserDataStruct->dataList[ii], 0, sizeof(SJP_token_t));
		}

		for (ii=0; ii<kSJP_MaxTokens_Errs; ii++)
		{
			memset(&theParserDataStruct->errorList[ii], 0, sizeof(SJP_token_t));
		}
	}
}

//**************************************************************************************
long	SJP_GetVersion(void)
{
	return(kJSONparse_Ver_long);
}

//**************************************************************************************
static void	ToUpperStr(char *theString)
{
short	sLen;
short	ii;

	sLen	=	strlen(theString);
	for (ii=0; ii<sLen; ii++)
	{
		theString[ii]	=	toupper(theString[ii]);
	}
}

//**************************************************************************************
//*	SJP_Private_GetString
//*		take in a ptr to a string
//*		skips any white space
//*		the first non-white space char determines the string type (quoted vs non-quoted)
//*		Returns an updated pointer to the data
//**************************************************************************************
static char *SJP_Private_GetString(	const char	*jsonDataPtr,
									char		*parsedString,
									const int	maxLen)
{
char	*myJsonDataPtr;
char	theFirstChar;
char	theSecondChar;
char	currChar;
short	cc;
short	uuCnt;
short	uuChar;

	parsedString[0]	=	0;
	myJsonDataPtr	=	(char *)jsonDataPtr;
	if (myJsonDataPtr != NULL)
	{
		//*	skip white space and any control chars (i.e. tab, cr, lf)
		while ((*myJsonDataPtr <= 0x20) && (*myJsonDataPtr > 0))
		{
			myJsonDataPtr++;
		}
		theFirstChar	=	*myJsonDataPtr;
		theSecondChar	=	*(myJsonDataPtr + 1);
		if ((theFirstChar == '"') && (theSecondChar == '{'))
		{
			myJsonDataPtr++;
			theFirstChar	=	*myJsonDataPtr;
		}
		if (theFirstChar == '{')
		{
		}
		else if (theFirstChar == '"')
		{
	#ifdef _DEBUG_ARRAY_
			CONSOLE_DEBUG("start of quoted string");
			CONSOLE_DEBUG_W_NUM("maxLen\t=", maxLen);
	#endif
			//*	we have a quoted string, proceed to the terminating quote
			myJsonDataPtr++;	//*	skip the '"'
			cc	=	0;
			while (	(*myJsonDataPtr != '"') &&
					(*myJsonDataPtr != 0) &&		//*	end of line
					(cc < (maxLen -1)))
			{
				currChar			=	*myJsonDataPtr;
				if (currChar == '\\')
				{
					myJsonDataPtr++;
					currChar		=	*myJsonDataPtr;
					myJsonDataPtr++;
					switch (currChar)
					{
						//*	Allowed escaped symbols
						case 0x22:							//*	"
							parsedString[cc++]	=	'"';
							break;

						case 0x5c:							//*	"\"
							parsedString[cc++]	=	'\\';
							break;

						case '/':
							parsedString[cc++]	=	'/';
							break;

						case 'b':
							parsedString[cc++]	=	0x08;	//*	backspace
							break;

						case 'f':
							parsedString[cc++]	=	0x0c;	//*	formfeed
							break;

						case 'r':
							parsedString[cc++]	=	0x0d;	//*	return
							break;

						case 'n':
							parsedString[cc++]	=	0x0a;	//*	new line (linefeed)
							break;

						case 't':
							parsedString[cc++]	=	0x09;	//*	tab
							break;


						//*	Allows escaped symbol \uXXXX
						case 'u':
							//*	this should be exactly 4 more chars
							uuCnt	=	0;
							uuChar	=	0;
							while ((uuCnt < 4) && (*myJsonDataPtr != 0))
							{
								uuChar		=	uuChar << 4;	//*	shift the current value to the left 4 bits
								currChar	=	*myJsonDataPtr;
								//*	make sure its a HEX char
								if ((currChar >= 0x30) && (currChar <= 0x39))	//*	numeric (0-9)
								{
									uuChar	+=	(currChar & 0x0f);
								}
								else if ((currChar >= 0x41) && (currChar <= 0x4F))	//*	hex (A-F)
								{
									uuChar	+=	(9 + (currChar & 0x0f));
								}
								else if ((currChar >= 0x61) && (currChar <= 0x6F))		//*	hex (A-F)
								{
									uuChar	+=	(9 + (currChar & 0x0f));
								}
								myJsonDataPtr++;
								uuCnt++;
							}
							//*	TODO: Finish the proper uuencode sequence
							parsedString[cc++]	=	uuChar;	//*	save the char
							break;

						//*	Unexpected symbol
						default:
							break;
					}
				}
				else
				{
					parsedString[cc++]	=	currChar;
					myJsonDataPtr++;
				}
				parsedString[cc]	=	0;	//*	null terminate (I know, it does it every time)
			}
			if (*myJsonDataPtr == '"')
			{
				//*	skip the closing quote
				myJsonDataPtr++;
			}
		}
		else if (theFirstChar == '[')
		{
			//*	noting for now
		}
		else if (theFirstChar == 0)
		{
			//	empty string, dont do anything
		}
		else
		{
			//*	we have a number or boolean value
			cc	=	0;
			//*	stop on space, control char or coma
			while (	(*myJsonDataPtr > 0x20) &&
					(*myJsonDataPtr != ',') &&
					(*myJsonDataPtr != ']') &&
					(*myJsonDataPtr != '}') &&
					(cc < (maxLen -1)))
			{
				parsedString[cc++]	=	*myJsonDataPtr;
				parsedString[cc]	=	0;	//*	null terminate (I know, it does it every time)
				myJsonDataPtr++;
			}

		}
	}
	CONSOLE_DEBUG_W_STR("parsedString\t\t=", parsedString);
	return(myJsonDataPtr);
}



//**************************************************************************************
static void	SJP_Private_ClearTokenList(SJP_token_t	*tokenList, const int	maxTokens)
{
short	tknIdx;

	if (tokenList != NULL)
	{
		for (tknIdx=0; tknIdx<maxTokens; tknIdx++)
		{
			memset(tokenList[tknIdx].keyword,		0, kSJP_MaxKeyLen);
			memset(tokenList[tknIdx].valueString,	0, kSJP_MaxValueLen);
		}
	}
}


//**************************************************************************************
//*	this is used as an indicator as to which data block we are currently working on.
enum
{
	kDataBlock_None		=	0,
	kDataBlock_Header,
	kDataBlock_Data,
	kDataBlock_Errors,
};

//**************************************************************************************
//*	returnCode < 0		Error code
//*	returnCode == 0		No error
//**************************************************************************************
int	SJP_ParseData(	SJP_Parser_t	*theParser,
					const char 		*jsonDataPtr)
{
int			returnCode;
int			dataLen;
int			tokenIdx;
char		theChar;
char		*myJsonDataPtr;
char		myKeywordString[kSJP_MaxValueLen];
char		myValueString[kSJP_MaxValueLen];
bool		headerFound;
SJP_token_t	*myCurrTokenList;
int			myCurrTokenMax;
int			myCurrDataBlock;
bool		arrayInProcess;
bool		masterArrayFlag;

#if defined(_DEBUG_PARSER_) || defined(_DEBUG_ARRAY_)
	char	debugString[64];
#endif

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("jsonDataPtr\t=", jsonDataPtr);

	returnCode		=	0;
	headerFound		=	false;
	arrayInProcess	=	false;
	masterArrayFlag	=	false;

	myCurrTokenList	=	NULL;
	myCurrTokenMax	=	0;
	myCurrDataBlock	=	kDataBlock_None;


	if ((theParser != NULL) && (jsonDataPtr != NULL))
	{
		memset(myKeywordString,	0,	kSJP_MaxValueLen);
		memset(myValueString,	0,	kSJP_MaxValueLen);

		theParser->tokenCount_Hdr	=	0;
		theParser->tokenCount_Data	=	0;
		theParser->tokenCount_Errs	=	0;

		//*	clear the token lists
		SJP_Private_ClearTokenList(theParser->headerList,	kSJP_MaxTokens_Hdr);
		SJP_Private_ClearTokenList(theParser->dataList,		kSJP_MaxTokens_Data);
		SJP_Private_ClearTokenList(theParser->errorList,	kSJP_MaxTokens_Errs);

		tokenIdx		=	0;
		dataLen			=	strlen(jsonDataPtr);
		myJsonDataPtr	=	(char *)jsonDataPtr;

		//*	set some defaults first
		myCurrDataBlock	=	kDataBlock_Data;
		myCurrTokenList	=	theParser->dataList;
		myCurrTokenMax	=	kSJP_MaxTokens_Data;
		tokenIdx		=	0;


		//*	we will be manipulating the data ptr as we go
		while (myJsonDataPtr < (jsonDataPtr + dataLen))
		{
			theChar	=	*myJsonDataPtr;

		#ifdef _DEBUG_PARSER_
			debugString[0]	=	0x30 + (theChar >> 4);
			debugString[1]	=	0x30 + (theChar & 0x0f);
			if (debugString[1] > 0x39)
			{
				debugString[1]	+=	7;
			}
			debugString[2]	=	0x20;
			if (theChar >= 0x20)
			{
				debugString[3]	=	theChar;
			}
			else
			{
				debugString[3]	=	'_';
			}
			debugString[4]	=	0;
//			CONSOLE_DEBUG_W_STR("theChar\t\t=", debugString);
		#endif

			switch(theChar)
			{
				case ',':
					if ((strlen(myCurrTokenList[tokenIdx].keyword) > 0) || (strlen(myCurrTokenList[tokenIdx].valueString) > 0))
					{
					#ifdef _DEBUG_PARSER_
						sprintf(debugString, "New entry: kw=%-10s\tval=%s", myCurrTokenList[tokenIdx].keyword, myCurrTokenList[tokenIdx].valueString);
						CONSOLE_DEBUG(debugString);
					#endif
						tokenIdx++;
					}
					if (arrayInProcess)
					{
						myJsonDataPtr++;
						CONSOLE_DEBUG("Array in progress");
						myJsonDataPtr	=	SJP_Private_GetString(myJsonDataPtr, myKeywordString, kSJP_MaxValueLen);
						CONSOLE_DEBUG_W_STR("myKeywordString\t=", myKeywordString);
						if (tokenIdx < myCurrTokenMax)
						{
							strcpy(myCurrTokenList[tokenIdx].valueString, myKeywordString);
							if ((strlen(myCurrTokenList[tokenIdx].keyword) > 0) || (strlen(myCurrTokenList[tokenIdx].valueString) > 0))
							{
							#ifdef _DEBUG_PARSER_
								sprintf(debugString, "New entry: kw=%-10s\tval=%s", myCurrTokenList[tokenIdx].keyword, myCurrTokenList[tokenIdx].valueString);
								CONSOLE_DEBUG(debugString);
							#endif
								tokenIdx++;
							}
						}
						break;
					}
					//else
					//*	FALL THROUGH......
					CONSOLE_DEBUG("FALL THROUGH......");
					CONSOLE_DEBUG_W_STR("myKeywordString\t=",	myKeywordString);
					CONSOLE_DEBUG_W_STR("myValueString\t=",		myValueString);

				case '{':
					myJsonDataPtr++;
					myJsonDataPtr	=	SJP_Private_GetString(myJsonDataPtr, myKeywordString, kSJP_MaxValueLen);
					ToUpperStr(myKeywordString);

					if ((myCurrTokenList != NULL) && (tokenIdx < myCurrTokenMax))
					{
						if (arrayInProcess)
						{
							strcpy(myCurrTokenList[tokenIdx].valueString, myKeywordString);
						}
						else
						{
							strcpy(myCurrTokenList[tokenIdx].keyword, myKeywordString);
						}
					}
					else if ((myCurrTokenList != NULL) && (tokenIdx >= myCurrTokenMax))
					{
						returnCode	=	SJP_ExceededTokenCnt;
					}
					else
					{
						CONSOLE_DEBUG("No data block found yet");
					}


					CONSOLE_DEBUG_W_STR("Token\t\t=", myKeywordString);
//					CONSOLE_DEBUG_W_STR("Next str\t=", myJsonDataPtr);
					if (strcmp(myKeywordString, "HDR") == 0)
					{
						myCurrDataBlock	=	kDataBlock_Header;
						headerFound		=	true;
						myCurrTokenList	=	theParser->headerList;
						myCurrTokenMax	=	kSJP_MaxTokens_Hdr;
						tokenIdx		=	0;
						strcpy(myCurrTokenList[tokenIdx].keyword, myKeywordString);
					}
					else if (strcmp(myKeywordString, "DATA") == 0)
					{
						myCurrDataBlock	=	kDataBlock_Data;
						myCurrTokenList	=	theParser->dataList;
						myCurrTokenMax	=	kSJP_MaxTokens_Data;
						tokenIdx		=	0;
						strcpy(myCurrTokenList[tokenIdx].keyword, myKeywordString);
					}
					else if (strcmp(myKeywordString, "ERROR") == 0)
					{
						myCurrDataBlock	=	kDataBlock_Errors;
						myCurrTokenList	=	theParser->errorList;
						myCurrTokenMax	=	kSJP_MaxTokens_Errs;
						tokenIdx		=	0;
						strcpy(myCurrTokenList[tokenIdx].keyword, myKeywordString);
					}
					else
					{
						CONSOLE_DEBUG_W_STR("myKeywordString\t=",	myKeywordString);
						CONSOLE_DEBUG_W_STR("myValueString\t=",		myValueString);
					}
					break;

				case ':':
					myJsonDataPtr++;
					arrayInProcess	=	false;
					myJsonDataPtr	=	SJP_Private_GetString(myJsonDataPtr, myValueString, kSJP_MaxValueLen);
				#ifdef _DEBUG_PARSER_
					CONSOLE_DEBUG_W_STR("myKeywordString\t=",	myKeywordString);
					CONSOLE_DEBUG_W_STR("myValueString\t=",		myValueString);
				//	CONSOLE_DEBUG_W_STR("myJsonDataPtr\t=",		myJsonDataPtr);
				#endif
					if ((myCurrTokenList != NULL) && (tokenIdx < myCurrTokenMax))
					{
						strcpy(myCurrTokenList[tokenIdx].keyword,		myKeywordString);
						strcpy(myCurrTokenList[tokenIdx].valueString,	myValueString);

						if ((strlen(myCurrTokenList[tokenIdx].keyword) > 0) || (strlen(myCurrTokenList[tokenIdx].valueString) > 0))
						{
						#ifdef _DEBUG_PARSER_
							sprintf(debugString, "New entry: kw=%-10s\tval=%s", myCurrTokenList[tokenIdx].keyword, myCurrTokenList[tokenIdx].valueString);
							CONSOLE_DEBUG(debugString);
						#endif
							tokenIdx++;
						}
					}
					else if ((myCurrTokenList != NULL) && (tokenIdx >= myCurrTokenMax))
					{
						returnCode	=	SJP_ExceededTokenCnt;
					}
				#ifdef _DEBUG_PARSER_
					CONSOLE_DEBUG_W_STR("myValueString\t=", myValueString);
				#endif
					//*	this checks to see if there is a block for the value of a token
					if (strcmp(myValueString, "{") == 0)
					{
						//*	back up one so that the argument gets parsed on the next round through
						myJsonDataPtr--;
					}
					break;

				case '[':
					myJsonDataPtr++;

					masterArrayFlag	=	true;
					arrayInProcess	=	true;
					if ((strlen(myCurrTokenList[tokenIdx].keyword) > 0) || (strlen(myCurrTokenList[tokenIdx].valueString) > 0))
					{
					#ifdef _DEBUG_PARSER_
						sprintf(debugString, "New entry: kw=%-10s\tval=%s", myCurrTokenList[tokenIdx].keyword, myCurrTokenList[tokenIdx].valueString);
						CONSOLE_DEBUG(debugString);
					#endif
						tokenIdx++;
					}

					if ((myCurrTokenList != NULL) && (tokenIdx < myCurrTokenMax))
					{
						strcpy(myCurrTokenList[tokenIdx].keyword, "ARRAY");
					}
					else if ((myCurrTokenList != NULL) && (tokenIdx >= myCurrTokenMax))
					{
						returnCode	=	SJP_ExceededTokenCnt;
					}
					if ((strlen(myCurrTokenList[tokenIdx].keyword) > 0) || (strlen(myCurrTokenList[tokenIdx].valueString) > 0))
					{
					#ifdef _DEBUG_PARSER_
						sprintf(debugString, "New entry: kw=%-10s\tval=%s", myCurrTokenList[tokenIdx].keyword, myCurrTokenList[tokenIdx].valueString);
						CONSOLE_DEBUG(debugString);
					#endif
						tokenIdx++;
					}

					myJsonDataPtr	=	SJP_Private_GetString(myJsonDataPtr, myKeywordString, kSJP_MaxValueLen);
					ToUpperStr(myKeywordString);


					if ((myCurrTokenList != NULL) && (tokenIdx < myCurrTokenMax))
					{
						CONSOLE_DEBUG_W_STR("myKeywordString\t=", myKeywordString);
						if (arrayInProcess)
						{
							strcpy(myCurrTokenList[tokenIdx].valueString, myKeywordString);
						}
						else
						{
							strcpy(myCurrTokenList[tokenIdx].keyword, myKeywordString);
						}
					#ifdef _DEBUG_ARRAY_
						sprintf(debugString, "New entry: kw=%-10s\tval=%s", myCurrTokenList[tokenIdx].keyword, myCurrTokenList[tokenIdx].valueString);
						CONSOLE_DEBUG(debugString);
					#endif
					}
					else if ((myCurrTokenList != NULL) && (tokenIdx >= myCurrTokenMax))
					{
						returnCode	=	SJP_ExceededTokenCnt;
					}
					break;


				case ']':

					arrayInProcess	=	false;
					masterArrayFlag	=	false;
					myJsonDataPtr++;

					if ((myCurrTokenList != NULL) && (tokenIdx < myCurrTokenMax))
					{
					#ifdef _DEBUG_ARRAY_
						CONSOLE_DEBUG("end of array");
						CONSOLE_DEBUG_W_STR("myCurrTokenList[tokenIdx].keyword", myCurrTokenList[tokenIdx].keyword);
						CONSOLE_DEBUG_W_STR("myCurrTokenList[tokenIdx].valueString", myCurrTokenList[tokenIdx].valueString);
					#endif
						if (strlen(myCurrTokenList[tokenIdx].valueString) > 0)
						{
							tokenIdx++;
						}
						strcpy(myCurrTokenList[tokenIdx].keyword, "]");
					#ifdef _DEBUG_PARSER_
						sprintf(debugString, "New entry: kw=%-10s\tval=%s", myCurrTokenList[tokenIdx].keyword, myCurrTokenList[tokenIdx].valueString);
						CONSOLE_DEBUG(debugString);
					#endif
						tokenIdx++;
					}
					break;

				case '}':
					myJsonDataPtr++;
					if (masterArrayFlag)
					{
						strcpy(myCurrTokenList[tokenIdx].keyword, "ARRAY-NEXT");
						strcpy(myCurrTokenList[tokenIdx].valueString, "");
						tokenIdx++;
					}
					break;

				//*	all other chars get skipped, i.e. space, tab, cr, lf
				default:
					myJsonDataPtr++;
					break;
			}

			//*	Update the number of tokens for this data type
			//*	I know it looks redundant but it easiest to do this every time through the loop
			switch(myCurrDataBlock)
			{

				case kDataBlock_None:
					//*	dont do anything
					break;

				case kDataBlock_Header:
					theParser->tokenCount_Hdr	=	tokenIdx;
					break;

				case kDataBlock_Data:
					theParser->tokenCount_Data	=	tokenIdx;
					break;

				case kDataBlock_Errors:
					theParser->tokenCount_Errs	=	tokenIdx;
					break;

			}
		}

		//*	did we fine a header?
		if (headerFound == false)
		{
			//*	report the fact that we did not find a header
			returnCode	=	SJP_NoHeader;
		}
	}
	else
	{
		returnCode	=	SJP_InvalidParameter;
	}

	return(returnCode);
}


//*****************************************************************************
//*	find a token in the table,
//*	stops when the there is an empty element in the table
//*	hard coded limit of 1000 entries in a table, just for code safety
//*	returns -1 if not found
//*****************************************************************************
int	SJP_FindKeyWordFromDictionary(	const char				*theKeyword,
									const SJP_Dictionary_t	*vocabList)
{
short	ii;
short	keyWordEnumValue;
char	myLowerCaseKeyWord[kSJP_MaxKeyLen + 1];

	ii	=	0;
	while ((theKeyword[ii] > 0) && (ii < kSJP_MaxKeyLen))
	{
		myLowerCaseKeyWord[ii]	=	toupper(theKeyword[ii]);
		ii++;
		myLowerCaseKeyWord[ii]	=	0;
	}

	keyWordEnumValue	=	-1;

	if (vocabList != NULL)
	{
		ii	=	0;
		while ((keyWordEnumValue < 0) && (ii < 1000) && (strlen(vocabList[ii].keyword) > 0))
		{
			if (strcmp(myLowerCaseKeyWord, vocabList[ii].keyword) == 0)
			{
				keyWordEnumValue	=	vocabList[ii].kw_enumValue;
			}
			ii++;
		}
	}
	return(keyWordEnumValue);

}


//**************************************************************************************
bool	SJP_FindKeyWordString(	const char	*keyWord,
								SJP_token_t	*tokenList,
								const short	tokenCnt,
								char		*valueString)
{
bool	foundIt;
short	ii;
char	upperCaseKeword[kSJP_MaxKeyLen];


	foundIt	=	false;
	if ((keyWord != NULL) && (tokenList != NULL))
	{
		strncpy(upperCaseKeword, keyWord, (kSJP_MaxKeyLen - 1));
		upperCaseKeword[kSJP_MaxKeyLen - 1]	=	0;
		ToUpperStr(upperCaseKeword);

		ii	=	0;
		while ((ii < tokenCnt) && (foundIt == false))
		{
			if (strcmp(upperCaseKeword, tokenList[ii].keyword) == 0)
			{
				strcpy(valueString, tokenList[ii].valueString);
				foundIt	=	true;
			}
			ii++;
		}
	}
	return(foundIt);
}

//**************************************************************************************
void	SJP_DumpJsonData(SJP_Parser_t *theParser)
{
short		ii;

	if (theParser != NULL)
	{
		printf("*********************************************\r\n");
		for (ii=0; ii<theParser->tokenCount_Data; ii++)
		{
			if (strlen(theParser->dataList[ii].keyword) > 0)
			{
				printf("%2d=%-20s\t%-20s\t\r\n",	ii,	theParser->dataList[ii].keyword,
														theParser->dataList[ii].valueString);
			}
		}
		printf("---------------------------------------------\r\n");
	}
}

#ifdef _TEST_JSON_PARSER_

//**************************************************************************************
void	ProcessJsonFile(const char *fileName)
{
FILE			*inputFilePointer;
char			fileBuffer[10000];
int				byteCount;
SJP_Parser_t	jsonParser;
int				ii;

	SJP_Init(&jsonParser);


	inputFilePointer	=	fopen(fileName, "r");
	if (inputFilePointer != NULL)
	{
		byteCount	=	fread(fileBuffer, 1, 9000, inputFilePointer);
		if (byteCount> 0)
		{
			SJP_ParseData(&jsonParser, fileBuffer);
		}
		else
		{
			printf("Read error\r\n");
		}

		fclose(inputFilePointer);


	}
	else
	{
		printf("Failed to open input file %s\r\n", fileName);
	}

	printf("\r\n");
	for (ii=0; ii<jsonParser.tokenCount_Hdr; ii++)
	{
		printf("HDR\t%s\t\t%s\r\n", 	jsonParser.headerList[ii].keyword,
										jsonParser.headerList[ii].valueString);
	}
	for (ii=0; ii<jsonParser.tokenCount_Data; ii++)
	{
		printf("DATA\t%s\t\t%s\r\n", 	jsonParser.dataList[ii].keyword,
										jsonParser.dataList[ii].valueString);
	}
	printf("tokenCount_Hdr=%d\r\n",		jsonParser.tokenCount_Hdr);
	printf("tokenCount_Data=%d\r\n",	jsonParser.tokenCount_Data);

}


//**************************************************************************************
static void	PrintHelp(const char *appName)
{
	printf("No file specified:\n");
	printf("Usage\n");
	printf("%s [-options] files\n", appName);

}


//*****************************************************************************
int	main(int argc, char *argv[])
{
short		argIdx;
short		fileCount;

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
			switch(argv[argIdx][1])
			{
				case 'h':
					PrintHelp(argv[0]);
					break;

			}
		}
		else
		{
			ProcessJsonFile(argv[argIdx]);
		}
	}


}

#endif	//	_TEST_JSON_PARSER_
