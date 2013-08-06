/*
   File:           move.cpp

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Search routines designated to be used in the
                   midgame phase of the game.
*/

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "global.h"
#include "colour.h"
#include "board.h"
#include "bitboard.h"
#include "hash.h"
#include "move.h"
#include "search.h"
#include "bitbmob.h"
#include "bitbchk.h"
#include "bitbtest.h"
#include "bitbtest1.h"
#include "bitbtest2.h"
#include "bitbtest4.h"
#include "bitbtest5.h"

/* Global variables */

int disks_played = 0;
ALIGN_PREFIX(64) int move_count[MAX_SEARCH_DEPTH] ALIGN_SUFFIX(64);
ALIGN_PREFIX(64) int move_list[MAX_SEARCH_DEPTH][64] ALIGN_SUFFIX(64);
int *first_flip_direction[100];
int flip_direction[100][16];   /* 100 * 9 used */
int **first_flipped_disc[100];
int *flipped_disc[100][8];
const int dir_mask[64] = {
	81,  81,  87,  87,  87,  87,  22,  22,
	81,  81,  87,  87,  87,  87,  22,  22,
	121, 121, 255, 255, 255, 255, 182, 182,
	121, 121, 255, 255, 255, 255, 182, 182,
	121, 121, 255, 255, 255, 255, 182, 182,
	121, 121, 255, 255, 255, 255, 182, 182,
	41,  41,  171, 171, 171, 171, 162, 162,
	41,  41,  171, 171, 171, 171, 162, 162
};
const int move_offset[8] = { 1, -1, 7, -7, 8, -8, 9, -9 };

ALIGN_PREFIX(64) BitBoard neighborhood_mask[64];

/* Local variables */

//static int flip_count[65];
static int sweep_status[MAX_SEARCH_DEPTH];

/*
  INIT_MOVES
  Initialize the move generation subsystem.
*/

void
init_moves( void ) {
	int i, j, k;
	int pos;
	int feasible;

	for ( i = 0; i < 8; i++ ) {
		for ( j = 0; j < 8; j++ ) {
			pos = 8 * i + j;
			for ( k = 0; k <= 8; k++ )
				flip_direction[pos][k] = 0;
			feasible = 0;
			for ( k = 0; k < 8; k++ ) {
				if ( dir_mask[pos] & (1 << k) ) {
					flip_direction[pos][feasible] = move_offset[k];
					feasible++;
				}
			}
			first_flip_direction[pos] = &flip_direction[pos][0];
		}
	}
}

/*
   INIT_NEIGHBORHOOD
   Calculate the neighborhood masks
*/
void
init_neighborhood( void ) {
	int pos;
	int dir_shift[8] = {1, -1, 7, -7, 8, -8, 9, -9};

	for ( pos = 0; pos < 64; pos++ ) {
		/* Create the neighborhood mask for the square POS */
		unsigned int k;

		neighborhood_mask[pos].low = 0;
		neighborhood_mask[pos].high = 0;

		for ( k = 0; k < 8; k++ ) {
			if ( dir_mask[pos] & (1 << k) ) {
				unsigned int neighbor = pos + dir_shift[k];
				if ( neighbor < 32 )
					neighborhood_mask[pos].low |= (1 << neighbor);
				else
					neighborhood_mask[pos].high |= (1 << (neighbor - 32));
			}
		}
	}
}

/*
   RESET_GENERATION
   Prepare for move generation at a given level in the tree.
*/

INLINE static void
reset_generation( int color ) {
	sweep_status[disks_played] = 0;
}

/*
   GAME_IN_PROGRESS
   Determines if any of the players has a valid move.
*/

int
game_in_progress( BitBoard my_bits, BitBoard opp_bits, int color ) {
	int black_count, white_count;

	if ( color == CHESS_BLACK ) {
		generate_all( my_bits, opp_bits, CHESS_BLACK );
		black_count = move_count[disks_played];
		generate_all( opp_bits, my_bits, CHESS_WHITE );
		white_count = move_count[disks_played];
	}
	else {
		generate_all( opp_bits, my_bits, CHESS_BLACK );
		black_count = move_count[disks_played];
		generate_all( my_bits, opp_bits, CHESS_WHITE );
		white_count = move_count[disks_played];
	}

	return (black_count > 0) || (white_count > 0);
}

/*
   GENERATE_ALL
   Generates a list containing all the moves possible in a position.
*/

INLINE void
generate_all( const BitBoard my_bits,
			 const BitBoard opp_bits, int color ) {
	int count, pos;
	BitPosition movelist_pos;
	BitBoard *movelist_bits;
	movelist_bits = (BitBoard *)ADDR_ALGIN_8BYTES(&movelist_pos);

	reset_generation( color );
	count = 0;

	// generate movelist
	bitboard_gen_movelist(my_bits, opp_bits, movelist_bits);
	if ( (movelist_bits->low != 0) || (movelist_bits->high != 0) ) {
		for ( pos = 0; pos < 64; pos++ ) {
			if ( ((movelist_bits->low & square_mask[pos].low) != 0) ||
				((movelist_bits->high & square_mask[pos].high) != 0) ) {
				move_list[disks_played][count] = pos;
				count++;
			}
		}
	}

	move_list[disks_played][count] = CHESS_ILLEGAL;
	move_count[disks_played] = count;
}

/*
   GET_MOVE
   Prompts the user to enter a move and checks if the move is legal.
*/

int
get_move( int color ) {
	char buffer[255];
	int ready = 0;
	int curr_move;

	while ( !ready ) {
		if ( color == CHESS_BLACK )
			//printf( "%s: ", BLACK_PROMPT );
			printf( "%s: ", "Black move" );
		else
			//printf( "%s: ", WHITE_PROMPT );
			printf( "%s: ", "White move" );
		scanf( "%s", buffer );
		curr_move = atoi( buffer );
		ready = move_is_valid2( curr_move, color );
		if ( !ready ) {
			curr_move = (buffer[0] - 'a' + 1) + 10 * (buffer[1] - '0');
			ready = move_is_valid2( curr_move, color );
		}
	}

	return curr_move;
}

/*
   MOVE_IS_VALID
   Determines if a move is legal.
*/
INLINE int
move_is_valid( const BitBoard my_bits,
					const BitBoard opp_bits,
					int move ) {
	int flip_count;

	///*
	if ( (move == D4) || (move > H8) || (move < A1) ||
		(((square_mask[move].low & my_bits.low) != 0) ||
			((square_mask[move].high & my_bits.high) != 0) ||
			((square_mask[move].low & opp_bits.low) != 0) ||
			((square_mask[move].high & opp_bits.high) != 0)) )
		return FALSE;
	//*/

	//BitBoard fliplist_bits;
	if ( ((neighborhood_mask[move].low & opp_bits.low) |
		(neighborhood_mask[move].high & opp_bits.high)) != 0 ) {
		flip_count = CheckFlips_wrapper( move, my_bits, opp_bits );
		//flip_count = GetFlipLists_wrapper( move, my_bits, opp_bits );
		//flip_count = GetFlipLists_MMX( my_bits, opp_bits, &fliplist_bits, move );
		return (flip_count > 0);
	}
	else
		return 0;
}

/*
   MOVE_IS_VALID2
   Determines if a move is legal.
*/
INLINE int
move_is_valid2( int move, int color ) {
	int i, pos, count;

	if ( (move < A1) || (move > H8) || (board[move] != CHESS_EMPTY) )
		return FALSE;

	for ( i = 0; i < 8; i++ ) {
		if ( dir_mask[move] & (1 << i) ) {
			for ( pos = move + move_offset[i], count = 0;
				board[pos] == OPP_COLOR( color ); pos += move_offset[i], count++ )
				;
			if ( board[pos] == color ) {
				if ( count >= 1 )
					return TRUE;
			}
		}
	}

	return FALSE;
}

/*
  CHECKFLIPS_WRAPPER
  Checks if SQ is a valid move by
  (1) verifying that there exists a neighboring opponent disc,
  (2) verifying that the move flips some disc.
*/

INLINE int
CheckFlips_wrapper( int sq,
				   BitBoard my_bits,
				   BitBoard opp_bits ) {
	int flipped;

	//flipped = CheckFlips_bitboard[sq]( my_bits, opp_bits );
	///*
	if ( ((neighborhood_mask[sq].low & opp_bits.low) |
		(neighborhood_mask[sq].high & opp_bits.high)) != 0 )
		flipped = CheckFlips_bitboard[sq]( my_bits, opp_bits );
	else
		flipped = 0;
	//*/

	return flipped;
}

/*
  CHECKFLIPS_WRAPPER
  Checks if SQ is a valid move by
  (1) verifying that there exists a neighboring opponent disc,
  (2) verifying that the move flips some disc.
*/

INLINE int
TestFlips_wrapper( int sq,
				  BitBoard my_bits,
				  BitBoard opp_bits ) {
	int flipped;

	/*
	if ( (sq == D4) || (sq > H8) || (sq < A1) ||
		(((square_mask[sq].low & my_bits.low) != 0) ||
			((square_mask[sq].high & my_bits.high) != 0) ||
			((square_mask[sq].low & opp_bits.low) != 0) ||
			((square_mask[sq].high & opp_bits.high) != 0)) )
		return 0;
	//*/

	//flipped = TestFlips_bitboard[sq]( my_bits, opp_bits );
	///*
	if ( ((neighborhood_mask[sq].low & opp_bits.low) |
		(neighborhood_mask[sq].high & opp_bits.high)) != 0 ) {
		//flipped = TestFlips_bitboard4[sq]( my_bits, opp_bits );
		flipped = TestFlips_bitboard1[sq]( my_bits.high, my_bits.low, opp_bits.high, opp_bits.low );
	}
	else
		flipped = 0;
	//*/

	return flipped;
}

/*
  GETFLIPLISTS_WRAPPER
  Checks if SQ is a valid move by
  (1) verifying that there exists a neighboring opponent disc,
  (2) verifying that the move flips some disc.
*/

INLINE int
GetFlipLists_wrapper( int sq,
					 BitBoard my_bits,
					 BitBoard opp_bits ) {
	int flipped;

	//flipped = TestFlips_bitboard[sq]( my_bits, opp_bits );
	///*
	if ( ((neighborhood_mask[sq].low & opp_bits.low) |
		(neighborhood_mask[sq].high & opp_bits.high)) != 0 ) {
		//flipped = TestFlips_bitboard4[sq]( my_bits, opp_bits );
		flipped = TestFlips_bitboard1[sq]( my_bits.high, my_bits.low, opp_bits.high, opp_bits.low );
	}
	else
		flipped = 0;
	//*/

	return flipped;
}

/*
   CAL_FLIP_DISC_HASH
   Calculate flip disc hash value.
*/

INLINE void
cal_flip_disc_hash0(const BitBoard fliplist_bits, int color) {
	register unsigned int flip_low;
	register unsigned int flip_high;
	register unsigned long mask;

	flip_low = fliplist_bits.low;
	flip_high = fliplist_bits.high;

	g_hash_update_v1 = 0;
	g_hash_update_v2 = 0;

	mask = flip_low & 0x0000fffful;
	if ( mask ) {
		g_hash_update_v1 ^= hash_two_row_value1[0][mask];
		g_hash_update_v2 ^= hash_two_row_value2[0][mask];
	}

	mask = (flip_low & 0xffff0000ul);
	if ( mask ) {
		mask >>= 16;
		g_hash_update_v1 ^= hash_two_row_value1[1][mask];
		g_hash_update_v2 ^= hash_two_row_value2[1][mask];
	}

	mask = (flip_high & 0x0000fffful);
	if ( mask ) {
		g_hash_update_v1 ^= hash_two_row_value1[2][mask];
		g_hash_update_v2 ^= hash_two_row_value2[2][mask];
	}

	mask = (flip_high & 0xffff0000ul);
	if ( mask ) {
		mask >>= 16;
		g_hash_update_v1 ^= hash_two_row_value1[3][mask];
		g_hash_update_v2 ^= hash_two_row_value2[3][mask];
	}
}

/*
   CAL_FLIP_DISC_HASH
   Calculate flip disc hash value.
*/

INLINE void
cal_flip_disc_hash(const BitBoard fliplist_bits, int color) {
	register unsigned int flip_low;
	register unsigned int flip_high;
	register unsigned long mask;

	flip_low = fliplist_bits.low;
	flip_high = fliplist_bits.high;

	g_hash_update_v1 = 0;
	g_hash_update_v2 = 0;

	//if ( (flip_low & 0x0000fffful) != 0 ) {
		mask = flip_low & 0x000000fful;
		if ( mask ) {
			g_hash_update_v1 ^= hash_row_value1[0][mask];
			g_hash_update_v2 ^= hash_row_value2[0][mask];
		}

		mask = (flip_low & 0x0000ff00ul);
		if ( mask ) {
			mask >>= 8;
			g_hash_update_v1 ^= hash_row_value1[1][mask];
			g_hash_update_v2 ^= hash_row_value2[1][mask];
		}
	//}

	//if ( (flip_low & 0xffff0000ul) != 0 ) {
		mask = (flip_low & 0x00ff0000ul);
		//if ( mask ) {
			mask >>= 16;
			g_hash_update_v1 ^= hash_row_value1[2][mask];
			g_hash_update_v2 ^= hash_row_value2[2][mask];
		//}

		mask = (flip_low & 0xff000000ul);
		//if ( mask ) {
			mask >>= 24;
			g_hash_update_v1 ^= hash_row_value1[3][mask];
			g_hash_update_v2 ^= hash_row_value2[3][mask];
		//}
	//}

	//if ( (flip_high & 0x0000fffful) != 0 ) {
		mask = flip_high & 0x000000fful;
		//if ( mask ) {
			g_hash_update_v1 ^= hash_row_value1[4][mask];
			g_hash_update_v2 ^= hash_row_value2[4][mask];
		//}

		mask = (flip_high & 0x0000ff00ul);
		//if ( mask ) {
			mask >>= 8;
			g_hash_update_v1 ^= hash_row_value1[5][mask];
			g_hash_update_v2 ^= hash_row_value2[5][mask];
		//}
	//}

	//if ( (flip_high & 0xffff0000ul) != 0 ) {
		mask = (flip_high & 0x00ff0000ul);
		if ( mask ) {
			mask >>= 16;
			g_hash_update_v1 ^= hash_row_value1[6][mask];
			g_hash_update_v2 ^= hash_row_value2[6][mask];
		}

		mask = (flip_high & 0xff000000ul);
		if ( mask ) {
			mask >>= 24;
			g_hash_update_v1 ^= hash_row_value1[7][mask];
			g_hash_update_v2 ^= hash_row_value2[7][mask];
		}
	//}
}

/*
   CAL_FLIP_DISC_HASH_END
   Calculate flip disc hash value.
*/

INLINE void
cal_flip_disc_hash_end(const BitBoard fliplist_bits, int color) {
	register unsigned int flip_low;
	register unsigned int flip_high;
	register unsigned long mask;

	flip_low = fliplist_bits.low;
	flip_high = fliplist_bits.high;

	g_hash_update_v1 = 0;
	g_hash_update_v2 = 0;

	//if ( (flip_low & 0x0000fffful) != 0 ) {
		mask = flip_low & 0x000000fful;
		g_hash_update_v1 ^= hash_row_value1[0][mask];
		g_hash_update_v2 ^= hash_row_value2[0][mask];

		mask = (flip_low & 0x0000ff00ul);
		mask >>= 8;
		g_hash_update_v1 ^= hash_row_value1[1][mask];
		g_hash_update_v2 ^= hash_row_value2[1][mask];
	//}

	//if ( (flip_low & 0xffff0000ul) != 0 ) {
		mask = (flip_low & 0x00ff0000ul);
		mask >>= 16;
		g_hash_update_v1 ^= hash_row_value1[2][mask];
		g_hash_update_v2 ^= hash_row_value2[2][mask];

		mask = (flip_low & 0xff000000ul);
		mask >>= 24;
		g_hash_update_v1 ^= hash_row_value1[3][mask];
		g_hash_update_v2 ^= hash_row_value2[3][mask];
	//}

	//if ( (flip_high & 0x0000fffful) != 0 ) {
		mask = flip_high & 0x000000fful;
		g_hash_update_v1 ^= hash_row_value1[4][mask];
		g_hash_update_v2 ^= hash_row_value2[4][mask];

		mask = (flip_high & 0x0000ff00ul);
		mask >>= 8;
		g_hash_update_v1 ^= hash_row_value1[5][mask];
		g_hash_update_v2 ^= hash_row_value2[5][mask];
	//}

	//if ( (flip_high & 0xffff0000ul) != 0 ) {
		mask = (flip_high & 0x00ff0000ul);
		mask >>= 16;
		g_hash_update_v1 ^= hash_row_value1[6][mask];
		g_hash_update_v2 ^= hash_row_value2[6][mask];

		mask = (flip_high & 0xff000000ul);
		mask >>= 24;
		g_hash_update_v1 ^= hash_row_value1[7][mask];
		g_hash_update_v2 ^= hash_row_value2[7][mask];
	//}
}

/*
   CAL_FLIP_DISC_HASH2
   Calculate flip disc hash value.
*/

INLINE void
cal_flip_disc_hash2(const BitBoard fliplist_bits, int color) {

	//register unsigned long flip_low;
	//register unsigned long flip_high;
	int i;
	unsigned long mask;

	//flip_low = fliplist_bits.low;
	//flip_high = fliplist_bits.high;

	g_hash_update_v1 = 0;
	g_hash_update_v2 = 0;

	if ( fliplist_bits.low != 0 ) {
		// bits 0-15
		//if ( (fliplist_bits.low & 0x0000fffful) != 0 ) {
			// bits 0-7
			if ( (fliplist_bits.low & 0x000000fful) != 0 ) {
				mask = 0x00000001ul;
				for ( i = 0; i < 8; i++, mask <<= 1 ) {
					if ( fliplist_bits.low & mask ) {
						g_hash_update_v1 ^= hash_flip1[i];
						g_hash_update_v2 ^= hash_flip2[i];
					}
				}
			}
			// bits 8-15
			if ( (fliplist_bits.low & 0x0000ff00ul) != 0 ) {
				mask = 0x00000100ul;
				for ( i = 8; i < 16; i++, mask <<= 1 ) {
					if ( fliplist_bits.low & mask ) {
						g_hash_update_v1 ^= hash_flip1[i];
						g_hash_update_v2 ^= hash_flip2[i];
					}
				}
			}
		//}
		// bits 16-31
		//if ( (fliplist_bits.low & 0xffff0000ul) != 0 ) {
			// bits 16-23
			if ( (fliplist_bits.low & 0x00ff0000ul) != 0 ) {
				mask = 0x00010000ul;
				for ( i = 16; i < 24; i++, mask <<= 1 ) {
					if ( fliplist_bits.low & mask ) {
						g_hash_update_v1 ^= hash_flip1[i];
						g_hash_update_v2 ^= hash_flip2[i];
					}
				}
			}
			// bits 24-31
			if ( (fliplist_bits.low & 0xff000000ul) != 0 ) {
				mask = 0x01000000ul;
				for ( i = 24; i < 32; i++, mask <<= 1 ) {
					if ( fliplist_bits.low & mask ) {
						g_hash_update_v1 ^= hash_flip1[i];
						g_hash_update_v2 ^= hash_flip2[i];
					}
				}
			}
		//}
	}

	if ( fliplist_bits.high != 0 ) {
		// bits 32-47
		//if ( (fliplist_bits.high & 0x0000fffful) != 0 ) {
			// bits 32-39
			if ( (fliplist_bits.high & 0x000000fful) != 0 ) {
				mask = 0x00000001ul;
				for ( i = 32; i < 40; i++, mask <<= 1 ) {
					if ( fliplist_bits.high & mask ) {
						g_hash_update_v1 ^= hash_flip1[i];
						g_hash_update_v2 ^= hash_flip2[i];
					}
				}
			}
			// bits 40-47
			if ( (fliplist_bits.high & 0x0000ff00ul) != 0 ) {
				mask = 0x00000100ul;
				for ( i = 40; i < 48; i++, mask <<= 1 ) {
					if ( fliplist_bits.high & mask ) {
						g_hash_update_v1 ^= hash_flip1[i];
						g_hash_update_v2 ^= hash_flip2[i];
					}
				}
			}
		//}
		// bits 48-63
		//if ( (fliplist_bits.high & 0xffff0000ul) != 0 ) {
			// bits 48-55
			if ( (fliplist_bits.high & 0x00ff0000ul) != 0 ) {
				mask = 0x00010000ul;
				for ( i = 48; i < 56; i++, mask <<= 1 ) {
					if ( fliplist_bits.high & mask ) {
						g_hash_update_v1 ^= hash_flip1[i];
						g_hash_update_v2 ^= hash_flip2[i];
					}
				}
			}
			// bits 56-63
			if ( (fliplist_bits.high & 0xff000000ul) != 0 ) {
				mask = 0x01000000ul;
				for ( i = 56; i < 64; i++, mask <<= 1 ) {
					if ( fliplist_bits.high & mask ) {
						g_hash_update_v1 ^= hash_flip1[i];
						g_hash_update_v2 ^= hash_flip2[i];
					}
				}
			}
		//}
	}
}

/*
   CAL_FLIP_DISC_HASH3
   Calculate flip disc hash value.
*/

INLINE void
cal_flip_disc_hash3(const BitBoard fliplist_bits, int color) {

	int i;
	unsigned long mask;

	g_hash_update_v1 = 0;
	g_hash_update_v2 = 0;

	if ( fliplist_bits.low != 0 ) {
		mask = 0x00000001ul;
		for ( i = 0; i < 32; i++, mask <<= 1 ) {
			if ( fliplist_bits.low & mask ) {
				g_hash_update_v1 ^= hash_flip1[i];
				g_hash_update_v2 ^= hash_flip2[i];
			}
		}
	}
	if ( fliplist_bits.high != 0 ) {
		mask = 0x00000001ul;
		for ( i = 32; i < 64; i++, mask <<= 1 ) {
			if ( fliplist_bits.high & mask ) {
				g_hash_update_v1 ^= hash_flip1[i];
				g_hash_update_v2 ^= hash_flip2[i];
			}
		}
	}
}

/*
   PREPARE_MOVELIST_PV
   Prepare generate movelist and check move is valid.
*/

INLINE int
prepare_movelist_pv( const BitBoard my_bits,
				 const BitBoard opp_bits,
				 int move) {

	BitPosition movelist_pos;
	BitBoard *movelist_bits;
	movelist_bits = (BitBoard *)ADDR_ALGIN_8BYTES(&movelist_pos);

	bitboard_gen_movelist(my_bits, opp_bits, movelist_bits);

	if ( ((movelist_bits->low & square_mask[move].low) == 0) &&
		((movelist_bits->high & square_mask[move].high) == 0) )
		return FALSE;

	return TRUE;
}

/*
   PREPARE_MOVELIST
   Prepare generate movelist and check move is valid.
*/

INLINE int
prepare_movelist( const BitBoard my_bits,
				 const BitBoard opp_bits,
				 BitBoard *movelist_bits,
				 int move,
				 int *need_gen_movelist ) {

	//return TRUE;
	// generate movelist
	if ( *need_gen_movelist ) {
		bitboard_gen_movelist(my_bits, opp_bits, movelist_bits);
		/*
		BitBoard movelist;
		bitboard_gen_movelist2(my_bits, opp_bits, &movelist);
		if ( ( movelist.low != movelist_bits->low ) ||
			( movelist.high != movelist_bits->high ) ) {
			*movelist_bits = movelist;
		}
		//*/
		*need_gen_movelist = FALSE;
	}

	if ( ((movelist_bits->low & square_mask[move].low) == 0) &&
		((movelist_bits->high & square_mask[move].high) == 0) )
		return FALSE;

	return TRUE;
}

/*
   PREPARE_MOVELIST_ADV
   Advanced prepare generate movelist and check move is valid.
*/

INLINE int
prepare_movelist_adv(const BitBoard my_bits,
				 const BitBoard opp_bits,
				 BitBoard *movelist_bits,
				 int move,
				 int *need_gen_movelist,
				 int is_best_move) {

	//return TRUE;
	if ( !is_best_move ) {
		// generate movelist
		if ( *need_gen_movelist ) {
			bitboard_gen_movelist(my_bits, opp_bits, movelist_bits);
			*need_gen_movelist = FALSE;
			//if ( (movelist_bits->low == 0) && (movelist_bits->high == 0) )
			//	return FALSE;
		}

		/*
		if ( ((movelist_bits->low & square_mask[move].low) != 0) ||
			((movelist_bits->high & square_mask[move].high) != 0) )
			return TRUE;
		else {
			if ( is_best_move )
				is_best_move = is_best_move;
			return FALSE;
		}
		//*/
	}
	return TRUE;
}

//#define USE_SIMD_FLIPFUNC
//#define USE_SSE_FLIP

/*
   MAKE_MOVE
   color = the side that is making the move
   move = the position giving the move

   Makes the necessary changes on the board and updates the
   counters.
*/

INLINE int
make_move( BitBoard *my_bits, BitBoard *opp_bits,
		  int color, int move, int update_hash ) {
	int flipped;
	unsigned int diff1, diff2;
	BitBoard flip_bits;

	if ( update_hash ) {
		flipped = GetFlipLists_wrapper( move, *my_bits, *opp_bits );
		if ( flipped <= 0 )
			return 0;

		// cal flip disc hash
		flip_bits.high = bb_flips.high ^ my_bits->high ^ square_mask[move].high;
		flip_bits.low = bb_flips.low ^ my_bits->low ^ square_mask[move].low;
		cal_flip_disc_hash(flip_bits, color);

		diff1 = g_hash_update_v1 ^ hash_put_value1[color][move];
		diff2 = g_hash_update_v2 ^ hash_put_value2[color][move];
		hash_stored1[disks_played] = g_hash1;
		hash_stored2[disks_played] = g_hash2;
		g_hash1 ^= diff1;
		g_hash2 ^= diff2;
	}
	else {
		flipped = GetFlipLists_wrapper( move, *my_bits, *opp_bits );
		if ( flipped <= 0 )
			return 0;
		hash_stored1[disks_played] = g_hash1;
		hash_stored2[disks_played] = g_hash2;
	}

	//flip_count[disks_played] = flipped;

	/*
	BITBOARD_ANDNOT( (*opp_bits), bb_flips );
	BITBOARD_OR( (*my_bits), bb_flips );
	bitboard_or_bit( my_bits, move );
	//*/
	BITBOARD_ANDNOT( (*opp_bits), bb_flips );
	*my_bits = bb_flips;

	if ( color == CHESS_BLACK ) {
		piece_count[CHESS_BLACK][disks_played + 1] =
			piece_count[CHESS_BLACK][disks_played] + flipped + 1;
		piece_count[CHESS_WHITE][disks_played + 1] =
			piece_count[CHESS_WHITE][disks_played] - flipped;
	}
	else {  /* color == WHITE */
		piece_count[CHESS_WHITE][disks_played + 1] =
			piece_count[CHESS_WHITE][disks_played] + flipped + 1;
		piece_count[CHESS_BLACK][disks_played + 1] =
			piece_count[CHESS_BLACK][disks_played] - flipped;
	}

	disks_played++;

	return flipped;
}

/*
  UNMAKE_MOVE
  Takes back a move.
*/

INLINE void
unmake_move( int color, int move ) {
	//board[move] = CHESS_EMPTY;

	disks_played--;

	g_hash1 = hash_stored1[disks_played];
	g_hash2 = hash_stored2[disks_played];

	//UndoFlips_inlined( flip_count[disks_played], OPP_COLOR( color ) );
}

/*
   MAKE_MOVE_NO_HASH
   color = the side that is making the move
   move = the position giving the move

   Makes the necessary changes on the board. Note that the hash table
   is not updated - the move has to be unmade using UNMAKE_MOVE_NO_HASH().
*/

INLINE int
make_move_no_hash( BitBoard *my_bits, BitBoard *opp_bits,
				  int color, int move ) {
	int flipped;

	flipped = GetFlipLists_wrapper( move, *my_bits, *opp_bits );
	if ( flipped <= 0 )
		return 0;

	//flip_count[disks_played] = flipped;

    /*
	BITBOARD_ANDNOT( (*opp_bits), bb_flips );
	BITBOARD_OR( (*my_bits), bb_flips );
	bitboard_or_bit( my_bits, move );
	//*/
	BITBOARD_ANDNOT( (*opp_bits), bb_flips );
	*my_bits = bb_flips;

	if ( color == CHESS_BLACK ) {
		piece_count[CHESS_BLACK][disks_played + 1] =
			piece_count[CHESS_BLACK][disks_played] + flipped + 1;
		piece_count[CHESS_WHITE][disks_played + 1] =
			piece_count[CHESS_WHITE][disks_played] - flipped;
	}
	else {  /* color == WHITE */
		piece_count[CHESS_WHITE][disks_played + 1] =
			piece_count[CHESS_WHITE][disks_played] + flipped + 1;
		piece_count[CHESS_BLACK][disks_played + 1] =
			piece_count[CHESS_BLACK][disks_played] - flipped;
	}

	disks_played++;

	return flipped;
}

/*
  UNMAKE_MOVE_NO_HASH
  Takes back a move. Only to be called when the move was made without
  updating hash table, preferrable through MAKE_MOVE_NO_HASH().
*/

INLINE void
unmake_move_no_hash( int color, int move ) {
	//board[move] = CHESS_EMPTY;

	disks_played--;

	//UndoFlips_inlined( flip_count[disks_played], OPP_COLOR( color ) );
}

/*
   MAKE_MOVE_END
   color = the side that is making the move
   move = the position giving the move

   Makes the necessary changes on the board and updates the
   counters.
*/

INLINE void
make_move_end( int color, int move,
			  //int flipped,
			  int update_hash ) {
	unsigned int diff1, diff2;

	//if (flipped > 0) {

		if ( update_hash ) {
			// cal flip disc hash
			cal_flip_disc_hash_end(bb_flip_bits, color);

			diff1 = g_hash_update_v1 ^ hash_put_value1[color][move];
			diff2 = g_hash_update_v2 ^ hash_put_value2[color][move];
			hash_stored1[disks_played] = g_hash1;
			hash_stored2[disks_played] = g_hash2;
			g_hash1 ^= diff1;
			g_hash2 ^= diff2;
		}
		else {
			hash_stored1[disks_played] = g_hash1;
			hash_stored2[disks_played] = g_hash2;
		}

		/*
		if ( color == CHESS_BLACK ) {
			piece_count[CHESS_BLACK][disks_played + 1] =
				piece_count[CHESS_BLACK][disks_played] + flipped + 1;
			piece_count[CHESS_WHITE][disks_played + 1] =
				piece_count[CHESS_WHITE][disks_played] - flipped;
		}
		else {  // color == WHITE
			piece_count[CHESS_WHITE][disks_played + 1] =
				piece_count[CHESS_WHITE][disks_played] + flipped + 1;
			piece_count[CHESS_BLACK][disks_played + 1] =
				piece_count[CHESS_BLACK][disks_played] - flipped;
		}
		//*/

		disks_played++;
	//}
}

/*
   MAKE_MOVE_END2
   color = the side that is making the move
   move = the position giving the move

   Makes the necessary changes on the board and updates the
   counters.
*/

INLINE void
make_move_end2( int color, int move,
			  int flipped,
			  int update_hash ) {
	unsigned int diff1, diff2;

	//if (flipped > 0) {
		if ( update_hash ) {
			// cal flip disc hash
			cal_flip_disc_hash_end(bb_flip_bits, color);

			diff1 = g_hash_update_v1 ^ hash_put_value1[color][move];
			diff2 = g_hash_update_v2 ^ hash_put_value2[color][move];
			hash_stored1[disks_played] = g_hash1;
			hash_stored2[disks_played] = g_hash2;
			g_hash1 ^= diff1;
			g_hash2 ^= diff2;
		}
		else {
			hash_stored1[disks_played] = g_hash1;
			hash_stored2[disks_played] = g_hash2;
		}

		if ( color == CHESS_BLACK ) {
			piece_count[CHESS_BLACK][disks_played + 1] =
				piece_count[CHESS_BLACK][disks_played] + flipped + 1;
			piece_count[CHESS_WHITE][disks_played + 1] =
				piece_count[CHESS_WHITE][disks_played] - flipped;
		}
		else {  // color == WHITE
			piece_count[CHESS_WHITE][disks_played + 1] =
				piece_count[CHESS_WHITE][disks_played] + flipped + 1;
			piece_count[CHESS_BLACK][disks_played + 1] =
				piece_count[CHESS_BLACK][disks_played] - flipped;
		}

		disks_played++;
	/*
	}
	else {
		hash_stored1[disks_played] = g_hash1;
		hash_stored2[disks_played] = g_hash2;
		disks_played++;
	}
	//*/
}

/*
  UNMAKE_MOVE_END
  Takes back a move.
*/

INLINE void
unmake_move_end( int color, int move ) {
	//board[move] = CHESS_EMPTY;

	disks_played--;

	g_hash1 = hash_stored1[disks_played];
	g_hash2 = hash_stored2[disks_played];
}
