/*
   File:           stable.cpp

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Code which conservatively estimates the number of
                   stable (unflippable) discs using the concept
                   "Zardoz stability" along with edge tables.
*/

#include <stdio.h>

#include "utils.h"
#include "colour.h"
#include "board.h"
#include "bitboard.h"
#include "bitbtest.h"
#include "endgame.h"
#include "pattern.h"
#include "stable.h"

/* This constant is used in the DynP stuff for edge stability
   and simply denotes "value not known". */
#define  UNDETERMINED             -1

/* The maximum number of nodes to search when attempting
   a perfect stability assessment */
#define  MAX_STABILITY_NODES      10000

/* When this flag is set, the DynP tables are calculated and
   output and then the program is terminated. */
#define  DEBUG                    0

/* Global variables */

/* All discs determined as stable last time COUNT_STABLE was called
   for the two colors */
BitBoard last_black_stable, last_white_stable;

/* Local variables */

/* For each of the 3^8 edges, edge_stable[] holds an 8-bit mask
   where a bit is set if the corresponding disc can't be changed EVER. */
static int edge_stable[6561];

/* For each edge, *_stable[] holds the number of safe discs counted
   as follows: 1 for a stable corner and 2 for a stable non-corner.
   This to avoid counting corners twice. */
static int black_stable[6561], white_stable[6561];

/* A conversion table from the 2^8 edge values for one player to
   the corresponding base-3 value. */
static int base_conversion[256];

/* The base-3 indices for the edges */
static int edge_a1h1, edge_a8h8, edge_a1a8, edge_h1h8;

/* Position list used in the complete stability search */

MoveLink stab_move_list[66];

INLINE static void
apply_64( BitBoard *target,
		 BitBoard base,
		 unsigned int hi_mask,
		 unsigned int lo_mask ) {
	if ( ((base.high & hi_mask) == hi_mask) &&
		((base.low & lo_mask) == lo_mask) ) {
		target->high |= hi_mask;
		target->low |= lo_mask;
	}
}

INLINE static void
line_shift_64( BitBoard *target,
			  BitBoard *base,
			  int shift,
			  BitBoard filled ) {
	BitBoard base_shift;

	target->high = filled.high;
	target->low = filled.low;

	/* Shift to the left */

	base_shift.high = base->high << shift;
	base_shift.high |= base->low >> (32 - shift);
	base_shift.low = base->low << shift;

	target->high |= base_shift.high;
	target->low |= base_shift.low;

	/* Shift to the right */

	base_shift.low = base->low >> shift;
	base_shift.low |= base->high << (32 - shift);
	base_shift.high = base->high >> shift;

	target->high |= base_shift.high;
	target->low |= base_shift.low;
}

/*
  EDGE_ZARDOZ_STABLE
  Determines the bit mask for (a subset of) the stable discs in a position.
  Zardoz' algorithm + edge tables is used.
*/

static void
edge_zardoz_stable( BitBoard *dd,
				   BitBoard *od,
				   BitBoard *ss ) {
				   /* dd is the disks of the side we are looking for stable disks for
				   od is the opponent
	ss are the stable disks */

	BitBoard ost, fb, lrf, udf, daf, dbf;
	BitBoard expand_ss, dir_ss;

	/* ost is a simple test to see if numbers of
	   stable disks have stopped increasing.

	   fb is the squares which have been played
	   ie either by white or black

	   udf are the up-down columns that are filled, and so no vertical flips
	   lrf are the left-right
	   daf are the NE-SW diags filled
	   dbf are the NW-SE diags filled */

	/* a stable disk is a disk that has a stable disk on one
	   side in each of the 4 directions
	   N.B. beyond the edges is of course stable */

	fb.high = dd->high | od->high;
	fb.low = dd->low | od->low;

	lrf.high = fb.high;
	lrf.high &= (lrf.high >> 4);
	lrf.high &= (lrf.high >> 2);
	lrf.high &= (lrf.high >> 1);
	lrf.high = ((lrf.high & 0x01010101L) * 255) | 0x81818181;
	lrf.low = fb.low;
	lrf.low &= (lrf.low >> 4);
	lrf.low &= (lrf.low >> 2);
	lrf.low &= (lrf.low >> 1);
	lrf.low = ((lrf.low & 0x01010101L) * 255) | 0x81818181;

	udf.high = (fb.high & (fb.high >> 8) & (fb.high >> 16) & (fb.high >> 24));
	udf.low = (fb.low & (fb.low >> 8) & (fb.low >> 16) & (fb.low >> 24)) & 0xFF;
	udf.low &= udf.high;
	udf.low *= 0x01010101;
	udf.high = udf.low;
	udf.high |= 0xFF000000;
	udf.low |= 0x000000FF;

	daf.high = 0xFF818181;
	daf.low = 0x818181FF;
	apply_64( &daf, fb, 0x00000000, 0x00010204 );
	apply_64( &daf, fb, 0x00000000, 0x01020408 );
	apply_64( &daf, fb, 0x00000001, 0x02040810 );
	apply_64( &daf, fb, 0x00000102, 0x04081020 );
	apply_64( &daf, fb, 0x00010204, 0x08102040 );
	apply_64( &daf, fb, 0x01020408, 0x10204080 );
	apply_64( &daf, fb, 0x02040810, 0x20408000 );
	apply_64( &daf, fb, 0x04081020, 0x40800000 );
	apply_64( &daf, fb, 0x08102040, 0x80000000 );
	apply_64( &daf, fb, 0x10204080, 0x00000000 );
	apply_64( &daf, fb, 0x20408000, 0x00000000 );

	dbf.high = 0xFF818181;
	dbf.low = 0x818181FF;
	apply_64( &dbf, fb, 0x00000000, 0x00804020 );
	apply_64( &dbf, fb, 0x00000000, 0x80402010 );
	apply_64( &dbf, fb, 0x00000080, 0x40201008 );
	apply_64( &dbf, fb, 0x00008040, 0x20100804 );
	apply_64( &dbf, fb, 0x00804020, 0x10080402 );
	apply_64( &dbf, fb, 0x80402010, 0x08040201 );
	apply_64( &dbf, fb, 0x40201008, 0x04020100 );
	apply_64( &dbf, fb, 0x20100804, 0x02010000 );
	apply_64( &dbf, fb, 0x10080402, 0x01000000 );
	apply_64( &dbf, fb, 0x08040201, 0x00000000 );
	apply_64( &dbf, fb, 0x04020100, 0x00000000 );

	ss->high |= (lrf.high & udf.high & daf.high & dbf.high & dd->high);
	ss->low |= (lrf.low & udf.low & daf.low & dbf.low & dd->low);

	if ( (ss->high == 0) && (ss->low == 0) )
		return;

	do {
		ost = *ss;

		line_shift_64( &expand_ss, ss, 1, lrf );

		line_shift_64( &dir_ss, ss, 8, udf );
		expand_ss.high &= dir_ss.high;
		expand_ss.low &= dir_ss.low;

		line_shift_64( &dir_ss, ss, 7, daf );
		expand_ss.high &= dir_ss.high;
		expand_ss.low &= dir_ss.low;

		line_shift_64( &dir_ss, ss, 9, dbf );
		expand_ss.high &= dir_ss.high;
		expand_ss.low &= dir_ss.low;

		expand_ss.high &= dd->high;
		expand_ss.low &= dd->low;

		ss->high |= expand_ss.high;
		ss->low |= expand_ss.low;
	} while ( (ost.high != ss->high) || (ost.low != ss->low) );

	ss->high &= dd->high;
	ss->low &= dd->low;
}

/*
  COUNT_EDGE_STABLE
  Returns the number of stable edge discs for COLOR.
  Side effect: The edge indices are calculated. They are needed
  by COUNT_STABLE below.
*/

int
count_edge_stable( int color,
				  BitBoard col_bits,
				  BitBoard opp_bits ) {
	int col_mask, opp_mask;

	edge_a1h1 = 3280 * CHESS_EMPTY;
	if ( color == CHESS_BLACK ) {
		edge_a1h1 += (CHESS_BLACK - CHESS_EMPTY) * base_conversion[col_bits.low & 255];
		edge_a1h1 += (CHESS_WHITE - CHESS_EMPTY) * base_conversion[opp_bits.low & 255];
	}
	else {
		edge_a1h1 += (CHESS_WHITE - CHESS_EMPTY) * base_conversion[col_bits.low & 255];
		edge_a1h1 += (CHESS_BLACK - CHESS_EMPTY) * base_conversion[opp_bits.low & 255];
	}

	edge_a8h8 = 3280 * CHESS_EMPTY;
	if ( color == CHESS_BLACK ) {
		edge_a8h8 += (CHESS_BLACK - CHESS_EMPTY) * base_conversion[col_bits.high >> 24];
		edge_a8h8 += (CHESS_WHITE - CHESS_EMPTY) * base_conversion[opp_bits.high >> 24];
	}
	else {
		edge_a8h8 += (CHESS_WHITE - CHESS_EMPTY) * base_conversion[col_bits.high >> 24];
		edge_a8h8 += (CHESS_BLACK - CHESS_EMPTY) * base_conversion[opp_bits.high >> 24];
	}

	edge_a1a8 = 3280 * CHESS_EMPTY;
	col_mask = (col_bits.low & 1) + (((col_bits.low >> 8) & 1) << 1) +
		(((col_bits.low >> 16) & 1) << 2) + (((col_bits.low >> 24) & 1) << 3) +
		((col_bits.high & 1) << 4) + (((col_bits.high >> 8) & 1) << 5) +
		(((col_bits.high >> 16) & 1) << 6) + (((col_bits.high >> 24) & 1) << 7);
	opp_mask = (opp_bits.low & 1) + (((opp_bits.low >> 8) & 1) << 1) +
		(((opp_bits.low >> 16) & 1) << 2) + (((opp_bits.low >> 24) & 1) << 3) +
		((opp_bits.high & 1) << 4) + (((opp_bits.high >> 8) & 1) << 5) +
		(((opp_bits.high >> 16) & 1) << 6) + (((opp_bits.high >> 24) & 1) << 7);
	if ( color == CHESS_BLACK ) {
		edge_a1a8 += (CHESS_BLACK - CHESS_EMPTY) * base_conversion[col_mask];
		edge_a1a8 += (CHESS_WHITE - CHESS_EMPTY) * base_conversion[opp_mask];
	}
	else {
		edge_a1a8 += (CHESS_WHITE - CHESS_EMPTY) * base_conversion[col_mask];
		edge_a1a8 += (CHESS_BLACK - CHESS_EMPTY) * base_conversion[opp_mask];
	}

	edge_h1h8 = 3280 * CHESS_EMPTY;
	col_mask = ((col_bits.low >> 7) & 1) + (((col_bits.low >> 15) & 1) << 1) +
		(((col_bits.low >> 23) & 1) << 2) + (((col_bits.low >> 31) & 1) << 3) +
		(((col_bits.high >> 7) & 1) << 4) + (((col_bits.high >> 15) & 1) << 5) +
		(((col_bits.high >> 23) & 1) << 6) + (((col_bits.high >> 31) & 1) << 7);
	opp_mask = ((opp_bits.low >> 7) & 1) + (((opp_bits.low >> 15) & 1) << 1) +
		(((opp_bits.low >> 23) & 1) << 2) + (((opp_bits.low >> 31) & 1) << 3) +
		(((opp_bits.high >> 7) & 1) << 4) + (((opp_bits.high >> 15) & 1) << 5) +
		(((opp_bits.high >> 23) & 1) << 6) + (((opp_bits.high >> 31) & 1) << 7);
	if ( color == CHESS_BLACK ) {
		edge_h1h8 += (CHESS_BLACK - CHESS_EMPTY) * base_conversion[col_mask];
		edge_h1h8 += (CHESS_WHITE - CHESS_EMPTY) * base_conversion[opp_mask];
	}
	else {
		edge_h1h8 += (CHESS_WHITE - CHESS_EMPTY) * base_conversion[col_mask];
		edge_h1h8 += (CHESS_BLACK - CHESS_EMPTY) * base_conversion[opp_mask];
	}

	if ( color == CHESS_BLACK )
		return (black_stable[edge_a1h1] + black_stable[edge_a1a8] +
		black_stable[edge_a8h8] + black_stable[edge_h1h8]) / 2;
	else
		return (white_stable[edge_a1h1] + white_stable[edge_a1a8] +
		white_stable[edge_a8h8] + white_stable[edge_h1h8]) / 2;
}

/*
  COUNT_STABLE
  Returns the number of stable discs for COLOR.
  Side effect: last_black_stable or last_white_stable is modified.
  Note: COUNT_EDGE_STABLE must have been called immediately
        before this function is called *or you lose big*.
*/

int
count_stable( int color,
			 BitBoard col_bits,
			 BitBoard opp_bits ) {
	int i;
	int pop_count;
	unsigned int mask;
	BitBoard col_mask, opp_mask;
	BitBoard col_stable;
	BitBoard common_stable;

	/* Stable edge discs */

	common_stable.low = edge_stable[edge_a1h1];

	common_stable.high = (edge_stable[edge_a8h8] << 24);

	for ( i = 0, mask = 1; i < 4; i++, mask <<= 8 ) {
		if ( edge_stable[edge_a1a8] & (1 << i) )
			common_stable.low |= mask;
	}
	for ( i = 4, mask = 1; i < 8; i++, mask <<= 8 ) {
		if ( edge_stable[edge_a1a8] & (1 << i) )
			common_stable.high |= mask;
	}

	for ( i = 0, mask = 128; i < 4; i++, mask <<= 8 ) {
		if ( edge_stable[edge_h1h8] & (1 << i) )
			common_stable.low |= mask;
	}
	for ( i = 4, mask = 128; i < 8; i++, mask <<= 8 ) {
		if ( edge_stable[edge_h1h8] & (1 << i) )
			common_stable.high |= mask;
	}

	/* Expand the stable edge discs into a full set of stable discs */

	col_mask = col_bits;
	opp_mask = opp_bits;

	col_stable.high = col_mask.high & common_stable.high;
	col_stable.low = col_mask.low & common_stable.low;
	edge_zardoz_stable( &col_mask, &opp_mask, &col_stable );
	if ( color == CHESS_BLACK )
		last_black_stable = col_stable;
	else
		last_white_stable = col_stable;
	pop_count = iterative_popcount( col_stable );

	return pop_count;
}

/*
  STABILITY_SEARCH
  Searches the subtree rooted at the current position and tries to
  find variations in which the discs in CANDIDATE_BITS are
  flipped. Aborts if all those discs are stable in the subtree.
*/

static void
stability_search( BitBoard my_bits,
				 BitBoard opp_bits,
				 int color,
				 BitBoard *candidate_bits,
				 int max_depth,
				 int last_was_pass,
				 int *stability_nodes ) {
	int sq, old_sq;
	int mobility;
	BitBoard black_bits, white_bits;
	BitBoard new_my_bits, new_opp_bits;
	BitBoard all_stable_bits;

	(*stability_nodes)++;
	if ( *stability_nodes > MAX_STABILITY_NODES )
		return;

	if ( max_depth >= 3 ) {
		if ( color == CHESS_BLACK ) {
			black_bits = my_bits;
			white_bits = opp_bits;
		}
		else {
			black_bits = opp_bits;
			white_bits = my_bits;
		}
		BITBOARD_CLEAR( all_stable_bits );
		(void) count_edge_stable( CHESS_BLACK, black_bits, white_bits );
		if ( (candidate_bits->high & black_bits.high) ||
			(candidate_bits->low  & black_bits.low ) ) {
			(void) count_stable( CHESS_BLACK, black_bits, white_bits );
			BITBOARD_OR( all_stable_bits, last_black_stable );
		}
		if ( (candidate_bits->high & white_bits.high) ||
			(candidate_bits->low  & white_bits.low ) ) {
			(void) count_stable( CHESS_WHITE, white_bits, black_bits );
			BITBOARD_OR( all_stable_bits, last_white_stable );
		}
		if ( ((candidate_bits->high & ~all_stable_bits.high) == 0) &&
			((candidate_bits->low  & ~all_stable_bits.low ) == 0) )
			return;
	}

	mobility = 0;
	for ( old_sq = END_MOVE_LIST_HEAD, sq = stab_move_list[old_sq].succ;
		sq != END_MOVE_LIST_TAIL;
		old_sq = sq, sq = stab_move_list[sq].succ ) {
		if ( TestFlips_bitboard[sq]( my_bits, opp_bits ) ) {
			BITBOARD_ANDNOT( (*candidate_bits), bb_flips );
			if ( max_depth > 1 ) {
				BITBOARD_FULL_XOR( new_opp_bits, opp_bits, bb_flips );
				BITBOARD_FULL_XOR( new_my_bits, my_bits, bb_flips );
				BITBOARD_XOR( new_my_bits, square_mask[sq] );
				stab_move_list[old_sq].succ = stab_move_list[sq].succ;
				stability_search( new_opp_bits, new_my_bits, OPP_COLOR( color ),
					candidate_bits, max_depth - 1, FALSE,
					stability_nodes );
				stab_move_list[old_sq].succ = sq;
			}
			mobility++;
		}
	}

	if ( (mobility == 0) && !last_was_pass )
		stability_search( opp_bits, my_bits, OPP_COLOR( color ),
		      candidate_bits, max_depth, TRUE, stability_nodes );
}

/*
  COMPLETE_STABILITY_SEARCH
  Tries to compute all stable discs by search the entire game tree.
  The actual work is performed by STABILITY_SEARCH above.
*/

static void
complete_stability_search( int *in_board,
						  int color,
						  BitBoard *stable_bits ) {
	int i, j;
	int empties;
	int shallow_depth;
	int stability_nodes;
	int abort;
	BitBoard my_bits, opp_bits;
	BitBoard all_bits, candidate_bits;
	BitBoard test_bits;

	/* Prepare the move list */

	int last_sq = END_MOVE_LIST_HEAD;
	for ( i = 0; i < 60; i++ ) {
		int sq = position_list[i];
		if ( board[sq] == CHESS_EMPTY ) {
			stab_move_list[last_sq].succ = sq;
			stab_move_list[sq].pred = last_sq;
			last_sq = sq;
		}
	}
	stab_move_list[last_sq].succ = END_MOVE_LIST_TAIL;

	empties = 0;
	for ( i = 0; i < 8; i++ ) {
		for ( j = 0; j < 8; j++ ) {
			if ( board[8 * i + j] == CHESS_EMPTY )
				empties++;
		}
	}

	/* Prepare the bitmaps for the stability search */

	set_bitboards( in_board, color, &my_bits, &opp_bits );

	BITBOARD_FULL_OR( all_bits, my_bits, opp_bits );

	BITBOARD_FULL_ANDNOT( candidate_bits, all_bits, (*stable_bits) );

	/* Search all potentially stable discs for at most 4 plies
		to weed out those easily flippable */

	stability_nodes = 0;
	shallow_depth = 4;
	stability_search( my_bits, opp_bits, color, &candidate_bits,
		MIN( empties, shallow_depth ), FALSE, &stability_nodes );

	/* Scan through the rest of the discs one at a time until the
		maximum number of stability nodes is exceeded. Hopefully
		a subset of the stable discs is found also if this happens. */

	abort = FALSE;
	for ( i = 0; (i < 8) && !abort; i++ ) {
		for ( j = 0; (j < 8) && !abort; j++ ) {
			int sq = 8 * i + j;
			test_bits = square_mask[sq];
			if ( (test_bits.high & candidate_bits.high) ||
				(test_bits.low  & candidate_bits.low ) ) {
				stability_search( my_bits, opp_bits, color, &test_bits,
					empties, FALSE, &stability_nodes );
				abort = (stability_nodes > MAX_STABILITY_NODES);
				if ( !abort ) {
					if ( test_bits.high | test_bits.low ) {
						stable_bits->high |= test_bits.high;
						stable_bits->low  |= test_bits.low;
					}
				}
			}
		}
	}
}

/*
  GET_STABLE
  Determines what discs on BOARD are stable with SIDE_TO_MOVE to play next.
  The stability status of all squares (black, white and empty)
  is returned in the boolean vector IS_STABLE.
*/

void
get_stable( int *in_board,
		   int color,
		   int *is_stable ) {
	int i, j;
	unsigned int mask;
	BitBoard black_bits, white_bits, all_stable;

	set_bitboards( in_board, CHESS_BLACK, &black_bits, &white_bits );

	for ( i = 0; i < 64; i++ )
		is_stable[i] = FALSE;

	if ( ((black_bits.high == 0) && (black_bits.low == 0)) ||
		((white_bits.high == 0) && (white_bits.low == 0)) ) {
		for ( i = 0; i < 8; i++ ) {
			for ( j = 0; j < 8; j++ )
				is_stable[8 * i + j] = TRUE;
		}
	}
	else {  /* Nobody wiped out */
		(void) count_edge_stable( CHESS_BLACK, black_bits, white_bits );
		(void) count_stable( CHESS_BLACK, black_bits, white_bits );
		(void) count_stable( CHESS_WHITE, white_bits, black_bits );

		BITBOARD_FULL_OR( all_stable, last_black_stable, last_white_stable );

		complete_stability_search( in_board, color, &all_stable );

		for ( i = 0, mask = 1; i < 4; i++ ) {
			for ( j = 0; j < 8; j++, mask <<= 1 ) {
				if ( all_stable.low & mask )
					is_stable[8 * i + j] = TRUE;
			}
		}
		for ( i = 4, mask = 1; i < 8; i++ ) {
			for ( j = 0; j < 8; j++, mask <<= 1 ) {
				if ( all_stable.high & mask )
					is_stable[8 * i + j] = TRUE;
			}
		}
	}
}

#if DEBUG
/*
  DISPLAY_ROW
  Display an edge configuration and highlight the stable discs.
*/

static void
display_row( int pattern ) {
	int i;
	int mask = edge_stable[pattern];
	int temp = pattern;

	for ( i = 0; i < 8; i++ ) {
		switch ( temp % 3) {
		case CHESS_EMPTY:
			putchar( '-' );
			break;
		case CHESS_BLACK:
			if ( mask & (1 << i) )
				putchar( 'X' );
			else
				putchar( 'x' );
			break;
		case CHESS_WHITE:
			if ( mask & (1 << i) )
				putchar( 'O' );
			else
				putchar( 'o' );
		}
		temp /= 3;
	}
#ifdef TEXT_BASED
	printf( "     pattern %4d   black %2d   white %2d\n", pattern,
		black_stable[pattern], white_stable[pattern] );
#endif
}
#endif

/*
  RECURSIVE_FIND_STABLE
  Returns a bit mask describing the set of stable discs in the
  edge PATTERN. When a bit mask is calculated, it's stored in
  a table so that any particular bit mask only is generated once.
*/

static int
recursive_find_stable( int pattern ) {
	int i, j;
	int new_pattern;
	int stable;
	int temp;
	int row[8], stored_row[8];

	if ( edge_stable[pattern] != UNDETERMINED )
		return edge_stable[pattern];

	temp = pattern;
	for ( i = 0; i < 8; i++, temp /= 3 )
		row[i] = temp % 3;

	/* All positions stable unless proved otherwise. */

	stable = 255;

	/* Play out the 8 different moves and AND together the stability masks. */

	for ( j = 0; j < 8; j++ )
		stored_row[j] = row[j];

	for ( i = 0; i < 8; i++ ) {

		/* Make sure we work with the original configuration */

		for ( j = 0; j < 8; j++ )
			row[j] = stored_row[j];

		if ( row[i] == CHESS_EMPTY ) {  /* Empty ==> playable! */

			/* Mark the empty square as unstable and store position */

			stable &= ~(1 << i);

			/* Play out a black move */

			row[i] = CHESS_BLACK;
			if ( i >= 2 ) {
				j = i - 1;
				while ( (j >= 1) && (row[j] == CHESS_WHITE) )
					j--;
				if ( row[j] == CHESS_BLACK ) {
					for ( j++; j < i; j++ ) {
						row[j] = CHESS_BLACK;
						stable &= ~(1 << j);
					}
				}
			}
			if ( i <= 5 ) {
				j = i + 1;
				while ( (j <= 6) && (row[j] == CHESS_WHITE) )
					j++;
				if ( row[j] == CHESS_BLACK ) {
					for ( j--; j > i; j-- ) {
						row[j] = CHESS_BLACK;
						stable &= ~(1 << j);
					}
				}
			}
			new_pattern = 0;
			for ( j = 0; j < 8; j++ )
				new_pattern += pow3[j] * row[j];
			stable &= recursive_find_stable( new_pattern );

			/* Restore position */

			for ( j = 0; j < 8; j++ )
				row[j] = stored_row[j];

			/* Play out a white move */

			row[i] = CHESS_WHITE;
			if ( i >= 2 ) {
				j = i - 1;
				while ( (j >= 1) && (row[j] == CHESS_BLACK) )
					j--;
				if ( row[j] == CHESS_WHITE ) {
					for ( j++; j < i; j++ ) {
						row[j] = CHESS_WHITE;
						stable &= ~(1 << j);
					}
				}
			}
			if ( i <= 5 ) {
				j = i + 1;
				while ( (j <= 6) && (row[j] == CHESS_BLACK) )
					j++;
				if ( row[j] == CHESS_WHITE ) {
					for ( j--; j > i; j-- ) {
						row[j] = CHESS_WHITE;
						stable &= ~(1 << j);
					}
				}
			}
			new_pattern = 0;
			for ( j = 0; j < 8; j++ )
				new_pattern += pow3[j] * row[j];
			stable &= recursive_find_stable( new_pattern );
		}
	}

	/* Store and return */

	edge_stable[pattern] = stable;

	return stable;
}

/*
  COUNT_COLOR_STABLE
  Determines the number of stable discs for each of the edge configurations
  for the two colors. This is done using the following convention:
  - a stable corner disc gives stability of 1
  - a stable non-corner disc gives stability of 2
  This way the stability values for the four edges can be added together
  without any risk for double-counting.
*/

static void
count_color_stable( void ) {
	int i, j;
	int pattern;
	int row[8];

	for ( i = 0; i < 8; i++ )
		row[i] = 0;

	for ( pattern = 0; pattern < 6561; pattern++ ) {
		black_stable[pattern] = 0;
		white_stable[pattern] = 0;
		for ( j = 0; j < 8; j++ ) {
			if ( edge_stable[pattern] & (1 << j) ) {
				if ( row[j] == CHESS_BLACK ) {
					if ( (j == 0) || (j == 7) )
						black_stable[pattern]++;
					else
						black_stable[pattern] += 2;
				}
				else if ( row[j] == CHESS_WHITE ) {
					if ( (j == 0) || (j == 7) )
						white_stable[pattern]++;
					else
						white_stable[pattern] += 2;
				}
			}
		}

		/* Next configuration */
		i = 0;
		do {  /* The odometer principle */
			row[i]++;
			if (row[i] == 3)
				row[i] = 0;
			i++;
		} while ( (row[i - 1] == 0) && (i < 8) );
	}
}

/*
  INIT_STABLE
  Build the table containing the stability masks for all edge
  configurations. This is done using dynamic programming.
*/

void
init_stable( void ) {
	int i, j;

	for ( i = 0; i < 256; i++ ) {
		base_conversion[i] = 0;
		for ( j = 0; j < 8; j++ )
			if ( i & (1 << j) )
				base_conversion[i] += pow3[j];
	}

	for ( i = 0; i < 6561; i++ )
		edge_stable[i] = UNDETERMINED;
	for ( i = 0; i < 6561; i++ ) {
		if ( edge_stable[i] == UNDETERMINED )
			(void) recursive_find_stable( i );
	}
	count_color_stable();

#if DEBUG
	for ( i = 0; i < 6561; i++ )
		display_row( i );
	exit( 1 );
#endif
}
