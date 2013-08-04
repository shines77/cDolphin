/*
   File:           pv.cpp

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Some routines about pv sequence.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "global.h"
#include "colour.h"
#include "board.h"
#include "bitboard.h"
#include "endgame.h"
#include "counter.h"
#include "hash.h"
#include "move.h"
#include "pv.h"

/* Global variables */

__declspec(align(64)) int pv[MAX_SEARCH_DEPTH][MAX_SEARCH_DEPTH];
__declspec(align(64)) int pv_depth[MAX_SEARCH_DEPTH];

int full_pv_depth;
int full_pv[120];

/* Local variables */

static int prefix_move = 0;

/*
  GET_PV
  Returns the principal variation.
*/

int
get_pv( int *destin ) {
	int i;

	if ( prefix_move == 0 ) {
		for ( i = 0; i < pv_depth[0]; i++ )
			destin[i] = pv[0][i];
		return pv_depth[0];
	}
	else {
		destin[0] = prefix_move;
		for ( i = 0; i < pv_depth[0]; i++ )
			destin[i + 1] = pv[0][i];
		return pv_depth[0] + 1;
	}
}

/*
   STORE_PV
   Saves the principal variation (the first row of the PV matrix).
*/

INLINE void
store_pv( int *pv_buffer, int *depth_buffer ) {
	int i;

	for ( i = 0; i < pv_depth[0]; i++ )
		pv_buffer[i] = pv[0][i];
	*depth_buffer = pv_depth[0];
}

/*
   RESTORE_PV
   Put the stored principal variation back into the PV matrix.
*/

INLINE void
restore_pv( int *pv_buffer, int depth_buffer ) {
	int i;

	for ( i = 0; i < depth_buffer; i++ )
		pv[0][i] = pv_buffer[i];
	pv_depth[0] = depth_buffer;
}

/*
  CLEAR_PV
  Clears the principal variation.
*/

INLINE void
clear_pv( void ) {
	pv_depth[0] = 0;
}

/*
  COMPLETE_PV
  Complete the principal variation with passes (if any there are any).
*/

void
complete_pv( BitBoard my_bits,
			BitBoard opp_bits, int color ) {
	int i, move;
	int actual_color[60];
	BitBoard tmp_bits;

	/*
	int save_hash1, save_hash2;
	save_hash1 = g_hash1;
	save_hash2 = g_hash2;
	//*/

	full_pv_depth = 0;
	for ( i = 0; i < pv_depth[0]; i++ ) {
		move = pv[0][i];
		if ( ( (my_bits.low & square_mask[move].low) == 0 &&
			(my_bits.high & square_mask[move].high) == 0 &&
			(opp_bits.low & square_mask[move].low) == 0 &&
			(opp_bits.high & square_mask[move].high) == 0 ) &&
			(make_move( &my_bits, &opp_bits, color, pv[0][i], TRUE ) > 0) ) {
			actual_color[i] = color;
			full_pv[full_pv_depth] = pv[0][i];
			full_pv_depth++;
		}
		else {
			full_pv[full_pv_depth] = PASS_MOVE;
			full_pv_depth++;
			color = OPP_COLOR( color );
			tmp_bits = my_bits;
			my_bits = opp_bits;
			opp_bits = tmp_bits;
			move = pv[0][i];
			if ( ( (my_bits.low & square_mask[move].low) == 0 &&
				(my_bits.high & square_mask[move].high) == 0 &&
				(opp_bits.low & square_mask[move].low) == 0 &&
				(opp_bits.high & square_mask[move].high) == 0 ) &&
				(make_move( &my_bits, &opp_bits, color, pv[0][i], TRUE ) > 0) ) {
				actual_color[i] = color;
				full_pv[full_pv_depth] = pv[0][i];
				full_pv_depth++;
			}
			else {
#ifdef TEXT_BASED
				int j;

				printf( "pv_depth[0] = %d\n", pv_depth[0] );
				for ( j = 0; j < pv_depth[0]; j++ )
					printf( "%c%c ", TO_SQUARE( pv[0][j] ) );
				puts( "" );
				printf( "i=%d\n", i );
#endif
				printf( "PV error" );
				exit( EXIT_FAILURE );
			}
		}
		color = OPP_COLOR( color );
		tmp_bits = my_bits;
		my_bits = opp_bits;
		opp_bits = tmp_bits;
	}

	for ( i = pv_depth[0] - 1; i >= 0; i-- )
		unmake_move( actual_color[i], pv[0][i] );

	/*
	g_hash1 = save_hash1;
	g_hash2 = save_hash2;
	//*/
}

/*
  HASH_EXPAND_PV
  Pad the existing PV with the move sequence suggested by the hash table.
*/

void
hash_expand_pv( BitBoard my_bits,
			   BitBoard opp_bits, int color,
			   int mode, int flags,
			   int max_selectivity ) {
	int i, move;
	int pass_count;
	int new_pv_depth;
	int new_pv[64];
	int new_color[64];
	BitBoard tmp_bits;
	HashEntry entry;

	/*
	int save_hash1, save_hash2;
	save_hash1 = g_hash1;
	save_hash2 = g_hash2;
	//*/

	hash_determine_values( my_bits, opp_bits, color );
	new_pv_depth = 0;
	pass_count = 0;

	while ( pass_count < 2 ) {
		new_color[new_pv_depth] = color;
		if ( (new_pv_depth < pv_depth[0]) && (new_pv_depth == 0) ) {
			move = pv[0][new_pv_depth];
			if ( ( (my_bits.low & square_mask[move].low) == 0 &&
				  (my_bits.high & square_mask[move].high) == 0 &&
				  (opp_bits.low & square_mask[move].low) == 0 &&
				  (opp_bits.high & square_mask[move].high) == 0 ) &&
				( prepare_movelist_pv( my_bits, opp_bits, move ) ) &&
				( make_move( &my_bits, &opp_bits, color, move, TRUE ) != 0 ) ) {
				new_pv[new_pv_depth] = move;
				new_pv_depth++;
				pass_count = 0;
			}
			else {
				g_hash1 ^= g_hash_switch_side1;
				g_hash2 ^= g_hash_switch_side2;
				pass_count++;
			}
		}
		else {
			entry = hash_find( mode );
			move = entry.move[0];
			if ( (entry.draft != NO_HASH_MOVE) &&
				(entry.flags & flags) &&
				(entry.selectivity <= max_selectivity) &&
				( (my_bits.low & square_mask[move].low) == 0 &&
				  (my_bits.high & square_mask[move].high) == 0 &&
				  (opp_bits.low & square_mask[move].low) == 0 &&
				  (opp_bits.high & square_mask[move].high) == 0 ) &&
				( prepare_movelist_pv( my_bits, opp_bits, move ) ) &&
				( make_move( &my_bits, &opp_bits, color, move, TRUE ) != 0) ) {
				new_pv[new_pv_depth] = move;
				new_pv_depth++;
				pass_count = 0;
			}
			else {
				g_hash1 ^= g_hash_switch_side1;
				g_hash2 ^= g_hash_switch_side2;
				pass_count++;
			}
		}
		color = OPP_COLOR( color );
		tmp_bits = my_bits;
		my_bits = opp_bits;
		opp_bits = tmp_bits;
	}
	for ( i = new_pv_depth - 1; i >= 0; i-- )
		unmake_move( new_color[i], new_pv[i] );
	for ( i = 0; i < new_pv_depth; i++ )
		pv[0][i] = new_pv[i];
	pv_depth[0] = new_pv_depth;

	/*
	g_hash1 = save_hash1;
	g_hash2 = save_hash2;
	//*/
}

/*
   FULL_EXPAND_PV
   Pad the PV with optimal moves in the low-level phase.
*/

void
full_expand_pv( BitBoard my_bits,
			   BitBoard opp_bits,
			   int color,
			   int selectivity ) {
	int i, movecount;
	int pass_count;
	int new_pv_depth;
	int new_pv[64];
	int new_color[64];
	BitBoard tmp_bits;

	/*
	int save_hash1, save_hash2;
	save_hash1 = g_hash1;
	save_hash2 = g_hash2;
	//*/

	new_pv_depth = 0;
	pass_count = 0;
	while ( pass_count < 2 ) {
		int move;

		generate_all( my_bits, opp_bits, color );
		if ( move_count[disks_played] > 0 ) {
			int empties = 64 - disc_count( my_bits, opp_bits, TRUE )
				- disc_count( my_bits, opp_bits, FALSE );

			(void) end_tree_wrapper( new_pv_depth, empties, my_bits, opp_bits,
				color, -64, 64, selectivity, TRUE );
			move = pv[new_pv_depth][new_pv_depth];

			movecount = make_move( &my_bits, &opp_bits, color, move, TRUE );
			if (movecount <= 0) {
				g_hash1 ^= g_hash_switch_side1;
				g_hash2 ^= g_hash_switch_side2;
				pass_count++;
				if (pass_count >= 2)
					break;
			}
			else {
				pass_count = 0;
				new_pv[new_pv_depth] = move;
				new_color[new_pv_depth] = color;
				if (new_pv_depth < 64 - 1)
					new_pv_depth++;
				else
					break;
			}
		}
		else {
			g_hash1 ^= g_hash_switch_side1;
			g_hash2 ^= g_hash_switch_side2;
			pass_count++;
		}
		color = OPP_COLOR( color );
		tmp_bits = my_bits;
		my_bits = opp_bits;
		opp_bits = tmp_bits;
	}
	for ( i = new_pv_depth - 1; i >= 0; i-- )
		unmake_move( new_color[i], new_pv[i] );
	for ( i = 0; i < new_pv_depth; i++ )
		pv[0][i] = new_pv[i];
	pv_depth[0] = new_pv_depth;

	/*
	g_hash1 = save_hash1;
	g_hash2 = save_hash2;
	//*/
}
