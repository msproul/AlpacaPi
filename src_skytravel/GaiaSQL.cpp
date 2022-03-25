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
//*****************************************************************************
//*	sudo apt-get install libmysqlclient-dev		<<<< Use this one
//*	sudo apt-get install libmariadb-dev			<<<< Use this for Raspberry-Pi
//*****************************************************************************
//*	https://www.cosmos.esa.int/web/gaia/earlydr3
//*		GAIA ER3 is JD2016.0
//*****************************************************************************

#ifdef _ENABLE_REMOTE_GAIA_

#include	<stdlib.h>
#include	<string.h>
#include	<math.h>
#include	<pthread.h>
#include	<unistd.h>

//	<KAS>	For SQL Functions
#if defined(__ARM_ARCH) || defined(__arm__)
	#include	<mariadb/mysql.h>
#else
	#include	<mysql/mysql.h>
#endif

#define _ENABLE_CONSOLE_DEBUG_
#include	"ConsoleDebug.h"

#include	"controller_skytravel.h"
#include	"helper_functions.h"
#include	"SkyStruc.h"
#include	"RemoteImage.h"
#include	"observatory_settings.h"

#include	"GaiaSQL.h"
#include	"RemoteGaia.h"


#define	_XREF_AAVSO_

char	gSQLsever_IPaddr[32]	=	"";
int		gSQLsever_Port			=	3306;
char	gSQLsever_UserName[32]	=	"";
char	gSQLsever_Password[32]	=	"";
char	gSQLsever_Database[32]	=	"gaia";

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


void	LogSqlTransaction(const char *sqlCommand, const char *results);


//*****************************************************************************
//*	returns false if the keyword was not valid
//*****************************************************************************
static	bool ProcessSQLserverLine(char *lineBuff)
{
char	keyword[32];
int		iii;
int		ccc;
int		slen;
char	*valueStrPtr;
bool	validEntry;

	iii			=	0;
	ccc			=	0;
	validEntry	=	true;
	slen		=	strlen(lineBuff);
	while ((iii<slen) && (ccc < 31) && (lineBuff[iii] > 0x20))
	{
		keyword[ccc]	=	lineBuff[iii];
		ccc++;
		iii++;
	}
	keyword[ccc]	=	0;

	valueStrPtr	=	strchr(lineBuff, '=');
	if (valueStrPtr != NULL)
	{
		valueStrPtr++;		//*	skip over the "="
		while (*valueStrPtr <= 0x20)
		{
			valueStrPtr++;
		}

		//*	look for the supported keywords
		if (strcasecmp(keyword, "server") == 0)
		{
			strcpy(gSQLsever_IPaddr, valueStrPtr);
		}
		else if (strcasecmp(keyword, "port") == 0)
		{
			gSQLsever_Port	=	atoi(valueStrPtr);
		}
		else if (strcasecmp(keyword, "username") == 0)
		{
			strcpy(gSQLsever_UserName, valueStrPtr);
		}
		else if (strcasecmp(keyword, "password") == 0)
		{
			strcpy(gSQLsever_Password, valueStrPtr);
		}
		else if (strcasecmp(keyword, "database") == 0)
		{
			//*	this is an entry to the list of possible data bases
//			CONSOLE_DEBUG_W_STR("database name\t=",		valueStrPtr);
			if (gDataBaseNameCnt < kMaxDataBaseNames)
			{
				strcpy(gDataBaseNames[gDataBaseNameCnt].Name, valueStrPtr);
				gDataBaseNameCnt++;
			}
//			else
//			{
//				CONSOLE_ABORT(__FUNCTION__);
//			}
		}
		else
		{
			validEntry	=	false;
		}
	}
	return(	validEntry);
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
FILE	*filePointer;
char	lineBuff[256];
int		iii;
int		slen;
bool	configOK;
bool	validEntry;

//	CONSOLE_DEBUG(__FUNCTION__);
	//*	set the database name list to all zeros
	for (iii=0; iii<kMaxGaiaDataSets; iii++)
	{
		memset((void *)&gDataBaseNames[iii], 0, sizeof(TYPE_DATABASE_NAME));
	}

	configOK	=	false;
	//*	check for the sql server settings file
	filePointer	=	fopen(kSQLserverConfigFile, "r");
	if (filePointer != NULL)
	{
		while (fgets(lineBuff, 200, filePointer))
		{
			//*	get rid of the trailing CR/LF
			slen	=	strlen(lineBuff);
			for (iii=0; iii<slen; iii++)
			{
				if ((lineBuff[iii] == 0x0d) || (lineBuff[iii] == 0x0a))
				{
					lineBuff[iii]	=	0;
					break;
				}
			}
			slen	=	strlen(lineBuff);
			if ((slen > 3) && (lineBuff[0] != '#'))
			{
				validEntry	=	ProcessSQLserverLine(lineBuff);
				if (validEntry == false)
				{
					CONSOLE_DEBUG_W_STR("SQL Config file contains invalid data:", lineBuff);
				}
			}

		}
		//*	check the configuration, makes sure that all the required parameters were specified
		configOK	=	CheckSQLconfiguration();
		fclose(filePointer);

		//*	set the current database to the first one in the list
	}
	else
	{
		strcpy(gDataBaseNames[0].Name, "gaia");
		gDataBaseNameCnt	=	1;
	}
	if (gDataBaseNameCnt > 0)
	{
		strcpy(gSQLsever_Database, gDataBaseNames[0].Name);
	}
//	CONSOLE_DEBUG("--------------------------------");
//	for (iii=0; iii<gDataBaseNameCnt; iii++)
//	{
//		CONSOLE_DEBUG_W_STR("Database name\t=", gDataBaseNames[iii].Name);
//	}
//
//	CONSOLE_DEBUG_W_NUM("gDataBaseNameCnt\t=", gDataBaseNameCnt);
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

//*****************************************************************************
static	int	LogSQLuser(void)
{
MYSQL 			*mySQLConnection = NULL;
char			mySQLCmd[256];
int				returnCode;
char			userString[64];

//	CONSOLE_DEBUG(__FUNCTION__);

	returnCode		=	-1;

	//*	figure out which user name to use
	if (strlen(gObseratorySettings.Observer) > 0)
	{
		strcpy(userString, gObseratorySettings.Observer);
	}
	else if (strlen(gObseratorySettings.Owner) > 0)
	{
		strcpy(userString, gObseratorySettings.Owner);
	}
	else if (strlen(gObseratorySettings.Name) > 0)
	{
		strcpy(userString, gObseratorySettings.Name);
	}
	else
	{
		//*	last resrot
		strcpy(userString, "Unknown");
	}

	if (strlen(userString) >= 32)
	{
		userString[31]	=	0;
	}

	sprintf(mySQLCmd, "call SetLogComment('%s');", userString);
//	CONSOLE_DEBUG_W_STR("mySQLCmd\t=", mySQLCmd);

	mySQLConnection	=	mysql_init(NULL);
	if (mySQLConnection != NULL)
	{
//		CONSOLE_DEBUG(__FUNCTION__);

		//*	establish connection to the database
		if (mysql_real_connect(	mySQLConnection,
								gSQLsever_IPaddr,
								gSQLsever_UserName,
								gSQLsever_Password,
								gSQLsever_Database, 0, NULL, 0) != NULL)
		{
//			CONSOLE_DEBUG_W_STR("Successfully connected to", gSQLsever_IPaddr);

			returnCode	=	mysql_select_db(mySQLConnection, gSQLsever_Database);
			if (returnCode == 0)
			{
//				CONSOLE_DEBUG("mysql_select_db -- OK");
				returnCode	=	mysql_query(mySQLConnection, mySQLCmd);
				if (returnCode == 0)
				{
//					CONSOLE_DEBUG("mysql_query -- OK");
				}
				else
				{
					CONSOLE_DEBUG_W_NUM("mysql_query returnCode\t=", returnCode);
				}
			}
			else
			{
				CONSOLE_DEBUG_W_NUM("mysql_select_db returnCode\t=", returnCode);
			}

		}
		else
		{
			CONSOLE_DEBUG("mysql_real_connect() failed");
			LogSqlTransaction("mysql_real_connect() failed", "");
		}
	}
	else
	{
		CONSOLE_DEBUG("mysql_init() failed");
		LogSqlTransaction("mysql_init() failed", "");
	}
	mysql_close(mySQLConnection);
	mysql_library_end();

	return(returnCode);
}


//*****************************************************************************
//*	returns # of fields
static int Querry_mySQL_cmd(	MYSQL		*myCon,
								MYSQL_RES	**mySQLresult,
								char		*myDataBase,
								char		*mySQLCmd )
{
int		num_fields;
int		returnCode;


//	CONSOLE_DEBUG_W_STR(myDataBase,		mySQLCmd);
	num_fields	=	-1;

	returnCode	=	mysql_select_db(myCon, myDataBase);
	if (returnCode == 0)
	{
//		CONSOLE_DEBUG_W_NUM("mysql_select_db returnCode\t=", returnCode);

//		CONSOLE_DEBUG_W_STR("Calling mysql_query() with\t=", mySQLCmd);
		returnCode	=	mysql_query(myCon, mySQLCmd);
//		CONSOLE_DEBUG_W_NUM("mysql_query returnCode\t=", returnCode);
		if (returnCode == 0)
		{
//			CONSOLE_DEBUG("calling mysql_store_result()");
			*mySQLresult	=	mysql_store_result(myCon);

			if (*mySQLresult != NULL)
			{
				num_fields	=	mysql_num_fields(*mySQLresult);
//				CONSOLE_DEBUG_W_NUM("num_fields\t=", num_fields);
			}
			else
			{
				CONSOLE_DEBUG("Error on Querry_mySQL_cmd mysql_store_result");
			}
		}
		else
		{
			CONSOLE_DEBUG_W_NUM("mysql_query failed with returnCode\t=", returnCode);
		}
	}
	else
	{
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
												long	*totalRecords)
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

//	CONSOLE_DEBUG(__FUNCTION__);

	recNum		=	0;
	gaiaData	=	NULL;

	startMilliSecs	=	millis();

	mySQLConnection	=	mysql_init(NULL);

	if (mySQLConnection != NULL)
	{
//		CONSOLE_DEBUG(__FUNCTION__);

//		CONSOLE_DEBUG("Trying to establish connection to the database");
//		CONSOLE_DEBUG_W_STR("gSQLsever_IPaddr\t=",		gSQLsever_IPaddr);
//		CONSOLE_DEBUG_W_STR("gSQLsever_UserName\t=",	gSQLsever_UserName);
//		CONSOLE_DEBUG_W_STR("gSQLsever_Password\t=",	gSQLsever_Password);
//		CONSOLE_DEBUG_W_STR("gSQLsever_Database\t=",	gSQLsever_Database);

		//*	establish connection to the database
		//*	updated <KAS> 2/22/2022
		if (mysql_real_connect(	mySQLConnection,
									gSQLsever_IPaddr,
									gSQLsever_UserName,
									gSQLsever_Password,
									gSQLsever_Database,
									0,
									NULL,
									CLIENT_MULTI_RESULTS) != NULL)
		{
//			CONSOLE_DEBUG_W_STR("Successfully connected to", gSQLsever_IPaddr);
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

//			CONSOLE_DEBUG(mySQLCmd);
//			CONSOLE_DEBUG("Calling Querry_mySQL_cmd()");

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
							localStarData.ra		=	RADIANS(atof(row[1]));
							localStarData.org_ra	=	localStarData.ra;

							//*	Get the DEC
							localStarData.decl		=	RADIANS(atof(row[2]));
							localStarData.org_decl	=	localStarData.decl;

							//*	Get the Magnitude
							if (row[3] != NULL)
							{
								localStarData.realMagnitude		=	atof(row[3]);
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
								bp_rp		=	atof(row[4]);
								localStarData.spectralClass		=	CalcSpectralClassFrom_BP_RP(bp_rp);
							}
							else
							{
								localStarData.spectralClass		=	0;
							}


							localStarData.dataSrc	=	kDataSrc_GAIA_gedr3;
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
//				CONSOLE_DEBUG("Calling mysql_free_result()");
				mysql_free_result(mySQLresult);

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
					LogSqlTransaction(mySQLCmd, textBuff);
				}
			}
			else
			{
				LogSqlTransaction(mySQLCmd, "num_fields <= 0");
			}
//			CONSOLE_DEBUG("Done");
		}
		else
		{
			LogSqlTransaction("mysql_real_connect() failed", "");
		}
	}
	else
	{
		LogSqlTransaction("mysql_init() failed", "");
	}
	mysql_close(mySQLConnection);
	mysql_library_end();

	*totalRecords	=	recNum;
	return(gaiaData);
}


//*****************************************************************************
//*****************************************************************************
bool	GetSQLdataFromIDnumber(const char *gaiaIDnumberStr, TYPE_CelestData *gaiaData)
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
bool			validFlag;

//	CONSOLE_DEBUG("-------------------------------------------------------------");
//	CONSOLE_DEBUG(__FUNCTION__);
//	CONSOLE_DEBUG_W_STR("Searching SQL database for", gaiaIDnumberStr);

	recNum		=	0;

	startMilliSecs	=	millis();

	mySQLConnection	=	mysql_init(NULL);
	validFlag		=	false;
	if (mySQLConnection != NULL)
	{
//		CONSOLE_DEBUG(__FUNCTION__);
//
//		CONSOLE_DEBUG("Trying to establish connection to the database");
//		CONSOLE_DEBUG_W_STR("gSQLsever_IPaddr\t=",		gSQLsever_IPaddr);
//		CONSOLE_DEBUG_W_STR("gSQLsever_UserName\t=",	gSQLsever_UserName);
//		CONSOLE_DEBUG_W_STR("gSQLsever_Password\t=",	gSQLsever_Password);
//		CONSOLE_DEBUG_W_STR("gSQLsever_Database\t=",	gSQLsever_Database);

		//*	establish connection to the database
		if (mysql_real_connect(	mySQLConnection,
								gSQLsever_IPaddr,
								gSQLsever_UserName,
								gSQLsever_Password,
								gSQLsever_Database, 0, NULL, 0) != NULL)
		{
//			CONSOLE_DEBUG_W_STR("Successfully connected to", gSQLsever_IPaddr);
		//	sprintf(mySQLCmd,"call GetGaiaSourceID(%s);", gaiaIDnumberStr);
			sprintf(mySQLCmd,"call GetStarFromID(%s);", gaiaIDnumberStr);
//			CONSOLE_DEBUG(mySQLCmd);
//			CONSOLE_DEBUG("Calling Querry_mySQL_cmd()");

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

					validFlag	=	true;
					recNum		=	1;
					//*	Get the name
//					CONSOLE_DEBUG_W_STR("row[0]\t=",	row[0]);
					strcpy(localStarData.longName, row[0]);
//					CONSOLE_DEBUG_W_STR("longName\t=",	localStarData.longName);

					//*	Get the RA
					localStarData.ra		=	RADIANS(atof(row[1]));
					localStarData.org_ra	=	localStarData.ra;

					//*	Get the DEC
					localStarData.decl		=	RADIANS(atof(row[2]));
					localStarData.org_decl	=	localStarData.decl;

					//*	Get the Magnitude
					if (row[3] != NULL)
					{
						localStarData.realMagnitude		=	atof(row[3]);
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
						bp_rp		=	atof(row[4]);
						localStarData.spectralClass		=	CalcSpectralClassFrom_BP_RP(bp_rp);
					}
					else
					{
						localStarData.spectralClass		=	0;
					}

					localStarData.dataSrc	=	kDataSrc_GAIA_gedr3;
				}
				else
				{
					CONSOLE_DEBUG("NO RESULTS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
					CONSOLE_DEBUG_W_NUM("num_fields", num_fields);
					CONSOLE_DEBUG_W_NUM("num_rows", num_rows);
				}
//				CONSOLE_DEBUG("Calling mysql_free_result()");
				mysql_free_result(mySQLresult);

				endMilliSecs	=	millis();

				if (gEnableSQLlogging)
				{
				char	textBuff[128];

					sprintf(textBuff, "Records rcvd=%d\tQuery time=%d", recNum, (endMilliSecs - startMilliSecs));
					LogSqlTransaction(mySQLCmd, textBuff);
				}
			}
			else
			{
				LogSqlTransaction("num_fields <= 0", "");
			}
//			CONSOLE_DEBUG("Done");
		}
		else
		{
			LogSqlTransaction("mysql_real_connect() failed", "");
		}
	}
	else
	{
		LogSqlTransaction("mysql_init() failed", "");
	}
	mysql_close(mySQLConnection);
	mysql_library_end();

	*gaiaData	=	localStarData;
//	CONSOLE_DEBUG(__FUNCTION__);

	return(validFlag);
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
																&gaiaDataCount);

						endMilliSecs	=	millis();
						requestOccured	=	true;
						CONSOLE_DEBUG_W_LONG("gaiaDataCount\t=", gaiaDataCount);

						//*	make sure we got some valid data
						if ((gaiaData != NULL) && (gaiaDataCount > 0))
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

							strcpy(gSQLsever_StatusMsg, "SQL success");
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
								strcpy(gSQLsever_StatusMsg, "Too many SQL errors, sleeping for 5 minutes");
								gSQLsever_MsgUpdated	=	true;
								CONSOLE_DEBUG(gSQLsever_StatusMsg);
								sleep(5 * 60);
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
void	LogSqlTransaction(const char *sqlCommand, const char *results)
{
FILE			*filePointer;
struct timeval	timeStamp;
char			formatString[48];

	CONSOLE_DEBUG(__FUNCTION__);

	filePointer	=	fopen("SQL_log.txt", "a");
	if (filePointer != NULL)
	{
		gettimeofday(&timeStamp, NULL);
		FormatDateTimeString_Local(&timeStamp, formatString);

		fprintf(filePointer, "%s\t%s\t%s\n",	formatString, sqlCommand, results);
		fclose(filePointer);
	}
	else
	{
		CONSOLE_DEBUG("Failed to create sql log file");
	}
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
