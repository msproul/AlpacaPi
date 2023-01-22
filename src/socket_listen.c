//**************************************************************************
//*	Name:			socket_listen.c
//*
//*	Author:			Mark Sproul
//*
//*	Description:
//*
//*	Limitations:
//*
//*	Usage notes:
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Feb 14,	2019	<MLS> Started on socket_listen.c
//*	Apr  9,	2019	<MLS> Added SocketListen_SetCallback()
//*	Apr  9,	2019	<MLS> Got polling version to work
//*	May 25,	2019	<MLS> Fixed EIVAL errors on accept
//*	Mar 31,	2020	<MLS> Updated socket receive code to have a timeout and do multiple reads
//*	Apr  7,	2020	<MLS> Added _FIX_ESCAPE_CHARS_ compile flag
//*	Apr  7,	2020	<MLS> Added bytesRead to callback function
//*	Feb 10,	2021	<MLS> Reduced timeout from 150000 to 15000 (micro-secs)
//*	Feb 10,	2021	<MLS> Reduced timeout to 10000 (micro-secs)
//*	Feb 10,	2021	<MLS> Reduced timeout to 2500 (micro-secs)
//*	Dec  3,	2022	<MLS> Added ipAddressString to SendDataToSocket()
//*****************************************************************************


#ifdef _ALPACA_PI_
	#define	_FIX_ESCAPE_CHARS_
#endif // _ALPACA_PI_

//*****************************************************************************
#include	<stdlib.h>
#include	<string.h>
#include	<strings.h>
#include	<unistd.h>
#include	<errno.h>
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<netinet/in.h>

#ifdef _BANDWIDTH_
//	#define _ENABLE_CONSOLE_DEBUG_
#endif // _BANDWIDTH_

//#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"socket_listen.h"

#define		kTimeOut_MicroSecs	2500

SocketData_Callback			gSocketCallbackProcPtr		=	NULL;

//*****************************************************************************
//*	globals so we can make this code non-blocking
static	int		gSocketFD;		//*	socket File Descriptor

void SendDataToSocket(const int sock, const char *ipAddressString);


//*****************************************************************************
static void error(char *msg)
{
	perror(msg);
	exit(1);
}


//*****************************************************************************
int SocketListen_Init(const int listenPortNum)
{
int					bindRetCode;
int					listenRetCode;
struct	sockaddr_in serv_addr;

	CONSOLE_DEBUG(__FUNCTION__);

	gSocketFD	=	socket(AF_INET, SOCK_STREAM, 0);
	if (gSocketFD < 0)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		error("ERROR opening socket");
	}
	CONSOLE_DEBUG_W_NUM("gSocketFD\t=", gSocketFD);
	CONSOLE_DEBUG_W_NUM("listenPortNum\t=", listenPortNum);
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family		=	AF_INET;
	serv_addr.sin_addr.s_addr	=	INADDR_ANY;
	serv_addr.sin_port			=	htons(listenPortNum);

	bindRetCode	=	bind(gSocketFD, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (bindRetCode < 0)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		error("ERROR on binding");
	}
	listenRetCode	=	listen(gSocketFD, 5);

	return(listenRetCode);
}

//*****************************************************************************
//void	SocketListen_SetCallback(SocketData_Callback *callBackPtr)
void	SocketListen_SetCallback(SocketData_Callback callBackPtr)
{
//	CONSOLE_DEBUG(__FUNCTION__);
	gSocketCallbackProcPtr	=	callBackPtr;
}




//*****************************************************************************
int SocketListen_Poll(void)
{
int					newsockfd;
unsigned int		clilen;
struct	sockaddr_in	client_addr;
int					closeRetCode;
int					shutDownRetCode;
char				ipAddrString[64];

	//*	Started getting EINVAL (Invalid argument) errors on accept
	//*	fixed the problem by cleared args first
	memset(&client_addr, 0, sizeof(struct	sockaddr_in));

	clilen		=	sizeof(client_addr);
	newsockfd	=	accept(gSocketFD, (struct sockaddr *) &client_addr, &clilen);

	inet_ntop(AF_INET, &(client_addr.sin_addr), ipAddrString, INET_ADDRSTRLEN);
//	CONSOLE_DEBUG_W_STR("Accepted from ", ipAddrString);
	if (newsockfd >= 0)
	{
		SendDataToSocket(newsockfd, ipAddrString);

		shutDownRetCode	=	shutdown(newsockfd, SHUT_RDWR);
		if (shutDownRetCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("shutDownRetCode\t=", shutDownRetCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}
		closeRetCode	=	close(newsockfd);
		if (closeRetCode != 0)
		{
			CONSOLE_DEBUG_W_NUM("Error closing socket\t=",	closeRetCode);
			CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		}
	}
	else if (newsockfd < 0)
	{
		CONSOLE_DEBUG(__FUNCTION__);
		CONSOLE_DEBUG_W_NUM("gSocketFD\t=", gSocketFD);
		CONSOLE_DEBUG_W_NUM("newsockfd\t=", newsockfd);
		CONSOLE_DEBUG_W_NUM("errno\t=", errno);
		error("ERROR on accept");
	}

	return 0;
}

#ifdef _FIX_ESCAPE_CHARS_
//*****************************************************************************
static int	FixEscapedChars(char *buffer)
{
int		cc;
int		ii;
int		sLen;
char	hi4bits;
char	lo4bits;
int		escCharCnt;

//	CONSOLE_DEBUG("---------------------------------------------------------");
//	CONSOLE_DEBUG_W_STR("buffer\t=", buffer);
	ii			=	0;
	cc			=	0;
	escCharCnt	=	0;
	sLen		=	strlen(buffer);
	while (ii<sLen)
	{
		if (buffer[ii] == '%')
		{
			ii++;
			hi4bits			=	(buffer[ii++] & 0x0f) << 4;
			lo4bits			=	buffer[ii++];
			if (lo4bits >= 'A')
			{
				lo4bits	=	9 + (lo4bits & 0x0f);
			}
			else
			{
				lo4bits	=	lo4bits & 0x0f;
			}
			buffer[cc++]	=	hi4bits + lo4bits;

			escCharCnt++;
		}
		else
		{
			buffer[cc++]	=	buffer[ii++];
		}
	}
	buffer[cc]	=	0;

	sLen	=	strlen(buffer);

//	if (escCharCnt > 0)
//	{
//	//	printf("%s\tescCharCnt=%d\r\n", __FUNCTION__, escCharCnt);
//		CONSOLE_DEBUG_W_STR("buffer\t=", buffer);
//		CONSOLE_DEBUG_W_NUM("escCharCnt\t=", escCharCnt);
//	}
	return(sLen);
}
#endif // _FIX_ESCAPE_CHARS_


int	gMessageCnt	=	1;

#define	kReadBuffLen	2048
#define	kMaxResponseLen	2048

#ifdef _BANDWIDTH_

#warning "_BANDWIDTH_ is defined"
#error "_BANDWIDTH_ is defined"

//*****************************************************************************
//*	SendDataToSocket()
//*		There is a separate instance of this function
//*		for each connection.  It handles all communication
//*		once a connection has been established.
//*****************************************************************************
void SendDataToSocket(const int sock, const char *ipAddressString)
{
int				bytesRead;
char			readBuffer[kReadBuffLen];
struct timeval	timeoutLength;
int				setOptRetCode;

	CONSOLE_DEBUG(__FUNCTION__);

	memset(readBuffer, 0, kReadBuffLen);

	//*	set a timeout
	timeoutLength.tv_sec	=	0;
	timeoutLength.tv_usec	=	kTimeOut_MicroSecs;
	setOptRetCode			=	setsockopt(	sock,
											SOL_SOCKET,
											SO_RCVTIMEO,
											&timeoutLength,
											sizeof(timeoutLength));
	if (setOptRetCode != 0)
	{
		CONSOLE_DEBUG_W_NUM("setsockopt() returned", setOptRetCode);
	}


	do
	{
		bytesRead	=	read(sock, readBuffer, (kReadBuffLen - 2));
//		CONSOLE_DEBUG_W_NUM("bytesRead=", bytesRead);
		if (gSocketCallbackProcPtr != NULL)
		{
			CONSOLE_DEBUG("Calling gSocketCallbackProcPtr");
			gSocketCallbackProcPtr(sock, readBuffer, bytesRead);
		}
	} while (bytesRead > 0);


	gMessageCnt++;
//	CONSOLE_DEBUG("EXIT");
}

#else

//*****************************************************************************
//*	SendDataToSocket()
//*		There is a separate instance of this function
//*		for each connection.  It handles all communication
//*		once a connection has been established.
//*****************************************************************************
void SendDataToSocket(const int sock, const char *ipAddressString)
{
int				bytesRead;
//int			bytesWritten;
//int			resultsMsgLen;
char			readBuffer[kReadBuffLen];
char			htmlBuffer[kReadBuffLen];
struct timeval	timeoutLength;
int				setOptRetCode;

//	CONSOLE_DEBUG(__FUNCTION__);

	memset(readBuffer, 0, kReadBuffLen);
	//*	3/31/2020, trying to see if we are missing anything
	memset(htmlBuffer, 0, kReadBuffLen);
	//*	set a timeout
	timeoutLength.tv_sec	=	0;
	timeoutLength.tv_usec	=	kTimeOut_MicroSecs;
	setOptRetCode			=	setsockopt(	sock,
											SOL_SOCKET,
											SO_RCVTIMEO,
											&timeoutLength,
											sizeof(timeoutLength));
	if (setOptRetCode != 0)
	{
		CONSOLE_DEBUG_W_NUM("setsockopt() returned", setOptRetCode);
	}

	do
	{
		bytesRead	=	read(sock, readBuffer, (kReadBuffLen - 2));
//		CONSOLE_DEBUG_W_NUM("bytesRead=", bytesRead);
		if (bytesRead > 0)
		{
			readBuffer[bytesRead]	=	0;
			strcat(htmlBuffer, readBuffer);
		}
		else if (bytesRead < 0)
		{
		//	error("ERROR reading from socket");
		}
	} while (bytesRead > 0);

#ifdef _FIX_ESCAPE_CHARS_
//	CONSOLE_DEBUG_W_NUM("bytesRead=", bytesRead);
	bytesRead	=	FixEscapedChars(htmlBuffer);
#endif
	if (gSocketCallbackProcPtr != NULL)
	{
//		CONSOLE_DEBUG("Calling gSocketCallbackProcPtr");
		gSocketCallbackProcPtr(sock, htmlBuffer, bytesRead, ipAddressString);
	}

//	printf("Here is the message (%d bytes long):\r\n%s\r\n", bytesRead, readBuffer);
#if 0
	resultsMsgLen	=	strlen(returnBuffer);
	bytesWritten	=	write(sock, returnBuffer, resultsMsgLen);
#endif


	gMessageCnt++;
//	CONSOLE_DEBUG("EXIT");
}
#endif // _BANDWIDTH_
