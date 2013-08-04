/*
   File:           epcstat.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Some colour defines and constants.
*/

#ifndef __EPCSTAT_H_
#define __EPCSTAT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_END_CORR_DEPTH       8

extern float end_mean[61][MAX_END_CORR_DEPTH+1];

extern float end_sigma[61][MAX_END_CORR_DEPTH+1];

extern short end_stats_available[61][MAX_END_CORR_DEPTH+1];

#ifdef __cplusplus
}
#endif

#endif  /* __EPCSTAT_H_ */
