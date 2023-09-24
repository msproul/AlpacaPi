//#include	"NMEA_helper.h"


short	Hextoi(const char theHexChar);
short	CalculateNMEACheckSum(const char *theNmeaLine);
short	ExtractChecksumFromNMEAline(const char *theNmeaLine);
