//*****************************************************************************
//*	File:	discoverythread.c
//*
//*		Alpaca discovery thread
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
//*	Mar  9,	2021	<MLS> Moved FindDeviceInList() to discoverythread.c
//*	Aug 20,	2021	<MLS> Added debugging code for new universal CONFORM program from Peter
//*	Aug 23,	2021	<MLS> Changed discovery response string from "alpacaport" to "AlpacaPort"
//*	Aug 23,	2021	<MLS> Added AlpacaUnitQsortProc(), ip addr list is now sorted
//*	Feb 21,	2022	<MLS> Added ResetExternalIPaddress()
//*	Mar 11,	2022	<MLS> WireGuard makes it so I cant figure out the local IP addres
//*	Mar 11,	2022	<MLS> Added ReadLocalIPaddress()
//*	Mar 11,	2022	<MLS> Working with WireGuard
//*	Jun  2,	2022	<MLS> Added parsing for upTime_Days & cpuTemp_DegF
//*	Aug  6,	2022	<MLS> Added CPU temp logging
//*	Aug 23,	2022	<MLS> Added keeping track of max CPU temp
//*	Dec 22,	2022	<MLS> Added WakeUpDiscoveryThread()
//*	Feb 10,	2024	<MLS> Added GetLibraryInfo()
//*	May 15,	2024	<MLS> Added _DEBUG_DISCOVERY_
//*****************************************************************************

//#define		_DEBUG_DISCOVERY_

#include	<ctype.h>
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

#define _DEBUG_TIMING_
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
#include	"linuxerrors.h"
#include	"helper_functions.h"

#ifdef _INCLUDE_CTRL_MAIN_
	#include	"controller_startup.h"
#else
	static	int		SetStartupText(const char *textMsg)	{ return(1);}
	static	void	SetStartupTextStatus(const int idx, const char *textMsg)	{ }
#endif

#include	"obsconditions_globals.h"
#include	"HostNames.h"

#define kAlpacaDiscoveryPORT	32227

#define	kReceiveBufferSize	1550



pthread_t	gDiscoveryListenThreadID;
pthread_t	gDiscoveryFindThreadID;


//*	this is a list of IP addresses
TYPE_ALPACA_UNIT	gAlpacaUnitList[kMaxAlpacaIPaddrCnt];
int					gAlpacaUnitCnt	=	0;


//*	this is a list of alpaca devices, can be more than one per IP address
TYPE_REMOTE_DEV		gRemoteList[kMaxAlpacaDeviceCnt];
int					gRemoteCnt		=	0;

bool				gDiscoveryThreadIsRunning	=	false;
bool				gDiscoveryThreadKeepRunning	=	true;
bool				gDiscoveryWakeUp			=	false;

static	int			gBroadcastSock;
static	uint32_t	gMyIPaddress		=	0;
static	int			gAlpacaListenPort	=	9999;


static void		*LookForAlpacaDevicesThread(void *arg);
static void		ReadExternalIPlist_FromThread(void);
static void		GetMyAddress(void);

static bool		gNeedToReadExternalList	=	true;	//*	we only need to do this once

#if !defined(__arm__)

//*****************************************************************************
static bool	ReadLocalIPaddress(void)
{
FILE		*filePointer;
char		lineBuff[256];
char		fileName[]	=	"local_ip_addr.txt";
char		outputIPaddrStr[64];
bool		validAddress;
int			slen;
int			iii;
int			startupWidgetIdx	=	-1;

//	CONSOLE_DEBUG(__FUNCTION__);

	validAddress		=	false;
	//*	see if there is a file with the local IP address
	filePointer	=	fopen(fileName, "r");
	if (filePointer != NULL)
	{
		startupWidgetIdx	=	SetStartupText("Local IP address:");
		while (fgets(lineBuff, 200, filePointer))
		{
			slen	=	strlen(lineBuff);
			if ((slen > 6) && (lineBuff[0] != '#'))
			{
//				CONSOLE_DEBUG_W_STR("Local IP address\t=",		lineBuff);

				if (isdigit(lineBuff[0]))
				{
					//*	get rid of the trailing CR/LF
					for (iii=0; iii<slen; iii++)
					{
						if ((lineBuff[iii] == 0x0d) || (lineBuff[iii] == 0x0a))
						{
							lineBuff[iii]	=	0;
							break;
						}
					}
					//*	extract the IP address
					inet_pton(AF_INET, lineBuff, &gMyIPaddress);
//					CONSOLE_DEBUG_W_HEX("sin_addr\t\t=",		gMyIPaddress);

					//*	this is just for debugging to make sure we got it right
					inet_ntop(AF_INET, &gMyIPaddress, outputIPaddrStr, INET_ADDRSTRLEN);
//					CONSOLE_DEBUG_W_STR("outputIPaddrStr\t=",		outputIPaddrStr);

					validAddress	=	true;
				}
			}
		}
		fclose(filePointer);
		SetStartupTextStatus(startupWidgetIdx, (validAddress ? "OK" : "Failed"));
	}
	return(validAddress);
}
#endif	//	!defined(__arm__)


//*****************************************************************************
void	Discovery_ClearIPAddrList(void)
{
int	iii;

	gAlpacaUnitCnt	=	0;
	gRemoteCnt		=	0;
	for (iii=0; iii<kMaxAlpacaIPaddrCnt; iii++)
	{
		memset((void *)&gAlpacaUnitList[iii], 0, sizeof(TYPE_ALPACA_UNIT));
		gAlpacaUnitList[iii].displayGraph	=	true;
	}

	for (iii=0; iii<kMaxAlpacaDeviceCnt; iii++)
	{
		memset((void *)&gRemoteList[iii], 0, sizeof(TYPE_REMOTE_DEV));
	}
	ResetExternalIPaddress();

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
char				responseBuff[256];
int					socketOption;
int					setSocketRtnCde;
bool				validDiscoveryRequest;
char				ipAddrSt[48];
char				errnoString[128];

	CONSOLE_DEBUG("Starting discovery listen thread");

	mySocket	=	socket(AF_INET, SOCK_DGRAM, 0);
	if (mySocket >= 0)
	{
		memset((void *)&serverAddress, 0, sizeof(struct sockaddr_in));
		serverAddress.sin_family			=	AF_INET;
		serverAddress.sin_family			=	AF_INET;
		serverAddress.sin_addr.s_addr		=	INADDR_ANY;
		serverAddress.sin_port				=	htons(kAlpacaDiscoveryPORT);

		//*	the SO_REUSEPORT option should allow more than one program to use the same port
		socketOption	=	1;
		setSocketRtnCde	=	setsockopt(	mySocket,
										SOL_SOCKET,
										(SO_REUSEPORT),
										(void *)&socketOption,
										sizeof(int));

		if (setSocketRtnCde < 0)
		{
			GetLinuxErrorString(errno, errnoString);
			CONSOLE_DEBUG_W_STR("setsockopt() Failed; error=", errnoString);
			perror("setsockopt(SO_REUSEPORT) failed");
		}

		if (bind(mySocket, (struct sockaddr *)&serverAddress, sizeof(struct sockaddr_in)) < 0)
		{
			GetLinuxErrorString(errno, errnoString);
			CONSOLE_DEBUG_W_STR("Binding Failed; error=", errnoString);
			perror("binding");
		}
		while (1)
		{
		#ifdef _DEBUG_DISCOVERY_
			CONSOLE_DEBUG(__FUNCTION__);
		#endif
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
				inet_ntop(AF_INET, &(fromAddress.sin_addr), ipAddrSt, INET_ADDRSTRLEN);
			#ifdef _DEBUG_DISCOVERY_
				sprintf(responseBuff, "Request from:%s, %s", ipAddrSt, readBuf);
				CONSOLE_DEBUG(responseBuff);
			#endif

				validDiscoveryRequest	=	false;
				//*	this was the original discovery query
				if (strncasecmp(readBuf,	"alpaca discovery", 16) == 0)
				{
					validDiscoveryRequest	=	true;

					CONSOLE_DEBUG_W_STR("Old style discovery request\t=", readBuf);
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
					sprintf(responseBuff, "{\"AlpacaPort\": %d}", gAlpacaListenPort);
				#ifdef _DEBUG_DISCOVERY_
					CONSOLE_DEBUG_W_STR("responseBuff\t=", responseBuff);
				#endif

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
		CONSOLE_DEBUG("Discovery listen port failed");
		perror("Opening socket");
	}
#ifdef _DEBUG_DISCOVERY_
	CONSOLE_DEBUG("exit");
#endif
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

#ifdef _DEBUG_DISCOVERY_
	CONSOLE_DEBUG(__FUNCTION__);
#endif
	newDevice	=	true;

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

	if (newDevice)
	{
		if (gRemoteCnt < kMaxAlpacaDeviceCnt)
		{
			gRemoteList[gRemoteCnt]					=	*newRemoteDevice;
			gRemoteList[gRemoteCnt].notSeenCounter	=	0;
			gRemoteList[gRemoteCnt].deviceTypeEnum	=	FindDeviceTypeByString(gRemoteList[gRemoteCnt].deviceTypeStr);

			//*	lookup the host name

			gRemoteCnt++;
		}
	}
}


//{"Value":[{	"DeviceName":"ASCOM Simulator SafetyMonitor Driver",
//			"DeviceType":"SafetyMonitor",
//			"DeviceNumber":0,
//			"UniqueID":"7B79C9E6-6338-41E5-89DE-4B85EAB47953"},
//			{
//			"DeviceName":"ASCOM Observing Conditions Hub (OCH)",
//			"DeviceType":"ObservingConditions",
//			"DeviceNumber":0,
//			"UniqueID":"4BDFE843-5E27-4983-816A-45FF3F92914C"},
//			{
//			"DeviceName":"Dome Simulator .NET",
//			"DeviceType":"Dome",
//			"DeviceNumber":0,
//			"UniqueID":"BEAD123F-518A-472F-BEBD-17CABA2994A8"},
//			{
//			"DeviceName":"Simulator",
//			"DeviceType":"Telescope",
//			"DeviceNumber":0,
//			"UniqueID":"8CBF0B5B-63EB-4B79-AF6D-A05FD7805A98"},
//			{
//			"DeviceName":"ASCOM Simulator Focuser Driver",
//			"DeviceType":"Focuser",
//			"DeviceNumber":0,
//			"UniqueID":"596C180F-8049-47ED-8D03-2D59237A6198"},
//			{
//			"DeviceName":"Camera V2 simulator",
//			"DeviceType":"Camera",
//			"DeviceNumber":0,
//			"UniqueID":"AE6042D5-161F-4DF3-B9C8-85D37E0CAEB0"}
//			],
//			"ClientTransactionID":0,
//			"ServerTransactionID":105682,
//			"ErrorNumber":0,
//			"ErrorMessage":""}
//


//*****************************************************************************
void	ExtractDevicesFromJSON(SJP_Parser_t *jsonParser, TYPE_ALPACA_UNIT *theDevice)
{
TYPE_REMOTE_DEV	myRemoteDevice;
int				iii;
int				jjj;
char			myVersionString[64];

#ifdef _DEBUG_DISCOVERY_
	CONSOLE_DEBUG(__FUNCTION__);
#endif
	memset((void *)&myRemoteDevice, 0, sizeof(TYPE_REMOTE_DEV));
	memset((void *)myVersionString, 0, sizeof(myVersionString));

	for (iii=0; iii<jsonParser->tokenCount_Data; iii++)
	{
		if (strcasecmp(jsonParser->dataList[iii].keyword, "VERSION") == 0)
		{
			strcpy(myVersionString,				jsonParser->dataList[iii].valueString);
			strcpy(theDevice->versionString,	jsonParser->dataList[iii].valueString);
		}
		else if (strcasecmp(jsonParser->dataList[iii].keyword, "upTime_Days") == 0)
		{
			theDevice->upTimeValid	=	true;
			theDevice->upTimeDays	=	atoi(jsonParser->dataList[iii].valueString);
		}
		else if (strcasecmp(jsonParser->dataList[iii].keyword, "cpuTemp_DegF") == 0)
		{
		int	minutesSinceMidnight;
		int	cpuTempIndex;

			theDevice->cpuTempValid	=	true;
			theDevice->cpuTemp_DegF	=	atof(jsonParser->dataList[iii].valueString);
			if (theDevice->cpuTemp_DegF > theDevice->cpuTemp_DegF_max)
			{
				theDevice->cpuTemp_DegF_max	=	theDevice->cpuTemp_DegF;
			}

			minutesSinceMidnight	=	GetMinutesSinceMidnight();
			cpuTempIndex			=	minutesSinceMidnight / 2;
			if (cpuTempIndex < kMaxCPUtempEntries)
			{
//				CONSOLE_DEBUG_W_NUM("cpuTempIndex\t=", cpuTempIndex);
				theDevice->cpuTempLog[cpuTempIndex]	=	theDevice->cpuTemp_DegF;
				//*	make sure there is a separator when wrapping to the previous day
				cpuTempIndex++;

				//*	set the next 10 values to zero for a break between days
				jjj	=	0;
				while ((cpuTempIndex < kMaxCPUtempEntries) && (jjj < 6))
				{
					theDevice->cpuTempLog[cpuTempIndex]	=	0;
					cpuTempIndex++;
					jjj++;
				}
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("ERROR!!!! cpuTempIndex\t=", cpuTempIndex);
			}
		}
		else if (strcasecmp(jsonParser->dataList[iii].keyword, "DEVICETYPE") == 0)
		{
			strcpy(myRemoteDevice.deviceTypeStr, jsonParser->dataList[iii].valueString);
		}
		else if (strcasecmp(jsonParser->dataList[iii].keyword, "DEVICENAME") == 0)
		{
			strcpy(myRemoteDevice.deviceNameStr, jsonParser->dataList[iii].valueString);
		}
		else if (strcasecmp(jsonParser->dataList[iii].keyword, "DEVICENUMBER") == 0)
		{
			myRemoteDevice.alpacaDeviceNum	=	atoi(jsonParser->dataList[iii].valueString);
		}
		else if (strcasecmp(jsonParser->dataList[iii].keyword, "TIMESTAMP") == 0)
		{
			strcpy(theDevice->timeStampString, jsonParser->dataList[iii].valueString);
		}
//		else
//		{
//			CONSOLE_DEBUG_W_STR("keyword\t=",	jsonParser->dataList[iii].keyword);
//			CONSOLE_DEBUG_W_STR("value  \t=",	jsonParser->dataList[iii].valueString);
//		}

		//------------------------------------
		if (strcasecmp(jsonParser->dataList[iii].keyword, "ARRAY-NEXT") == 0)
		{
			myRemoteDevice.deviceAddress	=	theDevice->deviceAddress;
			myRemoteDevice.port				=	theDevice->port;
			strcpy(myRemoteDevice.hostName, theDevice->hostName);
			strcpy(myRemoteDevice.versionString, myVersionString);

			UpdateRemoteList(&myRemoteDevice);

			memset((void *)&myRemoteDevice, 0, sizeof(TYPE_REMOTE_DEV));

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
char				returnedData[kReadBuffLen];
char				xmitBuffer[kReadBuffLen];
int					setOptRetCode;
char				errorString[64];

#ifdef _DEBUG_DISCOVERY_
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM(	"kReadBuffLen        \t=",	kReadBuffLen);
	CONSOLE_DEBUG_W_SIZE(	"sizeof(returnedData)\t=",	sizeof(returnedData));
#endif

	validData	=	false;
	socket_desc	=	socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc >= 0)
	{
		//*	Mar 13,	2021	<MLS> Added timeout to GetJsonResponse()
		//*	Jun 25,	2023	<MLS> Increased GetJsonResponse() to 5 seconds (was 3)
		//*	set a timeout
		setOptRetCode	=	SetSocketTimeouts(socket_desc, 5);
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

			strcat(xmitBuffer, " HTTP/1.0\r\n");
			strcat(xmitBuffer, "Host: 127.0.0.1:6800\r\n");
//			strcat(xmitBuffer, "User-Agent: AlpacaPi\r\n");
			strcat(xmitBuffer,	gUserAgentAlpacaPiStr);
			strcat(xmitBuffer, "Accept: text/html,application/json\r\n");
			strcat(xmitBuffer, "\r\n");

			sendRetCode	=	send(socket_desc , xmitBuffer , strlen(xmitBuffer) , 0);
			if (sendRetCode >= 0)
			{
				recvByteCnt	=	recv(socket_desc, returnedData, kReadBuffLen , 0);
				if (recvByteCnt >= 0)
				{
					validData					=	true;
					returnedData[recvByteCnt]	=	0;
//					printf("%s\r\n", returnedData);
					SJP_Init(jsonParser);
					SJP_ParseData(jsonParser, returnedData);
//					SJP_DumpJsonData(jsonParser);
				}
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("send() error", errno);
			}
			shutdownRetCode	=	shutdown(socket_desc, SHUT_RDWR);
			if (shutdownRetCode != 0)
			{
			//	CONSOLE_DEBUG_W_NUM("shutdown() error", errno);
				GetLinuxErrorString(errno, errorString);
				CONSOLE_DEBUG_W_STR("shutdown() error: Error message\t\t=",	errorString);
			}
		}
		else if (errno == ECONNREFUSED)
		{
		char	ipString[32];

			PrintIPaddressToString(deviceAddress->sin_addr.s_addr, ipString);
			sprintf(errorString, "connect refused by %s:%d", ipString, port);
			CONSOLE_DEBUG(errorString);
		}
		else
		{

		//	CONSOLE_DEBUG("connect error");
		//	CONSOLE_DEBUG_W_NUM("errno\t=", errno);
			GetLinuxErrorString(errno, errorString);
			CONSOLE_DEBUG_W_STR("Connect error: ErrMsg=",	errorString);
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
char				errMsgString[64];

#ifdef _DEBUG_DISCOVERY_
	CONSOLE_DEBUG(__FUNCTION__);
#endif

	validData	=	GetJsonResponse(	&theDevice->deviceAddress,
										theDevice->port,
										sendData,
										&jsonParser);
	if (validData)
	{
//		CONSOLE_DEBUG(sendData);
//		SJP_DumpJsonData(&jsonParser, __FUNCTION__);

		ExtractDevicesFromJSON(&jsonParser, theDevice);
		theDevice->queryOKcnt++;
		theDevice->currentlyActive	=	true;
	}
	else
	{
		inet_ntop(AF_INET, &theDevice->deviceAddress.sin_addr, ipString, INET_ADDRSTRLEN);
		sprintf(errMsgString, "No valid data from %s:%d", ipString, theDevice->port);
		CONSOLE_DEBUG(errMsgString);

		theDevice->queryERRcnt++;
		theDevice->currentlyActive	=	false;
	}
}

#if 0
//*****************************************************************************
static void	BumpNoResponseCount(void)
{
int		iii;

//	CONSOLE_DEBUG(__FUNCTION__);
	for (iii=0; iii<gAlpacaUnitCnt; iii++)
	{
		gAlpacaUnitList[iii].noResponseCnt++;
	}
}
#endif // 0

//*****************************************************************************
// 4=COMMAND             	libraries
// 5=LIBRARY-1           	software-gcc-10.2.1 20210110
// 6=LIBRARY-2           	software-libc-2.31
// 7=LIBRARY-3           	software-cfitsio-4.0
// 8=LIBRARY-4           	software-opencv-4.5.1
//*****************************************************************************
static void	GetLibraryInfo(TYPE_ALPACA_UNIT *alpacaUnit)
{
int				jjj;
bool			validData;
SJP_Parser_t	jsonParser;
char			*valuePtr;

	validData	=	GetJsonResponse(	&alpacaUnit->deviceAddress,
										alpacaUnit->port,
										"/api/v1/management/0/libraries",
										&jsonParser);
	if (validData)
	{
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
			//*	is this a library response
			if (strncasecmp(jsonParser.dataList[jjj].keyword, "LIBRARY", 7) == 0)
			{
				valuePtr	=	strchr(jsonParser.dataList[jjj].valueString, '-');
				if (valuePtr != NULL)
				{
					valuePtr	+=	1;
					if (strncasecmp(jsonParser.dataList[jjj].valueString, "software-opencv", 15) == 0)
					{
						strcpy(alpacaUnit->SoftwareVersion[kSoftwareVers_OpenCV].SoftwareVerStr, valuePtr);
					}
					else if (strncasecmp(jsonParser.dataList[jjj].valueString, "software-cfitsio", 16) == 0)
					{
						strcpy(alpacaUnit->SoftwareVersion[kSoftwareVers_Fits].SoftwareVerStr, valuePtr);
					}
					else if (strncasecmp(jsonParser.dataList[jjj].valueString, "software-wiringPi", 17) == 0)
					{
						strcpy(alpacaUnit->SoftwareVersion[kSoftwareVers_WiringPi].SoftwareVerStr, valuePtr);
					}
				}
			}
			else if (strcasecmp(jsonParser.dataList[jjj].keyword, "hardware") == 0)
			{
				//*	this is the hardware response
				strcpy(	alpacaUnit->SoftwareVersion[kSoftwareVers_Hardware].SoftwareVerStr,
						jsonParser.dataList[jjj].valueString);
			}
		}
	}
	alpacaUnit->SoftwareVersionOK	=	true;
}

//*****************************************************************************
static void	GetCPUstats(TYPE_ALPACA_UNIT *alpacaUnit)
{
int				jjj;
bool			validData;
SJP_Parser_t	jsonParser;

//	CONSOLE_DEBUG(__FUNCTION__);
	validData	=	GetJsonResponse(	&alpacaUnit->deviceAddress,
										alpacaUnit->port,
										"/api/v1/management/0/cpustats",
										&jsonParser);
	if (validData)
	{
		for (jjj=0; jjj<jsonParser.tokenCount_Data; jjj++)
		{
			//*	is this a hardware response
			if (strcasecmp(jsonParser.dataList[jjj].keyword, "hardware") == 0)
			{
				strcpy(	alpacaUnit->SoftwareVersion[kSoftwareVers_Hardware].SoftwareVerStr,
						jsonParser.dataList[jjj].valueString);
			}
			else if (strcasecmp(jsonParser.dataList[jjj].keyword, "platform") == 0)
			{
//				strcpy(alpacaUnit->Platform, jsonParser.dataList[jjj].valueString);
				strcpy(	alpacaUnit->SoftwareVersion[kSoftwareVers_Platform].SoftwareVerStr,
						jsonParser.dataList[jjj].valueString);
			}
		}
	}
}

//*****************************************************************************
static void	PollAllDevices(void)
{
int				iii;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("gAlpacaUnitCnt\t=", gAlpacaUnitCnt);
	for (iii=0; iii<gAlpacaUnitCnt; iii++)
	{
		if (gAlpacaUnitList[iii].noResponseCnt == 0)
		{
			SendGetRequest(&gAlpacaUnitList[iii], "/management/v1/configureddevices");
		}
		//-----------------------------------------------------------
		//*	check for software versions
		if (gAlpacaUnitList[iii].SoftwareVersionOK == false)
		{
			GetLibraryInfo(&gAlpacaUnitList[iii]);
			GetCPUstats(&gAlpacaUnitList[iii]);
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


//**************************************************************************************
static  int AlpacaUnitQsortProc(const void *e1, const void *e2)
{
TYPE_ALPACA_UNIT	*obj1, *obj2;
int					returnValue;
uint32_t			ipAddr1;
uint32_t			ipAddr2;


	obj1		=	(TYPE_ALPACA_UNIT *)e1;
	obj2		=	(TYPE_ALPACA_UNIT *)e2;

	ipAddr1	=	(obj1->deviceAddress.sin_addr.s_addr & 0x000000ff) << 24;
	ipAddr1	+=	(obj1->deviceAddress.sin_addr.s_addr & 0x0000ff00) << 8;
	ipAddr1	+=	(obj1->deviceAddress.sin_addr.s_addr & 0x00ff0000) >> 8;
	ipAddr1	+=	(obj1->deviceAddress.sin_addr.s_addr & 0xff000000) >> 24;

	ipAddr2	=	(obj2->deviceAddress.sin_addr.s_addr & 0x000000ff) << 24;
	ipAddr2	+=	(obj2->deviceAddress.sin_addr.s_addr & 0x0000ff00) << 8;
	ipAddr2	+=	(obj2->deviceAddress.sin_addr.s_addr & 0x00ff0000) >> 8;
	ipAddr2	+=	(obj2->deviceAddress.sin_addr.s_addr & 0xff000000) >> 24;

	returnValue	=	0;
	if (ipAddr1 < ipAddr2)
	{
		returnValue	=	-1;
	}
	else if (ipAddr1 > ipAddr2)
	{
		returnValue	=	1;
	}

	return(returnValue);

}

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
	//------------------------------------------------
	//*	find the alpaca port
	alpacaListenPort	=	12345;
	for (iii=0; iii<jsonParser->tokenCount_Data; iii++)
	{
//		CONSOLE_DEBUG_W_2STR("kw:val",	jsonParser->dataList[iii].keyword,
//										jsonParser->dataList[iii].valueString);
		if (strcasecmp(jsonParser->dataList[iii].keyword, "ALPACAPORT") == 0)
		{
			alpacaListenPort	=	atoi(jsonParser->dataList[iii].valueString);
		}
	}

	newDevice			=	true;
	theDeviceIdx		=	-1;
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
//			CONSOLE_DEBUG("Adding to table");
//			CONSOLE_DEBUG_W_NUM("gAlpacaUnitCnt\t=", gAlpacaUnitCnt);
			gAlpacaUnitList[gAlpacaUnitCnt].deviceAddress	=	*deviceAddress;
			gAlpacaUnitList[gAlpacaUnitCnt].port			=	alpacaListenPort;
			gAlpacaUnitList[gAlpacaUnitCnt].currentlyActive	=	false;
			gAlpacaUnitList[gAlpacaUnitCnt].displayGraph	=	true;

			//*	and lookup the host name
			foundHostName	=	LookupNameFromIPaddr(deviceAddress->sin_addr.s_addr, myHostNameStr);
			if (foundHostName)
			{
//				CONSOLE_DEBUG_W_STR("Found host name:", myHostNameStr);
				strcpy(gAlpacaUnitList[gAlpacaUnitCnt].hostName, myHostNameStr);
			}
			else
			{
			char	ipAddrSt[32];

				inet_ntop(AF_INET, &(deviceAddress->sin_addr), ipAddrSt, INET_ADDRSTRLEN);
//				CONSOLE_DEBUG_W_HEX("Did not find host name for address:", deviceAddress->sin_addr.s_addr);
				CONSOLE_DEBUG_W_STR("Did not find host name for address:", ipAddrSt);
			}
			gAlpacaUnitCnt++;

			//*	sort by IP address
			qsort(gAlpacaUnitList, gAlpacaUnitCnt, sizeof(TYPE_ALPACA_UNIT), AlpacaUnitQsortProc);

		}
		else
		{
			CONSOLE_DEBUG("Ran out of space in gAlpacaUnitList")
		}
#ifdef LOG_DISCOVERED_IP_ADDRS
		LogNewIpAddress(deviceAddress);
#endif // LOG_DISCOVERED_IP_ADDRS

	}
	else
	{
//		CONSOLE_DEBUG("Already in table")
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
int			startupWidgetIdx;

	startupWidgetIdx	=	SetStartupText("Starting Discovery Query Thread");
	GetMyAddress();

	gDiscoveryThreadKeepRunning	=	true;
	threadErr			=	pthread_create(&gDiscoveryFindThreadID, NULL, &LookForAlpacaDevicesThread, NULL);
	SetStartupTextStatus(startupWidgetIdx, ((threadErr == 0) ? "OK" : "Failed"));
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
int					sleepSecsCounter;

//	CONSOLE_DEBUG("*********************************************************");
//	CONSOLE_DEBUG(__FUNCTION__);
	gBroadcastSock	=	socket(AF_INET, SOCK_DGRAM, 0);
	if (gBroadcastSock  < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset((void *)&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family			=	AF_INET;
	servaddr.sin_port			=	htons(kAlpacaDiscoveryPORT);
	servaddr.sin_addr.s_addr	=	htonl(INADDR_BROADCAST);

	cliaddr.sin_family			=	AF_INET;
	cliaddr.sin_addr.s_addr		=	htonl(INADDR_ANY);
	cliaddr.sin_port			=	htons(0);

#if !defined(__arm__)
bool				validLocalAddress;
	validLocalAddress	=	ReadLocalIPaddress();
	if (validLocalAddress)
	{
	//	cliaddr.sin_addr.s_addr		=	htonl(0xc0a8019a);	//192.168.1.154
		cliaddr.sin_addr.s_addr		=	gMyIPaddress;
	}
#endif

//	inet_ntop(AF_INET, &(servaddr.sin_addr), ipAddressStr, INET_ADDRSTRLEN);
//	CONSOLE_DEBUG_W_STR("servaddr.sin_addr\t=", ipAddressStr);
//
//	inet_ntop(AF_INET, &(cliaddr.sin_addr), ipAddressStr, INET_ADDRSTRLEN);
//	CONSOLE_DEBUG_W_STR("cliaddr.sin_addr\t=", ipAddressStr);

	sockOptValue	=	1;
	setOptRetCode	=	setsockopt(gBroadcastSock, SOL_SOCKET, (SO_BROADCAST), &sockOptValue, sizeof(int));
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
		//-------------------------------------------------------------
		//*	see if we need to read the external IP list.
		if (gNeedToReadExternalList)
		{
			ReadExternalIPlist_FromThread();
			gNeedToReadExternalList	=	false;
		}

//		CONSOLE_DEBUG("**********************************************************");
		BumpNotSeenCounter();
		//-------------------------------------------------------------
		//*	send the broadcast message to everyone
		//CONSOLE_DEBUG("Calling sendto");
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
		while (timeOutCntr < 2)
		{
			fromlen	=	sizeof(struct sockaddr_in);
			rcvCnt	=	recvfrom(gBroadcastSock, buf, kReceiveBufferSize, 0, (struct sockaddr *)&from, &fromlen);
			if (rcvCnt > 0)
			{
				buf[rcvCnt]	=	0;
				SJP_Init(&jsonParser);
				SJP_ParseData(&jsonParser, buf);
//				SJP_DumpJsonData(&jsonParser);

				AddIPaddressToList(&from, &jsonParser);

				inet_ntop(AF_INET, &(from.sin_addr), ipAddressStr, INET_ADDRSTRLEN);
		//		CONSOLE_DEBUG_W_HEX("from.sin_addr\t=", from.sin_addr);
		//		CONSOLE_DEBUG_W_STR("from.sin_addr\t=", ipAddressStr);
			}
			else if (rcvCnt == 0)
			{
				CONSOLE_DEBUG("no response");
			}
			else
			{
			//	perror("recvfrom");
				timeOutCntr++;
			}
//			CONSOLE_DEBUG_W_HEX("from.sin_addr=", from.sin_addr.s_addr);
//			CONSOLE_DEBUG_W_NUM("from.sin_addr=", ((from.sin_addr.s_addr >> 24) & 0x0ff));
//			CONSOLE_DEBUG_W_NUM("from.sin_addr=", ((from.sin_addr.s_addr >> 16) & 0x0ff));
//			CONSOLE_DEBUG_W_NUM("from.sin_addr=", ((from.sin_addr.s_addr >> 8) & 0x0ff));
//			CONSOLE_DEBUG_W_NUM("from.sin_addr=", ((from.sin_addr.s_addr) & 0x0ff));

		}
		PollAllDevices();

//		PrintDeviceList();

		GetInformationFromOtherDevices();

		//*	we dont need to do this very often
		sleepSecsCounter	=	0;
		while ((sleepSecsCounter < 90) && (gDiscoveryWakeUp == false) && gDiscoveryThreadKeepRunning)
		{
		#define	kSleepTime	1

			sleep(kSleepTime);
			sleepSecsCounter	+=	kSleepTime;
		}
		gDiscoveryWakeUp	=	false;
	//	CONSOLE_DEBUG("Done sleeping");
	}

	CONSOLE_DEBUG("Thread exit!!!!!!!!");

	gDiscoveryThreadIsRunning	=	false;
	return(NULL);
}

//*****************************************************************************
void	WakeUpDiscoveryThread(void)
{
	gDiscoveryWakeUp	=	true;
}

//*****************************************************************************
void	ResetExternalIPaddress(void)
{
	CONSOLE_DEBUG(__FUNCTION__);
	gNeedToReadExternalList	=	true;
}

//*****************************************************************************
static void	ReadExternalIPlist_FromThread(void)
{
FILE				*filePointer;
char				lineBuff[256];
char				externalIPaddrStr[256];
char				outputIPaddrStr[256];
int					ii;
int					slen;
char				fileName[]	=	"external_ip_list.txt";
SJP_Parser_t		jsonParser;
struct sockaddr_in	from;
char				portNumStr[32];
char				*colonPtr;

//	CONSOLE_DEBUG(__FUNCTION__);

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
			slen	=	strlen(lineBuff);
			if ((slen > 6) && (lineBuff[0] != '#'))
			{
//				CONSOLE_DEBUG_W_STR("External IP address\t=",		lineBuff);
				//*	look to see if there is a port number specified
				colonPtr	=	strchr(lineBuff, ':');
				if (colonPtr != NULL)
				{
					colonPtr++;
					strcpy(portNumStr, colonPtr);
//					CONSOLE_DEBUG_W_STR("External Port\t\t=",		portNumStr);
				}
				else
				{
					//*	set the default
				//	strcpy(portNumStr, "6800");
					sprintf(portNumStr, "%d", kAlpacaPiDefaultPORT);
				}
				//*	isolate the IP address string
				strcpy(externalIPaddrStr, lineBuff);
				colonPtr	=	strchr(externalIPaddrStr, ':');
				if (colonPtr != NULL)
				{
					*colonPtr	=	0;
				}

				//*	extract the IP address
				inet_pton(AF_INET, externalIPaddrStr, &(from.sin_addr));

				//*	this is just for debugging to make sure we got it right
				inet_ntop(AF_INET, &(from.sin_addr), outputIPaddrStr, INET_ADDRSTRLEN);
//				CONSOLE_DEBUG_W_STR("outputIPaddrStr\t\t=",		outputIPaddrStr);

				SJP_Init(&jsonParser);
				strcpy(jsonParser.dataList[0].keyword, "ALPACAPORT");
				strcpy(jsonParser.dataList[0].valueString, portNumStr);
				jsonParser.tokenCount_Data	=	1;

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
static void GetMyAddress(void)
{
struct ifaddrs	*ifAddrStruct	=	NULL;
struct ifaddrs	*ifa			=	NULL;
void			*tmpAddrPtr		=	NULL;
char			addressBuffer[256];
//char			msgBuffer[256];
uint32_t		ipAddress32;
bool			keepGoing;

	CONSOLE_DEBUG(__FUNCTION__);

	getifaddrs(&ifAddrStruct);

	if (ifAddrStruct != NULL)
	{
		//*	step thru the linked list of ip addresses
		ifa			=	ifAddrStruct;
		keepGoing	=	true;
		while ((ifa != NULL) && keepGoing)
		{
//			CONSOLE_DEBUG(__FUNCTION__);
			// Check if it is a valid IPv4 address
			if (ifa ->ifa_addr != NULL)
			{
//				CONSOLE_DEBUG_W_NUM("ifa ->ifa_addr->sa_family\t=", ifa ->ifa_addr->sa_family);
				if (ifa ->ifa_addr->sa_family == AF_INET)
				{
//					CONSOLE_DEBUG("AF_INET");
					tmpAddrPtr	=	&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
//					CONSOLE_DEBUG_W_HEX("tmpAddrPtr\t=", tmpAddrPtr);
//					CONSOLE_DEBUG_W_HEX("sin_addr\t=", ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr);

					ipAddress32	=	ntohl(*((uint32_t *)tmpAddrPtr));
//					CONSOLE_DEBUG_W_HEX("ipAddress32\t=", ipAddress32);
					if (ipAddress32 != 0x7f000001)
					{
						gMyIPaddress	=	ipAddress32;
//						CONSOLE_DEBUG_W_HEX("gMyIPaddress\t=", gMyIPaddress);
					}
					inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
		//			printf("%s IPV4 Address %s\n", ifa->ifa_name, addressBuffer);
//					CONSOLE_DEBUG_W_STR("IPV4 Name   :", ifa->ifa_name);
//					CONSOLE_DEBUG_W_STR("IPV4 Address:", addressBuffer);
				}
				else if (ifa->ifa_addr->sa_family == AF_INET6)
				{
//					CONSOLE_DEBUG("AF_INET6");
					// Check if it is a valid IPv6 address
//					tmpAddrPtr	=	&((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
//					inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
//		//			printf("%s IPV6 Address %s\n", ifa->ifa_name, addressBuffer);
//
//					CONSOLE_DEBUG_W_STR("IPV6 Name   :", ifa->ifa_name);
//					CONSOLE_DEBUG_W_STR("IPV6 Address:", addressBuffer);
				}
				else if (ifa ->ifa_addr->sa_family == PF_PACKET)
				{
//					CONSOLE_DEBUG("PF_PACKET");
//					CONSOLE_DEBUG_W_STR("ifa->ifa_name:", ifa->ifa_name);
				}
				else
				{
					CONSOLE_DEBUG_W_STR("Unknown family Name   :", ifa->ifa_name);
				}
//				if (gDebugDiscovery)
//				{
//					CONSOLE_DEBUG("Stepping to Next ip address -----------------------");
//				}
//				CONSOLE_DEBUG("ifa	=	ifa->ifa_next;");
				ifa	=	ifa->ifa_next;
//				CONSOLE_DEBUG(__FUNCTION__);
			}
			else
			{
//				CONSOLE_DEBUG("Null");
				keepGoing	=	false;
			}
		}

//		CONSOLE_DEBUG("Freeing ifAddrStruct");
		freeifaddrs(ifAddrStruct);
	}
	else
	{
		CONSOLE_DEBUG("getifaddrs() Failed!!!!!");
	}
//	inet_ntop(AF_INET, &gMyIPaddress, addressBuffer, INET_ADDRSTRLEN);
//	sprintf(msgBuffer, "Local address:\t%s", addressBuffer);
//	SetStartupText(msgBuffer);

//	CONSOLE_DEBUG_W_HEX("gMyIPaddress\t=", gMyIPaddress);
//	CONSOLE_DEBUG_W_STR(__FUNCTION__, "EXIT");
}

//*****************************************************************************
int StartDiscoveryListenThread(const int alpacaListenPort)
{
int			threadErr;

	CONSOLE_DEBUG(__FUNCTION__);

	//*	yes, we have a separate copy of it here
	gAlpacaListenPort	=	alpacaListenPort;

	GetMyAddress();

	CONSOLE_DEBUG_W_NUM("Staring discovery listen thread on port", gAlpacaListenPort);
	SETUP_TIMING();
	threadErr			=	pthread_create(&gDiscoveryListenThreadID, NULL, &DiscoveryListenThread, NULL);
	if (threadErr == 0)
	{
		CONSOLE_DEBUG("Listen thread created successfully");
	}
	else
	{
		CONSOLE_DEBUG_W_NUM("Error on thread creation, Error number:", threadErr);
	}
	DEBUG_TIMING("Time to create thread (ms)\t=");
	return(threadErr);
}

#if 0

//*****************************************************************************
int	main(int argc, char **argv)
{
	printf("Staring discovery listen thread %s\r\n", __FUNCTION__);

	StartDiscoveryThread(kAlpacaPiDefaultPORT);

	while (1)
	{
		sleep(1);
	}
}
#endif // 0
