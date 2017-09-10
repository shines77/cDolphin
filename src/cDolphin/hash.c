/*
   File:           hash.c

   Created:        2007-07-10

   Modified:       2013-08-10

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines manipulating the hash table.
*/

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

#include "utils.h"
#include "myrandom.h"
#include "colour.h"
#include "dolphin.h"
#include "hash.h"
#include "search.h"
#include "eval.h"

/* Substitute an old position with draft n if the new position has
   draft >= n-4 */
#define REPLACEMENT_OFFSET          4

/* An 8-bit mask to isolate the "draft" part of the hash table info. */
#define DRAFT_MASK                  0x000000FFUL

#define KEY1_MASK                   0xFF000000UL

#define SECONDARY_HASH( a )         ((a) ^ 1)

/* Global variables */

ALIGN_PREFIX(64) int g_hash_size = 0 ALIGN_SUFFIX(64);
int g_hash_mask = 0;
CompactHashEntry *g_hash_table = NULL;
unsigned int g_hash1 = 0;
unsigned int g_hash2 = 0;
unsigned int g_hash_trans1 = 0;
unsigned int g_hash_trans2 = 0;
unsigned int g_hash_switch_side1 = 0;
unsigned int g_hash_switch_side2 = 0;
unsigned int g_hash_update_v1 = 0;
unsigned int g_hash_update_v2 = 0;
unsigned int hash_value1[3][64];
unsigned int hash_value2[3][64];
unsigned int hash_put_value1[3][64];
unsigned int hash_put_value2[3][64];
unsigned int hash_flip1[64];
unsigned int hash_flip2[64];
unsigned int hash_color1[3];
unsigned int hash_color2[3];
ALIGN_PREFIX(64) unsigned int hash_stored1[MAX_SEARCH_DEPTH] ALIGN_SUFFIX(64);
ALIGN_PREFIX(64) unsigned int hash_stored2[MAX_SEARCH_DEPTH] ALIGN_SUFFIX(64);
unsigned int hash_row_value1[8][256];
unsigned int hash_row_value2[8][256];
unsigned int hash_two_row_value1[4][65536];
unsigned int hash_two_row_value2[4][65536];
unsigned int hash_diff1[MAX_SEARCH_DEPTH];
unsigned int hash_diff2[MAX_SEARCH_DEPTH];

unsigned int g_save_hash1, g_save_hash2;

/* Local variables */
static CompactHashEntry *g_org_hash_table = NULL;
static int g_hash_bits = 0;
static int g_rehash_count = 0;

/*
   INIT_HASH
   Allocate memory for the hash table.
*/

void
hash_init(int in_hash_bits) {
    g_hash_bits = in_hash_bits;
    if (g_hash_bits <= 0) {
        g_hash_bits = DEFAULT_HASH_BITS;
    }
    g_hash_size = 1 << g_hash_bits;
    g_hash_mask = g_hash_size - 1;

    // free hash first
    hash_free();

    g_org_hash_table = (CompactHashEntry *)malloc(g_hash_size * sizeof(CompactHashEntry) + 64);
    if (g_org_hash_table == NULL) {
        return;
    }
    g_hash_table = (CompactHashEntry *)ADDR_ALGIN_64BYTES(g_org_hash_table);
    g_rehash_count = 0;
}

/*
  RESIZE_HASH
  Changes the size of the hash table.
*/

void
hash_resize(int new_hash_bits) {
    hash_free();
    hash_init(new_hash_bits);
    hash_setup(TRUE, TRUE);
}

/*
   FREE_HASH
   Remove the hash table.
*/

void
hash_free(void) {
    if (g_org_hash_table) {
        free(g_org_hash_table);
        g_org_hash_table = NULL;
        g_hash_table = NULL;
    }
}

/*
  HASH_CLEAR_DRAFTS
  Resets the draft information for all entries in the hash table.
*/

void
hash_clear_drafts(void) {
    int i;

    for (i = 0; i < g_hash_size; i++) { /* Set the draft to 0 */
        g_hash_table[i].key1_selectivity_flags_draft &= ~0x0FF;
    }
}

/*
  POPCOUNT
*/
static unsigned int
popcount(unsigned int b) {
    unsigned int n;

    for (n = 0; b != 0; n++, b &= (b - 1))
        ;

    return n;
}

/*
  GET_CLOSENESS
  Returns the closeness between the 64-bit integers (a0,a1) and (b0,b1).
  A closeness of 0 means that 32 bits differ.
*/
static unsigned int
get_closeness(unsigned int a0, unsigned int a1,
    unsigned int b0, unsigned int b1) {
    return abs(popcount(a0 ^ b0) + popcount(a1 ^ b1) - 32);
}

/*
   SETUP_HASH
   Determine randomized hash masks.
*/

void
hash_setup(int clear, int bsrand) {
    int i;
    int rand_index;
    int j, pos, mask;
    int first_bit;
    unsigned int scan_bit;
    unsigned int hash_tmp1, hash_tmp2;
    const unsigned int max_pair_closeness = 10;		// 10
    const unsigned int max_zero_closeness = 9;		// 9
    unsigned int closeness;
    unsigned int random_pair[130][2];
    time_t timer;
    //int count;

    /* Seed the random-number generator with current time so that
     * the numbers will be different every time we run.
     */
    if (bsrand) {
        srand((unsigned)time(NULL));
        time(&timer);
        my_srandom((int)timer);
    }

    if (g_hash_table == NULL) {
        if (g_hash_bits == 0)
            g_hash_bits = DEFAULT_HASH_BITS;
        hash_init(g_hash_bits);
    }

    if (clear) {
        for (i = 0; i < g_hash_size; i++) {
            g_hash_table[i].key1_selectivity_flags_draft &= ~DRAFT_MASK;
            g_hash_table[i].key2 = 0;
        }
    }

    //printf("\r\n");

    //count = 0;
    rand_index = 0;
    while (rand_index < 130) {
TRY_AGAIN2:
        //random_pair[rand_index][0] = (((unsigned int)my_random()) << 3) + (((unsigned int)my_random()) >> 2);
        //random_pair[rand_index][1] = (((unsigned int)my_random()) << 3) + (((unsigned int)my_random()) >> 2);

        random_pair[rand_index][0] = ((((unsigned int)my_random()) << 16) & 0xFFFF0000UL)
            | ((((unsigned int)my_random()) >> 15) & 0x0000FFFFUL);
        random_pair[rand_index][1] = ((((unsigned int)my_random()) << 16) & 0xFFFF0000UL)
            | ((((unsigned int)my_random()) >> 15) & 0x0000FFFFUL);

        //count++;
        //printf("\x08\x08\x08\x08\x08\x08\x08\x08\x08%d", count);
        //fflush(stdout);
        //Sleep(200);

        closeness = get_closeness(random_pair[rand_index][0], random_pair[rand_index][1], 0, 0);
        if (closeness > max_zero_closeness) {
            goto TRY_AGAIN2;
        }
        for (i = 0; i < rand_index; i++) {
            closeness = get_closeness(random_pair[rand_index][0], random_pair[rand_index][1],
                random_pair[i][0], random_pair[i][1]);
            if (closeness > max_pair_closeness)
                goto TRY_AGAIN2;
            closeness = get_closeness(random_pair[rand_index][0], random_pair[rand_index][1],
                random_pair[i][1], random_pair[i][0]);
            if (closeness > max_pair_closeness)
                goto TRY_AGAIN2;
        }
        rand_index++;
    }

    //printf("%d", count);
    //printf("\r\n");

    //printf("\x08\x08\x08\x08\x08\x08\x08\x08\x08%d", count);
    //fflush(stdout);

    rand_index = 0;

    for (i = 0; i < 64; i++) {
        hash_value1[CHESS_BLACK][i] = 0;
        hash_value2[CHESS_BLACK][i] = 0;
        hash_value1[CHESS_WHITE][i] = 0;
        hash_value2[CHESS_WHITE][i] = 0;
    }
    for (i = 0; i < 64; i++) {
        hash_value1[CHESS_BLACK][i] = random_pair[rand_index][0];
        hash_value2[CHESS_BLACK][i] = random_pair[rand_index][1];
        rand_index++;
        hash_value1[CHESS_WHITE][i] = random_pair[rand_index][0];
        hash_value2[CHESS_WHITE][i] = random_pair[rand_index][1];
        rand_index++;
    }

    for (i = 0; i < 64; i++) {
        hash_flip1[i] = hash_value1[CHESS_BLACK][i] ^ hash_value1[CHESS_WHITE][i];
        hash_flip2[i] = hash_value2[CHESS_BLACK][i] ^ hash_value2[CHESS_WHITE][i];
    }

    hash_color1[CHESS_BLACK] = random_pair[rand_index][0];
    hash_color2[CHESS_BLACK] = random_pair[rand_index][1];
    rand_index++;
    hash_color1[CHESS_WHITE] = random_pair[rand_index][0];
    hash_color2[CHESS_WHITE] = random_pair[rand_index][1];
    rand_index++;

    g_hash_switch_side1 = hash_color1[CHESS_BLACK] ^ hash_color1[CHESS_WHITE];
    g_hash_switch_side2 = hash_color2[CHESS_BLACK] ^ hash_color2[CHESS_WHITE];

    for (i = 0; i < 64; i++) {
        hash_put_value1[CHESS_BLACK][i] = hash_value1[CHESS_BLACK][i] ^ g_hash_switch_side1;
        hash_put_value2[CHESS_BLACK][i] = hash_value2[CHESS_BLACK][i] ^ g_hash_switch_side2;
        hash_put_value1[CHESS_WHITE][i] = hash_value1[CHESS_WHITE][i] ^ g_hash_switch_side1;
        hash_put_value2[CHESS_WHITE][i] = hash_value2[CHESS_WHITE][i] ^ g_hash_switch_side2;
    }

    // disc row hash values
    for (i = 0; i < 8; i++) {
        for (mask = 0; mask < 256; mask++) {
            hash_tmp1 = 0;
            hash_tmp2 = 0;
            pos = i * 8;
            first_bit = 1;
            scan_bit = 1;
            for (j = 0; j < 8; j++) {
                if ((mask & scan_bit) == scan_bit) {
                    if (first_bit) {
                        hash_tmp1 = hash_flip1[pos];
                        hash_tmp2 = hash_flip2[pos];
                        first_bit = 0;
                    }
                    else {
                        hash_tmp1 ^= hash_flip1[pos];
                        hash_tmp2 ^= hash_flip2[pos];
                    }
                }
                pos++;
                scan_bit <<= 1;
            }
            hash_row_value1[i][mask] = hash_tmp1;
            hash_row_value2[i][mask] = hash_tmp2;
        }
    }

    // disc row hash values
    for (i = 0; i < 4; i++) {
        for (mask = 0; mask < 65536; mask++) {
            hash_tmp1 = 0;
            hash_tmp2 = 0;
            pos = i * 16;
            first_bit = 1;
            scan_bit = 1;
            for (j = 0; j < 16; j++) {
                if ((mask & scan_bit) == scan_bit) {
                    if (first_bit) {
                        hash_tmp1 = hash_flip1[pos];
                        hash_tmp2 = hash_flip2[pos];
                        first_bit = 0;
                    }
                    else {
                        hash_tmp1 ^= hash_flip1[pos];
                        hash_tmp2 ^= hash_flip2[pos];
                    }
                }
                pos++;
                scan_bit <<= 1;
            }
            hash_two_row_value1[i][mask] = hash_tmp1;
            hash_two_row_value2[i][mask] = hash_tmp2;
        }
    }
}

#ifdef DOLPHIN_GGS

/*
   SETUP_HASH_GGS
   Determine randomized hash masks.
*/

void
hash_setup_ggs(int clear, int bsrand) {
    int i;
    if (g_hash_table == NULL) {
        if (g_hash_bits == 0)
            g_hash_bits = DEFAULT_HASH_BITS;
        hash_init(g_hash_bits);
    }

    if (clear) {
        for (i = 0; i < g_hash_size; i++) {
            g_hash_table[i].key1_selectivity_flags_draft &= ~DRAFT_MASK;
            g_hash_table[i].key2 = 0;
        }
    }
}

#endif

/*
   DETERMINE_HASH_VALUES
   Calculates the hash codes for the given board position.
*/

INLINE
void
hash_determine_values(const BitBoard my_bits,
    const BitBoard opp_bits, int color) {
    int pos;
    int opp_color = OPP_COLOR(color);

    g_hash1 = 0;
    g_hash2 = 0;
    for (pos = 0; pos < 64; pos++) {
        if (((my_bits.low & square_mask[pos].low) != 0) ||
            ((my_bits.high & square_mask[pos].high) != 0)) {
            g_hash1 ^= hash_value1[color][pos];
            g_hash2 ^= hash_value2[color][pos];
        }
        else {
            if (((opp_bits.low & square_mask[pos].low) != 0) ||
                ((opp_bits.high & square_mask[pos].high) != 0)) {
                g_hash1 ^= hash_value1[opp_color][pos];
                g_hash2 ^= hash_value2[opp_color][pos];
            }
        }
    }
    g_hash1 ^= hash_color1[color];
    g_hash2 ^= hash_color2[color];
}

/*
   DETERMINE_HASH_VALUES2
   Calculates the hash codes for the given board position.
*/

void
hash_determine_values2(const int *board, int color) {
    int i, j;

    g_hash1 = 0;
    g_hash2 = 0;
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            int pos = 8 * i + j;
            switch (board[pos]) {
            case CHESS_BLACK:
                g_hash1 ^= hash_value1[CHESS_BLACK][pos];
                g_hash2 ^= hash_value2[CHESS_BLACK][pos];
                break;

            case CHESS_WHITE:
                g_hash1 ^= hash_value1[CHESS_WHITE][pos];
                g_hash2 ^= hash_value2[CHESS_WHITE][pos];
                break;

            default:
                break;
            }
        }
    }
    g_hash1 ^= hash_color1[color];
    g_hash2 ^= hash_color2[color];
}

/*
WIDE_TO_COMPACT
Convert the easily readable representation to the more
compact one actually stored in the hash table.
*/

INLINE CompactHashEntry
wide_to_compact(HashEntry entry) {
    CompactHashEntry compact_entry;

    compact_entry.key2 = entry.key2;
    compact_entry.eval = entry.eval;
    compact_entry.moves = entry.move[0] + (entry.move[1] << 8) +
        (entry.move[2] << 16) + (entry.move[3] << 24);
    compact_entry.key1_selectivity_flags_draft =
        (entry.key1 & KEY1_MASK) + (entry.selectivity << 16) +
        (entry.flags << 8) + entry.draft;

    return compact_entry;
}

/*
COMPACT_TO_WIDE
Expand the compact internal representation of entries
in the hash table to something more usable.
*/

INLINE HashEntry
compact_to_wide(CompactHashEntry compact_entry) {
    HashEntry entry;

    entry.key2 = compact_entry.key2;
    entry.eval = compact_entry.eval;
    entry.move[0] = compact_entry.moves & 255;
    entry.move[1] = (compact_entry.moves >> 8) & 255;
    entry.move[2] = (compact_entry.moves >> 16) & 255;
    entry.move[3] = (compact_entry.moves >> 24) & 255;
    entry.key1 = compact_entry.key1_selectivity_flags_draft & KEY1_MASK;
    entry.selectivity =
        (compact_entry.key1_selectivity_flags_draft & 0x00ffffff) >> 16;
    entry.flags =
        (compact_entry.key1_selectivity_flags_draft & 0x0000ffff) >> 8;
    entry.draft =
        (compact_entry.key1_selectivity_flags_draft & 0x000000ff);

    return entry;
}

/*
  SET_HASH_TRANSFORMATION
  Specify the hash code transformation masks. Changing these masks
  is the poor man's way to achieve the effect of clearing the hash
  table.
*/

void
hash_set_transformation(unsigned int trans1, unsigned int trans2) {
    g_hash_trans1 = trans1;
    g_hash_trans2 = trans2;
}

/*
   ADD_HASH
   Add information to the hash table. Two adjacent positions are tried
   and the most shallow search is replaced.
*/

INLINE void
hash_add(int reverse_mode,
    int score,
    int best,
    int flags,
    int draft,
    int selectivity) {
    int old_draft;
    int change_encouragment;
    unsigned int index, index1, index2;
    unsigned int code1, code2;
    HashEntry entry;

    assert(abs(score) != SEARCH_ABORT);

    if (reverse_mode) {
        code1 = g_hash2 ^ g_hash_trans2;
        code2 = g_hash1 ^ g_hash_trans1;
        //code1 = g_hash2;
        //code2 = g_hash1;
    }
    else {
        code1 = g_hash1 ^ g_hash_trans1;
        code2 = g_hash2 ^ g_hash_trans2;
        //code1 = g_hash1;
        //code2 = g_hash2;
    }

    index1 = code1 & g_hash_mask;
    index2 = SECONDARY_HASH(index1);
    if ((g_hash_table[index1].key2 == code2) &&
        (((g_hash_table[index1].key1_selectivity_flags_draft ^ code1) & KEY1_MASK) == 0))
        index = index1;
    else {
        if ((g_hash_table[index2].key2 == code2) &&
            (((g_hash_table[index2].key1_selectivity_flags_draft ^code1) & KEY1_MASK) == 0))
            index = index2;
        else {
            if ((g_hash_table[index1].key1_selectivity_flags_draft & DRAFT_MASK) <=
                (g_hash_table[index2].key1_selectivity_flags_draft & DRAFT_MASK))
                index = index1;
            else
                index = index2;
        }
    }

    old_draft = g_hash_table[index].key1_selectivity_flags_draft & DRAFT_MASK;

    if (flags & EXACT_VALUE)  /* Exact scores are potentially more useful */
        change_encouragment = 2;
    else
        change_encouragment = 0;
    if (g_hash_table[index].key2 == code2) {
        if (old_draft > draft + change_encouragment + 2)
            return;
    }
    else if (old_draft > draft + change_encouragment + REPLACEMENT_OFFSET) {
        return;
    }

    entry.key1 = code1;
    entry.key2 = code2;
    entry.eval = score;
    entry.move[0] = best;
    entry.move[1] = D4;      // D4, _NULL_MOVE
    entry.move[2] = D4;
    entry.move[3] = D4;
    entry.flags = (short)flags;
    entry.draft = (short)draft;
    entry.selectivity = (short)selectivity;
    g_hash_table[index] = wide_to_compact(entry);
}

/*
   ADD_HASH_EXTENDED
   Add information to the hash table. Two adjacent positions are tried
   and the most shallow search is replaced.
*/

INLINE void
hash_add_extended(int reverse_mode, int score, int *best, int flags,
    int draft, int selectivity) {
    int i;
    int old_draft;
    int change_encouragment;
    unsigned int index, index1, index2;
    unsigned int code1, code2;
    HashEntry entry;

    if (reverse_mode) {
        code1 = g_hash2 ^ g_hash_trans2;
        code2 = g_hash1 ^ g_hash_trans1;
        //code1 = g_hash2;
        //code2 = g_hash1;
    }
    else {
        code1 = g_hash1 ^ g_hash_trans1;
        code2 = g_hash2 ^ g_hash_trans2;
        //code1 = g_hash1;
        //code2 = g_hash2;
    }

    index1 = code1 & g_hash_mask;
    index2 = SECONDARY_HASH(index1);
    if ((g_hash_table[index1].key2 == code2) &&
        (((g_hash_table[index1].key1_selectivity_flags_draft ^ code1) & KEY1_MASK) == 0)) {
        index = index1;
    }
    else {
        if ((g_hash_table[index2].key2 == code2) &&
            (((g_hash_table[index2].key1_selectivity_flags_draft ^ code1) & KEY1_MASK) == 0)) {
            index = index2;
        }
        else {
            if ((g_hash_table[index1].key1_selectivity_flags_draft & DRAFT_MASK) <=
                (g_hash_table[index2].key1_selectivity_flags_draft & DRAFT_MASK))
                index = index1;
            else
                index = index2;
        }
    }

    old_draft = g_hash_table[index].key1_selectivity_flags_draft & DRAFT_MASK;

    if (flags & EXACT_VALUE)  /* Exact scores are potentially more useful */
        change_encouragment = 2;
    else
        change_encouragment = 0;
    if (g_hash_table[index].key2 == code2) {
        if (old_draft > draft + change_encouragment + 2)
            return;
    }
    else if (old_draft > draft + change_encouragment + REPLACEMENT_OFFSET) {
        return;
    }

    entry.key1 = code1;
    entry.key2 = code2;
    entry.eval = score;
    for (i = 0; i < 4; i++)
        entry.move[i] = best[i];
    entry.flags = (short)flags;
    entry.draft = (short)draft;
    entry.selectivity = (short)selectivity;
    g_hash_table[index] = wide_to_compact(entry);
}

/*
   FIND_HASH
   Search the hash table for the current position. The two possible
   hash table positions are probed.
*/

INLINE HashEntry
hash_find(int reverse_mode) {
    int index1, index2;
    unsigned int code1, code2;
    HashEntry entry;

    if (reverse_mode) {
        code1 = g_hash2 ^ g_hash_trans2;
        code2 = g_hash1 ^ g_hash_trans1;
    }
    else {
        code1 = g_hash1 ^ g_hash_trans1;
        code2 = g_hash2 ^ g_hash_trans2;
    }

    index1 = code1 & g_hash_mask;
    index2 = SECONDARY_HASH(index1);
    if (g_hash_table[index1].key2 == code2) {
        if ((g_hash_table[index1].key1_selectivity_flags_draft & KEY1_MASK) ==
            (code1 & KEY1_MASK)) {
            return compact_to_wide(g_hash_table[index1]);
        }
    }
    else if ((g_hash_table[index2].key2 == code2) &&
        ((g_hash_table[index2].key1_selectivity_flags_draft & KEY1_MASK) ==
        (code1 & KEY1_MASK))) {
        return compact_to_wide(g_hash_table[index2]);
    }

    entry.draft = NO_HASH_MOVE;
    entry.flags = UPPER_BOUND;
    //entry.selectivity = 0;
    entry.eval = INFINITE_EVAL;
    entry.move[0] = D4;     // D4, _NULL_MOVE
    entry.move[1] = D4;
    entry.move[2] = D4;
    entry.move[3] = D4;
    return entry;
}
