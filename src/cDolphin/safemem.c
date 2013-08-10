/*
   File:            safemem.c

   Created:         August 30, 1998

   Modified:        November 1, 2000

   Author:          Gunnar Andersson (gunnar@radagast.se)

   Contents:        Provides safer memory allocation than malloc().
*/

#include <stdlib.h>
#include "utils.h"
#include "texts.h"
#include "error.h"
#include "safemem.h"

INLINE void *
safe_malloc( size_t size ) {
	void * block;

	block = malloc( size );
	if ( block == NULL )
		fatal_error( "%s %d\n", SAFEMEM_FAILURE, size );

	return block;
}

INLINE void *
safe_realloc( void *ptr, size_t size ) {
	void * block;

	block = realloc( ptr, size );
	if ( block == NULL )
		fatal_error( "%s %d\n", SAFEMEM_FAILURE, size );

	return block;
}
