/*
   File:       safemem.h

   Created:    August 30, 1998

   Modified:   January 25, 2000

   Author:     Gunnar Andersson (gunnar@radagast.se)

   Contents:   The interface to the safer version of malloc.
*/

#ifndef __SAFEMEM_H_
#define __SAFEMEM_H_

#include <stdlib.h>

void *
safe_malloc(size_t size);

void *
safe_realloc(void *ptr, size_t size);

#endif  /* __SAFEMEM_H_ */
