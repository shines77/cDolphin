/*
   File:           bitbtest5.cpp

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
#include "bitbtest5.h"

ALIGN_PREFIX(16) BitBoard bb_flips5 ALIGN_SUFFIX(16);

ALIGN_PREFIX(64) static unsigned int contiguous4[16] ALIGN_SUFFIX(64) = {
    0,  0,  0,  0,  0,  0,  0,  0,  8,  8,  8,  8, 12, 12, 14, 15
};

ALIGN_PREFIX(16) static unsigned int contiguous3[8] ALIGN_SUFFIX(64) = {
    0,  0,  0,  0,  4,  4,  6,  7
};

ALIGN_PREFIX(16) static unsigned int contiguous2[4] ALIGN_SUFFIX(64) = {
    0,  0,  2,  3
};

/*
ALIGN_PREFIX(64) static unsigned int contiguous1[2] ALIGN_SUFFIX(64) = {
    0,  1
};
//*/

ALIGN_PREFIX(64) static unsigned char pop_count[64] ALIGN_SUFFIX(64) = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,    /*  0 -- 15 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,    /* 16 -- 31 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,    /* 32 -- 47 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6     /* 48 -- 63 */
};

ALIGN_PREFIX(64) static unsigned char discs_count[64] ALIGN_SUFFIX(64) = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,    /*  0 -- 15 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,    /* 16 -- 31 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,    /* 32 -- 47 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6     /* 48 -- 63 */
};

/* rank(ÐÐ), file(ÁÐ), diag1, diag2 */
ALIGN_PREFIX(64) static unsigned char opp_mask[8][64] ALIGN_SUFFIX(64);
ALIGN_PREFIX(64) static unsigned char my_mask[8][256] ALIGN_SUFFIX(64);

//ALIGN_PREFIX(64) static const unsigned int lsb_mask[8] ALIGN_SUFFIX(64)   = { 0x000000FFul, 0x0000FFFFul, 0x00FFFFFFul, 0xFFFFFFFFul, 0, 0, 0, 0 };
ALIGN_PREFIX(64) static const unsigned int lsb_mask[8] ALIGN_SUFFIX(64) = { 0x00000000ul, 0x000000FFul, 0x0000FFFFul, 0x00FFFFFFul, 0, 0, 0, 0 };
//static const unsigned int msb_mask[4]   = { 0xFF000000ul, 0xFFFF0000ul, 0xFFFFFF00ul, 0xFFFFFFFFul };

///*
static const unsigned int right_flip[8] = {
    0x00000001ul, 0x00000003ul, 0x00000007ul, 0x0000000Ful,
    0x0000001Ful, 0x0000003Ful, 0x0000007Ful, 0,
};
//*/

/*
static const unsigned int left_flip[8]  = {
    0x80000000ul, 0xC0000000ul, 0xE0000000ul, 0xF0000000ul,
    0xF8000000ul, 0xFC000000ul, 0xFE000000ul, 0
};
//*/

ALIGN_PREFIX(64) static unsigned char rank_right_mask[128] ALIGN_SUFFIX(64) = {
    0
};

ALIGN_PREFIX(64) static unsigned char rank_left_mask[64] ALIGN_SUFFIX(64) = {
    0
};

ALIGN_PREFIX(64) static unsigned char file_high_contig[128] ALIGN_SUFFIX(64) = {
    0
};

ALIGN_PREFIX(64) static unsigned char file_low_contig[64] ALIGN_SUFFIX(64) = {
    0
};

void
init_disc_mask_5(void) {
    int i, j, k;
    unsigned int mask_bit, now_bit, l_bit, r_bit;
    unsigned int mask, n_mask, l_mask, r_mask;
    unsigned int mask_left[8], mask_right[8];
    int d_count, l_count, r_count;
    int mask_b, mask_w;
    int t, contig, flips_mask;

    now_bit = 1;
    mask_left[0] = 0;
    mask_right[0] = 0xfe;
    for (i = 1; i < 8; i++) {
        mask_left[i] = mask_left[i - 1] | now_bit;
        mask_right[i] = (mask_right[i - 1] << 1) & 0xff;
        now_bit <<= 1;
    }

    now_bit = 1;
    for (i = 0; i < 8; i++) {
        for (n_mask = 0; n_mask < 64; n_mask++) {
            mask = n_mask << 1;
            if ((mask & now_bit) == (now_bit + 0x80000000ul)) {
                opp_mask[i][n_mask] = 0;
            }
            else {
                // left direction
                l_mask = mask & mask_left[i];
                mask_bit = now_bit >> 1;
                d_count = 0;
                for (j = i - 1; j >= 0; j--) {
                    if ((mask_bit & l_mask) == 0)
                        break;
                    mask_bit >>= 1;
                    d_count++;
                }
                if (d_count == 0)
                    l_mask = 0;
                else
                    l_mask = mask_bit & mask_left[i];

                // right direction
                r_mask = mask & mask_right[i];
                mask_bit = now_bit << 1;
                d_count = 0;
                for (j = i + 1; j < 8; j++) {
                    if ((mask_bit & r_mask) == 0)
                        break;
                    mask_bit <<= 1;
                    d_count++;
                }
                if (d_count == 0)
                    r_mask = 0;
                else
                    r_mask = mask_bit & mask_right[i];

                opp_mask[i][n_mask] = l_mask | r_mask;
            }
        }
        now_bit <<= 1;
    }

    for (i = 0; i < 8; i++) {
        for (mask = 0; mask < 256; mask++) {
            my_mask[i][mask] = 0;
        }
    }

    now_bit = 1;
    for (i = 0; i < 8; i++) {
        // left dir
        j = i - 1;
        l_count = 0;
        l_bit = now_bit;
        do {
            l_bit = (l_bit >> 1) & mask_left[i];
            if (l_count == 0 || j < 0)
                l_mask = 0;
            else
                l_mask = (now_bit - 1) - ((l_bit << 1) - 1);

            // right dir
            k = i + 1;
            r_count = 0;
            r_bit = now_bit;
            do {
                r_bit = (r_bit << 1) & mask_right[i];
                if (r_count == 0 || k >= 8)
                    r_mask = 0;
                else
                    r_mask = (r_bit - 1) - now_bit - (now_bit - 1);

                n_mask = (l_mask & mask_left[i]) | (r_mask & mask_right[i]);

                // mask = r_bit - now_bit - (l_bit << 1);
                mask = l_bit | r_bit;
                my_mask[i][mask] = n_mask;
                if (l_count == 0)
                    my_mask[i][r_bit] = n_mask;
                if (r_count == 0)
                    my_mask[i][l_bit] = n_mask;

                r_count++;
                k++;
            } while (k < 8);

            l_count++;
            j--;
        } while (j >= 0);

        now_bit <<= 1;
    }

    for (mask_w = 0; mask_w < 8; mask_w++) {
        contig = 0;
        t = mask_w & 1;
        contig += t;
        t &= (mask_w >> 1);
        contig += t;
        t &= (mask_w >> 2);
        contig += t;
        flips_mask = 1 << contig;
        for (mask_b = 0; mask_b < 16; mask_b++) {
            mask = (mask_w << 4) | mask_b;
            if ((mask_w & mask_b) != 0) {
                rank_right_mask[mask] = 0;
                file_high_contig[mask] = 0;
            }
            else {
                rank_right_mask[mask] = 0;
                file_high_contig[mask] = 0;
                if (contig == 0) {
                    if (mask_b & flips_mask) {
                        rank_right_mask[mask] = (unsigned char)(0x0eul | ((flips_mask - 1) << 4));
                        file_high_contig[mask] = 4;
                    }
                }
                else {
                    if (mask_b & flips_mask) {
                        rank_right_mask[mask] = (unsigned char)(0x0eul | ((flips_mask - 1) << 4));
                        file_high_contig[mask] = contig;
                    }
                }
            }
        }
    }

    for (mask_w = 0; mask_w < 4; mask_w++) {
        contig = 0;
        t = mask_w & 1;
        contig += t;
        t &= (mask_w >> 1);
        contig += t;
        flips_mask = 1 << (contig + 1);
        for (mask_b = 0; mask_b < 16; mask_b++) {
            mask = (mask_w << 4) | mask_b;
            if (((mask_w << 1) & mask_b) != 0) {
                rank_left_mask[mask] = 0;
                file_low_contig[mask] = 0;
            }
            else {
                rank_left_mask[mask] = 0;
                file_low_contig[mask] = 0;
                if (contig > 0) {
                    if (mask_b & flips_mask) {
                        rank_left_mask[mask] = (flips_mask - 1);
                        file_low_contig[mask] = contig;
                    }
                }
            }
        }
    }
}

#define RANK_A1   0
#define FILE_A1   0
#define DIAG1_A1  0

#if 0
static int
TestFlips_bitboard5_a1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    unsigned long comm_mask, flips_mask;
    int flips_contig;
    int flip_count;

    flips_hi = 0;
    flips_lo = 0;
    flip_count = 0;

    /* Rank: Left */
    /*
    if ( opp_bits.low & 0x00000002ul ) {
        if ( ( ~opp_bits.low & 0x0000000eul ) == 0 ) {
            comm_mask = ((my_bits.low & 0x000000f0ul) >> 4) | (opp_bits.low & 0x00000070ul);
            flips_mask = rank_right_mask[comm_mask] & 0x0000007eul;
            flips_lo |= flips_mask;
            flip_count += discs_count[flips_mask >> 1];
        }
        else {
            comm_mask = (my_bits.low & 0x0000000ful) | ((opp_bits.low & 0x00000006ul) << 3);
            flips_mask = rank_left_mask[comm_mask] & 0x0000000eul;
            flips_lo |= flips_mask;
            flip_count += discs_count[flips_mask >> 1];
        }
    }
    //*/
    ///*
    if (opp_bits.low & 0x00000002ul) {
        if ((~opp_bits.low & 0x0000000eul) == 0) {
            comm_mask = ((my_bits.low & 0x000000f0ul) >> 4) | (opp_bits.low & 0x00000070ul);
            flips_contig = file_high_contig[comm_mask];
            if (flips_contig) {
                flips_lo |= (right_flip[flips_contig + 3] & 0x00000070ul) | 0x0000000eul;
                flip_count += (flips_contig & 3) + 3;
            }
        }
        else {
            comm_mask = (my_bits.low & 0x0000000ful) | ((opp_bits.low & 0x00000006ul) << 3);
            flips_contig = file_low_contig[comm_mask];
            //if (flips_contig) {
                flips_lo |= right_flip[flips_contig] & 0x0000000eul;
                flip_count += flips_contig;
            //}
        }
    }
    //*/

    /* File: Down */
    if (opp_bits.low & 0x00000100ul) {
        if ((~opp_bits.low & 0x01010100ul) == 0) {
            comm_mask = (((my_bits.high & 0x01010101ul) | ((opp_bits.high & 0x00010101ul) << 4)) * 0x01020408ul) >> 24;
            flips_contig = file_high_contig[comm_mask];
            if (flips_contig) {
                flips_lo |= 0x01010100ul;
                flips_hi |= lsb_mask[flips_contig] & 0x00010101ul;
                flip_count += (flips_contig & 3) + 3;
            }
        }
        else {
            comm_mask = (((my_bits.low & 0x01010101ul) | ((opp_bits.low & 0x00010100ul) >> 4)) * 0x01020408ul) >> 24;
            flips_contig = file_low_contig[comm_mask];
            //if (flips_contig) {
                flips_lo |= lsb_mask[flips_contig + 1] & 0x01010100ul;
                flip_count += flips_contig;
            //}
        }
    }

    /* Diagonal-1: Down right */
    if (opp_bits.low & 0x00000200ul) {
        if ((~opp_bits.low & 0x08040200ul) == 0) {
            comm_mask = ((((my_bits.high & 0x80402010ul) >> 4) | (opp_bits.high & 0x00402010ul)) * 0x01010101ul) >> 24;
            flips_contig = file_high_contig[comm_mask];
            if (flips_contig) {
                flips_lo |= 0x08040200ul;
                flips_hi |= lsb_mask[flips_contig] & 0x00402010ul;
                flip_count += (flips_contig & 3) + 3;
            }
        }
        else {
            comm_mask = (((my_bits.low & 0x08040201ul) | ((opp_bits.low & 0x00040200ul) >> 5)) * 0x01010101ul) >> 24;
            flips_contig = file_low_contig[comm_mask];
            //if (flips_contig) {
                flips_lo |= lsb_mask[flips_contig + 1] & 0x08040200ul;
                flip_count += flips_contig;
            //}
        }
    }

    flips_lo |= 0x00000001ul;
    //bb_flip_bits.low = flips_lo;
    //bb_flip_bits.high = flips_hi;

    flips_lo |= my_bits.low;
    flips_hi |= my_bits.high;

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#else

static int
TestFlips_bitboard5_a1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    unsigned long comm_mask;
    //unsigned long flips_mask;
    int flips_contig, t;
    int flip_count;

    flips_hi = 0;
    flips_lo = 0;
    flip_count = 0;

    ///*
    if (opp_bits.low & 0x00000002ul) {
        if ((~opp_bits.low & 0x0000000eul) == 0) {
            comm_mask = ((my_bits.low & 0x000000f0ul) >> 4) | (opp_bits.low & 0x00000070ul);
            flips_contig = file_high_contig[comm_mask];
            t = (-flips_contig) >> 31;
            flips_lo |= ((right_flip[flips_contig + 3] & 0x00000070ul) | 0x0000000eul) & t;
            flip_count += ((flips_contig & 3) + 3) & t;
        }
        else {
            comm_mask = (my_bits.low & 0x0000000ful) | ((opp_bits.low & 0x00000006ul) << 3);
            flips_contig = file_low_contig[comm_mask];
            //if (flips_contig) {
                flips_lo |= right_flip[flips_contig] & 0x0000000eul;
                flip_count += flips_contig;
            //}
        }
    }
    //*/

    /* File: Down */
    if (opp_bits.low & 0x00000100ul) {
        if ((~opp_bits.low & 0x01010100ul) == 0) {
            comm_mask = (((my_bits.high & 0x01010101ul) | ((opp_bits.high & 0x00010101ul) << 4)) * 0x01020408ul) >> 24;
            flips_contig = file_high_contig[comm_mask];
            t = (-flips_contig) >> 31;
            flips_lo |= 0x01010100ul & t;
            flips_hi |= (lsb_mask[flips_contig] & 0x00010101ul) & t;
            flip_count += ((flips_contig & 3) + 3) & t;
        }
        else {
            comm_mask = (((my_bits.low & 0x01010101ul) | ((opp_bits.low & 0x00010100ul) >> 4)) * 0x01020408ul) >> 24;
            flips_contig = file_low_contig[comm_mask];
            //if (flips_contig) {
                    flips_lo |= lsb_mask[flips_contig + 1] & 0x01010100ul;
                flip_count += flips_contig;
            //}
        }
    }

    /* Diagonal-1: Down right */
    if (opp_bits.low & 0x00000200ul) {
        if ((~opp_bits.low & 0x08040200ul) == 0) {
            comm_mask = ((((my_bits.high & 0x80402010ul) >> 4) | (opp_bits.high & 0x00402010ul)) * 0x01010101ul) >> 24;
            flips_contig = file_high_contig[comm_mask];
            t = (-flips_contig) >> 31;
            flips_lo |= 0x08040200ul & t;
            flips_hi |= (lsb_mask[flips_contig] & 0x00402010ul) & t;
            flip_count += ((flips_contig & 3) + 3) & t;
        }
        else {
            comm_mask = (((my_bits.low & 0x08040201ul) | ((opp_bits.low & 0x00040200ul) >> 5)) * 0x01010101ul) >> 24;
            flips_contig = file_low_contig[comm_mask];
            //if (flips_contig) {
                flips_lo |= lsb_mask[flips_contig + 1] & 0x08040200ul;
                flip_count += flips_contig;
            //}
        }
    }

    flips_lo |= 0x00000001ul;
    //bb_flip_bits.low = flips_lo;
    //bb_flip_bits.high = flips_hi;

    flips_lo |= my_bits.low;
    flips_hi |= my_bits.high;

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#endif

#define RANK_H1   7
#define FILE_H1   0
#define DIAG2_H1  7

static int
TestFlips_bitboard5_h1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000080ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x0000007eul) >> 1;
    my_discs = my_bits.low & 0x000000fful;

    opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    opp_discs3 = ((((opp_bits.low & 0x10204080ul) | (opp_bits.high & 0x01020408ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x10204080ul) | (my_bits.high & 0x01020408ul)) * 0x01010101ul) & 0xff000000ul) >> 24;

    flipped_discs = my_mask[RANK_H1][opp_mask[RANK_H1][opp_discs] & my_discs];
    flips_lo |= flipped_discs;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    //my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[FILE_H1][opp_mask[FILE_H1][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 7;
    flips_hi |= (flipped_bits & 0x10101010ul) << 3;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x10204080ul) | (opp_bits.high & 0x01020408ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x10204080ul) | (my_bits.high & 0x01020408ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[DIAG2_H1][opp_mask[DIAG2_H1][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x10204080ul;
    flips_hi |= flipped_bits & 0x01020408ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_A8   0
#define FILE_A8   7
#define DIAG2_A8  0

static int
TestFlips_bitboard5_a8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x01000000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x7e000000ul) >> 25;
    my_discs = (my_bits.high & 0xff000000ul) >> 24;

    opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    opp_discs3 = ((((opp_bits.low & 0x10204080ul) | (opp_bits.high & 0x01020408ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x10204080ul) | (my_bits.high & 0x01020408ul)) * 0x01010101ul) & 0xff000000ul) >> 24;

    flipped_discs = my_mask[RANK_A8][opp_mask[RANK_A8][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 24;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    //my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    flipped_discs = my_mask[FILE_A8][opp_mask[FILE_A8][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= flipped_bits & 0x01010101ul;
    flips_hi |= (flipped_bits >> 4) & 0x01010101ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x10204080ul) | (opp_bits.high & 0x01020408ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x10204080ul) | (my_bits.high & 0x01020408ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[DIAG2_A8][opp_mask[DIAG2_A8][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x10204080ul;
    flips_hi |= flipped_bits & 0x01020408ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_H8   7
#define FILE_H8   7
#define DIAG1_H8  7

static int
TestFlips_bitboard5_h8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x80000000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x7e000000ul) >> 25;
    my_discs = (my_bits.high & 0xff000000ul) >> 24;

    opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    opp_discs3 = ((((opp_bits.low & 0x08040201ul) | (opp_bits.high & 0x80402010ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x08040201ul) | (my_bits.high & 0x80402010ul)) * 0x01010101ul) & 0xff000000ul) >> 24;

    flipped_discs = my_mask[RANK_H8][opp_mask[RANK_H8][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 24;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    //my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[FILE_H8][opp_mask[FILE_H8][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 7;
    flips_hi |= (flipped_bits & 0x10101010ul) << 3;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x08040201ul) | (opp_bits.high & 0x80402010ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x08040201ul) | (my_bits.high & 0x80402010ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_H8][opp_mask[DIAG1_H8][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x08040201ul;
    flips_hi |= flipped_bits & 0x80402010ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_B1   1
#define FILE_B1   0
#define DIAG1_B1  0

static int
TestFlips_bitboard5_b1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000002ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x0000007eul) >> 1;
    my_discs = my_bits.low & 0x000000fful;

    opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x10080402ul) | (opp_bits.high & 0x00804020ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    my_discs3 = ((((my_bits.low & 0x10080402ul) | (my_bits.high & 0x00804020ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;

    flipped_discs = my_mask[RANK_B1][opp_mask[RANK_B1][opp_discs] & my_discs];
    flips_lo |= flipped_discs;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_B1][opp_mask[FILE_B1][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00408102ul;
    flips_lo |= flipped_bits & 0x02020202ul;
    flips_hi |= (flipped_bits >> 4) & 0x02020202ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x10080402ul) | (opp_bits.high & 0x00804020ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    //my_discs3 = ((((my_bits.low & 0x10080402ul) | (my_bits.high & 0x00804020ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;
    flipped_discs = my_mask[DIAG1_B1][opp_mask[DIAG1_B1][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x02020202ul;
    flips_lo |= flipped_bits & 0x10080402ul;
    flips_hi |= flipped_bits & 0x00804020ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_G1   6
#define FILE_G1   0
#define DIAG2_G1  6

#if 1
static int
TestFlips_bitboard5_g1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long flips_hi;
    register unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000040ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x0000007eul) >> 1;
    my_discs = my_bits.low & 0x000000fful;

    ///*
    opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;

    opp_discs3 = ((((opp_bits.low & 0x08102040ul) | (opp_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x08102040ul) | (my_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;
    //*/

    flipped_discs = my_mask[RANK_G1][opp_mask[RANK_G1][opp_discs] & my_discs];
    flips_lo |= flipped_discs;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_G1][opp_mask[FILE_G1][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 6;
    flips_hi |= (flipped_bits & 0x10101010ul) << 2;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x08102040ul) | (opp_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x08102040ul) | (my_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;
    flipped_discs = my_mask[DIAG2_G1][opp_mask[DIAG2_G1][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x08102040ul;
    flips_hi |= flipped_bits & 0x00010204ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#else

_declspec(naked) static int
TestFlips_bitboard5_g1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    __asm {
        push edi
        push esi
        push ebp
        push ebx
        push ecx

        mov ebp, dword ptr[esp + 0x18]; my_bits.low
        mov ecx, dword ptr[esp + 0x20]; opp_bits.low
        mov eax, ecx
        and eax, 0x0000007E
        shr eax, 1
        mov esi, ebp
        and esi, 0x000000FF
        and esi, dword ptr[offset opp_mask + eax * 4 + RANK_G1 * 256]

        mov eax, dword ptr[esp + 0x24]; opp_bits.high
        mov ebx, dword ptr[esp + 0x1C]; my_bits.high

        prefetchnta[offset my_mask + esi * 4 + RANK_G1 * 1024];

        mov edi, ecx
        and edi, 0x40404040
        and ecx, 0x08102040
        shr edi, 4
        mov edx, eax
        and edx, 0x00404040
        and eax, 0x00010204
        or edi, edx
        imul edx, edi, 0x00204081
        or ecx, eax
        imul eax, ecx, 0x01010101
        //prefetchnta [offset my_mask + esi*4 + RANK_G1*1024];
        mov edi, ebp
        and edx, 0x3F000000
        and eax, 0x3E000000
        and edi, 0x40404040
        shr edx, 24; opp_discs2
        shr eax, 25; opp_discs3

        mov ecx, ebp
        and ecx, 0x08102040
        shr edi, 4
        mov dword ptr[esp], eax; opp_discs3
        mov eax, ebx
        and eax, 0x40404040
        or edi, eax
        imul eax, edi, 0x00204081
        mov edi, ebx
        and edi, 0x00010204
        or ecx, edi
        imul edi, ecx, 0x01010101
        mov ecx, dword ptr[offset my_mask + esi * 4 + RANK_G1 * 1024]
        and eax, 0x7F800000
        or ebp, 0x00000040
        or ebp, ecx
        and edi, 0x7F000000
        shr eax, 23; my_discs2
        shr edi, 24; my_discs3
        and eax, dword ptr[offset opp_mask + edx * 4 + FILE_G1 * 256]
        mov edx, dword ptr[offset my_mask + eax * 4 + FILE_G1 * 1024]
        shr ecx, 1
        mov eax, edx
        mov ecx, dword ptr[offset discs_count + ecx * 4]
        and eax, 0x0000007E
        imul eax, 0x00204081
        shr edx, 1
        add ecx, dword ptr[offset discs_count + edx * 4]
        mov edx, dword ptr[esp]
        mov esi, eax
        and edi, dword ptr[offset opp_mask + edx * 4 + DIAG2_G1 * 256]
        and esi, 0x01010101
        shl esi, 6
        and eax, 0x10101010
        or ebp, esi
        mov esi, dword ptr[offset my_mask + edi * 4 + DIAG2_G1 * 1024]
        shl eax, 2
        or eax, ebx
        mov edx, esi
        and edx, 0x0000003E
        imul ebx, edx, 0x01010101
        shr esi, 1
        add ecx, dword ptr[offset discs_count + esi * 4]
        mov edx, ebx
        and edx, 0x08102040
        or ebp, edx
        and ebx, 0x00010204
        mov dword ptr[offset bb_flips.low], ebp
        or eax, ebx
        mov dword ptr[offset bb_flips.high], eax
        mov eax, ecx

        //emms

        pop ecx
        pop ebx
        pop ebp
        pop esi
        pop edi
        ret
    }
}

#endif

#define RANK_A2   0
#define FILE_A2   1
#define DIAG1_A2  0

static int
TestFlips_bitboard5_a2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000100ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x00007e00ul) >> 9;
    my_discs = (my_bits.low & 0x0000ff00ul) >> 8;

    opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    opp_discs3 = ((((opp_bits.low & 0x04020100ul) | (opp_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x04020100ul) | (my_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;

    flipped_discs = my_mask[RANK_A2][opp_mask[RANK_A2][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    //my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    flipped_discs = my_mask[FILE_A2][opp_mask[FILE_A2][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= flipped_bits & 0x01010101ul;
    flips_hi |= (flipped_bits >> 4) & 0x01010101ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x04020100ul) | (opp_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x04020100ul) | (my_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_A2][opp_mask[DIAG1_A2][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x04020100ul;
    flips_hi |= flipped_bits & 0x40201008ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_H2   7
#define FILE_H2   1
#define DIAG2_H2  6

static int
TestFlips_bitboard5_h2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00008000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x00007e00ul) >> 9;
    my_discs = (my_bits.low & 0x0000ff00ul) >> 8;

    opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    opp_discs3 = ((((opp_bits.low & 0x20408000ul) | (opp_bits.high & 0x02040810ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    my_discs3 = ((((my_bits.low & 0x20408000ul) | (my_bits.high & 0x02040810ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;

    flipped_discs = my_mask[RANK_H2][opp_mask[RANK_H2][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    //my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[FILE_H2][opp_mask[FILE_H2][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 7;
    flips_hi |= (flipped_bits & 0x10101010ul) << 3;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x20408000ul) | (opp_bits.high & 0x02040810ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    //my_discs3 = ((((my_bits.low & 0x20408000ul) | (my_bits.high & 0x02040810ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;
    flipped_discs = my_mask[DIAG2_H2][opp_mask[DIAG2_H2][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x02020202ul;
    flips_lo |= flipped_bits & 0x20408000ul;
    flips_hi |= flipped_bits & 0x02040810ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_A7   0
#define FILE_A7   6
#define DIAG2_A7  0

static int
TestFlips_bitboard5_a7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00010000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x007e0000ul) >> 17;
    my_discs = (my_bits.high & 0x00ff0000ul) >> 16;

    opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    opp_discs3 = ((((opp_bits.low & 0x08102040ul) | (opp_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x08102040ul) | (my_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;

    flipped_discs = my_mask[RANK_A7][opp_mask[RANK_A7][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    //my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    flipped_discs = my_mask[FILE_A7][opp_mask[FILE_A7][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= flipped_bits & 0x01010101ul;
    flips_hi |= (flipped_bits >> 4) & 0x01010101ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x08102040ul) | (opp_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x08102040ul) | (my_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;
    flipped_discs = my_mask[DIAG2_A7][opp_mask[DIAG2_A7][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x08102040ul;
    flips_hi |= flipped_bits & 0x00010204ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_H7   7
#define FILE_H7   6
#define DIAG1_H7  6

static int
TestFlips_bitboard5_h7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00800000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x007e0000ul) >> 17;
    my_discs = (my_bits.high & 0x00ff0000ul) >> 16;

    opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    opp_discs3 = ((((opp_bits.low & 0x10080402ul) | (opp_bits.high & 0x00804020ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    my_discs3 = ((((my_bits.low & 0x10080402ul) | (my_bits.high & 0x00804020ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;

    flipped_discs = my_mask[RANK_H7][opp_mask[RANK_H7][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    //my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[FILE_H7][opp_mask[FILE_H7][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 7;
    flips_hi |= (flipped_bits & 0x10101010ul) << 3;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x10080402ul) | (opp_bits.high & 0x00804020ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    //my_discs3 = ((((my_bits.low & 0x10080402ul) | (my_bits.high & 0x00804020ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;
    flipped_discs = my_mask[DIAG1_H7][opp_mask[DIAG1_H7][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x02020202ul;
    flips_lo |= flipped_bits & 0x10080402ul;
    flips_hi |= flipped_bits & 0x00804020ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_B8   1
#define FILE_B8   7
#define DIAG2_B8  0

static int
TestFlips_bitboard5_b8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x02000000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x7e000000ul) >> 25;
    my_discs = (my_bits.high & 0xff000000ul) >> 24;

    opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x20408000ul) | (opp_bits.high & 0x02040810ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    my_discs3 = ((((my_bits.low & 0x20408000ul) | (my_bits.high & 0x02040810ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;

    flipped_discs = my_mask[RANK_B8][opp_mask[RANK_B8][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 24;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_B8][opp_mask[FILE_B8][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00408102ul;
    flips_lo |= flipped_bits & 0x02020202ul;
    flips_hi |= (flipped_bits >> 4) & 0x02020202ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x20408000ul) | (opp_bits.high & 0x02040810ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    //my_discs3 = ((((my_bits.low & 0x20408000ul) | (my_bits.high & 0x02040810ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;
    flipped_discs = my_mask[DIAG2_B8][opp_mask[DIAG2_B8][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x02020202ul;
    flips_lo |= flipped_bits & 0x20408000ul;
    flips_hi |= flipped_bits & 0x02040810ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_G8   6
#define FILE_G8   7
#define DIAG1_G8  6

static int
TestFlips_bitboard5_g8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x40000000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x7e000000ul) >> 25;
    my_discs = (my_bits.high & 0xff000000ul) >> 24;

    opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x04020100ul) | (opp_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x04020100ul) | (my_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;

    flipped_discs = my_mask[RANK_G8][opp_mask[RANK_G8][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 24;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_G8][opp_mask[FILE_G8][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 6;
    flips_hi |= (flipped_bits & 0x10101010ul) << 2;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x04020100ul) | (opp_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x04020100ul) | (my_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_G8][opp_mask[DIAG1_G8][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x04020100ul;
    flips_hi |= flipped_bits & 0x40201008ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_B2   1
#define FILE_B2   1
#define DIAG1_B2  1

static int
TestFlips_bitboard5_b2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000200ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x00007e00ul) >> 9;
    my_discs = (my_bits.low & 0x0000ff00ul) >> 8;

    opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x08040201ul) | (opp_bits.high & 0x80402010ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x08040201ul) | (my_bits.high & 0x80402010ul)) * 0x01010101ul) & 0xff000000ul) >> 24;

    flipped_discs = my_mask[RANK_B2][opp_mask[RANK_B2][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_B2][opp_mask[FILE_B2][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00408102ul;
    flips_lo |= flipped_bits & 0x02020202ul;
    flips_hi |= (flipped_bits >> 4) & 0x02020202ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x08040201ul) | (opp_bits.high & 0x80402010ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x08040201ul) | (my_bits.high & 0x80402010ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_B2][opp_mask[DIAG1_B2][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x08040201ul;
    flips_hi |= flipped_bits & 0x80402010ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_G2   6
#define FILE_G2   1
#define DIAG2_G2  6

static int
TestFlips_bitboard5_g2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00004000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x00007e00ul) >> 9;
    my_discs = (my_bits.low & 0x0000ff00ul) >> 8;

    opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x10204080ul) | (opp_bits.high & 0x01020408ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x10204080ul) | (my_bits.high & 0x01020408ul)) * 0x01010101ul) & 0xff000000ul) >> 24;

    flipped_discs = my_mask[RANK_G2][opp_mask[RANK_G2][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_G2][opp_mask[FILE_G2][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 6;
    flips_hi |= (flipped_bits & 0x10101010ul) << 2;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x10204080ul) | (opp_bits.high & 0x01020408ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x10204080ul) | (my_bits.high & 0x01020408ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[DIAG2_G2][opp_mask[DIAG2_G2][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x10204080ul;
    flips_hi |= flipped_bits & 0x01020408ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_B7   1
#define FILE_B7   6
#define DIAG2_B7  1

static int
TestFlips_bitboard5_b7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00020000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x007e0000ul) >> 17;
    my_discs = (my_bits.high & 0x00ff0000ul) >> 16;

    opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x10204080ul) | (opp_bits.high & 0x01020408ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x10204080ul) | (my_bits.high & 0x01020408ul)) * 0x01010101ul) & 0xff000000ul) >> 24;

    flipped_discs = my_mask[RANK_B7][opp_mask[RANK_B7][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_B7][opp_mask[FILE_B7][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00408102ul;
    flips_lo |= flipped_bits & 0x02020202ul;
    flips_hi |= (flipped_bits >> 4) & 0x02020202ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x10204080ul) | (opp_bits.high & 0x01020408ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x10204080ul) | (my_bits.high & 0x01020408ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[DIAG2_B7][opp_mask[DIAG2_B7][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x10204080ul;
    flips_hi |= flipped_bits & 0x01020408ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_G7   6
#define FILE_G7   6
#define DIAG1_G7  6

static int
TestFlips_bitboard5_g7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00400000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x007e0000ul) >> 17;
    my_discs = (my_bits.high & 0x00ff0000ul) >> 16;

    opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x08040201ul) | (opp_bits.high & 0x80402010ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x08040201ul) | (my_bits.high & 0x80402010ul)) * 0x01010101ul) & 0xff000000ul) >> 24;

    flipped_discs = my_mask[RANK_G7][opp_mask[RANK_G7][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_G7][opp_mask[FILE_G7][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 6;
    flips_hi |= (flipped_bits & 0x10101010ul) << 2;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x08040201ul) | (opp_bits.high & 0x80402010ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x08040201ul) | (my_bits.high & 0x80402010ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_G7][opp_mask[DIAG1_G7][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x08040201ul;
    flips_hi |= flipped_bits & 0x80402010ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_C1   2
#define FILE_C1   0
#define DIAG1_C1  0
#define DIAG2_C1  2

static int
TestFlips_bitboard5_c1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000004ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x0000007eul) >> 1;
    my_discs = my_bits.low & 0x000000fful;

    opp_discs2 = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x20100804ul) | (opp_bits.high & 0x00008040ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    my_discs3 = ((((my_bits.low & 0x20100804ul) | (my_bits.high & 0x00008040ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;

    flipped_discs = my_mask[RANK_C1][opp_mask[RANK_C1][opp_discs] & my_discs];
    flips_lo |= flipped_discs;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_C1][opp_mask[FILE_C1][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00810204ul;
    flips_lo |= flipped_bits & 0x04040404ul;
    flips_hi |= (flipped_bits >> 4) & 0x04040404ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x20100804ul) | (opp_bits.high & 0x00008040ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    //my_discs3 = ((((my_bits.low & 0x20100804ul) | (my_bits.high & 0x00008040ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;
    flipped_discs = my_mask[DIAG1_C1][opp_mask[DIAG1_C1][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x04040404ul;
    flips_lo |= flipped_bits & 0x20100804ul;
    flips_hi |= flipped_bits & 0x00008040ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = (opp_bits.low & 0x00000200ul) >> 9;
    my_discs = (((my_bits.low & 0x00010204ul) * 0x01010101ul) & 0x00070000ul) >> 16;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_C1][opp_mask[DIAG2_C1][opp_discs] & my_discs];
    flips_lo |= (flipped_discs & 0x00000002ul) << 8;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_F1   5
#define FILE_F1   0
#define DIAG1_F1  0
#define DIAG2_F1  5

static int
TestFlips_bitboard5_f1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000020ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x0000007eul) >> 1;
    my_discs = my_bits.low & 0x000000fful;

    opp_discs2 = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x04081020ul) | (opp_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x04081020ul) | (my_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;

    flipped_discs = my_mask[RANK_F1][opp_mask[RANK_F1][opp_discs] & my_discs];
    flips_lo |= flipped_discs;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_F1][opp_mask[FILE_F1][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 5;
    flips_hi |= (flipped_bits & 0x10101010ul) << 1;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x04081020ul) | (opp_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x04081020ul) | (my_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;
    flipped_discs = my_mask[DIAG2_F1][opp_mask[DIAG2_F1][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x04081020ul;
    flips_hi |= flipped_bits & 0x00000102ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    opp_discs = (opp_bits.low & 0x00004000ul) >> 14;
    my_discs = (((my_bits.low & 0x00804020ul) * 0x01010101ul) & 0x00e00000ul) >> 21;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG1_F1][opp_mask[DIAG1_F1][opp_discs] & my_discs];
    flips_lo |= (flipped_discs & 0x00000002ul) << 13;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_A3   0
#define FILE_A3   2
#define DIAG1_A3  0
#define DIAG2_A3  0

static int
TestFlips_bitboard5_a3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00010000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x007e0000ul) >> 17;
    my_discs = (my_bits.low & 0x00ff0000ul) >> 16;

    opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    opp_discs3 = ((((opp_bits.low & 0x02010000ul) | (opp_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x02010000ul) | (my_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;

    flipped_discs = my_mask[RANK_A3][opp_mask[RANK_A3][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    //my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    flipped_discs = my_mask[FILE_A3][opp_mask[FILE_A3][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= flipped_bits & 0x01010101ul;
    flips_hi |= (flipped_bits >> 4) & 0x01010101ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x02010000ul) | (opp_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x02010000ul) | (my_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_A3][opp_mask[DIAG1_A3][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x02010000ul;
    flips_hi |= flipped_bits & 0x20100804ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = (opp_bits.low & 0x00000200ul) >> 9;
    my_discs = (((my_bits.low & 0x00010204ul) * 0x01010101ul) & 0x00070000ul) >> 16;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_A3][opp_mask[DIAG2_A3][opp_discs] & my_discs];
    flips_lo |= (flipped_discs & 0x00000002ul) << 8;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_H3   7
#define FILE_H3   2
#define DIAG2_H3  5
#define DIAG1_H3  2

static int
TestFlips_bitboard5_h3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00800000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x007e0000ul) >> 17;
    my_discs = (my_bits.low & 0x00ff0000ul) >> 16;

    opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    opp_discs3 = ((((opp_bits.low & 0x40800000ul) | (opp_bits.high & 0x04081020ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    my_discs3 = ((((my_bits.low & 0x40800000ul) | (my_bits.high & 0x04081020ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;

    flipped_discs = my_mask[RANK_H3][opp_mask[RANK_H3][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    //my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[FILE_H3][opp_mask[FILE_H3][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 7;
    flips_hi |= (flipped_bits & 0x10101010ul) << 3;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x40800000ul) | (opp_bits.high & 0x04081020ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    //my_discs3 = ((((my_bits.low & 0x40800000ul) | (my_bits.high & 0x04081020ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;
    flipped_discs = my_mask[DIAG2_H3][opp_mask[DIAG2_H3][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x04040404ul;
    flips_lo |= flipped_bits & 0x40800000ul;
    flips_hi |= flipped_bits & 0x04081020ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    opp_discs = (opp_bits.low & 0x00004000ul) >> 14;
    my_discs = (((my_bits.low & 0x00804020ul) * 0x01010101ul) & 0x00e00000ul) >> 21;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG1_H3][opp_mask[DIAG1_H3][opp_discs] & my_discs];
    flips_lo |= (flipped_discs & 0x00000002ul) << 13;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_A6   0
#define FILE_A6   5
#define DIAG2_A6  0
#define DIAG1_A6  0

static int
TestFlips_bitboard5_a6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000100ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x00007e00ul) >> 9;
    my_discs = (my_bits.high & 0x0000ff00ul) >> 8;

    opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    opp_discs3 = ((((opp_bits.low & 0x04081020ul) | (opp_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x04081020ul) | (my_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;

    flipped_discs = my_mask[RANK_A6][opp_mask[RANK_A6][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    //my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    flipped_discs = my_mask[FILE_A6][opp_mask[FILE_A6][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= flipped_bits & 0x01010101ul;
    flips_hi |= (flipped_bits >> 4) & 0x01010101ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x04081020ul) | (opp_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x04081020ul) | (my_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;
    flipped_discs = my_mask[DIAG2_A6][opp_mask[DIAG2_A6][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x04081020ul;
    flips_hi |= flipped_bits & 0x00000102ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    opp_discs = (opp_bits.high & 0x00020000ul) >> 17;
    my_discs = (((my_bits.high & 0x04020100ul) * 0x01010101ul) & 0x07000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG1_A6][opp_mask[DIAG1_A6][opp_discs] & my_discs];
    flips_hi |= (flipped_discs & 0x00000002ul) << 16;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_H6   7
#define FILE_H6   5
#define DIAG1_H6  5
#define DIAG2_H6  2

static int
TestFlips_bitboard5_h6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00008000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x00007e00ul) >> 9;
    my_discs = (my_bits.high & 0x0000ff00ul) >> 8;

    opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    opp_discs3 = ((((opp_bits.low & 0x20100804ul) | (opp_bits.high & 0x00008040ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    my_discs3 = ((((my_bits.low & 0x20100804ul) | (my_bits.high & 0x00008040ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;

    flipped_discs = my_mask[RANK_H6][opp_mask[RANK_H6][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    //my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[FILE_H6][opp_mask[FILE_H6][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 7;
    flips_hi |= (flipped_bits & 0x10101010ul) << 3;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x20100804ul) | (opp_bits.high & 0x00008040ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    //my_discs3 = ((((my_bits.low & 0x20100804ul) | (my_bits.high & 0x00008040ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;
    flipped_discs = my_mask[DIAG1_H6][opp_mask[DIAG1_H6][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x04040404ul;
    flips_lo |= flipped_bits & 0x20100804ul;
    flips_hi |= flipped_bits & 0x00008040ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = (opp_bits.high & 0x00400000ul) >> 22;
    my_discs = (((my_bits.high & 0x20408000ul) * 0x01010101ul) & 0xe0000000ul) >> 29;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_H6][opp_mask[DIAG2_H6][opp_discs] & my_discs];
    flips_hi |= (flipped_discs & 0x00000002ul) << 21;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_C8   2
#define FILE_C8   7
#define DIAG2_C8  0
#define DIAG1_C8  2

static int
TestFlips_bitboard5_c8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x04000000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x7e000000ul) >> 25;
    my_discs = (my_bits.high & 0xff000000ul) >> 24;

    opp_discs2 = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x40800000ul) | (opp_bits.high & 0x04081020ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    my_discs3 = ((((my_bits.low & 0x40800000ul) | (my_bits.high & 0x04081020ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;

    flipped_discs = my_mask[RANK_C8][opp_mask[RANK_C8][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 24;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_C8][opp_mask[FILE_C8][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00810204ul;
    flips_lo |= flipped_bits & 0x04040404ul;
    flips_hi |= (flipped_bits >> 4) & 0x04040404ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x40800000ul) | (opp_bits.high & 0x04081020ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    //my_discs3 = ((((my_bits.low & 0x40800000ul) | (my_bits.high & 0x04081020ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;
    flipped_discs = my_mask[DIAG2_C8][opp_mask[DIAG2_C8][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x04040404ul;
    flips_lo |= flipped_bits & 0x40800000ul;
    flips_hi |= flipped_bits & 0x04081020ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    opp_discs = (opp_bits.high & 0x00020000ul) >> 17;
    my_discs = (((my_bits.high & 0x04020100ul) * 0x01010101ul) & 0x07000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG1_C8][opp_mask[DIAG1_C8][opp_discs] & my_discs];
    flips_hi |= (flipped_discs & 0x00000002ul) << 16;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_F8   5
#define FILE_F8   7
#define DIAG1_F8  5
#define DIAG2_F8  0

static int
TestFlips_bitboard5_f8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x20000000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x7e000000ul) >> 25;
    my_discs = (my_bits.high & 0xff000000ul) >> 24;

    opp_discs2 = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x02010000ul) | (opp_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x02010000ul) | (my_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;

    flipped_discs = my_mask[RANK_F8][opp_mask[RANK_F8][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 24;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_F8][opp_mask[FILE_F8][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 5;
    flips_hi |= (flipped_bits & 0x10101010ul) << 1;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x02010000ul) | (opp_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x02010000ul) | (my_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_F8][opp_mask[DIAG1_F8][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x02010000ul;
    flips_hi |= flipped_bits & 0x20100804ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = (opp_bits.high & 0x00400000ul) >> 22;
    my_discs = (((my_bits.high & 0x20408000ul) * 0x01010101ul) & 0xe0000000ul) >> 29;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_F8][opp_mask[DIAG2_F8][opp_discs] & my_discs];
    flips_hi |= (flipped_discs & 0x00000002ul) << 21;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_D1   3
#define FILE_D1   0
#define DIAG1_D1  0
#define DIAG2_D1  3

static int
TestFlips_bitboard5_d1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3, my_discs4, opp_discs4;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000008ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x0000007eul) >> 1;
    my_discs = my_bits.low & 0x000000fful;

    opp_discs2 = ((((opp_bits.low & 0x08080808ul) | ((opp_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    my_discs2 = ((((my_bits.low & 0x08080808ul) | ((my_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0xff000000ul) >> 24;
    opp_discs3 = ((((opp_bits.low & 0x40201008ul) | (opp_bits.high & 0x00000080ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    my_discs3 = ((((my_bits.low & 0x40201008ul) | (my_bits.high & 0x00000080ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
    //opp_discs4 = (((opp_bits.low & 0x01020408ul) * 0x01010101ul) & 0x06000000ul) >> 25;
    //my_discs4 = (((my_bits.low & 0x01020408ul) * 0x01010101ul) & 0x0f000000ul) >> 24;

    flipped_discs = my_mask[RANK_D1][opp_mask[RANK_D1][opp_discs] & my_discs];
    flips_lo |= flipped_discs;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x08080808ul) | ((opp_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    //my_discs2 = ((((my_bits.low & 0x08080808ul) | ((my_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[FILE_D1][opp_mask[FILE_D1][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01020408ul;
    flips_lo |= flipped_bits & 0x08080808ul;
    flips_hi |= (flipped_bits >> 4) & 0x08080808ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x40201008ul) | (opp_bits.high & 0x00000080ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    //my_discs3 = ((((my_bits.low & 0x40201008ul) | (my_bits.high & 0x00000080ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
    flipped_discs = my_mask[DIAG1_D1][opp_mask[DIAG1_D1][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x08080808ul;
    flips_lo |= flipped_bits & 0x40201008ul;
    flips_hi |= flipped_bits & 0x00000080ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs4 = (((opp_bits.low & 0x01020408ul) * 0x01010101ul) & 0x06000000ul) >> 25;
    my_discs4 = (((my_bits.low & 0x01020408ul) * 0x01010101ul) & 0x0f000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_D1][opp_mask[DIAG2_D1][opp_discs4] & my_discs4];
    flipped_bits = (flipped_discs & 0x00000006ul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x01020408ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_E1   4
#define FILE_E1   0
#define DIAG1_E1  0
#define DIAG2_E1  4

static int
TestFlips_bitboard5_e1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3, my_discs4, opp_discs4;
    unsigned long flipped_discs2, flipped_discs3, flipped_discs4;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000010ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x0000007eul) >> 1;
    my_discs = my_bits.low & 0x000000fful;

    opp_discs2 = (((((opp_bits.low & 0x10101010ul) >> 4) | (opp_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    my_discs2 = (((((my_bits.low & 0x10101010ul) >> 4) | (my_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    opp_discs3 = (((opp_bits.low & 0x80402010ul) * 0x01010101ul) & 0x60000000ul) >> 29;
    my_discs3 = (((my_bits.low & 0x80402010ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
    //opp_discs4 = ((((opp_bits.low & 0x02040810ul) | (opp_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    //my_discs4 = ((((my_bits.low & 0x02040810ul) | (my_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;

    flipped_discs = my_mask[RANK_E1][opp_mask[RANK_E1][opp_discs] & my_discs];
    flips_lo |= flipped_discs;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x10101010ul) >> 4) | (opp_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    //my_discs2 = (((((my_bits.low & 0x10101010ul) >> 4) | (my_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    flipped_discs2 = my_mask[FILE_E1][opp_mask[FILE_E1][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs2 & 0x0000007eul) * 0x02040810ul;
    flips_lo |= flipped_bits & 0x10101010ul;
    flips_hi |= (flipped_bits >> 4) & 0x10101010ul;
    flip_count += discs_count[flipped_discs2 >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = (((opp_bits.low & 0x80402010ul) * 0x01010101ul) & 0x60000000ul) >> 29;
    //my_discs3 = (((my_bits.low & 0x80402010ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
    flipped_discs3 = my_mask[DIAG1_E1][opp_mask[DIAG1_E1][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs3 & 0x00000006ul) * 0x10101010ul;
    flips_lo |= flipped_bits & 0x80402010ul;
    //flip_count += discs_count[flipped_discs3 >> 1];

    /* Diagonal-2 : Down left */
    opp_discs4 = ((((opp_bits.low & 0x02040810ul) | (opp_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    my_discs4 = ((((my_bits.low & 0x02040810ul) | (my_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    flip_count += discs_count[flipped_discs3 >> 1];
    flipped_discs4 = my_mask[DIAG2_E1][opp_mask[DIAG2_E1][opp_discs4] & my_discs4];
    flipped_bits = (flipped_discs4 & 0x0000000eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x02040810ul;
    flips_hi |= flipped_bits & 0x00000001ul;
    flip_count += discs_count[flipped_discs4 >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_A4   0
#define FILE_A4   3
#define DIAG1_A4  0
#define DIAG2_A4  0

static int
TestFlips_bitboard5_a4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3, my_discs4, opp_discs4;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x01000000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x7e000000ul) >> 25;
    my_discs = (my_bits.low & 0xff000000ul) >> 24;

    opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    opp_discs3 = ((((opp_bits.low & 0x01000000ul) | (opp_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x01000000ul) | (my_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    //opp_discs4 = (((opp_bits.low & 0x01020408ul) * 0x01010101ul) & 0x06000000ul) >> 25;
    //my_discs4 = (((my_bits.low & 0x01020408ul) * 0x01010101ul) & 0x0f000000ul) >> 24;

    flipped_discs = my_mask[RANK_A4][opp_mask[RANK_A4][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 24;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    //my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    flipped_discs = my_mask[FILE_A4][opp_mask[FILE_A4][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= flipped_bits & 0x01010101ul;
    flips_hi |= (flipped_bits >> 4) & 0x01010101ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x01000000ul) | (opp_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x01000000ul) | (my_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_A4][opp_mask[DIAG1_A4][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x01000000ul;
    flips_hi |= flipped_bits & 0x10080402ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs4 = (((opp_bits.low & 0x01020408ul) * 0x01010101ul) & 0x06000000ul) >> 25;
    my_discs4 = (((my_bits.low & 0x01020408ul) * 0x01010101ul) & 0x0f000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_A4][opp_mask[DIAG2_A4][opp_discs4] & my_discs4];
    flipped_bits = (flipped_discs & 0x00000006ul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x01020408ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_H4   7
#define FILE_H4   3
#define DIAG1_H4  3
#define DIAG2_H4  4

static int
TestFlips_bitboard5_h4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3, my_discs4, opp_discs4;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x80000000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x7e000000ul) >> 25;
    my_discs = (my_bits.low & 0xff000000ul) >> 24;

    opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    opp_discs3 = (((opp_bits.low & 0x80402010ul) * 0x01010101ul) & 0x60000000ul) >> 29;
    my_discs3 = (((my_bits.low & 0x80402010ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
    //opp_discs4 = ((((opp_bits.low & 0x80000000ul) | (opp_bits.high & 0x08102040ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    //my_discs4 = ((((my_bits.low & 0x80000000ul) | (my_bits.high & 0x08102040ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;

    flipped_discs = my_mask[RANK_H4][opp_mask[RANK_H4][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 24;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    //my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[FILE_H4][opp_mask[FILE_H4][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 7;
    flips_hi |= (flipped_bits & 0x10101010ul) << 3;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = (((opp_bits.low & 0x80402010ul) * 0x01010101ul) & 0x60000000ul) >> 29;
    //my_discs3 = (((my_bits.low & 0x80402010ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
    flipped_discs = my_mask[DIAG1_H4][opp_mask[DIAG1_H4][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x00000006ul) * 0x10101010ul;
    flips_lo |= flipped_bits & 0x80402010ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs4 = ((((opp_bits.low & 0x80000000ul) | (opp_bits.high & 0x08102040ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    my_discs4 = ((((my_bits.low & 0x80000000ul) | (my_bits.high & 0x08102040ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_H4][opp_mask[DIAG2_H4][opp_discs4] & my_discs4];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x08080808ul;
    flips_lo |= flipped_bits & 0x80000000ul;
    flips_hi |= flipped_bits & 0x08102040ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_A5   0
#define FILE_A5   4
#define DIAG2_A5  0
#define DIAG1_A5  0

static int
TestFlips_bitboard5_a5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3, my_discs4, opp_discs4;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000001ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x0000007eul) >> 1;
    my_discs = my_bits.high & 0x000000fful;

    opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    opp_discs3 = ((((opp_bits.low & 0x02040810ul) | (opp_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x02040810ul) | (my_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    //opp_discs4 = (((opp_bits.high & 0x08040201ul) * 0x01010101ul) & 0x06000000ul) >> 25;
    //my_discs4 = (((my_bits.high & 0x08040201ul) * 0x01010101ul) & 0x0f000000ul) >> 24;

    flipped_discs = my_mask[RANK_A5][opp_mask[RANK_A5][opp_discs] & my_discs];
    flips_hi |= flipped_discs;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    //my_discs2 = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    flipped_discs = my_mask[FILE_A5][opp_mask[FILE_A5][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= flipped_bits & 0x01010101ul;
    flips_hi |= (flipped_bits >> 4) & 0x01010101ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x02040810ul) | (opp_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x02040810ul) | (my_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    flipped_discs = my_mask[DIAG2_A5][opp_mask[DIAG2_A5][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x02040810ul;
    flips_hi |= flipped_bits & 0x00000001ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    opp_discs4 = (((opp_bits.high & 0x08040201ul) * 0x01010101ul) & 0x06000000ul) >> 25;
    my_discs4 = (((my_bits.high & 0x08040201ul) * 0x01010101ul) & 0x0f000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG1_A5][opp_mask[DIAG1_A5][opp_discs4] & my_discs4];
    flipped_bits = (flipped_discs & 0x00000006ul) * 0x01010101ul;
    flips_hi |= flipped_bits & 0x08040201ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_H5   7
#define FILE_H5   4
#define DIAG1_H5  4
#define DIAG2_H5  3

static int
TestFlips_bitboard5_h5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3, my_discs4, opp_discs4;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000080ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x0000007eul) >> 1;
    my_discs = my_bits.high & 0x000000fful;

    opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    opp_discs3 = ((((opp_bits.low & 0x40201008ul) | (opp_bits.high & 0x00000080ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    my_discs3 = ((((my_bits.low & 0x40201008ul) | (my_bits.high & 0x00000080ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
    //opp_discs4 = (((opp_bits.high & 0x10204080ul) * 0x01010101ul) & 0x60000000ul) >> 29;
    //my_discs4 = (((my_bits.high & 0x10204080ul) * 0x01010101ul) & 0xf0000000ul) >> 28;

    flipped_discs = my_mask[RANK_H5][opp_mask[RANK_H5][opp_discs] & my_discs];
    flips_hi |= flipped_discs;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    //my_discs2 = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[FILE_H5][opp_mask[FILE_H5][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 7;
    flips_hi |= (flipped_bits & 0x10101010ul) << 3;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x40201008ul) | (opp_bits.high & 0x00000080ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    //my_discs3 = ((((my_bits.low & 0x40201008ul) | (my_bits.high & 0x00000080ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
    flipped_discs = my_mask[DIAG1_H5][opp_mask[DIAG1_H5][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x08080808ul;
    flips_lo |= flipped_bits & 0x40201008ul;
    flips_hi |= flipped_bits & 0x00000080ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs4 = (((opp_bits.high & 0x10204080ul) * 0x01010101ul) & 0x60000000ul) >> 29;
    my_discs4 = (((my_bits.high & 0x10204080ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_H5][opp_mask[DIAG2_H5][opp_discs4] & my_discs4];
    flipped_bits = (flipped_discs & 0x00000006ul) * 0x10101010ul;
    flips_hi |= flipped_bits & 0x10204080ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_D8   3
#define FILE_D8   7
#define DIAG1_D8  3
#define DIAG2_D8  0

static int
TestFlips_bitboard5_d8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3, my_discs4, opp_discs4;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x08000000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x7e000000ul) >> 25;
    my_discs = (my_bits.high & 0xff000000ul) >> 24;

    opp_discs2 = ((((opp_bits.low & 0x08080808ul) | ((opp_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    my_discs2 = ((((my_bits.low & 0x08080808ul) | ((my_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0xff000000ul) >> 24;
    opp_discs3 = (((opp_bits.high & 0x08040201ul) * 0x01010101ul) & 0x06000000ul) >> 25;
    my_discs3 = (((my_bits.high & 0x08040201ul) * 0x01010101ul) & 0x0f000000ul) >> 24;
    //opp_discs4 = ((((opp_bits.low & 0x80000000ul) | (opp_bits.high & 0x08102040ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    //my_discs4 = ((((my_bits.low & 0x80000000ul) | (my_bits.high & 0x08102040ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;

    flipped_discs = my_mask[RANK_D8][opp_mask[RANK_D8][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 24;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x08080808ul) | ((opp_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    //my_discs2 = ((((my_bits.low & 0x08080808ul) | ((my_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[FILE_D8][opp_mask[FILE_D8][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01020408ul;
    flips_lo |= flipped_bits & 0x08080808ul;
    flips_hi |= (flipped_bits >> 4) & 0x08080808ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = (((opp_bits.high & 0x08040201ul) * 0x01010101ul) & 0x06000000ul) >> 25;
    //my_discs3 = (((my_bits.high & 0x08040201ul) * 0x01010101ul) & 0x0f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_D8][opp_mask[DIAG1_D8][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x00000006ul) * 0x01010101ul;
    flips_hi |= flipped_bits & 0x08040201ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs4 = ((((opp_bits.low & 0x80000000ul) | (opp_bits.high & 0x08102040ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    my_discs4 = ((((my_bits.low & 0x80000000ul) | (my_bits.high & 0x08102040ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_D8][opp_mask[DIAG2_D8][opp_discs4] & my_discs4];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x08080808ul;
    flips_lo |= flipped_bits & 0x80000000ul;
    flips_hi |= flipped_bits & 0x08102040ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_E8   4
#define FILE_E8   7
#define DIAG1_E8  4
#define DIAG2_E8  0

static int
TestFlips_bitboard5_e8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3, my_discs4, opp_discs4;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x10000000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x7e000000ul) >> 25;
    my_discs = (my_bits.high & 0xff000000ul) >> 24;

    opp_discs2 = (((((opp_bits.low & 0x10101010ul) >> 4) | (opp_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    my_discs2 = (((((my_bits.low & 0x10101010ul) >> 4) | (my_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    opp_discs3 = ((((opp_bits.low & 0x01000000ul) | (opp_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x01000000ul) | (my_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    //opp_discs4 = (((opp_bits.high & 0x10204080ul) * 0x01010101ul) & 0x60000000ul) >> 29;
    //my_discs4 = (((my_bits.high & 0x10204080ul) * 0x01010101ul) & 0xf0000000ul) >> 28;

    flipped_discs = my_mask[RANK_E8][opp_mask[RANK_E8][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 24;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x10101010ul) >> 4) | (opp_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    //my_discs2 = (((((my_bits.low & 0x10101010ul) >> 4) | (my_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    flipped_discs = my_mask[FILE_E8][opp_mask[FILE_E8][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x02040810ul;
    flips_lo |= flipped_bits & 0x10101010ul;
    flips_hi |= (flipped_bits >> 4) & 0x10101010ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x01000000ul) | (opp_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x01000000ul) | (my_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_E8][opp_mask[DIAG1_E8][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x01000000ul;
    flips_hi |= flipped_bits & 0x10080402ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs4 = (((opp_bits.high & 0x10204080ul) * 0x01010101ul) & 0x60000000ul) >> 29;
    my_discs4 = (((my_bits.high & 0x10204080ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_E8][opp_mask[DIAG2_E8][opp_discs4] & my_discs4];
    flipped_bits = (flipped_discs & 0x00000006ul) * 0x10101010ul;
    flips_hi |= flipped_bits & 0x10204080ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_C2   2
#define FILE_C2   1
#define DIAG1_C2  1
#define DIAG2_C2  2

static int
TestFlips_bitboard5_c2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000400ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x00007e00ul) >> 9;
    my_discs = (my_bits.low & 0x0000ff00ul) >> 8;

    opp_discs2 = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x10080402ul) | (opp_bits.high & 0x00804020ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    my_discs3 = ((((my_bits.low & 0x10080402ul) | (my_bits.high & 0x00804020ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;

    flipped_discs = my_mask[RANK_C2][opp_mask[RANK_C2][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_C2][opp_mask[FILE_C2][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00810204ul;
    flips_lo |= flipped_bits & 0x04040404ul;
    flips_hi |= (flipped_bits >> 4) & 0x04040404ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x10080402ul) | (opp_bits.high & 0x00804020ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    //my_discs3 = ((((my_bits.low & 0x10080402ul) | (my_bits.high & 0x00804020ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;
    flipped_discs = my_mask[DIAG1_C2][opp_mask[DIAG1_C2][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x02020202ul;
    flips_lo |= flipped_bits & 0x10080402ul;
    flips_hi |= flipped_bits & 0x00804020ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = (opp_bits.low & 0x00020000ul) >> 17;
    my_discs = (((my_bits.low & 0x01020408ul) * 0x01010101ul) & 0x0f000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_C2][opp_mask[DIAG2_C2][opp_discs] & my_discs];
    flips_lo |= (flipped_discs & 0x00000002ul) << 16;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_F2   5
#define FILE_F2   1
#define DIAG2_F2  5
#define DIAG1_F2  1

static int
TestFlips_bitboard5_f2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00002000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x00007e00ul) >> 9;
    my_discs = (my_bits.low & 0x0000ff00ul) >> 8;

    opp_discs2 = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x08102040ul) | (opp_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x08102040ul) | (my_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;

    flipped_discs = my_mask[RANK_F2][opp_mask[RANK_F2][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_F2][opp_mask[FILE_F2][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 5;
    flips_hi |= (flipped_bits & 0x10101010ul) << 1;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x08102040ul) | (opp_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x08102040ul) | (my_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;
    flipped_discs = my_mask[DIAG2_F2][opp_mask[DIAG2_F2][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x08102040ul;
    flips_hi |= flipped_bits & 0x00010204ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    opp_discs = (opp_bits.low & 0x00400000ul) >> 21;
    my_discs = (((my_bits.low & 0x80402010ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG1_F2][opp_mask[DIAG1_F2][opp_discs] & my_discs];
    flips_lo |= (flipped_discs & 0x00000004ul) << 20;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_B3   1
#define FILE_B3   2
#define DIAG1_B3  1
#define DIAG2_B3  1

static int
TestFlips_bitboard5_b3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00020000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x007e0000ul) >> 17;
    my_discs = (my_bits.low & 0x00ff0000ul) >> 16;

    opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x04020100ul) | (opp_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x04020100ul) | (my_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;

    flipped_discs = my_mask[RANK_B3][opp_mask[RANK_B3][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_B3][opp_mask[FILE_B3][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00408102ul;
    flips_lo |= flipped_bits & 0x02020202ul;
    flips_hi |= (flipped_bits >> 4) & 0x02020202ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x04020100ul) | (opp_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x04020100ul) | (my_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_B3][opp_mask[DIAG1_B3][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x04020100ul;
    flips_hi |= flipped_bits & 0x40201008ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = (opp_bits.low & 0x00000400ul) >> 9;
    my_discs = (((my_bits.low & 0x01020408ul) * 0x01010101ul) & 0x0f000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_B3][opp_mask[DIAG2_B3][opp_discs] & my_discs];
    flips_lo |= (flipped_discs & 0x00000004ul) << 8;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_G3   6
#define FILE_G3   2
#define DIAG2_G3  5
#define DIAG1_G3  2

static int
TestFlips_bitboard5_g3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00400000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x007e0000ul) >> 17;
    my_discs = (my_bits.low & 0x00ff0000ul) >> 16;

    opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x20408000ul) | (opp_bits.high & 0x02040810ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    my_discs3 = ((((my_bits.low & 0x20408000ul) | (my_bits.high & 0x02040810ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;

    flipped_discs = my_mask[RANK_G3][opp_mask[RANK_G3][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_G3][opp_mask[FILE_G3][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 6;
    flips_hi |= (flipped_bits & 0x10101010ul) << 2;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x20408000ul) | (opp_bits.high & 0x02040810ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    //my_discs3 = ((((my_bits.low & 0x20408000ul) | (my_bits.high & 0x02040810ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;
    flipped_discs = my_mask[DIAG2_G3][opp_mask[DIAG2_G3][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x02020202ul;
    flips_lo |= flipped_bits & 0x20408000ul;
    flips_hi |= flipped_bits & 0x02040810ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    opp_discs = (opp_bits.low & 0x00002000ul) >> 13;
    my_discs = (((my_bits.low & 0x80402010ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG1_G3][opp_mask[DIAG1_G3][opp_discs] & my_discs];
    flips_lo |= (flipped_discs & 0x00000002ul) << 12;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_B6   1
#define FILE_B6   5
#define DIAG2_B6  1
#define DIAG1_B6  1

static int
TestFlips_bitboard5_b6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000200ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x00007e00ul) >> 9;
    my_discs = (my_bits.high & 0x0000ff00ul) >> 8;

    opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x08102040ul) | (opp_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x08102040ul) | (my_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;

    flipped_discs = my_mask[RANK_B6][opp_mask[RANK_B6][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_B6][opp_mask[FILE_B6][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00408102ul;
    flips_lo |= flipped_bits & 0x02020202ul;
    flips_hi |= (flipped_bits >> 4) & 0x02020202ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x08102040ul) | (opp_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x08102040ul) | (my_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;
    flipped_discs = my_mask[DIAG2_B6][opp_mask[DIAG2_B6][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x08102040ul;
    flips_hi |= flipped_bits & 0x00010204ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    opp_discs = (opp_bits.high & 0x00040000ul) >> 17;
    my_discs = (((my_bits.high & 0x08040201ul) * 0x01010101ul) & 0x0f000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG1_B6][opp_mask[DIAG1_B6][opp_discs] & my_discs];
    flips_hi |= (flipped_discs & 0x00000004ul) << 16;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_G6   6
#define FILE_G6   5
#define DIAG1_G6  5
#define DIAG2_G6  2

static int
TestFlips_bitboard5_g6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00004000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x00007e00ul) >> 9;
    my_discs = (my_bits.high & 0x0000ff00ul) >> 8;

    opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x10080402ul) | (opp_bits.high & 0x00804020ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    my_discs3 = ((((my_bits.low & 0x10080402ul) | (my_bits.high & 0x00804020ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;

    flipped_discs = my_mask[RANK_G6][opp_mask[RANK_G6][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_G6][opp_mask[FILE_G6][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 6;
    flips_hi |= (flipped_bits & 0x10101010ul) << 2;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x10080402ul) | (opp_bits.high & 0x00804020ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    //my_discs3 = ((((my_bits.low & 0x10080402ul) | (my_bits.high & 0x00804020ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;
    flipped_discs = my_mask[DIAG1_G6][opp_mask[DIAG1_G6][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x02020202ul;
    flips_lo |= flipped_bits & 0x10080402ul;
    flips_hi |= flipped_bits & 0x00804020ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = (opp_bits.high & 0x00200000ul) >> 21;
    my_discs = (((my_bits.high & 0x10204080ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_G6][opp_mask[DIAG2_G6][opp_discs] & my_discs];
    flips_hi |= (flipped_discs & 0x00000002ul) << 20;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_C7   2
#define FILE_C7   6
#define DIAG2_C7  1
#define DIAG1_C7  2

static int
TestFlips_bitboard5_c7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00040000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x007e0000ul) >> 17;
    my_discs = (my_bits.high & 0x00ff0000ul) >> 16;

    opp_discs2 = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x20408000ul) | (opp_bits.high & 0x02040810ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    my_discs3 = ((((my_bits.low & 0x20408000ul) | (my_bits.high & 0x02040810ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;

    flipped_discs = my_mask[RANK_C7][opp_mask[RANK_C7][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_C7][opp_mask[FILE_C7][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00810204ul;
    flips_lo |= flipped_bits & 0x04040404ul;
    flips_hi |= (flipped_bits >> 4) & 0x04040404ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    //opp_discs3 = ((((opp_bits.low & 0x20408000ul) | (opp_bits.high & 0x02040810ul)) * 0x01010101ul) & 0x7c000000ul) >> 26;
    //my_discs3 = ((((my_bits.low & 0x20408000ul) | (my_bits.high & 0x02040810ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;
    flipped_discs = my_mask[DIAG2_C7][opp_mask[DIAG2_C7][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x02020202ul;
    flips_lo |= flipped_bits & 0x20408000ul;
    flips_hi |= flipped_bits & 0x02040810ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    opp_discs = (opp_bits.high & 0x00000200ul) >> 9;
    my_discs = (((my_bits.high & 0x08040201ul) * 0x01010101ul) & 0x0f000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG1_C7][opp_mask[DIAG1_C7][opp_discs] & my_discs];
    flips_hi |= (flipped_discs & 0x00000002ul) << 8;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_F7   5
#define FILE_F7   6
#define DIAG1_F7  5
#define DIAG2_F7  1

static int
TestFlips_bitboard5_f7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00200000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x007e0000ul) >> 17;
    my_discs = (my_bits.high & 0x00ff0000ul) >> 16;

    opp_discs2 = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x04020100ul) | (opp_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x04020100ul) | (my_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;

    flipped_discs = my_mask[RANK_F7][opp_mask[RANK_F7][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_F7][opp_mask[FILE_F7][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 5;
    flips_hi |= (flipped_bits & 0x10101010ul) << 1;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x04020100ul) | (opp_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x3e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x04020100ul) | (my_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_F7][opp_mask[DIAG1_F7][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000003eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x04020100ul;
    flips_hi |= flipped_bits & 0x40201008ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = (opp_bits.high & 0x00004000ul) >> 13;
    my_discs = (((my_bits.high & 0x10204080ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_F7][opp_mask[DIAG2_F7][opp_discs] & my_discs];
    flips_hi |= (flipped_discs & 0x00000004ul) << 12;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_D2   3
#define FILE_D2   1
#define DIAG1_D2  1
#define DIAG2_D2  3

static int
TestFlips_bitboard5_d2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00000800ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x00007e00ul) >> 9;
    my_discs = (my_bits.low & 0x0000ff00ul) >> 8;

    opp_discs2 = ((((opp_bits.low & 0x08080808ul) | ((opp_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    my_discs2 = ((((my_bits.low & 0x08080808ul) | ((my_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0xff000000ul) >> 24;
    opp_discs3 = ((((opp_bits.low & 0x20100804ul) | (opp_bits.high & 0x00008040ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    my_discs3 = ((((my_bits.low & 0x20100804ul) | (my_bits.high & 0x00008040ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;

    flipped_discs = my_mask[RANK_D2][opp_mask[RANK_D2][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x08080808ul) | ((opp_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    //my_discs2 = ((((my_bits.low & 0x08080808ul) | ((my_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[FILE_D2][opp_mask[FILE_D2][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01020408ul;
    flips_lo |= flipped_bits & 0x08080808ul;
    flips_hi |= (flipped_bits >> 4) & 0x08080808ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x20100804ul) | (opp_bits.high & 0x00008040ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    //my_discs3 = ((((my_bits.low & 0x20100804ul) | (my_bits.high & 0x00008040ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;
    flipped_discs = my_mask[DIAG1_D2][opp_mask[DIAG1_D2][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x04040404ul;
    flips_lo |= flipped_bits & 0x20100804ul;
    flips_hi |= flipped_bits & 0x00008040ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = ((((opp_bits.low & 0x02040810ul) | (opp_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    my_discs = ((((my_bits.low & 0x02040810ul) | (my_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_D2][opp_mask[DIAG2_D2][opp_discs] & my_discs];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x02040810ul;
    flips_hi |= flipped_bits & 0x00000001ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_E2   4
#define FILE_E2   1
#define DIAG1_E2  1
#define DIAG2_E2  4

static int
TestFlips_bitboard5_e2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00001000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x00007e00ul) >> 9;
    my_discs = (my_bits.low & 0x0000ff00ul) >> 8;

    opp_discs2 = (((((opp_bits.low & 0x10101010ul) >> 4) | (opp_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    my_discs2 = (((((my_bits.low & 0x10101010ul) >> 4) | (my_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    opp_discs3 = ((((opp_bits.low & 0x40201008ul) | (opp_bits.high & 0x00000080ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    my_discs3 = ((((my_bits.low & 0x40201008ul) | (my_bits.high & 0x00000080ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;

    flipped_discs = my_mask[RANK_E2][opp_mask[RANK_E2][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x10101010ul) >> 4) | (opp_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    //my_discs2 = (((((my_bits.low & 0x10101010ul) >> 4) | (my_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    flipped_discs = my_mask[FILE_E2][opp_mask[FILE_E2][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x02040810ul;
    flips_lo |= flipped_bits & 0x10101010ul;
    flips_hi |= (flipped_bits >> 4) & 0x10101010ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x40201008ul) | (opp_bits.high & 0x00000080ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    //my_discs3 = ((((my_bits.low & 0x40201008ul) | (my_bits.high & 0x00000080ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
    flipped_discs = my_mask[DIAG1_E2][opp_mask[DIAG1_E2][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x08080808ul;
    flips_lo |= flipped_bits & 0x40201008ul;
    flips_hi |= flipped_bits & 0x00000080ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = ((((opp_bits.low & 0x04081020ul) | (opp_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    my_discs = ((((my_bits.low & 0x04081020ul) | (my_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_E2][opp_mask[DIAG2_E2][opp_discs] & my_discs];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x04081020ul;
    flips_hi |= flipped_bits & 0x00000102ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_B4   1
#define FILE_B4   3
#define DIAG1_B4  1
#define DIAG2_B4  1

static int
TestFlips_bitboard5_b4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x02000000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x7e000000ul) >> 25;
    my_discs = (my_bits.low & 0xff000000ul) >> 24;

    opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x02010000ul) | (opp_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x02010000ul) | (my_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;

    flipped_discs = my_mask[RANK_B4][opp_mask[RANK_B4][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 24;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_B4][opp_mask[FILE_B4][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00408102ul;
    flips_lo |= flipped_bits & 0x02020202ul;
    flips_hi |= (flipped_bits >> 4) & 0x02020202ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x02010000ul) | (opp_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x02010000ul) | (my_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_B4][opp_mask[DIAG1_B4][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x02010000ul;
    flips_hi |= flipped_bits & 0x20100804ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = ((((opp_bits.low & 0x02040810ul) | (opp_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    my_discs = ((((my_bits.low & 0x02040810ul) | (my_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_B4][opp_mask[DIAG2_B4][opp_discs] & my_discs];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x02040810ul;
    flips_hi |= flipped_bits & 0x00000001ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_G4   6
#define FILE_G4   3
#define DIAG1_G4  3
#define DIAG2_G4  4

static int
TestFlips_bitboard5_g4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x40000000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x7e000000ul) >> 25;
    my_discs = (my_bits.low & 0xff000000ul) >> 24;

    opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x40201008ul) | (opp_bits.high & 0x00000080ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    my_discs3 = ((((my_bits.low & 0x40201008ul) | (my_bits.high & 0x00000080ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;

    flipped_discs = my_mask[RANK_G4][opp_mask[RANK_G4][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 24;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_G4][opp_mask[FILE_G4][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 6;
    flips_hi |= (flipped_bits & 0x10101010ul) << 2;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x40201008ul) | (opp_bits.high & 0x00000080ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    //my_discs3 = ((((my_bits.low & 0x40201008ul) | (my_bits.high & 0x00000080ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
    flipped_discs = my_mask[DIAG1_G4][opp_mask[DIAG1_G4][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x08080808ul;
    flips_lo |= flipped_bits & 0x40201008ul;
    flips_hi |= flipped_bits & 0x00000080ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = ((((opp_bits.low & 0x40800000ul) | (opp_bits.high & 0x04081020ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    my_discs = ((((my_bits.low & 0x40800000ul) | (my_bits.high & 0x04081020ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_G4][opp_mask[DIAG2_G4][opp_discs] & my_discs];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x04040404ul;
    flips_lo |= flipped_bits & 0x40800000ul;
    flips_hi |= flipped_bits & 0x04081020ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_B5   1
#define FILE_B5   4
#define DIAG1_B5  1
#define DIAG2_B5  1

static int
TestFlips_bitboard5_b5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000002ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x0000007eul) >> 1;
    my_discs = my_bits.high & 0x000000fful;

    opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x01000000ul) | (opp_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x01000000ul) | (my_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;

    flipped_discs = my_mask[RANK_B5][opp_mask[RANK_B5][opp_discs] & my_discs];
    flips_hi |= flipped_discs;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_B5][opp_mask[FILE_B5][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00408102ul;
    flips_lo |= flipped_bits & 0x02020202ul;
    flips_hi |= (flipped_bits >> 4) & 0x02020202ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x01000000ul) | (opp_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x01000000ul) | (my_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_B5][opp_mask[DIAG1_B5][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x01000000ul;
    flips_hi |= flipped_bits & 0x10080402ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = ((((opp_bits.low & 0x04081020ul) | (opp_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    my_discs = ((((my_bits.low & 0x04081020ul) | (my_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_B5][opp_mask[DIAG2_B5][opp_discs] & my_discs];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x04081020ul;
    flips_hi |= flipped_bits & 0x00000102ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_G5   6
#define FILE_G5   4
#define DIAG1_G5  4
#define DIAG2_G5  3

static int
TestFlips_bitboard5_g5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000040ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x0000007eul) >> 1;
    my_discs = my_bits.high & 0x000000fful;

    opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x20100804ul) | (opp_bits.high & 0x00008040ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    my_discs3 = ((((my_bits.low & 0x20100804ul) | (my_bits.high & 0x00008040ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;

    flipped_discs = my_mask[RANK_G5][opp_mask[RANK_G5][opp_discs] & my_discs];
    flips_hi |= flipped_discs;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_G5][opp_mask[FILE_G5][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 6;
    flips_hi |= (flipped_bits & 0x10101010ul) << 2;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x20100804ul) | (opp_bits.high & 0x00008040ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    //my_discs3 = ((((my_bits.low & 0x20100804ul) | (my_bits.high & 0x00008040ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;
    flipped_discs = my_mask[DIAG1_G5][opp_mask[DIAG1_G5][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x04040404ul;
    flips_lo |= flipped_bits & 0x20100804ul;
    flips_hi |= flipped_bits & 0x00008040ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = ((((opp_bits.low & 0x80000000ul) | (opp_bits.high & 0x08102040ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    my_discs = ((((my_bits.low & 0x80000000ul) | (my_bits.high & 0x08102040ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_G5][opp_mask[DIAG2_G5][opp_discs] & my_discs];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x08080808ul;
    flips_lo |= flipped_bits & 0x80000000ul;
    flips_hi |= flipped_bits & 0x08102040ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_D7   3
#define FILE_D7   6
#define DIAG1_D7  3
#define DIAG2_D7  1

static int
TestFlips_bitboard5_d7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00080000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x007e0000ul) >> 17;
    my_discs = (my_bits.high & 0x00ff0000ul) >> 16;

    opp_discs2 = ((((opp_bits.low & 0x08080808ul) | ((opp_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    my_discs2 = ((((my_bits.low & 0x08080808ul) | ((my_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0xff000000ul) >> 24;
    opp_discs3 = ((((opp_bits.low & 0x01000000ul) | (opp_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x01000000ul) | (my_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;

    flipped_discs = my_mask[RANK_D7][opp_mask[RANK_D7][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x08080808ul) | ((opp_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0x7e000000ul) >> 25;
    //my_discs2 = ((((my_bits.low & 0x08080808ul) | ((my_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[FILE_D7][opp_mask[FILE_D7][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01020408ul;
    flips_lo |= flipped_bits & 0x08080808ul;
    flips_hi |= (flipped_bits >> 4) & 0x08080808ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x01000000ul) | (opp_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x01000000ul) | (my_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_D7][opp_mask[DIAG1_D7][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x01000000ul;
    flips_hi |= flipped_bits & 0x10080402ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = ((((opp_bits.low & 0x40800000ul) | (opp_bits.high & 0x04081020ul)) * 0x01010101ul) & 0x78000000ul) >> 27;
    my_discs = ((((my_bits.low & 0x40800000ul) | (my_bits.high & 0x04081020ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_D7][opp_mask[DIAG2_D7][opp_discs] & my_discs];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x04040404ul;
    flips_lo |= flipped_bits & 0x40800000ul;
    flips_hi |= flipped_bits & 0x04081020ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_E7   4
#define FILE_E7   6
#define DIAG1_E7  4
#define DIAG2_E7  1

static int
TestFlips_bitboard5_e7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00100000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x007e0000ul) >> 17;
    my_discs = (my_bits.high & 0x00ff0000ul) >> 16;

    opp_discs2 = (((((opp_bits.low & 0x10101010ul) >> 4) | (opp_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    my_discs2 = (((((my_bits.low & 0x10101010ul) >> 4) | (my_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    opp_discs3 = ((((opp_bits.low & 0x02010000ul) | (opp_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x02010000ul) | (my_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;

    flipped_discs = my_mask[RANK_E7][opp_mask[RANK_E7][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x10101010ul) >> 4) | (opp_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x0fc00000ul) >> 22;
    //my_discs2 = (((((my_bits.low & 0x10101010ul) >> 4) | (my_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
    flipped_discs = my_mask[FILE_E7][opp_mask[FILE_E7][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x02040810ul;
    flips_lo |= flipped_bits & 0x10101010ul;
    flips_hi |= (flipped_bits >> 4) & 0x10101010ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x02010000ul) | (opp_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x1e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x02010000ul) | (my_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_E7][opp_mask[DIAG1_E7][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000001eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x02010000ul;
    flips_hi |= flipped_bits & 0x20100804ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = ((((opp_bits.low & 0x80000000ul) | (opp_bits.high & 0x08102040ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    my_discs = ((((my_bits.low & 0x80000000ul) | (my_bits.high & 0x08102040ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_E7][opp_mask[DIAG2_E7][opp_discs] & my_discs];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x08080808ul;
    flips_lo |= flipped_bits & 0x80000000ul;
    flips_hi |= flipped_bits & 0x08102040ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_C3   2
#define FILE_C3   2
#define DIAG1_C3  2
#define DIAG2_C3  2

static int
TestFlips_bitboard5_c3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00040000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x007e0000ul) >> 17;
    my_discs = (my_bits.low & 0x00ff0000ul) >> 16;

    opp_discs2 = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x08040201ul) | (opp_bits.high & 0x80402010ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x08040201ul) | (my_bits.high & 0x80402010ul)) * 0x01010101ul) & 0xff000000ul) >> 24;

    flipped_discs = my_mask[RANK_C3][opp_mask[RANK_C3][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_C3][opp_mask[FILE_C3][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00810204ul;
    flips_lo |= flipped_bits & 0x04040404ul;
    flips_hi |= (flipped_bits >> 4) & 0x04040404ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x08040201ul) | (opp_bits.high & 0x80402010ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x08040201ul) | (my_bits.high & 0x80402010ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_C3][opp_mask[DIAG1_C3][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x08040201ul;
    flips_hi |= flipped_bits & 0x80402010ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = ((((opp_bits.low & 0x02040810ul) | (opp_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    my_discs = ((((my_bits.low & 0x02040810ul) | (my_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_C3][opp_mask[DIAG2_C3][opp_discs] & my_discs];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x02040810ul;
    flips_hi |= flipped_bits & 0x00000001ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_F3   5
#define FILE_F3   2
#define DIAG1_F3  2
#define DIAG2_F3  5

static int
TestFlips_bitboard5_f3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_lo = my_bits.low;
    flips_hi = my_bits.high;
    flips_lo |= 0x00200000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.low & 0x007e0000ul) >> 17;
    my_discs = (my_bits.low & 0x00ff0000ul) >> 16;

    opp_discs2 = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x40201008ul) | (opp_bits.high & 0x00000080ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    my_discs3 = ((((my_bits.low & 0x40201008ul) | (my_bits.high & 0x00000080ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;

    flipped_discs = my_mask[RANK_F3][opp_mask[RANK_F3][opp_discs] & my_discs];
    flips_lo |= flipped_discs << 16;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_F3][opp_mask[FILE_F3][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 5;
    flips_hi |= (flipped_bits & 0x10101010ul) << 1;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x40201008ul) | (opp_bits.high & 0x00000080ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    //my_discs3 = ((((my_bits.low & 0x40201008ul) | (my_bits.high & 0x00000080ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
    flipped_discs = my_mask[DIAG1_F3][opp_mask[DIAG1_F3][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x08080808ul;
    flips_lo |= flipped_bits & 0x40201008ul;
    flips_hi |= flipped_bits & 0x00000080ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = ((((opp_bits.low & 0x10204080ul) | (opp_bits.high & 0x01020408ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    my_discs = ((((my_bits.low & 0x10204080ul) | (my_bits.high & 0x01020408ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_F3][opp_mask[DIAG2_F3][opp_discs] & my_discs];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x10204080ul;
    flips_hi |= flipped_bits & 0x01020408ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_C6   2
#define FILE_C6   5
#define DIAG1_C6  2
#define DIAG2_C6  2

static int
TestFlips_bitboard5_c6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00000400ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x00007e00ul) >> 9;
    my_discs = (my_bits.high & 0x0000ff00ul) >> 8;

    opp_discs2 = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    my_discs2 = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    opp_discs3 = ((((opp_bits.low & 0x01000000ul) | (opp_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x01000000ul) | (my_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;

    flipped_discs = my_mask[RANK_C6][opp_mask[RANK_C6][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x3f000000ul) >> 24;
    //my_discs2 = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
    flipped_discs = my_mask[FILE_C6][opp_mask[FILE_C6][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00810204ul;
    flips_lo |= flipped_bits & 0x04040404ul;
    flips_hi |= (flipped_bits >> 4) & 0x04040404ul;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x01000000ul) | (opp_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x0e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x01000000ul) | (my_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_C6][opp_mask[DIAG1_C6][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x01000000ul;
    flips_hi |= flipped_bits & 0x10080402ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = ((((opp_bits.low & 0x10204080ul) | (opp_bits.high & 0x01020408ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    my_discs = ((((my_bits.low & 0x10204080ul) | (my_bits.high & 0x01020408ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_C6][opp_mask[DIAG2_C6][opp_discs] & my_discs];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x10204080ul;
    flips_hi |= flipped_bits & 0x01020408ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

#define RANK_F6   5
#define FILE_F6   5
#define DIAG1_F6  5
#define DIAG2_F6  2

static int
TestFlips_bitboard5_f6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
    int flip_count;
    unsigned long my_discs, opp_discs;
    unsigned long flipped_discs, flipped_bits;
    unsigned long my_discs2, opp_discs2, my_discs3, opp_discs3;

    flips_hi = my_bits.high;
    flips_lo = my_bits.low;
    flips_hi |= 0x00002000ul;
    flip_count = 0;

    /* Rank : Left and right */
    opp_discs = (opp_bits.high & 0x00007e00ul) >> 9;
    my_discs = (my_bits.high & 0x0000ff00ul) >> 8;

    opp_discs2 = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    my_discs2 = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    opp_discs3 = ((((opp_bits.low & 0x08040201ul) | (opp_bits.high & 0x80402010ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    my_discs3 = ((((my_bits.low & 0x08040201ul) | (my_bits.high & 0x80402010ul)) * 0x01010101ul) & 0xff000000ul) >> 24;

    flipped_discs = my_mask[RANK_F6][opp_mask[RANK_F6][opp_discs] & my_discs];
    flips_hi |= flipped_discs << 8;
    flip_count += discs_count[flipped_discs >> 1];

    /* File : Up and down */
    //opp_discs2 = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x1f800000ul) >> 23;
    //my_discs2 = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
    flipped_discs = my_mask[FILE_F6][opp_mask[FILE_F6][opp_discs2] & my_discs2];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x00204081ul;
    flips_lo |= (flipped_bits & 0x01010101ul) << 5;
    flips_hi |= (flipped_bits & 0x10101010ul) << 1;
    flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-1 : Down right */
    //opp_discs3 = ((((opp_bits.low & 0x08040201ul) | (opp_bits.high & 0x80402010ul)) * 0x01010101ul) & 0x7e000000ul) >> 25;
    //my_discs3 = ((((my_bits.low & 0x08040201ul) | (my_bits.high & 0x80402010ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
    flipped_discs = my_mask[DIAG1_F6][opp_mask[DIAG1_F6][opp_discs3] & my_discs3];
    flipped_bits = (flipped_discs & 0x0000007eul) * 0x01010101ul;
    flips_lo |= flipped_bits & 0x08040201ul;
    flips_hi |= flipped_bits & 0x80402010ul;
    //flip_count += discs_count[flipped_discs >> 1];

    /* Diagonal-2 : Down left */
    opp_discs = ((((opp_bits.low & 0x80000000ul) | (opp_bits.high & 0x08102040ul)) * 0x01010101ul) & 0x70000000ul) >> 28;
    my_discs = ((((my_bits.low & 0x80000000ul) | (my_bits.high & 0x08102040ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
    flip_count += discs_count[flipped_discs >> 1];
    flipped_discs = my_mask[DIAG2_F6][opp_mask[DIAG2_F6][opp_discs] & my_discs];
    flipped_bits = (flipped_discs & 0x0000000eul) * 0x08080808ul;
    flips_lo |= flipped_bits & 0x80000000ul;
    flips_hi |= flipped_bits & 0x08102040ul;
    flip_count += discs_count[flipped_discs >> 1];

    bb_flips.low = flips_lo;
    bb_flips.high = flips_hi;

    return flip_count;
}

static int
TestFlips_bitboard5_d3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
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
TestFlips_bitboard5_e3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
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
TestFlips_bitboard5_c4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
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
TestFlips_bitboard5_f4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
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
TestFlips_bitboard5_c5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
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
TestFlips_bitboard5_f5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
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
TestFlips_bitboard5_d6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
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
TestFlips_bitboard5_e6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
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
TestFlips_bitboard5_d4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
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
TestFlips_bitboard5_e4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
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
TestFlips_bitboard5_d5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
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
TestFlips_bitboard5_e5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    unsigned long flips_hi;
    unsigned long flips_lo;
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

int(*TestFlips_bitboard5[64])(const BitBoard, const BitBoard) = {

    TestFlips_bitboard5_a1,
    TestFlips_bitboard5_b1,
    TestFlips_bitboard5_c1,
    TestFlips_bitboard5_d1,
    TestFlips_bitboard5_e1,
    TestFlips_bitboard5_f1,
    TestFlips_bitboard5_g1,
    TestFlips_bitboard5_h1,

    TestFlips_bitboard5_a2,
    TestFlips_bitboard5_b2,
    TestFlips_bitboard5_c2,
    TestFlips_bitboard5_d2,
    TestFlips_bitboard5_e2,
    TestFlips_bitboard5_f2,
    TestFlips_bitboard5_g2,
    TestFlips_bitboard5_h2,

    TestFlips_bitboard5_a3,
    TestFlips_bitboard5_b3,
    TestFlips_bitboard5_c3,
    TestFlips_bitboard5_d3,
    TestFlips_bitboard5_e3,
    TestFlips_bitboard5_f3,
    TestFlips_bitboard5_g3,
    TestFlips_bitboard5_h3,

    TestFlips_bitboard5_a4,
    TestFlips_bitboard5_b4,
    TestFlips_bitboard5_c4,
    TestFlips_bitboard5_d4,
    TestFlips_bitboard5_e4,
    TestFlips_bitboard5_f4,
    TestFlips_bitboard5_g4,
    TestFlips_bitboard5_h4,

    TestFlips_bitboard5_a5,
    TestFlips_bitboard5_b5,
    TestFlips_bitboard5_c5,
    TestFlips_bitboard5_d5,
    TestFlips_bitboard5_e5,
    TestFlips_bitboard5_f5,
    TestFlips_bitboard5_g5,
    TestFlips_bitboard5_h5,

    TestFlips_bitboard5_a6,
    TestFlips_bitboard5_b6,
    TestFlips_bitboard5_c6,
    TestFlips_bitboard5_d6,
    TestFlips_bitboard5_e6,
    TestFlips_bitboard5_f6,
    TestFlips_bitboard5_g6,
    TestFlips_bitboard5_h6,

    TestFlips_bitboard5_a7,
    TestFlips_bitboard5_b7,
    TestFlips_bitboard5_c7,
    TestFlips_bitboard5_d7,
    TestFlips_bitboard5_e7,
    TestFlips_bitboard5_f7,
    TestFlips_bitboard5_g7,
    TestFlips_bitboard5_h7,

    TestFlips_bitboard5_a8,
    TestFlips_bitboard5_b8,
    TestFlips_bitboard5_c8,
    TestFlips_bitboard5_d8,
    TestFlips_bitboard5_e8,
    TestFlips_bitboard5_f8,
    TestFlips_bitboard5_g8,
    TestFlips_bitboard5_h8

};
