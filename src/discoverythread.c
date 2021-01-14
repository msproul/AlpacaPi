//*****************************************************************************
//*	Alpaca discovery thread
//*
//*	based on proposal by Daniel Van Noord
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
//*-------------------------------------------------------------------------
//*	Jan 13,	2121	<TODO> Add external IP list to discovery thread
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

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"



#ifndef SO_REUSEPORT
	#define	SO_REUSEPORT 1
#endif

#include	"json_parse.h"
#include	"discoverythread.h"
#include	"discovery_lib.h"
#include	"sendrequest_lib.h"

#include	"obsconditions_globals.h"


#define kAlpacaDiscoveryPORT	32227

#define	kReceiveBufferSize	1550



pthread_t	gDiscoveryListenThreadID;
pthread_t	gDiscoveryFindThreadID;
int			gAlpacaListenPort	=	9999;

//#define	kMaxDeviceListCnt	32

//*	this is a list of IP addresses
TYPE_ALPACA_UNIT	gAlpacaUnitList[kMaxAlpacaIPaddrCnt];
int					gAlpacaUnitCnt	=	0;


//*	this is a list of alpaca devices, can be more than one per IP address
TYPE_REMOTE_DEV		gRemoteList[kMaxDeviceListCnt];
int					gRemoteCnt		=	0;

static	int			gBroadcastSock;
static	uint32_t	gMyIPaddress	=	0;


static void	*LookForAlpacaDevicesThread(void *arg);


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

//	CONSOLE_DEBUG(__FUNCTION__);
	printf("Staring discovery listen thread %s\r\n", __FUNCTION__);

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
				if (strncmp(readBuf,	"alpaca discovery", 16) == 0)
				{
					CONSOLE_DEBUG_W_STR("Old style discovery request\t=", readBuf);
					validDiscoveryRequest	=	true;
				}
				if (strncasecmp(readBuf,	"alpacadiscovery",  15) == 0)
				{
					validDiscoveryRequest	=	true;
				}
				//*	double check just to make sure
				if ((validDiscoveryRequest == false) && (strncmp(readBuf, "alpaca", 6) == 0))
				{
					CONSOLE_DEBUG_W_STR("Incomplete discovery request\t=", readBuf);
					validDiscoveryRequest	=	true;
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
				//	printf("Unknown request====%s\r\n", readBuf);
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

	//*	dont add myself to the list
	if (newRemoteDevice->deviceAddress.sin_addr.s_addr == gMyIPaddress)
	{
//		CONSOLE_DEBUG("Leave me out of the list");
		newDevice	=	false;
	}
	else
	{
		//*	look to see if it is already in the list
		for (iii=0; iii<gRemoteCnt; iii++)
		{
			if ((newRemoteDevice->deviceAddress.sin_addr.s_addr ==	gRemoteList[iii].deviceAddress.sin_addr.s_addr)
				&& (strcmp(newRemoteDevice->deviceType,				gRemoteList[iii].deviceType) == 0)
				&& (strcmp(newRemoteDevice->deviceName,				gRemoteList[iii].deviceName) == 0)
				&& (newRemoteDevice->alpacaDeviceNum == gRemoteList[iii].alpacaDeviceNum)
				)
			{
				gRemoteList[iii].notSeenCounter	=	0;
				newDevice	=	false;
				break;
			}
			//*	I dont want the management device type in the list
			if (strcmp(newRemoteDevice->deviceType, "management") == 0)
			{
				newDevice	=	false;
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
			gRemoteCnt++;
		}
	}
}


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
		if (strcmp(jsonParser->dataList[ii].keyword, "VERSION") == 0)
		{
			strcpy(myVersionString, jsonParser->dataList[ii].valueString);
		}

		if (strcmp(jsonParser->dataList[ii].keyword, "DEVICETYPE") == 0)
		{
			strcpy(myRemoteDevice.deviceType, jsonParser->dataList[ii].valueString);
		}
		if (strcmp(jsonParser->dataList[ii].keyword, "DEVICENAME") == 0)
		{
			strcpy(myRemoteDevice.deviceName, jsonParser->dataList[ii].valueString);
		}
		if (strcmp(jsonParser->dataList[ii].keyword, "DEVICENUMBER") == 0)
		{
			myRemoteDevice.alpacaDeviceNum	=	atoi(jsonParser->dataList[ii].valueString);
		}

		//------------------------------------
		if (strcmp(jsonParser->dataList[ii].keyword, "ARRAY-NEXT") == 0)
		{
			myRemoteDevice.deviceAddress	=	theDevice->deviceAddress;
			myRemoteDevice.port				=	theDevice->port;
			strcpy(myRemoteDevice.versionString, myVersionString);
			UpdateRemoteList(&myRemoteDevice);

			memset(&myRemoteDevice, 0, sizeof(TYPE_REMOTE_DEV));

		}
	}
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
int					recvByteCnt;
char				returnedData[2000];
char				xmitBuffer[2000];


//	CONSOLE_DEBUG(__FUNCTION__);
	validData	=	false;
	socket_desc	=	socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc >= 0)
	{
		remoteDev.sin_addr.s_addr	=	deviceAddress->sin_addr.s_addr;
		remoteDev.sin_family		=	AF_INET;
		remoteDev.sin_port			=	htons(port);
		//*	Connect to remote device
		connRetCode	=	connect(socket_desc , (struct sockaddr *)&remoteDev , sizeof(remoteDev));
		if (connRetCode >= 0)
		{
			strcpy(xmitBuffer, "GET ");
			strcat(xmitBuffer, sendData);
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
			closeRetCode	=	close(socket_desc);
			if (closeRetCode != 0)
			{
				CONSOLE_DEBUG("Close error");
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
	}
	return(validData);
}


//*****************************************************************************
static void	SendGetRequest(TYPE_ALPACA_UNIT *theDevice, const char *sendData)
{
bool				validData;
SJP_Parser_t		jsonParser;

//	CONSOLE_DEBUG(__FUNCTION__);

	validData	=	GetJsonResponse(	&theDevice->deviceAddress,
										theDevice->port,
										sendData,
										&jsonParser);
	if (validData)
	{
//		SJP_DumpJsonData(&jsonParser);

		ExtractDevicesFromJSON(&jsonParser, theDevice);
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

	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_NUM("gAlpacaUnitCnt\t=", gAlpacaUnitCnt);
	for (ii=0; ii<gAlpacaUnitCnt; ii++)
	{
		if (gAlpacaUnitList[ii].noResponseCnt == 0)
		{
			SendGetRequest(&gAlpacaUnitList[ii], "/management/v1/configureddevices");
		}
	}
	CONSOLE_DEBUG_W_NUM("gRemoteCnt\t=", gRemoteCnt);
}

//*****************************************************************************
static void	AddIPaddressToList(struct sockaddr_in *deviceAddress, SJP_Parser_t *jsonParser)
{
int		ii;
bool	newDevice;
int		theDeviceIdx;

	CONSOLE_DEBUG(__FUNCTION__);
	newDevice		=	true;
	theDeviceIdx	=	-1;
	for (ii=0; ii<gAlpacaUnitCnt; ii++)
	{
		if (deviceAddress->sin_addr.s_addr == gAlpacaUnitList[ii].deviceAddress.sin_addr.s_addr)
		{
			newDevice		=	false;
			theDeviceIdx	=	ii;
			break;
		}
	}
	if (newDevice)
	{
		//*	add the new devices to our list
		CONSOLE_DEBUG("We have a new devices")
		if (gAlpacaUnitCnt < kMaxAlpacaIPaddrCnt)
		{
			gAlpacaUnitList[gAlpacaUnitCnt].deviceAddress	=	*deviceAddress;
			//*	now find the alpaca port
			for (ii=0; ii<jsonParser->tokenCount_Data; ii++)
			{
				if (strcasecmp(jsonParser->dataList[ii].keyword, "ALPACAPORT") == 0)
				{
					gAlpacaUnitList[gAlpacaUnitCnt].port	=	atoi(jsonParser->dataList[ii].valueString);
					theDeviceIdx	=	gAlpacaUnitCnt;
				}
			}
			gAlpacaUnitCnt++;
		}
		else
		{
			CONSOLE_DEBUG("Ran out of space in gAlpacaUnitList")
		}
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

		printf("%-20s\t",	gRemoteList[ii].deviceType);
		printf("%-20s\t",	gRemoteList[ii].deviceName);
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
	char			outputString[128];

#endif // _ENABLE_CAMERA_

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_NUM("gRemoteCnt\t=", gRemoteCnt);
	for (ii=0; ii<gRemoteCnt; ii++)
	{
	#ifdef _ENABLE_SKYTRAVEL_
	//	CONSOLE_DEBUG_W_STR("device type\t=", gRemoteList[ii].deviceType);

//		inet_ntop(AF_INET, &gRemoteList[ii].deviceAddress.sin_addr, ipAddressStr, INET_ADDRSTRLEN);

//		printf("%-17s\t%-17s\t%-17s\t%-17s\t\r\n",	ipAddressStr,
//													gRemoteList[ii].deviceType,
//													gRemoteList[ii].deviceName,
//													gRemoteList[ii].versionString);
	#endif
	#ifdef _ENABLE_CAMERA_
		if ((gRemoteList[ii].notSeenCounter == 0) &&
			(strcmp(gRemoteList[ii].deviceType, "observingconditions") == 0))
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
		}
	#endif
	}
	CONSOLE_DEBUG_W_STR(__FUNCTION__, "Exit");
}

//*****************************************************************************
int StartDiscoveryQuerryThread(void)
{
int			threadErr;

//	CONSOLE_DEBUG(__FUNCTION__);
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
struct timeval		timeoutLength;
int					timeOutCntr;
int					sockOptValue;

	CONSOLE_DEBUG(__FUNCTION__);
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
	timeoutLength.tv_sec	=	2;
	timeoutLength.tv_usec	=	0;
	setOptRetCode			=	setsockopt(	gBroadcastSock,
											SOL_SOCKET,
											SO_RCVTIMEO,
											&timeoutLength,
											sizeof(timeoutLength));


	bindRetCode	=	bind(gBroadcastSock, (const struct sockaddr *)&cliaddr, sizeof(cliaddr));
	if (bindRetCode < 0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	sendtoRetCode	=	0;
	while (sendtoRetCode >= 0)
	{
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
				CONSOLE_DEBUG_W_STR("from.sin_addr\t=", ipAddressStr);
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

		PollAllDevices();

//		PrintDeviceList();

		GetInformationFromOtherDevices();

		//*	we dont need to do this very often
	//	sleep(3000);
		sleep(15);
		CONSOLE_DEBUG("Done sleeping");
	}

	CONSOLE_DEBUG("Thread exit!!!!!!!!");

	return(NULL);
}




#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

//*****************************************************************************
int	GetMySubnetNumber(void)
{
	return(gMyIPaddress & 0x00ff);
}

//*****************************************************************************
static void GetMyAddress(void)
{
struct ifaddrs	*ifAddrStruct	=	NULL;
struct ifaddrs	*ifa			=	NULL;
void			*tmpAddrPtr		=	NULL;
char			addressBuffer[INET_ADDRSTRLEN * 3];
uint32_t		ipAddress32;

//	CONSOLE_DEBUG(__FUNCTION__);

	getifaddrs(&ifAddrStruct);

	//*	step thru the linked list of ip addresses
	ifa	=	ifAddrStruct;
	while (ifa != NULL)
	{
		// Check if it is a valid IPv4 address
		if (ifa ->ifa_addr->sa_family == AF_INET)
		{
			tmpAddrPtr	=	&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;

			ipAddress32	=	ntohl(*((uint32_t *)tmpAddrPtr));
			CONSOLE_DEBUG_W_HEX("ipAddress32\t=", ipAddress32);
	//		if (ipAddress32 != 0x0100007f)
			if (ipAddress32 != 0x7f000001)
			{
				gMyIPaddress	=	ipAddress32;
				CONSOLE_DEBUG_W_HEX("gMyIPaddress\t=", gMyIPaddress);
			}
			inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
			printf("%s IPV4 Address %s\n", ifa->ifa_name, addressBuffer);
		}
		// Check if it is a valid IPv6 address
//		else if (ifa->ifa_addr->sa_family==AF_INET6)
//		{
//			tmpAddrPtr = &((struct sockaddr_in6 *)ifa->ifa_addr)->sin6_addr;
//			char addressBuffer[INET6_ADDRSTRLEN];
//			inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
//			printf("%s IPV6 Address %s\n", ifa->ifa_name, addressBuffer);
//		}
		ifa	=	ifa->ifa_next;
	}
	if (ifAddrStruct != NULL)
	{
		freeifaddrs(ifAddrStruct);
	}
//	exit(0);
}

//*****************************************************************************
int StartDiscoveryListenThread(int alpacaListenPort)
{
int			threadErr;

	GetMyAddress();

//	printf("Staring discovery listen thread %s\r\n", __FUNCTION__);
	gAlpacaListenPort	=	alpacaListenPort;
	threadErr			=	pthread_create(&gDiscoveryListenThreadID, NULL, &DiscoveryListenThread, NULL);

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
