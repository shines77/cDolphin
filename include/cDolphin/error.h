/*
   File:           error.h

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The interface to the error handler.
*/

#ifndef __ERROR_H_
#define __ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

void
fatal_error(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif  /* __ERROR_H_ */
