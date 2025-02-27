#ifndef MoonRise_h
#define MoonRise_h

#include <time.h>

//*****************************************************************************
// Size of event search window in hours.
// Events further away from the search time than MR_WINDOW/2 will not be
// found.  At higher latitudes the moon rise/set intervals become larger, so if
// you want to find the nearest events this will need to increase.  Larger
// windows will increase interpolation error.  Useful values are probably from
// 12 - 48 but will depend upon your application.
//*****************************************************************************

#define	MR_WINDOW	48		//	Even integer

//*****************************************************************************
class MoonRise
{
	public:
		time_t	queryTime;
		time_t	riseTime;
		time_t	setTime;
		float	riseAz;
		float	setAz;
		bool	hasRise;
		bool	hasSet;
		bool	isVisible;

		void	calculate(double latitude, double longitude, time_t t);

	private:
		void	testMoonRiseSet(int i, double offsetDays, double latitude, double longitude,
								struct skyCoordinates *mp);
		struct	skyCoordinates moon(double dayOffset);
		double	interpolate(double f0, double f1, double f2, double p);
		double	julianDate(time_t t);
		double	localSiderealTime(double offsetDays, double longitude);
		void	initClass();
};
#endif
