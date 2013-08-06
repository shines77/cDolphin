/*
   File:           bitbmob.cpp

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard mobility(MMX version).
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "utils.h"
#include "bitboard.h"
#include "bitbmob.h"

/***********************************************************
 方向定义：
  0  1  2
   \ | /
    \|/
 7 --+-- 3
    /|\
   / | \
  6  5  4
***********************************************************/

#ifdef _WIN32

static ALIGN_PREFIX(8) unsigned __int64 dir_mask0 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned __int64 dir_mask1 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned __int64 dir_mask3 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned __int64 c0f ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned __int64 c33 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned __int64 c55 ALIGN_SUFFIX(8);

static ALIGN_PREFIX(8) unsigned __int64 dir_mask2 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned __int64 dir_mask4 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned __int64 dir_mask5 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned __int64 dir_mask6 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned __int64 dir_mask7 ALIGN_SUFFIX(8);

#else

static ALIGN_PREFIX(8) unsigned long long dir_mask0 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned long long dir_mask1 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned long long dir_mask3 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned long long c0f ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned long long c33 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned long long c55 ALIGN_SUFFIX(8);

static ALIGN_PREFIX(8) unsigned long long dir_mask2 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned long long dir_mask4 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned long long dir_mask5 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned long long dir_mask6 ALIGN_SUFFIX(8);
static ALIGN_PREFIX(8) unsigned long long dir_mask7 ALIGN_SUFFIX(8);

#endif

void
init_mmx( void ) {

#ifdef _WIN32
	dir_mask0 = 0x007e7e7e7e7e7e00;
	dir_mask1 = 0x00ffffffffffff00;
	dir_mask3 = 0x7e7e7e7e7e7e7e7e;
	c0f       = 0x0f0f0f0f0f0f0f0f;
	c33       = 0x3333333333333333;
	c55       = 0x5555555555555555;
	dir_mask2 = 0x007e7e7e7e7e7e00;
	dir_mask4 = 0x7e7e7e7e7e7e7e7e;
	dir_mask5 = 0x007e7e7e7e7e7e00;
	dir_mask6 = 0x00ffffffffffff00;
	dir_mask7 = 0x007e7e7e7e7e7e00;

#else
	dir_mask0 = 0x007e7e7e7e7e7e00ULL;
	dir_mask1 = 0x00ffffffffffff00ULL;
	dir_mask3 = 0x7e7e7e7e7e7e7e7eULL;
	c0f       = 0x0f0f0f0f0f0f0f0fULL;
	c33       = 0x3333333333333333ULL;
	c55       = 0x5555555555555555ULL;
	dir_mask2 = 0x007e7e7e7e7e7e00ULL;
	dir_mask4 = 0x7e7e7e7e7e7e7e7eULL;
	dir_mask5 = 0x007e7e7e7e7e7e00ULL;
	dir_mask6 = 0x00ffffffffffff00ULL;
	dir_mask7 = 0x007e7e7e7e7e7e00ULL;

#endif

}

void
bitboard_gen_movelist( const BitBoard my_bits,
					  const BitBoard opp_bits,
					  BitBoard *movelist_bits ) {

    __asm {
		movd        mm7, my_bits.low        ; dword ptr [esp+24]
		movd        mm6, opp_bits.low       ; dword ptr [esp+2C]
		punpckldq   mm7, my_bits.high       ; dword ptr [esp+20]
		punpckldq   mm6, opp_bits.high      ; dword ptr [esp+28]
		mov         esi, my_bits.high       ; dword ptr [esp+20]
		mov         edi, opp_bits.high      ; dword ptr [esp+28]
		mov         eax, esi
		movq        mm0, mm7
		movq        mm5, dir_mask3          ; 0x7e7e7e7e7e7e7e7e
		shr         eax, 1
		psrlq       mm0, 8
		and         edi, 0x7e7e7e7e
		pand        mm5, mm6
		and         eax, edi
		pand        mm0, mm6
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 1
		psrlq       mm0, 8
		mov         ecx, edi
		movq        mm3, mm6
		and         eax, edi
		pand        mm0, mm6
		shr         ecx, 1
		psrlq       mm3, 8
		or          eax, edx
		por         mm0, mm1
		and         ecx, edi
		pand        mm3, mm6
		mov         edx, eax
		movq        mm4, mm0
		shr         eax, 2
		psrlq       mm0, 16
		and         eax, ecx
		pand        mm0, mm3
		or          edx, eax
		por         mm4, mm0
		shr         eax, 2
		psrlq       mm0, 16
		and         eax, ecx
		pand        mm0, mm3
		or          eax, edx
		por         mm4, mm0
		shr         eax, 1
		psrlq       mm4, 8
		movq        mm0, mm7
		add         esi, esi
		psllq       mm0, 8
		and         esi, edi
		pand        mm0, mm6
		mov         edx, esi
		movq        mm1, mm0
		add         esi, esi
		psllq       mm0, 8
		and         esi, edi
		pand        mm0, mm6
		or          esi, edx
		por         mm0, mm1
		add         ecx, ecx
		psllq       mm3, 8
		mov         edx, esi
		movq        mm1, mm0
		shl         esi, 2
		psllq       mm0, 16
		and         esi, ecx
		pand        mm0, mm3
		or          edx, esi
		por         mm1, mm0
		shl         esi, 2
		psllq       mm0, 16
		and         esi, ecx
		pand        mm0, mm3
		or          esi, edx
		por         mm0, mm1
		add         esi, esi
		psllq       mm0, 8
		or          esi, eax
		por         mm4, mm0
		movq        mm0, mm7
		movd        mm1, esi
		psrlq       mm0, 7
		psllq       mm1, 32
		pand        mm0, mm5
		por         mm4, mm1
		movq        mm1, mm0
		psrlq       mm0, 7
		pand        mm0, mm5
		movq        mm3, mm5
		por         mm0, mm1
		psrlq       mm3, 7
		movq        mm1, mm0
		pand        mm3, mm5
		psrlq       mm0, 14
		pand        mm0, mm3
		por         mm1, mm0
		movd        edi, mm5
		psrlq       mm0, 14
		movd        esi, mm7
		pand        mm0, mm3
		mov         ecx, edi
		por         mm0, mm1
		shr         ecx, 1
		psrlq       mm0, 7
		and         ecx, edi
		por         mm4, mm0
		mov         eax, esi
		movq        mm0, mm7
		shr         eax, 1
		psllq       mm0, 7
		and         eax, edi
		pand        mm0, mm5
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 1
		psllq       mm0, 7
		and         eax, edi
		pand        mm0, mm5
		or          eax, edx
		por         mm0, mm1
		psllq       mm3, 7
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 2
		psllq       mm0, 14
		and         eax, ecx
		pand        mm0, mm3
		or          edx, eax
		por         mm1, mm0
		shr         eax, 2
		psllq       mm0, 14
		and         eax, ecx
		pand        mm0, mm3
		or          eax, edx
		por         mm0, mm1
		shr         eax, 1
		psllq       mm0, 7
		por         mm4, mm0
		movq        mm0, mm7
		add         esi, esi
		psrlq       mm0, 9
		and         esi, edi
		pand        mm0, mm5
		mov         edx, esi
		movq        mm1, mm0
		add         esi, esi
		psrlq       mm0, 9
		and         esi, edi
		pand        mm0, mm5
		movq        mm3, mm5
		or          esi, edx
		por         mm0, mm1
		psrlq       mm3, 9
		mov         edx, esi
		movq        mm1, mm0
		add         ecx, ecx
		pand        mm3, mm5
		shl         esi, 2
		psrlq       mm0, 18
		and         esi, ecx
		pand        mm0, mm3
		or          edx, esi
		por         mm1, mm0
		shl         esi, 2
		psrlq       mm0, 18
		and         esi, ecx
		pand        mm0, mm3
		or          esi, edx
		por         mm0, mm1
		add         esi, esi
		psrlq       mm0, 9
		or          esi, eax
		por         mm4, mm0
		movq        mm0, mm7
		movd        mm1, esi
		psllq       mm0, 9
		por         mm4, mm1
		pand        mm0, mm5
		movq        mm1, mm0
		psllq       mm0, 9
		pand        mm0, mm5
		por         mm0, mm1
		psllq       mm3, 9
		movq        mm1, mm0
		psllq       mm0, 18
		pand        mm0, mm3
		por         mm1, mm0
		psllq       mm0, 18
		pand        mm0, mm3
		por         mm0, mm1
		psllq       mm0, 9
		por         mm4, mm0
		por         mm7, mm6
		pandn       mm7, mm4

		mov         eax, movelist_bits
		movq        qword ptr[eax], mm7
		/*
		movd        dword ptr[eax], mm7
		psrlq       mm7, 32
		movd        dword ptr[eax+4], mm7
		//*/

		/*
		movq        mm1, mm7
		psrld       mm7, 1
		pand        mm7, c55
		psubd       mm1, mm7
		movq        mm7, mm1
		psrld       mm1, 2
		pand        mm7, c33
		pand        mm1, c33
		paddd       mm7, mm1
		movq        mm1, mm7
		psrld       mm7, 4
		paddd       mm7, mm1
		pand        mm7, c0f
		movq        mm1, mm7
		psrlq       mm7, 32
		paddd       mm7, mm1
		movd        eax, mm7
		imul        eax, 01010101B
		shr         eax, 24
		//*/
		emms
	}
}

void
bitboard_gen_movelist2( const BitBoard my_bits,
					   const BitBoard opp_bits,
					   BitBoard *movelist_bits ) {
    __asm {

        /* Ready for init data */
		/*
        mov     ebx, my_bits.high       ;
        mov     ecx, my_bits.low        ;
        mov     edi, opp_bits.high      ;
        mov     esi, opp_bits.low       ;

        movd    mm0, ebx                ;
        psllq   mm0, 32                 ;
        movd    mm3, ecx                ;
        por     mm0, mm3                ; mm0 is BitBoard of my_bits
        movd    mm1, edi                ;
        psllq   mm1, 32                 ;
        movd    mm4, esi                ;
        por     mm1, mm4                ; mm1 is BitBoard of opp_bits
		pxor    mm2, mm2                ; mm2 <- 0x0000000000000000
		//*/
		///*
		mov         ecx, my_bits.low        ;
        mov         ebx, my_bits.high       ;
		mov         esi, opp_bits.low       ;
		mov         edi, opp_bits.high      ;
		movd        mm0, ecx                ;
		movd        mm1, esi                ;
		punpckldq   mm0, my_bits.high       ;
		punpckldq   mm1, opp_bits.high      ;
        pxor        mm2, mm2                ; mm2 <- 0x0000000000000000
		//*/

        /* shift=-9   rowDelta=-1   colDelta=-1 */
        /* shift=+9   rowDelta=+1   colDelta=+1 */

        /* Disc #1, flip direction 0. */
        /* Disc #1, flip direction 7. */
        movq    mm3, mm1                ; mm3 <- opp_bits
        movq    mm4, mm0                ; mm4 <- my_bits
        movq    mm6, mm0                ; mm6 <- backup of my_bits
        pand    mm3, dir_mask0          ; 0x007e7e7e7e7e7e00

        push    esi                     ;
        psllq   mm4, 9                  ;
        psrlq   mm6, 9                  ;
        push    edi                     ;
        pand    mm4, mm3                ;
        pand    mm6, mm3                ;
        push    ecx                     ;

        /* Disc #2, flip direction 0. */
        /* Disc #2, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        psllq   mm5, 9                  ;
        psrlq   mm7, 9                  ;
        push    ebx                     ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        and     edi, 0x7e7e7e7e         ; 0x7e7e7e7e
        and     esi, 0x7e7e7e7e         ; 0x7e7e7e7e

        por     mm4, mm5                ;
        por     mm6, mm7                ;
        shl     ebx, 1                  ;
        shl     ecx, 1                  ;

        /* Disc #3, flip direction 0. */
        /* Disc #3, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     ebx, edi                ;
        and     ecx, esi                ;
        psllq   mm5, 9                  ;
        psrlq   mm7, 9                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;

        /* Disc #4, flip direction 0. */
        /* Disc #4, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psllq   mm5, 9                  ;
        psrlq   mm7, 9                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;

        /* Disc #5, flip direction 0. */
        /* Disc #5, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psllq   mm5, 9                  ;
        psrlq   mm7, 9                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;

        /* Disc #6, flip direction 0. */
        /* Disc #6, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psrlq   mm7, 9                  ;
        psllq   mm5, 9                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm4, 9                  ;
        psrlq   mm6, 9                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        por     mm2, mm4                ;
        por     mm2, mm6                ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;

        /* shift=-8   rowDelta=-1   colDelta=0 */
        /* shift=+8   rowDelta=1   colDelta=0 */

        /* Disc #1, flip direction 1. */
        /* Disc #1, flip direction 6. */
        movq    mm3, mm1                ;
        movq    mm4, mm0                ;
        movq    mm6, mm0                ;
        pand    mm3, dir_mask1          ; 0x00ffffffffffff00;

        psllq   mm4, 8                  ;
        psrlq   mm6, 8                  ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        pand    mm4, mm3                ;
        pand    mm6, mm3                ;
        and     eax, edi                ;
        and     edx, esi                ;
        /* Disc #2, flip direction 1. */
        /* Disc #2, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        shl     ebx, 1                  ;
        shl     ecx, 1                  ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;

        /* serialize here: add horizontal shl flips. */

        movd    mm5, ebx                ;
        psllq   mm5, 32                 ;
        movd    mm7, ecx                ;
        por     mm5, mm7                ;
        por     mm2, mm5                ;

        /* Disc #3, flip direction 1. */
        /* Disc #3, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        pop     ebx                     ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        pop     ecx                     ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        push    ecx                     ;

        /* Disc #4, flip direction 1. */
        /* Disc #4, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        push    ebx                     ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        shr     ebx, 1                  ;
        shr     ecx, 1                  ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        and     ebx, edi                ;
        and     ecx, esi                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;

        /* Disc #5, flip direction 1. */
        /* Disc #5, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;

        /* Disc #6, flip direction 1. */
        /* Disc #6, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm4, 8                  ;
        psrlq   mm6, 8                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        por     mm2, mm4                ;
        por     mm2, mm6                ;

        /* shift=-7   rowDelta=-1   colDelta=1 */
        /* shift=+7   rowDelta=1   colDelta=-1 */

        /* Disc #1, flip direction 2. */
        /* Disc #1, flip direction 5. */
        movq    mm3, mm1                ;
        movq    mm4, mm0                ;
        movq    mm6, mm0                ;
        pand    mm3, dir_mask0          ; 0x007e7e7e7e7e7e00;

        psllq   mm4, 7                  ;
        psrlq   mm6, 7                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm4, mm3                ;
        pand    mm6, mm3                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;

        /* Disc #2, flip direction 2. */
        /* Disc #2, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;

        /* Disc #3, flip direction 2. */
        /* Disc #3, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;

        /* Disc #4, flip direction 2. */
        /* Disc #4, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;

        /* Disc #5, flip direction 2. */
        /* Disc #5, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shr     ebx, 1                  ;
        shr     ecx, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;

        /* serialize here: add horizontal shr flips. */

        movd    mm5, ebx                ;
        psllq   mm5, 32                 ;
        movd    mm7, ecx                ;
        por     mm5, mm7                ;
        por     mm2, mm5                ;
        pop     ebx                     ;

        /* Disc #6, flip direction 2. */
        /* Disc #6, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        pop     ecx                     ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        pop     edi                     ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        pop     esi                     ;
        psllq   mm4, 7                  ;
        psrlq   mm6, 7                  ;
        por     mm2, mm4                ;
        por     mm2, mm6                ;

        /* mm2 is the pseudo-feasible moves at this point. */
        /* Let mm7 be the feasible moves, i.e., mm2 restricted to empty squares. */

        movq    mm7, mm0                ;
        por     mm7, mm1                ;
        pandn   mm7, mm2                ;

		//movq    qword ptr[movelist_bits], mm7
		mov     eax, movelist_bits
		movq    qword ptr[eax], mm7

        /* Count the moves, i.e., the number of bits set in mm7. */

		/*
        movq    mm1, mm7                ;
        psrld   mm7, 1                  ;
        pand    mm7, c55                ; c55 = 0x5555555555555555
        psubd   mm1, mm7                ;
        movq    mm7, mm1                ;
        psrld   mm1, 2                  ;
        pand    mm7, c33                ; c33 = 0x3333333333333333;
        pand    mm1, c33                ; c33 = 0x3333333333333333;
        paddd   mm7, mm1                ;
        movq    mm1, mm7                ;
        psrld   mm7, 4                  ;
        paddd   mm7, mm1                ;
        pand    mm7, c0f                ; c0f = 0x0f0f0f0f0f0f0f0f;

        movq    mm1, mm7                ;
        psrld   mm7, 8                  ;
        paddd   mm7, mm1                ;
        movq    mm1, mm7                ;
        psrld   mm7, 16                 ;
        paddd   mm7, mm1                ;
        movq    mm1, mm7                ;
        psrlq   mm7, 32                 ;
        paddd   mm7, mm1                ;
        movd    eax, mm7                ;
        and     eax, 63                 ;
        mov     count, eax              ;
		*/

        emms                            ;
    }
}

#if 0

__declspec(naked)
int
bitboard_mobility( const BitBoard my_bits,
				  const BitBoard opp_bits ) {

	__asm {
		sub         esp, 0x0000001C
		//mov         dword ptr [esp+0x10], ebx
		mov         dword ptr [esp+0x14], esi
		mov         dword ptr [esp+0x18], edi
		movd        mm7, dword ptr [esp+0x20]        ; my_bits.low
		movd        mm6, dword ptr [esp+0x28]        ; opp_bits.low
		punpckldq   mm7, dword ptr [esp+0x24]        ; my_bits.high
		punpckldq   mm6, dword ptr [esp+0x2C]        ; opp_bits.high
		mov         esi, dword ptr [esp+0x24]        ; my_bits.high
		mov         edi, dword ptr [esp+0x2C]        ; opp_bits.high
		mov         eax, esi
		movq        mm0, mm7
		movq        mm5, dir_mask3          ; 0x7e7e7e7e7e7e7e7e
		shr         eax, 1
		psrlq       mm0, 8
		and         edi, 0x7e7e7e7e
		pand        mm5, mm6
		and         eax, edi
		pand        mm0, mm6
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 1
		psrlq       mm0, 8
		mov         ecx, edi
		movq        mm3, mm6
		and         eax, edi
		pand        mm0, mm6
		shr         ecx, 1
		psrlq       mm3, 8
		or          eax, edx
		por         mm0, mm1
		and         ecx, edi
		pand        mm3, mm6
		mov         edx, eax
		movq        mm4, mm0
		shr         eax, 2
		psrlq       mm0, 16
		and         eax, ecx
		pand        mm0, mm3
		or          edx, eax
		por         mm4, mm0
		shr         eax, 2
		psrlq       mm0, 16
		and         eax, ecx
		pand        mm0, mm3
		or          eax, edx
		por         mm4, mm0
		shr         eax, 1
		psrlq       mm4, 8
		movq        mm0, mm7
		add         esi, esi
		psllq       mm0, 8
		and         esi, edi
		pand        mm0, mm6
		mov         edx, esi
		movq        mm1, mm0
		add         esi, esi
		psllq       mm0, 8
		and         esi, edi
		pand        mm0, mm6
		or          esi, edx
		por         mm0, mm1
		add         ecx, ecx
		psllq       mm3, 8
		mov         edx, esi
		movq        mm1, mm0
		shl         esi, 2
		psllq       mm0, 16
		and         esi, ecx
		pand        mm0, mm3
		or          edx, esi
		por         mm1, mm0
		shl         esi, 2
		psllq       mm0, 16
		and         esi, ecx
		pand        mm0, mm3
		or          esi, edx
		por         mm0, mm1
		add         esi, esi
		psllq       mm0, 8
		or          esi, eax
		por         mm4, mm0
		movq        mm0, mm7
		movd        mm1, esi
		psrlq       mm0, 7
		psllq       mm1, 32
		pand        mm0, mm5
		por         mm4, mm1
		movq        mm1, mm0
		psrlq       mm0, 7
		pand        mm0, mm5
		movq        mm3, mm5
		por         mm0, mm1
		psrlq       mm3, 7
		movq        mm1, mm0
		pand        mm3, mm5
		psrlq       mm0, 14
		pand        mm0, mm3
		por         mm1, mm0
		movd        edi, mm5
		psrlq       mm0, 14
		movd        esi, mm7
		pand        mm0, mm3
		mov         ecx, edi
		por         mm0, mm1
		shr         ecx, 1
		psrlq       mm0, 7
		and         ecx, edi
		por         mm4, mm0
		mov         eax, esi
		movq        mm0, mm7
		shr         eax, 1
		psllq       mm0, 7
		and         eax, edi
		pand        mm0, mm5
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 1
		psllq       mm0, 7
		and         eax, edi
		pand        mm0, mm5
		or          eax, edx
		por         mm0, mm1
		psllq       mm3, 7
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 2
		psllq       mm0, 14
		and         eax, ecx
		pand        mm0, mm3
		or          edx, eax
		por         mm1, mm0
		shr         eax, 2
		psllq       mm0, 14
		and         eax, ecx
		pand        mm0, mm3
		or          eax, edx
		por         mm0, mm1
		shr         eax, 1
		psllq       mm0, 7
		por         mm4, mm0
		movq        mm0, mm7
		add         esi, esi
		psrlq       mm0, 9
		and         esi, edi
		pand        mm0, mm5
		mov         edx, esi
		movq        mm1, mm0
		add         esi, esi
		psrlq       mm0, 9
		and         esi, edi
		pand        mm0, mm5
		movq        mm3, mm5
		or          esi, edx
		por         mm0, mm1
		psrlq       mm3, 9
		mov         edx, esi
		movq        mm1, mm0
		add         ecx, ecx
		pand        mm3, mm5
		shl         esi, 2
		psrlq       mm0, 18
		and         esi, ecx
		pand        mm0, mm3
		or          edx, esi
		por         mm1, mm0
		shl         esi, 2
		psrlq       mm0, 18
		and         esi, ecx
		pand        mm0, mm3
		or          esi, edx
		por         mm0, mm1
		add         esi, esi
		psrlq       mm0, 9
		or          esi, eax
		por         mm4, mm0
		movq        mm0, mm7
		movd        mm1, esi
		psllq       mm0, 9
		por         mm4, mm1
		pand        mm0, mm5
		movq        mm1, mm0
		psllq       mm0, 9
		pand        mm0, mm5
		por         mm0, mm1
		psllq       mm3, 9
		movq        mm1, mm0
		psllq       mm0, 18
		pand        mm0, mm3
		por         mm1, mm0
		psllq       mm0, 18
		pand        mm0, mm3
		por         mm0, mm1
		psllq       mm0, 9
		por         mm4, mm0
		por         mm7, mm6
		pandn       mm7, mm4

		movq        mm1, mm7
		psrld       mm7, 1
		pand        mm7, c55
		psubd       mm1, mm7
		movq        mm7, mm1
		psrld       mm1, 2
		pand        mm7, c33
		pand        mm1, c33
		paddd       mm7, mm1
		movq        mm1, mm7
		psrld       mm7, 4
		paddd       mm7, mm1
		pand        mm7, c0f
		movq        mm1, mm7
		psrlq       mm7, 32
		paddd       mm7, mm1
		movd        eax, mm7
		imul        eax, 01010101H
		shr         eax, 24
		emms

		//mov         ebx, dword ptr [esp+0x10]
		mov         esi, dword ptr [esp+0x14]
		mov         edi, dword ptr [esp+0x18]
		add         esp, 0x0000001C
		ret
	}
}

__declspec(naked)
int
weighted_mobility( const BitBoard my_bits,
				  const BitBoard opp_bits ) {

	__asm {
		sub         esp, 0x0000001C
		//mov         dword ptr [esp+0x10], ebx
		mov         dword ptr [esp+0x14], esi
		mov         dword ptr [esp+0x18], edi
		movd        mm7, dword ptr [esp+0x20]        ; my_bits.low
		movd        mm6, dword ptr [esp+0x28]        ; opp_bits.low
		punpckldq   mm7, dword ptr [esp+0x24]        ; my_bits.high
		punpckldq   mm6, dword ptr [esp+0x2C]        ; opp_bits.high
		mov         esi, dword ptr [esp+0x24]        ; my_bits.high
		mov         edi, dword ptr [esp+0x2C]        ; opp_bits.high
		mov         eax, esi
		movq        mm0, mm7
		movq        mm5, dir_mask3          ; 0x7e7e7e7e7e7e7e7e
		shr         eax, 1
		psrlq       mm0, 8
		and         edi, 0x7e7e7e7e
		pand        mm5, mm6
		and         eax, edi
		pand        mm0, mm6
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 1
		psrlq       mm0, 8
		mov         ecx, edi
		movq        mm3, mm6
		and         eax, edi
		pand        mm0, mm6
		shr         ecx, 1
		psrlq       mm3, 8
		or          eax, edx
		por         mm0, mm1
		and         ecx, edi
		pand        mm3, mm6
		mov         edx, eax
		movq        mm4, mm0
		shr         eax, 2
		psrlq       mm0, 16
		and         eax, ecx
		pand        mm0, mm3
		or          edx, eax
		por         mm4, mm0
		shr         eax, 2
		psrlq       mm0, 16
		and         eax, ecx
		pand        mm0, mm3
		or          eax, edx
		por         mm4, mm0
		shr         eax, 1
		psrlq       mm4, 8
		movq        mm0, mm7
		add         esi, esi
		psllq       mm0, 8
		and         esi, edi
		pand        mm0, mm6
		mov         edx, esi
		movq        mm1, mm0
		add         esi, esi
		psllq       mm0, 8
		and         esi, edi
		pand        mm0, mm6
		or          esi, edx
		por         mm0, mm1
		add         ecx, ecx
		psllq       mm3, 8
		mov         edx, esi
		movq        mm1, mm0
		shl         esi, 2
		psllq       mm0, 16
		and         esi, ecx
		pand        mm0, mm3
		or          edx, esi
		por         mm1, mm0
		shl         esi, 2
		psllq       mm0, 16
		and         esi, ecx
		pand        mm0, mm3
		or          esi, edx
		por         mm0, mm1
		add         esi, esi
		psllq       mm0, 8
		or          esi, eax
		por         mm4, mm0
		movq        mm0, mm7
		movd        mm1, esi
		psrlq       mm0, 7
		psllq       mm1, 32
		pand        mm0, mm5
		por         mm4, mm1
		movq        mm1, mm0
		psrlq       mm0, 7
		pand        mm0, mm5
		movq        mm3, mm5
		por         mm0, mm1
		psrlq       mm3, 7
		movq        mm1, mm0
		pand        mm3, mm5
		psrlq       mm0, 14
		pand        mm0, mm3
		por         mm1, mm0
		movd        edi, mm5
		psrlq       mm0, 14
		movd        esi, mm7
		pand        mm0, mm3
		mov         ecx, edi
		por         mm0, mm1
		shr         ecx, 1
		psrlq       mm0, 7
		and         ecx, edi
		por         mm4, mm0
		mov         eax, esi
		movq        mm0, mm7
		shr         eax, 1
		psllq       mm0, 7
		and         eax, edi
		pand        mm0, mm5
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 1
		psllq       mm0, 7
		and         eax, edi
		pand        mm0, mm5
		or          eax, edx
		por         mm0, mm1
		psllq       mm3, 7
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 2
		psllq       mm0, 14
		and         eax, ecx
		pand        mm0, mm3
		or          edx, eax
		por         mm1, mm0
		shr         eax, 2
		psllq       mm0, 14
		and         eax, ecx
		pand        mm0, mm3
		or          eax, edx
		por         mm0, mm1
		shr         eax, 1
		psllq       mm0, 7
		por         mm4, mm0
		movq        mm0, mm7
		add         esi, esi
		psrlq       mm0, 9
		and         esi, edi
		pand        mm0, mm5
		mov         edx, esi
		movq        mm1, mm0
		add         esi, esi
		psrlq       mm0, 9
		and         esi, edi
		pand        mm0, mm5
		movq        mm3, mm5
		or          esi, edx
		por         mm0, mm1
		psrlq       mm3, 9
		mov         edx, esi
		movq        mm1, mm0
		add         ecx, ecx
		pand        mm3, mm5
		shl         esi, 2
		psrlq       mm0, 18
		and         esi, ecx
		pand        mm0, mm3
		or          edx, esi
		por         mm1, mm0
		shl         esi, 2
		psrlq       mm0, 18
		and         esi, ecx
		pand        mm0, mm3
		or          esi, edx
		por         mm0, mm1
		add         esi, esi
		psrlq       mm0, 9
		or          esi, eax
		por         mm4, mm0
		movq        mm0, mm7
		movd        mm1, esi
		psllq       mm0, 9
		por         mm4, mm1
		pand        mm0, mm5
		movq        mm1, mm0
		psllq       mm0, 9
		pand        mm0, mm5
		por         mm0, mm1
		psllq       mm3, 9
		movq        mm1, mm0
		psllq       mm0, 18
		pand        mm0, mm3
		por         mm1, mm0
		psllq       mm0, 18
		pand        mm0, mm3
		por         mm0, mm1
		psllq       mm0, 9
		por         mm4, mm0
		por         mm7, mm6
		pandn       mm7, mm4

		movq        mm1, mm7
		psrld       mm7, 1
		pand        mm7, c55
		psubd       mm1, mm7
		movq        mm7, mm1
		psrld       mm1, 2
		pand        mm7, c33
		pand        mm1, c33
		paddd       mm7, mm1
		movq        mm1, mm7
		psrld       mm7, 4
		paddd       mm7, mm1
		pand        mm7, c0f
		movq        mm1, mm7
		psrlq       mm7, 32
		paddd       mm7, mm1
		movd        eax, mm7
		imul        eax, 01010101H
		shr         eax, 24
		shl         eax, 7
		emms

		//mov         ebx, dword ptr [esp+0x10]
		mov         esi, dword ptr [esp+0x14]
		mov         edi, dword ptr [esp+0x18]
		add         esp, 0x0000001C
		ret
	}
}

#else

int
bitboard_mobility( const BitBoard my_bits,
				  const BitBoard opp_bits ) {
	unsigned int count;

	__asm {
		movd        mm7, my_bits.low        ; dword ptr [esp+0x24]
		movd        mm6, opp_bits.low       ; dword ptr [esp+0x2C]
		punpckldq   mm7, my_bits.high       ; dword ptr [esp+0x20]
		punpckldq   mm6, opp_bits.high      ; dword ptr [esp+0x28]
		mov         esi, my_bits.high       ; dword ptr [esp+0x20]
		mov         edi, opp_bits.high      ; dword ptr [esp+0x28]
		mov         eax, esi
		movq        mm0, mm7
		movq        mm5, dir_mask3          ; 0x7e7e7e7e7e7e7e7e
		shr         eax, 1
		psrlq       mm0, 8
		and         edi, 0x7e7e7e7e
		pand        mm5, mm6
		and         eax, edi
		pand        mm0, mm6
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 1
		psrlq       mm0, 8
		mov         ecx, edi
		movq        mm3, mm6
		and         eax, edi
		pand        mm0, mm6
		shr         ecx, 1
		psrlq       mm3, 8
		or          eax, edx
		por         mm0, mm1
		and         ecx, edi
		pand        mm3, mm6
		mov         edx, eax
		movq        mm4, mm0
		shr         eax, 2
		psrlq       mm0, 16
		and         eax, ecx
		pand        mm0, mm3
		or          edx, eax
		por         mm4, mm0
		shr         eax, 2
		psrlq       mm0, 16
		and         eax, ecx
		pand        mm0, mm3
		or          eax, edx
		por         mm4, mm0
		shr         eax, 1
		psrlq       mm4, 8
		movq        mm0, mm7
		add         esi, esi
		psllq       mm0, 8
		and         esi, edi
		pand        mm0, mm6
		mov         edx, esi
		movq        mm1, mm0
		add         esi, esi
		psllq       mm0, 8
		and         esi, edi
		pand        mm0, mm6
		or          esi, edx
		por         mm0, mm1
		add         ecx, ecx
		psllq       mm3, 8
		mov         edx, esi
		movq        mm1, mm0
		shl         esi, 2
		psllq       mm0, 16
		and         esi, ecx
		pand        mm0, mm3
		or          edx, esi
		por         mm1, mm0
		shl         esi, 2
		psllq       mm0, 16
		and         esi, ecx
		pand        mm0, mm3
		or          esi, edx
		por         mm0, mm1
		add         esi, esi
		psllq       mm0, 8
		or          esi, eax
		por         mm4, mm0
		movq        mm0, mm7
		movd        mm1, esi
		psrlq       mm0, 7
		psllq       mm1, 32
		pand        mm0, mm5
		por         mm4, mm1
		movq        mm1, mm0
		psrlq       mm0, 7
		pand        mm0, mm5
		movq        mm3, mm5
		por         mm0, mm1
		psrlq       mm3, 7
		movq        mm1, mm0
		pand        mm3, mm5
		psrlq       mm0, 14
		pand        mm0, mm3
		por         mm1, mm0
		movd        edi, mm5
		psrlq       mm0, 14
		movd        esi, mm7
		pand        mm0, mm3
		mov         ecx, edi
		por         mm0, mm1
		shr         ecx, 1
		psrlq       mm0, 7
		and         ecx, edi
		por         mm4, mm0
		mov         eax, esi
		movq        mm0, mm7
		shr         eax, 1
		psllq       mm0, 7
		and         eax, edi
		pand        mm0, mm5
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 1
		psllq       mm0, 7
		and         eax, edi
		pand        mm0, mm5
		or          eax, edx
		por         mm0, mm1
		psllq       mm3, 7
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 2
		psllq       mm0, 14
		and         eax, ecx
		pand        mm0, mm3
		or          edx, eax
		por         mm1, mm0
		shr         eax, 2
		psllq       mm0, 14
		and         eax, ecx
		pand        mm0, mm3
		or          eax, edx
		por         mm0, mm1
		shr         eax, 1
		psllq       mm0, 7
		por         mm4, mm0
		movq        mm0, mm7
		add         esi, esi
		psrlq       mm0, 9
		and         esi, edi
		pand        mm0, mm5
		mov         edx, esi
		movq        mm1, mm0
		add         esi, esi
		psrlq       mm0, 9
		and         esi, edi
		pand        mm0, mm5
		movq        mm3, mm5
		or          esi, edx
		por         mm0, mm1
		psrlq       mm3, 9
		mov         edx, esi
		movq        mm1, mm0
		add         ecx, ecx
		pand        mm3, mm5
		shl         esi, 2
		psrlq       mm0, 18
		and         esi, ecx
		pand        mm0, mm3
		or          edx, esi
		por         mm1, mm0
		shl         esi, 2
		psrlq       mm0, 18
		and         esi, ecx
		pand        mm0, mm3
		or          esi, edx
		por         mm0, mm1
		add         esi, esi
		psrlq       mm0, 9
		or          esi, eax
		por         mm4, mm0
		movq        mm0, mm7
		movd        mm1, esi
		psllq       mm0, 9
		por         mm4, mm1
		pand        mm0, mm5
		movq        mm1, mm0
		psllq       mm0, 9
		pand        mm0, mm5
		por         mm0, mm1
		psllq       mm3, 9
		movq        mm1, mm0
		psllq       mm0, 18
		pand        mm0, mm3
		por         mm1, mm0
		psllq       mm0, 18
		pand        mm0, mm3
		por         mm0, mm1
		psllq       mm0, 9
		por         mm4, mm0
		por         mm7, mm6
		pandn       mm7, mm4

		movq        mm1, mm7
		psrld       mm7, 1
		pand        mm7, c55
		psubd       mm1, mm7
		movq        mm7, mm1
		psrld       mm1, 2
		pand        mm7, c33
		pand        mm1, c33
		paddd       mm7, mm1
		movq        mm1, mm7
		psrld       mm7, 4
		paddd       mm7, mm1
		pand        mm7, c0f
		movq        mm1, mm7
		psrlq       mm7, 32
		paddd       mm7, mm1
		movd        eax, mm7
		imul        eax, 01010101H
		shr         eax, 24
		mov         count, eax
		emms
	}
	return count;
}

int
weighted_mobility( const BitBoard my_bits,
				  const BitBoard opp_bits ) {
	unsigned int weighted_mob;

	__asm {
		movd        mm7, my_bits.low        ; dword ptr [esp+24]
		movd        mm6, opp_bits.low       ; dword ptr [esp+2C]
		punpckldq   mm7, my_bits.high       ; dword ptr [esp+20]
		punpckldq   mm6, opp_bits.high      ; dword ptr [esp+28]
		mov         esi, my_bits.high       ; dword ptr [esp+20]
		mov         edi, opp_bits.high      ; dword ptr [esp+28]
		mov         eax, esi
		movq        mm0, mm7
		movq        mm5, dir_mask3          ; 0x7e7e7e7e7e7e7e7e
		shr         eax, 1
		psrlq       mm0, 8
		and         edi, 0x7e7e7e7e
		pand        mm5, mm6
		and         eax, edi
		pand        mm0, mm6
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 1
		psrlq       mm0, 8
		mov         ecx, edi
		movq        mm3, mm6
		and         eax, edi
		pand        mm0, mm6
		shr         ecx, 1
		psrlq       mm3, 8
		or          eax, edx
		por         mm0, mm1
		and         ecx, edi
		pand        mm3, mm6
		mov         edx, eax
		movq        mm4, mm0
		shr         eax, 2
		psrlq       mm0, 16
		and         eax, ecx
		pand        mm0, mm3
		or          edx, eax
		por         mm4, mm0
		shr         eax, 2
		psrlq       mm0, 16
		and         eax, ecx
		pand        mm0, mm3
		or          eax, edx
		por         mm4, mm0
		shr         eax, 1
		psrlq       mm4, 8
		movq        mm0, mm7
		add         esi, esi
		psllq       mm0, 8
		and         esi, edi
		pand        mm0, mm6
		mov         edx, esi
		movq        mm1, mm0
		add         esi, esi
		psllq       mm0, 8
		and         esi, edi
		pand        mm0, mm6
		or          esi, edx
		por         mm0, mm1
		add         ecx, ecx
		psllq       mm3, 8
		mov         edx, esi
		movq        mm1, mm0
		shl         esi, 2
		psllq       mm0, 16
		and         esi, ecx
		pand        mm0, mm3
		or          edx, esi
		por         mm1, mm0
		shl         esi, 2
		psllq       mm0, 16
		and         esi, ecx
		pand        mm0, mm3
		or          esi, edx
		por         mm0, mm1
		add         esi, esi
		psllq       mm0, 8
		or          esi, eax
		por         mm4, mm0
		movq        mm0, mm7
		movd        mm1, esi
		psrlq       mm0, 7
		psllq       mm1, 32
		pand        mm0, mm5
		por         mm4, mm1
		movq        mm1, mm0
		psrlq       mm0, 7
		pand        mm0, mm5
		movq        mm3, mm5
		por         mm0, mm1
		psrlq       mm3, 7
		movq        mm1, mm0
		pand        mm3, mm5
		psrlq       mm0, 14
		pand        mm0, mm3
		por         mm1, mm0
		movd        edi, mm5
		psrlq       mm0, 14
		movd        esi, mm7
		pand        mm0, mm3
		mov         ecx, edi
		por         mm0, mm1
		shr         ecx, 1
		psrlq       mm0, 7
		and         ecx, edi
		por         mm4, mm0
		mov         eax, esi
		movq        mm0, mm7
		shr         eax, 1
		psllq       mm0, 7
		and         eax, edi
		pand        mm0, mm5
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 1
		psllq       mm0, 7
		and         eax, edi
		pand        mm0, mm5
		or          eax, edx
		por         mm0, mm1
		psllq       mm3, 7
		mov         edx, eax
		movq        mm1, mm0
		shr         eax, 2
		psllq       mm0, 14
		and         eax, ecx
		pand        mm0, mm3
		or          edx, eax
		por         mm1, mm0
		shr         eax, 2
		psllq       mm0, 14
		and         eax, ecx
		pand        mm0, mm3
		or          eax, edx
		por         mm0, mm1
		shr         eax, 1
		psllq       mm0, 7
		por         mm4, mm0
		movq        mm0, mm7
		add         esi, esi
		psrlq       mm0, 9
		and         esi, edi
		pand        mm0, mm5
		mov         edx, esi
		movq        mm1, mm0
		add         esi, esi
		psrlq       mm0, 9
		and         esi, edi
		pand        mm0, mm5
		movq        mm3, mm5
		or          esi, edx
		por         mm0, mm1
		psrlq       mm3, 9
		mov         edx, esi
		movq        mm1, mm0
		add         ecx, ecx
		pand        mm3, mm5
		shl         esi, 2
		psrlq       mm0, 18
		and         esi, ecx
		pand        mm0, mm3
		or          edx, esi
		por         mm1, mm0
		shl         esi, 2
		psrlq       mm0, 18
		and         esi, ecx
		pand        mm0, mm3
		or          esi, edx
		por         mm0, mm1
		add         esi, esi
		psrlq       mm0, 9
		or          esi, eax
		por         mm4, mm0
		movq        mm0, mm7
		movd        mm1, esi
		psllq       mm0, 9
		por         mm4, mm1
		pand        mm0, mm5
		movq        mm1, mm0
		psllq       mm0, 9
		pand        mm0, mm5
		por         mm0, mm1
		psllq       mm3, 9
		movq        mm1, mm0
		psllq       mm0, 18
		pand        mm0, mm3
		por         mm1, mm0
		psllq       mm0, 18
		pand        mm0, mm3
		por         mm0, mm1
		psllq       mm0, 9
		por         mm4, mm0
		por         mm7, mm6
		pandn       mm7, mm4

		movq        mm1, mm7
		psrld       mm7, 1
		pand        mm7, c55
		psubd       mm1, mm7
		movq        mm7, mm1
		psrld       mm1, 2
		pand        mm7, c33
		pand        mm1, c33
		paddd       mm7, mm1
		movq        mm1, mm7
		psrld       mm7, 4
		paddd       mm7, mm1
		pand        mm7, c0f
		movq        mm1, mm7
		psrlq       mm7, 32
		paddd       mm7, mm1
		movd        eax, mm7
		imul        eax, 01010101H
		shr         eax, 24
		shl         eax, 7
		mov         weighted_mob, eax
		emms
	}
	return weighted_mob;
}

#endif

int
bitboard_mobility2( const BitBoard my_bits,
				  const BitBoard opp_bits ) {
	unsigned int count;

    __asm {

        /* Ready for init data */
        mov     ebx, my_bits.high       ;
        mov     ecx, my_bits.low        ;
        mov     edi, opp_bits.high      ;
        mov     esi, opp_bits.low       ;

        movd    mm0, ebx                ;
        psllq   mm0, 32                 ;
        movd    mm3, ecx                ;
        por     mm0, mm3                ; mm0 is BitBoard of my_bits
        movd    mm1, edi                ;
        psllq   mm1, 32                 ;
        movd    mm4, esi                ;
        por     mm1, mm4                ; mm1 is BitBoard of opp_bits
        pxor    mm2, mm2                ; mm2 <- 0x0000000000000000

        /* shift=-9   rowDelta=-1   colDelta=-1 */
        /* shift=+9   rowDelta=+1   colDelta=+1 */

        /* Disc #1, flip direction 0. */
        /* Disc #1, flip direction 7. */
        movq    mm3, mm1                ; mm3 <- opp_bits
        movq    mm4, mm0                ; mm4 <- my_bits
        movq    mm6, mm0                ; mm6 <- backup of my_bits
        pand    mm3, dir_mask0          ; 0x007e7e7e7e7e7e00

        push    esi                     ;
        psllq   mm4, 9                  ;
        psrlq   mm6, 9                  ;
        push    edi                     ;
        pand    mm4, mm3                ;
        pand    mm6, mm3                ;
        push    ecx                     ;

        /* Disc #2, flip direction 0. */
        /* Disc #2, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        psllq   mm5, 9                  ;
        psrlq   mm7, 9                  ;
        push    ebx                     ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        and     edi, 0x7e7e7e7e         ; 0x7e7e7e7e
        and     esi, 0x7e7e7e7e         ; 0x7e7e7e7e

        por     mm4, mm5                ;
        por     mm6, mm7                ;
        shl     ebx, 1                  ;
        shl     ecx, 1                  ;

        /* Disc #3, flip direction 0. */
        /* Disc #3, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     ebx, edi                ;
        and     ecx, esi                ;
        psllq   mm5, 9                  ;
        psrlq   mm7, 9                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;

        /* Disc #4, flip direction 0. */
        /* Disc #4, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psllq   mm5, 9                  ;
        psrlq   mm7, 9                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;

        /* Disc #5, flip direction 0. */
        /* Disc #5, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psllq   mm5, 9                  ;
        psrlq   mm7, 9                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;

        /* Disc #6, flip direction 0. */
        /* Disc #6, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psrlq   mm7, 9                  ;
        psllq   mm5, 9                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm4, 9                  ;
        psrlq   mm6, 9                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        por     mm2, mm4                ;
        por     mm2, mm6                ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;

        /* shift=-8   rowDelta=-1   colDelta=0 */
        /* shift=+8   rowDelta=1   colDelta=0 */

        /* Disc #1, flip direction 1. */
        /* Disc #1, flip direction 6. */
        movq    mm3, mm1                ;
        movq    mm4, mm0                ;
        movq    mm6, mm0                ;
        pand    mm3, dir_mask1          ; 0x00ffffffffffff00;

        psllq   mm4, 8                  ;
        psrlq   mm6, 8                  ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        pand    mm4, mm3                ;
        pand    mm6, mm3                ;
        and     eax, edi                ;
        and     edx, esi                ;
        /* Disc #2, flip direction 1. */
        /* Disc #2, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        shl     ebx, 1                  ;
        shl     ecx, 1                  ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;

        /* serialize here: add horizontal shl flips. */

        movd    mm5, ebx                ;
        psllq   mm5, 32                 ;
        movd    mm7, ecx                ;
        por     mm5, mm7                ;
        por     mm2, mm5                ;

        /* Disc #3, flip direction 1. */
        /* Disc #3, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        pop     ebx                     ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        pop     ecx                     ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        push    ecx                     ;

        /* Disc #4, flip direction 1. */
        /* Disc #4, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        push    ebx                     ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        shr     ebx, 1                  ;
        shr     ecx, 1                  ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        and     ebx, edi                ;
        and     ecx, esi                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;

        /* Disc #5, flip direction 1. */
        /* Disc #5, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;

        /* Disc #6, flip direction 1. */
        /* Disc #6, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm4, 8                  ;
        psrlq   mm6, 8                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        por     mm2, mm4                ;
        por     mm2, mm6                ;

        /* shift=-7   rowDelta=-1   colDelta=1 */
        /* shift=+7   rowDelta=1   colDelta=-1 */

        /* Disc #1, flip direction 2. */
        /* Disc #1, flip direction 5. */
        movq    mm3, mm1                ;
        movq    mm4, mm0                ;
        movq    mm6, mm0                ;
        pand    mm3, dir_mask0          ; 0x007e7e7e7e7e7e00;

        psllq   mm4, 7                  ;
        psrlq   mm6, 7                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm4, mm3                ;
        pand    mm6, mm3                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;

        /* Disc #2, flip direction 2. */
        /* Disc #2, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;

        /* Disc #3, flip direction 2. */
        /* Disc #3, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;

        /* Disc #4, flip direction 2. */
        /* Disc #4, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;

        /* Disc #5, flip direction 2. */
        /* Disc #5, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shr     ebx, 1                  ;
        shr     ecx, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;

        /* serialize here: add horizontal shr flips. */

        movd    mm5, ebx                ;
        psllq   mm5, 32                 ;
        movd    mm7, ecx                ;
        por     mm5, mm7                ;
        por     mm2, mm5                ;
        pop     ebx                     ;

        /* Disc #6, flip direction 2. */
        /* Disc #6, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        pop     ecx                     ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        pop     edi                     ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        pop     esi                     ;
        psllq   mm4, 7                  ;
        psrlq   mm6, 7                  ;
        por     mm2, mm4                ;
        por     mm2, mm6                ;

        /* mm2 is the pseudo-feasible moves at this point. */
        /* Let mm7 be the feasible moves, i.e., mm2 restricted to empty squares. */

        movq    mm7, mm0                ;
        por     mm7, mm1                ;
        pandn   mm7, mm2                ;

        /* Count the moves, i.e., the number of bits set in mm7. */

        movq    mm1, mm7                ;
        psrld   mm7, 1                  ;
        pand    mm7, c55                ; c55 = 0x5555555555555555
        psubd   mm1, mm7                ;
        movq    mm7, mm1                ;
        psrld   mm1, 2                  ;
        pand    mm7, c33                ; c33 = 0x3333333333333333;
        pand    mm1, c33                ; c33 = 0x3333333333333333;
        paddd   mm7, mm1                ;
        movq    mm1, mm7                ;
        psrld   mm7, 4                  ;
        paddd   mm7, mm1                ;
        pand    mm7, c0f                ; c0f = 0x0f0f0f0f0f0f0f0f;

        movq    mm1, mm7                ;
        psrld   mm7, 8                  ;
        paddd   mm7, mm1                ;
        movq    mm1, mm7                ;
        psrld   mm7, 16                 ;
        paddd   mm7, mm1                ;
        movq    mm1, mm7                ;
        psrlq   mm7, 32                 ;
        paddd   mm7, mm1                ;
        movd    eax, mm7                ;
        and     eax, 63                 ;
        mov     count, eax              ;

        emms                            ;
    }
	return count;
}

int
weighted_mobility2( const BitBoard my_bits,
				  const BitBoard opp_bits ) {
	unsigned int weighted_mobility;
    __asm {

        /* Ready for init data */
        mov     ebx, my_bits.high       ;
        mov     ecx, my_bits.low        ;
        mov     edi, opp_bits.high      ;
        mov     esi, opp_bits.low       ;

        movd    mm0, ebx                ;
        psllq   mm0, 32                 ;
        movd    mm3, ecx                ;
        por     mm0, mm3                ; mm0 is BitBoard of my_bits
        movd    mm1, edi                ;
        psllq   mm1, 32                 ;
        movd    mm4, esi                ;
        por     mm1, mm4                ; mm1 is BitBoard of opp_bits
        pxor    mm2, mm2                ; mm2 <- 0x0000000000000000

        /* shift=-9   rowDelta=-1   colDelta=-1 */
        /* shift=+9   rowDelta=+1   colDelta=+1 */

        /* Disc #1, flip direction 0. */
        /* Disc #1, flip direction 7. */
        movq    mm3, mm1                ; mm3 <- opp_bits
        movq    mm4, mm0                ; mm4 <- my_bits
        movq    mm6, mm0                ; mm6 <- backup of my_bits
        pand    mm3, dir_mask0          ; 0x007e7e7e7e7e7e00
                                        ; dir_mask0 of value:
                                        ;   00000000
                                        ;   01111110
                                        ;   01111110
                                        ;   01111110
                                        ;   01111110
                                        ;   01111110
                                        ;   01111110
                                        ;   00000000
        push    esi                     ;
        psllq   mm4, 9                  ;
        psrlq   mm6, 9                  ;
        push    edi                     ;
        pand    mm4, mm3                ;
        pand    mm6, mm3                ;
        push    ecx                     ;

        /* Disc #2, flip direction 0. */
        /* Disc #2, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        psllq   mm5, 9                  ;
        psrlq   mm7, 9                  ;
        push    ebx                     ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        and     edi, 0x7e7e7e7e         ; 0x7e7e7e7e
        and     esi, 0x7e7e7e7e         ; 0x7e7e7e7e
                                        ; value of:
                                        ; 011111110
                                        ; 011111110
                                        ; 011111110
                                        ; 011111110
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        shl     ebx, 1                  ;
        shl     ecx, 1                  ;

        /* Disc #3, flip direction 0. */
        /* Disc #3, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     ebx, edi                ;
        and     ecx, esi                ;
        psllq   mm5, 9                  ;
        psrlq   mm7, 9                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;

        /* Disc #4, flip direction 0. */
        /* Disc #4, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psllq   mm5, 9                  ;
        psrlq   mm7, 9                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;

        /* Disc #5, flip direction 0. */
        /* Disc #5, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psllq   mm5, 9                  ;
        psrlq   mm7, 9                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;

        /* Disc #6, flip direction 0. */
        /* Disc #6, flip direction 7. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psrlq   mm7, 9                  ;
        psllq   mm5, 9                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm4, 9                  ;
        psrlq   mm6, 9                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        por     mm2, mm4                ;
        por     mm2, mm6                ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;

        /* shift=-8   rowDelta=-1   colDelta=0 */
        /* shift=+8   rowDelta=1   colDelta=0 */

        /* Disc #1, flip direction 1. */
        /* Disc #1, flip direction 6. */
        movq    mm3, mm1                ;
        movq    mm4, mm0                ;
        movq    mm6, mm0                ;
        pand    mm3, dir_mask1          ; 0x00ffffffffffff00;
                                        ; dir_mask1 of value:
                                        ;   00000000
                                        ;   11111111
                                        ;   11111111
                                        ;   11111111
                                        ;   11111111
                                        ;   11111111
                                        ;   11111111
                                        ;   00000000
        psllq   mm4, 8                  ;
        psrlq   mm6, 8                  ;
        shl     edx, 1                  ;
        shl     eax, 1                  ;
        pand    mm4, mm3                ;
        pand    mm6, mm3                ;
        and     eax, edi                ;
        and     edx, esi                ;
        /* Disc #2, flip direction 1. */
        /* Disc #2, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        shl     ebx, 1                  ;
        shl     ecx, 1                  ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;

        /* serialize here: add horizontal shl flips. */

        movd    mm5, ebx                ;
        psllq   mm5, 32                 ;
        movd    mm7, ecx                ;
        por     mm5, mm7                ;
        por     mm2, mm5                ;

        /* Disc #3, flip direction 1. */
        /* Disc #3, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        pop     ebx                     ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        pop     ecx                     ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        push    ecx                     ;

        /* Disc #4, flip direction 1. */
        /* Disc #4, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        push    ebx                     ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        shr     ebx, 1                  ;
        shr     ecx, 1                  ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        and     ebx, edi                ;
        and     ecx, esi                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;

        /* Disc #5, flip direction 1. */
        /* Disc #5, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;

        /* Disc #6, flip direction 1. */
        /* Disc #6, flip direction 6. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        psllq   mm5, 8                  ;
        psrlq   mm7, 8                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm4, 8                  ;
        psrlq   mm6, 8                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        por     mm2, mm4                ;
        por     mm2, mm6                ;

        /* shift=-7   rowDelta=-1   colDelta=1 */
        /* shift=+7   rowDelta=1   colDelta=-1 */

        /* Disc #1, flip direction 2. */
        /* Disc #1, flip direction 5. */
        movq    mm3, mm1                ;
        movq    mm4, mm0                ;
        movq    mm6, mm0                ;
        pand    mm3, dir_mask2          ; 0x007e7e7e7e7e7e00;
                                        ; dir_mask2 of value:
                                        ;   00000000
                                        ;   01111110
                                        ;   01111110
                                        ;   01111110
                                        ;   01111110
                                        ;   01111110
                                        ;   01111110
                                        ;   00000000
        psllq   mm4, 7                  ;
        psrlq   mm6, 7                  ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        pand    mm4, mm3                ;
        pand    mm6, mm3                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;

        /* Disc #2, flip direction 2. */
        /* Disc #2, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;

        /* Disc #3, flip direction 2. */
        /* Disc #3, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;

        /* Disc #4, flip direction 2. */
        /* Disc #4, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        mov     eax, ebx                ;
        mov     edx, ecx                ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        shr     eax, 1                  ;
        shr     edx, 1                  ;

        /* Disc #5, flip direction 2. */
        /* Disc #5, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        and     eax, edi                ;
        and     edx, esi                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        or      ebx, eax                ;
        or      ecx, edx                ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        shr     ebx, 1                  ;
        shr     ecx, 1                  ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;

        /* serialize here: add horizontal shr flips. */

        movd    mm5, ebx                ;
        psllq   mm5, 32                 ;
        movd    mm7, ecx                ;
        por     mm5, mm7                ;
        por     mm2, mm5                ;
        pop     ebx                     ;

        /* Disc #6, flip direction 2. */
        /* Disc #6, flip direction 5. */
        movq    mm5, mm4                ;
        movq    mm7, mm6                ;
        psllq   mm5, 7                  ;
        psrlq   mm7, 7                  ;
        pop     ecx                     ;
        pand    mm5, mm3                ;
        pand    mm7, mm3                ;
        pop     edi                     ;
        por     mm4, mm5                ;
        por     mm6, mm7                ;
        pop     esi                     ;
        psllq   mm4, 7                  ;
        psrlq   mm6, 7                  ;
        por     mm2, mm4                ;
        por     mm2, mm6                ;

        /* mm2 is the pseudo-feasible moves at this point. */
        /* Let mm7 be the feasible moves, i.e., mm2 restricted to empty squares. */

        movq    mm7, mm0                ;
        por     mm7, mm1                ;
        pandn   mm7, mm2                ;

        /* Count the moves, i.e., the number of bits set in mm7. */

		/* Let %eax be the number of bits set in mm7.
		Let %ebx be the number of corner bits set in mm7. */

        movq    mm1, mm7                ;
		movd    ebx, mm7				;
		movq    mm2, mm1				;
		psrlq   mm2, 56					;
		psrld   mm7, 1					;
        pand    mm7, c55                ; c55 = 0x5555555555555555
		movd    ecx, mm2				;
        psubd   mm1, mm7                ;
        movq    mm7, mm1                ;
		and     ebx, 129				;
		and     ecx, 129				;
        psrld   mm1, 2                  ;
        pand    mm7, c33                ; c33 = 0x3333333333333333;
		add     ebx, ecx				;
        pand    mm1, c33                ; c33 = 0x3333333333333333;
		mov     ecx, ebx				;
        paddd   mm7, mm1                ;
        movq    mm1, mm7                ;
		shr     ecx, 7					;
        psrld   mm7, 4                  ;
        paddd   mm7, mm1                ;
		add     ebx, ecx				;
        pand    mm7, c0f                ; c0f = 0x0f0f0f0f0f0f0f0f;
		and     ebx, 7					;
        movq    mm1, mm7                ;
        psrld   mm7, 8                  ;
        paddd   mm7, mm1                ;
        movq    mm1, mm7                ;
        psrld   mm7, 16                 ;
        paddd   mm7, mm1                ;
        movq    mm1, mm7                ;
        psrlq   mm7, 32                 ;
        paddd   mm7, mm1                ;
        movd    eax, mm7                ;
        and     eax, 63                 ;

		/* The weighted mobility is 128 * (#moves + #corner moves). */
		add     eax, ebx				;
		shl     eax, 7					;
        mov     weighted_mobility, eax  ;

		emms							;
	}
	return weighted_mobility;
}
