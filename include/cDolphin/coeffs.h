/*
   File:           coeffs.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       About pattern coeffs.
*/

#ifndef __COEFFS_H_
#define __COEFFS_H_

#include "bitboard.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Magic numbers for the file with evaluation coefficients.
   Name convention: Decimals of Pi. */

#define EVAL_MAGIC1                5358
#define EVAL_MAGIC2                9793

/* Magic numbers for the opening book file format.
   Name convention: Decimals of E. */

#define BOOK_MAGIC1                2718
#define BOOK_MAGIC2                2818

/*
#ifndef USE_BYTE_BOARD
extern int wBoard[64];
#else
extern char wBoard[64];
#endif
//*/

void
init_coeffs(void);

void
remove_coeffs(int phase);

void
clear_coeffs(void);

int
pattern_evaluation(int color,
                   const BitBoard my_bits,
                   const BitBoard opp_bits);

int
pattern_evaluation2(int color,
                    const BitBoard my_bits,
                    const BitBoard opp_bits);

void
init_disc_set_table(void);

void
init_pattern_mask(void);

#ifdef __cplusplus
}
#endif

#endif  /* __COEFFS_H_ */
