/*
   File:           bitbchk.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard check a move is valid.
*/

#ifndef __BITBCHK_H_
#define __BITBCHK_H_

#include "bitboard.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int (*CheckFlips_bitboard[64])(const BitBoard, const BitBoard);

#ifdef __cplusplus
}
#endif

#endif  /* __BITBCHK_H_ */
