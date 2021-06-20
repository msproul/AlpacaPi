//*****************************************************************************
//*	Alpaca discovery thread
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Oct 20,	2019	<MLS> Added DiscoveryThread()
//*	Jan 23,	2020	<MLS> Started on live discovery of other devices
//*	Jan 24,	2020	<MLS> Retrieving of remote pressure and humidity working
//*	Jan 27,	2020	<MLS> No longer adding myself to remote device list
//*	Feb 11,	2020	<MLS> Discovery thread now keeps track if a device goes offline
//*	Mar 19,	2020	<MLS> Added GetMySubnetNumber()
//*	Jan 14,	2021	<MLS> Apparently the DISCOVERY MESSAGE has changed, fixed
//*	Jan 14,	2021	<MLS> Discovery protocol now working with ASCOM Device Hub
//*	Jan 17,	2021	<MLS> Added external IP list to discovery thread
//*	Feb  4,	2021	<MLS> Rearranged close logic in GetJsonResponse()
//*	Feb  7,	2021	<MLS> Added Discovery_ClearIPAddrList()
//*	Feb  7,	2021	<MLS> Fixed bug that did not discover 2 listen ports on same IP addrr
//*	Mar  9,	2021	<MLS> Moved FindDeviceInList() to discoverytjread.c
//*****************************************************************************


#include	<stdio.h>
#include	<stdlib.h>
#include	<unistd.h>
#include	<string.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<arpa/inet.h>
#include	<netinet/in.h>
#include	<errno.h>
#include	<pthread.h>
#include	<ifaddrs.h>


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"


//*	for debugging rouge IP address
//#define	LOG_DISCOVERED_IP_ADDRS


#ifndef SO_REUSEPORT
	#define	SO_REUSEPORT 1
#endif

#include	"json_parse.h"
#include	"discoverythread.h"
#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#include	"obsconditions_globals.h"
#include	"HostNames.h"

#define kAlpacaDiscoveryPORT	32227

#define	kReceiveBufferSize	1550



pthread_t	gDiscoveryListenThreadID;
pthread_t	gDiscoveryFindThreadID;

//#define	kMaxDeviceListCnt	32

//*	this is a list of IP addresses
TYPE_ALPACA_UNIT	gAlpacaUnitList[kMaxAlpacaIPaddrCnt];
int					gAlpacaUnitCnt	=	0;


//*	this is a list of alpaca devices, can be more than one per IP address
TYPE_REMOTE_DEV		gRemoteList[kMaxDeviceListCnt];
int					gRemoteCnt		=	0;

bool				gDiscoveryThreadIsRunning	=	false;
bool				gDiscoveryThreadKeepRunning	=	true;

static	int			gBroadcastSock;
static	uint32_t	gMyIPaddress		=	0;
static	int			gAlpacaListenPort	=	9999;


static void		*LookForAlpacaDevicesThread(void *arg);
static void		ReadExternalIPlist_FromThread(void);
static bool		gNeedToReadExternalList	=	true;	//*	we only need to do this once


//*****************************************************************************
void	Discovery_ClearIPAddrList(void)
{
int	iii;

	gAlpacaUnitCnt	=	0;
	gRemoteCnt		=	0;
	for (iii=0; iii<kMaxAlpacaIPaddrCnt; iii++)
	{
		memset(&gAlpacaUnitList[iii], 0, sizeof(TYPE_ALPACA_UNIT));
	}

	for (iii=0; iii<kMaxDeviceListCnt; iii++)
	{
		memset(&gRemoteList[iii], 0, sizeof(TYPE_REMOTE_DEV));
	}

	gAlpacaUnitCnt	=	0;
}

//*****************************************************************************
static void	*DiscoveryListenThread(void *arg)
{
int					mySocket;
unsigned int		fromlen;
int					bytesRead;
int					bytesSent;
struct sockaddr_in	serverAddress, fromAddress;
char				readBuf[1024];
char				responseBuff[128];
int					socketOption;
int					setSocketRtnCde;
bool				validDiscoveryRequest;
char				ipAddrSt[48];


	CONSOLE_DEBUG("Starting discovery listen thread");

	mySocket	=	socket(AF_INET, SOCK_DGRAM, 0);
	if (mySocket >= 0)
	{
		memset(&serverAddress, '\0', sizeof(struct sockaddr_in));
		serverAddress.sin_family			=	AF_INET;
		serverAddress.sin_family			=	AF_INET;
		serverAddress.sin_addr.s_addr		=	INADDR_ANY;
		serverAddress.sin_port				=	htons(kAlpacaDiscoveryPORT);

		socketOption	=	1;
		setSocketRtnCde	=	setsockopt(	mySocket,
										SOL_SOCKET,
										(SO_REUSEPORT),
										(void *)&socketOption,
										sizeof(int));

		if (setSocketRtnCde < 0)
		{
			perror("setsockopt(SO_REUSEPORT) failed");
		}
		if (bind(mySocket, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in)) < 0)
		{
			perror("binding");
		}
		while (1)
		{
			fromlen		=	sizeof(struct sockaddr_in);
			bytesRead	=	recvfrom(	mySocket,
										readBuf,
										1024,
										0,
										(struct sockaddr *)&fromAddress,
										&fromlen);
			if (bytesRead >= 0)
			{
				readBuf[bytesRead]	=	0;
//				printf("%s\r\n", readBuf);

				validDiscoveryRequest	=	false;
				//*	this was the original discovery query
				if (strncasecmp(readBuf,	"alpaca discovery", 16) == 0)
				{
					validDiscoveryRequest	=	true;

					CONSOLE_DEBUG_W_STR("Old style discovery request\t=", readBuf);
					inet_ntop(AF_INET, &(fromAddress.sin_addr), ipAddrSt, INET_ADDRSTRLEN);
					CONSOLE_DEBUG_W_STR("From\t=", ipAddrSt);
				}
				if (strncasecmp(readBuf,	"alpacadiscovery",  15) == 0)
				{
					validDiscoveryRequest	=	true;
				}
				//*	double check just to make sure
				if ((validDiscoveryRequest == false) && (strncasecmp(readBuf, "alpaca", 6) == 0))
				{
					validDiscoveryRequest	=	true;

					CONSOLE_DEBUG_W_STR("Incomplete discovery request\t=", readBuf);
					inet_ntop(AF_INET, &(fromAddress.sin_addr), ipAddrSt, INET_ADDRSTRLEN);
					CONSOLE_DEBUG_W_STR("From\t=", ipAddrSt);
				}
				if (validDiscoveryRequest)
				{
					sprintf(responseBuff, "{\"alpacaport\": %d}", gAlpacaListenPort);

					bytesSent	=	sendto(mySocket, responseBuff, strlen(responseBuff), 0, (struct sockaddr *)&fromAddress, fromlen);
					if (bytesSent < 0)
					{
						perror("sendto");
					}
				}
				else
				{
					CONSOLE_DEBUG_W_STR("Unknown request====", readBuf);
				}
			}
			else if (bytesRead < 0)
			{
				perror("recvfrom");

			}
		}
	}
	else
	{
		perror("Opening socket");
	}
	return(NULL);
}

//*****************************************************************************
static void	BumpNotSeenCounter(void)
{
int	iii;

//	CONSOLE_DEBUG(__FUNCTION__);
	for (iii=0; iii<gRemoteCnt; iii++)
	{
		gRemoteList[iii].notSeenCounter	+=	1;
	}
}


//*****************************************************************************
static void	UpdateRemoteList(TYPE_REMOTE_DEV *newRemoteDevice)
{
int		iii;
bool	newDevice;

//	CONSOLE_DEBUG(__FUNCTION__);
	newDevice	=	true;

//	if (newRemoteDevice->port != 6800)
//	{
//	char	ipString[32];
//	char	urlString[128];
//		inet_ntop(AF_INET, &newRemoteDevice->deviceAddress.sin_addr, ipString, INET_ADDRSTRLEN);
//		sprintf(urlString, "%s:%d\t%s", ipString, newRemoteDevice->port, newRemoteDevice->deviceTypeStr);
//		CONSOLE_DEBUG(urlString);
//
//		CONSOLE_DEBUG_W_STR("newRemoteDevice->deviceTypeStr\t=",	newRemoteDevice->deviceTypeStr);
//		CONSOLE_DEBUG_W_STR("newRemoteDevice->deviceNameStr\t=",	newRemoteDevice->deviceNameStr);
//		CONSOLE_DEBUG_W_NUM("newRemoteDevice->alpacaDeviceNum\t=",	newRemoteDevice->alpacaDeviceNum);
//	}

	//*	dont add myself to the list
	if (newRemoteDevice->deviceAddress.sin_addr.s_addr == gMyIPaddress)
	{
		CONSOLE_DEBUG("Leave me out of the list");
//		CONSOLE_DEBUG_W_STR("newRemoteDevice->deviceTypeStr\t=",	newRemoteDevice->deviceTypeStr);
//		CONSOLE_DEBUG_W_STR("newRemoteDevice->deviceNameStr\t=",	newRemoteDevice->deviceNameStr);
		newDevice	=	false;
	}
	else
	{
		//*	look to see if it is already in the list
		for (iii=0; iii<gRemoteCnt; iii++)
		{
			if (	(newRemoteDevice->deviceAddress.sin_addr.s_addr ==	gRemoteList[iii].deviceAddress.sin_addr.s_addr)
				&&	(newRemoteDevice->port 							==	gRemoteList[iii].port)
				&&	(newRemoteDevice->alpacaDeviceNum				==	gRemoteList[iii].alpacaDeviceNum)
				&&	(strcmp(newRemoteDevice->deviceTypeStr,			gRemoteList[iii].deviceTypeStr) == 0)
				&&	(strcmp(newRemoteDevice->deviceNameStr,			gRemoteList[iii].deviceNameStr) == 0)
				)
			{
				gRemoteList[iii].notSeenCounter	=	0;
				newDevice						=	false;
				break;
			}
		}
	}

	if (newDevice)
	{
		if (gRemoteCnt < kMaxDeviceListCnt)
		{
			gRemoteList[gRemoteCnt]					=	*newRemoteDevice;
			gRemoteList[gRemoteCnt].notSeenCounter	=	0;
			gRemoteList[gRemoteCnt].deviceTypeEnum	=	FindDeviceTypeByString(gRemoteList[gRemoteCnt].deviceTypeStr);

			//*	lookup the host name

			gRemoteCnt++;
		}
	}
	else
	{
	//	CONSOLE_DEBUG_W_STR("Ignoring device\t=",	newRemoteDevice->deviceNameStr);
	}
}


/*
{"Value":[{	"DeviceName":"ASCOM Simulator SafetyMonitor Driver",
			"DeviceType":"SafetyMonitor",
			"DeviceNumber":0,
			"UniqueID":"7B79C9E6-6338-41E5-89DE-4B85EAB47953"},
			{
			"DeviceName":"ASCOM Observing Conditions Hub (OCH)",
			"DeviceType":"ObservingConditions",
			"DeviceNumber":0,
			"UniqueID":"4BDFE843-5E27-4983-816A-45FF3F92914C"},
			{
			"DeviceName":"Dome Simulator .NET",
			"DeviceType":"Dome",
			"DeviceNumber":0,
			"UniqueID":"BEAD123F-518A-472F-BEBD-17CABA2994A8"},
			{
			"DeviceName":"Simulator",
			"DeviceType":"Telescope",
			"DeviceNumber":0,
			"UniqueID":"8CBF0B5B-63EB-4B79-AF6D-A05FD7805A98"},
			{
			"DeviceName":"ASCOM Simulator Focuser Driver",
			"DeviceType":"Focuser",
			"DeviceNumber":0,
			"UniqueID":"596C180F-8049-47ED-8D03-2D59237A6198"},
			{
			"DeviceName":"Camera V2 simulator",
			"DeviceType":"Camera",
			"DeviceNumber":0,
			"UniqueID":"AE6042D5-161F-4DF3-B9C8-85D37E0CAEB0"}
			],
			"ClientTransactionID":0,
			"ServerTransactionID":105682,
			"ErrorNumber":0,
			"ErrorMessage":""}

*/


//*****************************************************************************
void	ExtractDevicesFromJSON(SJP_Parser_t *jsonParser, TYPE_ALPACA_UNIT *theDevice)
{
TYPE_REMOTE_DEV	myRemoteDevice;
int				ii;
char			myVersionString[64];

//	CONSOLE_DEBUG(__FUNCTION__);
	memset(&myRemoteDevice, 0, sizeof(TYPE_REMOTE_DEV));
	memset(myVersionString, 0, sizeof(myVersionString));

	for (ii=0; ii<jsonParser->tokenCount_Data; ii++)
	{
		if (strcasecmp(jsonParser->dataList[ii].keyword, "VERSION") == 0)
		{
			strcpy(myVersionString,				jsonParser->dataList[ii].valueString);
			strcpy(theDevice->versionString,	jsonParser->dataList[ii].valueString);
		}
		else if (strcasecmp(jsonParser->dataList[ii].keyword, "DEVICETYPE") == 0)
		{
			strcpy(myRemoteDevice.deviceTypeStr, jsonParser->dataList[ii].valueString);
		}
		else if (strcasecmp(jsonParser->dataList[ii].keyword, "DEVICENAME") == 0)
		{
			strcpy(myRemoteDevice.deviceNameStr, jsonParser->dataList[ii].valueString);
		}
		else if (strcasecmp(jsonParser->dataList[ii].keyword, "DEVICENUMBER") == 0)
		{
			myRemoteDevice.alpacaDeviceNum	=	atoi(jsonParser->dataList[ii].valueString);
		}

		//------------------------------------
		if (strcasecmp(jsonParser->dataList[ii].keyword, "ARRAY-NEXT") == 0)
		{
			myRemoteDevice.deviceAddress	=	theDevice->deviceAddress;
			myRemoteDevice.port				=	theDevice->port;
			strcpy(myRemoteDevice.hostName, theDevice->hostName);
			strcpy(myRemoteDevice.versionString, myVersionString);
			UpdateRemoteList(&myRemoteDevice);

			memset(&myRemoteDevice, 0, sizeof(TYPE_REMOTE_DEV));

		}
	}
}

//*****************************************************************************
static int	SetSocketTimeouts(int socket_desc, int timeOutSecs)
{
struct timeval		timeoutLength;
int					setOptRetCode;

	//*	set a timeout
	timeoutLength.tv_sec	=	timeOutSecs;
	timeoutLength.tv_usec	=	0;
	setOptRetCode			=	setsockopt(	socket_desc,
											SOL_SOCKET,
											SO_RCVTIMEO,
											&timeoutLength,
											sizeof(timeoutLength));

	if (setOptRetCode < 0)
	{
		perror("setsockopt(SO_RCVTIMEO) failed");
	}
	setOptRetCode			=	setsockopt(	socket_desc,
											SOL_SOCKET,
											SO_SNDTIMEO,
											&timeoutLength,
											sizeof(timeoutLength));
	if (setOptRetCode < 0)
	{
		perror("setsockopt(SO_SNDTIMEO) failed");
	}
	return(setOptRetCode);
}

//*****************************************************************************
static bool	GetJsonResponse(	struct sockaddr_in	*deviceAddress,
								const int			port,
								const char			*sendData,
								SJP_Parser_t		*jsonParser)
{
bool				validData;
int					socket_desc;
struct sockaddr_in	remoteDev;
int					connRetCode;
int					sendRetCode;
int					closeRetCode;
int					shutdownRetCode;
int					recvByteCnt;
char				returnedData[2000];
char				xmitBuffer[2000];
int					setOptRetCode;

//*	Mar 13,	2021	<MLS> Added timeout to GetJsonResponse()
//	CONSOLE_DEBUG(__FUNCTION__);
	validData	=	false;
	socket_desc	=	socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc >= 0)
	{
		//*	set a timeout
		setOptRetCode	=	SetSocketTimeouts(socket_desc, 3);
		if (setOptRetCode != 0)
		{
			CONSOLE_DEBUG("SetSocketTimeouts() returned error");
		}

		remoteDev.sin_addr.s_addr	=	deviceAddress->sin_addr.s_addr;
		remoteDev.sin_family		=	AF_INET;
		remoteDev.sin_port			=	htons(port);
		//*	Connect to remote device
		connRetCode	=	connect(socket_desc , (struct sockaddr *)&remoteDev , sizeof(remoteDev));
		if (connRetCode >= 0)
		{
			strcpy(xmitBuffer, "GET ");
			strcat(xmitBuffer, sendData);

			strcat(xmitBuffer, " HTTP/1.1\r\n");
			strcat(xmitBuffer, "Host: 127.0.0.1:6800\r\n");
			strcat(xmitBuffer, "User-Agent: AlpacaPi\r\n");
			strcat(xmitBuffer, "Accept: text/html,application/json\r\n");
			strcat(xmitBuffer, "\r\n");

			sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , 0);
			if (sendRetCode >= 0)
			{
				recvByteCnt	=	recv(socket_desc, returnedData , 2000 , 0);
				if (recvByteCnt >= 0)
				{
					validData	=	true;
					returnedData[recvByteCnt]	=	0;
//					printf("%s\r\n", returnedData);
					SJP_Init(jsonParser);
					SJP_ParseData(jsonParser, returnedData);
//					SJP_DumpJsonData(jsonParser);
				}
			}
			shutdownRetCode	=	shutdown(socket_desc, SHUT_RDWR);
			if (shutdownRetCode != 0)
			{
				CONSOLE_DEBUG("shutdown error");
			}
		}
		else if (errno == ECONNREFUSED)
		{
		char	ipString[32];

			PrintIPaddressToString(deviceAddress->sin_addr.s_addr, ipString);
			CONSOLE_DEBUG_W_STR("connect refused", ipString);
		}
		else
		{
			CONSOLE_DEBUG("connect error");
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}
		//*	Moved 2/4/2021
		closeRetCode	=	close(socket_desc);
		if (closeRetCode != 0)
		{
			CONSOLE_DEBUG("Close error");
		}
	}
	return(validData);
}


//*****************************************************************************
static void	SendGetRequest(TYPE_ALPACA_UNIT *theDevice, const char *sendData)
{
bool				validData;
SJP_Parser_t		jsonParser;
char				ipString[32];

//	CONSOLE_DEBUG(__FUNCTION__);

	validData	=	GetJsonResponse(	&theDevice->deviceAddress,
										theDevice->port,
										sendData,
										&jsonParser);
	if (validData)
	{
//		SJP_DumpJsonData(&jsonParser);

		ExtractDevicesFromJSON(&jsonParser, theDevice);
		theDevice->queryOKcnt++;
		theDevice->currentlyActive	=	true;
	}
	else
	{
		inet_ntop(AF_INET, &theDevice->deviceAddress.sin_addr, ipString, INET_ADDRSTRLEN);
		CONSOLE_DEBUG_W_STR("No valid data from", ipString);

		theDevice->queryERRcnt++;
		theDevice->currentlyActive	=	false;
	}
}

#if 0
//*****************************************************************************
static void	BumpNoResponseCount(void)
{
int		ii;

//	CONSOLE_DEBUG(__FUNCTION__);
	for (ii=0; ii<gAlpacaUnitCnt; ii++)
	{
		gAlpacaUnitList[ii].noResponseCnt++;
	}
}
#endif // 0

//*****************************************************************************
static void	PollAllDevices(void)
{
int		ii;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("gAlpacaUnitCnt\t=", gAlpacaUnitCnt);
	for (ii=0; ii<gAlpacaUnitCnt; ii++)
	{
//		CONSOLE_DEBUG_W_NUM("ii\t=", ii);
		if (gAlpacaUnitList[ii].noResponseCnt == 0)
		{
			SendGetRequest(&gAlpacaUnitList[ii], "/management/v1/configureddevices");
		}
	}
//	CONSOLE_DEBUG_W_NUM("gRemoteCnt\t=", gRemoteCnt);
}


#ifdef LOG_DISCOVERED_IP_ADDRS
#ifdef _ENABLE_SKYTRAVEL_
//*****************************************************************************
void	FormatTimeStringISO8601(struct timeval *tv, char *timeString)
{
struct tm	*linuxTime;
long		milliSecs;

	if ((tv != NULL) && (timeString != NULL))
	{
		linuxTime		=	gmtime(&tv->tv_sec);
		milliSecs		=	tv->tv_usec / 1000;

		sprintf(timeString, "%d-%02d-%02dT%02d:%02d:%02d.%03ld",
								(1900 + linuxTime->tm_year),
								(1 + linuxTime->tm_mon),
								linuxTime->tm_mday,
								linuxTime->tm_hour,
								linuxTime->tm_min,
								linuxTime->tm_sec,
								milliSecs);

	}
}
#endif // _ENABLE_SKYTRAVEL_


//*****************************************************************************
static void	LogNewIpAddress(struct sockaddr_in *deviceAddress)
{
char			logFileName[]	=	"IPaddressLog.txt";
char			timeString[64];
char			ipAddrSt[64];
FILE			*filePointer;
struct timeval	currentTime;		//*	time exposure or video was started for frame rate calculations

	filePointer	=	fopen(logFileName, "a");
	if (filePointer != NULL)
	{
		gettimeofday(&currentTime, NULL);	//*	save the time we started the exposure
		FormatTimeStringISO8601(	&currentTime, timeString);

		inet_ntop(AF_INET, &deviceAddress->sin_addr, ipAddrSt, INET_ADDRSTRLEN);


		fprintf(filePointer, "%s\t%s\n",	timeString, ipAddrSt);

		fclose(filePointer);
	}
}
#endif // LOG_DISCOVERED_IP_ADDRS



//*****************************************************************************
static void	AddIPaddressToList(struct sockaddr_in *deviceAddress, SJP_Parser_t *jsonParser)
{
int		iii;
bool	newDevice;
int		theDeviceIdx;
bool	foundHostName;
char	myHostNameStr[128];
int		alpacaListenPort;

//	CONSOLE_DEBUG(__FUNCTION__);
	//*	find the alpaca port
	alpacaListenPort	=	12345;

	for (iii=0; iii<jsonParser->tokenCount_Data; iii++)
	{
		if (strcmp(jsonParser->dataList[iii].keyword, "ALPACAPORT") == 0)
		{
			alpacaListenPort	=	atoi(jsonParser->dataList[iii].valueString);
		}
	}


	newDevice		=	true;
	theDeviceIdx	=	-1;
	for (iii=0; iii<gAlpacaUnitCnt; iii++)
	{
		if (	(deviceAddress->sin_addr.s_addr	==	gAlpacaUnitList[iii].deviceAddress.sin_addr.s_addr)
			&&	(alpacaListenPort				==	gAlpacaUnitList[iii].port))
		{
			newDevice		=	false;
			theDeviceIdx	=	iii;
			break;
		}
	}
	if (newDevice)
	{
		//*	add the new devices to our list
//		CONSOLE_DEBUG("We have a new devices")
		if (gAlpacaUnitCnt < kMaxAlpacaIPaddrCnt)
		{
			gAlpacaUnitList[gAlpacaUnitCnt].deviceAddress	=	*deviceAddress;
			gAlpacaUnitList[gAlpacaUnitCnt].port			=	alpacaListenPort;

			//*	and lookup the host name
			foundHostName	=	LookupNameFromIPaddr(deviceAddress->sin_addr.s_addr, myHostNameStr);
			if (foundHostName)
			{
			//	CONSOLE_DEBUG(myHostNameStr);
				strcpy(gAlpacaUnitList[gAlpacaUnitCnt].hostName, myHostNameStr);
			}
			gAlpacaUnitCnt++;
		}
		else
		{
			CONSOLE_DEBUG("Ran out of space in gAlpacaUnitList")
		}
#ifdef LOG_DISCOVERED_IP_ADDRS
		LogNewIpAddress(deviceAddress);
#endif // LOG_DISCOVERED_IP_ADDRS

	}

	if ((theDeviceIdx >= 0) && (theDeviceIdx < kMaxAlpacaIPaddrCnt))
	{
		//*	set the last time we heard from it
		gAlpacaUnitList[theDeviceIdx].noResponseCnt	=	0;
	}
}

#if 0
//*****************************************************************************
static	void PrintDeviceList(void)
{
int		ii;
char	ipAddrSt[32];

	for (ii=0; ii<gRemoteCnt; ii++)
	{
		inet_ntop(AF_INET, &(gRemoteList[ii].deviceAddress.sin_addr), ipAddrSt, INET_ADDRSTRLEN);

		printf("%s\t",		ipAddrSt);

		printf(":%d\t",		gRemoteList[ii].port);

		printf("%-20s\t",	gRemoteList[ii].deviceTypeStr);
		printf("%-20s\t",	gRemoteList[ii].deviceNameStr);
		printf("%4d\t",		gRemoteList[ii].alpacaDeviceNum);
		printf("%2d\t",		gRemoteList[ii].notSeenCounter);

		printf("\r\n");
		fflush(stdout);
	}
}
#endif // 0



//*****************************************************************************
//*	step through the other devices and see if there is any info we want.
static	void GetInformationFromOtherDevices(void)
{
int				ii;
//char			ipAddressStr[32];
#ifdef _ENABLE_CAMERA_
	int				jjj;
	SJP_Parser_t	jsonParser;
	bool			validData;
	bool			domeInfo;
	double			pressure_kPa;
	double			humidity;
#endif // _ENABLE_CAMERA_

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("gRemoteCnt\t=", gRemoteCnt);
	for (ii=0; ii<gRemoteCnt; ii++)
	{
	#ifdef _ENABLE_SKYTRAVEL_
	//	CONSOLE_DEBUG_W_STR("device type\t=", gRemoteList[ii].deviceTypeStr);

//		inet_ntop(AF_INET, &gRemoteList[ii].deviceAddress.sin_addr, ipAddressStr, INET_ADDRSTRLEN);

//		printf("%-17s\t%-17s\t%-17s\t%-17s\t\r\n",	ipAddressStr,
//													gRemoteList[ii].deviceTypeStr,
//													gRemoteList[ii].deviceNameStr,
//													gRemoteList[ii].versionString);
	#endif
	#ifdef _ENABLE_CAMERA_
		if ((gRemoteList[ii].notSeenCounter == 0) &&
			(strcmp(gRemoteList[ii].deviceTypeStr, "observingconditions") == 0))
		{
			domeInfo	=	false;
			//------------------------------------------------
			//*	description
			//*	we need the description to know if it is indoor or outdoor
			//*	http://192.168.1.166:6800/api/v1/observingconditions/0/description
			SJP_Init(&jsonParser);
			validData	=	GetJsonResponse(	&gRemoteList[ii].deviceAddress,
												gRemoteList[ii].port,
												"/api/v1/observingconditions/0/description",
												&jsonParser);
			if (validData)
			{
				//*	we have valid description
				for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
				{
					if (strcmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
					{
						if (strncasecmp(jsonParser.dataList[jjj].valueString, "dome", 4) == 0)
						{
	//						CONSOLE_DEBUG("We have DOME environmental information");
							domeInfo	=	true;
						}
					}
				}
			}
			else
			{
				CONSOLE_DEBUG("No valid data");
			}

			//------------------------------------------------
			//	pressure
			SJP_Init(&jsonParser);
			validData	=	GetJsonResponse(	&gRemoteList[ii].deviceAddress,
												gRemoteList[ii].port,
												"/api/v1/observingconditions/0/pressure",
												&jsonParser);
			if (validData)
			{
				//*	we have valid pressure
				for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
				{
					if (strcmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
					{
						//*	the response is in hectoPascals
						pressure_kPa	=	atof(jsonParser.dataList[jjj].valueString) / 10.0;
						if (pressure_kPa > 0.0)
						{
							if (domeInfo)
							{
								gEnvData.domeDataValid		=	true;
								gEnvData.domePressure_kPa	=	pressure_kPa;
								gettimeofday(&gEnvData.domeLastUpdate, NULL);

								strcpy(gEnvData.domeDataSource, "Data source: Remote R-Pi with sensehat");
							}
							else
							{
								gEnvData.siteDataValid		=	true;
								gEnvData.sitePressure_kPa	=	pressure_kPa;
								gettimeofday(&gEnvData.siteLastUpdate, NULL);
							}
						}
					}
				}
			}
			else
			{
				CONSOLE_DEBUG("No valid data");
			}

			//------------------------------------------------
			//	humidity
			SJP_Init(&jsonParser);
			validData	=	GetJsonResponse(	&gRemoteList[ii].deviceAddress,
												gRemoteList[ii].port,
												"/api/v1/observingconditions/0/humidity",
												&jsonParser);
			if (validData)
			{
				//*	we have valid humidity
				for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
				{
					if (strcmp(jsonParser.dataList[jjj].keyword, "VALUE") == 0)
					{
						humidity	=	atof(jsonParser.dataList[jjj].valueString);
						if (humidity > 0.0)
						{
						//	CONSOLE_DEBUG_W_DBL("Valid humidity data=", humidity);
							if (domeInfo)
							{
								gEnvData.domeDataValid		=	true;
								gEnvData.domeHumidity		=	humidity;
								gettimeofday(&gEnvData.domeLastUpdate, NULL);
							}
							else
							{
								gEnvData.siteDataValid		=	true;
								gEnvData.siteHumidity		=	humidity;
								gettimeofday(&gEnvData.siteLastUpdate, NULL);
							}
						}
					}
				}
			}
			else
			{
				CONSOLE_DEBUG("No valid data");
			}
		}
	#endif
	}
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
}

//*****************************************************************************
int StartDiscoveryQuerryThread(void)
{
int			threadErr;

	CONSOLE_DEBUG(__FUNCTION__);
	gDiscoveryThreadKeepRunning	=	true;
	threadErr			=	pthread_create(&gDiscoveryFindThreadID, NULL, &LookForAlpacaDevicesThread, NULL);

	return(threadErr);
}

//*****************************************************************************
//*	this thread looks for other devices on a periodic basis.
//*	it does not return unless there is an error
//*****************************************************************************
static void	*LookForAlpacaDevicesThread(void *arg)
{
char				broadCastMsg[]	=	kAlpacaDiscoveryMsg;
struct sockaddr_in	servaddr;
struct sockaddr_in	cliaddr;
struct sockaddr_in	from;
int					rcvCnt;
unsigned int		fromlen;
int					setOptRetCode;
int					bindRetCode;
int					sendtoRetCode;
char				buf[kReceiveBufferSize + 1];
char				ipAddressStr[INET_ADDRSTRLEN];
SJP_Parser_t		jsonParser;
int					timeOutCntr;
int					sockOptValue;

//	CONSOLE_DEBUG(__FUNCTION__);
	gBroadcastSock	=	socket(AF_INET, SOCK_DGRAM, 0);
	if (gBroadcastSock  < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, '\0', sizeof(struct sockaddr_in));
	servaddr.sin_family			=	AF_INET;
	servaddr.sin_port			=	htons(kAlpacaDiscoveryPORT);
	servaddr.sin_addr.s_addr	=	htonl(INADDR_BROADCAST);

	cliaddr.sin_family			=	AF_INET;
	cliaddr.sin_addr.s_addr		=	htonl(INADDR_ANY);
	cliaddr.sin_port			=	htons(0);


	sockOptValue	=	1;
	setOptRetCode	=	setsockopt(gBroadcastSock, SOL_SOCKET, (SO_BROADCAST), &sockOptValue, sizeof(int));
//	setOptRetCode	=	setsockopt(gBroadcastSock, SOL_SOCKET, (SO_BROADCAST), &(int) { 1 }, sizeof(int));
	if (setOptRetCode < 0)
	{
		perror("setsockopt(SO_BROADCAST) failed");
	}

	//*	set a timeout
	setOptRetCode	=	SetSocketTimeouts(gBroadcastSock, 3);


	bindRetCode	=	bind(gBroadcastSock, (const struct sockaddr *)&cliaddr, sizeof(cliaddr));
	if (bindRetCode < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	sendtoRetCode	=	0;
	while (gDiscoveryThreadKeepRunning && (sendtoRetCode >= 0))
	{
		gDiscoveryThreadIsRunning	=	true;
	#ifdef _ENABLE_SKYTRAVEL_
//		CONSOLE_DEBUG(__FUNCTION__);
	#endif
//		printf("*******************************************************************************\r\n");
		BumpNotSeenCounter();
//		CONSOLE_DEBUG("Calling sendto");
		//*	send the broadcast message to everyone
		sendtoRetCode	=	sendto(	gBroadcastSock,
									broadCastMsg,
									strlen(broadCastMsg),
									0,
									(struct sockaddr *)&servaddr,
									sizeof(struct sockaddr_in));
		if (sendtoRetCode < 0)
		{
			CONSOLE_DEBUG("sendto returned error");
			perror("sendto");
		}
		timeOutCntr	=	0;
		fromlen	=	sizeof(struct sockaddr_in);
		while (timeOutCntr < 2)
		{
			rcvCnt	=	recvfrom(gBroadcastSock, buf, kReceiveBufferSize, 0, (struct sockaddr *)&from, &fromlen);
			if (rcvCnt > 0)
			{
				buf[rcvCnt]	=	0;
				SJP_Init(&jsonParser);
				SJP_ParseData(&jsonParser, buf);
//				SJP_DumpJsonData(&jsonParser);

			#ifdef _ENABLE_SKYTRAVEL_
//				CONSOLE_DEBUG_W_STR("buf=", buf);
			#endif

				AddIPaddressToList(&from, &jsonParser);

				inet_ntop(AF_INET, &(from.sin_addr), ipAddressStr, INET_ADDRSTRLEN);
		//		CONSOLE_DEBUG_W_HEX("from.sin_addr\t=", from.sin_addr);
		//		CONSOLE_DEBUG_W_STR("from.sin_addr\t=", ipAddressStr);
			}
			else if (rcvCnt == 0)
			{
				printf("no response\r\n");
			}
			else
			{
			//	perror("recvfrom");
				timeOutCntr++;
			}
	//		CONSOLE_DEBUG_W_HEX("from.sin_addr=", from.sin_addr.s_addr);
	//		CONSOLE_DEBUG_W_NUM("from.sin_addr=", ((from.sin_addr.s_addr >> 24) & 0x0ff));
	//		CONSOLE_DEBUG_W_NUM("from.sin_addr=", ((from.sin_addr.s_addr >> 16) & 0x0ff));
	//		CONSOLE_DEBUG_W_NUM("from.sin_addr=", ((from.sin_addr.s_addr >> 8) & 0x0ff));
	//		CONSOLE_DEBUG_W_NUM("from.sin_addr=", ((from.sin_addr.s_addr) & 0x0ff));

		}
		if (gNeedToReadExternalList)
		{
			ReadExternalIPlist_FromThread();
			gNeedToReadExternalList	=	false;
		}
		PollAllDevices();

//		PrintDeviceList();

		GetInformationFromOtherDevices();

		//*	we dont need to do this very often
	//	sleep(3000);
		sleep(15);
	//	CONSOLE_DEBUG("Done sleeping");
	}

	CONSOLE_DEBUG("Thread exit!!!!!!!!");

	gDiscoveryThreadIsRunning	=	false;
	return(NULL);
}

//*****************************************************************************
static void	ReadExternalIPlist_FromThread(void)
{
FILE				*filePointer;
char				lineBuff[256];
char				outputIPaddr[256];
int					ii;
int					slen;
char				fileName[]	=	"external_ip_list.txt";
SJP_Parser_t		jsonParser;
struct sockaddr_in	from;
char				portNumStr[32];
char				*colonPtr;

	CONSOLE_DEBUG(__FUNCTION__);

	//*	see if there is a file listing extra IP address
	filePointer	=	fopen(fileName, "r");
	if (filePointer != NULL)
	{
		while (fgets(lineBuff, 200, filePointer))
		{
			//*	get rid of the trailing CR/LF
			slen	=	strlen(lineBuff);
			for (ii=0; ii<slen; ii++)
			{
				if ((lineBuff[ii] == 0x0d) || (lineBuff[ii] == 0x0a))
				{
					lineBuff[ii]	=	0;
					break;
				}
			}
			CONSOLE_DEBUG_W_STR("External IP address\t=",		lineBuff);
			slen	=	strlen(lineBuff);
			if ((slen > 6) && (lineBuff[0] != '#'))
			{
				//*	look to see if there is a port number specified
				colonPtr	=	strchr(lineBuff, ':');
				if (colonPtr != NULL)
				{
					colonPtr++;
					strcpy(portNumStr, colonPtr);
				}
				else
				{
					//*	set the default
					strcpy(portNumStr, "6800");
				}

//---				from.sin_addr.s_addr	=	htonl((192 << 24) + (168 << 16) + (50 << 8) + 46);
				inet_pton(AF_INET, lineBuff, &(from.sin_addr));

				inet_ntop(AF_INET, &(from.sin_addr), outputIPaddr, INET_ADDRSTRLEN);
				CONSOLE_DEBUG_W_STR("outputIPaddr\t\t=",		outputIPaddr);

				SJP_Init(&jsonParser);
				strcpy(jsonParser.dataList[0].keyword, "ALPACAPORT");
				strcpy(jsonParser.dataList[0].valueString, portNumStr);
				jsonParser.tokenCount_Data	=	1;

			//	AddUnitToList(&from, &jsonParser);
				AddIPaddressToList(&from, &jsonParser);
			}
		}
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG_W_STR("File Not found\t=",	fileName);
	}
}

//	//*****************************************************************************
//	int	GetMySubnetNumber(void)
//	{
//		return(gMyIPaddress & 0x00ff);
//	}

//*****************************************************************************
static void GetMyAddress(void)
{
struct ifaddrs	*ifAddrStruct	=	NULL;
struct ifaddrs	*ifa			=	NULL;
void			*tmpAddrPtr		=	NULL;
char			addressBuffer[256];
uint32_t		ipAddress32;

	CONSOLE_DEBUG(__FUNCTION__);

	getifaddrs(&ifAddrStruct);

	if (ifAddrStruct != NULL)
	{
		ifa	=	ifAddrStruct;
		//*	step thru the linked list of ip addresses
		while (ifa != NULL)
		{
			// Check if it is a valid IPv4 address
			if (ifa ->ifa_addr->sa_family == AF_INET)
			{
				tmpAddrPtr	=	&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;

				ipAddress32	=	ntohl(*((uint32_t *)tmpAddrPtr));
				CONSOLE_DEBUG_W_HEX("ipAddress32\t=", ipAddress32);
				if (ipAddress32 != 0x7f000001)
				{
					gMyIPaddress	=	ipAddress32;
					CONSOLE_DEBUG_W_HEX("gMyIPaddress\t=", gMyIPaddress);
				}
				inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
	//			printf("%s IPV4 Address %s\n", ifa->ifa_name, addressBuffer);
				CONSOLE_DEBUG_W_STR("IPV4 Name   :", ifa->ifa_name);
				CONSOLE_DEBUG_W_STR("IPV4 Address:", addressBuffer);
			}
			else if (ifa->ifa_addr->sa_family==AF_INET6)
			{
				// Check if it is a valid IPv6 address
				tmpAddrPtr	=	&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
				inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
	//			printf("%s IPV6 Address %s\n", ifa->ifa_name, addressBuffer);

				CONSOLE_DEBUG_W_STR("IPV6 Name   :", ifa->ifa_name);
				CONSOLE_DEBUG_W_STR("IPV6 Address:", addressBuffer);
			}
			else
			{
				CONSOLE_DEBUG_W_STR("Unknow family Name   :", ifa->ifa_name);
			}
	//+		if (gDebugDiscovery)
			{
				CONSOLE_DEBUG("Stepping to Next ip address --------------------------------------");
			}
			ifa	=	ifa->ifa_next;
		}


		CONSOLE_DEBUG("Freeing ifAddrStruct");
		freeifaddrs(ifAddrStruct);
	}
	else
	{
		CONSOLE_DEBUG("getifaddrs() Failed!!!!!");
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");
}


//**************************************************************************************
int	FindDeviceInList(TYPE_REMOTE_DEV *theDevice, TYPE_REMOTE_DEV *theList, int maxDevices)
{
int		foundIndex;
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);

	foundIndex	=	-1;
	iii			=	0;
	while ((foundIndex < 0) && (iii < maxDevices))
	{
		if ((theDevice->deviceAddress.sin_addr.s_addr	==	theList[iii].deviceAddress.sin_addr.s_addr)
			&&	(theDevice->port						==	theList[iii].port)
			&&	(theDevice->alpacaDeviceNum				==	theList[iii].alpacaDeviceNum)
			&&	(strcasecmp(theDevice->deviceTypeStr, theList[iii].deviceTypeStr) == 0)
				)
		{
			foundIndex	=	iii;
		}
		iii++;
	}
	return(foundIndex);
}


//*****************************************************************************
int StartDiscoveryListenThread(int alpacaListenPort)
{
int			threadErr;

	CONSOLE_DEBUG(__FUNCTION__);

	GetMyAddress();
	gAlpacaListenPort	=	alpacaListenPort;

	CONSOLE_DEBUG_W_NUM("Staring discovery listen thread on port", gAlpacaListenPort);
	threadErr			=	pthread_create(&gDiscoveryListenThreadID, NULL, &DiscoveryListenThread, NULL);
	if (threadErr == 0)
	{
		CONSOLE_DEBUG("Listen thread created successfully");
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Error on thread creation, Error number:", threadErr);
	}
	return(threadErr);
}




#if 0

//*****************************************************************************
int	main(int argc, char **argv)
{
	printf("Staring discovery listen thread %s\r\n", __FUNCTION__);

	StartDiscoveryThread(6800);

	while(1)
	{
		sleep(1);
	}
}
#endif // 0
