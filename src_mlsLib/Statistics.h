//*****************************************************************************
//*	Statistics.h
//*****************************************************************************
//#include "Statistics.h"



double	STAT_Rand_SD(double mu, double sigma);
void	STAT_GenerateData_SD(const double mean, const double stadDev, const int numValues, double *dataArray);
double	STAT_CalcMean(const int numValues, const double *dataArray);
double	STAT_CalcVariance(const int numValues, const double *dataArray);
double	STAT_CalcStdDeviation(const int numValues, const double *dataArray);
double	STAT_GetMinimum(const int numValues, const double *dataArray);
double	STAT_GetMaximum(const int numValues, const double *dataArray);
double	STAT_CalcSigmaForX(double xx, double mean, double sigma);
double	STAT_CalcSigmaCurve(double mean, double sigma, int numValues, double *xValues, double *yValues);
