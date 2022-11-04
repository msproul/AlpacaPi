//*****************************************************************************
//*	Dec  6,	2021	<MLS> Increased max alpaca device count to 75
//*****************************************************************************
//#include	"discovery_lib.h"

#ifndef _DISCOVERY_LIB_H_
#define	_DISCOVERY_LIB_H_


#ifndef _ARPA_INET_H
	#include	<arpa/inet.h>
#endif // _ARPA_INET_H



#ifndef _JSON_PARSE_H_
	#include	"json_parse.h"
#endif // _JSON_PARSE_H_

#ifndef _ALPACA_HELPER_H_
	#include	"alpacadriver_helper.h"
#endif // _ALPACA_HELPER_H_

#define	kAlpacaDiscoveryMsg	"alpacadiscovery1"

#define	kMaxCPUtempEntries	(((24 * 60) / 2) + 10)

//*****************************************************************************
//*	this is a list of IP addresses
typedef struct
{
	struct sockaddr_in	deviceAddress;
	int					port;
	int					noResponseCnt;
	char				hostName[48];		//*	device name from hosts file


	//*	for use by discovery thread for keeping track of responses
	int					queryOKcnt;
	int					queryERRcnt;
//	int					ignoredCount;
	bool				currentlyActive;
	char				versionString[64];

	//-------------------------------------------------------------------------------
	//*	This data only works with AlpacaPi drivers, beyond the scope of normal Alpaca
	bool				upTimeValid;
	int					upTimeDays;

	bool				cpuTempValid;
	double				cpuTemp_DegF;
	double				cpuTemp_DegF_max;
	double				cpuTempLog[kMaxCPUtempEntries];

} TYPE_ALPACA_UNIT;


//*****************************************************************************
//*	this is a list of alpaca devices, can be more than one per IP address
typedef struct
{
	bool				validEntry;
	bool				onLine;
	struct sockaddr_in	deviceAddress;
	int					port;
	char				hostName[64];
	TYPE_DEVICETYPE		deviceTypeEnum;
	char				deviceTypeStr[32];
	char				deviceNameStr[64];
	char				versionString[64];
	int					alpacaDeviceNum;
	int					interfaceVersion;
	int					notSeenCounter;

//	char				webPrefixString[64];


} TYPE_REMOTE_DEV;

//*	this is a list of IP addresses
#define	kMaxAlpacaIPaddrCnt	32

#define	kMaxAlpacaDeviceCnt	100


#ifdef __cplusplus
	extern "C" {
#endif



//*	this is a list of alpaca devices, can be more than one per IP address
extern	TYPE_REMOTE_DEV	gAlpacaDiscoveredList[];
extern	int				gAlpacaDiscoveredCnt;

int		FindDeviceInList(TYPE_REMOTE_DEV *theDevice, TYPE_REMOTE_DEV *theList, int maxDevices);

bool	SetupBroadcast(void);
int		SendAlpacaQueryBroadcast(void);
void	ExtractDevicesFromJSON(SJP_Parser_t *jsonParser, TYPE_ALPACA_UNIT *theDevice);
void	ReadExternalIPlist(void);


#ifdef __cplusplus
}
#endif


#endif		//	_DISCOVERY_LIB_H_

