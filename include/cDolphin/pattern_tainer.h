
#ifndef __PATTERN_TAINER_H_
#define __PATTERN_TAINER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "utils.h"
#include "colour.h"
#include "board.h"
#include "bitboard.h"

typedef struct tagInfoItem {
	double solution;
	int pattern;
} InfoItem;

void
initialize_pattern_coffes( void );

double
calc_pattern_score( int color,
				   const BitBoard my_bits,
				   const BitBoard opp_bits );

#ifdef __cplusplus
}
#endif

#endif  /* __PATTERN_TAINER_H_ */
