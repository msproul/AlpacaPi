//*****************************************************************************
//#include	"MountData.h"


#define	kSecondsPerDay		(86400)
#define	kMaxMountData		(kSecondsPerDay / 4)
extern double	gHourAngleData[];
extern double	gRightAsceData[];
extern double	gDeclinationData[];

#ifdef __cplusplus
	extern "C" {
#endif

void	MountData_Init(void);

#ifdef __cplusplus
}
#endif
