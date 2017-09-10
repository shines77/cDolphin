/*
   File:           bitbtest.cpp

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard get the flips of a move.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "bitboard.h"
#include "bitbtest.h"

ALIGN_PREFIX(64) BitBoard bb_flips ALIGN_SUFFIX(64);
ALIGN_PREFIX(16) BitBoard bb_flip_bits ALIGN_SUFFIX(16);

ALIGN_PREFIX(64) static unsigned int pop_count[64] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,    /*  0 -- 15 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,    /* 16 -- 31 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,    /* 32 -- 47 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6     /* 48 -- 63 */
} ALIGN_SUFFIX(64);

ALIGN_PREFIX(64) static unsigned int contiguous6[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    48, 48, 48, 48, 48, 48, 48, 48, 56, 56, 56, 56, 60, 60, 62, 63
} ALIGN_SUFFIX(64);

ALIGN_PREFIX(64) static unsigned int contiguous5[32] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    16, 16, 16, 16, 16, 16, 16, 16, 24, 24, 24, 24, 28, 28, 30, 31
} ALIGN_SUFFIX(64);

ALIGN_PREFIX(64) static unsigned int contiguous4[16] = {
    0,  0,  0,  0,  0,  0,  0,  0,  8,  8,  8,  8, 12, 12, 14, 15
} ALIGN_SUFFIX(64);

ALIGN_PREFIX(16) static unsigned int contiguous3[8] = {
    0,  0,  0,  0,  4,  4,  6,  7
} ALIGN_SUFFIX(64);

ALIGN_PREFIX(16) static unsigned int contiguous2[4] = {
    0,  0,  2,  3
} ALIGN_SUFFIX(64);

/*
ALIGN_PREFIX(64) static unsigned int contiguous1[2] = {
    0,  1
} ALIGN_SUFFIX(64);
//*/

static int
TestFlips_bitboard_a1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    /*
    __asm {
        emms
    }
    //*/

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000001ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00000002ul) & my_bits.low & 0x000000fcul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffffeul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 1];
    }
    /* Down */
    if (opp_bits.low & 0x00000100ul) {
        if (opp_bits.low & 0x00010000ul) {
            if (opp_bits.low & 0x01000000ul) {
                if (opp_bits.high & 0x00000001ul) {
                    if (opp_bits.high & 0x00000100ul) {
                        if (opp_bits.high & 0x00010000ul) {
                            if (my_bits.high & 0x01000000ul) {
                                flips_lo |= 0x01010100ul;
                                flips_hi |= 0x00010101ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.high & 0x00010000ul) {
                            flips_lo |= 0x01010100ul;
                            flips_hi |= 0x00000101ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00000100ul) {
                        flips_lo |= 0x01010100ul;
                        flips_hi |= 0x00000001ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000001ul) {
                    flips_lo |= 0x01010100ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x01000000ul) {
                flips_lo |= 0x00010100ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00010000ul) {
            flips_lo |= 0x00000100ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x00000200ul) {
        if (opp_bits.low & 0x00040000ul) {
            if (opp_bits.low & 0x08000000ul) {
                if (opp_bits.high & 0x00000010ul) {
                    if (opp_bits.high & 0x00002000ul) {
                        if (opp_bits.high & 0x00400000ul) {
                            if (my_bits.high & 0x80000000ul) {
                                flips_lo |= 0x08040200ul;
                                flips_hi |= 0x00402010ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.high & 0x00400000ul) {
                            flips_lo |= 0x08040200ul;
                            flips_hi |= 0x00002010ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00002000ul) {
                        flips_lo |= 0x08040200ul;
                        flips_hi |= 0x00000010ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000010ul) {
                    flips_lo |= 0x08040200ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x08000000ul) {
                flips_lo |= 0x00040200ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00040000ul) {
            flips_lo |= 0x00000200ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_h1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    //flips_lo = my_bits.low;
    //flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000080ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.low & 0x00000040ul) {
        unsigned int cont = contiguous6[(opp_bits.low >> 1) & 63];
        if (cont & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 1);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x00004000ul) {
        if (opp_bits.low & 0x00200000ul) {
            if (opp_bits.low & 0x10000000ul) {
                if (opp_bits.high & 0x00000008ul) {
                    if (opp_bits.high & 0x00000400ul) {
                        if (opp_bits.high & 0x00020000ul) {
                            if (my_bits.high & 0x01000000ul) {
                                flips_lo |= 0x10204000ul;
                                flips_hi |= 0x00020408ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.high & 0x00020000ul) {
                            flips_lo |= 0x10204000ul;
                            flips_hi |= 0x00000408ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00000400ul) {
                        flips_lo |= 0x10204000ul;
                        flips_hi |= 0x00000008ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000008ul) {
                    flips_lo |= 0x10204000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x10000000ul) {
                flips_lo |= 0x00204000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00200000ul) {
            flips_lo |= 0x00004000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x00008000ul) {
        if (opp_bits.low & 0x00800000ul) {
            if (opp_bits.low & 0x80000000ul) {
                if (opp_bits.high & 0x00000080ul) {
                    if (opp_bits.high & 0x00008000ul) {
                        if (opp_bits.high & 0x00800000ul) {
                            if (my_bits.high & 0x80000000ul) {
                                flips_lo |= 0x80808000ul;
                                flips_hi |= 0x00808080ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.high & 0x00800000ul) {
                            flips_lo |= 0x80808000ul;
                            flips_hi |= 0x00008080ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00008000ul) {
                        flips_lo |= 0x80808000ul;
                        flips_hi |= 0x00000080ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000080ul) {
                    flips_lo |= 0x80808000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x80000000ul) {
                flips_lo |= 0x00808000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00800000ul) {
            flips_lo |= 0x00008000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_a8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    //flips_hi = my_bits.high;
    //flips_lo = my_bits.low;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_hi |= 0x01000000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x02000000ul) & my_bits.high;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfe000000ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 25];
    }
    /* Up right */
    if (opp_bits.high & 0x00020000ul) {
        if (opp_bits.high & 0x00000400ul) {
            if (opp_bits.high & 0x00000008ul) {
                if (opp_bits.low & 0x10000000ul) {
                    if (opp_bits.low & 0x00200000ul) {
                        if (opp_bits.low & 0x00004000ul) {
                            if (my_bits.low & 0x00000080ul) {
                                flips_lo |= 0x10204000ul;
                                flips_hi |= 0x00020408ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.low & 0x00004000ul) {
                            flips_lo |= 0x10200000ul;
                            flips_hi |= 0x00020408ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00200000ul) {
                        flips_lo |= 0x10000000ul;
                        flips_hi |= 0x00020408ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x10000000ul) {
                    flips_hi |= 0x00020408ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000008ul) {
                flips_hi |= 0x00020400ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000400ul) {
            flips_hi |= 0x00020000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00010000ul) {
        if (opp_bits.high & 0x00000100ul) {
            if (opp_bits.high & 0x00000001ul) {
                if (opp_bits.low & 0x01000000ul) {
                    if (opp_bits.low & 0x00010000ul) {
                        if (opp_bits.low & 0x00000100ul) {
                            if (my_bits.low & 0x00000001ul) {
                                flips_lo |= 0x01010100ul;
                                flips_hi |= 0x00010101ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.low & 0x00000100ul) {
                            flips_lo |= 0x01010000ul;
                            flips_hi |= 0x00010101ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00010000ul) {
                        flips_lo |= 0x01000000ul;
                        flips_hi |= 0x00010101ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x01000000ul) {
                    flips_hi |= 0x00010101ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000001ul) {
                flips_hi |= 0x00010100ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000100ul) {
            flips_hi |= 0x00010000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_h8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    //flips_hi = my_bits.high;
    //flips_lo = my_bits.low;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_hi |= 0x80000000ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.high & 0x40000000ul) {
        unsigned int cont = contiguous6[(opp_bits.high >> 25) & 63];
        if ((cont << 24) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 25);
        }
    }
    /* Up */
    if (opp_bits.high & 0x00800000ul) {
        if (opp_bits.high & 0x00008000ul) {
            if (opp_bits.high & 0x00000080ul) {
                if (opp_bits.low & 0x80000000ul) {
                    if (opp_bits.low & 0x00800000ul) {
                        if (opp_bits.low & 0x00008000ul) {
                            if (my_bits.low & 0x00000080ul) {
                                flips_lo |= 0x80808000ul;
                                flips_hi |= 0x00808080ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.low & 0x00008000ul) {
                            flips_lo |= 0x80800000ul;
                            flips_hi |= 0x00808080ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00800000ul) {
                        flips_lo |= 0x80000000ul;
                        flips_hi |= 0x00808080ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x80000000ul) {
                    flips_hi |= 0x00808080ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000080ul) {
                flips_hi |= 0x00808000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00008000ul) {
            flips_hi |= 0x00800000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00400000ul) {
        if (opp_bits.high & 0x00002000ul) {
            if (opp_bits.high & 0x00000010ul) {
                if (opp_bits.low & 0x08000000ul) {
                    if (opp_bits.low & 0x00040000ul) {
                        if (opp_bits.low & 0x00000200ul) {
                            if (my_bits.low & 0x00000001ul) {
                                flips_lo |= 0x08040200ul;
                                flips_hi |= 0x00402010ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.low & 0x00000200ul) {
                            flips_lo |= 0x08040000ul;
                            flips_hi |= 0x00402010ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00040000ul) {
                        flips_lo |= 0x08000000ul;
                        flips_hi |= 0x00402010ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x08000000ul) {
                    flips_hi |= 0x00402010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000010ul) {
                flips_hi |= 0x00402000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00002000ul) {
            flips_hi |= 0x00400000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_b1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    //flips_lo = my_bits.low;
    //flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000002ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00000004ul) & my_bits.low & 0x000000f8ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffffcul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 2];
    }
    /* Down */
    if (opp_bits.low & 0x00000200ul) {
        if (opp_bits.low & 0x00020000ul) {
            if (opp_bits.low & 0x02000000ul) {
                if (opp_bits.high & 0x00000002ul) {
                    if (opp_bits.high & 0x00000200ul) {
                        if (opp_bits.high & 0x00020000ul) {
                            if (my_bits.high & 0x02000000ul) {
                                flips_lo |= 0x02020200ul;
                                flips_hi |= 0x00020202ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.high & 0x00020000ul) {
                            flips_lo |= 0x02020200ul;
                            flips_hi |= 0x00000202ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00000200ul) {
                        flips_lo |= 0x02020200ul;
                        flips_hi |= 0x00000002ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000002ul) {
                    flips_lo |= 0x02020200ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x02000000ul) {
                flips_lo |= 0x00020200ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00020000ul) {
            flips_lo |= 0x00000200ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x00000400ul) {
        if (opp_bits.low & 0x00080000ul) {
            if (opp_bits.low & 0x10000000ul) {
                if (opp_bits.high & 0x00000020ul) {
                    if (opp_bits.high & 0x00004000ul) {
                        if (my_bits.high & 0x00800000ul) {
                            flips_lo |= 0x10080400ul;
                            flips_hi |= 0x00004020ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00004000ul) {
                        flips_lo |= 0x10080400ul;
                        flips_hi |= 0x00000020ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000020ul) {
                    flips_lo |= 0x10080400ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x10000000ul) {
                flips_lo |= 0x00080400ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00080000ul) {
            flips_lo |= 0x00000400ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_g1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;

    //flips_lo = my_bits.low;
    //flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000040ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.low & 0x00000020ul) {
        unsigned int cont = contiguous5[(opp_bits.low >> 1) & 31];
        if (cont & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 1);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x00002000ul) {
        if (opp_bits.low & 0x00100000ul) {
            if (opp_bits.low & 0x08000000ul) {
                if (opp_bits.high & 0x00000004ul) {
                    if (opp_bits.high & 0x00000200ul) {
                        if (my_bits.high & 0x00010000ul) {
                            flips_lo |= 0x08102000ul;
                            flips_hi |= 0x00000204ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00000200ul) {
                        flips_lo |= 0x08102000ul;
                        flips_hi |= 0x00000004ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000004ul) {
                    flips_lo |= 0x08102000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x08000000ul) {
                flips_lo |= 0x00102000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00100000ul) {
            flips_lo |= 0x00002000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x00004000ul) {
        if (opp_bits.low & 0x00400000ul) {
            if (opp_bits.low & 0x40000000ul) {
                if (opp_bits.high & 0x00000040ul) {
                    if (opp_bits.high & 0x00004000ul) {
                        if (opp_bits.high & 0x00400000ul) {
                            if (my_bits.high & 0x40000000ul) {
                                flips_lo |= 0x40404000ul;
                                flips_hi |= 0x00404040ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.high & 0x00400000ul) {
                            flips_lo |= 0x40404000ul;
                            flips_hi |= 0x00004040ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00004000ul) {
                        flips_lo |= 0x40404000ul;
                        flips_hi |= 0x00000040ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000040ul) {
                    flips_lo |= 0x40404000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x40000000ul) {
                flips_lo |= 0x00404000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00400000ul) {
            flips_lo |= 0x00004000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_a2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000100ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00000200ul) & my_bits.low & 0x0000fc00ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffe00ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 9];
    }
    /* Down */
    if (opp_bits.low & 0x00010000ul) {
        if (opp_bits.low & 0x01000000ul) {
            if (opp_bits.high & 0x00000001ul) {
                if (opp_bits.high & 0x00000100ul) {
                    if (opp_bits.high & 0x00010000ul) {
                        if (my_bits.high & 0x01000000ul) {
                            flips_lo |= 0x01010000ul;
                            flips_hi |= 0x00010101ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00010000ul) {
                        flips_lo |= 0x01010000ul;
                        flips_hi |= 0x00000101ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000100ul) {
                    flips_lo |= 0x01010000ul;
                    flips_hi |= 0x00000001ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000001ul) {
                flips_lo |= 0x01010000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x01000000ul) {
            flips_lo |= 0x00010000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x00020000ul) {
        if (opp_bits.low & 0x04000000ul) {
            if (opp_bits.high & 0x00000008ul) {
                if (opp_bits.high & 0x00001000ul) {
                    if (opp_bits.high & 0x00200000ul) {
                        if (my_bits.high & 0x40000000ul) {
                            flips_lo |= 0x04020000ul;
                            flips_hi |= 0x00201008ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00200000ul) {
                        flips_lo |= 0x04020000ul;
                        flips_hi |= 0x00001008ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00001000ul) {
                    flips_lo |= 0x04020000ul;
                    flips_hi |= 0x00000008ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000008ul) {
                flips_lo |= 0x04020000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x04000000ul) {
            flips_lo |= 0x00020000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_h2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00008000ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.low & 0x00004000ul) {
        unsigned int cont = contiguous6[(opp_bits.low >> 9) & 63];
        if ((cont << 8) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 9);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x00400000ul) {
        if (opp_bits.low & 0x20000000ul) {
            if (opp_bits.high & 0x00000010ul) {
                if (opp_bits.high & 0x00000800ul) {
                    if (opp_bits.high & 0x00040000ul) {
                        if (my_bits.high & 0x02000000ul) {
                            flips_lo |= 0x20400000ul;
                            flips_hi |= 0x00040810ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00040000ul) {
                        flips_lo |= 0x20400000ul;
                        flips_hi |= 0x00000810ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000800ul) {
                    flips_lo |= 0x20400000ul;
                    flips_hi |= 0x00000010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000010ul) {
                flips_lo |= 0x20400000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x20000000ul) {
            flips_lo |= 0x00400000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x00800000ul) {
        if (opp_bits.low & 0x80000000ul) {
            if (opp_bits.high & 0x00000080ul) {
                if (opp_bits.high & 0x00008000ul) {
                    if (opp_bits.high & 0x00800000ul) {
                        if (my_bits.high & 0x80000000ul) {
                            flips_lo |= 0x80800000ul;
                            flips_hi |= 0x00808080ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00800000ul) {
                        flips_lo |= 0x80800000ul;
                        flips_hi |= 0x00008080ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00008000ul) {
                    flips_lo |= 0x80800000ul;
                    flips_hi |= 0x00000080ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000080ul) {
                flips_lo |= 0x80800000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x80000000ul) {
            flips_lo |= 0x00800000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_a7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00010000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00020000ul) & my_bits.high & 0x00fc0000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffe0000ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 17];
    }
    /* Up right */
    if (opp_bits.high & 0x00000200ul) {
        if (opp_bits.high & 0x00000004ul) {
            if (opp_bits.low & 0x08000000ul) {
                if (opp_bits.low & 0x00100000ul) {
                    if (opp_bits.low & 0x00002000ul) {
                        if (my_bits.low & 0x00000040ul) {
                            flips_lo |= 0x08102000ul;
                            flips_hi |= 0x00000204ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00002000ul) {
                        flips_lo |= 0x08100000ul;
                        flips_hi |= 0x00000204ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00100000ul) {
                    flips_lo |= 0x08000000ul;
                    flips_hi |= 0x00000204ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x08000000ul) {
                flips_hi |= 0x00000204ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000004ul) {
            flips_hi |= 0x00000200ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00000100ul) {
        if (opp_bits.high & 0x00000001ul) {
            if (opp_bits.low & 0x01000000ul) {
                if (opp_bits.low & 0x00010000ul) {
                    if (opp_bits.low & 0x00000100ul) {
                        if (my_bits.low & 0x00000001ul) {
                            flips_lo |= 0x01010100ul;
                            flips_hi |= 0x00000101ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00000100ul) {
                        flips_lo |= 0x01010000ul;
                        flips_hi |= 0x00000101ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00010000ul) {
                    flips_lo |= 0x01000000ul;
                    flips_hi |= 0x00000101ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x01000000ul) {
                flips_hi |= 0x00000101ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000001ul) {
            flips_hi |= 0x00000100ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_h7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00800000ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.high & 0x00400000ul) {
        unsigned int cont = contiguous6[(opp_bits.high >> 17) & 63];
        if ((cont << 16) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 17);
        }
    }
    /* Up */
    if (opp_bits.high & 0x00008000ul) {
        if (opp_bits.high & 0x00000080ul) {
            if (opp_bits.low & 0x80000000ul) {
                if (opp_bits.low & 0x00800000ul) {
                    if (opp_bits.low & 0x00008000ul) {
                        if (my_bits.low & 0x00000080ul) {
                            flips_lo |= 0x80808000ul;
                            flips_hi |= 0x00008080ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00008000ul) {
                        flips_lo |= 0x80800000ul;
                        flips_hi |= 0x00008080ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00800000ul) {
                    flips_lo |= 0x80000000ul;
                    flips_hi |= 0x00008080ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x80000000ul) {
                flips_hi |= 0x00008080ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000080ul) {
            flips_hi |= 0x00008000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00004000ul) {
        if (opp_bits.high & 0x00000020ul) {
            if (opp_bits.low & 0x10000000ul) {
                if (opp_bits.low & 0x00080000ul) {
                    if (opp_bits.low & 0x00000400ul) {
                        if (my_bits.low & 0x00000002ul) {
                            flips_lo |= 0x10080400ul;
                            flips_hi |= 0x00004020ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00000400ul) {
                        flips_lo |= 0x10080000ul;
                        flips_hi |= 0x00004020ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00080000ul) {
                    flips_lo |= 0x10000000ul;
                    flips_hi |= 0x00004020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x10000000ul) {
                flips_hi |= 0x00004020ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000020ul) {
            flips_hi |= 0x00004000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_b8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x02000000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x04000000ul) & my_bits.high;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfc000000ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 26];
    }
    /* Up right */
    if (opp_bits.high & 0x00040000ul) {
        if (opp_bits.high & 0x00000800ul) {
            if (opp_bits.high & 0x00000010ul) {
                if (opp_bits.low & 0x20000000ul) {
                    if (opp_bits.low & 0x00400000ul) {
                        if (my_bits.low & 0x00008000ul) {
                            flips_lo |= 0x20400000ul;
                            flips_hi |= 0x00040810ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00400000ul) {
                        flips_lo |= 0x20000000ul;
                        flips_hi |= 0x00040810ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x20000000ul) {
                    flips_hi |= 0x00040810ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000010ul) {
                flips_hi |= 0x00040800ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000800ul) {
            flips_hi |= 0x00040000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00020000ul) {
        if (opp_bits.high & 0x00000200ul) {
            if (opp_bits.high & 0x00000002ul) {
                if (opp_bits.low & 0x02000000ul) {
                    if (opp_bits.low & 0x00020000ul) {
                        if (opp_bits.low & 0x00000200ul) {
                            if (my_bits.low & 0x00000002ul) {
                                flips_lo |= 0x02020200ul;
                                flips_hi |= 0x00020202ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.low & 0x00000200ul) {
                            flips_lo |= 0x02020000ul;
                            flips_hi |= 0x00020202ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00020000ul) {
                        flips_lo |= 0x02000000ul;
                        flips_hi |= 0x00020202ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x02000000ul) {
                    flips_hi |= 0x00020202ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000002ul) {
                flips_hi |= 0x00020200ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000200ul) {
            flips_hi |= 0x00020000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_g8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x40000000ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.high & 0x20000000ul) {
        unsigned int cont = contiguous5[(opp_bits.high >> 25) & 31];
        if ((cont << 24) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 25);
        }
    }
    /* Up */
    if (opp_bits.high & 0x00400000ul) {
        if (opp_bits.high & 0x00004000ul) {
            if (opp_bits.high & 0x00000040ul) {
                if (opp_bits.low & 0x40000000ul) {
                    if (opp_bits.low & 0x00400000ul) {
                        if (opp_bits.low & 0x00004000ul) {
                            if (my_bits.low & 0x00000040ul) {
                                flips_lo |= 0x40404000ul;
                                flips_hi |= 0x00404040ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.low & 0x00004000ul) {
                            flips_lo |= 0x40400000ul;
                            flips_hi |= 0x00404040ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00400000ul) {
                        flips_lo |= 0x40000000ul;
                        flips_hi |= 0x00404040ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x40000000ul) {
                    flips_hi |= 0x00404040ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000040ul) {
                flips_hi |= 0x00404000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00004000ul) {
            flips_hi |= 0x00400000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00200000ul) {
        if (opp_bits.high & 0x00001000ul) {
            if (opp_bits.high & 0x00000008ul) {
                if (opp_bits.low & 0x04000000ul) {
                    if (opp_bits.low & 0x00020000ul) {
                        if (my_bits.low & 0x00000100ul) {
                            flips_lo |= 0x04020000ul;
                            flips_hi |= 0x00201008ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00020000ul) {
                        flips_lo |= 0x04000000ul;
                        flips_hi |= 0x00201008ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x04000000ul) {
                    flips_hi |= 0x00201008ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000008ul) {
                flips_hi |= 0x00201000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00001000ul) {
            flips_hi |= 0x00200000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_b2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000200ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00000400ul) & my_bits.low & 0x0000f800ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffc00ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 10];
    }
    /* Down */
    if (opp_bits.low & 0x00020000ul) {
        if (opp_bits.low & 0x02000000ul) {
            if (opp_bits.high & 0x00000002ul) {
                if (opp_bits.high & 0x00000200ul) {
                    if (opp_bits.high & 0x00020000ul) {
                        if (my_bits.high & 0x02000000ul) {
                            flips_lo |= 0x02020000ul;
                            flips_hi |= 0x00020202ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00020000ul) {
                        flips_lo |= 0x02020000ul;
                        flips_hi |= 0x00000202ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000200ul) {
                    flips_lo |= 0x02020000ul;
                    flips_hi |= 0x00000002ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000002ul) {
                flips_lo |= 0x02020000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x02000000ul) {
            flips_lo |= 0x00020000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x00040000ul) {
        if (opp_bits.low & 0x08000000ul) {
            if (opp_bits.high & 0x00000010ul) {
                if (opp_bits.high & 0x00002000ul) {
                    if (opp_bits.high & 0x00400000ul) {
                        if (my_bits.high & 0x80000000ul) {
                            flips_lo |= 0x08040000ul;
                            flips_hi |= 0x00402010ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00400000ul) {
                        flips_lo |= 0x08040000ul;
                        flips_hi |= 0x00002010ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00002000ul) {
                    flips_lo |= 0x08040000ul;
                    flips_hi |= 0x00000010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000010ul) {
                flips_lo |= 0x08040000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x08000000ul) {
            flips_lo |= 0x00040000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_g2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00004000ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.low & 0x00002000ul) {
        unsigned int cont = contiguous5[(opp_bits.low >> 9) & 31];
        if ((cont << 8) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 9);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x00200000ul) {
        if (opp_bits.low & 0x10000000ul) {
            if (opp_bits.high & 0x00000008ul) {
                if (opp_bits.high & 0x00000400ul) {
                    if (opp_bits.high & 0x00020000ul) {
                        if (my_bits.high & 0x01000000ul) {
                            flips_lo |= 0x10200000ul;
                            flips_hi |= 0x00020408ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00020000ul) {
                        flips_lo |= 0x10200000ul;
                        flips_hi |= 0x00000408ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000400ul) {
                    flips_lo |= 0x10200000ul;
                    flips_hi |= 0x00000008ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000008ul) {
                flips_lo |= 0x10200000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x10000000ul) {
            flips_lo |= 0x00200000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x00400000ul) {
        if (opp_bits.low & 0x40000000ul) {
            if (opp_bits.high & 0x00000040ul) {
                if (opp_bits.high & 0x00004000ul) {
                    if (opp_bits.high & 0x00400000ul) {
                        if (my_bits.high & 0x40000000ul) {
                            flips_lo |= 0x40400000ul;
                            flips_hi |= 0x00404040ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00400000ul) {
                        flips_lo |= 0x40400000ul;
                        flips_hi |= 0x00004040ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00004000ul) {
                    flips_lo |= 0x40400000ul;
                    flips_hi |= 0x00000040ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000040ul) {
                flips_lo |= 0x40400000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x40000000ul) {
            flips_lo |= 0x00400000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_b7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00020000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00040000ul) & my_bits.high & 0x00f80000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffc0000ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 18];
    }
    /* Up right */
    if (opp_bits.high & 0x00000400ul) {
        if (opp_bits.high & 0x00000008ul) {
            if (opp_bits.low & 0x10000000ul) {
                if (opp_bits.low & 0x00200000ul) {
                    if (opp_bits.low & 0x00004000ul) {
                        if (my_bits.low & 0x00000080ul) {
                            flips_lo |= 0x10204000ul;
                            flips_hi |= 0x00000408ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00004000ul) {
                        flips_lo |= 0x10200000ul;
                        flips_hi |= 0x00000408ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00200000ul) {
                    flips_lo |= 0x10000000ul;
                    flips_hi |= 0x00000408ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x10000000ul) {
                flips_hi |= 0x00000408ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000008ul) {
            flips_hi |= 0x00000400ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00000200ul) {
        if (opp_bits.high & 0x00000002ul) {
            if (opp_bits.low & 0x02000000ul) {
                if (opp_bits.low & 0x00020000ul) {
                    if (opp_bits.low & 0x00000200ul) {
                        if (my_bits.low & 0x00000002ul) {
                            flips_lo |= 0x02020200ul;
                            flips_hi |= 0x00000202ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00000200ul) {
                        flips_lo |= 0x02020000ul;
                        flips_hi |= 0x00000202ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00020000ul) {
                    flips_lo |= 0x02000000ul;
                    flips_hi |= 0x00000202ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x02000000ul) {
                flips_hi |= 0x00000202ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000002ul) {
            flips_hi |= 0x00000200ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_g7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00400000ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.high & 0x00200000ul) {
        unsigned int cont = contiguous5[(opp_bits.high >> 17) & 31];
        if ((cont << 16) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 17);
        }
    }
    /* Up */
    if (opp_bits.high & 0x00004000ul) {
        if (opp_bits.high & 0x00000040ul) {
            if (opp_bits.low & 0x40000000ul) {
                if (opp_bits.low & 0x00400000ul) {
                    if (opp_bits.low & 0x00004000ul) {
                        if (my_bits.low & 0x00000040ul) {
                            flips_lo |= 0x40404000ul;
                            flips_hi |= 0x00004040ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00004000ul) {
                        flips_lo |= 0x40400000ul;
                        flips_hi |= 0x00004040ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00400000ul) {
                    flips_lo |= 0x40000000ul;
                    flips_hi |= 0x00004040ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x40000000ul) {
                flips_hi |= 0x00004040ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000040ul) {
            flips_hi |= 0x00004000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00002000ul) {
        if (opp_bits.high & 0x00000010ul) {
            if (opp_bits.low & 0x08000000ul) {
                if (opp_bits.low & 0x00040000ul) {
                    if (opp_bits.low & 0x00000200ul) {
                        if (my_bits.low & 0x00000001ul) {
                            flips_lo |= 0x08040200ul;
                            flips_hi |= 0x00002010ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00000200ul) {
                        flips_lo |= 0x08040000ul;
                        flips_hi |= 0x00002010ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00040000ul) {
                    flips_lo |= 0x08000000ul;
                    flips_hi |= 0x00002010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x08000000ul) {
                flips_hi |= 0x00002010ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000010ul) {
            flips_hi |= 0x00002000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_c1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000004ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00000008ul) & my_bits.low & 0x000000f0ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffff8ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 3];
    }
    /* Left */
    if (opp_bits.low & 0x00000002ul) {
        if (my_bits.low & 0x00000001ul) {
            flips_lo |= 0x00000002ul;
            flip_count += 1;
        }
    }
    /* Down left */
    if (opp_bits.low & 0x00000200ul) {
        if (my_bits.low & 0x00010000ul) {
            flips_lo |= 0x00000200ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x00000400ul) {
        if (opp_bits.low & 0x00040000ul) {
            if (opp_bits.low & 0x04000000ul) {
                if (opp_bits.high & 0x00000004ul) {
                    if (opp_bits.high & 0x00000400ul) {
                        if (opp_bits.high & 0x00040000ul) {
                            if (my_bits.high & 0x04000000ul) {
                                flips_lo |= 0x04040400ul;
                                flips_hi |= 0x00040404ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.high & 0x00040000ul) {
                            flips_lo |= 0x04040400ul;
                            flips_hi |= 0x00000404ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00000400ul) {
                        flips_lo |= 0x04040400ul;
                        flips_hi |= 0x00000004ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000004ul) {
                    flips_lo |= 0x04040400ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x04000000ul) {
                flips_lo |= 0x00040400ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00040000ul) {
            flips_lo |= 0x00000400ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x00000800ul) {
        if (opp_bits.low & 0x00100000ul) {
            if (opp_bits.low & 0x20000000ul) {
                if (opp_bits.high & 0x00000040ul) {
                    if (my_bits.high & 0x00008000ul) {
                        flips_lo |= 0x20100800ul;
                        flips_hi |= 0x00000040ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000040ul) {
                    flips_lo |= 0x20100800ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x20000000ul) {
                flips_lo |= 0x00100800ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00100000ul) {
            flips_lo |= 0x00000800ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_f1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000020ul;
    flip_count = 0;

    /* Right */
    if (opp_bits.low & 0x00000040ul) {
        if (my_bits.low & 0x00000080ul) {
            flips_lo |= 0x00000040ul;
            flip_count += 1;
        }
    }
    /* Left */
    if (opp_bits.low & 0x00000010ul) {
        unsigned int cont = contiguous4[(opp_bits.low >> 1) & 15];
        if (cont & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 1);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x00001000ul) {
        if (opp_bits.low & 0x00080000ul) {
            if (opp_bits.low & 0x04000000ul) {
                if (opp_bits.high & 0x00000002ul) {
                    if (my_bits.high & 0x00000100ul) {
                        flips_lo |= 0x04081000ul;
                        flips_hi |= 0x00000002ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000002ul) {
                    flips_lo |= 0x04081000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x04000000ul) {
                flips_lo |= 0x00081000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00080000ul) {
            flips_lo |= 0x00001000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x00002000ul) {
        if (opp_bits.low & 0x00200000ul) {
            if (opp_bits.low & 0x20000000ul) {
                if (opp_bits.high & 0x00000020ul) {
                    if (opp_bits.high & 0x00002000ul) {
                        if (opp_bits.high & 0x00200000ul) {
                            if (my_bits.high & 0x20000000ul) {
                                flips_lo |= 0x20202000ul;
                                flips_hi |= 0x00202020ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.high & 0x00200000ul) {
                            flips_lo |= 0x20202000ul;
                            flips_hi |= 0x00002020ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00002000ul) {
                        flips_lo |= 0x20202000ul;
                        flips_hi |= 0x00000020ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000020ul) {
                    flips_lo |= 0x20202000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x20000000ul) {
                flips_lo |= 0x00202000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00200000ul) {
            flips_lo |= 0x00002000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x00004000ul) {
        if (my_bits.low & 0x00800000ul) {
            flips_lo |= 0x00004000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_a3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00010000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00020000ul) & my_bits.low & 0x00fc0000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffe0000ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 17];
    }
    /* Up right */
    if (opp_bits.low & 0x00000200ul) {
        if (my_bits.low & 0x00000004ul) {
            flips_lo |= 0x00000200ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x01000000ul) {
        if (opp_bits.high & 0x00000001ul) {
            if (opp_bits.high & 0x00000100ul) {
                if (opp_bits.high & 0x00010000ul) {
                    if (my_bits.high & 0x01000000ul) {
                        flips_lo |= 0x01000000ul;
                        flips_hi |= 0x00010101ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00010000ul) {
                    flips_lo |= 0x01000000ul;
                    flips_hi |= 0x00000101ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000100ul) {
                flips_lo |= 0x01000000ul;
                flips_hi |= 0x00000001ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000001ul) {
            flips_lo |= 0x01000000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00000100ul) {
        if (my_bits.low & 0x00000001ul) {
            flips_lo |= 0x00000100ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x02000000ul) {
        if (opp_bits.high & 0x00000004ul) {
            if (opp_bits.high & 0x00000800ul) {
                if (opp_bits.high & 0x00100000ul) {
                    if (my_bits.high & 0x20000000ul) {
                        flips_lo |= 0x02000000ul;
                        flips_hi |= 0x00100804ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00100000ul) {
                    flips_lo |= 0x02000000ul;
                    flips_hi |= 0x00000804ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000800ul) {
                flips_lo |= 0x02000000ul;
                flips_hi |= 0x00000004ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000004ul) {
            flips_lo |= 0x02000000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_h3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00800000ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.low & 0x00400000ul) {
        unsigned int cont = contiguous6[(opp_bits.low >> 17) & 63];
        if ((cont << 16) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 17);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x40000000ul) {
        if (opp_bits.high & 0x00000020ul) {
            if (opp_bits.high & 0x00001000ul) {
                if (opp_bits.high & 0x00080000ul) {
                    if (my_bits.high & 0x04000000ul) {
                        flips_lo |= 0x40000000ul;
                        flips_hi |= 0x00081020ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00080000ul) {
                    flips_lo |= 0x40000000ul;
                    flips_hi |= 0x00001020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00001000ul) {
                flips_lo |= 0x40000000ul;
                flips_hi |= 0x00000020ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000020ul) {
            flips_lo |= 0x40000000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x80000000ul) {
        if (opp_bits.high & 0x00000080ul) {
            if (opp_bits.high & 0x00008000ul) {
                if (opp_bits.high & 0x00800000ul) {
                    if (my_bits.high & 0x80000000ul) {
                        flips_lo |= 0x80000000ul;
                        flips_hi |= 0x00808080ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00800000ul) {
                    flips_lo |= 0x80000000ul;
                    flips_hi |= 0x00008080ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00008000ul) {
                flips_lo |= 0x80000000ul;
                flips_hi |= 0x00000080ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000080ul) {
            flips_lo |= 0x80000000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00008000ul) {
        if (my_bits.low & 0x00000080ul) {
            flips_lo |= 0x00008000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x00004000ul) {
        if (my_bits.low & 0x00000020ul) {
            flips_lo |= 0x00004000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_a6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000100ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00000200ul) & my_bits.high & 0x0000fc00ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffe00ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 9];
    }
    /* Up right */
    if (opp_bits.high & 0x00000002ul) {
        if (opp_bits.low & 0x04000000ul) {
            if (opp_bits.low & 0x00080000ul) {
                if (opp_bits.low & 0x00001000ul) {
                    if (my_bits.low & 0x00000020ul) {
                        flips_lo |= 0x04081000ul;
                        flips_hi |= 0x00000002ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00001000ul) {
                    flips_lo |= 0x04080000ul;
                    flips_hi |= 0x00000002ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00080000ul) {
                flips_lo |= 0x04000000ul;
                flips_hi |= 0x00000002ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x04000000ul) {
            flips_hi |= 0x00000002ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00010000ul) {
        if (my_bits.high & 0x01000000ul) {
            flips_hi |= 0x00010000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00000001ul) {
        if (opp_bits.low & 0x01000000ul) {
            if (opp_bits.low & 0x00010000ul) {
                if (opp_bits.low & 0x00000100ul) {
                    if (my_bits.low & 0x00000001ul) {
                        flips_lo |= 0x01010100ul;
                        flips_hi |= 0x00000001ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00000100ul) {
                    flips_lo |= 0x01010000ul;
                    flips_hi |= 0x00000001ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00010000ul) {
                flips_lo |= 0x01000000ul;
                flips_hi |= 0x00000001ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x01000000ul) {
            flips_hi |= 0x00000001ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00020000ul) {
        if (my_bits.high & 0x04000000ul) {
            flips_hi |= 0x00020000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_h6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00008000ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.high & 0x00004000ul) {
        unsigned int cont = contiguous6[(opp_bits.high >> 9) & 63];
        if ((cont << 8) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 9);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00400000ul) {
        if (my_bits.high & 0x20000000ul) {
            flips_hi |= 0x00400000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00800000ul) {
        if (my_bits.high & 0x80000000ul) {
            flips_hi |= 0x00800000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00000080ul) {
        if (opp_bits.low & 0x80000000ul) {
            if (opp_bits.low & 0x00800000ul) {
                if (opp_bits.low & 0x00008000ul) {
                    if (my_bits.low & 0x00000080ul) {
                        flips_lo |= 0x80808000ul;
                        flips_hi |= 0x00000080ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00008000ul) {
                    flips_lo |= 0x80800000ul;
                    flips_hi |= 0x00000080ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00800000ul) {
                flips_lo |= 0x80000000ul;
                flips_hi |= 0x00000080ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x80000000ul) {
            flips_hi |= 0x00000080ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00000040ul) {
        if (opp_bits.low & 0x20000000ul) {
            if (opp_bits.low & 0x00100000ul) {
                if (opp_bits.low & 0x00000800ul) {
                    if (my_bits.low & 0x00000004ul) {
                        flips_lo |= 0x20100800ul;
                        flips_hi |= 0x00000040ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00000800ul) {
                    flips_lo |= 0x20100000ul;
                    flips_hi |= 0x00000040ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00100000ul) {
                flips_lo |= 0x20000000ul;
                flips_hi |= 0x00000040ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x20000000ul) {
            flips_hi |= 0x00000040ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_c8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x04000000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x08000000ul) & my_bits.high;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xf8000000ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 27];
    }
    /* Left */
    if (opp_bits.high & 0x02000000ul) {
        if (my_bits.high & 0x01000000ul) {
            flips_hi |= 0x02000000ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.high & 0x00080000ul) {
        if (opp_bits.high & 0x00001000ul) {
            if (opp_bits.high & 0x00000020ul) {
                if (opp_bits.low & 0x40000000ul) {
                    if (my_bits.low & 0x00800000ul) {
                        flips_lo |= 0x40000000ul;
                        flips_hi |= 0x00081020ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x40000000ul) {
                    flips_hi |= 0x00081020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000020ul) {
                flips_hi |= 0x00081000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00001000ul) {
            flips_hi |= 0x00080000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00040000ul) {
        if (opp_bits.high & 0x00000400ul) {
            if (opp_bits.high & 0x00000004ul) {
                if (opp_bits.low & 0x04000000ul) {
                    if (opp_bits.low & 0x00040000ul) {
                        if (opp_bits.low & 0x00000400ul) {
                            if (my_bits.low & 0x00000004ul) {
                                flips_lo |= 0x04040400ul;
                                flips_hi |= 0x00040404ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.low & 0x00000400ul) {
                            flips_lo |= 0x04040000ul;
                            flips_hi |= 0x00040404ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00040000ul) {
                        flips_lo |= 0x04000000ul;
                        flips_hi |= 0x00040404ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x04000000ul) {
                    flips_hi |= 0x00040404ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000004ul) {
                flips_hi |= 0x00040400ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000400ul) {
            flips_hi |= 0x00040000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00020000ul) {
        if (my_bits.high & 0x00000100ul) {
            flips_hi |= 0x00020000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_f8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x20000000ul;
    flip_count = 0;

    /* Right */
    if (opp_bits.high & 0x40000000ul) {
        if (my_bits.high & 0x80000000ul) {
            flips_hi |= 0x40000000ul;
            flip_count += 1;
        }
    }
    /* Left */
    if (opp_bits.high & 0x10000000ul) {
        unsigned int cont = contiguous4[(opp_bits.high >> 25) & 15];
        if ((cont << 24) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 25);
        }
    }
    /* Up right */
    if (opp_bits.high & 0x00400000ul) {
        if (my_bits.high & 0x00008000ul) {
            flips_hi |= 0x00400000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00200000ul) {
        if (opp_bits.high & 0x00002000ul) {
            if (opp_bits.high & 0x00000020ul) {
                if (opp_bits.low & 0x20000000ul) {
                    if (opp_bits.low & 0x00200000ul) {
                        if (opp_bits.low & 0x00002000ul) {
                            if (my_bits.low & 0x00000020ul) {
                                flips_lo |= 0x20202000ul;
                                flips_hi |= 0x00202020ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.low & 0x00002000ul) {
                            flips_lo |= 0x20200000ul;
                            flips_hi |= 0x00202020ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00200000ul) {
                        flips_lo |= 0x20000000ul;
                        flips_hi |= 0x00202020ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x20000000ul) {
                    flips_hi |= 0x00202020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000020ul) {
                flips_hi |= 0x00202000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00002000ul) {
            flips_hi |= 0x00200000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00100000ul) {
        if (opp_bits.high & 0x00000800ul) {
            if (opp_bits.high & 0x00000004ul) {
                if (opp_bits.low & 0x02000000ul) {
                    if (my_bits.low & 0x00010000ul) {
                        flips_lo |= 0x02000000ul;
                        flips_hi |= 0x00100804ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x02000000ul) {
                    flips_hi |= 0x00100804ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000004ul) {
                flips_hi |= 0x00100800ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000800ul) {
            flips_hi |= 0x00100000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_d1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000008ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00000010ul) & my_bits.low & 0x000000e0ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffff0ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 4];
    }
    /* Left */
    if (opp_bits.low & 0x00000004ul) {
        unsigned int cont = contiguous2[(opp_bits.low >> 1) & 3];
        if (cont & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 1);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x00000400ul) {
        if (opp_bits.low & 0x00020000ul) {
            if (my_bits.low & 0x01000000ul) {
                flips_lo |= 0x00020400ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00020000ul) {
            flips_lo |= 0x00000400ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x00000800ul) {
        if (opp_bits.low & 0x00080000ul) {
            if (opp_bits.low & 0x08000000ul) {
                if (opp_bits.high & 0x00000008ul) {
                    if (opp_bits.high & 0x00000800ul) {
                        if (opp_bits.high & 0x00080000ul) {
                            if (my_bits.high & 0x08000000ul) {
                                flips_lo |= 0x08080800ul;
                                flips_hi |= 0x00080808ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.high & 0x00080000ul) {
                            flips_lo |= 0x08080800ul;
                            flips_hi |= 0x00000808ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00000800ul) {
                        flips_lo |= 0x08080800ul;
                        flips_hi |= 0x00000008ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000008ul) {
                    flips_lo |= 0x08080800ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x08000000ul) {
                flips_lo |= 0x00080800ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00080000ul) {
            flips_lo |= 0x00000800ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x00001000ul) {
        if (opp_bits.low & 0x00200000ul) {
            if (opp_bits.low & 0x40000000ul) {
                if (my_bits.high & 0x00000080ul) {
                    flips_lo |= 0x40201000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x40000000ul) {
                flips_lo |= 0x00201000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00200000ul) {
            flips_lo |= 0x00001000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_e1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000010ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00000020ul) & my_bits.low & 0x000000c0ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xffffffe0ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 5];
    }
    /* Left */
    if (opp_bits.low & 0x00000008ul) {
        unsigned int cont = contiguous3[(opp_bits.low >> 1) & 7];
        if (cont & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 1);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x00000800ul) {
        if (opp_bits.low & 0x00040000ul) {
            if (opp_bits.low & 0x02000000ul) {
                if (my_bits.high & 0x00000001ul) {
                    flips_lo |= 0x02040800ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x02000000ul) {
                flips_lo |= 0x00040800ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00040000ul) {
            flips_lo |= 0x00000800ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x00001000ul) {
        if (opp_bits.low & 0x00100000ul) {
            if (opp_bits.low & 0x10000000ul) {
                if (opp_bits.high & 0x00000010ul) {
                    if (opp_bits.high & 0x00001000ul) {
                        if (opp_bits.high & 0x00100000ul) {
                            if (my_bits.high & 0x10000000ul) {
                                flips_lo |= 0x10101000ul;
                                flips_hi |= 0x00101010ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.high & 0x00100000ul) {
                            flips_lo |= 0x10101000ul;
                            flips_hi |= 0x00001010ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00001000ul) {
                        flips_lo |= 0x10101000ul;
                        flips_hi |= 0x00000010ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000010ul) {
                    flips_lo |= 0x10101000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x10000000ul) {
                flips_lo |= 0x00101000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00100000ul) {
            flips_lo |= 0x00001000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x00002000ul) {
        if (opp_bits.low & 0x00400000ul) {
            if (my_bits.low & 0x80000000ul) {
                flips_lo |= 0x00402000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00400000ul) {
            flips_lo |= 0x00002000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_a4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x01000000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x02000000ul) & my_bits.low;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfe000000ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 25];
    }
    /* Up right */
    if (opp_bits.low & 0x00020000ul) {
        if (opp_bits.low & 0x00000400ul) {
            if (my_bits.low & 0x00000008ul) {
                flips_lo |= 0x00020400ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00000400ul) {
            flips_lo |= 0x00020000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00000001ul) {
        if (opp_bits.high & 0x00000100ul) {
            if (opp_bits.high & 0x00010000ul) {
                if (my_bits.high & 0x01000000ul) {
                    flips_hi |= 0x00010101ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00010000ul) {
                flips_hi |= 0x00000101ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000100ul) {
            flips_hi |= 0x00000001ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00010000ul) {
        if (opp_bits.low & 0x00000100ul) {
            if (my_bits.low & 0x00000001ul) {
                flips_lo |= 0x00010100ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00000100ul) {
            flips_lo |= 0x00010000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00000002ul) {
        if (opp_bits.high & 0x00000400ul) {
            if (opp_bits.high & 0x00080000ul) {
                if (my_bits.high & 0x10000000ul) {
                    flips_hi |= 0x00080402ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00080000ul) {
                flips_hi |= 0x00000402ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000400ul) {
            flips_hi |= 0x00000002ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_h4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x80000000ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.low & 0x40000000ul) {
        unsigned int cont = contiguous6[(opp_bits.low >> 25) & 63];
        if ((cont << 24) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 25);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00000040ul) {
        if (opp_bits.high & 0x00002000ul) {
            if (opp_bits.high & 0x00100000ul) {
                if (my_bits.high & 0x08000000ul) {
                    flips_hi |= 0x00102040ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00100000ul) {
                flips_hi |= 0x00002040ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00002000ul) {
            flips_hi |= 0x00000040ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00000080ul) {
        if (opp_bits.high & 0x00008000ul) {
            if (opp_bits.high & 0x00800000ul) {
                if (my_bits.high & 0x80000000ul) {
                    flips_hi |= 0x00808080ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00800000ul) {
                flips_hi |= 0x00008080ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00008000ul) {
            flips_hi |= 0x00000080ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00800000ul) {
        if (opp_bits.low & 0x00008000ul) {
            if (my_bits.low & 0x00000080ul) {
                flips_lo |= 0x00808000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00008000ul) {
            flips_lo |= 0x00800000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x00400000ul) {
        if (opp_bits.low & 0x00002000ul) {
            if (my_bits.low & 0x00000010ul) {
                flips_lo |= 0x00402000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00002000ul) {
            flips_lo |= 0x00400000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_a5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000001ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00000002ul) & my_bits.high & 0x000000fcul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffffeul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 1];
    }
    /* Up right */
    if (opp_bits.low & 0x02000000ul) {
        if (opp_bits.low & 0x00040000ul) {
            if (opp_bits.low & 0x00000800ul) {
                if (my_bits.low & 0x00000010ul) {
                    flips_lo |= 0x02040800ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00000800ul) {
                flips_lo |= 0x02040000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00040000ul) {
            flips_lo |= 0x02000000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00000100ul) {
        if (opp_bits.high & 0x00010000ul) {
            if (my_bits.high & 0x01000000ul) {
                flips_hi |= 0x00010100ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00010000ul) {
            flips_hi |= 0x00000100ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x01000000ul) {
        if (opp_bits.low & 0x00010000ul) {
            if (opp_bits.low & 0x00000100ul) {
                if (my_bits.low & 0x00000001ul) {
                    flips_lo |= 0x01010100ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00000100ul) {
                flips_lo |= 0x01010000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00010000ul) {
            flips_lo |= 0x01000000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00000200ul) {
        if (opp_bits.high & 0x00040000ul) {
            if (my_bits.high & 0x08000000ul) {
                flips_hi |= 0x00040200ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00040000ul) {
            flips_hi |= 0x00000200ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_h5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000080ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.high & 0x00000040ul) {
        unsigned int cont = contiguous6[(opp_bits.high >> 1) & 63];
        if (cont & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 1);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00004000ul) {
        if (opp_bits.high & 0x00200000ul) {
            if (my_bits.high & 0x10000000ul) {
                flips_hi |= 0x00204000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00200000ul) {
            flips_hi |= 0x00004000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00008000ul) {
        if (opp_bits.high & 0x00800000ul) {
            if (my_bits.high & 0x80000000ul) {
                flips_hi |= 0x00808000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00800000ul) {
            flips_hi |= 0x00008000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x80000000ul) {
        if (opp_bits.low & 0x00800000ul) {
            if (opp_bits.low & 0x00008000ul) {
                if (my_bits.low & 0x00000080ul) {
                    flips_lo |= 0x80808000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00008000ul) {
                flips_lo |= 0x80800000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00800000ul) {
            flips_lo |= 0x80000000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x40000000ul) {
        if (opp_bits.low & 0x00200000ul) {
            if (opp_bits.low & 0x00001000ul) {
                if (my_bits.low & 0x00000008ul) {
                    flips_lo |= 0x40201000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00001000ul) {
                flips_lo |= 0x40200000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00200000ul) {
            flips_lo |= 0x40000000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_d8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x08000000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x10000000ul) & my_bits.high;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xf0000000ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 28];
    }
    /* Left */
    if (opp_bits.high & 0x04000000ul) {
        unsigned int cont = contiguous2[(opp_bits.high >> 25) & 3];
        if ((cont << 24) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 25);
        }
    }
    /* Up right */
    if (opp_bits.high & 0x00100000ul) {
        if (opp_bits.high & 0x00002000ul) {
            if (opp_bits.high & 0x00000040ul) {
                if (my_bits.low & 0x80000000ul) {
                    flips_hi |= 0x00102040ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000040ul) {
                flips_hi |= 0x00102000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00002000ul) {
            flips_hi |= 0x00100000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00080000ul) {
        if (opp_bits.high & 0x00000800ul) {
            if (opp_bits.high & 0x00000008ul) {
                if (opp_bits.low & 0x08000000ul) {
                    if (opp_bits.low & 0x00080000ul) {
                        if (opp_bits.low & 0x00000800ul) {
                            if (my_bits.low & 0x00000008ul) {
                                flips_lo |= 0x08080800ul;
                                flips_hi |= 0x00080808ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.low & 0x00000800ul) {
                            flips_lo |= 0x08080000ul;
                            flips_hi |= 0x00080808ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00080000ul) {
                        flips_lo |= 0x08000000ul;
                        flips_hi |= 0x00080808ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x08000000ul) {
                    flips_hi |= 0x00080808ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000008ul) {
                flips_hi |= 0x00080800ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000800ul) {
            flips_hi |= 0x00080000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00040000ul) {
        if (opp_bits.high & 0x00000200ul) {
            if (my_bits.high & 0x00000001ul) {
                flips_hi |= 0x00040200ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000200ul) {
            flips_hi |= 0x00040000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_e8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x10000000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x20000000ul) & my_bits.high;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xe0000000ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 29];
    }
    /* Left */
    if (opp_bits.high & 0x08000000ul) {
        unsigned int cont = contiguous3[(opp_bits.high >> 25) & 7];
        if ((cont << 24) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 25);
        }
    }
    /* Up right */
    if (opp_bits.high & 0x00200000ul) {
        if (opp_bits.high & 0x00004000ul) {
            if (my_bits.high & 0x00000080ul) {
                flips_hi |= 0x00204000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00004000ul) {
            flips_hi |= 0x00200000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00100000ul) {
        if (opp_bits.high & 0x00001000ul) {
            if (opp_bits.high & 0x00000010ul) {
                if (opp_bits.low & 0x10000000ul) {
                    if (opp_bits.low & 0x00100000ul) {
                        if (opp_bits.low & 0x00001000ul) {
                            if (my_bits.low & 0x00000010ul) {
                                flips_lo |= 0x10101000ul;
                                flips_hi |= 0x00101010ul;
                                flip_count += 6;
                            }
                        }
                        else if (my_bits.low & 0x00001000ul) {
                            flips_lo |= 0x10100000ul;
                            flips_hi |= 0x00101010ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00100000ul) {
                        flips_lo |= 0x10000000ul;
                        flips_hi |= 0x00101010ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x10000000ul) {
                    flips_hi |= 0x00101010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000010ul) {
                flips_hi |= 0x00101000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00001000ul) {
            flips_hi |= 0x00100000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00080000ul) {
        if (opp_bits.high & 0x00000400ul) {
            if (opp_bits.high & 0x00000002ul) {
                if (my_bits.low & 0x01000000ul) {
                    flips_hi |= 0x00080402ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000002ul) {
                flips_hi |= 0x00080400ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000400ul) {
            flips_hi |= 0x00080000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_c2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000400ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00000800ul) & my_bits.low & 0x0000f000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffff800ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 11];
    }
    /* Left */
    if (opp_bits.low & 0x00000200ul) {
        if (my_bits.low & 0x00000100ul) {
            flips_lo |= 0x00000200ul;
            flip_count += 1;
        }
    }
    /* Down left */
    if (opp_bits.low & 0x00020000ul) {
        if (my_bits.low & 0x01000000ul) {
            flips_lo |= 0x00020000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x00040000ul) {
        if (opp_bits.low & 0x04000000ul) {
            if (opp_bits.high & 0x00000004ul) {
                if (opp_bits.high & 0x00000400ul) {
                    if (opp_bits.high & 0x00040000ul) {
                        if (my_bits.high & 0x04000000ul) {
                            flips_lo |= 0x04040000ul;
                            flips_hi |= 0x00040404ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00040000ul) {
                        flips_lo |= 0x04040000ul;
                        flips_hi |= 0x00000404ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000400ul) {
                    flips_lo |= 0x04040000ul;
                    flips_hi |= 0x00000004ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000004ul) {
                flips_lo |= 0x04040000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x04000000ul) {
            flips_lo |= 0x00040000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x00080000ul) {
        if (opp_bits.low & 0x10000000ul) {
            if (opp_bits.high & 0x00000020ul) {
                if (opp_bits.high & 0x00004000ul) {
                    if (my_bits.high & 0x00800000ul) {
                        flips_lo |= 0x10080000ul;
                        flips_hi |= 0x00004020ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00004000ul) {
                    flips_lo |= 0x10080000ul;
                    flips_hi |= 0x00000020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000020ul) {
                flips_lo |= 0x10080000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x10000000ul) {
            flips_lo |= 0x00080000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_f2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00002000ul;
    flip_count = 0;

    /* Right */
    if (opp_bits.low & 0x00004000ul) {
        if (my_bits.low & 0x00008000ul) {
            flips_lo |= 0x00004000ul;
            flip_count += 1;
        }
    }
    /* Left */
    if (opp_bits.low & 0x00001000ul) {
        unsigned int cont = contiguous4[(opp_bits.low >> 9) & 15];
        if ((cont << 8) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 9);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x00100000ul) {
        if (opp_bits.low & 0x08000000ul) {
            if (opp_bits.high & 0x00000004ul) {
                if (opp_bits.high & 0x00000200ul) {
                    if (my_bits.high & 0x00010000ul) {
                        flips_lo |= 0x08100000ul;
                        flips_hi |= 0x00000204ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000200ul) {
                    flips_lo |= 0x08100000ul;
                    flips_hi |= 0x00000004ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000004ul) {
                flips_lo |= 0x08100000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x08000000ul) {
            flips_lo |= 0x00100000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x00200000ul) {
        if (opp_bits.low & 0x20000000ul) {
            if (opp_bits.high & 0x00000020ul) {
                if (opp_bits.high & 0x00002000ul) {
                    if (opp_bits.high & 0x00200000ul) {
                        if (my_bits.high & 0x20000000ul) {
                            flips_lo |= 0x20200000ul;
                            flips_hi |= 0x00202020ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00200000ul) {
                        flips_lo |= 0x20200000ul;
                        flips_hi |= 0x00002020ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00002000ul) {
                    flips_lo |= 0x20200000ul;
                    flips_hi |= 0x00000020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000020ul) {
                flips_lo |= 0x20200000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x20000000ul) {
            flips_lo |= 0x00200000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x00400000ul) {
        if (my_bits.low & 0x80000000ul) {
            flips_lo |= 0x00400000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_b3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00020000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00040000ul) & my_bits.low & 0x00f80000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffc0000ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 18];
    }
    /* Up right */
    if (opp_bits.low & 0x00000400ul) {
        if (my_bits.low & 0x00000008ul) {
            flips_lo |= 0x00000400ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x02000000ul) {
        if (opp_bits.high & 0x00000002ul) {
            if (opp_bits.high & 0x00000200ul) {
                if (opp_bits.high & 0x00020000ul) {
                    if (my_bits.high & 0x02000000ul) {
                        flips_lo |= 0x02000000ul;
                        flips_hi |= 0x00020202ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00020000ul) {
                    flips_lo |= 0x02000000ul;
                    flips_hi |= 0x00000202ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000200ul) {
                flips_lo |= 0x02000000ul;
                flips_hi |= 0x00000002ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000002ul) {
            flips_lo |= 0x02000000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00000200ul) {
        if (my_bits.low & 0x00000002ul) {
            flips_lo |= 0x00000200ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x04000000ul) {
        if (opp_bits.high & 0x00000008ul) {
            if (opp_bits.high & 0x00001000ul) {
                if (opp_bits.high & 0x00200000ul) {
                    if (my_bits.high & 0x40000000ul) {
                        flips_lo |= 0x04000000ul;
                        flips_hi |= 0x00201008ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00200000ul) {
                    flips_lo |= 0x04000000ul;
                    flips_hi |= 0x00001008ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00001000ul) {
                flips_lo |= 0x04000000ul;
                flips_hi |= 0x00000008ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000008ul) {
            flips_lo |= 0x04000000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_g3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00400000ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.low & 0x00200000ul) {
        unsigned int cont = contiguous5[(opp_bits.low >> 17) & 31];
        if ((cont << 16) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 17);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x20000000ul) {
        if (opp_bits.high & 0x00000010ul) {
            if (opp_bits.high & 0x00000800ul) {
                if (opp_bits.high & 0x00040000ul) {
                    if (my_bits.high & 0x02000000ul) {
                        flips_lo |= 0x20000000ul;
                        flips_hi |= 0x00040810ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00040000ul) {
                    flips_lo |= 0x20000000ul;
                    flips_hi |= 0x00000810ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000800ul) {
                flips_lo |= 0x20000000ul;
                flips_hi |= 0x00000010ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000010ul) {
            flips_lo |= 0x20000000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x40000000ul) {
        if (opp_bits.high & 0x00000040ul) {
            if (opp_bits.high & 0x00004000ul) {
                if (opp_bits.high & 0x00400000ul) {
                    if (my_bits.high & 0x40000000ul) {
                        flips_lo |= 0x40000000ul;
                        flips_hi |= 0x00404040ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00400000ul) {
                    flips_lo |= 0x40000000ul;
                    flips_hi |= 0x00004040ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00004000ul) {
                flips_lo |= 0x40000000ul;
                flips_hi |= 0x00000040ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000040ul) {
            flips_lo |= 0x40000000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00004000ul) {
        if (my_bits.low & 0x00000040ul) {
            flips_lo |= 0x00004000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x00002000ul) {
        if (my_bits.low & 0x00000010ul) {
            flips_lo |= 0x00002000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_b6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000200ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00000400ul) & my_bits.high & 0x0000f800ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffc00ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 10];
    }
    /* Up right */
    if (opp_bits.high & 0x00000004ul) {
        if (opp_bits.low & 0x08000000ul) {
            if (opp_bits.low & 0x00100000ul) {
                if (opp_bits.low & 0x00002000ul) {
                    if (my_bits.low & 0x00000040ul) {
                        flips_lo |= 0x08102000ul;
                        flips_hi |= 0x00000004ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00002000ul) {
                    flips_lo |= 0x08100000ul;
                    flips_hi |= 0x00000004ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00100000ul) {
                flips_lo |= 0x08000000ul;
                flips_hi |= 0x00000004ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x08000000ul) {
            flips_hi |= 0x00000004ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00020000ul) {
        if (my_bits.high & 0x02000000ul) {
            flips_hi |= 0x00020000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00000002ul) {
        if (opp_bits.low & 0x02000000ul) {
            if (opp_bits.low & 0x00020000ul) {
                if (opp_bits.low & 0x00000200ul) {
                    if (my_bits.low & 0x00000002ul) {
                        flips_lo |= 0x02020200ul;
                        flips_hi |= 0x00000002ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00000200ul) {
                    flips_lo |= 0x02020000ul;
                    flips_hi |= 0x00000002ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00020000ul) {
                flips_lo |= 0x02000000ul;
                flips_hi |= 0x00000002ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x02000000ul) {
            flips_hi |= 0x00000002ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00040000ul) {
        if (my_bits.high & 0x08000000ul) {
            flips_hi |= 0x00040000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_g6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00004000ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.high & 0x00002000ul) {
        unsigned int cont = contiguous5[(opp_bits.high >> 9) & 31];
        if ((cont << 8) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 9);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00200000ul) {
        if (my_bits.high & 0x10000000ul) {
            flips_hi |= 0x00200000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00400000ul) {
        if (my_bits.high & 0x40000000ul) {
            flips_hi |= 0x00400000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00000040ul) {
        if (opp_bits.low & 0x40000000ul) {
            if (opp_bits.low & 0x00400000ul) {
                if (opp_bits.low & 0x00004000ul) {
                    if (my_bits.low & 0x00000040ul) {
                        flips_lo |= 0x40404000ul;
                        flips_hi |= 0x00000040ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00004000ul) {
                    flips_lo |= 0x40400000ul;
                    flips_hi |= 0x00000040ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00400000ul) {
                flips_lo |= 0x40000000ul;
                flips_hi |= 0x00000040ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x40000000ul) {
            flips_hi |= 0x00000040ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00000020ul) {
        if (opp_bits.low & 0x10000000ul) {
            if (opp_bits.low & 0x00080000ul) {
                if (opp_bits.low & 0x00000400ul) {
                    if (my_bits.low & 0x00000002ul) {
                        flips_lo |= 0x10080400ul;
                        flips_hi |= 0x00000020ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00000400ul) {
                    flips_lo |= 0x10080000ul;
                    flips_hi |= 0x00000020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00080000ul) {
                flips_lo |= 0x10000000ul;
                flips_hi |= 0x00000020ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x10000000ul) {
            flips_hi |= 0x00000020ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_c7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00040000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00080000ul) & my_bits.high & 0x00f00000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfff80000ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 19];
    }
    /* Left */
    if (opp_bits.high & 0x00020000ul) {
        if (my_bits.high & 0x00010000ul) {
            flips_hi |= 0x00020000ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.high & 0x00000800ul) {
        if (opp_bits.high & 0x00000010ul) {
            if (opp_bits.low & 0x20000000ul) {
                if (opp_bits.low & 0x00400000ul) {
                    if (my_bits.low & 0x00008000ul) {
                        flips_lo |= 0x20400000ul;
                        flips_hi |= 0x00000810ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00400000ul) {
                    flips_lo |= 0x20000000ul;
                    flips_hi |= 0x00000810ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x20000000ul) {
                flips_hi |= 0x00000810ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000010ul) {
            flips_hi |= 0x00000800ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00000400ul) {
        if (opp_bits.high & 0x00000004ul) {
            if (opp_bits.low & 0x04000000ul) {
                if (opp_bits.low & 0x00040000ul) {
                    if (opp_bits.low & 0x00000400ul) {
                        if (my_bits.low & 0x00000004ul) {
                            flips_lo |= 0x04040400ul;
                            flips_hi |= 0x00000404ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00000400ul) {
                        flips_lo |= 0x04040000ul;
                        flips_hi |= 0x00000404ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00040000ul) {
                    flips_lo |= 0x04000000ul;
                    flips_hi |= 0x00000404ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x04000000ul) {
                flips_hi |= 0x00000404ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000004ul) {
            flips_hi |= 0x00000400ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00000200ul) {
        if (my_bits.high & 0x00000001ul) {
            flips_hi |= 0x00000200ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_f7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00200000ul;
    flip_count = 0;

    /* Right */
    if (opp_bits.high & 0x00400000ul) {
        if (my_bits.high & 0x00800000ul) {
            flips_hi |= 0x00400000ul;
            flip_count += 1;
        }
    }
    /* Left */
    if (opp_bits.high & 0x00100000ul) {
        unsigned int cont = contiguous4[(opp_bits.high >> 17) & 15];
        if ((cont << 16) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 17);
        }
    }
    /* Up right */
    if (opp_bits.high & 0x00004000ul) {
        if (my_bits.high & 0x00000080ul) {
            flips_hi |= 0x00004000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00002000ul) {
        if (opp_bits.high & 0x00000020ul) {
            if (opp_bits.low & 0x20000000ul) {
                if (opp_bits.low & 0x00200000ul) {
                    if (opp_bits.low & 0x00002000ul) {
                        if (my_bits.low & 0x00000020ul) {
                            flips_lo |= 0x20202000ul;
                            flips_hi |= 0x00002020ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00002000ul) {
                        flips_lo |= 0x20200000ul;
                        flips_hi |= 0x00002020ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00200000ul) {
                    flips_lo |= 0x20000000ul;
                    flips_hi |= 0x00002020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x20000000ul) {
                flips_hi |= 0x00002020ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000020ul) {
            flips_hi |= 0x00002000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00001000ul) {
        if (opp_bits.high & 0x00000008ul) {
            if (opp_bits.low & 0x04000000ul) {
                if (opp_bits.low & 0x00020000ul) {
                    if (my_bits.low & 0x00000100ul) {
                        flips_lo |= 0x04020000ul;
                        flips_hi |= 0x00001008ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00020000ul) {
                    flips_lo |= 0x04000000ul;
                    flips_hi |= 0x00001008ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x04000000ul) {
                flips_hi |= 0x00001008ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000008ul) {
            flips_hi |= 0x00001000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_d2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000800ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00001000ul) & my_bits.low & 0x0000e000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffff000ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 12];
    }
    /* Left */
    if (opp_bits.low & 0x00000400ul) {
        unsigned int cont = contiguous2[(opp_bits.low >> 9) & 3];
        if ((cont << 8) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 9);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x00040000ul) {
        if (opp_bits.low & 0x02000000ul) {
            if (my_bits.high & 0x00000001ul) {
                flips_lo |= 0x02040000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x02000000ul) {
            flips_lo |= 0x00040000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x00080000ul) {
        if (opp_bits.low & 0x08000000ul) {
            if (opp_bits.high & 0x00000008ul) {
                if (opp_bits.high & 0x00000800ul) {
                    if (opp_bits.high & 0x00080000ul) {
                        if (my_bits.high & 0x08000000ul) {
                            flips_lo |= 0x08080000ul;
                            flips_hi |= 0x00080808ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00080000ul) {
                        flips_lo |= 0x08080000ul;
                        flips_hi |= 0x00000808ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00000800ul) {
                    flips_lo |= 0x08080000ul;
                    flips_hi |= 0x00000008ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000008ul) {
                flips_lo |= 0x08080000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x08000000ul) {
            flips_lo |= 0x00080000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x00100000ul) {
        if (opp_bits.low & 0x20000000ul) {
            if (opp_bits.high & 0x00000040ul) {
                if (my_bits.high & 0x00008000ul) {
                    flips_lo |= 0x20100000ul;
                    flips_hi |= 0x00000040ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000040ul) {
                flips_lo |= 0x20100000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x20000000ul) {
            flips_lo |= 0x00100000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_e2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00001000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00002000ul) & my_bits.low & 0x0000c000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xffffe000ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 13];
    }
    /* Left */
    if (opp_bits.low & 0x00000800ul) {
        unsigned int cont = contiguous3[(opp_bits.low >> 9) & 7];
        if ((cont << 8) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 9);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x00080000ul) {
        if (opp_bits.low & 0x04000000ul) {
            if (opp_bits.high & 0x00000002ul) {
                if (my_bits.high & 0x00000100ul) {
                    flips_lo |= 0x04080000ul;
                    flips_hi |= 0x00000002ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000002ul) {
                flips_lo |= 0x04080000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x04000000ul) {
            flips_lo |= 0x00080000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x00100000ul) {
        if (opp_bits.low & 0x10000000ul) {
            if (opp_bits.high & 0x00000010ul) {
                if (opp_bits.high & 0x00001000ul) {
                    if (opp_bits.high & 0x00100000ul) {
                        if (my_bits.high & 0x10000000ul) {
                            flips_lo |= 0x10100000ul;
                            flips_hi |= 0x00101010ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.high & 0x00100000ul) {
                        flips_lo |= 0x10100000ul;
                        flips_hi |= 0x00001010ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00001000ul) {
                    flips_lo |= 0x10100000ul;
                    flips_hi |= 0x00000010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000010ul) {
                flips_lo |= 0x10100000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x10000000ul) {
            flips_lo |= 0x00100000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x00200000ul) {
        if (opp_bits.low & 0x40000000ul) {
            if (my_bits.high & 0x00000080ul) {
                flips_lo |= 0x40200000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x40000000ul) {
            flips_lo |= 0x00200000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_b4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x02000000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x04000000ul) & my_bits.low;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfc000000ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 26];
    }
    /* Up right */
    if (opp_bits.low & 0x00040000ul) {
        if (opp_bits.low & 0x00000800ul) {
            if (my_bits.low & 0x00000010ul) {
                flips_lo |= 0x00040800ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00000800ul) {
            flips_lo |= 0x00040000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00000002ul) {
        if (opp_bits.high & 0x00000200ul) {
            if (opp_bits.high & 0x00020000ul) {
                if (my_bits.high & 0x02000000ul) {
                    flips_hi |= 0x00020202ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00020000ul) {
                flips_hi |= 0x00000202ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000200ul) {
            flips_hi |= 0x00000002ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00020000ul) {
        if (opp_bits.low & 0x00000200ul) {
            if (my_bits.low & 0x00000002ul) {
                flips_lo |= 0x00020200ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00000200ul) {
            flips_lo |= 0x00020000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00000004ul) {
        if (opp_bits.high & 0x00000800ul) {
            if (opp_bits.high & 0x00100000ul) {
                if (my_bits.high & 0x20000000ul) {
                    flips_hi |= 0x00100804ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00100000ul) {
                flips_hi |= 0x00000804ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000800ul) {
            flips_hi |= 0x00000004ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_g4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x40000000ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.low & 0x20000000ul) {
        unsigned int cont = contiguous5[(opp_bits.low >> 25) & 31];
        if ((cont << 24) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 25);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00000020ul) {
        if (opp_bits.high & 0x00001000ul) {
            if (opp_bits.high & 0x00080000ul) {
                if (my_bits.high & 0x04000000ul) {
                    flips_hi |= 0x00081020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00080000ul) {
                flips_hi |= 0x00001020ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00001000ul) {
            flips_hi |= 0x00000020ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00000040ul) {
        if (opp_bits.high & 0x00004000ul) {
            if (opp_bits.high & 0x00400000ul) {
                if (my_bits.high & 0x40000000ul) {
                    flips_hi |= 0x00404040ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00400000ul) {
                flips_hi |= 0x00004040ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00004000ul) {
            flips_hi |= 0x00000040ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00400000ul) {
        if (opp_bits.low & 0x00004000ul) {
            if (my_bits.low & 0x00000040ul) {
                flips_lo |= 0x00404000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00004000ul) {
            flips_lo |= 0x00400000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x00200000ul) {
        if (opp_bits.low & 0x00001000ul) {
            if (my_bits.low & 0x00000008ul) {
                flips_lo |= 0x00201000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00001000ul) {
            flips_lo |= 0x00200000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_b5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000002ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00000004ul) & my_bits.high & 0x000000f8ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffffcul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 2];
    }
    /* Up right */
    if (opp_bits.low & 0x04000000ul) {
        if (opp_bits.low & 0x00080000ul) {
            if (opp_bits.low & 0x00001000ul) {
                if (my_bits.low & 0x00000020ul) {
                    flips_lo |= 0x04081000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00001000ul) {
                flips_lo |= 0x04080000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00080000ul) {
            flips_lo |= 0x04000000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00000200ul) {
        if (opp_bits.high & 0x00020000ul) {
            if (my_bits.high & 0x02000000ul) {
                flips_hi |= 0x00020200ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00020000ul) {
            flips_hi |= 0x00000200ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x02000000ul) {
        if (opp_bits.low & 0x00020000ul) {
            if (opp_bits.low & 0x00000200ul) {
                if (my_bits.low & 0x00000002ul) {
                    flips_lo |= 0x02020200ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00000200ul) {
                flips_lo |= 0x02020000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00020000ul) {
            flips_lo |= 0x02000000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00000400ul) {
        if (opp_bits.high & 0x00080000ul) {
            if (my_bits.high & 0x10000000ul) {
                flips_hi |= 0x00080400ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00080000ul) {
            flips_hi |= 0x00000400ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_g5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000040ul;
    flip_count = 0;

    /* Left */
    if (opp_bits.high & 0x00000020ul) {
        unsigned int cont = contiguous5[(opp_bits.high >> 1) & 31];
        if (cont & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 1);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00002000ul) {
        if (opp_bits.high & 0x00100000ul) {
            if (my_bits.high & 0x08000000ul) {
                flips_hi |= 0x00102000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00100000ul) {
            flips_hi |= 0x00002000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00004000ul) {
        if (opp_bits.high & 0x00400000ul) {
            if (my_bits.high & 0x40000000ul) {
                flips_hi |= 0x00404000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00400000ul) {
            flips_hi |= 0x00004000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x40000000ul) {
        if (opp_bits.low & 0x00400000ul) {
            if (opp_bits.low & 0x00004000ul) {
                if (my_bits.low & 0x00000040ul) {
                    flips_lo |= 0x40404000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00004000ul) {
                flips_lo |= 0x40400000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00400000ul) {
            flips_lo |= 0x40000000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x20000000ul) {
        if (opp_bits.low & 0x00100000ul) {
            if (opp_bits.low & 0x00000800ul) {
                if (my_bits.low & 0x00000004ul) {
                    flips_lo |= 0x20100800ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00000800ul) {
                flips_lo |= 0x20100000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00100000ul) {
            flips_lo |= 0x20000000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_d7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00080000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00100000ul) & my_bits.high & 0x00e00000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfff00000ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 20];
    }
    /* Left */
    if (opp_bits.high & 0x00040000ul) {
        unsigned int cont = contiguous2[(opp_bits.high >> 17) & 3];
        if ((cont << 16) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 17);
        }
    }
    /* Up right */
    if (opp_bits.high & 0x00001000ul) {
        if (opp_bits.high & 0x00000020ul) {
            if (opp_bits.low & 0x40000000ul) {
                if (my_bits.low & 0x00800000ul) {
                    flips_lo |= 0x40000000ul;
                    flips_hi |= 0x00001020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x40000000ul) {
                flips_hi |= 0x00001020ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000020ul) {
            flips_hi |= 0x00001000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00000800ul) {
        if (opp_bits.high & 0x00000008ul) {
            if (opp_bits.low & 0x08000000ul) {
                if (opp_bits.low & 0x00080000ul) {
                    if (opp_bits.low & 0x00000800ul) {
                        if (my_bits.low & 0x00000008ul) {
                            flips_lo |= 0x08080800ul;
                            flips_hi |= 0x00000808ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00000800ul) {
                        flips_lo |= 0x08080000ul;
                        flips_hi |= 0x00000808ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00080000ul) {
                    flips_lo |= 0x08000000ul;
                    flips_hi |= 0x00000808ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x08000000ul) {
                flips_hi |= 0x00000808ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000008ul) {
            flips_hi |= 0x00000800ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00000400ul) {
        if (opp_bits.high & 0x00000002ul) {
            if (my_bits.low & 0x01000000ul) {
                flips_hi |= 0x00000402ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000002ul) {
            flips_hi |= 0x00000400ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_e7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00100000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00200000ul) & my_bits.high & 0x00c00000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xffe00000ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 21];
    }
    /* Left */
    if (opp_bits.high & 0x00080000ul) {
        unsigned int cont = contiguous3[(opp_bits.high >> 17) & 7];
        if ((cont << 16) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 17);
        }
    }
    /* Up right */
    if (opp_bits.high & 0x00002000ul) {
        if (opp_bits.high & 0x00000040ul) {
            if (my_bits.low & 0x80000000ul) {
                flips_hi |= 0x00002040ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000040ul) {
            flips_hi |= 0x00002000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00001000ul) {
        if (opp_bits.high & 0x00000010ul) {
            if (opp_bits.low & 0x10000000ul) {
                if (opp_bits.low & 0x00100000ul) {
                    if (opp_bits.low & 0x00001000ul) {
                        if (my_bits.low & 0x00000010ul) {
                            flips_lo |= 0x10101000ul;
                            flips_hi |= 0x00001010ul;
                            flip_count += 5;
                        }
                    }
                    else if (my_bits.low & 0x00001000ul) {
                        flips_lo |= 0x10100000ul;
                        flips_hi |= 0x00001010ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00100000ul) {
                    flips_lo |= 0x10000000ul;
                    flips_hi |= 0x00001010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x10000000ul) {
                flips_hi |= 0x00001010ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000010ul) {
            flips_hi |= 0x00001000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00000800ul) {
        if (opp_bits.high & 0x00000004ul) {
            if (opp_bits.low & 0x02000000ul) {
                if (my_bits.low & 0x00010000ul) {
                    flips_lo |= 0x02000000ul;
                    flips_hi |= 0x00000804ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x02000000ul) {
                flips_hi |= 0x00000804ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000004ul) {
            flips_hi |= 0x00000800ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_c3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00040000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00080000ul) & my_bits.low & 0x00f00000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfff80000ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 19];
    }
    /* Left */
    if (opp_bits.low & 0x00020000ul) {
        if (my_bits.low & 0x00010000ul) {
            flips_lo |= 0x00020000ul;
            flip_count += 1;
        }
    }
    /* Down left */
    if (opp_bits.low & 0x02000000ul) {
        if (my_bits.high & 0x00000001ul) {
            flips_lo |= 0x02000000ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.low & 0x00000800ul) {
        if (my_bits.low & 0x00000010ul) {
            flips_lo |= 0x00000800ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x04000000ul) {
        if (opp_bits.high & 0x00000004ul) {
            if (opp_bits.high & 0x00000400ul) {
                if (opp_bits.high & 0x00040000ul) {
                    if (my_bits.high & 0x04000000ul) {
                        flips_lo |= 0x04000000ul;
                        flips_hi |= 0x00040404ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00040000ul) {
                    flips_lo |= 0x04000000ul;
                    flips_hi |= 0x00000404ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000400ul) {
                flips_lo |= 0x04000000ul;
                flips_hi |= 0x00000004ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000004ul) {
            flips_lo |= 0x04000000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00000400ul) {
        if (my_bits.low & 0x00000004ul) {
            flips_lo |= 0x00000400ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x08000000ul) {
        if (opp_bits.high & 0x00000010ul) {
            if (opp_bits.high & 0x00002000ul) {
                if (opp_bits.high & 0x00400000ul) {
                    if (my_bits.high & 0x80000000ul) {
                        flips_lo |= 0x08000000ul;
                        flips_hi |= 0x00402010ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00400000ul) {
                    flips_lo |= 0x08000000ul;
                    flips_hi |= 0x00002010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00002000ul) {
                flips_lo |= 0x08000000ul;
                flips_hi |= 0x00000010ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000010ul) {
            flips_lo |= 0x08000000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x00000200ul) {
        if (my_bits.low & 0x00000001ul) {
            flips_lo |= 0x00000200ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_f3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00200000ul;
    flip_count = 0;

    /* Right */
    if (opp_bits.low & 0x00400000ul) {
        if (my_bits.low & 0x00800000ul) {
            flips_lo |= 0x00400000ul;
            flip_count += 1;
        }
    }
    /* Left */
    if (opp_bits.low & 0x00100000ul) {
        unsigned int cont = contiguous4[(opp_bits.low >> 17) & 15];
        if ((cont << 16) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 17);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x10000000ul) {
        if (opp_bits.high & 0x00000008ul) {
            if (opp_bits.high & 0x00000400ul) {
                if (opp_bits.high & 0x00020000ul) {
                    if (my_bits.high & 0x01000000ul) {
                        flips_lo |= 0x10000000ul;
                        flips_hi |= 0x00020408ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00020000ul) {
                    flips_lo |= 0x10000000ul;
                    flips_hi |= 0x00000408ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000400ul) {
                flips_lo |= 0x10000000ul;
                flips_hi |= 0x00000008ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000008ul) {
            flips_lo |= 0x10000000ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.low & 0x00004000ul) {
        if (my_bits.low & 0x00000080ul) {
            flips_lo |= 0x00004000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x20000000ul) {
        if (opp_bits.high & 0x00000020ul) {
            if (opp_bits.high & 0x00002000ul) {
                if (opp_bits.high & 0x00200000ul) {
                    if (my_bits.high & 0x20000000ul) {
                        flips_lo |= 0x20000000ul;
                        flips_hi |= 0x00202020ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00200000ul) {
                    flips_lo |= 0x20000000ul;
                    flips_hi |= 0x00002020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00002000ul) {
                flips_lo |= 0x20000000ul;
                flips_hi |= 0x00000020ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000020ul) {
            flips_lo |= 0x20000000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00002000ul) {
        if (my_bits.low & 0x00000020ul) {
            flips_lo |= 0x00002000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x40000000ul) {
        if (my_bits.high & 0x00000080ul) {
            flips_lo |= 0x40000000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x00001000ul) {
        if (my_bits.low & 0x00000008ul) {
            flips_lo |= 0x00001000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_c6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000400ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00000800ul) & my_bits.high & 0x0000f000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffff800ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 11];
    }
    /* Left */
    if (opp_bits.high & 0x00000200ul) {
        if (my_bits.high & 0x00000100ul) {
            flips_hi |= 0x00000200ul;
            flip_count += 1;
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00020000ul) {
        if (my_bits.high & 0x01000000ul) {
            flips_hi |= 0x00020000ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.high & 0x00000008ul) {
        if (opp_bits.low & 0x10000000ul) {
            if (opp_bits.low & 0x00200000ul) {
                if (opp_bits.low & 0x00004000ul) {
                    if (my_bits.low & 0x00000080ul) {
                        flips_lo |= 0x10204000ul;
                        flips_hi |= 0x00000008ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00004000ul) {
                    flips_lo |= 0x10200000ul;
                    flips_hi |= 0x00000008ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00200000ul) {
                flips_lo |= 0x10000000ul;
                flips_hi |= 0x00000008ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x10000000ul) {
            flips_hi |= 0x00000008ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00040000ul) {
        if (my_bits.high & 0x04000000ul) {
            flips_hi |= 0x00040000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00000004ul) {
        if (opp_bits.low & 0x04000000ul) {
            if (opp_bits.low & 0x00040000ul) {
                if (opp_bits.low & 0x00000400ul) {
                    if (my_bits.low & 0x00000004ul) {
                        flips_lo |= 0x04040400ul;
                        flips_hi |= 0x00000004ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00000400ul) {
                    flips_lo |= 0x04040000ul;
                    flips_hi |= 0x00000004ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00040000ul) {
                flips_lo |= 0x04000000ul;
                flips_hi |= 0x00000004ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x04000000ul) {
            flips_hi |= 0x00000004ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00080000ul) {
        if (my_bits.high & 0x10000000ul) {
            flips_hi |= 0x00080000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00000002ul) {
        if (my_bits.low & 0x01000000ul) {
            flips_hi |= 0x00000002ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_f6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00002000ul;
    flip_count = 0;

    /* Right */
    if (opp_bits.high & 0x00004000ul) {
        if (my_bits.high & 0x00008000ul) {
            flips_hi |= 0x00004000ul;
            flip_count += 1;
        }
    }
    /* Left */
    if (opp_bits.high & 0x00001000ul) {
        unsigned int cont = contiguous4[(opp_bits.high >> 9) & 15];
        if ((cont << 8) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 9);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00100000ul) {
        if (my_bits.high & 0x08000000ul) {
            flips_hi |= 0x00100000ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.high & 0x00000040ul) {
        if (my_bits.low & 0x80000000ul) {
            flips_hi |= 0x00000040ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00200000ul) {
        if (my_bits.high & 0x20000000ul) {
            flips_hi |= 0x00200000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00000020ul) {
        if (opp_bits.low & 0x20000000ul) {
            if (opp_bits.low & 0x00200000ul) {
                if (opp_bits.low & 0x00002000ul) {
                    if (my_bits.low & 0x00000020ul) {
                        flips_lo |= 0x20202000ul;
                        flips_hi |= 0x00000020ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00002000ul) {
                    flips_lo |= 0x20200000ul;
                    flips_hi |= 0x00000020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00200000ul) {
                flips_lo |= 0x20000000ul;
                flips_hi |= 0x00000020ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x20000000ul) {
            flips_hi |= 0x00000020ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00400000ul) {
        if (my_bits.high & 0x80000000ul) {
            flips_hi |= 0x00400000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00000010ul) {
        if (opp_bits.low & 0x08000000ul) {
            if (opp_bits.low & 0x00040000ul) {
                if (opp_bits.low & 0x00000200ul) {
                    if (my_bits.low & 0x00000001ul) {
                        flips_lo |= 0x08040200ul;
                        flips_hi |= 0x00000010ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00000200ul) {
                    flips_lo |= 0x08040000ul;
                    flips_hi |= 0x00000010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00040000ul) {
                flips_lo |= 0x08000000ul;
                flips_hi |= 0x00000010ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x08000000ul) {
            flips_hi |= 0x00000010ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_d3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00080000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00100000ul) & my_bits.low & 0x00e00000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfff00000ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 20];
    }
    /* Left */
    if (opp_bits.low & 0x00040000ul) {
        unsigned int cont = contiguous2[(opp_bits.low >> 17) & 3];
        if ((cont << 16) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 17);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x04000000ul) {
        if (opp_bits.high & 0x00000002ul) {
            if (my_bits.high & 0x00000100ul) {
                flips_lo |= 0x04000000ul;
                flips_hi |= 0x00000002ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000002ul) {
            flips_lo |= 0x04000000ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.low & 0x00001000ul) {
        if (my_bits.low & 0x00000020ul) {
            flips_lo |= 0x00001000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x08000000ul) {
        if (opp_bits.high & 0x00000008ul) {
            if (opp_bits.high & 0x00000800ul) {
                if (opp_bits.high & 0x00080000ul) {
                    if (my_bits.high & 0x08000000ul) {
                        flips_lo |= 0x08000000ul;
                        flips_hi |= 0x00080808ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00080000ul) {
                    flips_lo |= 0x08000000ul;
                    flips_hi |= 0x00000808ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000800ul) {
                flips_lo |= 0x08000000ul;
                flips_hi |= 0x00000008ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000008ul) {
            flips_lo |= 0x08000000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00000800ul) {
        if (my_bits.low & 0x00000008ul) {
            flips_lo |= 0x00000800ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x10000000ul) {
        if (opp_bits.high & 0x00000020ul) {
            if (opp_bits.high & 0x00004000ul) {
                if (my_bits.high & 0x00800000ul) {
                    flips_lo |= 0x10000000ul;
                    flips_hi |= 0x00004020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00004000ul) {
                flips_lo |= 0x10000000ul;
                flips_hi |= 0x00000020ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000020ul) {
            flips_lo |= 0x10000000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x00000400ul) {
        if (my_bits.low & 0x00000002ul) {
            flips_lo |= 0x00000400ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_e3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00100000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x00200000ul) & my_bits.low & 0x00c00000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xffe00000ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 21];
    }
    /* Left */
    if (opp_bits.low & 0x00080000ul) {
        unsigned int cont = contiguous3[(opp_bits.low >> 17) & 7];
        if ((cont << 16) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 17);
        }
    }
    /* Down left */
    if (opp_bits.low & 0x08000000ul) {
        if (opp_bits.high & 0x00000004ul) {
            if (opp_bits.high & 0x00000200ul) {
                if (my_bits.high & 0x00010000ul) {
                    flips_lo |= 0x08000000ul;
                    flips_hi |= 0x00000204ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00000200ul) {
                flips_lo |= 0x08000000ul;
                flips_hi |= 0x00000004ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000004ul) {
            flips_lo |= 0x08000000ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.low & 0x00002000ul) {
        if (my_bits.low & 0x00000040ul) {
            flips_lo |= 0x00002000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.low & 0x10000000ul) {
        if (opp_bits.high & 0x00000010ul) {
            if (opp_bits.high & 0x00001000ul) {
                if (opp_bits.high & 0x00100000ul) {
                    if (my_bits.high & 0x10000000ul) {
                        flips_lo |= 0x10000000ul;
                        flips_hi |= 0x00101010ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.high & 0x00100000ul) {
                    flips_lo |= 0x10000000ul;
                    flips_hi |= 0x00001010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00001000ul) {
                flips_lo |= 0x10000000ul;
                flips_hi |= 0x00000010ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000010ul) {
            flips_lo |= 0x10000000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00001000ul) {
        if (my_bits.low & 0x00000010ul) {
            flips_lo |= 0x00001000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.low & 0x20000000ul) {
        if (opp_bits.high & 0x00000040ul) {
            if (my_bits.high & 0x00008000ul) {
                flips_lo |= 0x20000000ul;
                flips_hi |= 0x00000040ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000040ul) {
            flips_lo |= 0x20000000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x00000800ul) {
        if (my_bits.low & 0x00000004ul) {
            flips_lo |= 0x00000800ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_c4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x04000000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x08000000ul) & my_bits.low;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xf8000000ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 27];
    }
    /* Left */
    if (opp_bits.low & 0x02000000ul) {
        if (my_bits.low & 0x01000000ul) {
            flips_lo |= 0x02000000ul;
            flip_count += 1;
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00000002ul) {
        if (my_bits.high & 0x00000100ul) {
            flips_hi |= 0x00000002ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.low & 0x00080000ul) {
        if (opp_bits.low & 0x00001000ul) {
            if (my_bits.low & 0x00000020ul) {
                flips_lo |= 0x00081000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00001000ul) {
            flips_lo |= 0x00080000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00000004ul) {
        if (opp_bits.high & 0x00000400ul) {
            if (opp_bits.high & 0x00040000ul) {
                if (my_bits.high & 0x04000000ul) {
                    flips_hi |= 0x00040404ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00040000ul) {
                flips_hi |= 0x00000404ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000400ul) {
            flips_hi |= 0x00000004ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00040000ul) {
        if (opp_bits.low & 0x00000400ul) {
            if (my_bits.low & 0x00000004ul) {
                flips_lo |= 0x00040400ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00000400ul) {
            flips_lo |= 0x00040000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00000008ul) {
        if (opp_bits.high & 0x00001000ul) {
            if (opp_bits.high & 0x00200000ul) {
                if (my_bits.high & 0x40000000ul) {
                    flips_hi |= 0x00201008ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00200000ul) {
                flips_hi |= 0x00001008ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00001000ul) {
            flips_hi |= 0x00000008ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x00020000ul) {
        if (my_bits.low & 0x00000100ul) {
            flips_lo |= 0x00020000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_f4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x20000000ul;
    flip_count = 0;

    /* Right */
    if (opp_bits.low & 0x40000000ul) {
        if (my_bits.low & 0x80000000ul) {
            flips_lo |= 0x40000000ul;
            flip_count += 1;
        }
    }
    /* Left */
    if (opp_bits.low & 0x10000000ul) {
        unsigned int cont = contiguous4[(opp_bits.low >> 25) & 15];
        if ((cont << 24) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 25);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00000010ul) {
        if (opp_bits.high & 0x00000800ul) {
            if (opp_bits.high & 0x00040000ul) {
                if (my_bits.high & 0x02000000ul) {
                    flips_hi |= 0x00040810ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00040000ul) {
                flips_hi |= 0x00000810ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000800ul) {
            flips_hi |= 0x00000010ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.low & 0x00400000ul) {
        if (my_bits.low & 0x00008000ul) {
            flips_lo |= 0x00400000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00000020ul) {
        if (opp_bits.high & 0x00002000ul) {
            if (opp_bits.high & 0x00200000ul) {
                if (my_bits.high & 0x20000000ul) {
                    flips_hi |= 0x00202020ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00200000ul) {
                flips_hi |= 0x00002020ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00002000ul) {
            flips_hi |= 0x00000020ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00200000ul) {
        if (opp_bits.low & 0x00002000ul) {
            if (my_bits.low & 0x00000020ul) {
                flips_lo |= 0x00202000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00002000ul) {
            flips_lo |= 0x00200000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00000040ul) {
        if (my_bits.high & 0x00008000ul) {
            flips_hi |= 0x00000040ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x00100000ul) {
        if (opp_bits.low & 0x00000800ul) {
            if (my_bits.low & 0x00000004ul) {
                flips_lo |= 0x00100800ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00000800ul) {
            flips_lo |= 0x00100000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_c5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000004ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00000008ul) & my_bits.high & 0x000000f0ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffff8ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 3];
    }
    /* Left */
    if (opp_bits.high & 0x00000002ul) {
        if (my_bits.high & 0x00000001ul) {
            flips_hi |= 0x00000002ul;
            flip_count += 1;
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00000200ul) {
        if (my_bits.high & 0x00010000ul) {
            flips_hi |= 0x00000200ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.low & 0x08000000ul) {
        if (opp_bits.low & 0x00100000ul) {
            if (opp_bits.low & 0x00002000ul) {
                if (my_bits.low & 0x00000040ul) {
                    flips_lo |= 0x08102000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00002000ul) {
                flips_lo |= 0x08100000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00100000ul) {
            flips_lo |= 0x08000000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00000400ul) {
        if (opp_bits.high & 0x00040000ul) {
            if (my_bits.high & 0x04000000ul) {
                flips_hi |= 0x00040400ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00040000ul) {
            flips_hi |= 0x00000400ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x04000000ul) {
        if (opp_bits.low & 0x00040000ul) {
            if (opp_bits.low & 0x00000400ul) {
                if (my_bits.low & 0x00000004ul) {
                    flips_lo |= 0x04040400ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00000400ul) {
                flips_lo |= 0x04040000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00040000ul) {
            flips_lo |= 0x04000000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00000800ul) {
        if (opp_bits.high & 0x00100000ul) {
            if (my_bits.high & 0x20000000ul) {
                flips_hi |= 0x00100800ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00100000ul) {
            flips_hi |= 0x00000800ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x02000000ul) {
        if (my_bits.low & 0x00010000ul) {
            flips_lo |= 0x02000000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_f5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000020ul;
    flip_count = 0;

    /* Right */
    if (opp_bits.high & 0x00000040ul) {
        if (my_bits.high & 0x00000080ul) {
            flips_hi |= 0x00000040ul;
            flip_count += 1;
        }
    }
    /* Left */
    if (opp_bits.high & 0x00000010ul) {
        unsigned int cont = contiguous4[(opp_bits.high >> 1) & 15];
        if (cont & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 1);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00001000ul) {
        if (opp_bits.high & 0x00080000ul) {
            if (my_bits.high & 0x04000000ul) {
                flips_hi |= 0x00081000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00080000ul) {
            flips_hi |= 0x00001000ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.low & 0x40000000ul) {
        if (my_bits.low & 0x00800000ul) {
            flips_lo |= 0x40000000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00002000ul) {
        if (opp_bits.high & 0x00200000ul) {
            if (my_bits.high & 0x20000000ul) {
                flips_hi |= 0x00202000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00200000ul) {
            flips_hi |= 0x00002000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x20000000ul) {
        if (opp_bits.low & 0x00200000ul) {
            if (opp_bits.low & 0x00002000ul) {
                if (my_bits.low & 0x00000020ul) {
                    flips_lo |= 0x20202000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00002000ul) {
                flips_lo |= 0x20200000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00200000ul) {
            flips_lo |= 0x20000000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00004000ul) {
        if (my_bits.high & 0x00800000ul) {
            flips_hi |= 0x00004000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x10000000ul) {
        if (opp_bits.low & 0x00080000ul) {
            if (opp_bits.low & 0x00000400ul) {
                if (my_bits.low & 0x00000002ul) {
                    flips_lo |= 0x10080400ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00000400ul) {
                flips_lo |= 0x10080000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00080000ul) {
            flips_lo |= 0x10000000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_d6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000800ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00001000ul) & my_bits.high & 0x0000e000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffff000ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 12];
    }
    /* Left */
    if (opp_bits.high & 0x00000400ul) {
        unsigned int cont = contiguous2[(opp_bits.high >> 9) & 3];
        if ((cont << 8) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 9);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00040000ul) {
        if (my_bits.high & 0x02000000ul) {
            flips_hi |= 0x00040000ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.high & 0x00000010ul) {
        if (opp_bits.low & 0x20000000ul) {
            if (opp_bits.low & 0x00400000ul) {
                if (my_bits.low & 0x00008000ul) {
                    flips_lo |= 0x20400000ul;
                    flips_hi |= 0x00000010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00400000ul) {
                flips_lo |= 0x20000000ul;
                flips_hi |= 0x00000010ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x20000000ul) {
            flips_hi |= 0x00000010ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00080000ul) {
        if (my_bits.high & 0x08000000ul) {
            flips_hi |= 0x00080000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00000008ul) {
        if (opp_bits.low & 0x08000000ul) {
            if (opp_bits.low & 0x00080000ul) {
                if (opp_bits.low & 0x00000800ul) {
                    if (my_bits.low & 0x00000008ul) {
                        flips_lo |= 0x08080800ul;
                        flips_hi |= 0x00000008ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00000800ul) {
                    flips_lo |= 0x08080000ul;
                    flips_hi |= 0x00000008ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00080000ul) {
                flips_lo |= 0x08000000ul;
                flips_hi |= 0x00000008ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x08000000ul) {
            flips_hi |= 0x00000008ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00100000ul) {
        if (my_bits.high & 0x20000000ul) {
            flips_hi |= 0x00100000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00000004ul) {
        if (opp_bits.low & 0x02000000ul) {
            if (my_bits.low & 0x00010000ul) {
                flips_lo |= 0x02000000ul;
                flips_hi |= 0x00000004ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x02000000ul) {
            flips_hi |= 0x00000004ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_e6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00001000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00002000ul) & my_bits.high & 0x0000c000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xffffe000ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 13];
    }
    /* Left */
    if (opp_bits.high & 0x00000800ul) {
        unsigned int cont = contiguous3[(opp_bits.high >> 9) & 7];
        if ((cont << 8) & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 9);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00080000ul) {
        if (my_bits.high & 0x04000000ul) {
            flips_hi |= 0x00080000ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.high & 0x00000020ul) {
        if (opp_bits.low & 0x40000000ul) {
            if (my_bits.low & 0x00800000ul) {
                flips_lo |= 0x40000000ul;
                flips_hi |= 0x00000020ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x40000000ul) {
            flips_hi |= 0x00000020ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00100000ul) {
        if (my_bits.high & 0x10000000ul) {
            flips_hi |= 0x00100000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.high & 0x00000010ul) {
        if (opp_bits.low & 0x10000000ul) {
            if (opp_bits.low & 0x00100000ul) {
                if (opp_bits.low & 0x00001000ul) {
                    if (my_bits.low & 0x00000010ul) {
                        flips_lo |= 0x10101000ul;
                        flips_hi |= 0x00000010ul;
                        flip_count += 4;
                    }
                }
                else if (my_bits.low & 0x00001000ul) {
                    flips_lo |= 0x10100000ul;
                    flips_hi |= 0x00000010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00100000ul) {
                flips_lo |= 0x10000000ul;
                flips_hi |= 0x00000010ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x10000000ul) {
            flips_hi |= 0x00000010ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00200000ul) {
        if (my_bits.high & 0x40000000ul) {
            flips_hi |= 0x00200000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.high & 0x00000008ul) {
        if (opp_bits.low & 0x04000000ul) {
            if (opp_bits.low & 0x00020000ul) {
                if (my_bits.low & 0x00000100ul) {
                    flips_lo |= 0x04020000ul;
                    flips_hi |= 0x00000008ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00020000ul) {
                flips_lo |= 0x04000000ul;
                flips_hi |= 0x00000008ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x04000000ul) {
            flips_hi |= 0x00000008ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_d4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x08000000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x10000000ul) & my_bits.low;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xf0000000ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 28];
    }
    /* Left */
    if (opp_bits.low & 0x04000000ul) {
        unsigned int cont = contiguous2[(opp_bits.low >> 25) & 3];
        if ((cont << 24) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 25);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00000004ul) {
        if (opp_bits.high & 0x00000200ul) {
            if (my_bits.high & 0x00010000ul) {
                flips_hi |= 0x00000204ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000200ul) {
            flips_hi |= 0x00000004ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.low & 0x00100000ul) {
        if (opp_bits.low & 0x00002000ul) {
            if (my_bits.low & 0x00000040ul) {
                flips_lo |= 0x00102000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00002000ul) {
            flips_lo |= 0x00100000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00000008ul) {
        if (opp_bits.high & 0x00000800ul) {
            if (opp_bits.high & 0x00080000ul) {
                if (my_bits.high & 0x08000000ul) {
                    flips_hi |= 0x00080808ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00080000ul) {
                flips_hi |= 0x00000808ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000800ul) {
            flips_hi |= 0x00000008ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00080000ul) {
        if (opp_bits.low & 0x00000800ul) {
            if (my_bits.low & 0x00000008ul) {
                flips_lo |= 0x00080800ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00000800ul) {
            flips_lo |= 0x00080000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00000010ul) {
        if (opp_bits.high & 0x00002000ul) {
            if (opp_bits.high & 0x00400000ul) {
                if (my_bits.high & 0x80000000ul) {
                    flips_hi |= 0x00402010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00400000ul) {
                flips_hi |= 0x00002010ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00002000ul) {
            flips_hi |= 0x00000010ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x00040000ul) {
        if (opp_bits.low & 0x00000200ul) {
            if (my_bits.low & 0x00000001ul) {
                flips_lo |= 0x00040200ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00000200ul) {
            flips_lo |= 0x00040000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_e4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x10000000ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.low + 0x20000000ul) & my_bits.low;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xe0000000ul;
        flips_lo |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 29];
    }
    /* Left */
    if (opp_bits.low & 0x08000000ul) {
        unsigned int cont = contiguous3[(opp_bits.low >> 25) & 7];
        if ((cont << 24) & my_bits.low) {
            flip_count += pop_count[cont];
            flips_lo |= (cont << 25);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00000008ul) {
        if (opp_bits.high & 0x00000400ul) {
            if (opp_bits.high & 0x00020000ul) {
                if (my_bits.high & 0x01000000ul) {
                    flips_hi |= 0x00020408ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00020000ul) {
                flips_hi |= 0x00000408ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00000400ul) {
            flips_hi |= 0x00000008ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.low & 0x00200000ul) {
        if (opp_bits.low & 0x00004000ul) {
            if (my_bits.low & 0x00000080ul) {
                flips_lo |= 0x00204000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00004000ul) {
            flips_lo |= 0x00200000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00000010ul) {
        if (opp_bits.high & 0x00001000ul) {
            if (opp_bits.high & 0x00100000ul) {
                if (my_bits.high & 0x10000000ul) {
                    flips_hi |= 0x00101010ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.high & 0x00100000ul) {
                flips_hi |= 0x00001010ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00001000ul) {
            flips_hi |= 0x00000010ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x00100000ul) {
        if (opp_bits.low & 0x00001000ul) {
            if (my_bits.low & 0x00000010ul) {
                flips_lo |= 0x00101000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00001000ul) {
            flips_lo |= 0x00100000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00000020ul) {
        if (opp_bits.high & 0x00004000ul) {
            if (my_bits.high & 0x00800000ul) {
                flips_hi |= 0x00004020ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00004000ul) {
            flips_hi |= 0x00000020ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x00080000ul) {
        if (opp_bits.low & 0x00000400ul) {
            if (my_bits.low & 0x00000002ul) {
                flips_lo |= 0x00080400ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00000400ul) {
            flips_lo |= 0x00080000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_d5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000008ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00000010ul) & my_bits.high & 0x000000e0ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffff0ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 4];
    }
    /* Left */
    if (opp_bits.high & 0x00000004ul) {
        unsigned int cont = contiguous2[(opp_bits.high >> 1) & 3];
        if (cont & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 1);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00000400ul) {
        if (opp_bits.high & 0x00020000ul) {
            if (my_bits.high & 0x01000000ul) {
                flips_hi |= 0x00020400ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00020000ul) {
            flips_hi |= 0x00000400ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.low & 0x10000000ul) {
        if (opp_bits.low & 0x00200000ul) {
            if (opp_bits.low & 0x00004000ul) {
                if (my_bits.low & 0x00000080ul) {
                    flips_lo |= 0x10204000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00004000ul) {
                flips_lo |= 0x10200000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00200000ul) {
            flips_lo |= 0x10000000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00000800ul) {
        if (opp_bits.high & 0x00080000ul) {
            if (my_bits.high & 0x08000000ul) {
                flips_hi |= 0x00080800ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00080000ul) {
            flips_hi |= 0x00000800ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x08000000ul) {
        if (opp_bits.low & 0x00080000ul) {
            if (opp_bits.low & 0x00000800ul) {
                if (my_bits.low & 0x00000008ul) {
                    flips_lo |= 0x08080800ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00000800ul) {
                flips_lo |= 0x08080000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00080000ul) {
            flips_lo |= 0x08000000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00001000ul) {
        if (opp_bits.high & 0x00200000ul) {
            if (my_bits.high & 0x40000000ul) {
                flips_hi |= 0x00201000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00200000ul) {
            flips_hi |= 0x00001000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x04000000ul) {
        if (opp_bits.low & 0x00020000ul) {
            if (my_bits.low & 0x00000100ul) {
                flips_lo |= 0x04020000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00020000ul) {
            flips_lo |= 0x04000000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard_e5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned int first_flip;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000010ul;
    flip_count = 0;

    /* Right */
    first_flip = (opp_bits.high + 0x00000020ul) & my_bits.high & 0x000000c0ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xffffffe0ul;
        flips_hi |= flipped_discs;
        flip_count += pop_count[flipped_discs >> 5];
    }
    /* Left */
    if (opp_bits.high & 0x00000008ul) {
        unsigned int cont = contiguous3[(opp_bits.high >> 1) & 7];
        if (cont & my_bits.high) {
            flip_count += pop_count[cont];
            flips_hi |= (cont << 1);
        }
    }
    /* Down left */
    if (opp_bits.high & 0x00000800ul) {
        if (opp_bits.high & 0x00040000ul) {
            if (my_bits.high & 0x02000000ul) {
                flips_hi |= 0x00040800ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00040000ul) {
            flips_hi |= 0x00000800ul;
            flip_count += 1;
        }
    }
    /* Up right */
    if (opp_bits.low & 0x20000000ul) {
        if (opp_bits.low & 0x00400000ul) {
            if (my_bits.low & 0x00008000ul) {
                flips_lo |= 0x20400000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00400000ul) {
            flips_lo |= 0x20000000ul;
            flip_count += 1;
        }
    }
    /* Down */
    if (opp_bits.high & 0x00001000ul) {
        if (opp_bits.high & 0x00100000ul) {
            if (my_bits.high & 0x10000000ul) {
                flips_hi |= 0x00101000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00100000ul) {
            flips_hi |= 0x00001000ul;
            flip_count += 1;
        }
    }
    /* Up */
    if (opp_bits.low & 0x10000000ul) {
        if (opp_bits.low & 0x00100000ul) {
            if (opp_bits.low & 0x00001000ul) {
                if (my_bits.low & 0x00000010ul) {
                    flips_lo |= 0x10101000ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00001000ul) {
                flips_lo |= 0x10100000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00100000ul) {
            flips_lo |= 0x10000000ul;
            flip_count += 1;
        }
    }
    /* Down right */
    if (opp_bits.high & 0x00002000ul) {
        if (opp_bits.high & 0x00400000ul) {
            if (my_bits.high & 0x80000000ul) {
                flips_hi |= 0x00402000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.high & 0x00400000ul) {
            flips_hi |= 0x00002000ul;
            flip_count += 1;
        }
    }
    /* Up left */
    if (opp_bits.low & 0x08000000ul) {
        if (opp_bits.low & 0x00040000ul) {
            if (opp_bits.low & 0x00000200ul) {
                if (my_bits.low & 0x00000001ul) {
                    flips_lo |= 0x08040200ul;
                    flip_count += 3;
                }
            }
            else if (my_bits.low & 0x00000200ul) {
                flips_lo |= 0x08040000ul;
                flip_count += 2;
            }
        }
        else if (my_bits.low & 0x00040000ul) {
            flips_lo |= 0x08000000ul;
            flip_count += 1;
        }
    }
    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

int(*TestFlips_bitboard[64])(const BitBoard, const BitBoard) = {

    TestFlips_bitboard_a1,
    TestFlips_bitboard_b1,
    TestFlips_bitboard_c1,
    TestFlips_bitboard_d1,
    TestFlips_bitboard_e1,
    TestFlips_bitboard_f1,
    TestFlips_bitboard_g1,
    TestFlips_bitboard_h1,

    TestFlips_bitboard_a2,
    TestFlips_bitboard_b2,
    TestFlips_bitboard_c2,
    TestFlips_bitboard_d2,
    TestFlips_bitboard_e2,
    TestFlips_bitboard_f2,
    TestFlips_bitboard_g2,
    TestFlips_bitboard_h2,

    TestFlips_bitboard_a3,
    TestFlips_bitboard_b3,
    TestFlips_bitboard_c3,
    TestFlips_bitboard_d3,
    TestFlips_bitboard_e3,
    TestFlips_bitboard_f3,
    TestFlips_bitboard_g3,
    TestFlips_bitboard_h3,

    TestFlips_bitboard_a4,
    TestFlips_bitboard_b4,
    TestFlips_bitboard_c4,
    TestFlips_bitboard_d4,
    TestFlips_bitboard_e4,
    TestFlips_bitboard_f4,
    TestFlips_bitboard_g4,
    TestFlips_bitboard_h4,

    TestFlips_bitboard_a5,
    TestFlips_bitboard_b5,
    TestFlips_bitboard_c5,
    TestFlips_bitboard_d5,
    TestFlips_bitboard_e5,
    TestFlips_bitboard_f5,
    TestFlips_bitboard_g5,
    TestFlips_bitboard_h5,

    TestFlips_bitboard_a6,
    TestFlips_bitboard_b6,
    TestFlips_bitboard_c6,
    TestFlips_bitboard_d6,
    TestFlips_bitboard_e6,
    TestFlips_bitboard_f6,
    TestFlips_bitboard_g6,
    TestFlips_bitboard_h6,

    TestFlips_bitboard_a7,
    TestFlips_bitboard_b7,
    TestFlips_bitboard_c7,
    TestFlips_bitboard_d7,
    TestFlips_bitboard_e7,
    TestFlips_bitboard_f7,
    TestFlips_bitboard_g7,
    TestFlips_bitboard_h7,

    TestFlips_bitboard_a8,
    TestFlips_bitboard_b8,
    TestFlips_bitboard_c8,
    TestFlips_bitboard_d8,
    TestFlips_bitboard_e8,
    TestFlips_bitboard_f8,
    TestFlips_bitboard_g8,
    TestFlips_bitboard_h8

};
