/*
   File:           utils.h

   Created:        2007-07-10

   Modified:       2013-08-10

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Some utility defined.
*/

#ifndef __UTILS_H__
#define __UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#if (defined(WIN32) || defined(__windows__) || defined(__WIN32__) || defined(_WINDOWS))
#   ifndef _WIN32
#       define _WIN32
#   endif
#endif

#if defined(DOL_STATIC)
#   ifndef DOL_DECLARE_STATIC
#       define DOL_DECLARE_STATIC
#   endif
#endif

#if defined(DOL_EXPORT)
#   ifndef DOL_DECLARE_EXPORT
#       define DOL_DECLARE_EXPORT
#   endif
#endif

#ifdef DOL_EXPORT
#undef DOL_EXPORT
#endif

/*************************************************************************
    Dynamic Library import / export control conditional
    (Define DOL_DECLARE_EXPORT to export symbols, else they are imported)
*************************************************************************/
#if defined(_WIN32) && (!defined(DOL_DECLARE_STATIC))
#   if defined(DOL_DECLARE_EXPORT)
#       define DOL_EXPORT     __declspec(dllexport)
#   else
#       define DOL_EXPORT     __declspec(dllimport)
#   endif
#       define DOL_PRIVATE
#else
#       define DOL_EXPORT
#       define DOL_PRIVATE
#endif

// constants
#ifndef TRUE
#define TRUE        1
#endif

#ifndef FALSE
#define FALSE       0
#endif

//#define TEXT_BASED
#define USE_WINDOW      FALSE

/* Define the inline directive when available */

#if defined( _MSC_VER )

	#if defined( __cplusplus )
	    #define INLINE __inline
	#else
	    #define INLINE
	#endif

#else

	#if defined( __GNUC__ ) && defined( __cplusplus )
	    #define INLINE __inline__
	#else
	    #define INLINE
	#endif

#endif

// marcos

#ifdef _MSC_VER
    #define S64(u) (u##i64)
    #define U64(u) (u##ui64)
#else
    #define S64(u) (u##LL)
    #define U64(u) (u##ULL)
#endif

#ifndef _MSC_VER
#undef __int64
#define __int64 long long
#endif

#define MAX(a,b)                (((a) > (b)) ? (a) : (b))

#define MIN(a,b)                (((a) < (b)) ? (a) : (b))

#define SQR(a)                  ((a) * (a))

/* Convert index to square, e.g. 27 -> g2 */
#define TO_SQUARE(index)        'a'+(index & 0x07ul),'1'+(index >> 3)
#define TO_SQUARE_L(index)      'a'+(index & 0x07ul),'1'+(index >> 3)
#define TO_SQUARE_U(index)      'A'+(index & 0x07ul),'1'+(index >> 3)

// Defined whether use memory attribute aligned.

#define USE_ATTR_ALIGNED        1

#if defined(USE_ATTR_ALIGNED) && (USE_ATTR_ALIGNED != 0)
#   if defined(__GNUC__)
#       define DECLARE_ALIGNED(x)      __attribute__((aligned(x)))
#   elif defined(_WIN32) || defined(_MSC_VER)
#       define DECLARE_ALIGNED(x)      __declspec(align(x))
#   else
#       define DECLARE_ALIGNED(x)
#   endif
#else
#   define DECLARE_ALIGNED(x)
#endif

// Statement: int __declspec(align(N)) X; support from MS VC++ 7.1 (_MSC_VER >= 1310)
// or Intel C++ Compile (ICC) Version > 8.1

#ifdef ALIGN_PREFIX
#undef ALIGN_PREFIX
#endif

#if defined(USE_ATTR_ALIGNED) && (USE_ATTR_ALIGNED != 0)
#   if defined(__ICL) && (defined(__VERSION__) || defined(__INTEL_COMPILER_BUILD_DATE))
#       define ALIGN_PREFIX(N)      __declspec(align(N))
#   elif defined(_MSC_VER) && (_MSC_VER >= 1310)
#       define ALIGN_PREFIX(N)      __declspec(align(N))
#   else
#       define ALIGN_PREFIX(N)
#   endif
#else
#   define ALIGN_PREFIX(N)
#endif

// Statement: int X __attribute__((aligned(N))); support by gcc (GNU C)

#ifdef ALIGN_SUFFIX
#undef ALIGN_SUFFIX
#endif

#if defined(USE_ATTR_ALIGNED) && (USE_ATTR_ALIGNED != 0)
#   if defined(__GNUC__)
#       define ALIGN_SUFFIX(N)      __attribute__((aligned(N)))
#   else
#       define ALIGN_SUFFIX(N)
#   endif
#else
#   define ALIGN_SUFFIX(N)
#endif

#define ALIGN_PREFIX_8          ALIGN_PREFIX(8)
#define ALIGN_PREFIX_16         ALIGN_PREFIX(16)
#define ALIGN_PREFIX_32         ALIGN_PREFIX(32)
#define ALIGN_PREFIX_64         ALIGN_PREFIX(64)

#define ALIGN_SUFFIX_8          ALIGN_SUFFIX(8)
#define ALIGN_SUFFIX_16         ALIGN_SUFFIX(16)
#define ALIGN_SUFFIX_32         ALIGN_SUFFIX(32)
#define ALIGN_SUFFIX_64         ALIGN_SUFFIX(64)

// address align
#define ADDR_ALGIN_8BYTES(p)    (((unsigned)(p) +  7) & 0xfffffff8ul)
#define ADDR_ALGIN_16BYTES(p)   (((unsigned)(p) + 15) & 0xfffffff0ul)
#define ADDR_ALGIN_32BYTES(p)   (((unsigned)(p) + 31) & 0xffffffe0ul)
#define ADDR_ALGIN_64BYTES(p)   (((unsigned)(p) + 63) & 0xffffffc0ul)

/* Define function attributes directive when available */
#if __GNUC__ >= 3
#define	REGPARM(num)	__attribute__((regparm(num)))
#else
#if defined (_MSC_VER) || defined(__BORLANDC__) || defined(__ICL)
#define	REGPARM(num)	__fastcall
//#define	REGPARM(num)
#else
#define	REGPARM(num)
#endif
#endif

// types

typedef signed char sint8;
typedef unsigned char uint8;

typedef signed short sint16;
typedef unsigned short uint16;

typedef signed int sint32;
typedef unsigned int uint32;

#ifdef _MSC_VER
  typedef signed __int64 sint64;
  typedef unsigned __int64 uint64;
#else
  typedef signed long long sint64;
  typedef unsigned long long uint64;
#endif

// types

typedef char BoolChar;          // bc
typedef short BoolShort;        // bw
typedef long BoolLong;          // bdw
#ifdef _MSC_VER
typedef __int64 BoolLongLong;   // bqw
#else
typedef long long BoolLongLong; // bqw
#endif

typedef int Boolean;            // b

Boolean Eqv(Boolean bArg1, Boolean bArg2);

Boolean Xor(Boolean bArg1, Boolean bArg2);


#if defined(_WIN32)

/* Ignore most warnings (back down to /W3) for poorly constructed headers
 */
#if defined(_MSC_VER) && _MSC_VER >= 1200
#   pragma warning(push, 3)
#endif

// totally kill this warning (debug info truncated to 255 chars etc...) on <= VC6
#if defined(_MSC_VER) && (_MSC_VER <= 1200)
#   pragma warning(disable : 4786)
#endif

/* Ignore Microsoft's interpretation of secure development
 * and the POSIX string handling API
 */
#if defined(_MSC_VER) && _MSC_VER >= 1400
#   ifndef _CRT_SECURE_NO_DEPRECATE
#       define _CRT_SECURE_NO_DEPRECATE
#   endif
#   pragma warning(disable: 4996)
#endif

/* Done with badly written headers
 */
#if defined(_MSC_VER) && _MSC_VER >= 1200
#pragma warning(pop)
#pragma warning(disable: 4996)
#endif

#endif  /* !WIN32 */

#ifdef __cplusplus
}
#endif

#endif  /* __UTILS_H__ */
