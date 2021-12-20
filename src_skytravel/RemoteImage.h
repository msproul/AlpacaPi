//*****************************************************************************
//#include	"RemoteImage.h"

enum
{
	kRemoteSrc_stsci_fits	=	0,
	kRemoteSrc_stsci_gif,
	kRemoteSrc_SDSS,

	kRemoteSrc_LAST
};

//*****************************************************************************
typedef struct
{
	int		RequestCount;
	char	LastCmdString[512];

} TYPE_RemoteData;

extern	TYPE_RemoteData	gRemoteDataStats[];
extern	int				gRemoteSourceID;
extern	char			gRemoteImageStatusMsg[];
extern	int				gRemoteGetCounts[];


//*	returns 0=OK, -1, failed to create, +1 busy
int		GetRemoteImage(	double	ra_Radians,
						double	dec_Radians,
						double	arcSecondsPerPixel,
						double	fieldOfView_deg);
bool	IsRemoteImageRunning(void);
bool	RemoteDataMsgUpdated(void);
void	ClearRemoteDataMsgFlag(void);
