/*
   File:           midgame.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The midgame search driver.
*/

#ifndef __MIDGAME_H_
#define __MIDGAME_H_

#include "bitboard.h"
#include "search.h"

#ifdef __cplusplus
extern "C" {
#endif

#define INFINITE_DEPTH         60

/* The minimum depth to perform Multi-ProbCut */
#define MIN_MPC_DEPTH          9

void
setup_midgame( void );

void
toggle_midgame_hash_usage( int allow_read, int allow_write );

void
clear_midgame_abort( void );

int
is_midgame_abort( void );

void
set_midgame_abort( void );

void
toggle_midgame_abort_check( int toggle );

void
calculate_perturbation( void );

void
set_perturbation( int amplitude );

void
toggle_perturbation_usage( int toggle );

int
tree_search( BitBoard my_bits, BitBoard opp_bits, int depth, int max_depth,
			int alpha, int beta, int color, int allow_hash,
			int allow_mpc, int void_legal );

int
root_tree_search( BitBoard my_bits, BitBoard opp_bits, int depth, int max_depth,
				 int alpha, int beta, int color,
				 int allow_hash, int allow_mpc, int void_legal );

int
middle_game( BitBoard my_bits, BitBoard opp_bits, int max_depth, int color,
			int update_evals, EvaluationType *eval_info );

int
generate_random_game( BitBoard my_bits, BitBoard opp_bits,
					 int color, int depth, int max_depth );

#ifdef __cplusplus
}
#endif

#endif  /* __MIDGAME_H_ */

