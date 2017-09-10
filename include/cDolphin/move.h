/*
   File:           move.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The move generator's interface.
*/

#ifndef __MOVE_H_
#define __MOVE_H_

#include "bitboard.h"
#include "global.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The number of disks played from the initial position.
   Must match the current status of the BOARD variable. */
extern int disks_played;

/* Holds the last move made on the board for each different
   game stage. */
//extern int last_move[65];

/* The number of moves available after a certain number
   of disks played. */
extern ALIGN_PREFIX(64) int move_count[MAX_SEARCH_DEPTH] ALIGN_SUFFIX(64);

/* The actual moves available after a certain number of
   disks played. */
extern ALIGN_PREFIX(64) int move_list[MAX_SEARCH_DEPTH][64] ALIGN_SUFFIX(64);

/* Directional flip masks for all board positions. */
extern const int dir_mask[64];

/* Increments corresponding to the different flip directions */
extern const int move_offset[8];

/* The directions in which there exist valid moves. If there are N such
   directions for a square, the Nth element of the list is 0. */
extern int flip_direction[100][16];

/* Pointers to FLIPDIRECTION[][0]. */
extern int *first_flip_direction[100];

extern ALIGN_PREFIX(64) BitBoard neighborhood_mask[64] ALIGN_SUFFIX(64);

void
init_moves(void);

void
init_neighborhood(void);

int
game_in_progress(BitBoard my_bits,
                 BitBoard opp_bits,
                 int color);

void
generate_all(const BitBoard my_bits,
             const BitBoard opp_bits, int color);

int
get_move(int color);

int
move_is_valid(const BitBoard my_bits,
              const BitBoard opp_bits,
              int move);

int
move_is_valid2(int move, int color);

int
CheckFlips_wrapper(int sq,
                   BitBoard my_bits,
                   BitBoard opp_bits);

int
TestFlips_wrapper(int sq,
                  BitBoard my_bits,
                  BitBoard opp_bits);

int
GetFlipLists_wrapper(int sq,
                     BitBoard my_bits,
                     BitBoard opp_bits);

int
prepare_movelist(const BitBoard my_bits,
                 const BitBoard opp_bits,
                 BitBoard *movelist_bits,
                 int move,
                 int *need_gen_movelist);

int
prepare_movelist_adv(const BitBoard my_bits,
                     const BitBoard opp_bits,
                     BitBoard *movelist_bits,
                     int move,
                     int *need_gen_movelist,
                     int is_best_move);

int
prepare_movelist_pv(const BitBoard my_bits,
                    const BitBoard opp_bits,
                    int move);

int
make_move(BitBoard *my_bits, BitBoard *opp_bits,
          int color, int move, int update_hash);

void
unmake_move(int color, int move);

int
make_move_no_hash(BitBoard *my_bits, BitBoard *opp_bits,
                  int color, int move);

void
unmake_move_no_hash(int color, int move);

void
make_move_end(int color, int move,
              //int flipped,
              int update_hash);

void
make_move_end2(int color, int move,
               int flipped,
               int update_hash);

void
unmake_move_end(int color, int move);

#ifdef __cplusplus
}
#endif

#endif  /* __MOVE_H_ */
