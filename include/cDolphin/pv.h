/*
   File:           pv.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The interface to the PV sequence.
*/

#ifndef __PV_H_
#define __PV_H_

#include "global.h"
#include "bitboard.h"
#include "endgame.h"

#ifdef __cplusplus
extern "C" {
#endif

/* pv[n][n..<depth>] contains the principal variation from the
   node on recursion depth n on the current recursive call sequence.
   After the search, pv[0][0..<depth>] contains the principal
   variation from the root position. */
extern __declspec(align(64)) int pv[MAX_SEARCH_DEPTH][MAX_SEARCH_DEPTH];

/* pv_depth[n] contains the depth of the principal variation
   starting at level n in the call sequence.
   After the search, pv[0] holds the depth of the principal variation
   from the root position. */
extern __declspec(align(64)) int pv_depth[MAX_SEARCH_DEPTH];

/* The principal variation including passes */
extern int full_pv_depth;
extern int full_pv[120];

int
get_pv( int *destin );

void
store_pv( int *pv_buffer, int *depth_buffer );

void
restore_pv( int *pv_buffer, int depth_buffer );

void
clear_pv( void );

void
complete_pv( BitBoard my_bits,
			BitBoard opp_bits, int color );

void
hash_expand_pv( BitBoard my_bits,
			   BitBoard opp_bits,
			   int color, int mode,
			   int flags, int max_selectivity );

void
full_expand_pv( BitBoard my_bits,
			   BitBoard opp_bits,
			   int color,
			   int selectivity );

#ifdef __cplusplus
}
#endif

#endif  /* __PV_H_ */
