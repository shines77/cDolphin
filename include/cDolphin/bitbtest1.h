/*
   File:          bitbtest1.h

   Created:       November 22, 1999
   
   Modified:      November 24, 2005

   Authors:       Gunnar Andersson (gunnar@radagast.se)

   Contents:
*/


#ifndef BITBTEST1_H
#define BITBTEST1_H

#include "utils.h"
#include "bitboard.h"

extern BitBoard bb_flips1;

extern int (REGPARM(2) * const TestFlips_bitboard1[64])(unsigned int, unsigned int, unsigned int, unsigned int);


#endif  /* BITBTEST1_H */

