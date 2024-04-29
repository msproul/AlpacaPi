//*****************************************************************************
//*	Statistics.c
//*
//*		by Mark Sproul 2016
//*****************************************************************************
//*	Nov  9,	2016	<MLS> Started on Statistics.c
//*	Nov  9,	2016	<MLS> Added STAT_Rand_SD()
//*	Nov  9,	2016	<MLS> Added STAT_GenerateData_SD()
//*	Nov  9,	2016	<MLS> Added STAT_CalcMean()
//*	Nov  9,	2016	<MLS> Added STAT_CalcVariance()
//*	Nov  9,	2016	<MLS> Added STAT_CalcStdDeviation()
//*	Dec  9,	2016	<MLS> Added STAT_GetMinimum() & STAT_GetMaximum()
//*	Jan 26,	2017	<MLS> Added STAT_CalcSigmaForX() & STAT_CalcSigmaCurve()
//*****************************************************************************



#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#define	_ENABLE_CONSOLE_DEBUG_
#include "ConsoleDebug.h"


#include "Statistics.h"

#ifndef M_PI
	#define M_PI		(3.14159265358979323846)
#endif

//*****************************************************************************
double	STAT_Rand_SD(double mu, double sigma)
{
double U1, U2, W, mult;
static double X1, X2;
static int call	=	0;

	if (call == 1)
	{
		call	=	!call;
		return (mu + sigma * (double) X2);
	}

	do
	{
		U1	=	-1 + ((double) rand() / RAND_MAX) * 2;
		U2	=	-1 + ((double) rand() / RAND_MAX) * 2;

		W	=	pow (U1, 2) + pow (U2, 2);
	} while (W >= 1 || W == 0);

	mult	=	sqrt ((-2 * log (W)) / W);
	X1		=	U1 * mult;
	X2		=	U2 * mult;

	call	=	!call;

	return (mu + sigma * (double) X1);
}


//*****************************************************************************
void	STAT_GenerateData_SD(const double mu, const double stadDev, const int numValues, double *dataArray)
{
int	ii;

	if (dataArray != NULL)
	{
		for (ii=0; ii<numValues; ii++)
		{
			dataArray[ii]	=	STAT_Rand_SD(mu, stadDev);
		}
	}
}


//*****************************************************************************
double	STAT_CalcMean(const int numValues, const double *dataArray)
{
int		ii;
double	sum;

	sum	=	0;
	for (ii=0; ii<numValues; ii++)
	{
		sum	+=	dataArray[ii];
	}
	return(sum / numValues);
}


//*****************************************************************************
//*	http://www.sanfoundry.com/c-program-mean-variance-standard-deviation
double	STAT_CalcVariance(const int numValues, const double *dataArray)
{
int		ii;
double	sum1;
double	variance;
double	average;
double	delta;

	average	=	STAT_CalcMean(numValues, dataArray);
//	CONSOLE_DEBUG_W_DBL("average",		average);
	sum1	=	0;
	for (ii=0; ii<numValues; ii++)
	{
		delta	=	dataArray[ii] - average;
		sum1	=	sum1 + pow(delta, 2);
	}
	variance	=	sum1 / numValues;


//	CONSOLE_DEBUG_W_NUM("numValues",	numValues);
//	CONSOLE_DEBUG_W_DBL("sum1",			sum1);
//	CONSOLE_DEBUG_W_DBL("variance",		variance);



	return(variance);
}


//*****************************************************************************
//*	http://www.sanfoundry.com/c-program-mean-variance-standard-deviation
double	STAT_CalcStdDeviation(const int numValues, const double *dataArray)
{
double	variance;
double	std_deviation;

	variance		=	STAT_CalcVariance(numValues, dataArray);
	std_deviation	=	sqrt(variance);

	return(std_deviation);
}


//*****************************************************************************
double	STAT_GetMinimum(const int numValues, const double *dataArray)
{
double	myMin;
int		ii;

	myMin	=	9999999.0;
	for (ii=0; ii<numValues; ii++)
	{
		if (dataArray[ii] < myMin)
		{
			myMin	=	dataArray[ii];
		}
	}
	return(myMin);
}

//*****************************************************************************
double	STAT_GetMaximum(const int numValues, const double *dataArray)
{
double	myMax;
int		ii;

	myMax	=	-9999999.0;
	for (ii=0; ii<numValues; ii++)
	{
		if (dataArray[ii] > myMax)
		{
			myMax	=	dataArray[ii];
		}
	}
	return(myMax);
}


#pragma mark -
//*****************************************************************************
//*	https://en.wikipedia.org/wiki/Standard_deviation
//*****************************************************************************
double	STAT_CalcSigmaForX(double xx, double mean, double sigma)
{
double	term1;
double	term2;
double	sigmaYvalue;

	term1		=	1.0 / (sigma * sqrt(2 * M_PI));
	term2		=	exp(-0.5 * pow(((xx - mean) / sigma), 2));
	sigmaYvalue	=	term1 * term2;
	return(sigmaYvalue);
}

//*****************************************************************************
//*	returns the max value
double	STAT_CalcSigmaCurve(double mean, double sigma, int numValues, double *xValues, double *yValues)
{
int		ii;
double	maxValue;

	maxValue	=	0;
	for (ii=0; ii<numValues; ii++)
	{
		yValues[ii]	=	STAT_CalcSigmaForX(xValues[ii], mean, sigma);
		if (yValues[ii] > maxValue)
		{
			maxValue	=	yValues[ii];
		}
	}
	return(maxValue);
}


#if 0
#define	kNumValues	1000

//*****************************************************************************
int	main(int argc, char **argv)
{
double			array[kNumValues];
double			meanAvg;
double			variance;
double			std_deviation;
int				ii;
unsigned int	seedValue;

	if (argc > 1)
	{
		seedValue	=	atoi(argv[1]);
		srand(seedValue);
	}
	
	printf("Stat gen\r\n");

	STAT_GenerateData_SD(100.0, 3, kNumValues, array);

	for (ii=0; ii<kNumValues; ii++)
	{
//		printf("[%4d]=%f\r\n", ii, array[ii]);
	}
	
	meanAvg		=	STAT_CalcMean(kNumValues, array);
	variance	=	STAT_CalcVariance(kNumValues, array);
	
	std_deviation	=	sqrt(variance);
	
	CONSOLE_DEBUG_W_DBL("mean",				meanAvg);
	CONSOLE_DEBUG_W_DBL("variance",			variance);
	CONSOLE_DEBUG_W_DBL("std_deviation",	std_deviation);
}
#endif

