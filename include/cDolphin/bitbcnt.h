/*
   File:           bitbcnt.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard get the filps disc count.
*/

#ifndef __BITBCNT_H_
#define __BITBCNT_H_

#include "bitboard.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int (*CountFlips_bitboard[64])(const BitBoard my_bits);

#ifdef __cplusplus
}
#endif

#endif  /* __BITBCNT_H_ */
