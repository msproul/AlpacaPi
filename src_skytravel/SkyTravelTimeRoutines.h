//**********************************************************************
//#include	"SkyTravelTimeRoutines.h"


#ifndef _SKY_STRUCTS_H_
	#include	"SkyStruc.h"
#endif // _SKY_STRUCTS_H_



#ifdef __cplusplus
	extern "C" {
#endif

unsigned char Lastday(TYPE_Time *timeptr);
void	Add_year(TYPE_Time *timeptr,int delta);
void	Add_month(TYPE_Time *timeptr);
void	Add_day(TYPE_Time *timeptr);
void	Add_hour(TYPE_Time *timeptr);
void	Add_min(TYPE_Time *timeptr);
void	Add_sec(TYPE_Time *timeptr);
void	Sub_year(TYPE_Time *timeptr,int delta);
void	Sub_month(TYPE_Time *timeptr);
void	Sub_day(TYPE_Time *timeptr);
void	Sub_hour(TYPE_Time *timeptr);
void	Sub_min(TYPE_Time *timeptr);
void	Sub_sec(TYPE_Time *timeptr);
void	Add_local_year(TYPE_Time *timeptr, int delta);
void	Add_local_month(TYPE_Time *timeptr);
void	Add_local_day(TYPE_Time *timeptr);
void	Sub_local_month(TYPE_Time *timeptr);
void	Sub_local_day(TYPE_Time *timeptr);
void	Compute_Timezone(TYPE_LatLon *locptr, TYPE_Time *timeptr);
void	Local_Time(TYPE_Time *timeptr);
void	CalanendarTime(TYPE_Time *timeptr);	//* compute dte and cent

#ifdef __cplusplus
}
#endif
