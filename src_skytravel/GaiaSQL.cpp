//*****************************************************************************
//*		GaiaSQL.cpp		(c) 2021 by Mark Sproul
//*
//*	Description:
//*
//*****************************************************************************
//*	AlpacaPi is an open source project written in C/C++
//*
//*	Use of this source code for private or individual use is granted
//*	Use of this source code, in whole or in part for commercial purpose requires
//*	written agreement in advance.
//*
//*	You may use or modify this source code in any way you find useful, provided
//*	that you agree that the author(s) have no warranty, obligations or liability.  You
//*	must determine the suitability of this source code for your use.
//*
//*	Redistribution of this source code must retain this copyright notice.
//*****************************************************************************
//*	Edit History
//*****************************************************************************
//*	Jan  7,	2022	<MLS> Created GaiaSQL.cpp
//*	Jan  7,	2022	<KAS> Added SQL code
//*	Jan  8,	2022	<MLS> Standalone SQL query working to Keith's house
//*	Jan  8,	2022	<MLS> Dynamic purge of SQL data working
//*	Jan 12,	2022	<MLS> Added CalcRA_DEC_Distance_Deg()
//*	Jan 16,	2022	<MLS> Now sorting Gaia remote data to speed up drawing
//*	Jan 19,	2022	<MLS> Added SQL logging
//*	Jan 25,	2022	<MLS> Changed UpdateSkyTravelView() to use degrees
//*	Jan 25,	2022	<MLS> Fixed 0 <-> 360 boundary bug in CalcRA_DEC_Distance_Deg()
//*	Feb 11,	2022	<MLS> Added CheckSQLconfiguration()
//*	Feb 12,	2022	<MLS> Added GetSQLdataFromIDnumber()
//*	Feb 22,	2022	<KAS> Fixed dangling result that caused the server to log an error
//*	Mar 13,	2022	<MLS> Added support for multiple database names
//*	Mar 14,	2022	<MLS> Starting to make SQL routines more generic, i.e. remove "GAIA"
//*	Mar 24,	2022	<MLS> SQL now working on Raspberry Pi with "mariadb"
//*	Mar 25,	2022	<MLS> First external user accessing Gaia database (Ron S.)
//*	Mar 26,	2022	<MLS> Added the sql library version string to the login string
//*	May  9,	2022	<MLS> Added skytravel version number to remote SQL login information
//*	May 10,	2022	<MLS> Added mysql error strings to local logging function
//*	Jun  8,	2022	<MLS> Updated mysql_real_connect() calls to use specified port number
//*	Jun 27,	2022	<MLS> Switched parsing to use ReadGenericConfigFile()
//*	Sep 22,	2022	<MLS> Added UpdateDataBaseListFromServer()
//*	Sep 22,	2022	<MLS> Query of "Stars" database to get list of databases fully working
//*	Sep 24,	2022	<MLS> Database list now defaults to "gaia" and "2mass"
//*	Nov 11,	2022	<MLS> Added return error msg to GetSQLdataFromIDnumber()
//*	Nov 24,	2022	<KAS> Worked on cleaning up SQL Intiliazation as per Mark's request
//*****************************************************************************
//*	sudo apt-get install libmysqlclient-dev		<<<< Use this one
//*	sudo apt-get install libmariadb-dev			<<<< Use this for Raspberry-Pi
//*****************************************************************************
//*	https://www.cosmos.esa.int/web/gaia/earlydr3
//*	https://gea.esac.esa.int/archive/documentation/GEDR3/Gaia_archive/chap_datamodel/sec_dm_main_tables/ssec_dm_gaia_source.html
//*		GAIA ER3 is JD2016.0
//*****************************************************************************

#ifdef _ENABLE_REMOTE_GAIA_

#include	<stdlib.h>
#include	<string.h>
#include	<math.h>
#include	<pthread.h>
#include	<unistd.h>

//	<KAS>	For SQL Functions
//*	this is defined in the Makefile to select which version of sql to use
#if defined(_SQL_mysqlclient)
	#include	<mysql/mysql.h>
#elif defined(_SQL_mariadb)
	#include	<mariadb/mysql.h>
#else
	#error	"Unable to determine SQL library version"
#endif


#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

//#define	_VERBOSE_SQL_DEBUG_

#include	"controller_skytravel.h"
#include	"helper_functions.h"
#include	"SkyStruc.h"
#include	"RemoteImage.h"
#include	"observatory_settings.h"
#include	"readconfigfile.h"

#include	"GaiaSQL.h"
#include	"RemoteGaia.h"


#define	_XREF_AAVSO_

char	gSQLsever_IPaddr[32]	=	"";
int		gSQLsever_Port			=	4570;	//*	the standard QSL port is 3306
char	gSQLsever_UserName[32]	=	"";
char	gSQLsever_Password[32]	=	"";
char	gSQLsever_Database[32]	=	"gaia";
char	gSQLclientVersion[64]	=	"";

int		gSQLerror_Count			=	0;
char	gSQLsever_StatusMsg[128]=	"";
bool	gSQLsever_MsgUpdated	=	false;

bool	gEnableSQLlogging		=	true;


static			pthread_t	gSQL_ThreadID			=	-1;
static			bool		gSQL_ThreadIsRunning	=	false;
static			bool		gSQL_KeepRunning		=	false;

TYPE_GAIA_REMOTE_DATA		gGaiaDataList[kMaxGaiaDataSets];
static			bool		gGaiaDataListNeedsInit		=	true;
static			bool		gSkyTravelUpdateOccured		=	false;
static			int			gGaiaSQLsequenceNum			=	0;
static			int			gGaiaStartThreadCnt			=	0;

TYPE_DATABASE_NAME			gDataBaseNames[kMaxDataBaseNames];
				int			gDataBaseNameCnt			=	0;


void	LogSqlTransaction(const char *dbName,const char *sqlCommand, const char *results, const char *errorString);


//*****************************************************************************
static void	ProcessSQLConfig(const char *keyword, const char *valueString, void *userDataPtr)
{

	//*	look for the supported keywords
	if (strcasecmp(keyword, "server") == 0)
	{
		strcpy(gSQLsever_IPaddr, valueString);
	}
	else if (strcasecmp(keyword, "port") == 0)
	{
		gSQLsever_Port	=	atoi(valueString);
	}
	else if (strcasecmp(keyword, "username") == 0)
	{
		strcpy(gSQLsever_UserName, valueString);
	}
	else if (strcasecmp(keyword, "password") == 0)
	{
		strcpy(gSQLsever_Password, valueString);
	}
	else if (strcasecmp(keyword, "database") == 0)
	{
		//*	this is an entry to the list of possible data bases
//			CONSOLE_DEBUG_W_STR("database name\t=",		valueString);
		if (gDataBaseNameCnt < kMaxDataBaseNames)
		{
			strcpy(gDataBaseNames[gDataBaseNameCnt].Name, valueString);
			gDataBaseNameCnt++;
		}
		else
		{
			CONSOLE_DEBUG("Number of database entries exceeded");
		}
	}
//	else
//	{
//		validEntry	=	false;
//	}
}

//*****************************************************************************
static bool	CheckSQLconfiguration(void)
{
bool			configOK;

	//*	check to see if we have all the information
	configOK	=	true;
	if (strlen(gSQLsever_IPaddr) == 0)
	{
		configOK	=	false;
	}
	if (strlen(gSQLsever_UserName) == 0)
	{
		configOK	=	false;
	}
	if (strlen(gSQLsever_Password) == 0)
	{
		configOK	=	false;
	}
	if (strlen(gSQLsever_Database) == 0)
	{
		configOK	=	false;
	}
	if (gSQLsever_Port == 0)
	{
		configOK	=	false;
	}
	return(configOK);
}


//*****************************************************************************
static bool	ReadSQLconfigFile(void)
{
int		iii;
bool	configOK;
int		linesRead;

//	CONSOLE_DEBUG(__FUNCTION__);
	configOK	=	false;

	//*	set the database name list to all zeros
	for (iii=0; iii<kMaxDataBaseNames; iii++)
	{
		memset((void *)&gDataBaseNames[iii], 0, sizeof(TYPE_DATABASE_NAME));
	}

	//*	"sqlserver.txt"
	linesRead	=	ReadGenericConfigFile(kSQLserverConfigFile, '=', &ProcessSQLConfig, NULL);
	if (linesRead > 2)
	{
		//*	check the configuration, makes sure that all the required parameters were specified
		configOK	=	CheckSQLconfiguration();
	}

	if (gDataBaseNameCnt <= 0)
	{
		strcpy(gDataBaseNames[gDataBaseNameCnt].Name, "gaia");
		gDataBaseNameCnt++;
		strcpy(gDataBaseNames[gDataBaseNameCnt].Name, "2mass");
		gDataBaseNameCnt++;
	}


	if (gDataBaseNameCnt > 0)
	{
		//*	set the current database to the first one in the list
		strcpy(gSQLsever_Database, gDataBaseNames[0].Name);
	}
	else
	{
		strcpy(gDataBaseNames[0].Name, "gaia");
		gDataBaseNameCnt	=	1;
	}
	return(configOK);
}

//*****************************************************************************
//*	returns true if valid config file
bool	GaiaSQLinit(void)
{
int		iii;
bool	configOK;

//	CONSOLE_DEBUG(__FUNCTION__);
	configOK	=	ReadSQLconfigFile();
	//*	set the Gaia data list to all zeros
	for (iii=0; iii<kMaxGaiaDataSets; iii++)
	{
		memset((void *)&gGaiaDataList[iii], 0, sizeof(TYPE_GAIA_REMOTE_DATA));
	}
	gGaiaDataListNeedsInit	=	false;

#if defined(MYSQL_BASE_VERSION)
	strcpy(gSQLclientVersion, MYSQL_BASE_VERSION);
#elif defined(MARIADB_BASE_VERSION)
	strcpy(gSQLclientVersion, MARIADB_BASE_VERSION);
#else
	strcpy(gSQLclientVersion, "Unknown:");
#endif
	strcat(gSQLclientVersion, "/");
	strcat(gSQLclientVersion, mysql_get_client_info());
//	CONSOLE_DEBUG_W_STR("SQL library:", gSQLclientVersion);

	return(configOK);
}

//*****************************************************************************
void	ClearAllSQLdata(void)
{
int		iii;

	if (gGaiaDataListNeedsInit == false)
	{
		for (iii=0; iii<kMaxGaiaDataSets; iii++)
		{
			if (gGaiaDataList[iii].gaiaData != NULL)
			{
				free(gGaiaDataList[iii].gaiaData);
			}
			memset(&gGaiaDataList[iii], 0, sizeof(TYPE_GAIA_REMOTE_DATA));
		}
	}
	gSQLerror_Count	=	0;
}


enum
{
	kSQL_info_UserName	=	0,
	kSQL_info_Version,

};
//*****************************************************************************
static void	BuildSQLlogString(int whichString, char *logCommentString)
{
	switch(whichString)
	{

		case kSQL_info_UserName:
		default:
			//*	figure out which user name to use
			if (strlen(gObseratorySettings.Observer) > 0)
			{
				strcpy(logCommentString, gObseratorySettings.Observer);
			}
			else if (strlen(gObseratorySettings.Owner) > 0)
			{
				strcpy(logCommentString, gObseratorySettings.Owner);
			}
			else if (strlen(gObseratorySettings.Name) > 0)
			{
				strcpy(logCommentString, gObseratorySettings.Name);
			}
			else
			{
				//*	last resort
				strcpy(logCommentString, "Unknown");
			}

			//*	add the sql library version string to the login string
			strcat(logCommentString, " (");
			strcat(logCommentString, gSQLclientVersion);
			strcat(logCommentString, ")");
			break;


		case kSQL_info_Version:
			strcpy(logCommentString, gFullVersionString);
			break;
	}
	//*	make sure the string does not exceed what the database is expecting
	#define	kMaxSQLloginfoLen	60
	if (strlen(logCommentString) >= kMaxSQLloginfoLen)
	{
		logCommentString[kMaxSQLloginfoLen]	=	0;
	}
}

//*****************************************************************************
//*	Nov 24,	2022	<KAS> Worked on cleaning up SQL Intiliazation as per Mark's request
//*****************************************************************************
static MYSQL	*OpenMysql(char *dataBaseName)
{
MYSQL	*mySQLConnection = NULL;

#ifdef _VERBOSE_SQL_DEBUG_
	CONSOLE_DEBUG(__FUNCTION__);
#endif // _VERBOSE_SQL_DEBUG_
	mySQLConnection	=	mysql_init(NULL);
	if (mySQLConnection != NULL)
	{
		//*	establish connection to the database
		CONSOLE_DEBUG_W_STR("dataBaseName\t", dataBaseName);
		mySQLConnection	=	mysql_real_connect(	mySQLConnection,
												gSQLsever_IPaddr,
												gSQLsever_UserName,
												gSQLsever_Password,
												dataBaseName,
												gSQLsever_Port,
												NULL,
												0);
		if (mySQLConnection == NULL)
		{
			CONSOLE_DEBUG("mysql_real_connect() failed");
			CONSOLE_DEBUG_W_STR("gSQLsever_IPaddr  \t=",	gSQLsever_IPaddr);
			CONSOLE_DEBUG_W_STR("gSQLsever_UserName\t=",	gSQLsever_UserName);
			CONSOLE_DEBUG_W_STR("gSQLsever_Password\t=",	gSQLsever_Password);
			CONSOLE_DEBUG_W_STR("dataBaseName      \t=",	dataBaseName);
			CONSOLE_DEBUG_W_NUM("gSQLsever_Port    \t=",	gSQLsever_Port);
			LogSqlTransaction(	dataBaseName,
								"mysql_real_connect() failed",
								__FUNCTION__,
								mysql_error(mySQLConnection));
		}
	}
	else
	{
		CONSOLE_DEBUG("mysql_init() failed");
		LogSqlTransaction(	dataBaseName,
							"mysql_init() failed",
							__FUNCTION__,
							mysql_error(mySQLConnection));
	}
	return(mySQLConnection);
}

//*****************************************************************************
static	int	LogSQLuser(void)
{
MYSQL 			*mySQLConnection = NULL;
char			mySQLCmd[256];
int				returnCode;
char			userString[128];

//	CONSOLE_DEBUG(__FUNCTION__);

	returnCode		=	-1;

	mySQLConnection	=	OpenMysql(gSQLsever_Database);
	if (mySQLConnection != NULL)
	{
	#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG_W_STR("Successfully connected to", gSQLsever_IPaddr);
		CONSOLE_DEBUG("mysql_select_db -- OK");
	#endif
		BuildSQLlogString(kSQL_info_UserName, userString);
		sprintf(mySQLCmd, "call SetLogComment('%s');", userString);
	#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG_W_STR("mySQLCmd\t=", mySQLCmd);
	#endif
		returnCode	=	mysql_query(mySQLConnection, mySQLCmd);
		if (returnCode == 0)
		{
		#ifdef _VERBOSE_SQL_DEBUG_
			CONSOLE_DEBUG("mysql_query -- OK");
		#endif
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("mysql_query ERROR returnCode\t=", returnCode);
			CONSOLE_DEBUG(mysql_error(mySQLConnection));
			LogSqlTransaction(gSQLsever_Database, "mysql_query() failed", __FUNCTION__, mysql_error(mySQLConnection));
		}

		BuildSQLlogString(kSQL_info_Version, userString);
		sprintf(mySQLCmd, "call SetLogComment('%s');", userString);
	#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG_W_STR("mySQLCmd\t=", mySQLCmd);
	#endif
		returnCode	=	mysql_query(mySQLConnection, mySQLCmd);
		if (returnCode == 0)
		{
		#ifdef _VERBOSE_SQL_DEBUG_
			CONSOLE_DEBUG("mysql_query -- OK");
		#endif
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("mysql_query ERROR returnCode\t=", returnCode);
			LogSqlTransaction(gSQLsever_Database, "mysql_query() failed", __FUNCTION__, mysql_error(mySQLConnection));
		}
	}
	else
	{
		CONSOLE_DEBUG("OpenMysql / mysql_init() failed");
		LogSqlTransaction(gSQLsever_Database, "mysql_init() failed", __FUNCTION__, mysql_error(mySQLConnection));
	}
	mysql_close(mySQLConnection);
	mysql_library_end();

	return(returnCode);
}


//*****************************************************************************
//*	returns # of fields
//*****************************************************************************
static int Querry_mySQL_cmd(	MYSQL		*myCon,
								MYSQL_RES	**mySQLresult,
								const char	*myDataBase,
								const char	*mySQLCmd )
{
int		num_fields;
int		returnCode;


#ifdef _VERBOSE_SQL_DEBUG_
	CONSOLE_DEBUG_W_STR(myDataBase,		mySQLCmd);
#endif
	num_fields	=	-1;

	returnCode	=	mysql_select_db(myCon, myDataBase);
	if (returnCode == 0)
	{
	#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG_W_NUM("mysql_select_db returnCode\t=", returnCode);
		CONSOLE_DEBUG_W_STR("Calling mysql_query() with\t=", mySQLCmd);
	#endif
		returnCode	=	mysql_query(myCon, mySQLCmd);
	#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG_W_NUM("mysql_query returnCode\t=", returnCode);
	#endif
		if (returnCode == 0)
		{
		#ifdef _VERBOSE_SQL_DEBUG_
			CONSOLE_DEBUG("calling mysql_store_result()");
		#endif
			*mySQLresult	=	mysql_store_result(myCon);

			if (*mySQLresult != NULL)
			{
				num_fields	=	mysql_num_fields(*mySQLresult);
			#ifdef _VERBOSE_SQL_DEBUG_
				CONSOLE_DEBUG_W_NUM("num_fields\t=", num_fields);
			#endif
			}
			else
			{
				LogSqlTransaction(gSQLsever_Database, "mysql_store_result() failed", __FUNCTION__, mysql_error(myCon));
				CONSOLE_DEBUG("Error on Querry_mySQL_cmd mysql_store_result");
			}
		}
		else
		{
			LogSqlTransaction(gSQLsever_Database, "mysql_query() failed", __FUNCTION__, mysql_error(myCon));
			CONSOLE_DEBUG_W_NUM("mysql_query failed with returnCode\t=", returnCode);
		}
	}
	else
	{
		LogSqlTransaction(gSQLsever_Database, "mysql_select_db() failed", __FUNCTION__, mysql_error(myCon));
		CONSOLE_DEBUG_W_NUM("mysql_select_db failed with returnCode\t=", returnCode);
	}

	return(num_fields);
}

//**************************************************************************
static char	CalcSpectralClassFrom_BP_RP(double bp_rp)
{
char	spectralClass;
double	bpLimits[]	=	{0.435,	0.870, 1.304, 1.729, 2.174, 2.609 };

	if (bp_rp < bpLimits[0])
	{
		spectralClass	=	'O';
	}
	else if (bp_rp < bpLimits[1])
	{
		spectralClass	=	'B';
	}
	else if (bp_rp < bpLimits[2])
	{
		spectralClass	=	'A';
	}
	else if (bp_rp < bpLimits[3])
	{
		spectralClass	=	'F';
	}
	else if (bp_rp < bpLimits[4])
	{
		spectralClass	=	'G';
	}
	else if (bp_rp < bpLimits[5])
	{
		spectralClass	=	'k';
	}
	else
	{
		spectralClass	=	'M';
	}
	return(spectralClass);
}


#ifdef _XREF_AAVSO_

//*****************************************************************************
const char	*gAAVSO_alertStars[]	=
{
	"4048168377818693632",
	"4111779763989583232",
	"4115021291723497088",
	"4120809606303456896",
	"4131587500273361280",
	"4150099732733146112",
	"4150446010182968192",
	"4168021909706732672",
	"4334886650491663104",
	"4636654969717900032",
	"5919388180059095296",
	"6019720819446985984",
	"6341112191380600448",
	"6345873798283774848",
	"",
	"",
	""
};

//*****************************************************************************
static bool	CheckAAVSOalerts(const char *starname)
{
int		iii;
bool	foundIt;

	foundIt	=	false;
	iii		=	0;
	while ((foundIt == false) && (iii < 14))
	{
		if (strstr(starname, gAAVSO_alertStars[iii]) != NULL)
		{
			foundIt	=	true;
		}
		iii++;
	}
	return(foundIt);
}


#endif // _XREF_AAVSO_


//*****************************************************************************
static	TYPE_CelestData	*GetGAIAdataFromSQL(	double	ra_Degrees,
												double	dec_Degrees,
												long	*totalRecords,
												bool	*successFlag)
{
TYPE_CelestData	*gaiaData;
int				recNum;
TYPE_CelestData	localStarData;
//*	mySWQL Variables
MYSQL 			*mySQLConnection = NULL;
char			mySQLCmd[256];
MYSQL_ROW		row;
MYSQL_RES		*mySQLresult;
int				ra_int;
int				dec_int;			//	Integers for Right Ascension and Declination
int				num_fields;
int				num_rows;
int				nextResultRetCode;
double			bp_rp;
unsigned int	startMilliSecs;
unsigned int	endMilliSecs;

#ifdef _VERBOSE_SQL_DEBUG_
	CONSOLE_DEBUG(__FUNCTION__);
#endif
	recNum			=	0;
	gaiaData		=	NULL;
	*successFlag	=	false;
	startMilliSecs	=	millis();

	mySQLConnection	=	OpenMysql(gSQLsever_Database);
	if (mySQLConnection != NULL)
	{
#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG_W_STR("Successfully connected to", gSQLsever_IPaddr);
#endif
		ra_int		=	floor(ra_Degrees);
		dec_int		=	floor(dec_Degrees);

		if (fabs(dec_Degrees) >= kPolarDeclinationLimit)
		{
			sprintf(mySQLCmd,"call GetDeclination(%d);", dec_int);
		}
		else
		{
			sprintf(mySQLCmd,"call GetRaDec(%d,%d);", ra_int, dec_int);
		}

#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG(mySQLCmd);
		CONSOLE_DEBUG("Calling Querry_mySQL_cmd()");
#endif
		mySQLresult	=	NULL;
		num_fields	=	Querry_mySQL_cmd(	mySQLConnection,
											&mySQLresult,
											gSQLsever_Database,
											mySQLCmd);
#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG_W_NUM("num_fields", num_fields);
#endif
		if (num_fields > 0)
		{
			*successFlag	=	true;
			num_rows		=	mysql_num_rows(mySQLresult);
#ifdef _VERBOSE_SQL_DEBUG_
			CONSOLE_DEBUG_W_NUM("num_rows", num_rows);
#endif
			if ((num_rows > 0) && (num_fields >= 3))
			{
				gaiaData		=	(TYPE_CelestData *)calloc((num_rows + 10), sizeof(TYPE_CelestData));
				if (gaiaData != NULL)
				{
					//	Loop through the Rows
					while ((row = mysql_fetch_row(mySQLresult)) && (recNum < num_rows))
					{
						//*	Get the name
						strcpy(localStarData.longName, row[0]);
//						CONSOLE_DEBUG_W_STR("longName\t=",	localStarData.longName);

						//*	Get the RA
						localStarData.ra		=	RADIANS(AsciiToDouble(row[1]));
						localStarData.org_ra	=	localStarData.ra;

						//*	Get the DEC
						localStarData.decl		=	RADIANS(AsciiToDouble(row[2]));
						localStarData.org_decl	=	localStarData.decl;

						//*	Get the Magnitude
						if (row[3] != NULL)
						{
							localStarData.realMagnitude		=	AsciiToDouble(row[3]);
//								if (localStarData.realMagnitude > 21.0)
//								{
//									CONSOLE_DEBUG_W_STR("longName     \t=",	localStarData.longName);
//									CONSOLE_DEBUG_W_DBL("realMagnitude\t=",	localStarData.realMagnitude);
//									CONSOLE_DEBUG_W_STR("row[3]       \t=",	row[3]);
//								}
						}
						else
						{
							localStarData.realMagnitude		=	15;
						}

					#ifdef _XREF_AAVSO_
						if (CheckAAVSOalerts(localStarData.longName))
						{
							localStarData.realMagnitude		=	0.1;
						}
					#endif // _XREF_AAVSO_

						//*	Get the spectral class
						if (row[4] != NULL)
						{
							bp_rp		=	AsciiToDouble(row[4]);
							localStarData.spectralClass		=	CalcSpectralClassFrom_BP_RP(bp_rp);
						}
						else
						{
							localStarData.spectralClass		=	0;
						}


						localStarData.dataSrc	=	kDataSrc_GAIA_SQL;
						gaiaData[recNum]		=	localStarData;
						recNum++;
					}
				}
			}
			else
			{
				CONSOLE_DEBUG("NO RESULTS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				CONSOLE_DEBUG_W_NUM("num_fields", num_fields);
				CONSOLE_DEBUG_W_NUM("num_rows", num_rows);
			}
		#ifdef _VERBOSE_SQL_DEBUG_
			CONSOLE_DEBUG("Calling mysql_free_result()");
		#endif
		//	SQL-Error	2022-11-13	Mark had this here
		//	mysql_free_result(mySQLresult);

			//*	Feb 22,	2022	<KAS> Fixed dangling result Bug
			nextResultRetCode	=	mysql_next_result(mySQLConnection);
			if (nextResultRetCode != 0)
			{
				CONSOLE_DEBUG_W_NUM("mysql_next_result", nextResultRetCode);
			}

			endMilliSecs	=	millis();

			if (gEnableSQLlogging)
			{
			char	textBuff[128];

				sprintf(textBuff, "Records rcvd=%d\tQuery time=%d", recNum, (endMilliSecs - startMilliSecs));
				LogSqlTransaction(gSQLsever_Database, mySQLCmd, textBuff, "");
			}
		}
		else
		{
			LogSqlTransaction(gSQLsever_Database, mySQLCmd, "num_fields <= 0", mysql_error(mySQLConnection));
		}
		//	SQL-Error	2022-11-13	Keith says this goes here
		mysql_free_result(mySQLresult);
#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG("Done");
#endif
	}
	else
	{
		LogSqlTransaction(gSQLsever_Database, "mysql_init() failed", __FUNCTION__, mysql_error(mySQLConnection));
	}
	mysql_close(mySQLConnection);
	mysql_library_end();

	*totalRecords	=	recNum;
	return(gaiaData);
}


//*****************************************************************************
//*	returns true if found
//*	if there is an error, the error is returned in rtnErrorMessage
//*****************************************************************************
bool	GetSQLdataFromIDnumber(const char *gaiaIDnumberStr, TYPE_CelestData *gaiaData, char *rtnErrorMessage)
{
int				recNum;
TYPE_CelestData	localStarData;
//*	mySWQL Variables
MYSQL 			*mySQLConnection = NULL;
char			mySQLCmd[256];
MYSQL_ROW		row;
MYSQL_RES		*mySQLresult;
int				num_fields;
int				num_rows;
double			bp_rp;
unsigned int	startMilliSecs;
unsigned int	endMilliSecs;
bool			foundIt;

#ifdef _VERBOSE_SQL_DEBUG_
	CONSOLE_DEBUG("-------------------------------------------------------------");
	CONSOLE_DEBUG(__FUNCTION__);
	CONSOLE_DEBUG_W_STR("Searching SQL database for", gaiaIDnumberStr);
#endif
	recNum		=	0;

	startMilliSecs		=	millis();

	foundIt				=	false;
	rtnErrorMessage[0]	=	0;

	mySQLConnection	=	OpenMysql(gSQLsever_Database);
	if (mySQLConnection != NULL)
	{
#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG(__FUNCTION__);

		CONSOLE_DEBUG("Trying to establish connection to the database");
		CONSOLE_DEBUG_W_STR("gSQLsever_IPaddr\t=",		gSQLsever_IPaddr);
		CONSOLE_DEBUG_W_STR("gSQLsever_UserName\t=",	gSQLsever_UserName);
		CONSOLE_DEBUG_W_STR("gSQLsever_Password\t=",	gSQLsever_Password);
		CONSOLE_DEBUG_W_STR("gSQLsever_Database\t=",	gSQLsever_Database);
#endif

		sprintf(mySQLCmd,"call GetStarFromID(%s);", gaiaIDnumberStr);
#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG(mySQLCmd);
		CONSOLE_DEBUG("Calling Querry_mySQL_cmd()");
#endif
		mySQLresult	=	NULL;
		num_fields	=	Querry_mySQL_cmd(	mySQLConnection,
											&mySQLresult,
											gSQLsever_Database,
											mySQLCmd);
//			CONSOLE_DEBUG_W_NUM("num_fields", num_fields);
		if (num_fields > 0)
		{
			num_rows	=	mysql_num_rows(mySQLresult);
//				CONSOLE_DEBUG_W_NUM("num_rows", num_rows);

			if ((num_rows > 0) && (num_fields >= 3))
			{
//					CONSOLE_DEBUG(__FUNCTION__);
				row			=	mysql_fetch_row(mySQLresult);

				foundIt		=	true;
				recNum		=	1;
				//*	Get the name
//					CONSOLE_DEBUG_W_STR("row[0]\t=",	row[0]);
				strcpy(localStarData.longName, row[0]);
//					CONSOLE_DEBUG_W_STR("longName\t=",	localStarData.longName);

				//*	Get the RA
				localStarData.ra		=	RADIANS(AsciiToDouble(row[1]));
				localStarData.org_ra	=	localStarData.ra;

				//*	Get the DEC
				localStarData.decl		=	RADIANS(AsciiToDouble(row[2]));
				localStarData.org_decl	=	localStarData.decl;

				//*	Get the Magnitude
				if (row[3] != NULL)
				{
					localStarData.realMagnitude		=	AsciiToDouble(row[3]);
					if (localStarData.realMagnitude > 21.0)
					{
						CONSOLE_DEBUG_W_STR("longName     \t=",	localStarData.longName);
						CONSOLE_DEBUG_W_DBL("realMagnitude\t=",	localStarData.realMagnitude);
						CONSOLE_DEBUG_W_STR("row[3]       \t=",	row[3]);
					}
				}
				else
				{
					localStarData.realMagnitude		=	15;
				}

				//*	Get the spectral class
				if (row[4] != NULL)
				{
					bp_rp		=	AsciiToDouble(row[4]);
					localStarData.spectralClass		=	CalcSpectralClassFrom_BP_RP(bp_rp);
				}
				else
				{
					localStarData.spectralClass		=	0;
				}

				localStarData.dataSrc	=	kDataSrc_GAIA_SQL;
			}
			else
			{
				CONSOLE_DEBUG("NO RESULTS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				CONSOLE_DEBUG_W_NUM("num_fields", num_fields);
				CONSOLE_DEBUG_W_NUM("num_rows", num_rows);
				sprintf(rtnErrorMessage,	"ID=%s, was not found in %s database", gaiaIDnumberStr, gSQLsever_Database);
			}
//				CONSOLE_DEBUG("Calling mysql_free_result()");
		//	SQL-Error	2022-11-13	Mark had this here
		//	mysql_free_result(mySQLresult);

			endMilliSecs	=	millis();

			if (gEnableSQLlogging)
			{
			char	textBuff[128];

				sprintf(textBuff, "Records rcvd=%d\tQuery time=%d", recNum, (endMilliSecs - startMilliSecs));
				LogSqlTransaction(gSQLsever_Database, mySQLCmd, textBuff, "");
			}
		}
		else
		{
			LogSqlTransaction(gSQLsever_Database, "num_fields <= 0", __FUNCTION__, mysql_error(mySQLConnection));
			strcpy(rtnErrorMessage,	"num_fields <= 0: ");
			strcat(rtnErrorMessage,	mysql_error(mySQLConnection));
		}
	//	SQL-Error	2022-11-13	Keith says this goes here
		mysql_free_result(mySQLresult);
//			CONSOLE_DEBUG("Done");
	}
	else
	{
		LogSqlTransaction(gSQLsever_Database, "OpenMysql / mysql_init() failed", __FUNCTION__, mysql_error(mySQLConnection));
		strcpy(rtnErrorMessage,	"mysql_init() failed: ");
		strcat(rtnErrorMessage,	mysql_error(mySQLConnection));
	}
	mysql_close(mySQLConnection);
	mysql_library_end();

	*gaiaData	=	localStarData;
//	CONSOLE_DEBUG(__FUNCTION__);

	return(foundIt);
}


//*****************************************************************************
//Database name	Stars
//Function	call GetStarCatalogs;
//
//It returns a list of database names:
//
//gdr1
//gdr2
//Gaia
//2mass
//
//*****************************************************************************
int		UpdateDataBaseListFromServer(char *errorString)
{
int				dbaseCount;
MYSQL 			*mySQLConnection = NULL;
char			mySQLCmd[256];
MYSQL_ROW		row;
MYSQL_RES		*mySQLresult;
int				num_fields;
int				num_rows;
int				recNum;
int				nextResultRetCode;
char			starsDBname[]	=	"Stars";
char			textBuff[128];
unsigned int	startMilliSecs;
unsigned int	endMilliSecs;

	CONSOLE_DEBUG(__FUNCTION__);

	startMilliSecs	=	millis();

	dbaseCount		=	0;
	errorString[0]	=	0;

	mySQLConnection	=	OpenMysql(starsDBname);
	if (mySQLConnection != NULL)
	{
#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG(__FUNCTION__);

		CONSOLE_DEBUG("Trying to establish connection to the database");
		CONSOLE_DEBUG_W_STR("gSQLsever_IPaddr  \t=",	gSQLsever_IPaddr);
		CONSOLE_DEBUG_W_STR("gSQLsever_UserName\t=",	gSQLsever_UserName);
		CONSOLE_DEBUG_W_STR("gSQLsever_Password\t=",	gSQLsever_Password);
		CONSOLE_DEBUG_W_STR("gSQLsever_Database\t=",	starsDBname);
		CONSOLE_DEBUG_W_NUM("gSQLsever_Port    \t=",	gSQLsever_Port);
#endif
		strcpy(mySQLCmd, "call GetStarCatalogs();");
#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG(mySQLCmd);
		CONSOLE_DEBUG("Calling Querry_mySQL_cmd()");
#endif
		mySQLresult	=	NULL;
		num_fields	=	Querry_mySQL_cmd(	mySQLConnection,
											&mySQLresult,
											starsDBname,
											mySQLCmd);
#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG_W_NUM("num_fields", num_fields);
#endif
		if (num_fields > 0)
		{
			num_rows	=	mysql_num_rows(mySQLresult);
#ifdef _VERBOSE_SQL_DEBUG_
			CONSOLE_DEBUG_W_NUM("num_rows", num_rows);
#endif
			if ((num_rows > 0) && (num_fields > 0))
			{
				dbaseCount		=	0;
				recNum			=	0;
				//	Loop through the Rows
				while ((row = mysql_fetch_row(mySQLresult)) && (recNum < num_rows))
				{
					//*	Get the name
					CONSOLE_DEBUG(row[0]);
					if (strlen(row[0]) > 0)
					{
						strcpy(gDataBaseNames[dbaseCount].Name, row[0]);
						dbaseCount++;
					}
					recNum++;
				}
				gDataBaseNameCnt	=	dbaseCount;

				endMilliSecs	=	millis();
				sprintf(textBuff, "Records rcvd=%d\tQuery time=%d", recNum, (endMilliSecs - startMilliSecs));

				LogSqlTransaction(starsDBname, mySQLCmd, textBuff, "");
			}
			else
			{
				CONSOLE_DEBUG("NO RESULTS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
				CONSOLE_DEBUG_W_NUM("num_fields", num_fields);
				CONSOLE_DEBUG_W_NUM("num_rows", num_rows);
			}
		#ifdef _VERBOSE_SQL_DEBUG_
			CONSOLE_DEBUG("Calling mysql_free_result()");
		#endif
		//	SQL-Error	2022-11-13	Mark had this here
		//	mysql_free_result(mySQLresult);

			//*	Feb 22,	2022	<KAS> Fixed dangling result Bug
			nextResultRetCode	=	mysql_next_result(mySQLConnection);
			if (nextResultRetCode != 0)
			{
				CONSOLE_DEBUG_W_NUM("mysql_next_result", nextResultRetCode);
			}
		}
		else
		{
			CONSOLE_DEBUG(__FUNCTION__);
			LogSqlTransaction(starsDBname, mySQLCmd, "num_fields <= 0", mysql_error(mySQLConnection));
		}
	//	SQL-Error	2022-11-13	Keith says this goes here
		mysql_free_result(mySQLresult);
#ifdef _VERBOSE_SQL_DEBUG_
		CONSOLE_DEBUG("Done");
#endif
	}
	else
	{
		CONSOLE_DEBUG(__FUNCTION__);
		LogSqlTransaction(starsDBname, "OpenMysql() failed", __FUNCTION__, mysql_error(mySQLConnection));
		sprintf(errorString, "OpenMysql() failed: %s", mysql_error(mySQLConnection));
	}
	mysql_close(mySQLConnection);
	mysql_library_end();

	CONSOLE_DEBUG_W_NUM("Number of databases found\t=", dbaseCount);
	return(dbaseCount);
}



//*****************************************************************************
double	CalcRA_DEC_Distance_Deg(const double	ra1_Deg,
								const double	dec1_Deg,
								const double	ra2_Deg,
								const double	dec2_Deg)
{
double	deltaRA;
double	deltaDEC;
double	distanceSqrd;
double	middleDecl_deg;
double	middleDecl_rad;
double	distance_Deg;
double	my_ra1_Deg;
double	my_dec1_Deg;
double	my_ra2_Deg;
double	my_dec2_Deg;
double	tempValue_RA;
double	tempValue_DEC;

//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_DBL("ra1_Deg  \t=", ra1_Deg);
//	CONSOLE_DEBUG_W_DBL("ra2_Deg  \t=", ra2_Deg);

	//*	this gets complicated if we are across the 360 to 0 degree boundry
	//*	to solve that issue, we are going to
	//*		put them in order,
	//*		add 360 if needed

	my_ra1_Deg	=	ra1_Deg;
	my_dec1_Deg	=	dec1_Deg;
	my_ra2_Deg	=	ra2_Deg;
	my_dec2_Deg	=	dec2_Deg;
	if (my_ra1_Deg < 0.0)
	{
		my_ra1_Deg	+=	360.0;
	}
	if (my_ra2_Deg < 0.0)
	{
		my_ra2_Deg	+=	360.0;
	}
//	CONSOLE_DEBUG_W_DBL("my_ra1_Deg  \t=", my_ra1_Deg);
//	CONSOLE_DEBUG_W_DBL("my_ra2_Deg  \t=", my_ra2_Deg);

	if (my_ra2_Deg <= my_ra1_Deg)
	{
		//*	ok, swap them
		tempValue_RA	=	my_ra1_Deg;
		tempValue_DEC	=	my_dec1_Deg;

		my_ra1_Deg		=	my_ra2_Deg;
		my_dec1_Deg		=	my_dec2_Deg;

		my_ra2_Deg		=	tempValue_RA;
		my_dec2_Deg		=	tempValue_DEC;
	}


	if ((my_ra2_Deg - my_ra1_Deg) > 180)
	{
		my_ra1_Deg	+=	360;
//		CONSOLE_DEBUG_W_DBL("delta > 180: my_ra1_Deg\t=", my_ra1_Deg);
	}


	//*	because of the converging nature of RA at the poles, we have to account for the
	//*	declination angle in the RA calculation
	middleDecl_deg	=	(my_dec1_Deg + my_dec2_Deg) / 2.0;
	middleDecl_rad	=	RADIANS(middleDecl_deg);


	//*	if we are above the point where we ignore RA, then set it to zero
	if (fabs(middleDecl_deg) >= kPolarDeclinationLimit)
	{
		deltaRA			=	0;
	}
	else
	{
		deltaRA			=	cos(middleDecl_rad) * (my_ra2_Deg - my_ra1_Deg);
	}

	deltaDEC		=	my_dec2_Deg - my_dec1_Deg;
	distanceSqrd	=	(deltaRA * deltaRA) + (deltaDEC * deltaDEC);

	distance_Deg	=	sqrt(distanceSqrd);

//	if ((my_ra2_Deg - my_ra1_Deg) > 180)
//	{
//		CONSOLE_DEBUG_W_DBL("distance_Deg\t=", distance_Deg);
//		CONSOLE_DEBUG_W_DBL("distance_Deg\t=", (my_ra2_Deg - my_ra1_Deg) );
//		CONSOLE_DEBUG_W_DBL("my_ra1_Deg  \t=", my_ra1_Deg);
//		CONSOLE_DEBUG_W_DBL("my_dec1_Deg \t=", my_dec1_Deg);
//		CONSOLE_DEBUG_W_DBL("my_ra2_Deg  \t=", my_ra2_Deg);
//		CONSOLE_DEBUG_W_DBL("my_dec2_Deg \t=", my_dec2_Deg);
//
//		CONSOLE_DEBUG_W_DBL("middleDecl_deg    \t=", middleDecl_deg);
//		CONSOLE_DEBUG_W_DBL("cos(middleDecl)   \t=", cos(middleDecl_rad));
//	}
//	CONSOLE_DEBUG_W_DBL("distance_Deg    \t=", distance_Deg);
	return(distance_Deg);
}

//*****************************************************************************
static int	PurgeFartherestEntry(double ra_Degrees, double dec_Degrees)
{
int		iii;
int		freeSlot;
double	distance_Deg;
double	maxDist_Deg;

//	CONSOLE_DEBUG(__FUNCTION__);
	freeSlot	=	-1;
	maxDist_Deg	=	0;

	for (iii=0; iii<kMaxGaiaDataSets; iii++)
	{
		if (gGaiaDataList[iii].validData)
		{
			//*	compute the distance from the CURRENT RA/DEC to the ones in the table
			distance_Deg	=	CalcRA_DEC_Distance_Deg(ra_Degrees,
													dec_Degrees,
													gGaiaDataList[iii].centerRA_deg,
													gGaiaDataList[iii].centerDEC_deg);
			gGaiaDataList[iii].distanceCtrScrn	=	distance_Deg;
			if (distance_Deg > maxDist_Deg)
			{
				maxDist_Deg	=	distance_Deg;
				freeSlot	=	iii;
			}
		}
	}

	if ((freeSlot >= 0) && (freeSlot < kMaxGaiaDataSets))
	{
		if (gGaiaDataList[freeSlot].gaiaData != NULL)
		{
			free(gGaiaDataList[freeSlot].gaiaData);
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("No data to FREE() !!!!!!!!!!!!!!!!!!!! Slot#", freeSlot);
		}
		//*	set everything to NULL
		memset(&gGaiaDataList[freeSlot], 0, sizeof(TYPE_GAIA_REMOTE_DATA));
	}
	return(freeSlot);
}

#if 0
//*****************************************************************************
static void	DumpGaiaRemoteTable(const char *functionName)
{
int		iii;

	CONSOLE_DEBUG_W_STR(functionName, "--------------------------------------------------");
	for (iii=0; iii<kMaxGaiaDataSets; iii++)
	{
		printf("%02d\t",	iii);
		printf("%c\t",		(gGaiaDataList[iii].validData ? 'V' : '-'));
		printf("%4.2f\t",	gGaiaDataList[iii].centerRA_deg);
		printf("%3d H\t",	gGaiaDataList[iii].block_RA_deg/15);
		printf("%4.2f\t",	gGaiaDataList[iii].centerDEC_deg);
		printf("%3d\t",		gGaiaDataList[iii].block_DEC_deg);
		printf("%5d\t",		gGaiaDataList[iii].gaiaDataCnt);
		printf("dT=%5d\t",	gGaiaDataList[iii].elapsedMilliSecs);

		printf("\r\n");
	}
}
#endif // 0


//*****************************************************************************
//*	returns 1 if new request was started, 0 if not
//*		using int so the calling routine can count how many
//*****************************************************************************
int	UpdateSkyTravelView(double ra_Degrees, double dec_Degrees, double viewAngle_Degrees)
{
int		ra_deg_int;
int		dec_deg_int;
int		slotIdx;
bool	dataNeesToBeLoaded;
int		iii;
int		requestStarted;

	requestStarted		=	0;

	if ((viewAngle_Degrees < 5.0) && (dec_Degrees < 90.0) && (dec_Degrees >= -90.0))
	{
		if (gST_DispOptions.RemoteGAIAenabled && (gSQL_ThreadIsRunning == false))
		{
			CONSOLE_DEBUG("**************************************************************");
			CONSOLE_DEBUG("Calling StartGaiaSQLthread()");
			StartGaiaSQLthread();
		}

		if (ra_Degrees < 0.0)
		{
			ra_Degrees	+=	360.0;
		}
		dataNeesToBeLoaded	=	true;
		ra_deg_int	=	floor(ra_Degrees);
		dec_deg_int	=	floor(dec_Degrees);

		//*	first check to see if this block is already loaded
		//*	SPECIAL CASE for Dec value near the poles
		if (fabs(dec_Degrees) >= kPolarDeclinationLimit)
		{
			for (iii=0; iii<kMaxGaiaDataSets; iii++)
			{
				if (gGaiaDataList[iii].validData)
				{
					if (dec_deg_int == gGaiaDataList[iii].block_DEC_deg)
					{
					//	CONSOLE_DEBUG("GAIA section all ready loaded");
						dataNeesToBeLoaded	=	false;
						break;
					}
				}
			}
		}
		else
		{
			for (iii=0; iii<kMaxGaiaDataSets; iii++)
			{
				if (gGaiaDataList[iii].validData)
				{
					if ((ra_deg_int == gGaiaDataList[iii].block_RA_deg) &&
						(dec_deg_int == gGaiaDataList[iii].block_DEC_deg))
					{
					//	CONSOLE_DEBUG("GAIA section all ready loaded");
						dataNeesToBeLoaded	=	false;
						break;
					}
				}
			}
		}

		if (dataNeesToBeLoaded)
		{
			CONSOLE_DEBUG("dataNeesToBeLoaded");
			CONSOLE_DEBUG_W_DBL("ra_Degrees \t=", ra_Degrees);
			CONSOLE_DEBUG_W_NUM("ra_deg_int \t=", ra_deg_int);

			CONSOLE_DEBUG_W_DBL("dec_Degrees\t=", dec_Degrees);
			CONSOLE_DEBUG_W_NUM("dec_deg_int\t=", dec_deg_int);

			//*	go through the list and find an empty slot
			slotIdx	=	-1;
			for (iii=0; iii<kMaxGaiaDataSets; iii++)
			{
				if (gGaiaDataList[iii].validData == false)
				{
					//*	we have an empty slot
					slotIdx	=	iii;
					break;
				}
			}

			if (slotIdx < 0)
			{
				//*	no empty slot, delete one of them.
				slotIdx	=	PurgeFartherestEntry(ra_Degrees, dec_Degrees);
			}

			if (slotIdx >= 0)
			{
				CONSOLE_DEBUG_W_NUM("slotIdx\t=", slotIdx);
				gGaiaDataList[slotIdx].centerRA_deg		=	ra_Degrees;
				gGaiaDataList[slotIdx].centerDEC_deg	=	dec_Degrees;
				gGaiaDataList[slotIdx].block_RA_deg		=	ra_deg_int;
				gGaiaDataList[slotIdx].block_DEC_deg	=	dec_deg_int;

				gGaiaDataList[slotIdx].validData		=	true;				//*	do this LAST

				requestStarted	=	1;
			}
			else
			{
				CONSOLE_DEBUG("Not able to find an available slot");
			}
			gSkyTravelUpdateOccured	=	true;

//			DumpGaiaRemoteTable(__FUNCTION__);
		}
	}
	return(requestStarted);
}


#ifndef _INCLUDE_GAIA_MAIN_

//*****************************************************************************
static void	*GaiaSQL_Thead(void *arg)
{
int				iii;
TYPE_CelestData	*gaiaData;
long			gaiaDataCount;
unsigned int	startMilliSecs;
unsigned int	endMilliSecs;
bool			requestOccured;
bool			successFlag;

	gSQL_ThreadIsRunning	=	true;
	while (gSQL_KeepRunning)
	{
		//*
		if (gST_DispOptions.RemoteGAIAenabled && gSkyTravelUpdateOccured)
		{
			CONSOLE_DEBUG(__FUNCTION__);
			requestOccured	=	true;
			while (requestOccured)
			{
				requestOccured	=	false;
				//*	lets see if we have any work to do
				for (iii=0; iii<kMaxGaiaDataSets; iii++)
				{
					if (gGaiaDataList[iii].validData && (gGaiaDataList[iii].gaiaData == NULL))
					{
						CONSOLE_DEBUG("GAIA_Thread, going to get data");
						CONSOLE_DEBUG_W_DBL("centerRA_deg\t=", gGaiaDataList[iii].centerRA_deg);
						CONSOLE_DEBUG_W_DBL("centerDEC_deg\t=", gGaiaDataList[iii].centerDEC_deg);

						gettimeofday(&gGaiaDataList[iii].timeStamp, NULL);	//*	save the time we started the request

						startMilliSecs	=	millis();
						gaiaData		=	GetGAIAdataFromSQL(	gGaiaDataList[iii].centerRA_deg,
																gGaiaDataList[iii].centerDEC_deg,
																&gaiaDataCount,
																&successFlag);

						endMilliSecs	=	millis();
						requestOccured	=	true;
						CONSOLE_DEBUG_W_LONG("gaiaDataCount\t=", gaiaDataCount);

						//*	make sure we got some valid data
//						if ((gaiaData != NULL) && (gaiaDataCount > 0))
						if (successFlag)
						{
							//*	sort the data to speed up drawing
							if (gaiaDataCount > 1)
							{
								CONSOLE_DEBUG("Sorting");
								qsort(gaiaData, gaiaDataCount, sizeof(TYPE_CelestData), CelestObjDeclinationQsortProc);
							}
							CONSOLE_DEBUG_W_NUM("assigning gaia data, iii=", iii);

							gGaiaDataList[iii].gaiaData			=	gaiaData;
							gGaiaDataList[iii].gaiaDataCnt		=	gaiaDataCount;
							gGaiaDataList[iii].elapsedMilliSecs	=	endMilliSecs - startMilliSecs;
							gGaiaDataList[iii].sequenceNum		=	gGaiaSQLsequenceNum;
							gGaiaSQLsequenceNum++;

							sprintf(gSQLsever_StatusMsg, "SQL success, record count=%ld", gaiaDataCount);
							gSQLsever_MsgUpdated	=	true;
						}
						else
						{
							CONSOLE_DEBUG("Failed to get GAIA data!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
							strcpy(gSQLsever_StatusMsg, "SQL failed!!!");
							gSQLsever_MsgUpdated	=	true;

							gGaiaDataList[iii].validData	=	false;
							memset(&gGaiaDataList[iii], 0, sizeof(TYPE_GAIA_REMOTE_DATA));

							gSQLerror_Count++;
							if (gSQLerror_Count > 10)
							{
								strcpy(gSQLsever_StatusMsg, "Too many SQL errors, sleeping for 1 minute");
								gSQLsever_MsgUpdated	=	true;
								CONSOLE_DEBUG(gSQLsever_StatusMsg);
								sleep(1 * 60);
							}
						}
					//	CONSOLE_DEBUG("Quick nap between requests");

						//*	sleep for 100 millseconds
						usleep(100 * 1000);
					//	CONSOLE_DEBUG("Awake");

					//	DumpGaiaRemoteTable(__FUNCTION__);
					}
				}
			}
			gSkyTravelUpdateOccured	=	false;


		}
		else
		{
		//	CONSOLE_DEBUG("Nothing to do... Sleeping");
			sleep(5);
		//	CONSOLE_DEBUG("Awake");
		}
	}
	gSQL_ThreadIsRunning	=	false;
	return(NULL);
}



//*****************************************************************************
//*	returns 0=OK, -1, failed to create, +1 busy
//*****************************************************************************
//*	inputs:
//*		arcSecondsPerPixel or fieldOfView_deg, use only one.
//*****************************************************************************
int	StartGaiaSQLthread(void)
{
int		threadStatus;
int		threadErr;
int		loopCnt;
bool	configOK;


	CONSOLE_DEBUG("**************************************************************");
	CONSOLE_DEBUG_W_NUM(__FUNCTION__, gGaiaStartThreadCnt);
	gGaiaStartThreadCnt++;

	//*	do we need to initialize the data array
	if (gGaiaDataListNeedsInit)
	{
		GaiaSQLinit();
	}

	configOK	=	CheckSQLconfiguration();
	if (configOK)
	{
		if (gSQL_ThreadIsRunning == false)
		{
			LogSQLuser();

			CONSOLE_DEBUG("Staring Gaia SQL Thread");
			gSQL_KeepRunning	=	true;
			threadErr	=	pthread_create(	&gSQL_ThreadID,
											NULL,
											&GaiaSQL_Thead,
											NULL);
			if (threadErr == 0)
			{
				CONSOLE_DEBUG("GAIA SQL thread created successfully");
				threadStatus	=	0;

				//*	sleep a little so the thread has a chance to get running
				loopCnt	=	0;
				while ((gSQL_ThreadIsRunning == false) && (loopCnt < 200))
				{
					usleep(5000);
					loopCnt++;
				}
				CONSOLE_DEBUG_W_NUM("gSQL_ThreadIsRunning\t=", gSQL_ThreadIsRunning);
				CONSOLE_DEBUG_W_NUM("loopCnt\t\t\t=", loopCnt);

			}
			else
			{
				CONSOLE_DEBUG_W_NUM("Error on thread creation, Error number:", threadErr);
				threadStatus	=	-1;
			}
		}
		else
		{
			CONSOLE_DEBUG("-------------------------------------------------------------");
			CONSOLE_DEBUG("Thread already running");
			threadStatus	=	1;
		}
	}
	else
	{
		CONSOLE_DEBUG("SQL not configured properly");
		threadStatus	=	-1;
	}
	return(threadStatus);
}


//*****************************************************************************
void	StopGaiaSQLthread(void)
{
	gSQL_KeepRunning	=	false;
}
#endif // _INCLUDE_GAIA_MAIN_

//*****************************************************************************
void	LogSqlTransaction(	const char	*dbName,
							const char	*sqlCommand,
							const char	*results,
							const char	*errorString)
{
FILE			*filePointer;
struct timeval	timeStamp;
char			formatString[256];

	CONSOLE_DEBUG_W_STR("dbName      \t=", dbName);
	CONSOLE_DEBUG_W_STR("sqlCommand  \t=", sqlCommand);
	CONSOLE_DEBUG_W_STR("results/FUNC\t=", results);
	CONSOLE_DEBUG_W_STR("errorString \t=", errorString);

	filePointer	=	fopen("SQL_log.txt", "a");
	if (filePointer != NULL)
	{
		gettimeofday(&timeStamp, NULL);
		FormatDateTimeString_Local(&timeStamp, formatString);

		fprintf(filePointer, "%s\t%s\t%s\t%s\t%s\n", formatString, dbName, sqlCommand, results, errorString);
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create sql log file");
	}
//	CONSOLE_DEBUG("exit");
}

#ifdef _INCLUDE_GAIA_MAIN_
//*****************************************************************************
int main(int argc, char *argv[])
{
TYPE_CelestData	*gaiaData;
long			gaiaDataCount;
double			ra_Degrees;
double			dec_Degrees;
unsigned int	startMilliSecs;
unsigned int	endMilliSecs;
unsigned int	elapsedMilliSecs;
unsigned int	totalMilliSecs;
int				requestCount;
double			avgTime;

	CONSOLE_DEBUG(__FUNCTION__);

	ra_Degrees		=	0.0;
	dec_Degrees		=	10.0;
	requestCount	=	0;
	totalMilliSecs	=	0;
	while (ra_Degrees < 360.0)
	{
		dec_Degrees	=	10.0;
		while (dec_Degrees < 90.0)
		{
			startMilliSecs	=	millis();
			CONSOLE_DEBUG("----------------------------------");
			CONSOLE_DEBUG_W_DBL("ra_Degrees  \t=", ra_Degrees);
			CONSOLE_DEBUG_W_DBL("dec_Degrees \t=", dec_Degrees);
			gaiaData		=	GetGAIAdataFromSQL(	ra_Degrees,
													dec_Degrees,
													&gaiaDataCount);
			endMilliSecs		=	millis();
			elapsedMilliSecs	=	endMilliSecs - startMilliSecs;
			totalMilliSecs		+=	elapsedMilliSecs;
			requestCount++;

			avgTime				=	(totalMilliSecs * 1.0) / (requestCount * 1000.0);

			CONSOLE_DEBUG_W_LONG("gaiaDataCount\t=", gaiaDataCount);
			CONSOLE_DEBUG_W_NUM("elapsed time  \t=", elapsedMilliSecs);
			CONSOLE_DEBUG_W_DBL("avgTime       \t=", avgTime);

			dec_Degrees	+=	10.0;
		}
		ra_Degrees	+=	1.0;
	}

	CONSOLE_DEBUG_W_LONG("gaiaDataCount", gaiaDataCount);
}
#endif // _INCLUDE_GAIA_MAIN_


#endif // _ENABLE_REMOTE_GAIA_
