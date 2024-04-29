//#include	"GPS_graph.h"


int		SaveGPS_HTMLandGRAPHS(const char *imageFolderName, const char *htmlFileName, bool dataIsLive);
void	DrawGPS_AlmanacGrid(void);
void	CreateSatelliteTrailsGraph(		FILE *htmlFile, const char *imageFolderName, const char *gpsGraphFileName);
void	CreateLatLonHistoryPlot(		FILE *htmlFile, const char *imageFolderName, const char *latlonGraphFileName);
void	CreatePDOPhistoryPlot(			FILE *htmlFile, const char *imageFolderName, const char *pdopGraphFileName);
void	CreatePositionErrorHistoryPlot(	FILE *htmlFile, const char *imageFolderName, const char *posErrGraphFileName);
void	CreateAltitudeHistoryPlot(		FILE *htmlFile, const char *imageFolderName, const char *altGraphFileName);
void	CreateSNRdistrbutionPlot(		FILE *htmlFile, const char *imageFolderName, const char *snrGraphFileName);
void	CreateSatsInUseHistoryPlot(		FILE *htmlFile, const char *imageFolderName, const char *satsInUseGraphFileName);
void	CreateSatelliteElevationGraph(	FILE *htmlFile, const char *imageFolderName, const char *elevationGraphFileName);

#ifdef _ENABLE_SATELLITE_ALMANAC_
	void	DisplayGPSalmanac(TYPE_SatStatsStruct *theSatData, bool showSatNum);
#endif // _ENABLE_SATELLITE_ALMANAC_


//*****************************************************************************
const char	elevationGraphFileName[]	=	"gps-elevation.jpg";
const char	snrGraphFileName[]			=	"gps-snrplot.jpg";
const char	pdopGraphFileName[]			=	"gps-pdop.jpg";
const char	altGraphFileName[]			=	"gps-alt.jpg";
const char	latlonGraphFileName[]		=	"gps-latlon.jpg";
const char	satsInUseGraphFileName[]	=	"gps-satsInUse.jpg";
const char	posErrGraphFileName[]		=	"gps-posErr.jpg";
const char	magVarGraphFileName[]		=	"gps-magvar.jpg";
