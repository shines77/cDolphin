/*
   File:           colour.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Some colour defines and constants.
*/

#ifndef __COLOUR_H_
#define __COLOUR_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Symbolic values for the possible contents of a square */
#define CHESS_ILLEGAL               -1
#define CHESS_EMPTY                 1
#define CHESS_BLACK                 0
#define CHESS_WHITE                 2
#define CHESS_OUTSIDE               3

#define OPP_COLOR( color )          ((CHESS_BLACK + CHESS_WHITE) - (color))

#ifdef __cplusplus
}
#endif

#endif  /* __COLOUR_H_ */
