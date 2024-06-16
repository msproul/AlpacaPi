//**********************************************************************
//#include	"SkyTravelTimeRoutines.h"


#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif // _SKY_STRUCTS_H_



#ifdef __cplusplus
	extern "C" {
#endif

int		Lastday(	TYPE_SkyTime *timeptr);
void	Add_year(	TYPE_SkyTime *timeptr,int delta);
void	Add_month(	TYPE_SkyTime *timeptr);
void	Add_day(	TYPE_SkyTime *timeptr);
void	Add_hour(	TYPE_SkyTime *timeptr);
void	Add_min(	TYPE_SkyTime *timeptr);
void	Add_sec(	TYPE_SkyTime *timeptr);
void	Sub_year(	TYPE_SkyTime *timeptr,int delta);
void	Sub_month(	TYPE_SkyTime *timeptr);
void	Sub_day(	TYPE_SkyTime *timeptr);
void	Sub_hour(	TYPE_SkyTime *timeptr);
void	Sub_min(	TYPE_SkyTime *timeptr);
void	Sub_sec(	TYPE_SkyTime *timeptr);
void	Add_local_year(		TYPE_SkyTime *timeptr, int delta);
void	Add_local_month(	TYPE_SkyTime *timeptr);
void	Add_local_day(		TYPE_SkyTime *timeptr);
void	Sub_local_month(	TYPE_SkyTime *timeptr);
void	Sub_local_day(		TYPE_SkyTime *timeptr);
void	Compute_Timezone(	TYPE_LatLon *locptr, TYPE_SkyTime *timeptr);
void	Local_Time(			TYPE_SkyTime *timeptr);
void	CalanendarTime(		TYPE_SkyTime *timeptr);	//* compute dte and cent
void	ProcessTimeAdjustmentChar(char theChar);
void	SetCurrentTimeStruct(TYPE_SkyTime *timeptr);

void	DumpTimeStruct(		TYPE_SkyTime *timeptr, const char *callingFunctionName);

extern bool	gAutoAdvanceTime;

#ifdef __cplusplus
}
#endif
