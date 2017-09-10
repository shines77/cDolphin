/*
   File:           probcut.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The declaration of the Multi-Prob-Cut variables.
*/

#ifndef __PROBCUT_H_
#define __PROBCUT_H_

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PERCENTILE                 1.0

#define MAX_CUT_DEPTH              22

typedef struct tagDepthInfo {
    int cut_tries;
    int cut_depth[2];
    int bias[2][61];
    int window[2][61];
    //char alignment_padding[36];  /* In order to achieve 1024-byte alignment */
} DepthInfo;

extern int use_end_cut[61];
extern int end_mpc_depth[61][4];
extern ALIGN_PREFIX(64) DepthInfo mpc_cut[MAX_CUT_DEPTH + 1] ALIGN_SUFFIX(64) ALIGN_SUFFIX(64);

void
init_probcut(void);

#ifdef __cplusplus
}
#endif

#endif  /* __PROBCUT_H_ */
