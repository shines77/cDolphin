/*
   File:           bitbtest2.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard get the flips of a move.
*/

#ifndef __BITBTEST2_H_
#define __BITBTEST2_H_

#include "bitboard.h"

#ifdef __cplusplus
extern "C" {
#endif

extern BitBoard bb_flips2;

extern int (*TestFlips_bitboard2[64])(const BitBoard, const BitBoard);

void
init_disc_mask( void );

#ifdef __cplusplus
}
#endif

#endif  /* __BITBTEST2_H_ */
