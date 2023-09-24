//***************************************************************************************
//*	Console debugging
//*
//*
//***************************************************************************************
//*	Edit History
//***************************************************************************************
//*	Jul 12,	2012	<MLS> Created ConsoleDebug.h
//*	Nov 30,	2012	<MLS> Added CONSOLE_ERROR macros, same as CONSOLE_DEBUG but always print
//*	Sep  6,	2018	<MLS> Re-arranged ifdefs such that stdio is not included if debuging not enabled
//*	Apr 29,	2019	<MLS> Added CONSOLE_ERROR_W_INT32()
//*	Dec 25,	2019	<MLS> Added checking for 8 byte longs
//*	Jun 22,	2020	<MLS> Added debug timing to ConsoleDebug.h
//*	Jun 30,	2020	<MLS> Added CONSOLE_ABORT()
//*	Aug 10,	2021	<MLS> Added CONSOLE_DEBUG_W_LHEX()	long hex
//*	Apr 18,	2022	<MLS> Added CONSOLE_DEBUG_W_BOOL()
//*	Mar  7,	2023	<MLS> Added CONSOLE_DEBUG_W_SIZE()
//*	Sep  5,	2023	<MLS> Added CONSOLE_DEBUG_W_SIZE() to quite mode
//***************************************************************************************
//#include	"ConsoleDebug.h"


#ifndef	_CONSOLE_DEBUG_H_
#define	_CONSOLE_DEBUG_H_

//*	this prints out the file, line number function, and message to the console,
//*	it also does a flush to insure the data goes out in case of a crash
//*	when its time to make a production version, comment out _ENABLE_CONSOLE_DEBUG_
#if defined(__XWIN__) || defined(_GDLibrary_)
//	#define	_ENABLE_CONSOLE_DEBUG_
#endif
//	#define	_ENABLE_CONSOLE_DEBUG_




#if defined(_SINGLE_STEP_DEBUG_)
	//*	use these defines for debugging
	#define	CONSOLE_DEBUG(msg)							printf("%-40s:%4d [%-20s] %s\n",			__FILE__, __LINE__, __FUNCTION__, msg);			fflush(stdout);		getc(stdin);
	#define	CONSOLE_DEBUG_W_NUM(msg, num)				printf("%-40s:%4d [%-20s] %s %d\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);		getc(stdin);
	#define	CONSOLE_DEBUG_W_INT32(msg, num)				printf("%-40s:%4d [%-20s] %s %ld\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);		getc(stdin);
	#define	CONSOLE_DEBUG_W_LONG(msg, num)				printf("%-40s:%4d [%-20s] %s %ld\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);		getc(stdin);
	#define	CONSOLE_DEBUG_W_DBL(msg, num)				printf("%-40s:%4d [%-20s] %s %f\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);		getc(stdin);
	#define	CONSOLE_DEBUG_W_HEX(msg, num)				printf("%-40s:%4d [%-20s] %s %02X\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);		getc(stdin);
	#define	CONSOLE_DEBUG_W_STR(msg, str)				printf("%-40s:%4d [%-20s] %s %s\n", 		__FILE__, __LINE__, __FUNCTION__, msg, str);	fflush(stdout);		getc(stdin);
	#define	CONSOLE_DEBUG_W_2STR(msg, str1, str2)		printf("%-40s:%4d [%-20s] %s %s, %s\n",		__FILE__, __LINE__, __FUNCTION__, msg, str1, str2);	fflush(stdout);		getc(stdin);
	#define	CONSOLE_DEBUG_W_3STR(msg, str1, str2, str3)	printf("%-40s:%4d [%-20s] %s %s, %s %s\n",	__FILE__, __LINE__, __FUNCTION__, msg, str1, str2, str3);	fflush(stdout);		getc(stdin);
#elif defined(_ENABLE_CONSOLE_DEBUG_)

	//*	use these defines for debugging
	#define	CONSOLE_DEBUG(msg)							printf("%-40s:%4d [%-20s] %s\n",			__FILE__, __LINE__, __FUNCTION__, msg);			fflush(stdout);
	#define	CONSOLE_DEBUG_W_BOOL(msg, tfValue)			printf("%-40s:%4d [%-20s] %s %s\n", 		__FILE__, __LINE__, __FUNCTION__, msg, (tfValue ? "TRUE" : "FALSE"));	fflush(stdout);
	#define	CONSOLE_DEBUG_W_NUM(msg, num)				printf("%-40s:%4d [%-20s] %s %d\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);
	#define	CONSOLE_DEBUG_W_NUM(msg, num)				printf("%-40s:%4d [%-20s] %s %d\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);
	#define	CONSOLE_DEBUG_W_LONG(msg, num)				printf("%-40s:%4d [%-20s] %s %ld\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);
	#define	CONSOLE_DEBUG_W_DBL(msg, num)				printf("%-40s:%4d [%-20s] %s %3.25f\n", 	__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);
	#define	CONSOLE_DEBUG_W_HEX(msg, num)				printf("%-40s:%4d [%-20s] %s 0x%02X\n", 	__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);
	#define	CONSOLE_DEBUG_W_LHEX(msg, num)				printf("%-40s:%4d [%-20s] %s 0x%02lX\n", 	__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);
	#define	CONSOLE_DEBUG_W_STR(msg, str)				printf("%-40s:%4d [%-20s] %s %s\n", 		__FILE__, __LINE__, __FUNCTION__, msg, str);	fflush(stdout);
	#define	CONSOLE_DEBUG_W_2STR(msg, str1, str2)		printf("%-40s:%4d [%-20s] %s %s, %s\n",		__FILE__, __LINE__, __FUNCTION__, msg, str1, str2);	fflush(stdout);
	#define	CONSOLE_DEBUG_W_3STR(msg, str1, str2, str3)	printf("%-40s:%4d [%-20s] %s %s, %s %s\n",	__FILE__, __LINE__, __FUNCTION__, msg, str1, str2, str3);	fflush(stdout);

	#if (__SIZEOF_SIZE_T__ == 8)
		#define	CONSOLE_DEBUG_W_SIZE(msg, num)			printf("%-40s:%4d [%-20s] %s %ld\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);
	#else
		#define	CONSOLE_DEBUG_W_SIZE(msg, num)			printf("%-40s:%4d [%-20s] %s %d\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);
	#endif

	#define	CONSOLE_ABORT(msg)							printf("%-40s:%4d [%-20s] %s\n",			__FILE__, __LINE__, __FUNCTION__, msg);			fflush(stdout); \
														printf("!!!!!!!!ABORT!!!!!!!!\n"); exit(0);

	#if (__SIZEOF_LONG__ == 8)
		#define	CONSOLE_DEBUG_W_INT32(msg, num)				printf("%-40s:%4d [%-20s] %s %d\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);
	#else
		#define	CONSOLE_DEBUG_W_INT32(msg, num)				printf("%-40s:%4d [%-20s] %s %d\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stdout);
	#endif

#else

	//*	disable macros
	//*	use these defines for production
	#define	CONSOLE_DEBUG(msg)
	#define	CONSOLE_DEBUG_W_NUM(msg, num)
	#define	CONSOLE_DEBUG_W_INT32(msg, num)
	#define	CONSOLE_DEBUG_W_LONG(msg, num)
	#define	CONSOLE_DEBUG_W_DBL(msg, num)
	#define	CONSOLE_DEBUG_W_HEX(msg, num)
	#define	CONSOLE_DEBUG_W_SIZE(msg, num)
	#define	CONSOLE_DEBUG_W_STR(msg, str)
	#define	CONSOLE_DEBUG_W_2STR(msg, str1, str2)
	#define	CONSOLE_DEBUG_W_3STR(msg, str1, str2, str3)
	#define	CONSOLE_ABORT(msg)
#endif



#if defined(_SINGLE_STEP_DEBUG_) || defined(_ENABLE_CONSOLE_DEBUG_)
	#ifndef _STDIO_H
		#include	<stdio.h>
	#endif

	#define	CONSOLE_ERROR(msg)							fprintf(stderr, "%-40s:%4d [%-20s] %s\n",			__FILE__, __LINE__, __FUNCTION__, msg);			fflush(stderr);
	#define	CONSOLE_ERROR_W_NUM(msg, num)				fprintf(stderr, "%-40s:%4d [%-20s] %s %d\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stderr);
	#define	CONSOLE_ERROR_W_INT32(msg, num)				fprintf(stderr, "%-40s:%4d [%-20s] %s %ld\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stderr);
	#define	CONSOLE_ERROR_W_DBL(msg, num)				fprintf(stderr, "%-40s:%4d [%-20s] %s %3.15f\n", 	__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stderr);
	#define	CONSOLE_ERROR_W_HEX(msg, num)				fprintf(stderr, "%-40s:%4d [%-20s] %s %X\n", 		__FILE__, __LINE__, __FUNCTION__, msg, num);	fflush(stderr);
	#define	CONSOLE_ERROR_W_STR(msg, str)				fprintf(stderr, "%-40s:%4d [%-20s] %s %s\n", 		__FILE__, __LINE__, __FUNCTION__, msg, str);	fflush(stderr);
	#define	CONSOLE_ERROR_W_2STR(msg, str1, str2)		fprintf(stderr, "%-40s:%4d [%-20s] %s %s, %s\n", 	__FILE__, __LINE__, __FUNCTION__, msg, str1, str2);	fflush(stderr);
#else
	//*	disable macros
	#define	CONSOLE_ERROR(msg)
	#define	CONSOLE_ERROR_W_NUM(msg, num)
	#define	CONSOLE_ERROR_W_INT32(msg, num)
	#define	CONSOLE_ERROR_W_DBL(msg, num)
	#define	CONSOLE_ERROR_W_HEX(msg, num)
	#define	CONSOLE_ERROR_W_STR(msg, str)
	#define	CONSOLE_ERROR_W_2STR(msg, str1, str2)
#endif


//***************************************************************************************
#if defined(_DEBUG_TIMING_) && defined(_ENABLE_CONSOLE_DEBUG_)
#include	<stdint.h>

#ifdef __cplusplus
	extern "C" {
#endif
	uint32_t	millis(void);
#ifdef __cplusplus
}
#endif

	#define		SETUP_TIMING()					\
		uint32_t		tStartMillisecs;		\
		uint32_t		tStopMillisecs;			\
		uint32_t		tDeltaMillisecs;		\
		tStartMillisecs	=	millis();


	#define		START_TIMING()					\
		tStartMillisecs	=	millis();


	//*	this macro calculates delta milliseconds for timing testing
	#define		DEBUG_TIMING(string)									\
				tStopMillisecs	=	millis();							\
				tDeltaMillisecs	=	tStopMillisecs - tStartMillisecs;	\
				CONSOLE_DEBUG_W_NUM(string,	tDeltaMillisecs);

#else
	#define		SETUP_TIMING()
	#define		START_TIMING()
	#define		DEBUG_TIMING(string)
#endif



#endif	//	_CONSOLE_DEBUG_H_
