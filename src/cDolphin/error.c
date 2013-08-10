/*
   File:           error.c

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The text-based error handler.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#ifndef _WIN32_WCE
#include <time.h>
#endif

#include "utils.h"
#include "error.h"

#if defined(_WIN32_WCE) || defined(_MSC_VER) || defined(_WIN32)

#include <windows.h>

void
fatal_error( const char *format, ... ) {
	va_list arg_ptr;
	char sError[128];
	WCHAR wcsError[128];

	va_start( arg_ptr, format );

	vsprintf(sError, format, arg_ptr);
	mbstowcs(wcsError, sError, 128);
	OutputDebugStringW((LPCWSTR)wcsError);
	MessageBoxW(NULL, (LPCWSTR)wcsError, L"Fatal Error", MB_OK);

	exit( EXIT_FAILURE );
}

#else	/* not Windows CE */

/* Error header in error.c */
#define  FATAL_ERROR_TEXT      "Fatal error"

void
fatal_error( const char *format, ... ) {
	FILE *stream;
	time_t timer;
	va_list arg_ptr;

	va_start( arg_ptr, format );
	fprintf( stderr, "\n%s: ", FATAL_ERROR_TEXT );
	vfprintf( stderr, format, arg_ptr );

	va_end( arg_ptr );

	stream = fopen( "Dolphin.err", "a" );
	if ( stream != NULL ) {
		time( &timer );
		fprintf( stream, "%s @ %s\n  ", FATAL_ERROR_TEXT, ctime( &timer ) );
		va_start( arg_ptr, format );
		vfprintf( stream, format, arg_ptr );
		va_end( arg_ptr );
	}

	exit( EXIT_FAILURE );
}

#endif
