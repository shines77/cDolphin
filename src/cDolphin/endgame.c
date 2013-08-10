/*
   File:           endgame.c

   Created:        2007-07-10

   Modified:       2013-08-10

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The module which controls the operation of standalone Dolphin.
*/

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "global.h"
#include "colour.h"
#include "board.h"
#include "bitboard.h"
#include "timer.h"
#include "myrandom.h"
#include "endgame.h"
#include "midgame.h"
#include "stable.h"
#include "hash.h"
#include "search.h"
#include "pv.h"
#include "move.h"
#include "eval.h"
#include "epcstat.h"
#include "probcut.h"
#include "display.h"
#include "bitbmob.h"
#include "bitbtest.h"
#include "bitbvald.h"
#include "bitbcnt.h"
#include "bitbcnt2.h"

#define USE_MPC                      1
#define MAX_SELECTIVITY              9
#define DISABLE_SELECTIVITY          18

#define PV_EXPANSION                 16

#define DEPTH_TWO_SEARCH             15
#define DEPTH_THREE_SEARCH           20
#define DEPTH_FOUR_SEARCH            24
#define DEPTH_SIX_SEARCH             30
#define EXTRA_ROOT_SEARCH            2

#ifdef _WIN32_WCE
#define EVENT_CHECK_INTERVAL         25000.0
#else
#define EVENT_CHECK_INTERVAL         250000.0
#endif

#define LOW_LEVEL_DEPTH              8
#define FASTEST_FIRST_DEPTH          12
#define HASH_DEPTH                   (LOW_LEVEL_DEPTH + 1)

#define VERY_HIGH_EVAL               1000000

#define GOOD_TRANSPOSITION_EVAL      10000000

/* Parameters for the fastest-first algorithm. The performance does
   not seem to depend a lot on the precise values. */
#define FAST_FIRST_FACTOR            0.45
#define MOB_FACTOR                   460

/* The disc difference when special wipeout move ordering is tried.
   This means more aggressive use of fastest first. */
#define WIPEOUT_THRESHOLD            60

/* Use stability pruning? */
#define USE_STABILITY                TRUE

typedef enum eSearchStatus {
	NOTHING,
	SELECTIVE_SCORE,
	WLD_SCORE,
	EXACT_SCORE
} SearchStatus;

ALIGN_PREFIX(64)
MoveLink end_move_list[64] ALIGN_SUFFIX(64);

/* The parities of the regions are in the region_parity bit vector. */

static unsigned int region_parity;

/* Pseudo-probabilities corresponding to the percentiles.
   These are taken from the normal distribution; to the percentile
   x corresponds the probability Pr(-x <= Y <= x) where Y is a N(0,1)
   variable. */

static double confidence[MAX_SELECTIVITY + 1] =
{ 1.000, 0.99, 0.98, 0.954, 0.911, 0.838, 0.729, 0.576, 0.383, 0.197 };

/* Percentiles used in the endgame MPC */
static double end_percentile[MAX_SELECTIVITY + 1] =
{ 100.0, 4.0, 3.0, 2.0, 1.7, 1.4, 1.1, 0.8, 0.5, 0.25 };

#if USE_STABILITY
#define  HIGH_STABILITY_THRESHOLD     24
static int stability_threshold[] = {
	65, 65, 65, 65, 65, 46, 38, 30, 24,
	24, 24, 24, 0, 0, 0, 0, 0, 0, 0 };
#endif

static double fast_first_mean[61][64];
static double fast_first_sigma[61][64];
static int best_move = _NULL_MOVE, best_end_root_move = _NULL_MOVE;
static int true_found, true_val;
static int full_output_mode;
static int earliest_wld_solve, earliest_full_solve;
static int fast_first_threshold[61][64];
static int ff_mob_factor[61];

ALIGN_PREFIX(64)
unsigned int quadrant_mask[64] ALIGN_SUFFIX(64) = {
	1, 1, 1, 1, 2, 2, 2, 2,
	1, 1, 1, 1, 2, 2, 2, 2,
	1, 1, 1, 1, 2, 2, 2, 2,
	1, 1, 1, 1, 2, 2, 2, 2,
	4, 4, 4, 4, 8, 8, 8, 8,
	4, 4, 4, 4, 8, 8, 8, 8,
	4, 4, 4, 4, 8, 8, 8, 8,
	4, 4, 4, 4, 8, 8, 8, 8,
};

/*
   SETUP_END
   Prepares the endgame solver for a new game.
   This means clearing a few status fields.
*/

void
setup_end( void ) {
	double last_mean, last_sigma;
	double ff_threshold[61];
	double prelim_threshold[61][64];
	int i, j;
	//int dir_shift[8] = {1, -1, 7, -7, 8, -8, 9, -9};

	earliest_wld_solve = 0;
	earliest_full_solve = 0;
	full_output_mode = TRUE;  // TRUE

	/* Set the fastest-first mobility encouragements and thresholds */

	for ( i = 0; i <= 60; i++ )
		ff_mob_factor[i] = MOB_FACTOR;
	for ( i = 0; i <= 60; i++ )
		ff_threshold[i] = FAST_FIRST_FACTOR;

	/* Calculate the alpha thresholds for using fastest-first for
		each #empty and shallow search depth. */

	for ( j = 0; j <= MAX_END_CORR_DEPTH; j++ ) {
		last_sigma = 100.0;  /* Infinity in disc difference */
		last_mean = 0.0;
		for ( i = 60; i >= 0; i-- ) {
			if ( end_stats_available[i][j] ) {
				last_mean = end_mean[i][j];
				last_sigma = ff_threshold[i] * end_sigma[i][j];
			}
			fast_first_mean[i][j] = last_mean;
			fast_first_sigma[i][j] = last_sigma;
			prelim_threshold[i][j] = last_mean + last_sigma;
		}
	}
	for ( j = MAX_END_CORR_DEPTH + 1; j < 64; j++ ) {
		for ( i = 0; i <= 60; i++ )
			prelim_threshold[i][j] = prelim_threshold[i][MAX_END_CORR_DEPTH];
	}
	for ( i = 0; i <= 60; i++ ) {
		for ( j = 0; j < 64; j++ )
			fast_first_threshold[i][j] =
				(int) ceil( prelim_threshold[i][j] * 128.0 );
	}
}

/*
  GET_EARLIEST_WLD_SOLVE
  GET_EARLIEST_FULL_SOLVE
  Return the highest #empty when WLD and full solve respectively
  were completed (not initiated).
*/

int
get_earliest_wld_solve( void ) {
	return earliest_wld_solve;
}

int
get_earliest_full_solve( void ) {
	return earliest_full_solve;
}

/*
  SET_OUTPUT_MODE
  Toggles output of intermediate search status on/off.
*/

void
set_output_mode( int full ) {
	full_output_mode = full;
}

/*
  PREPARE_TO_SOLVE
  Create the list of empty squares.
*/

static void
prepare_to_solve( BitBoard my_bits, BitBoard opp_bits ) {
	/* fixed square ordering: */
	/* jcw's order, which is the best of 4 tried (according to Warren Smith) */
	static int worst2best[64] = {
		/*B2*/        B2 , G2 , B7 , G7 ,
		/*B1*/        B1 , G1 , A2 , H2 , A7 , H7 , B8 , G8 ,
		/*C2*/        C2 , F2 , B3 , G3 , B6 , G6 , C7 , F7 ,
		/*D2*/        D2 , E2 , B4 , G4 , B5 , G5 , D7 , E7 ,
		/*D3*/        D3 , E5 , C4 , F4 , C5 , F5 , D6 , E6 ,
		/*C1*/        C1 , F1 , A3 , H3 , A6 , H6 , C8 , F8 ,
		/*D1*/        D1 , E1 , A4 , H4 , A5 , H5 , D8 , E8 ,
		/*C3*/        C3 , F3 , C6 , F6 ,
		/*A1*/        A1 , H8 , A8 , H1 ,
		/*D4*/        D4 , E4 , D5 , E5
	};
	int i;
	int last_sq;

	region_parity = 0;

	last_sq = END_MOVE_LIST_HEAD;
	for ( i = 59; i >= 0; i-- ) {
		int sq = worst2best[i];
		if ( (((my_bits.low & square_mask[sq].low) == 0) &&
			((my_bits.high & square_mask[sq].high) == 0) &&
			((opp_bits.low & square_mask[sq].low) == 0) &&
			((opp_bits.high & square_mask[sq].high) == 0)) ) {
			end_move_list[last_sq].succ = sq;
			end_move_list[sq].pred = last_sq;
			region_parity ^= quadrant_mask[sq];
			last_sq = sq;
		}
	}
	end_move_list[last_sq].succ = END_MOVE_LIST_TAIL;
}

#if 0
/*
  PREPARE_TO_SOLVE2
  Create the list of empty squares.
*/

static void
prepare_to_solve2( const int *in_board ) {
	/* fixed square ordering: */
	/* jcw's order, which is the best of 4 tried (according to Warren Smith) */
	static int worst2best[64] = {
		/*B2*/        B2 , G2 , B7 , G7 ,
		/*B1*/        B1 , G1 , A2 , H2 , A7 , H7 , B8 , G8 ,
		/*C2*/        C2 , F2 , B3 , G3 , B6 , G6 , C7 , F7 ,
		/*D2*/        D2 , E2 , B4 , G4 , B5 , G5 , D7 , E7 ,
		/*D3*/        D3 , E5 , C4 , F4 , C5 , F5 , D6 , E6 ,
		/*C1*/        C1 , F1 , A3 , H3 , A6 , H6 , C8 , F8 ,
		/*D1*/        D1 , E1 , A4 , H4 , A5 , H5 , D8 , E8 ,
		/*C3*/        C3 , F3 , C6 , F6 ,
		/*A1*/        A1 , H8 , A8 , H1 ,
		/*D4*/        D4 , E4 , D5 , E5
	};
	int i;
	int last_sq;

	region_parity = 0;

	last_sq = END_MOVE_LIST_HEAD;
	for ( i = 59; i >= 0; i-- ) {
		int sq = worst2best[i];
		if ( in_board[sq] == CHESS_EMPTY ) {
			end_move_list[last_sq].succ = sq;
			end_move_list[sq].pred = last_sq;
			region_parity ^= quadrant_mask[sq];
			last_sq = sq;
		}
	}
	end_move_list[last_sq].succ = END_MOVE_LIST_TAIL;
}

#endif

/*
  UPDATE_BEST_LIST
*/

static void
update_best_list( int *best_list, int move, int best_list_index,
				 int *best_list_length, int verbose ) {
	int i;

	//verbose = FALSE;

	/*
	if ( verbose ) {
		printf( "move=%2d  index=%d  length=%d      ", move, best_list_index,
			*best_list_length );
		printf( "Before:  " );
		for ( i = 0; i < 4; i++ )
			printf( "%2d ", best_list[i] );
	}
	//*/

	if ( best_list_index < *best_list_length ) {
		for ( i = best_list_index; i >= 1; i-- )
			best_list[i] = best_list[i - 1];
	}
	else {
		for ( i = 3; i >= 1; i-- )
			best_list[i] = best_list[i - 1];
		if ( *best_list_length < 4 )
			(*best_list_length)++;
	}
	best_list[0] = move;

	/*
	if ( verbose ) {
		printf( "      After:  " );
		for ( i = 0; i < 4; i++ )
			printf( "%2d ", best_list[i] );
		puts( "" );
	}
	//*/
}

/*
  SEND_SOLVE_STATUS
  Displays endgame results - partial or full.
*/

static void
send_solve_status( int empties,
				  int color,
				  EvaluationType *eval_info ) {
	char *eval_str;
	double node_val;

	set_current_eval( *eval_info );
	clear_status();
	send_status( "-->  %2d  ", empties );
	eval_str = produce_eval_text( *eval_info, TRUE );
	send_status( "%-10s  ", eval_str );
	free( eval_str );
	node_val = counter_value( &tree_nodes );
	send_status_nodes( node_val );
	if ( get_ponder_move() != _NULL_MOVE )
		send_status( "{%c%c} ", TO_SQUARE( get_ponder_move() ) );
	send_status_pv( pv[0], empties );
	send_status_time( get_elapsed_time() );
	if ( get_elapsed_time() > 0.0001 )
		send_status( "%6.0f %s  ", node_val / (get_elapsed_time() + 0.0001),
		"nps" );
}

/*
  SOLVE_TWO_EMPTY
  SOLVE_THREE_EMPTY
  SOLVE_FOUR_EMPTY
  SOLVE_PARITY
  SOLVE_PARITY_HASH
  SOLVE_PARITY_HASH_HIGH
  These are the core routines of the low level endgame code.
  They all perform the same task: Return the score for the side to move.
  Structural differences:
  * SOLVE_TWO_EMPTY may only be called for *exactly* two empty
  * SOLVE_THREE_EMPTY may only be called for *exactly* three empty
  * SOLVE_FOUR_EMPTY may only be called for *exactly* four empty
  * SOLVE_PARITY uses stability, parity and fixed move ordering
  * SOLVE_PARITY_HASH uses stability, hash table and fixed move ordering
  * SOLVE_PARITY_HASH_HIGH uses stability, hash table and (non-thresholded)
    fastest first
*/

#define COUNTFLIPS2

static int
solve_two_empty( BitBoard my_bits,
				BitBoard opp_bits,
				int sq1,
				int sq2,
				int alpha,
				int beta,
				int disc_diff,
				int pass_legal ) {
	BitBoard new_opp_bits;
	int score = -INFINITE_EVAL;
	int flipped;
	int ev;

	INCREMENT_COUNTER( tree_nodes );

	/* Overall strategy: Lazy evaluation whenever possible, i.e., don't
		update bitboards until they are used. Also look at alpha and beta
		in order to perform strength reduction: Feasibility testing is
		faster than counting number of flips. */

	/* Try the first of the two empty squares... */

	flipped = TestFlips_wrapper( sq1, my_bits, opp_bits );
	if ( flipped != 0 ) {  /* SQ1 feasible for me */
		BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

		INCREMENT_COUNTER( tree_nodes );

		ev = disc_diff + 2 * flipped;

#if 0
		if ( ev - 2 <= alpha ) { /* Fail-low if he can play SQ2 */
			if ( ValidOneEmpty_bitboard[sq2]( new_opp_bits ) != 0 )
				ev = alpha;
			else {  /* He passes, check if SQ2 is feasible for me */
				if ( ev >= 0 ) {  /* I'm ahead, so EV will increase by at least 2 */
					ev += 2;
					if ( ev < beta )  /* Only bother if not certain fail-high */
#ifdef COUNTFLIPS1
						ev += 2 * CountFlips_bitboard[sq2]( bb_flips );
#else
						ev += 2 * CountFlips_bitboard2[sq2]( bb_flips.high, bb_flips.low );
#endif
				}
				else {
					if ( ev < beta ) {  /* Only bother if not fail-high already */
#ifdef COUNTFLIPS1
						flipped = CountFlips_bitboard[sq2]( bb_flips );
#else
						flipped = CountFlips_bitboard2[sq2]( bb_flips.high, bb_flips.low );
#endif
						if ( flipped != 0 )  /* SQ2 feasible for me, game over */
							ev += 2 * (flipped + 1);
						/* ELSE: SQ2 will end up empty, game over */
					}
				}
			}
		} else
#endif
		{
#ifdef COUNTFLIPS1
			flipped = CountFlips_bitboard[sq2]( new_opp_bits );
#else
			flipped = CountFlips_bitboard2[sq2]( new_opp_bits.high, new_opp_bits.low );
#endif
			if ( flipped != 0 )
				ev -= 2 * flipped;
			else {  /* He passes, check if SQ2 is feasible for me */
				if ( ev >= 0 ) {  /* I'm ahead, so EV will increase by at least 2 */
					ev += 2;
					if ( ev < beta )  /* Only bother if not certain fail-high */
#ifdef COUNTFLIPS1
						ev += 2 * CountFlips_bitboard[sq2]( bb_flips );
#else
						ev += 2 * CountFlips_bitboard2[sq2]( bb_flips.high, bb_flips.low );
#endif
				}
				else {
					if ( ev < beta ) {  /* Only bother if not fail-high already */
#ifdef COUNTFLIPS1
						flipped = CountFlips_bitboard[sq2]( bb_flips );
#else
						flipped = CountFlips_bitboard2[sq2]( bb_flips.high, bb_flips.low );
#endif
						if ( flipped != 0 )  /* SQ2 feasible for me, game over */
							ev += 2 * (flipped + 1);
						/* ELSE: SQ2 will end up empty, game over */
					}
				}
			}
		}

		/* Being legal, the first move is the best so far */
		score = ev;
		if ( score > alpha ) {
			if ( score >= beta )
				return score;
			alpha = score;
		}
	}

	/* ...and then the second */

	flipped = TestFlips_wrapper( sq2, my_bits, opp_bits );
	if ( flipped != 0 ) {  /* SQ2 feasible for me */
		BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

		INCREMENT_COUNTER( tree_nodes );

		ev = disc_diff + 2 * flipped;
#if 0
		if ( ev - 2 <= alpha ) {  /* Fail-low if he can play SQ1 */
			if ( ValidOneEmpty_bitboard[sq1]( new_opp_bits ) != 0 )
				ev = alpha;
			else {  /* He passes, check if SQ1 is feasible for me */
				if ( ev >= 0 ) {  /* I'm ahead, so EV will increase by at least 2 */
					ev += 2;
					if ( ev < beta )  /* Only bother if not certain fail-high */
#ifdef COUNTFLIPS1
						ev += 2 * CountFlips_bitboard[sq1]( bb_flips );
#else
						ev += 2 * CountFlips_bitboard2[sq1]( bb_flips.high, bb_flips.low );
#endif
				}
				else {
					if ( ev < beta ) {  /* Only bother if not fail-high already */
#ifdef COUNTFLIPS1
						flipped = CountFlips_bitboard[sq1]( bb_flips );
#else
						flipped = CountFlips_bitboard2[sq1]( bb_flips.high, bb_flips.low );
#endif
						if ( flipped != 0 )  /* SQ1 feasible for me, game over */
							ev += 2 * (flipped + 1);
						/* ELSE: SQ1 will end up empty, game over */
					}
				}
			}
		} else 
#endif
		{
#ifdef COUNTFLIPS1
			flipped = CountFlips_bitboard[sq1]( new_opp_bits );
#else
			flipped = CountFlips_bitboard2[sq1]( new_opp_bits.high, new_opp_bits.low );
#endif
			if ( flipped != 0 )  /* SQ1 feasible for him, game over */
				ev -= 2 * flipped;
			else {  /* He passes, check if SQ1 is feasible for me */
				if ( ev >= 0 ) {  /* I'm ahead, so EV will increase by at least 2 */
					ev += 2;
					if ( ev < beta )  /* Only bother if not certain fail-high */
#ifdef COUNTFLIPS1
						ev += 2 * CountFlips_bitboard[sq1]( bb_flips );
#else
						ev += 2 * CountFlips_bitboard2[sq1]( bb_flips.high, bb_flips.low );
#endif
				}
				else {
					if ( ev < beta ) {  /* Only bother if not fail-high already */
#ifdef COUNTFLIPS1
						flipped = CountFlips_bitboard[sq1]( bb_flips );
#else
						flipped = CountFlips_bitboard2[sq1]( bb_flips.high, bb_flips.low );
#endif
						if ( flipped != 0 )  /* SQ1 feasible for me, game over */
							ev += 2 * (flipped + 1);
						/* ELSE: SQ1 will end up empty, game over */
					}
				}
			}
		}

		/* If the second move if better than the first (if that move was legal),
			its score is the score of the position */
		if ( ev >= score )
			return ev;
	}

	/* If both SQ1 and SQ2 are illegal I have to pass,
		otherwise return the best score. */

	if ( score == -INFINITE_EVAL ) {
		if ( !pass_legal ) {  /* Two empty squares */
			if ( disc_diff > 0 )
				return disc_diff + 2;
			if ( disc_diff < 0 )
				return disc_diff - 2;
			return 0;
		}
		else
			return -solve_two_empty( opp_bits, my_bits, sq1, sq2, -beta,
								-alpha, -disc_diff, FALSE );
	}
	else
		return score;
}

static int
solve_three_empty( BitBoard my_bits,
				  BitBoard opp_bits,
				  int sq1,
				  int sq2,
				  int sq3,
				  int alpha,
				  int beta,
				  int disc_diff,
				  int pass_legal ) {
	BitBoard new_opp_bits;
	int score = -INFINITE_EVAL;
	int flipped;
	int new_disc_diff;
	int ev;

	INCREMENT_COUNTER( tree_nodes );

	flipped = TestFlips_wrapper( sq1, my_bits, opp_bits );
	if ( flipped != 0 ) {
		BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

		new_disc_diff = -disc_diff - 2 * flipped - 1;
		score = -solve_two_empty( new_opp_bits, bb_flips, sq2, sq3,
			-beta, -alpha, new_disc_diff, TRUE );
		if ( score >= beta )
			return score;
		else if ( score > alpha )
			alpha = score;
	}

	flipped = TestFlips_wrapper( sq2, my_bits, opp_bits );
	if ( flipped != 0 ) {
		BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

		new_disc_diff = -disc_diff - 2 * flipped - 1;
		ev = -solve_two_empty( new_opp_bits, bb_flips, sq1, sq3,
			-beta, -alpha, new_disc_diff, TRUE );
		if ( ev >= beta )
			return ev;
		else if ( ev > score ) {
			score = ev;
			if ( score > alpha )
				alpha = score;
		}
	}

	flipped = TestFlips_wrapper( sq3, my_bits, opp_bits );
	if ( flipped != 0 ) {
		BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

		new_disc_diff = -disc_diff - 2 * flipped - 1;
		ev = -solve_two_empty( new_opp_bits, bb_flips, sq1, sq2,
				-beta, -alpha, new_disc_diff, TRUE );
		if ( ev >= score )
			return ev;
	}

	if ( score == -INFINITE_EVAL ) {
		if ( !pass_legal ) {  /* Three empty squares */
			if ( disc_diff > 0 )
				return disc_diff + 3;
			if ( disc_diff < 0 )
				return disc_diff - 3;
			return 0;  /* Can't reach this code, only keep it for symmetry */
		}
		else
			return -solve_three_empty( opp_bits, my_bits, sq1, sq2, sq3,
					-beta, -alpha, -disc_diff, FALSE );
	}

	return score;
}

static int
solve_four_empty( BitBoard my_bits,
				 BitBoard opp_bits,
				 int sq1,
				 int sq2,
				 int sq3,
				 int sq4,
				 int alpha,
				 int beta,
				 int disc_diff,
				 int pass_legal ) {
	BitBoard new_opp_bits;
	int score = -INFINITE_EVAL;
	int flipped;
	int new_disc_diff;
	int ev;

	INCREMENT_COUNTER( tree_nodes );

	flipped = TestFlips_wrapper( sq1, my_bits, opp_bits );
	if ( flipped != 0 ) {
		BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

		new_disc_diff = -disc_diff - 2 * flipped - 1;
		score = -solve_three_empty( new_opp_bits, bb_flips, sq2, sq3, sq4,
					-beta, -alpha, new_disc_diff, TRUE );
		if ( score >= beta )
			return score;
		else if ( score > alpha )
			alpha = score;
	}

	flipped = TestFlips_wrapper( sq2, my_bits, opp_bits );
	if ( flipped != 0 ) {
		BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

		new_disc_diff = -disc_diff - 2 * flipped - 1;
		ev = -solve_three_empty( new_opp_bits, bb_flips, sq1, sq3, sq4,
				-beta, -alpha, new_disc_diff, TRUE );
		if ( ev >= beta )
			return ev;
		else if ( ev > score ) {
			score = ev;
			if ( score > alpha )
				alpha = score;
		}
	}

	flipped = TestFlips_wrapper( sq3, my_bits, opp_bits );
	if ( flipped != 0 ) {
		BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

		new_disc_diff = -disc_diff - 2 * flipped - 1;
		ev = -solve_three_empty( new_opp_bits, bb_flips, sq1, sq2, sq4,
				-beta, -alpha, new_disc_diff, TRUE );
		if ( ev >= beta )
			return ev;
		else if ( ev > score ) {
			score = ev;
			if ( score > alpha )
				alpha = score;
		}
	}

	flipped = TestFlips_wrapper( sq4, my_bits, opp_bits );
	if ( flipped != 0 ) {
		BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

		new_disc_diff = -disc_diff - 2 * flipped - 1;
		ev = -solve_three_empty( new_opp_bits, bb_flips, sq1, sq2, sq3,
				-beta, -alpha, new_disc_diff, TRUE );
		if ( ev >= score )
			return ev;
	}

	if ( score == -INFINITE_EVAL ) {
		if ( !pass_legal ) {  /* Four empty squares */
			if ( disc_diff > 0 )
				return disc_diff + 4;
			if ( disc_diff < 0 )
				return disc_diff - 4;
			return 0;
		}
		else
			return -solve_four_empty( opp_bits, my_bits, sq1, sq2, sq3, sq4,
						-beta, -alpha, -disc_diff, FALSE );
	}

	return score;
}

static int
solve_parity( BitBoard my_bits,
			 BitBoard opp_bits,
			 int alpha,
			 int beta,
			 int color,
			 int empties,
			 int disc_diff,
			 int pass_legal ) {
	BitBoard new_opp_bits;
	int score = -INFINITE_EVAL;
	int opp_col = OPP_COLOR( color );
	int ev;
	int flipped;
	int new_disc_diff;
	int sq, old_sq, best_sq = _NULL_MOVE;
	unsigned int parity_mask;

	INCREMENT_COUNTER( tree_nodes );

	/* Check for stability cutoff */

#if USE_STABILITY
	if ( alpha >= stability_threshold[empties] ) {
		int stability_bound;
		stability_bound = 64 - 2 * count_edge_stable( opp_col, opp_bits, my_bits );
		if ( stability_bound <= alpha )
			return alpha;
		stability_bound = 64 - 2 * count_stable( opp_col, opp_bits, my_bits );
		if ( stability_bound < beta )
			beta = stability_bound + 1;
		if ( stability_bound <= alpha )
			return alpha;
	}
#endif

	/* Odd parity */

	parity_mask = region_parity;

	if ( region_parity != 0 ) { /* Is there any region with odd parity? */
		for ( old_sq = END_MOVE_LIST_HEAD, sq = end_move_list[old_sq].succ;
			sq != END_MOVE_LIST_TAIL;
			old_sq = sq, sq = end_move_list[sq].succ ) {
			unsigned int holepar = quadrant_mask[sq];
			if ( holepar & parity_mask ) {
				flipped = TestFlips_wrapper( sq, my_bits, opp_bits );
				if ( flipped != 0 ) {
					BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

					end_move_list[old_sq].succ = end_move_list[sq].succ;
					new_disc_diff = -disc_diff - 2 * flipped - 1;
					if ( empties == 5 ) {
						int sq1 = end_move_list[END_MOVE_LIST_HEAD].succ;
						int sq2 = end_move_list[sq1].succ;
						int sq3 = end_move_list[sq2].succ;
						int sq4 = end_move_list[sq3].succ;
						ev = -solve_four_empty( new_opp_bits, bb_flips, sq1, sq2, sq3, sq4,
								-beta, -alpha, new_disc_diff, TRUE );
					}
					else {
						region_parity ^= holepar;
						ev = -solve_parity( new_opp_bits, bb_flips, -beta, -alpha,
								opp_col, empties - 1, new_disc_diff, TRUE );
						region_parity ^= holepar;
					}
					end_move_list[old_sq].succ = sq;

					if ( ev > score ) {
						if ( ev > alpha ) {
							if ( ev >= beta ) {
								best_move = sq;
								return ev;
							}
							alpha = ev;
						}
						score = ev;
						best_sq = sq;
					}
				}
			}
		}
	}

	/* Even parity */

	parity_mask = ~parity_mask;
	for ( old_sq = END_MOVE_LIST_HEAD, sq = end_move_list[old_sq].succ;
		sq != END_MOVE_LIST_TAIL;
		old_sq = sq, sq = end_move_list[sq].succ ) {
		unsigned int holepar = quadrant_mask[sq];
		if ( holepar & parity_mask ) {
			flipped = TestFlips_wrapper( sq, my_bits, opp_bits );
			if ( flipped != 0 ) {
				BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

				end_move_list[old_sq].succ = end_move_list[sq].succ;
				new_disc_diff = -disc_diff - 2 * flipped - 1;
				if ( empties == 5 ) {
					int sq1 = end_move_list[END_MOVE_LIST_HEAD].succ;
					int sq2 = end_move_list[sq1].succ;
					int sq3 = end_move_list[sq2].succ;
					int sq4 = end_move_list[sq3].succ;
					ev = -solve_four_empty( new_opp_bits, bb_flips, sq1, sq2, sq3, sq4,
							-beta, -alpha, new_disc_diff, TRUE );
				}
				else {
					region_parity ^= holepar;
					ev = -solve_parity( new_opp_bits, bb_flips, -beta, -alpha,
							opp_col, empties - 1, new_disc_diff, TRUE );
					region_parity ^= holepar;
				}
				end_move_list[old_sq].succ = sq;

				if ( ev > score ) {
					if ( ev > alpha ) {
						if ( ev >= beta ) {
							best_move = sq;
							return ev;
						}
						alpha = ev;
					}
					score = ev;
					best_sq = sq;
				}
			}
		}
	}

	if ( score == -INFINITE_EVAL ) {
		if ( !pass_legal ) {
			if ( disc_diff > 0 )
				return disc_diff + empties;
			if ( disc_diff < 0 )
				return disc_diff - empties;
			return 0;
		}
		else
			return -solve_parity( opp_bits, my_bits, -beta, -alpha, opp_col,
								empties, -disc_diff, FALSE );
	}
	best_move = best_sq;

	return score;
}

static int
solve_parity_hash( BitBoard my_bits,
				  BitBoard opp_bits,
				  int alpha,
				  int beta,
				  int color,
				  int empties,
				  int disc_diff,
				  int pass_legal ) {
	BitBoard new_opp_bits;
	int score = -INFINITE_EVAL;
	int opp_col = OPP_COLOR( color );
	int in_alpha = alpha;
	int ev;
	int flipped;
	int new_disc_diff;
	int sq, old_sq, best_sq = _NULL_MOVE;
	unsigned int parity_mask;
	HashEntry entry;

	INCREMENT_COUNTER( tree_nodes );

	entry = hash_find( ENDGAME_MODE );
	if ( (entry.draft == empties) &&
		(entry.selectivity == 0) &&
		move_is_valid( my_bits, opp_bits, entry.move[0] ) &&
		(entry.flags & ENDGAME_SCORE) &&
		((entry.flags & EXACT_VALUE) ||
		((entry.flags & LOWER_BOUND) && entry.eval >= beta) ||
		((entry.flags & UPPER_BOUND) && entry.eval <= alpha)) ) {
		best_move = entry.move[0];
		return entry.eval;
	}

	/* Check for stability cutoff */

#if USE_STABILITY
	if ( alpha >= stability_threshold[empties] ) {
		int stability_bound;

		stability_bound = 64 - 2 * count_edge_stable( opp_col, opp_bits, my_bits );
		if ( stability_bound <= alpha )
			return alpha;
		stability_bound = 64 - 2 * count_stable( opp_col, opp_bits, my_bits );
		if ( stability_bound < beta )
			beta = stability_bound + 1;
		if ( stability_bound <= alpha )
			return alpha;
	}
#endif

	/* Odd parity. */

	parity_mask = region_parity;

	if ( region_parity != 0 ) { /* Is there any region with odd parity? */
		for ( old_sq = END_MOVE_LIST_HEAD, sq = end_move_list[old_sq].succ;
			sq != END_MOVE_LIST_TAIL;
			old_sq = sq, sq = end_move_list[sq].succ ) {
			unsigned int holepar = quadrant_mask[sq];
			if ( holepar & parity_mask ) {
				flipped = TestFlips_wrapper( sq, my_bits, opp_bits );
				if ( flipped != 0 ) {
					BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

					region_parity ^= holepar;
					end_move_list[old_sq].succ = end_move_list[sq].succ;
					new_disc_diff = -disc_diff - 2 * flipped - 1;
					ev = -solve_parity( new_opp_bits, bb_flips, -beta, -alpha, opp_col,
							empties - 1, new_disc_diff, TRUE );
					region_parity ^= holepar;
					end_move_list[old_sq].succ = sq;

					if ( ev > score ) {
						score = ev;
						if ( ev > alpha ) {
							if ( ev >= beta ) {
								best_move = sq;
								hash_add( ENDGAME_MODE, score, best_move,
									ENDGAME_SCORE | LOWER_BOUND, empties, 0 );
								return score;
							}
							alpha = ev;
						}
						best_sq = sq;
					}
				}
			}
		}
	}

	/* Even parity. */

	parity_mask = ~parity_mask;

	for ( old_sq = END_MOVE_LIST_HEAD, sq = end_move_list[old_sq].succ;
		sq != END_MOVE_LIST_TAIL;
		old_sq = sq, sq = end_move_list[sq].succ ) {
		unsigned int holepar = quadrant_mask[sq];
		if ( holepar & parity_mask ) {
			flipped = TestFlips_wrapper( sq, my_bits, opp_bits );
			if ( flipped != 0 ) {
				BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

				region_parity ^= holepar;
				end_move_list[old_sq].succ = end_move_list[sq].succ;
				new_disc_diff = -disc_diff - 2 * flipped - 1;
				ev = -solve_parity( new_opp_bits, bb_flips, -beta, -alpha, opp_col,
						empties - 1, new_disc_diff, TRUE );
				region_parity ^= holepar;
				end_move_list[old_sq].succ = sq;

				if ( ev > score ) {
					score = ev;
					if ( ev > alpha ) {
						if ( ev >= beta ) {
							best_move = sq;
							hash_add( ENDGAME_MODE, score, best_move,
								ENDGAME_SCORE | LOWER_BOUND, empties, 0 );
							return score;
						}
						alpha = ev;
					}
					best_sq = sq;
				}
			}
		}
	}

	if ( score == -INFINITE_EVAL ) {
		if ( !pass_legal ) {
			if ( disc_diff > 0 )
				return disc_diff + empties;
			if ( disc_diff < 0 )
				return disc_diff - empties;
			return 0;
		}
		else {
			g_hash1 ^= g_hash_switch_side1;
			g_hash2 ^= g_hash_switch_side2;
			score = -solve_parity_hash( opp_bits, my_bits, -beta, -alpha, opp_col,
						empties, -disc_diff, FALSE );
			g_hash1 ^= g_hash_switch_side1;
			g_hash2 ^= g_hash_switch_side2;
		}
	}
	else {
		best_move = best_sq;
		if ( score > in_alpha)
			hash_add( ENDGAME_MODE, score, best_move, ENDGAME_SCORE | EXACT_VALUE,
				empties, 0 );
		else
			hash_add( ENDGAME_MODE, score, best_move, ENDGAME_SCORE | UPPER_BOUND,
				empties, 0 );
	}

	return score;
}

static int
solve_parity_hash_high( BitBoard my_bits,
					   BitBoard opp_bits,
					   int alpha,
					   int beta,
					   int color,
					   int empties,
					   int disc_diff,
					   int pass_legal ) {
	/* Move bonuses without and with parity for the squares.
	   These are only used when sorting moves in the 9-12 empties
	   range and were automatically tuned by OPTIMIZE. */
	static int move_bonus[2][64] = {  /* 2 * 64 used */
		{
			24,   1,   0,  25,  25,   0,   1,  24,
			 1,   0,   0,   0,   0,   0,   0,   1,
			 0,   0,   0,   0,   0,   0,   0,   0,
			25,   0,   0,   0,   0,   0,   0,  25,
			25,   0,   0,   0,   0,   0,   0,  25,
			 0,   0,   0,   0,   0,   0,   0,   0,
			 1,   0,   0,   0,   0,   0,   0,   1,
			24,   1,   0,  25,  25,   0,   1,  24,
			/*
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			//*/
		},
		{
			128,  86, 122, 125, 125, 122,  86, 128,
			 86, 117, 128, 128, 128, 128, 117,  86,
			122, 128, 128, 128, 128, 128, 128, 122,
			125, 128, 128, 128, 128, 128, 128, 125,
			125, 128, 128, 128, 128, 128, 128, 125,
			122, 128, 128, 128, 128, 128, 128, 122,
			 86, 117, 128, 128, 128, 128, 117,  86,
			128,  86, 122, 125, 125, 122,  86, 128,
			/*
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
			//*/
		}
	};
	BitBoard new_opp_bits;
	BitBoard best_new_my_bits, best_new_opp_bits;
	BitBoard best_flip_bits;
	int i;
	int score;
	int in_alpha = alpha;
	int opp_col = OPP_COLOR( color );
	int flipped, best_flipped;
	int new_disc_diff;
	int ev;
	int hash_move;
	int moves;
	int parity;
	int best_value, best_index;
	int pred, succ;
	int sq, old_sq, best_sq = _NULL_MOVE;
	int move_order[64];
	int goodness[64];
	//unsigned int diff1, diff2;
	HashEntry entry;

	INCREMENT_COUNTER( tree_nodes );

	hash_move = _NULL_MOVE;
	entry = hash_find( ENDGAME_MODE );
	if ( entry.draft == empties ) {
		if ( (entry.selectivity == 0) &&
			(entry.flags & ENDGAME_SCORE) &&
			move_is_valid( my_bits, opp_bits, entry.move[0] ) &&
			((entry.flags & EXACT_VALUE) ||
			((entry.flags & LOWER_BOUND) && entry.eval >= beta) ||
			((entry.flags & UPPER_BOUND) && entry.eval <= alpha)) ) {
			best_move = entry.move[0];
			return entry.eval;
		}
	}

	/* Check for stability cutoff */

#if USE_STABILITY
	if ( alpha >= stability_threshold[empties] ) {
		int stability_bound;

		stability_bound = 64 - 2 * count_edge_stable( opp_col, opp_bits, my_bits );
		if ( stability_bound <= alpha )
			return alpha;
		stability_bound = 64 - 2 * count_stable( opp_col, opp_bits, my_bits );
		if ( stability_bound < beta )
			beta = stability_bound + 1;
		if ( stability_bound <= alpha )
			return alpha;
	}
#endif

	/* Calculate goodness values for all moves */

	moves = 0;
	best_value = -INFINITE_EVAL;
	best_index = 0;
	best_flipped = 0;

	for ( old_sq = END_MOVE_LIST_HEAD, sq = end_move_list[old_sq].succ;
		sq != END_MOVE_LIST_TAIL;
		old_sq = sq, sq = end_move_list[sq].succ ) {
		flipped = TestFlips_wrapper( sq, my_bits, opp_bits );
		if ( flipped != 0 ) {
			INCREMENT_COUNTER( tree_nodes );

			BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

			end_move_list[old_sq].succ = end_move_list[sq].succ;

			if ( quadrant_mask[sq] & region_parity )
				parity = 1;
			else
				parity = 0;
			goodness[moves] = move_bonus[parity][sq];
			if ( sq == hash_move )
				goodness[moves] += 128;

			goodness[moves] -= weighted_mobility( new_opp_bits, bb_flips );

			if ( goodness[moves] > best_value ) {
				best_value = goodness[moves];
				best_index = moves;
				best_new_my_bits = bb_flips;
				best_new_opp_bits = new_opp_bits;
				best_flip_bits.high = bb_flips.high ^ my_bits.high ^ square_mask[sq].high;
				best_flip_bits.low = bb_flips.low ^ my_bits.low ^ square_mask[sq].low;
				//best_flip_bits = bb_flips;
				best_flipped = flipped;
			}

			end_move_list[old_sq].succ = sq;
			move_order[moves] = sq;
			moves++;
		}
	}

	/* Maybe there aren't any legal moves */

	if ( moves == 0 ) {  /* I have to pass */
		if ( !pass_legal ) {  /* Last move also pass, game over */
			if ( disc_diff > 0 )
				return disc_diff + empties;
			if ( disc_diff < 0 )
				return disc_diff - empties;
			return 0;
		}
		else {  /* Opponent gets the chance to play */
			g_hash1 ^= g_hash_switch_side1;
			g_hash2 ^= g_hash_switch_side2;
			score = -solve_parity_hash_high( opp_bits, my_bits, -beta, -alpha,
						opp_col, empties, -disc_diff, FALSE );
			g_hash1 ^= g_hash_switch_side1;
			g_hash2 ^= g_hash_switch_side2;
			return score;
		}
	}

	/* Try move with highest goodness value */

	sq = move_order[best_index];

	//TestFlips_wrapper( sq, my_bits, opp_bits );
	//bb_flips = best_flip_bits;
	bb_flip_bits = best_flip_bits;
	make_move_end( color, sq, TRUE );

	/*
	(void) DoFlips_hash( sq, color );

	board[sq] = color;
	diff1 = g_hash_update_v1 ^ hash_put_value1[color][sq];
	diff2 = g_hash_update_v2 ^ hash_put_value2[color][sq];
	g_hash1 ^= diff1;
	g_hash2 ^= diff2;
	//*/

	region_parity ^= quadrant_mask[sq];

	pred = end_move_list[sq].pred;
	succ = end_move_list[sq].succ;
	end_move_list[pred].succ = succ;
	end_move_list[succ].pred = pred;

	new_disc_diff = -disc_diff - 2 * best_flipped - 1;
	if ( empties <= LOW_LEVEL_DEPTH + 1 )
		score = -solve_parity_hash( best_new_opp_bits, best_new_my_bits,
					-beta, -alpha, opp_col, empties - 1,
					new_disc_diff, TRUE );
	else
		score = -solve_parity_hash_high( best_new_opp_bits, best_new_my_bits,
					-beta, -alpha, opp_col, empties - 1,
					new_disc_diff, TRUE );

	unmake_move_end( color, sq );
	/*
	//UndoFlips( best_flipped, opp_col );
	g_hash1 ^= diff1;
	g_hash2 ^= diff2;
	board[sq] = CHESS_EMPTY;
	//*/

	region_parity ^= quadrant_mask[sq];

	end_move_list[pred].succ = sq;
	end_move_list[succ].pred = sq;

	best_sq = sq;
	if ( score > alpha ) {
		if ( score >= beta ) {
			best_move = best_sq;
			hash_add( ENDGAME_MODE, score, best_move,
				ENDGAME_SCORE | LOWER_BOUND, empties, 0 );
			return score;
		}
		alpha = score;
	}

	/* Play through the rest of the moves */

	move_order[best_index] = move_order[0];
	goodness[best_index] = goodness[0];

	for ( i = 1; i < moves; i++ ) {
		int j;

		best_value = goodness[i];
		best_index = i;
		for ( j = i + 1; j < moves; j++ ) {
			if ( goodness[j] > best_value ) {
				best_value = goodness[j];
				best_index = j;
			}
		}
		sq = move_order[best_index];
		move_order[best_index] = move_order[i];
		goodness[best_index] = goodness[i];

		flipped = TestFlips_wrapper( sq, my_bits, opp_bits );
		BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

		bb_flip_bits.high = bb_flips.high ^ my_bits.high ^ square_mask[sq].high;
		bb_flip_bits.low = bb_flips.low ^ my_bits.low ^ square_mask[sq].low;
		make_move_end( color, sq, TRUE );

		/*
		(void) DoFlips_hash( sq, color );

		board[sq] = color;
		diff1 = g_hash_update_v1 ^ hash_put_value1[color][sq];
		diff2 = g_hash_update_v2 ^ hash_put_value2[color][sq];
		g_hash1 ^= diff1;
		g_hash2 ^= diff2;
		//*/

		region_parity ^= quadrant_mask[sq];

		pred = end_move_list[sq].pred;
		succ = end_move_list[sq].succ;
		end_move_list[pred].succ = succ;
		end_move_list[succ].pred = pred;

		new_disc_diff = -disc_diff - 2 * flipped - 1;

		if ( empties <= LOW_LEVEL_DEPTH )  /* Fail-high for opp is likely. */
			ev = -solve_parity_hash( new_opp_bits, bb_flips, -beta, -alpha,
					opp_col, empties - 1, new_disc_diff, TRUE );
		else
			ev = -solve_parity_hash_high( new_opp_bits, bb_flips, -beta, -alpha,
					opp_col, empties - 1, new_disc_diff, TRUE );

		region_parity ^= quadrant_mask[sq];

		unmake_move_end( color, sq );
		/*
		UndoFlips( flipped, opp_col );
		g_hash1 ^= diff1;
		g_hash2 ^= diff2;
		board[sq] = CHESS_EMPTY;
		//*/

		end_move_list[pred].succ = sq;
		end_move_list[succ].pred = sq;

		if ( ev > score ) {
			score = ev;
			if ( ev > alpha ) {
				if ( ev >= beta ) {
					best_move = sq;
					hash_add( ENDGAME_MODE, score, best_move,
						ENDGAME_SCORE | LOWER_BOUND, empties, 0 );
					return score;
				}
				alpha = ev;
			}
			best_sq = sq;
		}
	}

	best_move = best_sq;
	if ( score > in_alpha )
		hash_add( ENDGAME_MODE, score, best_move,
			ENDGAME_SCORE | EXACT_VALUE, empties, 0 );
	else
		hash_add( ENDGAME_MODE, score, best_move,
			ENDGAME_SCORE | UPPER_BOUND, empties, 0 );

	return score;
}

/*
  END_SOLVE
  The search itself. Assumes relevant data structures have been set up with
  PREPARE_TO__SOLVE(). Returns difference between disc count for
  COLOR and disc count for the opponent of COLOR.
*/

static int
end_solve( BitBoard my_bits,
		  BitBoard opp_bits,
		  int alpha,
		  int beta,
		  int color,
		  int empties,
		  int discdiff,
		  int prevmove ) {
	int result;

	if ( empties <= LOW_LEVEL_DEPTH )
		result = solve_parity( my_bits, opp_bits, alpha, beta, color, empties,
			discdiff, prevmove );
	else
		result = solve_parity_hash_high( my_bits, opp_bits, alpha, beta, color,
			empties, discdiff, prevmove );

	return result;
}


/*
  END_TREE_SEARCH
  Plain nega-scout with fastest-first move ordering.
*/

static int
end_tree_search( int depth,
				int max_depth,
				BitBoard my_bits,
				BitBoard opp_bits,
				int color,
				int alpha,
				int beta,
				int selectivity,
				int *selective_cutoff,
				int void_legal ) {
	static char buffer[32];
	double node_val;
	int i, j;
	int empties;
	int disk_diff;
	int previous_move;
	int result;
	int curr_val, best;
	int move;
	int hash_hit;
	int move_index;
	int remains, exp_depth, pre_depth;
	int update_pv, first, use_hash;
	int my_discs, opp_discs;
	int curr_alpha;
	int pre_search_done;
	int mobility;
	int threshold;
	int flipped;
	int best_list_index, best_list_length;
	int best_list[4];
	HashEntry entry, mid_entry;
    HashEntry etc_entry;
#if CHECK_HASH_CODES
	unsigned int h1, h2;
#endif
#if USE_STABILITY
	int stability_bound;
#endif

	if ( depth == 0 ) {
		sprintf( buffer, "[%d,%d]:", alpha, beta );
		clear_sweep();
	}
	remains = max_depth - depth;
	*selective_cutoff = FALSE;

	/* Always (almost) check for stability cutoff in this region of search */

#if USE_STABILITY
	if ( alpha >= HIGH_STABILITY_THRESHOLD ) {
		stability_bound = 64 -
			2 * count_edge_stable( OPP_COLOR( color ), opp_bits, my_bits );
		if ( stability_bound <= alpha ) {
			pv_depth[depth] = depth;
			return alpha;
		}
		stability_bound = 64 -
			2 * count_stable( OPP_COLOR( color ), opp_bits, my_bits );
		if ( stability_bound < beta )
			beta = stability_bound + 1;
		if ( stability_bound <= alpha ) {
			pv_depth[depth] = depth;
			return alpha;
		}
	}
#endif

	/* Check if the low-level code is to be invoked */

	my_discs = piece_count[color][disks_played];
	opp_discs = piece_count[OPP_COLOR( color )][disks_played];
	//my_discs = disc_count(my_bits, opp_bits, TRUE);
	//opp_discs = disc_count(my_bits, opp_bits, FALSE);

	empties = 64 - my_discs - opp_discs;
	if ( remains <= FASTEST_FIRST_DEPTH ) {
		disk_diff = my_discs - opp_discs;
		if ( void_legal )  /* Is PASS legal or was last move a pass? */
			previous_move = D4; //D4;   /* d4, of course impossible */
		else
			previous_move = 0;  //0;    /* pass move */

		prepare_to_solve( my_bits, opp_bits );
		result = end_solve( my_bits, opp_bits, alpha, beta, color,
			empties, disk_diff, previous_move );

		pv_depth[depth] = depth + 1;
		pv[depth][depth] = best_move;

		if ( (depth == 0) && (get_ponder_move() == _NULL_MOVE) ) {
			send_sweep( "%-10s ", buffer );
			send_sweep( "%c%c", TO_SQUARE( best_move ) );
			if ( result <= alpha )
				send_sweep( "<%d", result + 1 );
			else if ( result >= beta )
				send_sweep( ">%d", result - 1 );
			else
				send_sweep( "=%d", result );
		}
		return result;
	}

	/* Otherwise normal search */

	INCREMENT_COUNTER( tree_nodes );

	use_hash = USE_HASH_TABLE;
	if ( use_hash ) {

#if CHECK_HASH_CODES
		h1 = g_hash1;
		h2 = g_hash2;
#endif

		/* Check for endgame hash table move */

		entry = hash_find( ENDGAME_MODE );
		if ( (entry.draft == remains) &&
			(entry.selectivity <= selectivity) &&
			move_is_valid( my_bits, opp_bits, entry.move[0] ) &&
			(entry.flags & ENDGAME_SCORE) &&
			((entry.flags & EXACT_VALUE) ||
			((entry.flags & LOWER_BOUND) && entry.eval >= beta) ||
			((entry.flags & UPPER_BOUND) && entry.eval <= alpha)) ) {
			pv[depth][depth] = entry.move[0];
			pv_depth[depth] = depth + 1;
			if ( (depth == 0) && (get_ponder_move() == _NULL_MOVE) ) {  /* Output some stats */
				send_sweep( "%c%c", TO_SQUARE( entry.move[0] ) );
				if ( (entry.flags & ENDGAME_SCORE) &&
					(entry.flags & EXACT_VALUE) )
					send_sweep( "=%d", entry.eval );
				else if ( (entry.flags & ENDGAME_SCORE) &&
					(entry.flags & LOWER_BOUND) )
					send_sweep( ">%d", entry.eval - 1 );
				else
					send_sweep( "<%d", entry.eval + 1 );
#ifdef TEXT_BASED
				fflush( stdout );
#endif
			}
			if ( entry.selectivity > 0 )
				*selective_cutoff = TRUE;
			return entry.eval;
		}

		hash_hit = (entry.draft != NO_HASH_MOVE);

		/* If not any such found, check for a midgame hash move */

		mid_entry = hash_find( MIDGAME_MODE );
		if ( (mid_entry.draft != NO_HASH_MOVE) &&
			(mid_entry.flags & MIDGAME_SCORE) ) {
			if ( (depth <= 4) || (mid_entry.flags & (EXACT_VALUE | LOWER_BOUND)) ) {
				/* Give the midgame move full priority if we're are the root
					of the tree, no endgame hash move was found and the position
					isn't in the wipeout zone. */

				if ( (depth == 0) && (!hash_hit) &&
					(mid_entry.eval < WIPEOUT_THRESHOLD * 128) ) {
					entry = mid_entry;
					hash_hit = TRUE;
				}
			}
		}
	}
	/*
	else {
		hash_hit = FALSE;
	}
	//*/

	/* Use endgame multi-prob-cut to selectively prune the tree */

	if ( USE_MPC && (depth > 2) && (selectivity > 0) ) {
		int cut;
		for ( cut = 0; cut < use_end_cut[disks_played]; cut++ ) {
			int shallow_remains = end_mpc_depth[disks_played][cut];
			int mpc_bias = (int)ceil( end_mean[disks_played][shallow_remains] * 128.0 );
			int mpc_window = (int)ceil( end_sigma[disks_played][shallow_remains] *
				end_percentile[selectivity] * 128.0 );
			int beta_bound = 128 * beta + mpc_bias + mpc_window;
			int alpha_bound = 128 * alpha + mpc_bias - mpc_window;
			int shallow_val =
				tree_search( my_bits, opp_bits, depth, depth + shallow_remains,
					alpha_bound, beta_bound, color, use_hash, FALSE, void_legal );
			if ( shallow_val >= beta_bound ) {
				if ( use_hash )
					hash_add( ENDGAME_MODE, alpha, pv[depth][depth],
						ENDGAME_SCORE | LOWER_BOUND, remains, selectivity );
				*selective_cutoff = TRUE;
				return beta;
			}
			if ( shallow_val <= alpha_bound ) {
				if ( use_hash )
					hash_add( ENDGAME_MODE, beta, pv[depth][depth],
						ENDGAME_SCORE | UPPER_BOUND, remains, selectivity );
				*selective_cutoff = TRUE;
				return alpha;
			}
		}
	}

	/* Determine the depth of the shallow search used to find
		achieve good move sorting */

	if ( remains >= DEPTH_TWO_SEARCH ) {
		if ( remains >= DEPTH_THREE_SEARCH ) {
			if ( remains >= DEPTH_FOUR_SEARCH ) {
				if ( remains >= DEPTH_SIX_SEARCH )
					pre_depth = 6;
				else
					pre_depth = 4;
			}
			else {
				pre_depth = 3;
			}
		}
		else {
			pre_depth = 2;
		}
	}
	else
		pre_depth = 1;

	if ( depth == 0 ) {  /* Deeper pre-search from the root */
		pre_depth += EXTRA_ROOT_SEARCH;
		if ( (pre_depth % 2) == 1 )  /* Avoid odd depths from the root */
			pre_depth++;
	}

	/* The nega-scout search */

	exp_depth = remains;
	first = TRUE;
	best = -INFINITE_EVAL;
	pre_search_done = FALSE;
	curr_alpha = alpha;

	/* Initialize the move list and check the hash table move list */

	move_count[disks_played] = 0;
	best_list_length = 0;
	for ( i = 0; i < 4; i++ )
		best_list[i] = 0;

	if ( hash_hit ) {
		//BitBoard save_my_bits, save_opp_bits;
		//save_my_bits = my_bits;
		//save_opp_bits = opp_bits;
		for ( i = 0; i < 4; i++ ) {
			if ( move_is_valid( my_bits, opp_bits, entry.move[i] ) ) {
				best_list[best_list_length++] = entry.move[i];

				/* Check for ETC among the hash table moves */
				//int test_move = entry.move[i];
				if ( use_hash &&
					//( (((my_bits.low | opp_bits.low) & square_mask[test_move].low) == 0) &&
					//(((my_bits.high | opp_bits.high) & square_mask[test_move].high) == 0) ) &&
					/*
					( ((my_bits.low & square_mask[test_move].low) == 0) &&
					((my_bits.high & square_mask[test_move].high) == 0) &&
					((opp_bits.low & square_mask[test_move].low) == 0) &&
					((opp_bits.high & square_mask[test_move].high) == 0) ) &&
					//*/
					//(make_move( &my_bits, &opp_bits, color, entry.move[i], TRUE ) != 0) ) /* [ */
					( (flipped = TestFlips_wrapper( entry.move[i], my_bits, opp_bits )) > 0 ) ) {

					/*
					BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );
					//*/
					bb_flip_bits.high = bb_flips.high ^ my_bits.high ^ square_mask[entry.move[i]].high;
					bb_flip_bits.low = bb_flips.low ^ my_bits.low ^ square_mask[entry.move[i]].low;
					make_move_end2( color, entry.move[i], flipped, TRUE );

					etc_entry = hash_find( ENDGAME_MODE );

					if ( (etc_entry.flags & ENDGAME_SCORE) &&
						(etc_entry.draft == empties - 1) &&
						(etc_entry.selectivity <= selectivity) &&
						(etc_entry.flags & (UPPER_BOUND | EXACT_VALUE)) &&
						(etc_entry.eval <= -beta) ) {

						/* Immediate cutoff from this move, move it up front */

						for ( j = best_list_length - 1; j >= 1; j-- )
							best_list[j] = best_list[j - 1];
						best_list[0] = entry.move[i];
					}
					unmake_move( color, entry.move[i] );
					//my_bits = save_my_bits;
					//opp_bits = save_opp_bits;
				}
			}
		}
	}

	for ( move_index = 0, best_list_index = 0; TRUE;
		move_index++, best_list_index++ ) {
		int child_selective_cutoff;
		BitBoard new_my_bits;
		BitBoard new_opp_bits;

		/* Use results of shallow searches to determine the move order */

		if ( (best_list_index < best_list_length) ) {
			move = best_list[best_list_index];
			move_count[disks_played]++;
		}
		else {
			if ( !pre_search_done ) {
				int shallow_index;

				pre_search_done = TRUE;

				threshold =
					MIN( WIPEOUT_THRESHOLD * 128,
					128 * alpha + fast_first_threshold[disks_played][pre_depth] );

				for ( shallow_index = 0; shallow_index < MOVE_ORDER_SIZE;
					shallow_index++ ) {
					int already_checked;

					move = sorted_move_order[disks_played][shallow_index];
					already_checked = FALSE;
					for ( j = 0; j < best_list_length; j++ ) {
						if ( move == best_list[j] )
							already_checked = TRUE;
					}

					if ( (!already_checked) &&
						//( (((my_bits.low | opp_bits.low) & square_mask[move].low) == 0) &&
						//(((my_bits.high | opp_bits.high) & square_mask[move].high) == 0) ) &&
						///*
						( ((my_bits.low & square_mask[move].low) == 0) &&
						((my_bits.high & square_mask[move].high) == 0) &&
						((opp_bits.low & square_mask[move].low) == 0) &&
						((opp_bits.high & square_mask[move].high) == 0) ) &&
						//*/
						((flipped = TestFlips_wrapper( move, my_bits, opp_bits )) > 0) ) {

						new_my_bits = bb_flips;
						BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

						bb_flip_bits.high = bb_flips.high ^ my_bits.high ^ square_mask[move].high;
						bb_flip_bits.low = bb_flips.low ^ my_bits.low ^ square_mask[move].low;
						make_move_end2( color, move, flipped, TRUE );
						curr_val = 0;

						/* Enhanced Transposition Cutoff: It's a good idea to
							transpose back into a position in the hash table. */

						if ( use_hash ) {
							HashEntry etc_entry = hash_find( ENDGAME_MODE );
							if ( (etc_entry.flags & ENDGAME_SCORE) &&
								(etc_entry.draft == empties - 1) ) {
								curr_val += 384;
								if ( etc_entry.selectivity <= selectivity ) {
									if ( (etc_entry.flags & (UPPER_BOUND | EXACT_VALUE)) &&
										(etc_entry.eval <= -beta) )
										curr_val = GOOD_TRANSPOSITION_EVAL;
									if ( (etc_entry.flags & LOWER_BOUND) &&
										(etc_entry.eval >= -alpha) )
										curr_val -= 640;
								}
							}
						}

						/* Determine the midgame score. If it is worse than
							alpha-8, a fail-high is likely so precision in that
							range is not worth the extra nodes required. */

						if ( curr_val != GOOD_TRANSPOSITION_EVAL )
							curr_val -=
								tree_search( new_opp_bits, bb_flips, depth + 1, depth + pre_depth,
									-INFINITE_EVAL, (-alpha + 8) * 128, OPP_COLOR( color ),
									TRUE, TRUE, TRUE );

						/* Make the moves which are highly likely to result in
							fail-high in decreasing order of mobility for the
							opponent. */

						if ( (curr_val > threshold) || (move == mid_entry.move[0]) ) {
							if ( curr_val > WIPEOUT_THRESHOLD * 128 )
								curr_val += 2 * VERY_HIGH_EVAL;
							else
								curr_val += VERY_HIGH_EVAL;
							if ( curr_val < GOOD_TRANSPOSITION_EVAL ) {
								mobility = bitboard_mobility( new_opp_bits, new_my_bits );
								if ( curr_val > 2 * VERY_HIGH_EVAL )
									curr_val -= 2 * ff_mob_factor[disks_played - 1] * mobility;
								else
									curr_val -= ff_mob_factor[disks_played - 1] * mobility;
							}
						}

						unmake_move( color, move );
						evals[disks_played][move] = curr_val;
						move_list[disks_played][move_count[disks_played]] = move;
						move_count[disks_played]++;
					}
				}
			}

			if ( move_index == move_count[disks_played] )
				break;
			move = select_move( move_index, move_count[disks_played] );
		}

		node_val = counter_value( &tree_nodes );
		if ( node_val - last_panic_check >= EVENT_CHECK_INTERVAL) {
			/* Check for time abort */
			last_panic_check = node_val;
			check_panic_abort();

			/* Output status buffers if in interactive mode */
			if ( echo )
				display_buffers();

			/* Check for events */
			//handle_event( TRUE, FALSE, TRUE );
			if ( is_panic_abort() || force_return )
				return SEARCH_ABORT;
		}

		if ( (depth == 0) && (get_ponder_move() == _NULL_MOVE) ) {
			if ( first )
				send_sweep( "%-10s ", buffer );
			send_sweep( "%c%c", TO_SQUARE( move ) );
		}

		flipped = TestFlips_wrapper( move, my_bits, opp_bits );
		new_my_bits = bb_flips;
		BITBOARD_FULL_ANDNOT( new_opp_bits, opp_bits, bb_flips );

		bb_flip_bits.high = bb_flips.high ^ my_bits.high ^ square_mask[move].high;
		bb_flip_bits.low = bb_flips.low ^ my_bits.low ^ square_mask[move].low;
		make_move_end2( color, move, flipped, use_hash );

		update_pv = FALSE;
		if ( first ) {
			best = curr_val =
				-end_tree_search( depth + 1, depth + exp_depth,
					new_opp_bits, new_my_bits, OPP_COLOR( color ),
					-beta, -curr_alpha, selectivity,
					&child_selective_cutoff, TRUE );
			update_pv = TRUE;
			if ( depth == 0 )
				best_end_root_move = move;
		}
		else {
			curr_alpha = MAX( best, curr_alpha );
			curr_val =
				-end_tree_search( depth + 1, depth + exp_depth,
					new_opp_bits, new_my_bits, OPP_COLOR( color ),
					-(curr_alpha + 1), -curr_alpha,
					selectivity, &child_selective_cutoff, TRUE );
			if ( (curr_val > curr_alpha) && (curr_val < beta) ) {
				if ( selectivity > 0 )
					curr_val =
						-end_tree_search( depth + 1, depth + exp_depth,
							new_opp_bits, new_my_bits, OPP_COLOR( color ),
							-beta, INFINITE_EVAL,
							selectivity, &child_selective_cutoff,  TRUE );
				else
					curr_val =
						-end_tree_search( depth + 1, depth + exp_depth,
							new_opp_bits, new_my_bits, OPP_COLOR( color ),
							-beta, -curr_val,
							selectivity, &child_selective_cutoff, TRUE );
				if ( curr_val > best ) {
					best = curr_val;
					update_pv = TRUE;
					if ( (depth == 0) && !is_panic_abort() && !force_return )
						best_end_root_move = move;
				}
			}
			else if ( curr_val > best ) {
				best = curr_val;
				update_pv = TRUE;
				if ( (depth == 0) && !is_panic_abort() && !force_return )
					best_end_root_move = move;
			}
		}

		if ( best >= beta )  /* The other children don't matter in this case. */
			*selective_cutoff = child_selective_cutoff;
		else if ( child_selective_cutoff )
			*selective_cutoff = TRUE;

		unmake_move( color, move );

		if ( is_panic_abort() || force_return )
			return SEARCH_ABORT;

		if ( (depth == 0) && (get_ponder_move() == _NULL_MOVE) ) {  /* Output some stats */
			if ( update_pv ) {
				if ( curr_val <= alpha )
					send_sweep( "<%d", curr_val + 1 );
				else {
					if ( curr_val >= beta )
						send_sweep( ">%d", curr_val - 1 );
					else {
						send_sweep( "=%d", curr_val );
						true_found = TRUE;
						true_val = curr_val;
					}
				}
			}
			send_sweep( " " );
			if ( update_pv && (move_index > 0) && echo)
				display_sweep( stdout );
		}

		if ( update_pv ) {
			update_best_list( best_list, move, best_list_index, &best_list_length,
				depth == 0 );
			pv[depth][depth] = move;
			pv_depth[depth] = pv_depth[depth + 1];
			for ( i = depth + 1; i < pv_depth[depth + 1]; i++ )
				pv[depth][i] = pv[depth + 1][i];
		}
		if ( best >= beta ) {  /* Fail high */
			if ( use_hash )
				hash_add_extended( ENDGAME_MODE, best, best_list,
					ENDGAME_SCORE | LOWER_BOUND, remains,
					*selective_cutoff ? selectivity : 0 );
			return best;
		}

		if ( (best_list_index >= best_list_length) && !update_pv &&
			(best_list_length < 4) )
			best_list[best_list_length++] = move;

		first = FALSE;
	}

#if CHECK_HASH_CODES && defined( TEXT_BASED )
	if ( use_hash ) {
		if ( (h1 != g_hash1) || (h2 != g_hash2) )
			printf( "%s: %x%x    %s: %x%x", HASH_BEFORE, h1, h2,
				HASH_AFTER, g_hash1, g_hash2 );
	}
#endif

	if ( !first ) {
		if ( use_hash ) {
			int flags = ENDGAME_SCORE;
			if ( best > alpha )
				flags |= EXACT_VALUE;
			else
				flags |= UPPER_BOUND;
			hash_add_extended( ENDGAME_MODE, best, best_list, flags, remains,
				*selective_cutoff ? selectivity : 0 );
		}
		return best;
	}
	else if ( void_legal ) {
		if ( use_hash ) {
			g_hash1 ^= g_hash_switch_side1;
			g_hash2 ^= g_hash_switch_side2;
		}
		curr_val = -end_tree_search( depth, max_depth,
						opp_bits, my_bits, OPP_COLOR( color ),
						-beta, -alpha,
						selectivity, selective_cutoff, FALSE );

		if ( use_hash ) {
			g_hash1 ^= g_hash_switch_side1;
			g_hash2 ^= g_hash_switch_side2;
		}
		return curr_val;
	}
	else {
		pv_depth[depth] = depth;
		my_discs = piece_count[color][disks_played];
		opp_discs = piece_count[OPP_COLOR( color )][disks_played];
		disk_diff = my_discs - opp_discs;
		if ( my_discs > opp_discs )
			return 64 - 2 * opp_discs;
		else if ( my_discs == opp_discs )
			return 0;
		else
			return -(64 - 2 * my_discs);
	}
}

/*
  END_TREE_WRAPPER
  Wrapper onto END_TREE_SEARCH which applies the knowledge that
  the range of valid scores is [-64,+64].
*/

int
end_tree_wrapper(int depth,
				 int max_depth,
				 BitBoard my_bits,
				 BitBoard opp_bits,
				 int color,
				 int alpha,
				 int beta,
				 int selectivity,
				 int void_legal ) {
	int selective_cutoff;
	//BitBoard my_bits, opp_bits;

	//init_mmx();
	///////////////////////////////////////////////////////
	//set_bitboards( board, color, &my_bits, &opp_bits );
	///////////////////////////////////////////////////////

	return end_tree_search( depth, max_depth,
		my_bits, opp_bits, color,
		MAX( alpha, -64 ), MIN( beta, 64 ),
		selectivity, &selective_cutoff, void_legal );
}

/*
  END_TREE_WRAPPER2
  For pv routines call end_tree_wrapper().
*/
INLINE int
end_tree_wrapper2(int depth,
				 int max_depth,
				 BitBoard my_bits,
				 BitBoard opp_bits,
				 int color,
				 int alpha,
				 int beta,
				 int selectivity,
				 int void_legal ) {
	return end_tree_wrapper( depth, max_depth, my_bits, opp_bits, color, alpha, beta, selectivity, void_legal );
}

/*
  END_GAME
  Provides an interface to the fast endgame solver.
*/

int
end_game( BitBoard my_bits,
		 BitBoard opp_bits,
		 int color,
		 int wld,
		 int force_echo,
		 int allow_book,
		 int komi,
		 EvaluationType *eval_info ) {
	double current_confidence;
	enum { WIN, LOSS, DRAW, UNKNOWN } solve_status;
	int book_move;
	int empties;
	int selectivity;
	int alpha, beta;
	int any_search_result, exact_score_failed;
	int incomplete_search;
	int long_selective_search;
	int losing_fail_high;
	int komi_shift;
	int old_depth, old_eval;
	int last_window_center;
	int old_pv[MAX_SEARCH_DEPTH];
	EvaluationType book_eval_info;

	book_eval_info = create_eval_info( SELECTIVE_EVAL, UNSOLVED_POSITION,
		-32767, 0.0, 0, FALSE );

	empties = 64 - disc_count( my_bits, opp_bits, TRUE )
		- disc_count( my_bits, opp_bits, FALSE );

	/* In komi games, the WLD window is adjusted. */

	if ( color == CHESS_BLACK )
		komi_shift = komi;
	else
		komi_shift = -komi;
	if ( (abs( komi ) % 2 ) == 1 )  /* Compute closest possible loss */
		losing_fail_high = komi_shift - 1;
	else
		losing_fail_high = komi_shift - 2;

	/* Check if the position is solved (WLD or exact) in the book. */

	book_move = PASS_MOVE;
	/*
	if ( allow_book ) {
		/-* Is the exact score known? *-/

		fill_move_alternatives( color, FULL_SOLVED );
		book_move = get_book_move( color, FALSE, eval_info );
		if ( book_move != PASS ) {
			root_eval = eval_info->score / 128;
			hash_expand_pv( color, ENDGAME_MODE, EXACT_VALUE, 0 );
			send_solve_status( empties, color, eval_info );
			return book_move;
		}

		/-* Is the WLD status known? *-/

		fill_move_alternatives( color, WLD_SOLVED );
		book_move = get_book_move( color, FALSE, eval_info );
		if ( book_move != PASS ) {
			if ( wld ) {
				root_eval = eval_info->score / 128;
				hash_expand_pv( color, ENDGAME_MODE,
					EXACT_VALUE | UPPER_BOUND | LOWER_BOUND, 0  );
				send_solve_status( empties, color, eval_info );
				return book_move;
			}
			else
				book_eval_info = *eval_info;
		}
		fill_endgame_hash( HASH_DEPTH, 0 );
	}
	//*/

	last_panic_check = 0.0;
	solve_status = UNKNOWN;
	old_eval = 0;

	/* Prepare for the shallow searches using the midgame eval */

	//piece_count[CHESS_BLACK][disks_played] = disc_count( my_bits, opp_bits, (color == CHESS_BLACK) );
	//piece_count[CHESS_WHITE][disks_played] = disc_count( my_bits, opp_bits, (color == CHESS_WHITE) );

	if ( color == CHESS_BLACK ) {
		piece_count[CHESS_BLACK][disks_played] = disc_count( my_bits, opp_bits, TRUE );
		piece_count[CHESS_WHITE][disks_played] = disc_count( my_bits, opp_bits, FALSE );
	}
	else {
		piece_count[CHESS_BLACK][disks_played] = disc_count( my_bits, opp_bits, FALSE );
		piece_count[CHESS_WHITE][disks_played] = disc_count( my_bits, opp_bits, TRUE );
	}

	//hash_determine_values( my_bits, opp_bits, color );

	if ( empties > 32 )
		set_panic_threshold( 0.20 );
	else if ( empties < 22 )
		set_panic_threshold( 0.50 );
	else
		set_panic_threshold( 0.50 - (empties - 22) * 0.03 );

	reset_buffer_display();

	/* Make sure the pre-searches don't mess up the hash table */

	toggle_midgame_hash_usage( TRUE, FALSE );

	incomplete_search = FALSE;
	any_search_result = FALSE;

	/* Start off by selective endgame search */

	last_window_center = 0;

	if ( empties > DISABLE_SELECTIVITY ) {
		if ( wld ) {
			for ( selectivity = MAX_SELECTIVITY; (selectivity > 0) &&
				!is_panic_abort() && !force_return; selectivity-- ) {
				unsigned int flags;
				EvalResult res;

				alpha = -1;
				beta = +1;
				root_eval = end_tree_wrapper( 0, empties, my_bits, opp_bits, color,
					alpha, beta, selectivity, TRUE );

				adjust_counter( &tree_nodes );

				if ( is_panic_abort() || force_return )
					break;

				any_search_result = TRUE;
				old_eval = root_eval;
				store_pv( old_pv, &old_depth );
				current_confidence = confidence[selectivity];

				flags = EXACT_VALUE;
				if ( root_eval == 0 )
					res = DRAWN_POSITION;
				else {
					flags |= (UPPER_BOUND | LOWER_BOUND);
					if ( root_eval > 0 )
						res = WON_POSITION;
					else
						res = LOST_POSITION;
				}
				*eval_info =
					create_eval_info( SELECTIVE_EVAL, res, root_eval * 128,
						current_confidence, empties, FALSE );
				if ( full_output_mode ) {
					hash_expand_pv( my_bits, opp_bits, color, ENDGAME_MODE,
						flags, selectivity );
					send_solve_status( empties, color, eval_info );
				}
			}
		}
		else {
			for ( selectivity = MAX_SELECTIVITY; (selectivity > 0) &&
				!is_panic_abort() && !force_return; selectivity-- ) {
				alpha = last_window_center - 1;
				beta = last_window_center + 1;

				root_eval = end_tree_wrapper( 0, empties, my_bits, opp_bits, color,
					alpha, beta, selectivity, TRUE );

				if ( root_eval <= alpha ) {
					do {
						last_window_center -= 2;
						alpha = last_window_center - 1;
						beta = last_window_center + 1;
						if ( is_panic_abort() || force_return )
							break;
						root_eval = end_tree_wrapper( 0, empties, my_bits, opp_bits,
							color, alpha, beta, selectivity, TRUE );
					} while ( root_eval <= alpha );
					root_eval = last_window_center;
				}
				else if ( root_eval >= beta ) {
					do {
						last_window_center += 2;
						alpha = last_window_center - 1;
						beta = last_window_center + 1;
						if ( is_panic_abort() || force_return )
							break;
						root_eval = end_tree_wrapper( 0, empties, my_bits, opp_bits,
							color, alpha, beta, selectivity, TRUE );
					} while ( root_eval >= beta );
					root_eval = last_window_center;
				}

				adjust_counter( &tree_nodes );

				if ( is_panic_abort() || force_return )
					break;

				last_window_center = root_eval;

				if ( !is_panic_abort() && !force_return ) {
					any_search_result = TRUE;
					old_eval = root_eval;
					store_pv( old_pv, &old_depth );
					current_confidence = confidence[selectivity];

					*eval_info =
						create_eval_info( SELECTIVE_EVAL, UNSOLVED_POSITION,
						root_eval * 128, current_confidence,
						empties, FALSE );
					if ( full_output_mode ) {
						hash_expand_pv( my_bits, opp_bits, color, ENDGAME_MODE,
							EXACT_VALUE, selectivity );
						send_solve_status( empties, color, eval_info );
					}
				}
			}
		}
	}
	else
		selectivity = 0;

	/* Check if the selective search took more than 40% of the allocated
		time. If this is the case, there is no point attempting WLD. */

	long_selective_search = check_threshold( 0.35 );

	/* Make sure the panic abort flag is set properly; it must match
		the status of long_selective_search. This is not automatic as
		it is not guaranteed that any selective search was performed. */

	check_panic_abort();

	if ( is_panic_abort() || force_return || long_selective_search ) {

		/* Don't try non-selective solve. */

		if ( any_search_result ) {
			if ( echo && (is_panic_abort() || force_return) ) {
#ifdef TEXT_BASED
            /*
				printf( "%s %.1f %c %s\n", SEMI_PANIC_ABORT_TEXT, get_elapsed_time(),
					SECOND_ABBREV, SEL_SEARCH_TEXT );
            //*/
#endif
				if ( full_output_mode ) {
					unsigned int flags;

					flags = EXACT_VALUE;
					if ( solve_status != DRAW )
						flags |= (UPPER_BOUND | LOWER_BOUND);
					hash_expand_pv( my_bits, opp_bits, color, ENDGAME_MODE,
						flags, selectivity );
					send_solve_status( empties, color, eval_info );
				}
			}
			pv[0][0] = best_end_root_move;
			pv_depth[0] = 1;
			root_eval = old_eval;
			clear_panic_abort();
		}
		else {
#ifdef TEXT_BASED
			/*
			if ( echo )
				printf( "%s %.1f %c %s\n", PANIC_ABORT_TEXT, get_elapsed_time(),
				SECOND_ABBREV, SEL_SEARCH_TEXT );
			//*/
#endif
			root_eval = SEARCH_ABORT;
		}

		if ( echo || force_echo )
			display_status( stdout, FALSE );

		if ( (book_move != PASS_MOVE) &&
			((book_eval_info.res == WON_POSITION) ||
			(book_eval_info.res == DRAWN_POSITION)) ) {
			/* If there is a known win (or mismarked draw) available,
				always play it upon timeout. */
			*eval_info = book_eval_info;
			root_eval = eval_info->score / 128;
			return book_move;
		}
		else
			return pv[0][0];
	}

	/* Start non-selective solve */

	if ( wld ) {
		alpha = -1;
		beta = +1;
	}
	else {
		alpha = last_window_center - 1;
		beta = last_window_center + 1;
	}

	root_eval = end_tree_wrapper( 0, empties, my_bits, opp_bits, color,
		alpha, beta, 0, TRUE );

	adjust_counter( &tree_nodes );

	if ( !is_panic_abort() && !force_return ) {
		if ( !wld ) {
			if ( root_eval <= alpha ) {
				int ceiling_value = last_window_center - 2;
				while ( 1 ) {
					alpha = ceiling_value - 1;
					beta = ceiling_value;
					root_eval = end_tree_wrapper( 0, empties, my_bits, opp_bits,
						color, alpha, beta, 0, TRUE );
					if ( is_panic_abort() || force_return )
						break;
					if ( root_eval > alpha )
						break;
					else
						ceiling_value -= 2;
				}
			}
			else if ( root_eval >= beta ) {
				int floor_value = last_window_center + 2;
				while ( 1 ) {
					alpha = floor_value - 1;
					beta = floor_value + 1;
					root_eval = end_tree_wrapper( 0, empties, my_bits, opp_bits,
						color, alpha, beta, 0, TRUE );
					if ( is_panic_abort() || force_return )
						break;
					assert( root_eval > alpha );
					if ( root_eval < beta )
						break;
					else
						floor_value += 2;
				}
			}
		}
		if ( !is_panic_abort() && !force_return ) {
			EvalResult res;
			if ( root_eval < 0 )
				res = LOST_POSITION;
			else if ( root_eval == 0 )
				res = DRAWN_POSITION;
			else
				res = WON_POSITION;
			if ( wld ) {
				unsigned int flags;

				if ( root_eval == 0 )
					flags = EXACT_VALUE;
				else
					flags = UPPER_BOUND | LOWER_BOUND;
				*eval_info =
					create_eval_info( WLD_EVAL, res, root_eval * 128, 0.0,
						empties, FALSE );
				if ( full_output_mode ) {
					hash_expand_pv( my_bits, opp_bits, color, ENDGAME_MODE, flags, 0 );
					send_solve_status( empties, color, eval_info );
				}
			}
			else {
				*eval_info =
					create_eval_info( EXACT_EVAL, res, root_eval * 128, 0.0,
					empties, FALSE );
				if ( full_output_mode ) {
					hash_expand_pv( my_bits, opp_bits, color, ENDGAME_MODE, EXACT_VALUE, 0 );
					send_solve_status( empties, color, eval_info );
				}
			}
		}
	}

	adjust_counter( &tree_nodes );

	/* Check for abort. */

	if ( is_panic_abort() || force_return ) {
		if ( any_search_result ) {
			if ( echo ) {
#ifdef TEXT_BASED
            /*
				printf( "%s %.1f %c %s\n", SEMI_PANIC_ABORT_TEXT,
					get_elapsed_time(), SECOND_ABBREV, WLD_SEARCH_TEXT );
            //*/
#endif
				if ( full_output_mode ) {
					unsigned int flags;

					flags = EXACT_VALUE;
					if ( root_eval != komi_shift )  /* Not draw. */
						flags |= (UPPER_BOUND | LOWER_BOUND);
					hash_expand_pv( my_bits, opp_bits, color, ENDGAME_MODE, flags, 0 );
					send_solve_status( empties, color, eval_info );
				}
				if ( echo || force_echo )
					display_status( stdout, FALSE );
			}
			restore_pv( old_pv, old_depth );
			root_eval = old_eval;
			clear_panic_abort();
		}
		else {
#ifdef TEXT_BASED
			/*
			if ( echo )
				printf( "%s %.1f %c %s\n", PANIC_ABORT_TEXT,
				get_elapsed_time(), SECOND_ABBREV, WLD_SEARCH_TEXT );
			//*/
#endif
			root_eval = SEARCH_ABORT;
		}

		return pv[0][0];
	}

	/* Update solve info. */

	store_pv( old_pv, &old_depth );
	old_eval = root_eval;

	if ( !is_panic_abort() && !force_return && (empties > earliest_wld_solve) )
		earliest_wld_solve = empties;

	/* Check for aborted search. */

	exact_score_failed = FALSE;
	if ( incomplete_search ) {
		if ( echo ) {
#ifdef TEXT_BASED
			/*
			printf( "%s %.1f %c %s\n", SEMI_PANIC_ABORT_TEXT,
				get_elapsed_time(), SECOND_ABBREV, EXACT_SEARCH_TEXT );
			//*/
#endif
			if ( full_output_mode ) {
				hash_expand_pv( my_bits, opp_bits, color, ENDGAME_MODE, EXACT_VALUE, 0 );
				send_solve_status( empties, color, eval_info );
			}
			if ( echo || force_echo )
				display_status( stdout, FALSE );
		}
		pv[0][0] = best_end_root_move;
		pv_depth[0] = 1;
		root_eval = old_eval;
		exact_score_failed = TRUE;
		clear_panic_abort();
	}

	if ( (abs( root_eval ) % 2 ) == 1 ) {
		if ( root_eval > 0 )
			root_eval++;
		else
			root_eval--;
	}

	if ( !exact_score_failed && !wld && (empties > earliest_full_solve) )
		earliest_full_solve = empties;

	if ( !wld && !exact_score_failed ) {
		eval_info->type = EXACT_EVAL;
		eval_info->score = root_eval * 128;
	}

	if ( !wld && !exact_score_failed ) {
		hash_expand_pv( my_bits, opp_bits, color, ENDGAME_MODE, EXACT_VALUE, 0 );
		send_solve_status( empties, color, eval_info );
	}

	if ( echo || force_echo )
		display_status( stdout, FALSE );

	/* For shallow endgames, we can afford to compute the entire PV
		move by move. */

	/*
	if ( !wld && !incomplete_search && !force_return &&
		(empties <= PV_EXPANSION) )
		full_expand_pv( my_bits, opp_bits, color, 0 );
	//*/

	return pv[0][0];
}
