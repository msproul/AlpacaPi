//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jan  1,	2021	<MLS> Created lx200_com.h
//*****************************************************************************
//#include	"lx200_com.h"

#ifndef RADIANS
	#define	RADIANS(degrees)	(degrees * M_PI / 180.0)
#endif

#ifndef DEGREES
	#define	DEGREES(radians)	(radians * 180.0 / M_PI)
#endif

#ifdef __cplusplus
	extern "C" {
#endif

int		LX200_StartThread(const char *ipAddrString, const int tcpPort, char *errorMsg);
void	LX200_StopThread(void);

int		LX200_SendCommand(int socket_desc, const char *cmdString, char *dataBuffer, unsigned int dataBufferLen);
bool	LX200_SyncScope(		const double	newRtAscen_Radians,
								const double	new_Declination_Radians,
								char			*returnErrMsg);

bool	LX200_SlewScopeDegrees(	const double	newRtAscen_Hours,
								const double	newDeclination_Degrees,
								char			*returnErrMsg);

bool	LX200_SyncScopeDegrees(	const double	newRtAscen_Hours,
								const double	newDeclination_Degrees,
								char			*returnErrMsg);


bool	LX200_StopMovement(void);


extern	double	gTelescopeRA_Hours;
extern	double	gTelescopeRA_Radians;
extern	double	gTelescopeDecl_Degrees;
extern	double	gTelescopeDecl_Radians;

extern	double	gTelescopeTrackingRate;



extern	bool	gTelescopeUpdated;
extern	bool	gTelescopeInfoValid;
extern	char	gTelescopeRA_String[];
extern	char	gTelescopeDecl_String[];
extern	int		gTelescopeUpdateCnt;
extern	char	gTelescopeErrorString[];

extern	bool	gLX200_ThreadActive;


#ifdef __cplusplus
}
#endif

