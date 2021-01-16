//*****************************************************************************
//#include	"HostNames.h"



#ifdef __cplusplus
	extern "C" {
#endif


bool	LookupNameFromString(const char *ipAddrStr, char *returnedHostName);

bool	LookupNameFromIPaddr(uint32_t ipAddrNum, char *returnedHostName);


#ifdef __cplusplus
}
#endif
