/*
   File:           bitbtest.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard get the flips of a move.
*/

#ifndef __BITBTEST_H_
#define __BITBTEST_H_

#include "bitboard.h"

#ifdef __cplusplus
extern "C" {
#endif

extern ALIGN_PREFIX(64) BitBoard bb_flips ALIGN_SUFFIX(64);
extern ALIGN_PREFIX(16) BitBoard bb_flip_bits ALIGN_SUFFIX(16);

extern int (*TestFlips_bitboard[64])(const BitBoard, const BitBoard);

#ifdef __cplusplus
}
#endif

#endif  /* __BITBTEST_H_ */
