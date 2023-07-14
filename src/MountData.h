//*****************************************************************************
//#include	"MountData.h"


#define	kSecondsPerDay		(86400)
#define	kMaxMountData		(kSecondsPerDay / 4)
extern double	gHourAngleData[];
extern double	gRightAsceData[];
extern double	gDeclinationData[];

//*****************************************************************************
enum
{
	kMountData_HA	=	0,
	kMountData_RA,
	kMountData_DEC
};

#ifdef __cplusplus
	extern "C" {
#endif

void	MountData_Init(void);

#ifdef __cplusplus
}
#endif
