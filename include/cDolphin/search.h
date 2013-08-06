/*
   File:           search.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The interface to common search routines and variables.
*/

#ifndef __SEARCH_H_
#define __SEARCH_H_

#include "global.h"
#include "counter.h"
#include "board.h"
#include "bitboard.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USE_RANDOMIZATION       TRUE
#define USE_HASH_TABLE          TRUE
#define CHECK_HASH_CODES        FALSE

#define MOVE_ORDER_SIZE         60

typedef enum eEvalType {
	MIDGAME_EVAL,
	EXACT_EVAL,
	WLD_EVAL,
	SELECTIVE_EVAL,
	FORCED_EVAL,
	PASS_EVAL,
	UNDEFINED_EVAL,
	INTERRUPTED_EVAL,
	UNINITIALIZED_EVAL
} EvalType;

typedef enum eEvalResult {
	WON_POSITION,
	DRAWN_POSITION,
	LOST_POSITION,
	UNSOLVED_POSITION
} EvalResult;

/* All information available about a move decision. */
typedef struct tagEvaluationType {
	EvalType type;
	EvalResult res;
	int score;              /* For BOOK, MIDGAME and EXACT */
	double confidence;      /* For SELECTIVE */
	int search_depth;       /* For MIDGAME */
	int is_book;
} EvaluationType;

/* The time spent searching during the game. */
extern double total_time;

/* The value of the root position from the last midgame or
   endgame search. Can contain strange values if an event
   occurred. */
extern int root_eval;

/* Event flag which forces the search to abort immediately when set. */
extern int force_return;

/* Holds the number of leaf nodes searched during the current search. */
extern ALIGN_PREFIX(8) CounterType leaf_nodes ALIGN_SUFFIX(8);

/* Holds the number of tree nodes searched during the current search. */
extern ALIGN_PREFIX(8) CounterType tree_nodes ALIGN_SUFFIX(8);

/* The number of positions evaluated during the current search. */
extern ALIGN_PREFIX(8) CounterType evaluations ALIGN_SUFFIX(8);

/* Holds the total number of nodes searched during the entire game. */
extern ALIGN_PREFIX(8) CounterType total_nodes ALIGN_SUFFIX(8);

/* The number of positions evaluated during the entire game. */
extern ALIGN_PREFIX(8) CounterType total_evaluations ALIGN_SUFFIX(8);

/* The last available evaluations for all possible moves at all
   possible game stages. */
extern ALIGN_PREFIX(64) Board evals[61] ALIGN_SUFFIX(64);

/* Move lists */
extern ALIGN_PREFIX(64) int sorted_move_order[64][64] ALIGN_SUFFIX(64);  /* 61*60 used */

/* JCW's move order */
extern int position_list[64];

void
inherit_move_lists( int stage );

void
reorder_move_list( const BitBoard my_bits,
				  const BitBoard opp_bits,
				  int stage );

void
setup_search( void );

int
disc_count( BitBoard my_bits,
		   BitBoard opp_bits,
		   int is_my_color);

int
disc_count2( int color );

void
set_ponder_move( int move );

void
clear_ponder_move( void );

int
get_ponder_move( void );

EvaluationType
create_eval_info( EvalType in_type, EvalResult in_res, int in_score,
		  double in_conf, int in_depth, int in_book );

void
adjust_wld_eval( EvaluationType *eval_info );

double
produce_compact_eval( EvaluationType eval_info );

int
select_move( int first, int list_size );

void
set_current_eval( EvaluationType eval );

EvaluationType
get_current_eval( void );

void
negate_current_eval( int negate );

#ifdef __cplusplus
}
#endif

#endif  /* __SEARCH_H_ */
