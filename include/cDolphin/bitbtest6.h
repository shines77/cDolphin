/*
   File:           bitbtest6.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard get the flips of a move.
*/

#ifndef __BITBTEST6_H_
#define __BITBTEST6_H_

#include "bitboard.h"

#ifdef __cplusplus
extern "C" {
#endif

extern BitBoard bb_flips6;

extern int (*TestFlips_bitboard6[64])(const BitBoard, const BitBoard);

void
init_disc_mask_6( void );

#ifdef __cplusplus
}
#endif

#endif  /* __BITBTEST6_H_ */
