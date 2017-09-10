/*
   File:           eval.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Evaluate board interface.
*/

#ifndef __EVAL_H_
#define __EVAL_H_

#include "bitboard.h"

#ifdef __cplusplus
extern "C" {
#endif

/* An evaluation indicating a won midgame position where no
   player has any moves available. */

// 12345678
#define INFINITE_EVAL           32767
//#define INFINITE_EVAL           32000
//#define INFINITE_EVAL           12345678
#define MIDGAME_WIN             29000
#define SEARCH_ABORT            -27000

/* An eval so high it must have originated from a midgame win
   disturbed by some randomness. */
#define ALMOST_MIDGAME_WIN    (MIDGAME_WIN - 4000)

void
toggle_experimental(int use);

int
experimental_eval(void);

void
init_eval(void);

int
evaluate_position(int color,
                  const BitBoard my_bits,
                  const BitBoard opp_bits);

int
evaluate_position2(int color);

int
terminal_evaluation(int color);

#ifdef __cplusplus
}
#endif

#endif  /* __EVAL_H_ */
