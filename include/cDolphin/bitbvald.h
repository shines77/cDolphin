/*
   File:           bitbvald.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard get the valid one empty.
*/

#ifndef __BITBVALD_H_
#define __BITBVALD_H_

#include "bitboard.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int (*ValidOneEmpty_bitboard[64])(const BitBoard my_bits);

#ifdef __cplusplus
}
#endif

#endif  /* __BITBVALD_H_ */

