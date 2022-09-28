//*****************************************************************************
//#include	"readconfigfile.h"


#ifdef __cplusplus
	extern "C" {
#endif

#define	kMaxKeyWordLen	64
#define	kMaxVakyeStrLen	256

//*****************************************************************************
typedef struct
{
	char	keyword[kMaxKeyWordLen];
	int		enumValue;
} TYPE_KEYWORDS;


//*	call back function for processing config entry
typedef void (ProcessConfigEntry)(const char *keyword, const char *value);


//*	returns # of processed lines
//*	-1 means failed to open config file
int	ReadGenericConfigFile(	const char			*configFilePath,
							const char			separterChar,
							ProcessConfigEntry *configCallBack);
//*	returns -1 if keyword not found
int		FindKeywordFromTable(const char *keyword, const TYPE_KEYWORDS *keywordTable);

#ifdef __cplusplus
}
#endif
