/*
   File:           counter.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The interface to the counter code.
*/

#ifndef __COUNTER_H_
#define __COUNTER_H_

#include "utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// 268435456 = 0x10000000
#define  DECIMAL_BASIS     100000000

#define INCREMENT_COUNTER( counter )              counter.low++
#define INCREMENT_COUNTER_BY( counter, term )     counter.low += (term)

typedef struct tagCounterType {
	unsigned int high;
	unsigned int low;
} CounterType;

void reset_counter( CounterType *counter );

void adjust_counter( CounterType *counter );

void add_counter( CounterType *sum, CounterType *term );

void sub_counter( CounterType *sub, CounterType *term1, CounterType *term2 );

void sum_counter( CounterType *sum, CounterType *term1, CounterType *term2 );

double counter_value( CounterType *counter );

double counter_sum_value( CounterType *sum, CounterType *term1, CounterType *term2 );

unsigned __int64 counter_int64( CounterType *counter );

#ifdef __cplusplus
}
#endif

#endif  /* __COUNTER_H_ */
