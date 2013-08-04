/*
   File:           myrandom.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The myself randomize number interface.
*/

#ifndef __MYRANDOM_H_
#define __MYRANDOM_H_

#ifdef __cplusplus
extern "C" {
#endif

int
my_srandom( int x );

char *
my_initstate( unsigned seed, char *arg_state, int n );

char *
my_setstate( char *arg_state );

long
my_random( void );

void
randomize(unsigned int seed);

#ifdef __cplusplus
}
#endif

#endif  /* __MYRANDOM_H_ */
