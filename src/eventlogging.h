//**************************************************************************
//*	Name:			eventlogging.h
//*
//*	Author:			Mark Sproul
//*
//*****************************************************************************
//#include	"eventlogging.h"


#ifndef _EVENT_LOGGING_H_
#define	_EVENT_LOGGING_H_

#ifndef	_ALPACA_DEFS_H_
	#include	"alpaca_defs.h"
#endif // _ALPACA_DEFS_H_


#ifdef __cplusplus
	extern "C" {
#endif

void	LogEvent(	const char				*eventName,
					const char				*eventDescription,
					const char				*resultString,
					const TYPE_ASCOM_STATUS	alpacaErrCode,
					const char				*errorString);
void	PrintLog(void);
void	SendHtmlLog(int mySocketFD);
#ifdef __cplusplus
}
#endif

#endif	//	_EVENT_LOGGING_H_
