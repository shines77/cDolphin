/*
   File:           search.c

   Created:        2007-07-10

   Modified:       2013-08-10

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Search routines designated to be used in the
                   midgame phase of the game.
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "global.h"
#include "colour.h"
#include "board.h"
#include "counter.h"
#include "hash.h"
#include "move.h"
#include "search.h"

/* Global variables */

double total_time;
int root_eval = 0;
int force_return = FALSE;
int list_inherited[61];
ALIGN_PREFIX(64) int sorted_move_order[64][64] ALIGN_SUFFIX(64);  /* 61*60 used */
ALIGN_PREFIX(64) Board evals[61] ALIGN_SUFFIX(64);

ALIGN_PREFIX(8) CounterType leaf_nodes, tree_nodes, total_nodes ALIGN_SUFFIX(8);
ALIGN_PREFIX(8) CounterType evaluations, total_evaluations ALIGN_SUFFIX(8);

/* When no other information is available, JCW's endgame
   priority order is used also in the midgame. */
int position_list[64] = {
    /* A1 */    A1 , H8 , A8 , H1 ,
    /* C1 */    C1 , F1 , A3 , H3 , A6 , H6 , C8 , F8 ,
    /* C3 */    C3 , F3 , C6 , F6 ,
    /* D1 */    D1 , E1 , A4 , H4 , A5 , H5 , D8 , E8 ,
    /* D3 */    D3 , E3 , C4 , F4 , C5 , F5 , D6 , E6 ,   // Bugfix: E3 wrong write to E5 !!!
    /* D2 */    D2 , E2 , B4 , G4 , B5 , G5 , D7 , E7 ,
    /* C2 */    C2 , F2 , B3 , G3 , B6 , G6 , C7 , F7 ,
    /* B1 */    B1 , G1 , A2 , H2 , A7 , H7 , B8 , G8 ,
    /* B2 */    B2 , G2 , B7 , G7 ,
    /* D4 */    D4 , E4 , D5 , E5
};

/* Local variables */

static int pondered_move = _NULL_MOVE;
static int negate_eval = 0;
static EvaluationType last_eval;

/*
  INIT_MOVE_LISTS
  Initalize the self-organizing move lists.
*/

static void
init_move_lists(void) {
    int i, j;

    for (i = 0; i <= 60; i++) {
        for (j = 0; j < MOVE_ORDER_SIZE; j++)
            sorted_move_order[i][j] = position_list[j];
    }
    for (i = 0; i <= 60; i++)
        list_inherited[i] = FALSE;
}

/*
  INHERIT_MOVE_LISTS
  If possible, initialize the move list corresponding to STAGE
  moves being played with an earlier move list from a stage
  corresponding to the same parity (i.e., in practice side to move).
*/

void
inherit_move_lists(int stage) {
    int i;
    int last;

    if (list_inherited[stage])
        return;
    list_inherited[stage] = TRUE;
    if (stage == 0)
        return;
    last = stage - 2;
    while ((last >= 0) && (!list_inherited[last]))
        last -= 2;
    if (last < 0)
        return;
    for (i = 0; i < MOVE_ORDER_SIZE; i++)
        sorted_move_order[stage][i] = sorted_move_order[last][i];
}

/*
  REORDER_MOVE_LIST
  Move the empty squares to the front of the move list.  Empty squares
  high up in the ranking are kept in place as they probably are empty
  in many variations in the tree.
*/

void
reorder_move_list(const BitBoard my_bits,
    const BitBoard opp_bits,
    int stage) {
    const int dont_touch = 24;
    int i;
    int move;
    int empty_pos;
    int nonempty_pos;
    int empty_buffer[MOVE_ORDER_SIZE];
    int nonempty_buffer[MOVE_ORDER_SIZE];

    empty_pos = 0;
    for (i = 0; i < MOVE_ORDER_SIZE; i++) {
        move = sorted_move_order[stage][i];
        if (((((my_bits.low & square_mask[move].low) == 0) &&
            ((my_bits.high & square_mask[move].high) == 0) &&
            ((opp_bits.low & square_mask[move].low) == 0) &&
            ((opp_bits.high & square_mask[move].high) == 0))) ||
            (i < dont_touch)) {
            empty_buffer[empty_pos] = move;
            empty_pos++;
        }
    }
    nonempty_pos = MOVE_ORDER_SIZE - 1;
    for (i = MOVE_ORDER_SIZE - 1; i >= 0; i--) {
        move = sorted_move_order[stage][i];
        if (((((my_bits.low & square_mask[move].low) != 0) ||
            ((my_bits.high & square_mask[move].high) != 0) ||
            ((opp_bits.low & square_mask[move].low) != 0) ||
            ((opp_bits.high & square_mask[move].high) != 0))) &&
            (i >= dont_touch)) {
            nonempty_buffer[nonempty_pos] = move;
            nonempty_pos--;
        }
    }
    for (i = 0; i < empty_pos; i++)
        sorted_move_order[stage][i] = empty_buffer[i];
    for (i = empty_pos; i < MOVE_ORDER_SIZE; i++)
        sorted_move_order[stage][i] = nonempty_buffer[i];
}

/*
   SETUP_SEARCH
   Initialize the history of the game in the search driver.
*/

void
setup_search(void) {
    init_move_lists();
    create_eval_info(UNINITIALIZED_EVAL, UNSOLVED_POSITION, 0, 0.0, 0, FALSE);
    negate_eval = FALSE;
}

/*
  NON_ITERATIVE_POPCOUNT
  Counts the number of bits set in a 64-bit integer.
  This is done using some bitfiddling tricks.
*/

#define m1      0x55555555
#define m2      0x33333333

INLINE unsigned int
non_iterative_popcount2(BitBoard b) {
    unsigned int a, n1, n2;

    a = b.high - ((b.high >> 1) & m1);
    n1 = (a & m2) + ((a >> 2) & m2);
    n1 = (n1 & 0x0F0F0F0F) + ((n1 >> 4) & 0x0F0F0F0F);
    n1 = (n1 & 0xFFFF) + (n1 >> 16);
    n1 = (n1 & 0xFF) + (n1 >> 8);

    a = b.low - ((b.low >> 1) & m1);
    n2 = (a & m2) + ((a >> 2) & m2);
    n2 = (n2 & 0x0F0F0F0F) + ((n2 >> 4) & 0x0F0F0F0F);
    n2 = (n2 & 0xFFFF) + (n2 >> 16);
    n2 = (n2 & 0xFF) + (n2 >> 8);

    return n1 + n2;
}

/*
   POP_COUNT_LOOP
   Slow but portable popcount and mobility codes.
*/

INLINE int
pop_count_loop(BitBoard bits) {
    unsigned int count = 0;

    while (bits.high) {
        bits.high &= (bits.high - 1);
        count++;
    }

    while (bits.low) {
        bits.low &= (bits.low - 1);
        count++;
    }

    return count;
}

/*
   DISC_COUNT
   color = the player whose disks are to be counted
   Returns the number of disks of a specified color.
*/

INLINE int
disc_count(BitBoard my_bits, BitBoard opp_bits, int is_my_color) {
    if (is_my_color)
        return non_iterative_popcount2(my_bits);
    else
        return non_iterative_popcount2(opp_bits);
}

/*
   DISC_COUNT2
   color = the player whose disks are to be counted
   Returns the number of disks of a specified color.
*/

INLINE int
disc_count2(int color) {
    int i, sum;

    sum = 0;
    for (i = 0; i < 64; i++) {
        if (board[i] == color)
            sum++;
    }

    return sum;
}

/*
  SET_PONDER_MOVE
  CLEAR_PONDER_MOVE
  GET_PONDER_MOVE
  A value of 0 denotes a normal search while anything else means
  that the search is performed given that the move indicated has
  been made.
*/

INLINE void
set_ponder_move(int move) {
    pondered_move = move;
}

INLINE void
clear_ponder_move(void) {
    pondered_move = _NULL_MOVE;
}

INLINE int
get_ponder_move(void) {
    return pondered_move;
}

/*
  SELECT_MOVE
  Finds the best move in the move list neglecting the first FIRST moves.
  Moves this move to the front of the sub-list.
*/

INLINE int
select_move(int first, int list_size) {
    int i;
    int temp_move;
    int best, best_eval;

    best = first;
    best_eval = evals[disks_played][move_list[disks_played][first]];
    for (i = first + 1; i < list_size; i++) {
        if (evals[disks_played][move_list[disks_played][i]] > best_eval) {
            best = i;
            best_eval = evals[disks_played][move_list[disks_played][i]];
        }
    }
    if (best != first) {
        temp_move = move_list[disks_played][first];
        move_list[disks_played][first] = move_list[disks_played][best];
        move_list[disks_played][best] = temp_move;
    }

    return move_list[disks_played][first];
}

/*
  CREATE_EVAL_INFO
  Creates a result descriptor given all the information available
  about the last search.
*/

EvaluationType
create_eval_info(EvalType in_type, EvalResult in_res,
                 int in_score, double in_conf,
                 int in_depth, int in_book) {
    EvaluationType out;

    out.type = in_type;
    out.res = in_res;
    out.score = in_score;
    out.confidence = in_conf;
    out.search_depth = in_depth;
    out.is_book = in_book;

    return out;
}

/*
  ADJUST_WLD_EVAL
  Adjust wld eval +1.00/-1.00 to +2.00/-2.00
*/

void
adjust_wld_eval(EvaluationType *eval_info) {
    double eval;
    int need_adjust;
    need_adjust = FALSE;

    switch (eval_info->type) {

    case WLD_EVAL:
        switch (eval_info->res) {
        case WON_POSITION:
            if (eval_info->score > 2 * 128)  /* Win by more than 2 */
                eval = (eval_info->score / 128.0) - 0.001;
            else
                eval = 2.0;
            break;
        case DRAWN_POSITION:
            eval = 0.0;
            break;
        case LOST_POSITION:
            if (eval_info->score < -2 * 128)  /* Loss by more than 2 */
                eval = (eval_info->score / 128.0) + 0.001;
            else
                eval = -2.0;
            break;
        case UNSOLVED_POSITION:
            eval = 0.0;
            break;
        }
        need_adjust = TRUE;

    case SELECTIVE_EVAL:
        switch (eval_info->res) {
        case WON_POSITION:
            eval = 1.0 + eval_info->confidence;
            break;
        case DRAWN_POSITION:
            eval = -1.0 + eval_info->confidence;
            break;
        case LOST_POSITION:
            eval = -1.0 - eval_info->confidence;
            break;
        case UNSOLVED_POSITION:
            eval = eval_info->score / 128.0;
            if (eval > 0.0 && eval <= 1.01)
                eval = 2.0;
            else if (eval < 0.0 && eval >= -1.01)
                eval = -2.0;
            break;
        }
        need_adjust = TRUE;

    case FORCED_EVAL:
    case PASS_EVAL:
    case INTERRUPTED_EVAL:
    case UNDEFINED_EVAL:
    case UNINITIALIZED_EVAL:
        break;

    default:
        break;

    }

    if (need_adjust) {
        if (eval >= 0.0)
            eval_info->score = (int)floor(eval * 128.0 + 0.0001);
        else
            eval_info->score = (int)floor(eval * 128.0 - 0.0001);
    }
}

/*
  PRODUCE_COMPACT_EVAL
  Converts a result descriptor into a number between -99.99 and 99.99 a la GGS.
*/

double
produce_compact_eval(EvaluationType eval_info) {
    double eval;

    switch (eval_info.type) {

    case MIDGAME_EVAL:
        /*
        eval = eval_info.search_depth + logistic_map( eval_info.score );
        if ( eval_info.is_book )
            eval = -eval;
        */
        eval = eval_info.score / 128.0;
        return eval;

    case EXACT_EVAL:
        return eval_info.score / 128.0;

    case WLD_EVAL:
        switch (eval_info.res) {
        case WON_POSITION:
            if (eval_info.score > 2 * 128)  /* Win by more than 2 */
                return (eval_info.score / 128.0) - 0.01;
            else
                return 1.99;
        case DRAWN_POSITION:
            return 0.0;
        case LOST_POSITION:
            if (eval_info.score < -2 * 128)  /* Loss by more than 2 */
                return (eval_info.score / 128.0) + 0.01;
            else
                return -1.99;
        case UNSOLVED_POSITION:
            return 0.0;
        }

    case SELECTIVE_EVAL:
        switch (eval_info.res) {
        case WON_POSITION:
            return 1.0 + eval_info.confidence;
        case DRAWN_POSITION:
            return -1.0 + eval_info.confidence;
        case LOST_POSITION:
            return -1.0 - eval_info.confidence;
        case UNSOLVED_POSITION:
            return eval_info.score / 128.0;
        }

    case FORCED_EVAL:
    case PASS_EVAL:
    case INTERRUPTED_EVAL:
    case UNDEFINED_EVAL:
    case UNINITIALIZED_EVAL:
        return 0.0;

    }

    return 0.0;  /* This statement shouldn't be reached */
}

/*
  SET_CURRENT_EVAL
  GET_CURRENT_EVAL
  NEGATE_CURRENT_EVAL
  Mutator and accessor functions for the global variable
  holding the last available position evaluation.
*/

void
set_current_eval(EvaluationType eval) {
    last_eval = eval;
    if (negate_eval) {
        last_eval.score = -last_eval.score;
        if (last_eval.res == WON_POSITION)
            last_eval.res = LOST_POSITION;
        else if (last_eval.res == LOST_POSITION)
            last_eval.res = WON_POSITION;
    }
}

INLINE EvaluationType
get_current_eval(void) {
    return last_eval;
}

INLINE void
negate_current_eval(int negate) {
    negate_eval = negate;
}
