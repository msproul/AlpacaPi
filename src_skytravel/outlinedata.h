//************************************************************************
//*	May 29,	2022	<MLS> Created outlinedata.h
//************************************************************************
//#include	"outlinedata.h"

#ifndef	_OUTLINE_DATA_H_

#define	_OUTLINE_DATA_H_


//************************************************************************
typedef struct
{
		double	ra_rad;
		double	decl_rad;
		short	flag;
		short	level;		//*	this was added for the milkyway data
} TYPE_OutlineData;


#endif // _OUTLINE_DATA_H_
