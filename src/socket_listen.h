//**************************************************************************
//*	Name:			socket_listen.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	<MLS>	=	Mark L Sproul
//*****************************************************************************
//*	Feb 14,	2019	<MLS> Started on socket_listen.h
//*****************************************************************************


#ifndef _SOCKET_LISTEN_H_
#define	_SOCKET_LISTEN_H_


#ifdef __cplusplus
	extern "C" {
#endif
typedef	int (*SocketData_Callback)(int socket, char *htmlData, long bytesRead, const char *ipAddressString);

int		SocketListen_Init(const int listenPortNum);
int		SocketListen_Poll(void);
void	SocketListen_SetCallback(SocketData_Callback callBackPtr);

#ifdef __cplusplus
}
#endif


#endif		//	_SOCKET_LISTEN_H_
