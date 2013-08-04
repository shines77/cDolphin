/*
   File:           dolphin.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Declarations of the main functions.
*/

#ifndef __DOLPHIN_H_
#define __DOLPHIN_H_

#include "utils.h"
#include "game.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Some default values for the interface */
#define DEFAULT_WAIT              0
#define DEFAULT_ECHO              1
#define DEFAULT_DISPLAY_PV        1

#define DEFAULT_HASH_BITS         20
#define DEFAULT_RANDOM            TRUE
#define DEFAULT_USE_BOOK          TRUE
#define DEFAULT_USE_THOR          FALSE
#define DEFAULT_SLACK             0.0
#define DEFAULT_WLD_ONLY          FALSE
#define SEPARATE_TABLES           FALSE
#define MAX_TOURNAMENT_SIZE       8
#define INFINIT_TIME              10000000.0
#define DEFAULT_DISPLAY_LINE      FALSE

extern int g_rand_terminal;

/* Administrative routines */

void
play_game( const char *file_name,
		  const char *move_string,
		  const char *move_file_name,
		  int repeat );

void
run_endgame_script( const char *in_file_name,
				   const char *out_file_name,
				   int display_line );

void
special_test( const char *file_name );

void dump_position( int color );
void dump_game_score( int color );

void
generate_random_endgame(const char *game_file,
						int rnd_discs,
						int empties_disc,
						int mid_depth,
						int max_position,
						int hash_bits);

#ifdef DOLPHIN_GGS

void
global_setup_ggs( int use_random,
				 int hash_bits );

int
play_game_ggs( int color,
			  int my_time,
			  int my_incr,
			  int opp_time,
			  int opp_incr,
			  int book,
			  int mid,
			  int exact,
			  int wld,
			  const char *idgame,
			  EvaluationType &eval_info );
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __DOLPHIN_H_ */
