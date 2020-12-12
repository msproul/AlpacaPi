//*****************************************************************************
//*	json_parse.h
//*		simple json parser
//*		written by Mark Sproul mark.l.sproul.civ@mail.mil
//*		unlimited use rights, modify and use as you wish
//*		Please send updates and bug fixes to the above email address
//*
//*****************************************************************************
//*	Oct 16,	2019	<MLS> Changed some int's to short's to save memory
//*****************************************************************************
//#include	"json_parse.h"


#ifndef _JSON_PARSE_H_
#define	_JSON_PARSE_H_

#ifndef _STDBOOL_H
	#include	<stdbool.h>
#endif

#ifdef __cplusplus
	extern "C" {
#endif


#define	kJSONparse_Ver_String		"JSONparse V1.0.0"
#define	kJSONparse_Ver_NumberString	"1.0.0"
#define	kJSONparse_Ver_long			100


#define	kSJP_MaxKeyLen		64
#define	kSJP_MaxValueLen	256


//*	these number can be adjusted as needed
#define	kSJP_MaxTokens_Hdr	15
#define	kSJP_MaxTokens_Data	200
#define	kSJP_MaxTokens_Errs	5




//*****************************************************************************
//*	error list
//*	the errors must be negative numbers
//*****************************************************************************
enum
{
	SJP_InvalidParameter	=	-100,
	SJP_ExceededTokenCnt,
	SJP_NoHeader,
};

//*****************************************************************************
//*	token list
typedef struct
{
	char	keyword[kSJP_MaxKeyLen];
	char	valueString[kSJP_MaxValueLen];

} SJP_token_t;


//*****************************************************************************
//*	parser data structure, so that we don't have to use globals
typedef struct
{
	SJP_token_t		headerList[kSJP_MaxTokens_Hdr];
	short			tokenCount_Hdr;

	SJP_token_t		dataList[kSJP_MaxTokens_Data];
	short			tokenCount_Data;


	SJP_token_t		errorList[kSJP_MaxTokens_Errs];
	short			tokenCount_Errs;


} SJP_Parser_t;

extern	SJP_Parser_t		gJsonParser;

//*****************************************************************************
//*	Json dictionary table structure
//*	enum values should be positive
//*	keyword values MUST be all UPPER case
//*****************************************************************************
typedef struct
{
	char	keyword[kSJP_MaxKeyLen];
	short	kw_enumValue;


} SJP_Dictionary_t;


//*	routines for parsing the data
void	SJP_Init(SJP_Parser_t *theParserDataStruct);
long	SJP_GetVersion(void);
int		SJP_ParseData(SJP_Parser_t *theParser, const char  *jsonDataPtr);
bool	SJP_FindKeyWordString(const char *keyWord, SJP_token_t *tokenList, const short tokenCnt, char *valueString);
void	SJP_DumpJsonData(SJP_Parser_t *theParser);


//*	routines for processing the data
int		SJP_FindKeyWordFromDictionary(const char *theKeyword, const SJP_Dictionary_t *vocabList);



#ifdef __cplusplus
}
#endif

#endif	//	_JSON_PARSE_H_


