/*
   File:           bitbvald.c

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard Valid One Empty.
*/

#include <stdlib.h>

#include "utils.h"
#include "bitboard.h"
#include "bitbvald.h"

/*
  This block generated on Sun Apr 30 17:57:03 2000
*/

static int
ValidOneEmpty_bitboard_a1(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00000002ul) &&
        (my_bits.low & 0x000000fcul)) ||
        (!(my_bits.low & 0x00000100ul) &&
        ((my_bits.low & 0x01010000ul) || (my_bits.high & 0x01010101ul))) ||
        (!(my_bits.low & 0x00000200ul) &&
        ((my_bits.low & 0x08040000ul) || (my_bits.high & 0x80402010ul)));
}

static int
ValidOneEmpty_bitboard_h1(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00000040ul) &&
        (my_bits.low & 0x0000003ful)) ||
        (!(my_bits.low & 0x00004000ul) &&
        ((my_bits.low & 0x10200000ul) || (my_bits.high & 0x01020408ul))) ||
        (!(my_bits.low & 0x00008000ul) &&
        ((my_bits.low & 0x80800000ul) || (my_bits.high & 0x80808080ul)));
}

static int
ValidOneEmpty_bitboard_a8(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x02000000ul) &&
        (my_bits.high & 0xfc000000ul)) ||
        (!(my_bits.high & 0x00020000ul) &&
        ((my_bits.low & 0x10204080ul) || (my_bits.high & 0x00000408ul))) ||
        (!(my_bits.high & 0x00010000ul) &&
        ((my_bits.low & 0x01010101ul) || (my_bits.high & 0x00000101ul)));
}

static int
ValidOneEmpty_bitboard_h8(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x40000000ul) &&
        (my_bits.high & 0x3f000000ul)) ||
        (!(my_bits.high & 0x00800000ul) &&
        ((my_bits.low & 0x80808080ul) || (my_bits.high & 0x00008080ul))) ||
        (!(my_bits.high & 0x00400000ul) &&
        ((my_bits.low & 0x08040201ul) || (my_bits.high & 0x00002010ul)));
}

static int
ValidOneEmpty_bitboard_b1(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00000004ul) &&
        (my_bits.low & 0x000000f8ul)) ||
        (!(my_bits.low & 0x00000200ul) &&
        ((my_bits.low & 0x02020000ul) || (my_bits.high & 0x02020202ul))) ||
        (!(my_bits.low & 0x00000400ul) &&
        ((my_bits.low & 0x10080000ul) || (my_bits.high & 0x00804020ul)));
}

static int
ValidOneEmpty_bitboard_g1(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00000020ul) &&
        (my_bits.low & 0x0000001ful)) ||
        (!(my_bits.low & 0x00002000ul) &&
        ((my_bits.low & 0x08100000ul) || (my_bits.high & 0x00010204ul))) ||
        (!(my_bits.low & 0x00004000ul) &&
        ((my_bits.low & 0x40400000ul) || (my_bits.high & 0x40404040ul)));
}

static int
ValidOneEmpty_bitboard_a2(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00000200ul) &&
        (my_bits.low & 0x0000fc00ul)) ||
            (!(my_bits.low & 0x00010000ul) &&
        ((my_bits.low & 0x01000000ul) || (my_bits.high & 0x01010101ul))) ||
                (!(my_bits.low & 0x00020000ul) &&
        ((my_bits.low & 0x04000000ul) || (my_bits.high & 0x40201008ul)));
}

static int
ValidOneEmpty_bitboard_h2(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00004000ul) &&
        (my_bits.low & 0x00003f00ul)) ||
        (!(my_bits.low & 0x00400000ul) &&
        ((my_bits.low & 0x20000000ul) || (my_bits.high & 0x02040810ul))) ||
        (!(my_bits.low & 0x00800000ul) &&
        ((my_bits.low & 0x80000000ul) || (my_bits.high & 0x80808080ul)));
}

static int
ValidOneEmpty_bitboard_a7(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00020000ul) &&
        (my_bits.high & 0x00fc0000ul)) ||
        (!(my_bits.high & 0x00000200ul) &&
        ((my_bits.low & 0x08102040ul) || (my_bits.high & 0x00000004ul))) ||
        (!(my_bits.high & 0x00000100ul) &&
        ((my_bits.low & 0x01010101ul) || (my_bits.high & 0x00000001ul)));
}

static int
ValidOneEmpty_bitboard_h7(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00400000ul) &&
        (my_bits.high & 0x003f0000ul)) ||
        (!(my_bits.high & 0x00008000ul) &&
        ((my_bits.low & 0x80808080ul) || (my_bits.high & 0x00000080ul))) ||
        (!(my_bits.high & 0x00004000ul) &&
        ((my_bits.low & 0x10080402ul) || (my_bits.high & 0x00000020ul)));
}

static int
ValidOneEmpty_bitboard_b8(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x04000000ul) &&
        (my_bits.high & 0xf8000000ul)) ||
        (!(my_bits.high & 0x00040000ul) &&
        ((my_bits.low & 0x20408000ul) || (my_bits.high & 0x00000810ul))) ||
        (!(my_bits.high & 0x00020000ul) &&
        ((my_bits.low & 0x02020202ul) || (my_bits.high & 0x00000202ul)));
}

static int
ValidOneEmpty_bitboard_g8(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x20000000ul) &&
        (my_bits.high & 0x1f000000ul)) ||
        (!(my_bits.high & 0x00400000ul) &&
        ((my_bits.low & 0x40404040ul) || (my_bits.high & 0x00004040ul))) ||
        (!(my_bits.high & 0x00200000ul) &&
        ((my_bits.low & 0x04020100ul) || (my_bits.high & 0x00001008ul)));
}

static int
ValidOneEmpty_bitboard_b2(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00000400ul) &&
        (my_bits.low & 0x0000f800ul)) ||
        (!(my_bits.low & 0x00020000ul) &&
        ((my_bits.low & 0x02000000ul) || (my_bits.high & 0x02020202ul))) ||
        (!(my_bits.low & 0x00040000ul) &&
        ((my_bits.low & 0x08000000ul) || (my_bits.high & 0x80402010ul)));
}

static int
ValidOneEmpty_bitboard_g2(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00002000ul) &&
        (my_bits.low & 0x00001f00ul)) ||
        (!(my_bits.low & 0x00200000ul) &&
        ((my_bits.low & 0x10000000ul) || (my_bits.high & 0x01020408ul))) ||
        (!(my_bits.low & 0x00400000ul) &&
        ((my_bits.low & 0x40000000ul) || (my_bits.high & 0x40404040ul)));
}

static int
ValidOneEmpty_bitboard_b7(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00040000ul) &&
        (my_bits.high & 0x00f80000ul)) ||
        (!(my_bits.high & 0x00000400ul) &&
        ((my_bits.low & 0x10204080ul) || (my_bits.high & 0x00000008ul))) ||
        (!(my_bits.high & 0x00000200ul) &&
        ((my_bits.low & 0x02020202ul) || (my_bits.high & 0x00000002ul)));
}

static int
ValidOneEmpty_bitboard_g7(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00200000ul) &&
        (my_bits.high & 0x001f0000ul)) ||
        (!(my_bits.high & 0x00004000ul) &&
        ((my_bits.low & 0x40404040ul) || (my_bits.high & 0x00000040ul))) ||
        (!(my_bits.high & 0x00002000ul) &&
        ((my_bits.low & 0x08040201ul) || (my_bits.high & 0x00000010ul)));
}

static int
ValidOneEmpty_bitboard_c1(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00000008ul) &&
        (my_bits.low & 0x000000f0ul)) ||
        (!(my_bits.low & 0x00000002ul) &&
        (my_bits.low & 0x00000001ul)) ||
        (!(my_bits.low & 0x00000200ul) &&
        (my_bits.low & 0x00010000ul)) ||
        (!(my_bits.low & 0x00000400ul) &&
        ((my_bits.low & 0x04040000ul) || (my_bits.high & 0x04040404ul))) ||
        (!(my_bits.low & 0x00000800ul) &&
        ((my_bits.low & 0x20100000ul) || (my_bits.high & 0x00008040ul)));
}

static int
ValidOneEmpty_bitboard_f1(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00000040ul) &&
        (my_bits.low & 0x00000080ul)) ||
        (!(my_bits.low & 0x00000010ul) &&
        (my_bits.low & 0x0000000ful)) ||
        (!(my_bits.low & 0x00001000ul) &&
        ((my_bits.low & 0x04080000ul) || (my_bits.high & 0x00000102ul))) ||
        (!(my_bits.low & 0x00002000ul) &&
        ((my_bits.low & 0x20200000ul) || (my_bits.high & 0x20202020ul))) ||
        (!(my_bits.low & 0x00004000ul) &&
        (my_bits.low & 0x00800000ul));
}

static int
ValidOneEmpty_bitboard_a3(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00020000ul) &&
        (my_bits.low & 0x00fc0000ul)) ||
        (!(my_bits.low & 0x00000200ul) &&
        (my_bits.low & 0x00000004ul)) ||
        (!(my_bits.low & 0x01000000ul) &&
        (my_bits.high & 0x01010101ul)) ||
        (!(my_bits.low & 0x00000100ul) &&
        (my_bits.low & 0x00000001ul)) ||
        (!(my_bits.low & 0x02000000ul) &&
        (my_bits.high & 0x20100804ul));
}

static int
ValidOneEmpty_bitboard_h3(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00400000ul) &&
        (my_bits.low & 0x003f0000ul)) ||
        (!(my_bits.low & 0x40000000ul) &&
        (my_bits.high & 0x04081020ul)) ||
        (!(my_bits.low & 0x80000000ul) &&
        (my_bits.high & 0x80808080ul)) ||
        (!(my_bits.low & 0x00008000ul) &&
        (my_bits.low & 0x00000080ul)) ||
        (!(my_bits.low & 0x00004000ul) &&
        (my_bits.low & 0x00000020ul));
}

static int
ValidOneEmpty_bitboard_a6(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00000200ul) &&
        (my_bits.high & 0x0000fc00ul)) ||
        (!(my_bits.high & 0x00000002ul) &&
        (my_bits.low & 0x04081020ul)) ||
        (!(my_bits.high & 0x00010000ul) &&
        (my_bits.high & 0x01000000ul)) ||
        (!(my_bits.high & 0x00000001ul) &&
        (my_bits.low & 0x01010101ul)) ||
        (!(my_bits.high & 0x00020000ul) &&
        (my_bits.high & 0x04000000ul));
}

static int
ValidOneEmpty_bitboard_h6(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00004000ul) &&
        (my_bits.high & 0x00003f00ul)) ||
        (!(my_bits.high & 0x00400000ul) &&
        (my_bits.high & 0x20000000ul)) ||
        (!(my_bits.high & 0x00800000ul) &&
        (my_bits.high & 0x80000000ul)) ||
        (!(my_bits.high & 0x00000080ul) &&
        (my_bits.low & 0x80808080ul)) ||
        (!(my_bits.high & 0x00000040ul) &&
        (my_bits.low & 0x20100804ul));
}

static int
ValidOneEmpty_bitboard_c8(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x08000000ul) &&
        (my_bits.high & 0xf0000000ul)) ||
        (!(my_bits.high & 0x02000000ul) &&
        (my_bits.high & 0x01000000ul)) ||
        (!(my_bits.high & 0x00080000ul) &&
        ((my_bits.low & 0x40800000ul) || (my_bits.high & 0x00001020ul))) ||
        (!(my_bits.high & 0x00040000ul) &&
        ((my_bits.low & 0x04040404ul) || (my_bits.high & 0x00000404ul))) ||
        (!(my_bits.high & 0x00020000ul) &&
        (my_bits.high & 0x00000100ul));
}

static int
ValidOneEmpty_bitboard_f8(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x40000000ul) &&
        (my_bits.high & 0x80000000ul)) ||
        (!(my_bits.high & 0x10000000ul) &&
        (my_bits.high & 0x0f000000ul)) ||
        (!(my_bits.high & 0x00400000ul) &&
        (my_bits.high & 0x00008000ul)) ||
        (!(my_bits.high & 0x00200000ul) &&
        ((my_bits.low & 0x20202020ul) || (my_bits.high & 0x00002020ul))) ||
        (!(my_bits.high & 0x00100000ul) &&
        ((my_bits.low & 0x02010000ul) || (my_bits.high & 0x00000804ul)));
}

static int
ValidOneEmpty_bitboard_d1(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00000010ul) &&
        (my_bits.low & 0x000000e0ul)) ||
        (!(my_bits.low & 0x00000004ul) &&
        (my_bits.low & 0x00000003ul)) ||
        (!(my_bits.low & 0x00000400ul) &&
        (my_bits.low & 0x01020000ul)) ||
        (!(my_bits.low & 0x00000800ul) &&
        ((my_bits.low & 0x08080000ul) || (my_bits.high & 0x08080808ul))) ||
        (!(my_bits.low & 0x00001000ul) &&
        ((my_bits.low & 0x40200000ul) || (my_bits.high & 0x00000080ul)));
}

static int
ValidOneEmpty_bitboard_e1(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00000020ul) &&
        (my_bits.low & 0x000000c0ul)) ||
        (!(my_bits.low & 0x00000008ul) &&
        (my_bits.low & 0x00000007ul)) ||
        (!(my_bits.low & 0x00000800ul) &&
        ((my_bits.low & 0x02040000ul) || (my_bits.high & 0x00000001ul))) ||
        (!(my_bits.low & 0x00001000ul) &&
        ((my_bits.low & 0x10100000ul) || (my_bits.high & 0x10101010ul))) ||
        (!(my_bits.low & 0x00002000ul) &&
        (my_bits.low & 0x80400000ul));
}

static int
ValidOneEmpty_bitboard_a4(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x02000000ul) &&
        (my_bits.low & 0xfc000000ul)) ||
        (!(my_bits.low & 0x00020000ul) &&
        (my_bits.low & 0x00000408ul)) ||
        (!(my_bits.high & 0x00000001ul) &&
        (my_bits.high & 0x01010100ul)) ||
        (!(my_bits.low & 0x00010000ul) &&
        (my_bits.low & 0x00000101ul)) ||
        (!(my_bits.high & 0x00000002ul) &&
        (my_bits.high & 0x10080400ul));
}

static int
ValidOneEmpty_bitboard_h4(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x40000000ul) &&
        (my_bits.low & 0x3f000000ul)) ||
        (!(my_bits.high & 0x00000040ul) &&
        (my_bits.high & 0x08102000ul)) ||
        (!(my_bits.high & 0x00000080ul) &&
        (my_bits.high & 0x80808000ul)) ||
        (!(my_bits.low & 0x00800000ul) &&
        (my_bits.low & 0x00008080ul)) ||
        (!(my_bits.low & 0x00400000ul) &&
        (my_bits.low & 0x00002010ul));
}

static int
ValidOneEmpty_bitboard_a5(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00000002ul) &&
        (my_bits.high & 0x000000fcul)) ||
        (!(my_bits.low & 0x02000000ul) &&
        (my_bits.low & 0x00040810ul)) ||
        (!(my_bits.high & 0x00000100ul) &&
        (my_bits.high & 0x01010000ul)) ||
        (!(my_bits.low & 0x01000000ul) &&
        (my_bits.low & 0x00010101ul)) ||
        (!(my_bits.high & 0x00000200ul) &&
        (my_bits.high & 0x08040000ul));
}

static int
ValidOneEmpty_bitboard_h5(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00000040ul) &&
        (my_bits.high & 0x0000003ful)) ||
        (!(my_bits.high & 0x00004000ul) &&
        (my_bits.high & 0x10200000ul)) ||
        (!(my_bits.high & 0x00008000ul) &&
        (my_bits.high & 0x80800000ul)) ||
        (!(my_bits.low & 0x80000000ul) &&
        (my_bits.low & 0x00808080ul)) ||
        (!(my_bits.low & 0x40000000ul) &&
        (my_bits.low & 0x00201008ul));
}

static int
ValidOneEmpty_bitboard_d8(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x10000000ul) &&
        (my_bits.high & 0xe0000000ul)) ||
        (!(my_bits.high & 0x04000000ul) &&
        (my_bits.high & 0x03000000ul)) ||
        (!(my_bits.high & 0x00100000ul) &&
        ((my_bits.low & 0x80000000ul) || (my_bits.high & 0x00002040ul))) ||
        (!(my_bits.high & 0x00080000ul) &&
        ((my_bits.low & 0x08080808ul) || (my_bits.high & 0x00000808ul))) ||
        (!(my_bits.high & 0x00040000ul) &&
        (my_bits.high & 0x00000201ul));
}

static int
ValidOneEmpty_bitboard_e8(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x20000000ul) &&
        (my_bits.high & 0xc0000000ul)) ||
        (!(my_bits.high & 0x08000000ul) &&
        (my_bits.high & 0x07000000ul)) ||
        (!(my_bits.high & 0x00200000ul) &&
        (my_bits.high & 0x00004080ul)) ||
        (!(my_bits.high & 0x00100000ul) &&
        ((my_bits.low & 0x10101010ul) || (my_bits.high & 0x00001010ul))) ||
        (!(my_bits.high & 0x00080000ul) &&
        ((my_bits.low & 0x01000000ul) || (my_bits.high & 0x00000402ul)));
}

static int
ValidOneEmpty_bitboard_c2(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00000800ul) &&
        (my_bits.low & 0x0000f000ul)) ||
        (!(my_bits.low & 0x00000200ul) &&
        (my_bits.low & 0x00000100ul)) ||
        (!(my_bits.low & 0x00020000ul) &&
        (my_bits.low & 0x01000000ul)) ||
        (!(my_bits.low & 0x00040000ul) &&
        ((my_bits.low & 0x04000000ul) || (my_bits.high & 0x04040404ul))) ||
        (!(my_bits.low & 0x00080000ul) &&
        ((my_bits.low & 0x10000000ul) || (my_bits.high & 0x00804020ul)));
}

static int
ValidOneEmpty_bitboard_f2(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00004000ul) &&
        (my_bits.low & 0x00008000ul)) ||
        (!(my_bits.low & 0x00001000ul) &&
        (my_bits.low & 0x00000f00ul)) ||
        (!(my_bits.low & 0x00100000ul) &&
        ((my_bits.low & 0x08000000ul) || (my_bits.high & 0x00010204ul))) ||
        (!(my_bits.low & 0x00200000ul) &&
        ((my_bits.low & 0x20000000ul) || (my_bits.high & 0x20202020ul))) ||
        (!(my_bits.low & 0x00400000ul) &&
        (my_bits.low & 0x80000000ul));
}

static int
ValidOneEmpty_bitboard_b3(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00040000ul) &&
        (my_bits.low & 0x00f80000ul)) ||
        (!(my_bits.low & 0x00000400ul) &&
        (my_bits.low & 0x00000008ul)) ||
        (!(my_bits.low & 0x02000000ul) &&
        (my_bits.high & 0x02020202ul)) ||
        (!(my_bits.low & 0x00000200ul) &&
        (my_bits.low & 0x00000002ul)) ||
        (!(my_bits.low & 0x04000000ul) &&
        (my_bits.high & 0x40201008ul));
}

static int
ValidOneEmpty_bitboard_g3(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00200000ul) &&
        (my_bits.low & 0x001f0000ul)) ||
        (!(my_bits.low & 0x20000000ul) &&
        (my_bits.high & 0x02040810ul)) ||
        (!(my_bits.low & 0x40000000ul) &&
        (my_bits.high & 0x40404040ul)) ||
        (!(my_bits.low & 0x00004000ul) &&
        (my_bits.low & 0x00000040ul)) ||
        (!(my_bits.low & 0x00002000ul) &&
        (my_bits.low & 0x00000010ul));
}

static int
ValidOneEmpty_bitboard_b6(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00000400ul) &&
        (my_bits.high & 0x0000f800ul)) ||
        (!(my_bits.high & 0x00000004ul) &&
        (my_bits.low & 0x08102040ul)) ||
        (!(my_bits.high & 0x00020000ul) &&
        (my_bits.high & 0x02000000ul)) ||
        (!(my_bits.high & 0x00000002ul) &&
        (my_bits.low & 0x02020202ul)) ||
        (!(my_bits.high & 0x00040000ul) &&
        (my_bits.high & 0x08000000ul));
}

static int
ValidOneEmpty_bitboard_g6(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00002000ul) &&
        (my_bits.high & 0x00001f00ul)) ||
        (!(my_bits.high & 0x00200000ul) &&
        (my_bits.high & 0x10000000ul)) ||
        (!(my_bits.high & 0x00400000ul) &&
        (my_bits.high & 0x40000000ul)) ||
        (!(my_bits.high & 0x00000040ul) &&
        (my_bits.low & 0x40404040ul)) ||
        (!(my_bits.high & 0x00000020ul) &&
        (my_bits.low & 0x10080402ul));
}

static int
ValidOneEmpty_bitboard_c7(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00080000ul) &&
        (my_bits.high & 0x00f00000ul)) ||
        (!(my_bits.high & 0x00020000ul) &&
        (my_bits.high & 0x00010000ul)) ||
        (!(my_bits.high & 0x00000800ul) &&
        ((my_bits.low & 0x20408000ul) || (my_bits.high & 0x00000010ul))) ||
        (!(my_bits.high & 0x00000400ul) &&
        ((my_bits.low & 0x04040404ul) || (my_bits.high & 0x00000004ul))) ||
        (!(my_bits.high & 0x00000200ul) &&
        (my_bits.high & 0x00000001ul));
}

static int
ValidOneEmpty_bitboard_f7(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00400000ul) &&
        (my_bits.high & 0x00800000ul)) ||
        (!(my_bits.high & 0x00100000ul) &&
        (my_bits.high & 0x000f0000ul)) ||
        (!(my_bits.high & 0x00004000ul) &&
        (my_bits.high & 0x00000080ul)) ||
        (!(my_bits.high & 0x00002000ul) &&
        ((my_bits.low & 0x20202020ul) || (my_bits.high & 0x00000020ul))) ||
        (!(my_bits.high & 0x00001000ul) &&
        ((my_bits.low & 0x04020100ul) || (my_bits.high & 0x00000008ul)));
}

static int
ValidOneEmpty_bitboard_d2(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00001000ul) &&
        (my_bits.low & 0x0000e000ul)) ||
        (!(my_bits.low & 0x00000400ul) &&
        (my_bits.low & 0x00000300ul)) ||
        (!(my_bits.low & 0x00040000ul) &&
        ((my_bits.low & 0x02000000ul) || (my_bits.high & 0x00000001ul))) ||
        (!(my_bits.low & 0x00080000ul) &&
        ((my_bits.low & 0x08000000ul) || (my_bits.high & 0x08080808ul))) ||
        (!(my_bits.low & 0x00100000ul) &&
        ((my_bits.low & 0x20000000ul) || (my_bits.high & 0x00008040ul)));
}

static int
ValidOneEmpty_bitboard_e2(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00002000ul) &&
        (my_bits.low & 0x0000c000ul)) ||
        (!(my_bits.low & 0x00000800ul) &&
        (my_bits.low & 0x00000700ul)) ||
        (!(my_bits.low & 0x00080000ul) &&
        ((my_bits.low & 0x04000000ul) || (my_bits.high & 0x00000102ul))) ||
        (!(my_bits.low & 0x00100000ul) &&
        ((my_bits.low & 0x10000000ul) || (my_bits.high & 0x10101010ul))) ||
        (!(my_bits.low & 0x00200000ul) &&
        ((my_bits.low & 0x40000000ul) || (my_bits.high & 0x00000080ul)));
}

static int
ValidOneEmpty_bitboard_b4(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x04000000ul) &&
        (my_bits.low & 0xf8000000ul)) ||
        (!(my_bits.low & 0x00040000ul) &&
        (my_bits.low & 0x00000810ul)) ||
        (!(my_bits.high & 0x00000002ul) &&
        (my_bits.high & 0x02020200ul)) ||
        (!(my_bits.low & 0x00020000ul) &&
        (my_bits.low & 0x00000202ul)) ||
        (!(my_bits.high & 0x00000004ul) &&
        (my_bits.high & 0x20100800ul));
}

static int
ValidOneEmpty_bitboard_g4(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x20000000ul) &&
        (my_bits.low & 0x1f000000ul)) ||
        (!(my_bits.high & 0x00000020ul) &&
        (my_bits.high & 0x04081000ul)) ||
        (!(my_bits.high & 0x00000040ul) &&
        (my_bits.high & 0x40404000ul)) ||
        (!(my_bits.low & 0x00400000ul) &&
        (my_bits.low & 0x00004040ul)) ||
        (!(my_bits.low & 0x00200000ul) &&
        (my_bits.low & 0x00001008ul));
}

static int
ValidOneEmpty_bitboard_b5(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00000004ul) &&
        (my_bits.high & 0x000000f8ul)) ||
        (!(my_bits.low & 0x04000000ul) &&
        (my_bits.low & 0x00081020ul)) ||
        (!(my_bits.high & 0x00000200ul) &&
        (my_bits.high & 0x02020000ul)) ||
        (!(my_bits.low & 0x02000000ul) &&
        (my_bits.low & 0x00020202ul)) ||
        (!(my_bits.high & 0x00000400ul) &&
        (my_bits.high & 0x10080000ul));
}

static int
ValidOneEmpty_bitboard_g5(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00000020ul) &&
        (my_bits.high & 0x0000001ful)) ||
        (!(my_bits.high & 0x00002000ul) &&
        (my_bits.high & 0x08100000ul)) ||
        (!(my_bits.high & 0x00004000ul) &&
        (my_bits.high & 0x40400000ul)) ||
        (!(my_bits.low & 0x40000000ul) &&
        (my_bits.low & 0x00404040ul)) ||
        (!(my_bits.low & 0x20000000ul) &&
        (my_bits.low & 0x00100804ul));
}

static int
ValidOneEmpty_bitboard_d7(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00100000ul) &&
        (my_bits.high & 0x00e00000ul)) ||
        (!(my_bits.high & 0x00040000ul) &&
        (my_bits.high & 0x00030000ul)) ||
        (!(my_bits.high & 0x00001000ul) &&
        ((my_bits.low & 0x40800000ul) || (my_bits.high & 0x00000020ul))) ||
        (!(my_bits.high & 0x00000800ul) &&
        ((my_bits.low & 0x08080808ul) || (my_bits.high & 0x00000008ul))) ||
        (!(my_bits.high & 0x00000400ul) &&
        ((my_bits.low & 0x01000000ul) || (my_bits.high & 0x00000002ul)));
}

static int
ValidOneEmpty_bitboard_e7(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00200000ul) &&
        (my_bits.high & 0x00c00000ul)) ||
        (!(my_bits.high & 0x00080000ul) &&
        (my_bits.high & 0x00070000ul)) ||
        (!(my_bits.high & 0x00002000ul) &&
        ((my_bits.low & 0x80000000ul) || (my_bits.high & 0x00000040ul))) ||
        (!(my_bits.high & 0x00001000ul) &&
        ((my_bits.low & 0x10101010ul) || (my_bits.high & 0x00000010ul))) ||
        (!(my_bits.high & 0x00000800ul) &&
        ((my_bits.low & 0x02010000ul) || (my_bits.high & 0x00000004ul)));
}

static int
ValidOneEmpty_bitboard_c3(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00080000ul) &&
        (my_bits.low & 0x00f00000ul)) ||
        (!(my_bits.low & 0x00020000ul) &&
        (my_bits.low & 0x00010000ul)) ||
        (!(my_bits.low & 0x02000000ul) &&
        (my_bits.high & 0x00000001ul)) ||
        (!(my_bits.low & 0x00000800ul) &&
        (my_bits.low & 0x00000010ul)) ||
        (!(my_bits.low & 0x04000000ul) &&
        (my_bits.high & 0x04040404ul)) ||
        (!(my_bits.low & 0x00000400ul) &&
        (my_bits.low & 0x00000004ul)) ||
        (!(my_bits.low & 0x08000000ul) &&
        (my_bits.high & 0x80402010ul)) ||
        (!(my_bits.low & 0x00000200ul) &&
        (my_bits.low & 0x00000001ul));
}

static int
ValidOneEmpty_bitboard_f3(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00400000ul) &&
        (my_bits.low & 0x00800000ul)) ||
        (!(my_bits.low & 0x00100000ul) &&
        (my_bits.low & 0x000f0000ul)) ||
        (!(my_bits.low & 0x10000000ul) &&
        (my_bits.high & 0x01020408ul)) ||
        (!(my_bits.low & 0x00004000ul) &&
        (my_bits.low & 0x00000080ul)) ||
        (!(my_bits.low & 0x20000000ul) &&
        (my_bits.high & 0x20202020ul)) ||
        (!(my_bits.low & 0x00002000ul) &&
        (my_bits.low & 0x00000020ul)) ||
        (!(my_bits.low & 0x40000000ul) &&
        (my_bits.high & 0x00000080ul)) ||
        (!(my_bits.low & 0x00001000ul) &&
        (my_bits.low & 0x00000008ul));
}

static int
ValidOneEmpty_bitboard_c6(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00000800ul) &&
        (my_bits.high & 0x0000f000ul)) ||
        (!(my_bits.high & 0x00000200ul) &&
        (my_bits.high & 0x00000100ul)) ||
        (!(my_bits.high & 0x00020000ul) &&
        (my_bits.high & 0x01000000ul)) ||
        (!(my_bits.high & 0x00000008ul) &&
        (my_bits.low & 0x10204080ul)) ||
        (!(my_bits.high & 0x00040000ul) &&
        (my_bits.high & 0x04000000ul)) ||
        (!(my_bits.high & 0x00000004ul) &&
        (my_bits.low & 0x04040404ul)) ||
        (!(my_bits.high & 0x00080000ul) &&
        (my_bits.high & 0x10000000ul)) ||
        (!(my_bits.high & 0x00000002ul) &&
        (my_bits.low & 0x01000000ul));
}

static int
ValidOneEmpty_bitboard_f6(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00004000ul) &&
        (my_bits.high & 0x00008000ul)) ||
        (!(my_bits.high & 0x00001000ul) &&
        (my_bits.high & 0x00000f00ul)) ||
        (!(my_bits.high & 0x00100000ul) &&
        (my_bits.high & 0x08000000ul)) ||
        (!(my_bits.high & 0x00000040ul) &&
        (my_bits.low & 0x80000000ul)) ||
        (!(my_bits.high & 0x00200000ul) &&
        (my_bits.high & 0x20000000ul)) ||
        (!(my_bits.high & 0x00000020ul) &&
        (my_bits.low & 0x20202020ul)) ||
        (!(my_bits.high & 0x00400000ul) &&
        (my_bits.high & 0x80000000ul)) ||
        (!(my_bits.high & 0x00000010ul) &&
        (my_bits.low & 0x08040201ul));
}

static int
ValidOneEmpty_bitboard_d3(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00100000ul) &&
        (my_bits.low & 0x00e00000ul)) ||
        (!(my_bits.low & 0x00040000ul) &&
        (my_bits.low & 0x00030000ul)) ||
        (!(my_bits.low & 0x04000000ul) &&
        (my_bits.high & 0x00000102ul)) ||
        (!(my_bits.low & 0x00001000ul) &&
        (my_bits.low & 0x00000020ul)) ||
        (!(my_bits.low & 0x08000000ul) &&
        (my_bits.high & 0x08080808ul)) ||
        (!(my_bits.low & 0x00000800ul) &&
        (my_bits.low & 0x00000008ul)) ||
        (!(my_bits.low & 0x10000000ul) &&
        (my_bits.high & 0x00804020ul)) ||
        (!(my_bits.low & 0x00000400ul) &&
        (my_bits.low & 0x00000002ul));
}

static int
ValidOneEmpty_bitboard_e3(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x00200000ul) &&
        (my_bits.low & 0x00c00000ul)) ||
        (!(my_bits.low & 0x00080000ul) &&
        (my_bits.low & 0x00070000ul)) ||
        (!(my_bits.low & 0x08000000ul) &&
        (my_bits.high & 0x00010204ul)) ||
        (!(my_bits.low & 0x00002000ul) &&
        (my_bits.low & 0x00000040ul)) ||
        (!(my_bits.low & 0x10000000ul) &&
        (my_bits.high & 0x10101010ul)) ||
        (!(my_bits.low & 0x00001000ul) &&
        (my_bits.low & 0x00000010ul)) ||
        (!(my_bits.low & 0x20000000ul) &&
        (my_bits.high & 0x00008040ul)) ||
        (!(my_bits.low & 0x00000800ul) &&
        (my_bits.low & 0x00000004ul));
}

static int
ValidOneEmpty_bitboard_c4(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x08000000ul) &&
        (my_bits.low & 0xf0000000ul)) ||
        (!(my_bits.low & 0x02000000ul) &&
        (my_bits.low & 0x01000000ul)) ||
        (!(my_bits.high & 0x00000002ul) &&
        (my_bits.high & 0x00000100ul)) ||
        (!(my_bits.low & 0x00080000ul) &&
        (my_bits.low & 0x00001020ul)) ||
        (!(my_bits.high & 0x00000004ul) &&
        (my_bits.high & 0x04040400ul)) ||
        (!(my_bits.low & 0x00040000ul) &&
        (my_bits.low & 0x00000404ul)) ||
        (!(my_bits.high & 0x00000008ul) &&
        (my_bits.high & 0x40201000ul)) ||
        (!(my_bits.low & 0x00020000ul) &&
        (my_bits.low & 0x00000100ul));
}

static int
ValidOneEmpty_bitboard_f4(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x40000000ul) &&
        (my_bits.low & 0x80000000ul)) ||
        (!(my_bits.low & 0x10000000ul) &&
        (my_bits.low & 0x0f000000ul)) ||
        (!(my_bits.high & 0x00000010ul) &&
        (my_bits.high & 0x02040800ul)) ||
        (!(my_bits.low & 0x00400000ul) &&
        (my_bits.low & 0x00008000ul)) ||
        (!(my_bits.high & 0x00000020ul) &&
        (my_bits.high & 0x20202000ul)) ||
        (!(my_bits.low & 0x00200000ul) &&
        (my_bits.low & 0x00002020ul)) ||
        (!(my_bits.high & 0x00000040ul) &&
        (my_bits.high & 0x00008000ul)) ||
        (!(my_bits.low & 0x00100000ul) &&
        (my_bits.low & 0x00000804ul));
}

static int
ValidOneEmpty_bitboard_c5(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00000008ul) &&
        (my_bits.high & 0x000000f0ul)) ||
        (!(my_bits.high & 0x00000002ul) &&
        (my_bits.high & 0x00000001ul)) ||
        (!(my_bits.high & 0x00000200ul) &&
        (my_bits.high & 0x00010000ul)) ||
        (!(my_bits.low & 0x08000000ul) &&
        (my_bits.low & 0x00102040ul)) ||
        (!(my_bits.high & 0x00000400ul) &&
        (my_bits.high & 0x04040000ul)) ||
        (!(my_bits.low & 0x04000000ul) &&
        (my_bits.low & 0x00040404ul)) ||
        (!(my_bits.high & 0x00000800ul) &&
        (my_bits.high & 0x20100000ul)) ||
        (!(my_bits.low & 0x02000000ul) &&
        (my_bits.low & 0x00010000ul));
}

static int
ValidOneEmpty_bitboard_f5(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00000040ul) &&
        (my_bits.high & 0x00000080ul)) ||
        (!(my_bits.high & 0x00000010ul) &&
        (my_bits.high & 0x0000000ful)) ||
        (!(my_bits.high & 0x00001000ul) &&
        (my_bits.high & 0x04080000ul)) ||
        (!(my_bits.low & 0x40000000ul) &&
        (my_bits.low & 0x00800000ul)) ||
        (!(my_bits.high & 0x00002000ul) &&
        (my_bits.high & 0x20200000ul)) ||
        (!(my_bits.low & 0x20000000ul) &&
        (my_bits.low & 0x00202020ul)) ||
        (!(my_bits.high & 0x00004000ul) &&
        (my_bits.high & 0x00800000ul)) ||
        (!(my_bits.low & 0x10000000ul) &&
        (my_bits.low & 0x00080402ul));
}

static int
ValidOneEmpty_bitboard_d6(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00001000ul) &&
        (my_bits.high & 0x0000e000ul)) ||
        (!(my_bits.high & 0x00000400ul) &&
        (my_bits.high & 0x00000300ul)) ||
        (!(my_bits.high & 0x00040000ul) &&
        (my_bits.high & 0x02000000ul)) ||
        (!(my_bits.high & 0x00000010ul) &&
        (my_bits.low & 0x20408000ul)) ||
        (!(my_bits.high & 0x00080000ul) &&
        (my_bits.high & 0x08000000ul)) ||
        (!(my_bits.high & 0x00000008ul) &&
        (my_bits.low & 0x08080808ul)) ||
        (!(my_bits.high & 0x00100000ul) &&
        (my_bits.high & 0x20000000ul)) ||
        (!(my_bits.high & 0x00000004ul) &&
        (my_bits.low & 0x02010000ul));
}

static int
ValidOneEmpty_bitboard_e6(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00002000ul) &&
        (my_bits.high & 0x0000c000ul)) ||
        (!(my_bits.high & 0x00000800ul) &&
        (my_bits.high & 0x00000700ul)) ||
        (!(my_bits.high & 0x00080000ul) &&
        (my_bits.high & 0x04000000ul)) ||
        (!(my_bits.high & 0x00000020ul) &&
        (my_bits.low & 0x40800000ul)) ||
        (!(my_bits.high & 0x00100000ul) &&
        (my_bits.high & 0x10000000ul)) ||
        (!(my_bits.high & 0x00000010ul) &&
        (my_bits.low & 0x10101010ul)) ||
        (!(my_bits.high & 0x00200000ul) &&
        (my_bits.high & 0x40000000ul)) ||
        (!(my_bits.high & 0x00000008ul) &&
        (my_bits.low & 0x04020100ul));
}

static int
ValidOneEmpty_bitboard_d4(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x10000000ul) &&
        (my_bits.low & 0xe0000000ul)) ||
        (!(my_bits.low & 0x04000000ul) &&
        (my_bits.low & 0x03000000ul)) ||
        (!(my_bits.high & 0x00000004ul) &&
        (my_bits.high & 0x00010200ul)) ||
        (!(my_bits.low & 0x00100000ul) &&
        (my_bits.low & 0x00002040ul)) ||
        (!(my_bits.high & 0x00000008ul) &&
        (my_bits.high & 0x08080800ul)) ||
        (!(my_bits.low & 0x00080000ul) &&
        (my_bits.low & 0x00000808ul)) ||
        (!(my_bits.high & 0x00000010ul) &&
        (my_bits.high & 0x80402000ul)) ||
        (!(my_bits.low & 0x00040000ul) &&
        (my_bits.low & 0x00000201ul));
}

static int
ValidOneEmpty_bitboard_e4(const BitBoard my_bits) {
    return
        (!(my_bits.low & 0x20000000ul) &&
        (my_bits.low & 0xc0000000ul)) ||
        (!(my_bits.low & 0x08000000ul) &&
        (my_bits.low & 0x07000000ul)) ||
        (!(my_bits.high & 0x00000008ul) &&
        (my_bits.high & 0x01020400ul)) ||
        (!(my_bits.low & 0x00200000ul) &&
        (my_bits.low & 0x00004080ul)) ||
        (!(my_bits.high & 0x00000010ul) &&
        (my_bits.high & 0x10101000ul)) ||
        (!(my_bits.low & 0x00100000ul) &&
        (my_bits.low & 0x00001010ul)) ||
        (!(my_bits.high & 0x00000020ul) &&
        (my_bits.high & 0x00804000ul)) ||
        (!(my_bits.low & 0x00080000ul) &&
        (my_bits.low & 0x00000402ul));
}

static int
ValidOneEmpty_bitboard_d5(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00000010ul) &&
        (my_bits.high & 0x000000e0ul)) ||
        (!(my_bits.high & 0x00000004ul) &&
        (my_bits.high & 0x00000003ul)) ||
        (!(my_bits.high & 0x00000400ul) &&
        (my_bits.high & 0x01020000ul)) ||
        (!(my_bits.low & 0x10000000ul) &&
        (my_bits.low & 0x00204080ul)) ||
        (!(my_bits.high & 0x00000800ul) &&
        (my_bits.high & 0x08080000ul)) ||
        (!(my_bits.low & 0x08000000ul) &&
        (my_bits.low & 0x00080808ul)) ||
        (!(my_bits.high & 0x00001000ul) &&
        (my_bits.high & 0x40200000ul)) ||
        (!(my_bits.low & 0x04000000ul) &&
        (my_bits.low & 0x00020100ul));
}

static int
ValidOneEmpty_bitboard_e5(const BitBoard my_bits) {
    return
        (!(my_bits.high & 0x00000020ul) &&
        (my_bits.high & 0x000000c0ul)) ||
        (!(my_bits.high & 0x00000008ul) &&
        (my_bits.high & 0x00000007ul)) ||
        (!(my_bits.high & 0x00000800ul) &&
        (my_bits.high & 0x02040000ul)) ||
        (!(my_bits.low & 0x20000000ul) &&
        (my_bits.low & 0x00408000ul)) ||
        (!(my_bits.high & 0x00001000ul) &&
        (my_bits.high & 0x10100000ul)) ||
        (!(my_bits.low & 0x10000000ul) &&
        (my_bits.low & 0x00101010ul)) ||
        (!(my_bits.high & 0x00002000ul) &&
        (my_bits.high & 0x80400000ul)) ||
        (!(my_bits.low & 0x08000000ul) &&
        (my_bits.low & 0x00040201ul));
}

int(*ValidOneEmpty_bitboard[64])(const BitBoard my_bits) = {

    ValidOneEmpty_bitboard_a1,
    ValidOneEmpty_bitboard_b1,
    ValidOneEmpty_bitboard_c1,
    ValidOneEmpty_bitboard_d1,
    ValidOneEmpty_bitboard_e1,
    ValidOneEmpty_bitboard_f1,
    ValidOneEmpty_bitboard_g1,
    ValidOneEmpty_bitboard_h1,

    ValidOneEmpty_bitboard_a2,
    ValidOneEmpty_bitboard_b2,
    ValidOneEmpty_bitboard_c2,
    ValidOneEmpty_bitboard_d2,
    ValidOneEmpty_bitboard_e2,
    ValidOneEmpty_bitboard_f2,
    ValidOneEmpty_bitboard_g2,
    ValidOneEmpty_bitboard_h2,

    ValidOneEmpty_bitboard_a3,
    ValidOneEmpty_bitboard_b3,
    ValidOneEmpty_bitboard_c3,
    ValidOneEmpty_bitboard_d3,
    ValidOneEmpty_bitboard_e3,
    ValidOneEmpty_bitboard_f3,
    ValidOneEmpty_bitboard_g3,
    ValidOneEmpty_bitboard_h3,

    ValidOneEmpty_bitboard_a4,
    ValidOneEmpty_bitboard_b4,
    ValidOneEmpty_bitboard_c4,
    ValidOneEmpty_bitboard_d4,
    ValidOneEmpty_bitboard_e4,
    ValidOneEmpty_bitboard_f4,
    ValidOneEmpty_bitboard_g4,
    ValidOneEmpty_bitboard_h4,

    ValidOneEmpty_bitboard_a5,
    ValidOneEmpty_bitboard_b5,
    ValidOneEmpty_bitboard_c5,
    ValidOneEmpty_bitboard_d5,
    ValidOneEmpty_bitboard_e5,
    ValidOneEmpty_bitboard_f5,
    ValidOneEmpty_bitboard_g5,
    ValidOneEmpty_bitboard_h5,

    ValidOneEmpty_bitboard_a6,
    ValidOneEmpty_bitboard_b6,
    ValidOneEmpty_bitboard_c6,
    ValidOneEmpty_bitboard_d6,
    ValidOneEmpty_bitboard_e6,
    ValidOneEmpty_bitboard_f6,
    ValidOneEmpty_bitboard_g6,
    ValidOneEmpty_bitboard_h6,

    ValidOneEmpty_bitboard_a7,
    ValidOneEmpty_bitboard_b7,
    ValidOneEmpty_bitboard_c7,
    ValidOneEmpty_bitboard_d7,
    ValidOneEmpty_bitboard_e7,
    ValidOneEmpty_bitboard_f7,
    ValidOneEmpty_bitboard_g7,
    ValidOneEmpty_bitboard_h7,

    ValidOneEmpty_bitboard_a8,
    ValidOneEmpty_bitboard_b8,
    ValidOneEmpty_bitboard_c8,
    ValidOneEmpty_bitboard_d8,
    ValidOneEmpty_bitboard_e8,
    ValidOneEmpty_bitboard_f8,
    ValidOneEmpty_bitboard_g8,
    ValidOneEmpty_bitboard_h8

};
