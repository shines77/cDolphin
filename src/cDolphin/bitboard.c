/*
   File:           bitboard.cpp

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines abort bitboard.
*/

#include "utils.h"
#include "colour.h"
#include "bitboard.h"

__declspec(align(64)) BitBoard square_mask[64];

/*
  NON_ITERATIVE_POPCOUNT
  Counts the number of bits set in a 64-bit integer.
  This is done using some bitfiddling tricks.
*/

#define m1      0x55555555
#define m2      0x33333333

INLINE unsigned int
non_iterative_popcount( BitBoard b ) {
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
  NON_ITERATIVE_POPCOUNT3
  Counts the number of bits set in a 64-bit integer.
  This is done using some bitfiddling tricks.
*/

INLINE unsigned int REGPARM(2)
non_iterative_popcount3( unsigned int n1, unsigned int n2 ) {
	n1 = n1 - ((n1 >> 1) & 0x55555555u);
	n2 = n2 - ((n2 >> 1) & 0x55555555u);
	n1 = (n1 & 0x33333333u) + ((n1 >> 2) & 0x33333333u);
	n2 = (n2 & 0x33333333u) + ((n2 >> 2) & 0x33333333u);
	n1 = (n1 + (n1 >> 4)) & 0x0F0F0F0Fu;
	n2 = (n2 + (n2 >> 4)) & 0x0F0F0F0Fu;
	return ((n1 + n2) * 0x01010101u) >> 24;
}

/*
ITERATIVE_POPCOUNT
Counts the number of bits set in a 64-bit integer.
This is done using an iterative procedure which loops
a number of times equal to the number of bits set,
hence this function is fast when the number of bits
set is low.
*/

INLINE unsigned int
iterative_popcount( BitBoard b ) {
	unsigned int n;
	n = 0;
	for ( ; b.high != 0; n++, b.high &= (b.high - 1) )
		;
	for ( ; b.low != 0; n++, b.low &= (b.low - 1) )
		;

	return n;
}

/*
BIT_REVERSE_32
Returns the bit-reverse of a 32-bit integer.
*/

unsigned int
bit_reverse_32( unsigned int val ) {
	val = ((val >>  1) & 0x55555555) | ((val <<  1) & 0xAAAAAAAA);
	val = ((val >>  2) & 0x33333333) | ((val <<  2) & 0xCCCCCCCC);
	val = ((val >>  4) & 0x0F0F0F0F) | ((val <<  4) & 0xF0F0F0F0);
	val = ((val >>  8) & 0x00FF00FF) | ((val <<  8) & 0xFF00FF00);
	val = ((val >> 16) & 0x0000FFFF) | ((val << 16) & 0xFFFF0000);

	return val;
}

INLINE void
bitboard_set_bit(BitBoard *b, unsigned long pos)
{
	if (pos < 32) {
		b->low  = (1 << pos);
		b->high = 0;
	}
	else {
		b->low  = 0;
		b->high = (1 << (pos - 32));
	}
}

INLINE void
bitboard_and_bit(BitBoard *b, unsigned long pos)
{
	if (pos < 32) {
		b->low  = b->low & (1 << pos);
		b->high = 0;
	}
	else {
		b->low  = 0;
		b->high = b->high & (1 << (pos - 32));
	}
}

INLINE void
bitboard_or_bit(BitBoard *b, unsigned long pos)
{
	if (pos < 32) {
		b->low |= (1 << pos);
	}
	else {
		b->high |= (1 << (pos - 32));
	}
}

/*
SET_BITBOARDS
Converts the vector board representation to the bitboard representation.
*/

void
set_bitboards( int *in_board, int color,
			  BitBoard *my_out, BitBoard *opp_out ) {
	int i, j;
	int pos;
	unsigned long mask;
	int opp_color = OPP_COLOR( color );
	BitBoard my_bits, opp_bits;

	my_bits.high = 0;
	my_bits.low = 0;
	opp_bits.high = 0;
	opp_bits.low = 0;

	mask = 1;
	for ( i = 0; i < 4; i++ ) {
		for ( j = 0; j < 8; j++, mask <<= 1 ) {
			pos = 8 * i + j;
			if ( in_board[pos] == color )
				my_bits.low |= mask;
			else if ( in_board[pos] == opp_color )
				opp_bits.low |= mask;
		}
	}

	mask = 1;
	for ( i = 4; i < 8; i++ ) {
		for ( j = 0; j < 8; j++, mask <<= 1 ) {
			pos = 8 * i + j;
			if ( in_board[pos] == color )
				my_bits.high |= mask;
			else if ( in_board[pos] == opp_color )
				opp_bits.high |= mask;
		}
	}

	*my_out = my_bits;
	*opp_out = opp_bits;
}

/*
SET_BOARDS
Converts the bitboard representation to the board representation.
*/

void
set_boards( int color,
		   BitBoard my_bits,
		   BitBoard opp_bits,
		   int *in_board ) {
	int i, j;
	int pos;
	int opp_color = OPP_COLOR( color );

	for ( i = 0; i < 8; i++ ) {
		for ( j = 0; j < 8; j++ ) {
			pos = 8 * i + j;
			if ( (square_mask[pos].low & my_bits.low)
				| (square_mask[pos].high & my_bits.high) ) {
				in_board[pos] = color;
			}
			else if ( (square_mask[pos].low & opp_bits.low)
				| (square_mask[pos].high & opp_bits.high) ) {
				in_board[pos] = opp_color;
			}
			else
				in_board[pos] = CHESS_EMPTY;
		}
	}
}

/*
  INIT_SQUARE_MASK
*/

void
init_square_mask( void ) {
	int pos;
	for ( pos = 0; pos < 64; pos++ ) {
		if ( pos < 32 ) {
			square_mask[pos].low = 1ul << pos;
			square_mask[pos].high = 0;
		}
		else {
			square_mask[pos].low = 0;
			square_mask[pos].high = 1ul << (pos - 32);
		}
	}
}

/*
  INIT_SQUARE_MASK2
*/

void
init_square_mask2( void ) {
	int pos;
	unsigned long mask;

	mask = 1;
	for ( pos = 0; pos < 32; pos++, mask <<= 1 ) {
		square_mask[pos].low = mask;
		square_mask[pos].high = 0;
	}

	mask = 1;
	for ( pos = 32; pos < 64; pos++, mask <<= 1 ) {
		square_mask[pos].low = 0;
		square_mask[pos].high = mask;
	}
}
