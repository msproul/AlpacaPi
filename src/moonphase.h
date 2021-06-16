//*****************************************************************************
//#include	"moonphase.h"


#ifdef __cplusplus
	extern "C" {
#endif

double	CalcDaysSinceNewMoon(const int month, const int day, const int year);
double	CalcMoonIllumination(const int month, const int day, const int year);
void	GetCurrentMoonPhase(char *moonPhaseStr);


#ifdef __cplusplus
}
#endif
