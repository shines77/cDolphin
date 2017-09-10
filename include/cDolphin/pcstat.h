/*
   File:           pcstat.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The declaration of the pcstat variables.
*/

#ifndef __PCSTAT_H_
#define __PCSTAT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_CORR_DEPTH       14

#define MAX_SHALLOW_DEPTH    8

typedef struct tagCorrelation {
    float const_base;
    float const_slope;
    float sigma_base;
    float sigma_slope;
} Correlation;

extern Correlation mid_corr[61][MAX_SHALLOW_DEPTH + 1];

#ifdef __cplusplus
}
#endif

#endif  /* __PCSTAT_H_ */
