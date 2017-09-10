/*
   File:           endgame.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about DoFlip discs.
*/

#ifndef __ENDGAME_H_
#define __ENDGAME_H_

#include "board.h"
#include "bitboard.h"
#include "search.h"

#ifdef __cplusplus
extern "C" {
#endif

//#define END_MOVE_LIST_HEAD        65
//#define END_MOVE_LIST_TAIL        64

#define END_MOVE_LIST_HEAD        D4        // D4 = 27
#define END_MOVE_LIST_TAIL        E5        // E5 = 36

typedef struct tagMoveLink {
    int pred;
    int succ;
} MoveLink;

extern MoveLink end_move_list[64];
extern unsigned int quadrant_mask[64];

void
setup_end(void);

int
end_tree_wrapper(int depth,
                 int max_depth,
                 BitBoard my_bits,
                 BitBoard opp_bits,
                 int color,
                 int alpha,
                 int beta,
                 int selectivity,
                 int void_legal );

int
end_tree_wrapper2(int depth,
                 int max_depth,
                 BitBoard my_bits,
                 BitBoard opp_bits,
                 int color,
                 int alpha,
                 int beta,
                 int selectivity,
                 int void_legal );

int
end_game(BitBoard my_bits,
         BitBoard opp_bits,
         int color,
         int wld,
         int force_echo,
         int allow_book,
         int komi,
         EvaluationType *eval_info);

void
set_output_mode(int full);

void
setup_end(void);

int
get_earliest_wld_solve(void);

int
get_earliest_full_solve(void);

#ifdef __cplusplus
}
#endif

#endif  /* __ENDGAME_H_ */
