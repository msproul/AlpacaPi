//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jan  1,	2021	<MLS> Created lx200_com.h
//*****************************************************************************
//#include	"lx200_com.h"

#define	RADIANS(degrees)	(degrees * M_PI / 180.0)
#define	DEGREES(radians)	(radians * 180.0 / M_PI)

#ifdef __cplusplus
	extern "C" {
#endif

int		LX200_StartThread(char *errorMsg);
void	LX200_StopThread(void);

int		LX200_OpenSocket(struct sockaddr_in *deviceAddress, const int port);
int		LX200_SendCommand(int socket_desc, const char *cmdString, char *dataBuffer, unsigned int dataBufferLen);
bool	LX200_SyncScope(const double newRtAscen_Radians, const double new_Declination_Radians, char *returnErrMsg);

extern	double	gTelescopeRA_Radians;
extern	double	gTelescopeDecl_Radians;

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

