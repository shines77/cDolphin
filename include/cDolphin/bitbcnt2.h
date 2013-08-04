/*
   File:          bitbcnt2.h

   Created:       November 22, 1999
   
   Modified:      November 24, 2005

   Authors:       Gunnar Andersson (gunnar@radagast.se)

   Contents:
*/



#ifndef BITBCNT2_H
#define BITBCNT2_H



#include "utils.h"
#include "bitboard.h"



extern int (REGPARM(2) * const CountFlips_bitboard2[64])(unsigned int my_bits_high, unsigned int my_bits_low);



#endif  /* BITBCNT2_H */
