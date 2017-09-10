/*
   File:           timer.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The time control mechanism.
*/

#ifndef __TIMER_H_
#define __TIMER_H_

#include "global.h"
#include "counter.h"

#ifdef __cplusplus
extern "C" {
#endif

extern int current_ponder_depth;
extern int ponder_depth[100];

extern double frozen_ponder_time;

extern int frozen_ponder_depth;

/* Holds the value of the variable NODES the last time the
   timer module was called to check if a panic abort occured. */
extern double last_panic_check;

void
determine_move_time(double time_left, double incr, int discs);

void
start_move(double in_total_time, double increment, int discs);

void
set_panic_threshold(double value);

void
check_panic_abort(void);

int
check_threshold(double threshold);

void
clear_panic_abort(void);

int
is_panic_abort(void);

void
toggle_abort_check(int enable);

void
init_timer(void);

double
get_real_timer(void);

void
reset_real_timer(void);

double
get_elapsed_time(void);

void
clear_ponder_times(void);

void
add_ponder_time(int move, double time);

void
adjust_current_ponder_time(int move);

int
above_recommended(void);

int
extended_above_recommended(void);

#ifdef __cplusplus
}
#endif

#endif  /* __TIMER_H_ */
