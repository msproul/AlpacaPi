//*****************************************************************************
//#include	"slittracker_data.h"

#ifndef _SLITTRACKER_DATA_H_
#define	_SLITTRACKER_DATA_H_

//*****************************************************************************
typedef struct
{
	bool	validData;
	bool	updated;
	double	distanceInches;
	long	readCount;
} TYPE_SLITCLOCK;

#define		kSensorValueCnt	12


extern	TYPE_SLITCLOCK	gSlitDistance[];
extern	int				gSlitLogIdx;


//*****************************************************************************
typedef struct
{
	bool	validData;
	double	distanceInches[kSensorValueCnt];
	double	average20pt[kSensorValueCnt];
} TYPE_SLIT_LOG;

#define	kSlitLogCount	400

extern	TYPE_SLIT_LOG	gSlitLog[kSlitLogCount];


#endif // _SLITTRACKER_DATA_H_
