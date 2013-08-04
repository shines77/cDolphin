/*
   File:           bitbtest5.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard get the flips of a move.
*/

#ifndef __BITBTEST5_H_
#define __BITBTEST5_H_

#include "bitboard.h"

#ifdef __cplusplus
extern "C" {
#endif

extern BitBoard bb_flips5;

extern int (*TestFlips_bitboard5[64])(const BitBoard, const BitBoard);

void
init_disc_mask_5( void );

#ifdef __cplusplus
}
#endif

#endif  /* __BITBTEST5_H_ */
