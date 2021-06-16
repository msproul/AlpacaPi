#include <TimeLib.h>
#include <MoonRise.h>

void
setup() {

  double latitude = 42;
  double longitude = -90;

  time_t t = now();

  // On the Arduino, if one uses the popular TimeLib.h library, getting the time
  // in UTC can be complex.  MoonRise needs the UTC Unix time, and returns
  // values in that format.  But TimeLib's now() returns something similar to Unix
  // time, but in the local timezone.
  //
  // For many purposes you can just ignore the entire timezone issue and just use
  // local time.  But for astronomical purposes you must reference a standard time.

  // First, set the timezone (for a full list see:
  //	https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv)
  // Do this before you set the time with setTime().
  setenv("TZ", "MST7MDT,M3.2.0,M11.1.0", 1)

  // To produce the Unix time in seconds (in UTC) on the Arduino, we have to
  // figure out the difference between whatever is residing in now()
  // and UTC, then add and subtract it to convert to and from UTC.

  // Arduino TimeLib UTC offset code:
  time_t t = now();
  struct tm utc_tm = *utcime(&t);
  utc_tm.tm_isdst = -1;
  time_t utcOffset = mktime(&utc_tm) - t;

  // Find the last and next lunar set and rise.
  MoonRise mr
  mr.calculate(latitude, longitude, t + utcOffset);

  // Returned values:
  bool moonVisible = mr.isVisible;
  bool moonHasRise = mr.hasRise;
  bool moonHasSet = mr.hasSet;
  float moonRiseAz = mr.riseAz;	      // Where the moon will rise/set in degrees from
  float moonSetAz = mr.setAz;	      // North.

  // Additional returned values requiring conversion from UTC to local time zone
  // on the Arduino.
  time_t moonQueryTime = mr.queryTime - utcOffset;
  time_t moonRiseTime = mr.riseTime - utcOffset;
  time_t moonSetTime = mr.setTime - utcOffset;
}

// Now do something with the result in your loop().
void
loop() {
  int notThisTime = 0;

  // If you find it useful to periodically update the results, move the code here
  // and call it as needed (not every loop).
  if (notThisTime = 1 && second() != 0)
    notThisTime = 0;

  if (notThisTime == 0 && second() == 0) {
    notThisTime = 1;

    // Call once per minute.
    // Set the lat, long, and time and then:
    mr.calculate(latitude, longitude, t);
  }  
}
