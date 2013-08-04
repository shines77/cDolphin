/*
   File:           dolphin.cpp

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines manipulating the evaluation of board.
*/

#include <stdio.h>

#include "utils.h"
#include "global.h"
#include "colour.h"
#include "board.h"
#include "eval.h"
#include "move.h"
#include "search.h"
#include "coeffs.h"

/* Local variables */

static int use_experimental;

/*
   TOGGLE_EXPERIMENTAL
   Toggles usage of novelties in the evaluation function on/off.
*/

INLINE void
toggle_experimental( int use ) {
	use_experimental = use;
}

/*
  EXPERIMENTAL_EVAL
  Returns 1 if the experimental eval (if there is such) is used,
  0 otherwise.
*/

INLINE int
experimental_eval( void ) {
	return use_experimental;
}

/*
  INIT_EVAL
  Reset the evaluation module.
*/

void
init_eval( void ) {
}

#if 0

#ifdef _WIN32
static __int64
rdtsc( void ) {
	__int64 iCounter;
	__asm {
		rdtsc
		mov DWORD PTR iCounter, EAX
		mov DWORD PTR [iCounter+4], EDX
	}
	return iCounter;
	//__asm RDTSC
	//return 0;
}
#else
static long long int
rdtsc( void ) {
#if defined(__GNUC__)
	long long a;
	asm volatile("rdtsc":"=A" (a));
	return a;
#else
	return 0;
#endif
}
#endif

#endif

/*
  TERMINAL_EVALUATION
  Evaluates the position when no player has any legal moves.
*/

INLINE int
terminal_evaluation( int color ) {
	int disc_diff;
	int my_discs, opp_discs;

	INCREMENT_COUNTER( leaf_nodes );

	my_discs = piece_count[color][disks_played];
	opp_discs = piece_count[OPP_COLOR( color )][disks_played];

	if ( my_discs > opp_discs )
		disc_diff = 64 - 2 * opp_discs;
	else if ( opp_discs > my_discs )
		disc_diff = 2 * my_discs - 64;
	else
		disc_diff = 0;

	if ( disc_diff > 0 )
		return +MIDGAME_WIN + disc_diff;
	else if ( disc_diff == 0 )
		return 0;
	else
		return -MIDGAME_WIN + disc_diff;
}

/*
  EVALUATE_POSITION
  Evaluates the position.
  Invokes the proper evaluation function depending on whether the
  board is filled or not.
*/
INLINE int
evaluate_position(int color,
				  const BitBoard my_bits,
				  const BitBoard opp_bits)
{
	if ( disks_played >= 60 )
		return terminal_evaluation( color );
	else
		return pattern_evaluation( color, my_bits, opp_bits );
}
