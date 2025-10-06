#ifndef CALCULATE_STATISTICS_H
#define CALCULATE_STATISTICS_H

#include "dem.h"

void calculateStatistics(const CDEM& dem, double* min, double* max, double* mean, double* stdDev);

#endif // CALCULATE_STATISTICS_H

