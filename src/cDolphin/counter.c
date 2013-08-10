/*
   File:           counter.c

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The counter code. The current implementation is
                   capable of representing values up to 2^32 * 10^8,
		   i.e., 429496729600000000, assuming 32-bit integers.
*/

#include <math.h>
#include "utils.h"
#include "counter.h"

/*
  RESET_COUNTER
*/

INLINE void
reset_counter( CounterType *counter ) {
	counter->low = 0;
	counter->high = 0;
}

/*
  ADJUST_COUNTER
  Makes sure that the LO part of the counter only contains 8 decimal digits.
*/

INLINE void
adjust_counter( CounterType *counter ) {
	while ( counter->low >= DECIMAL_BASIS ) {
		counter->low -= DECIMAL_BASIS;
		counter->high++;
	}
}

/*
  ADD_COUNTER
  Adds the value of the counter TERM to the counter SUM.
*/

INLINE void
add_counter( CounterType *sum, CounterType *term ) {
	sum->low += term->low;
	sum->high += term->high;
	adjust_counter( sum );
}

/*
  SUB_COUNTER
  Subs the value of the counter TERM1 and TERM2 to the counter SUM.
*/

INLINE void
sub_counter( CounterType *sub, CounterType *term1, CounterType *term2 ) {
	adjust_counter( term1 );
	adjust_counter( term2 );
	if (term1->low >= term2->low) {
		sub->low = term1->low - term2->low;
		sub->high = term1->high - term2->high;
	}
	else {
		sub->low = (DECIMAL_BASIS - term2->low) + term1->low;
		sub->high = term1->high - term2->high - 1;
	}
	adjust_counter( sub );
}

/*
  SUM_COUNTER
  Adds the value of the counter TERM1 and TERM2 to the counter SUM.
*/

INLINE void
sum_counter( CounterType *sum, CounterType *term1, CounterType *term2 ) {
	adjust_counter( term1 );
	adjust_counter( term2 );
	sum->low = term1->low + term2->low;
	sum->high = term1->high + term2->high;
	adjust_counter( sum );
}

/*
  COUNTER_VALUE
  Converts a counter to a single floating-point value.
*/

INLINE double
counter_value( CounterType *counter ) {
	adjust_counter( counter );
	return ((double) DECIMAL_BASIS) * counter->high + counter->low;
}

/*
  COUNTER_SUM_VALUE
  Adds the value of the counter TERM1 and TERM2 to the counter SUM.
  And converts a counter SUM to a single floating-point value.
*/

INLINE double
counter_sum_value( CounterType *sum, CounterType *term1, CounterType *term2 ) {
	adjust_counter( term1 );
	adjust_counter( term2 );
	sum->low = term1->low + term2->low;
	sum->high = term1->high + term2->high;
	adjust_counter( sum );
	return ((double) DECIMAL_BASIS) * sum->high + sum->low;
}

INLINE unsigned __int64
counter_int64( CounterType *counter ) {
	adjust_counter( counter );
	return ((unsigned __int64)DECIMAL_BASIS) * counter->high + counter->low;
}
