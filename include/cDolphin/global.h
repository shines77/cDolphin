/*
   File:           global.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Global state variables.
*/

#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include <stdio.h>

#ifdef DOLPHIN_GGS
#include "..\Dolphin_GGS\ODKStream.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Miscellaneous */
#define MAX_SEARCH_DEPTH            64
#define PASS_MOVE                   -1

extern int score_sheet_row;

/* piece_count[col][n] holds the number of disks of color col after
   n moves have been played. */
extern __declspec(align(64)) int piece_count[3][MAX_SEARCH_DEPTH];

/* These variables hold the game score. The meaning is similar
   to how a human would fill out a game score except for that
   the row counter, score_sheet_row, starts at zero. */
extern int black_moves[60+4];
extern int white_moves[60+4];

extern FILE *g_stdout;

#ifdef __cplusplus
}
#endif


#ifdef DOLPHIN_GGS

extern CODKStream *g_odk_stream;

extern CODKStream *odk_stream_init(CODKStream *gs);

extern void odk_stream_release(void);

#endif  // DOLPHIN_GGS


#endif  /* __GLOBAL_H_ */
