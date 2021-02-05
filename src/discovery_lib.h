//*****************************************************************************
//#include	"discovery_lib.h"

#ifndef _DISCOVERY_LIB_H_
#define	_DISCOVERY_LIB_H_


#ifndef _ARPA_INET_H
	#include	<arpa/inet.h>
#endif // _ARPA_INET_H


#ifdef __cplusplus
	extern "C" {
#endif


#ifndef _JSON_PARSE_H_
	#include	"json_parse.h"
#endif // _JSON_PARSE_H_

#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif // _ALPACA_HELPER_H_

#define	kAlpacaDiscoveryMsg	"alpacadiscovery1"


//*****************************************************************************
//*	this is a list of IP addresses
typedef struct
{
	struct sockaddr_in	deviceAddress;
	int					port;
	int					noResponseCnt;
	char				hostName[48];		//*	device name from hosts file

} TYPE_ALPACA_UNIT;


//*****************************************************************************
//*	this is a list of alpaca devices, can be more than one per IP address
typedef struct
{
	bool				validEntry;
	struct sockaddr_in	deviceAddress;
	int					port;
	char				hostName[64];
	TYPE_DEVICETYPE		deviceTypeEnum;
	char				deviceTypeStr[32];
	char				deviceNameStr[64];
	char				versionString[64];
	int					alpacaDeviceNum;
	int					notSeenCounter;

	char				webPrefixString[64];


} TYPE_REMOTE_DEV;

#define	kMaxAlpacaIPaddrCnt	24

#define	kMaxDeviceListCnt	48


extern	TYPE_REMOTE_DEV	gAlpacaDiscoveredList[kMaxDeviceListCnt];
extern	int				gAlpacaDiscoveredCnt;


bool	SetupBroadcast(void);
int		SendAlpacaQueryBroadcast(void);
void	ExtractDevicesFromJSON(SJP_Parser_t *jsonParser, TYPE_ALPACA_UNIT *theDevice);
void	ReadExternalIPlist(void);


#ifdef __cplusplus
}
#endif


#endif		//	_DISCOVERY_LIB_H_

