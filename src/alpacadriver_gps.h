//*****************************************************************************
//#include	"alpacadriver_gps.h"



#ifndef _ALPACADRIVER_GPS_H_
#define	_ALPACADRIVER_GPS_H_

#ifndef _REQUESTDATA_H_
	#include	"RequestData.h"
#endif

void	SendHtml_GPS(TYPE_GetPutRequestData *reqData);


extern	bool		gEnableGlobalGPS;
extern	char		gGlobalGPSbaudrate;
extern	char		gGlobalGPSpath[];

#endif // _ALPACADRIVER_GPS_H_
