//*****************************************************************************
//#include	"discoverythread.h"


#ifndef _DISCOVERY_THREAD_H_
#define	_DISCOVERY_THREAD_H_

#ifndef _DISCOVERY_LIB_H_
	#include	"discovery_lib.h"
#endif


#ifndef _JSON_PARSE_H_
//-	#include	"json_parse.h"
#endif

//*	this is a list of IP addresses
extern	TYPE_ALPACA_UNIT	gAlpacaUnitList[];
extern	int					gAlpacaUnitCnt;


//*	this is a list of alpaca devices, can be more than one per IP address
extern TYPE_REMOTE_DEV		gRemoteList[];
extern int					gRemoteCnt;
extern bool					gDiscoveryThreadIsRunning;
extern bool					gDiscoveryThreadKeepRunning;


#ifdef __cplusplus
	extern "C" {
#endif


int		StartDiscoveryListenThread(const int alpacaListenPort);
int		StartDiscoveryQuerryThread(void);
void	Discovery_ClearIPAddrList(void);
void	ResetExternalIPaddress(void);
void	WakeUpDiscoveryThread(void);

//-bool	GetJsonResponse(	struct sockaddr_in	*deviceAddress,
//-							const int			port,
//-							const char			*sendData,
//-							SJP_Parser_t		*jsonParser);


#ifdef __cplusplus
}
#endif


#endif		//	_DISCOVERY_THREAD_H_
