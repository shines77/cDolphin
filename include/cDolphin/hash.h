/*
   File:           hash.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines manipulating the hash table
*/

#ifndef __HASH_H_
#define __HASH_H_

#include "global.h"
#include "bitboard.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOWER_BOUND               0x01u     //  1
#define UPPER_BOUND               0x02u     //  2
#define EXACT_VALUE               0x04u     //  4
#define MIDGAME_SCORE             0x08u     //  8
#define ENDGAME_SCORE             0x10u     // 16
#define SELECTIVE                 0x20u     // 32

#define ENDGAME_MODE              TRUE
#define MIDGAME_MODE              FALSE

#define NO_HASH_MOVE              0


/* The structure returned when a hash probe resulted in a hit.
   DRAFT is the depth of the subtree beneath the node and FLAGS
   contains a flag mask (see the flag bits above). */
typedef struct tagHashEntry {
	unsigned int key1, key2;
	int eval;
	int move[4];
	short draft;
	short selectivity;
	short flags;
} HashEntry;

typedef struct tagCompactHashEntry {
	unsigned int key2;
	int eval;
	unsigned int moves;
	unsigned int key1_selectivity_flags_draft;
} CompactHashEntry;

/* The number of entries in the hash table. Always a power of 2. */
extern int g_hash_size;
extern int g_hash_mask;
extern CompactHashEntry *g_hash_table;

/* The 64-bit hash key. */
extern unsigned int g_hash1;
extern unsigned int g_hash2;

extern unsigned int g_hash_trans1;
extern unsigned int g_hash_trans2;

/* The XOR of the hash_color*, used for disk flipping. */
extern unsigned int g_hash_switch_side1;
extern unsigned int g_hash_switch_side2;

extern unsigned int g_hash_update_v1;
extern unsigned int g_hash_update_v2;

/* The 64-bit hash masks for a piece of a certain color in a
   certain position. */
extern unsigned int hash_value1[3][64];
extern unsigned int hash_value2[3][64];

/* 64-bit hash masks used when a disc is played on the board;
   the relation
     hash_put_value?[][] == hash_value?[][] ^ hash_flip_color?
   is guaranteed to hold. */

extern unsigned int hash_put_value1[3][64];
extern unsigned int hash_put_value2[3][64];

/* XORs of hash_value* - used for disk flipping. */
extern unsigned int hash_flip1[64];
extern unsigned int hash_flip2[64];

/* 64-bit hash mask for the two different sides to move. */
extern unsigned int hash_color1[3];
extern unsigned int hash_color2[3];

/* Stored 64-bit hash mask which hold the hash codes at different nodes
   in the search tree. */
extern ALIGN_PREFIX(64) unsigned int hash_stored1[MAX_SEARCH_DEPTH] ALIGN_SUFFIX(64);
extern ALIGN_PREFIX(64) unsigned int hash_stored2[MAX_SEARCH_DEPTH] ALIGN_SUFFIX(64);

extern unsigned int hash_row_value1[8][256];
extern unsigned int hash_row_value2[8][256];

extern unsigned int hash_two_row_value1[4][65536];
extern unsigned int hash_two_row_value2[4][65536];

extern unsigned int g_save_hash1, g_save_hash2;

void
hash_init( int in_hash_bits );

void
hash_resize( int new_hash_bits );

void
hash_setup( int clear, int bsrand );

#ifdef DOLPHIN_GGS

void
hash_setup_ggs( int clear, int bsrand = TRUE );

#endif

void
hash_clear_drafts( void );

void
hash_free( void );

void
hash_set_transformation( unsigned int trans1,
			 unsigned int trans2 );

void
hash_add( int reverse_mode,
	  int score,
	  int best,
	  int flags,
	  int draft,
	  int selectivity );

void
hash_add_extended( int reverse_mode,
		   int score,
		   int *best,
		   int flags,
		   int draft,
		   int selectivity );

HashEntry
hash_find( int reverse_mode );

void
hash_determine_values( const BitBoard my_bits,
					  const BitBoard opp_bits,
					  int color );

void
hash_determine_values2( const int *board, int color );

#ifdef __cplusplus
}
#endif

#endif  /* __HASH_H_ */
