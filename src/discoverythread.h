//*****************************************************************************
//#include	"discoverythread.h"


#ifndef _DISCOVERY_THREAD_H_
#define	_DISCOVERY_THREAD_H_


#ifndef _JSON_PARSE_H_
//-	#include	"json_parse.h"
#endif // _JSON_PARSE_H_

#ifdef __cplusplus
	extern "C" {
#endif


int StartDiscoveryListenThread(int alpacaListenPort);
int StartDiscoveryQuerryThread(void);

int	GetMySubnetNumber(void);

//-bool	GetJsonResponse(	struct sockaddr_in	*deviceAddress,
//-							const int			port,
//-							const char			*sendData,
//-							SJP_Parser_t		*jsonParser);


#ifdef __cplusplus
}
#endif


#endif		//	_DISCOVERY_THREAD_H_
