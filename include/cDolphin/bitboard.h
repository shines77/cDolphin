/*
   File:           bitboard.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Some about bitboard types and defines.
*/

#ifndef __BITBOARD_H_
#define __BITBOARD_H_

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

#define BITBOARD_CLEAR( a ) { \
  a.low = 0; \
  a.high = 0; \
}

#define BITBOARD_NOT( a ) { \
  a.low = ~a.low; \
  a.high = ~a.high; \
}

#define BITBOARD_XOR( a, b ) { \
  a.low ^= b.low; \
  a.high ^= b.high; \
}

#define BITBOARD_OR( a, b ) { \
  a.low |= b.low; \
  a.high |= b.high; \
}

#define BITBOARD_AND( a, b ) { \
  a.low &= b.low; \
  a.high &= b.high; \
}

#define BITBOARD_ANDNOT( a, b ) { \
  a.low &= ~b.low; \
  a.high &= ~b.high; \
}

#define BITBOARD_FULL_XOR( a, b, c ) { \
  a.low = b.low ^ c.low; \
  a.high = b.high ^ c.high; \
}

#define BITBOARD_FULL_OR( a, b, c ) { \
  a.low = b.low | c.low; \
  a.high = b.high | c.high; \
}

#define BITBOARD_FULL_AND( a, b, c ) { \
  a.low = b.low & c.low; \
  a.high = b.high & c.high; \
}

#define BITBOARD_FULL_ANDNOT( a, b, c ) { \
  a.low = b.low & ~c.low; \
  a.high = b.high & ~c.high; \
}

#define BITBOARD_FULL_NOTOR( a, b, c ) { \
  a.low = ~(b.low | c.low); \
  a.high = ~(b.high | c.high); \
}

typedef struct tagBitBoard {
	unsigned long low;
	unsigned long high;
} BitBoard;

typedef union tagUBitBoard {
	BitBoard bb;
	uint64 u64;
} uBitBoard;

typedef struct tagBitPosition {
	uBitBoard my_bits;
	uBitBoard opp_bits;
	//uBitBoard reserve1;
	//uBitBoard reserve2;
} BitPosition;

typedef struct tagBitPosition2 {
	BitBoard my_bits;
	BitBoard opp_bits;
} BitPosition2;

typedef union tagUBitPosition {
	BitPosition bpos;
	uint64 u64;
} uBitPosition;

extern __declspec(align(64)) BitBoard square_mask[64];

void
bitboard_set_bit(BitBoard *b, unsigned long pos);

void
bitboard_and_bit(BitBoard *b, unsigned long pos);

void
bitboard_or_bit(BitBoard *b, unsigned long pos);

unsigned int
non_iterative_popcount( BitBoard b );

unsigned int REGPARM(2)
non_iterative_popcount3( unsigned int n1, unsigned int n2 );

unsigned int
iterative_popcount( BitBoard b );

unsigned int
bit_reverse_32( unsigned int val );

void
set_bitboards( int *in_board, int side_to_move,
			  BitBoard *my_out, BitBoard *opp_out );

void
set_boards( int color,
		   BitBoard my_bits,
		   BitBoard opp_bits,
		   int *in_board );

void
init_square_mask( void );

#ifdef __cplusplus
}
#endif

#endif  /* __BITBOARD_H_ */
