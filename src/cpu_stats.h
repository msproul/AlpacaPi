//*****************************************************************************
//#include	"cpu_stats.h"

#ifndef _CPU_STATS_H
#define	_CPU_STATS_H

#ifdef __cplusplus
	extern "C" {
#endif


double		CPUstats_GetTemperature(char *theTempString);
uint32_t	CPUstats_GetUptime(void);
uint32_t	CPUstats_GetTotalRam(void);
uint32_t	CPUstats_GetFreeRam(void);
uint32_t	CPUstats_GetFreeDiskSpace(const char* path);


#ifdef __cplusplus
}
#endif


#endif // _CPU_STATS_H
