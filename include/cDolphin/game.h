/*
   File:           game.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The interface to the game routines.
*/

#ifndef __GAME_H_
#define __GAME_H_

#include "bitboard.h"
#include "search.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct tagEvaluatedMove {
	EvaluationType eval;
	int color;
	int move;
	int pv_depth;
	int pv[60];
} EvaluatedMove;

void
toggle_status_log( int write_log );

void
global_setup( int use_random,
	      int hash_bits );

void
global_terminate( void );

void
game_init( const char *file_name, int *color );

#ifdef DOLPHIN_GGS

void
game_init_ggs( int color );

#endif

void
setup_game( const char *file_name, int *color );

void
set_komi( int in_komi );

void
toggle_human_openings( int toggle );

void
toggle_thor_match_openings( int toggle );

int
get_evaluated_count( void );

EvaluatedMove
get_evaluated( int index );

int
compute_move( BitBoard my_bits,
			 BitBoard opp_bits,
			 int color,
			 int update_all,
			 int my_time,
			 int my_incr,
			 int timed_depth,
			 int book,
			 int mid,
			 int exact,
			 int wld,
			 int search_forced,
			 EvaluationType *eval_info );

void
get_search_statistics( int *max_depth,
		       double *node_count );

int
get_pv( int *destin );

void
do_some_tests();

#ifdef __cplusplus
}
#endif

#endif  /* __GAME_H_ */
