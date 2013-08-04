/*
   File:           bitbmob.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard mobility(MMX version).
*/

#ifndef __BITBMOB_H_
#define __BITBMOB_H_

#include "bitboard.h"

#ifdef __cplusplus
extern "C" {
#endif

void
init_mmx( void );

int
bitboard_mobility( const BitBoard my_bits,
				  const BitBoard opp_bits );

int
bitboard_mobility1( const BitBoard my_bits,
				  const BitBoard opp_bits );

int
bitboard_mobility2( const BitBoard my_bits,
				  const BitBoard opp_bits );

int
weighted_mobility( const BitBoard my_bits,
				  const BitBoard opp_bits );

int
weighted_mobility1( const BitBoard my_bits,
				  const BitBoard opp_bits );

int
weighted_mobility2( const BitBoard my_bits,
				  const BitBoard opp_bits );

void
bitboard_gen_movelist( const BitBoard my_bits,
					  const BitBoard opp_bits, BitBoard *movelist_bits );

void
bitboard_gen_movelist2( const BitBoard my_bits,
					  const BitBoard opp_bits, BitBoard *movelist_bits );

#ifdef __cplusplus
}
#endif

#endif  /* __BITBMOB_H_ */
