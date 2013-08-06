/*
   File:           global.cpp

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Global state variables.
*/

#include "global.h"

/* Global variables */

int score_sheet_row;
ALIGN_PREFIX(64) int piece_count[3][MAX_SEARCH_DEPTH] ALIGN_SUFFIX(64);
int black_moves[60+4];
int white_moves[60+4];

FILE *g_stdout = NULL;

#ifdef DOLPHIN_GGS

CODKStream *g_odk_stream = NULL;

CODKStream *odk_stream_init(CODKStream *gs) {
	CODKStream *old_gs = g_odk_stream;
	g_odk_stream = gs;
	return old_gs;
}

void odk_stream_release( void ) {
	g_odk_stream = NULL;
}

#endif  // DOLPHIN_GGS
