/*
   File:           bitbchk.cpp

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard check a move is valid.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "utils.h"
#include "bitboard.h"
#include "bitbchk.h"

static unsigned int pop_count[64] = {
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,    /*  0 -- 15 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,    /* 16 -- 31 */
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,    /* 32 -- 47 */
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6     /* 48 -- 63 */
};

static unsigned int contiguous6[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    48, 48, 48, 48, 48, 48, 48, 48, 56, 56, 56, 56, 60, 60, 62, 63
};

static unsigned int contiguous5[32] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    16, 16, 16, 16, 16, 16, 16, 16, 24, 24, 24, 24, 28, 28, 30, 31
};
static unsigned int contiguous4[16] = {
    0,  0,  0,  0,  0,  0,  0,  0,  8,  8,  8,  8, 12, 12, 14, 15
};

static unsigned int contiguous3[8] = {
    0,  0,  0,  0,  4,  4,  6,  7
};

static unsigned int contiguous2[4] = {
    0,  0,  2,  3
};

/*
static unsigned int contiguous1[2] = {
    0,  1
};
//*/

static int
CheckFlips_bitboard_a1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00000002ul) & my_bits.low & 0x000000fcul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffffeul;
        flip_count = pop_count[flipped_discs >> 1];
        if (flip_count > 0)
            return TRUE;
    }
    /* Down */
    if (opp_low & 0x00000100ul) {
        if (opp_low & 0x00010000ul) {
            if (opp_low & 0x01000000ul) {
                if (opp_high & 0x00000001ul) {
                    if (opp_high & 0x00000100ul) {
                        if (opp_high & 0x00010000ul) {
                            if (my_bits.high & 0x01000000ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.high & 0x00010000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00000100ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000001ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x01000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00010000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x00000200ul) {
        if (opp_low & 0x00040000ul) {
            if (opp_low & 0x08000000ul) {
                if (opp_high & 0x00000010ul) {
                    if (opp_high & 0x00002000ul) {
                        if (opp_high & 0x00400000ul) {
                            if (my_bits.high & 0x80000000ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.high & 0x00400000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00002000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000010ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x08000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00040000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_h1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_low & 0x00000040ul) {
        unsigned int cont = contiguous6[(opp_low >> 1) & 63];
        if (cont & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x00004000ul) {
        if (opp_low & 0x00200000ul) {
            if (opp_low & 0x10000000ul) {
                if (opp_high & 0x00000008ul) {
                    if (opp_high & 0x00000400ul) {
                        if (opp_high & 0x00020000ul) {
                            if (my_bits.high & 0x01000000ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.high & 0x00020000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00000400ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000008ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x10000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00200000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x00008000ul) {
        if (opp_low & 0x00800000ul) {
            if (opp_low & 0x80000000ul) {
                if (opp_high & 0x00000080ul) {
                    if (opp_high & 0x00008000ul) {
                        if (opp_high & 0x00800000ul) {
                            if (my_bits.high & 0x80000000ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.high & 0x00800000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00008000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000080ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x80000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00800000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_a8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x02000000ul) & my_bits.high;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfe000000ul;
        flip_count = pop_count[flipped_discs >> 25];
        if (flip_count > 0)
            return TRUE;
    }
    /* Up right */
    if (opp_high & 0x00020000ul) {
        if (opp_high & 0x00000400ul) {
            if (opp_high & 0x00000008ul) {
                if (opp_low & 0x10000000ul) {
                    if (opp_low & 0x00200000ul) {
                        if (opp_low & 0x00004000ul) {
                            if (my_bits.low & 0x00000080ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.low & 0x00004000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00200000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x10000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000008ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000400ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00010000ul) {
        if (opp_high & 0x00000100ul) {
            if (opp_high & 0x00000001ul) {
                if (opp_low & 0x01000000ul) {
                    if (opp_low & 0x00010000ul) {
                        if (opp_low & 0x00000100ul) {
                            if (my_bits.low & 0x00000001ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.low & 0x00000100ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00010000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x01000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000001ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000100ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_h8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_high & 0x40000000ul) {
        unsigned int cont = contiguous6[(opp_high >> 25) & 63];
        if ((cont << 24) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00800000ul) {
        if (opp_high & 0x00008000ul) {
            if (opp_high & 0x00000080ul) {
                if (opp_low & 0x80000000ul) {
                    if (opp_low & 0x00800000ul) {
                        if (opp_low & 0x00008000ul) {
                            if (my_bits.low & 0x00000080ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.low & 0x00008000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00800000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x80000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000080ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00008000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00400000ul) {
        if (opp_high & 0x00002000ul) {
            if (opp_high & 0x00000010ul) {
                if (opp_low & 0x08000000ul) {
                    if (opp_low & 0x00040000ul) {
                        if (opp_low & 0x00000200ul) {
                            if (my_bits.low & 0x00000001ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.low & 0x00000200ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00040000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x08000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000010ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00002000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_b1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00000004ul) & my_bits.low & 0x000000f8ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffffcul;
        flip_count = pop_count[flipped_discs >> 2];
        if (flip_count > 0)
            return TRUE;
    }
    /* Down */
    if (opp_low & 0x00000200ul) {
        if (opp_low & 0x00020000ul) {
            if (opp_low & 0x02000000ul) {
                if (opp_high & 0x00000002ul) {
                    if (opp_high & 0x00000200ul) {
                        if (opp_high & 0x00020000ul) {
                            if (my_bits.high & 0x02000000ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.high & 0x00020000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00000200ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000002ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x02000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00020000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x00000400ul) {
        if (opp_low & 0x00080000ul) {
            if (opp_low & 0x10000000ul) {
                if (opp_high & 0x00000020ul) {
                    if (opp_high & 0x00004000ul) {
                        if (my_bits.high & 0x00800000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00004000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000020ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x10000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00080000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_g1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_low & 0x00000020ul) {
        unsigned int cont = contiguous5[(opp_low >> 1) & 31];
        if (cont & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x00002000ul) {
        if (opp_low & 0x00100000ul) {
            if (opp_low & 0x08000000ul) {
                if (opp_high & 0x00000004ul) {
                    if (opp_high & 0x00000200ul) {
                        if (my_bits.high & 0x00010000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00000200ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000004ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x08000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00100000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x00004000ul) {
        if (opp_low & 0x00400000ul) {
            if (opp_low & 0x40000000ul) {
                if (opp_high & 0x00000040ul) {
                    if (opp_high & 0x00004000ul) {
                        if (opp_high & 0x00400000ul) {
                            if (my_bits.high & 0x40000000ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.high & 0x00400000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00004000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000040ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x40000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00400000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_a2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00000200ul) & my_bits.low & 0x0000fc00ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffe00ul;
        flip_count = pop_count[flipped_discs >> 9];
        if (flip_count > 0)
            return TRUE;
    }
    /* Down */
    if (opp_low & 0x00010000ul) {
        if (opp_low & 0x01000000ul) {
            if (opp_high & 0x00000001ul) {
                if (opp_high & 0x00000100ul) {
                    if (opp_high & 0x00010000ul) {
                        if (my_bits.high & 0x01000000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00010000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000100ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000001ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x01000000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x00020000ul) {
        if (opp_low & 0x04000000ul) {
            if (opp_high & 0x00000008ul) {
                if (opp_high & 0x00001000ul) {
                    if (opp_high & 0x00200000ul) {
                        if (my_bits.high & 0x40000000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00200000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00001000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000008ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x04000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_h2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_low & 0x00004000ul) {
        unsigned int cont = contiguous6[(opp_low >> 9) & 63];
        if ((cont << 8) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x00400000ul) {
        if (opp_low & 0x20000000ul) {
            if (opp_high & 0x00000010ul) {
                if (opp_high & 0x00000800ul) {
                    if (opp_high & 0x00040000ul) {
                        if (my_bits.high & 0x02000000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00040000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000800ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000010ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x20000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x00800000ul) {
        if (opp_low & 0x80000000ul) {
            if (opp_high & 0x00000080ul) {
                if (opp_high & 0x00008000ul) {
                    if (opp_high & 0x00800000ul) {
                        if (my_bits.high & 0x80000000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00800000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00008000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000080ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x80000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_a7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00020000ul) & my_bits.high & 0x00fc0000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffe0000ul;
        flip_count = pop_count[flipped_discs >> 17];
        if (flip_count > 0)
            return TRUE;
    }
    /* Up right */
    if (opp_high & 0x00000200ul) {
        if (opp_high & 0x00000004ul) {
            if (opp_low & 0x08000000ul) {
                if (opp_low & 0x00100000ul) {
                    if (opp_low & 0x00002000ul) {
                        if (my_bits.low & 0x00000040ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00002000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00100000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x08000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000004ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00000100ul) {
        if (opp_high & 0x00000001ul) {
            if (opp_low & 0x01000000ul) {
                if (opp_low & 0x00010000ul) {
                    if (opp_low & 0x00000100ul) {
                        if (my_bits.low & 0x00000001ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00000100ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00010000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x01000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000001ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_h7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_high & 0x00400000ul) {
        unsigned int cont = contiguous6[(opp_high >> 17) & 63];
        if ((cont << 16) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00008000ul) {
        if (opp_high & 0x00000080ul) {
            if (opp_low & 0x80000000ul) {
                if (opp_low & 0x00800000ul) {
                    if (opp_low & 0x00008000ul) {
                        if (my_bits.low & 0x00000080ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00008000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00800000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x80000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000080ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00004000ul) {
        if (opp_high & 0x00000020ul) {
            if (opp_low & 0x10000000ul) {
                if (opp_low & 0x00080000ul) {
                    if (opp_low & 0x00000400ul) {
                        if (my_bits.low & 0x00000002ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00000400ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00080000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x10000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000020ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_b8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x04000000ul) & my_bits.high;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfc000000ul;
        flip_count = pop_count[flipped_discs >> 26];
        if (flip_count > 0)
            return TRUE;
    }
    /* Up right */
    if (opp_high & 0x00040000ul) {
        if (opp_high & 0x00000800ul) {
            if (opp_high & 0x00000010ul) {
                if (opp_low & 0x20000000ul) {
                    if (opp_low & 0x00400000ul) {
                        if (my_bits.low & 0x00008000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00400000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x20000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000010ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000800ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00020000ul) {
        if (opp_high & 0x00000200ul) {
            if (opp_high & 0x00000002ul) {
                if (opp_low & 0x02000000ul) {
                    if (opp_low & 0x00020000ul) {
                        if (opp_low & 0x00000200ul) {
                            if (my_bits.low & 0x00000002ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.low & 0x00000200ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00020000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x02000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000002ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000200ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_g8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_high & 0x20000000ul) {
        unsigned int cont = contiguous5[(opp_high >> 25) & 31];
        if ((cont << 24) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00400000ul) {
        if (opp_high & 0x00004000ul) {
            if (opp_high & 0x00000040ul) {
                if (opp_low & 0x40000000ul) {
                    if (opp_low & 0x00400000ul) {
                        if (opp_low & 0x00004000ul) {
                            if (my_bits.low & 0x00000040ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.low & 0x00004000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00400000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x40000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000040ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00004000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00200000ul) {
        if (opp_high & 0x00001000ul) {
            if (opp_high & 0x00000008ul) {
                if (opp_low & 0x04000000ul) {
                    if (opp_low & 0x00020000ul) {
                        if (my_bits.low & 0x00000100ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00020000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x04000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000008ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00001000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_b2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00000400ul) & my_bits.low & 0x0000f800ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffc00ul;
        flip_count = pop_count[flipped_discs >> 10];
        if (flip_count > 0)
            return TRUE;
    }
    /* Down */
    if (opp_low & 0x00020000ul) {
        if (opp_low & 0x02000000ul) {
            if (opp_high & 0x00000002ul) {
                if (opp_high & 0x00000200ul) {
                    if (opp_high & 0x00020000ul) {
                        if (my_bits.high & 0x02000000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00020000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000200ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000002ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x02000000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x00040000ul) {
        if (opp_low & 0x08000000ul) {
            if (opp_high & 0x00000010ul) {
                if (opp_high & 0x00002000ul) {
                    if (opp_high & 0x00400000ul) {
                        if (my_bits.high & 0x80000000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00400000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00002000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000010ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x08000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_g2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_low & 0x00002000ul) {
        unsigned int cont = contiguous5[(opp_low >> 9) & 31];
        if ((cont << 8) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x00200000ul) {
        if (opp_low & 0x10000000ul) {
            if (opp_high & 0x00000008ul) {
                if (opp_high & 0x00000400ul) {
                    if (opp_high & 0x00020000ul) {
                        if (my_bits.high & 0x01000000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00020000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000400ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000008ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x10000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x00400000ul) {
        if (opp_low & 0x40000000ul) {
            if (opp_high & 0x00000040ul) {
                if (opp_high & 0x00004000ul) {
                    if (opp_high & 0x00400000ul) {
                        if (my_bits.high & 0x40000000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00400000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00004000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000040ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x40000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_b7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00040000ul) & my_bits.high & 0x00f80000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffc0000ul;
        flip_count = pop_count[flipped_discs >> 18];
        if (flip_count > 0)
            return TRUE;
    }
    /* Up right */
    if (opp_high & 0x00000400ul) {
        if (opp_high & 0x00000008ul) {
            if (opp_low & 0x10000000ul) {
                if (opp_low & 0x00200000ul) {
                    if (opp_low & 0x00004000ul) {
                        if (my_bits.low & 0x00000080ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00004000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00200000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x10000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000008ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00000200ul) {
        if (opp_high & 0x00000002ul) {
            if (opp_low & 0x02000000ul) {
                if (opp_low & 0x00020000ul) {
                    if (opp_low & 0x00000200ul) {
                        if (my_bits.low & 0x00000002ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00000200ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00020000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x02000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000002ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_g7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_high & 0x00200000ul) {
        unsigned int cont = contiguous5[(opp_high >> 17) & 31];
        if ((cont << 16) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00004000ul) {
        if (opp_high & 0x00000040ul) {
            if (opp_low & 0x40000000ul) {
                if (opp_low & 0x00400000ul) {
                    if (opp_low & 0x00004000ul) {
                        if (my_bits.low & 0x00000040ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00004000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00400000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x40000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000040ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00002000ul) {
        if (opp_high & 0x00000010ul) {
            if (opp_low & 0x08000000ul) {
                if (opp_low & 0x00040000ul) {
                    if (opp_low & 0x00000200ul) {
                        if (my_bits.low & 0x00000001ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00000200ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00040000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x08000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000010ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_c1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00000008ul) & my_bits.low & 0x000000f0ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffff8ul;
        flip_count = pop_count[flipped_discs >> 3];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_low & 0x00000002ul) {
        if (my_bits.low & 0x00000001ul) {
            return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x00000200ul) {
        if (my_bits.low & 0x00010000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x00000400ul) {
        if (opp_low & 0x00040000ul) {
            if (opp_low & 0x04000000ul) {
                if (opp_high & 0x00000004ul) {
                    if (opp_high & 0x00000400ul) {
                        if (opp_high & 0x00040000ul) {
                            if (my_bits.high & 0x04000000ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.high & 0x00040000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00000400ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000004ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x04000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00040000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x00000800ul) {
        if (opp_low & 0x00100000ul) {
            if (opp_low & 0x20000000ul) {
                if (opp_high & 0x00000040ul) {
                    if (my_bits.high & 0x00008000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000040ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x20000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00100000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_f1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    if (opp_low & 0x00000040ul) {
        if (my_bits.low & 0x00000080ul) {
            return TRUE;
        }
    }
    /* Left */
    if (opp_low & 0x00000010ul) {
        unsigned int cont = contiguous4[(opp_low >> 1) & 15];
        if (cont & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x00001000ul) {
        if (opp_low & 0x00080000ul) {
            if (opp_low & 0x04000000ul) {
                if (opp_high & 0x00000002ul) {
                    if (my_bits.high & 0x00000100ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000002ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x04000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00080000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x00002000ul) {
        if (opp_low & 0x00200000ul) {
            if (opp_low & 0x20000000ul) {
                if (opp_high & 0x00000020ul) {
                    if (opp_high & 0x00002000ul) {
                        if (opp_high & 0x00200000ul) {
                            if (my_bits.high & 0x20000000ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.high & 0x00200000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00002000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000020ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x20000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00200000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x00004000ul) {
        if (my_bits.low & 0x00800000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_a3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00020000ul) & my_bits.low & 0x00fc0000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffe0000ul;
        flip_count = pop_count[flipped_discs >> 17];
        if (flip_count > 0)
            return TRUE;
    }
    /* Up right */
    if (opp_low & 0x00000200ul) {
        if (my_bits.low & 0x00000004ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x01000000ul) {
        if (opp_high & 0x00000001ul) {
            if (opp_high & 0x00000100ul) {
                if (opp_high & 0x00010000ul) {
                    if (my_bits.high & 0x01000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00010000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000100ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000001ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00000100ul) {
        if (my_bits.low & 0x00000001ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x02000000ul) {
        if (opp_high & 0x00000004ul) {
            if (opp_high & 0x00000800ul) {
                if (opp_high & 0x00100000ul) {
                    if (my_bits.high & 0x20000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00100000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000800ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000004ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_h3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_low & 0x00400000ul) {
        unsigned int cont = contiguous6[(opp_low >> 17) & 63];
        if ((cont << 16) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x40000000ul) {
        if (opp_high & 0x00000020ul) {
            if (opp_high & 0x00001000ul) {
                if (opp_high & 0x00080000ul) {
                    if (my_bits.high & 0x04000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00080000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00001000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000020ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x80000000ul) {
        if (opp_high & 0x00000080ul) {
            if (opp_high & 0x00008000ul) {
                if (opp_high & 0x00800000ul) {
                    if (my_bits.high & 0x80000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00800000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00008000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000080ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00008000ul) {
        if (my_bits.low & 0x00000080ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x00004000ul) {
        if (my_bits.low & 0x00000020ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_a6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00000200ul) & my_bits.high & 0x0000fc00ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffe00ul;
        flip_count = pop_count[flipped_discs >> 9];
        if (flip_count > 0)
            return TRUE;
    }
    /* Up right */
    if (opp_high & 0x00000002ul) {
        if (opp_low & 0x04000000ul) {
            if (opp_low & 0x00080000ul) {
                if (opp_low & 0x00001000ul) {
                    if (my_bits.low & 0x00000020ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00001000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00080000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x04000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00010000ul) {
        if (my_bits.high & 0x01000000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00000001ul) {
        if (opp_low & 0x01000000ul) {
            if (opp_low & 0x00010000ul) {
                if (opp_low & 0x00000100ul) {
                    if (my_bits.low & 0x00000001ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00000100ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00010000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x01000000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00020000ul) {
        if (my_bits.high & 0x04000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_h6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_high & 0x00004000ul) {
        unsigned int cont = contiguous6[(opp_high >> 9) & 63];
        if ((cont << 8) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00400000ul) {
        if (my_bits.high & 0x20000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00800000ul) {
        if (my_bits.high & 0x80000000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00000080ul) {
        if (opp_low & 0x80000000ul) {
            if (opp_low & 0x00800000ul) {
                if (opp_low & 0x00008000ul) {
                    if (my_bits.low & 0x00000080ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00008000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00800000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x80000000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00000040ul) {
        if (opp_low & 0x20000000ul) {
            if (opp_low & 0x00100000ul) {
                if (opp_low & 0x00000800ul) {
                    if (my_bits.low & 0x00000004ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00000800ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00100000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x20000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_c8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x08000000ul) & my_bits.high;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xf8000000ul;
        flip_count = pop_count[flipped_discs >> 27];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_high & 0x02000000ul) {
        if (my_bits.high & 0x01000000ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_high & 0x00080000ul) {
        if (opp_high & 0x00001000ul) {
            if (opp_high & 0x00000020ul) {
                if (opp_low & 0x40000000ul) {
                    if (my_bits.low & 0x00800000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x40000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000020ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00001000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00040000ul) {
        if (opp_high & 0x00000400ul) {
            if (opp_high & 0x00000004ul) {
                if (opp_low & 0x04000000ul) {
                    if (opp_low & 0x00040000ul) {
                        if (opp_low & 0x00000400ul) {
                            if (my_bits.low & 0x00000004ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.low & 0x00000400ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00040000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x04000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000004ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000400ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00020000ul) {
        if (my_bits.high & 0x00000100ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_f8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    if (opp_high & 0x40000000ul) {
        if (my_bits.high & 0x80000000ul) {
            return TRUE;
        }
    }
    /* Left */
    if (opp_high & 0x10000000ul) {
        unsigned int cont = contiguous4[(opp_high >> 25) & 15];
        if ((cont << 24) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Up right */
    if (opp_high & 0x00400000ul) {
        if (my_bits.high & 0x00008000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00200000ul) {
        if (opp_high & 0x00002000ul) {
            if (opp_high & 0x00000020ul) {
                if (opp_low & 0x20000000ul) {
                    if (opp_low & 0x00200000ul) {
                        if (opp_low & 0x00002000ul) {
                            if (my_bits.low & 0x00000020ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.low & 0x00002000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00200000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x20000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000020ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00002000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00100000ul) {
        if (opp_high & 0x00000800ul) {
            if (opp_high & 0x00000004ul) {
                if (opp_low & 0x02000000ul) {
                    if (my_bits.low & 0x00010000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x02000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000004ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000800ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_d1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00000010ul) & my_bits.low & 0x000000e0ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffff0ul;
        flip_count = pop_count[flipped_discs >> 4];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_low & 0x00000004ul) {
        unsigned int cont = contiguous2[(opp_low >> 1) & 3];
        if (cont & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x00000400ul) {
        if (opp_low & 0x00020000ul) {
            if (my_bits.low & 0x01000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00020000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x00000800ul) {
        if (opp_low & 0x00080000ul) {
            if (opp_low & 0x08000000ul) {
                if (opp_high & 0x00000008ul) {
                    if (opp_high & 0x00000800ul) {
                        if (opp_high & 0x00080000ul) {
                            if (my_bits.high & 0x08000000ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.high & 0x00080000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00000800ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000008ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x08000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00080000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x00001000ul) {
        if (opp_low & 0x00200000ul) {
            if (opp_low & 0x40000000ul) {
                if (my_bits.high & 0x00000080ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x40000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00200000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_e1(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00000020ul) & my_bits.low & 0x000000c0ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xffffffe0ul;
        flip_count = pop_count[flipped_discs >> 5];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_low & 0x00000008ul) {
        unsigned int cont = contiguous3[(opp_low >> 1) & 7];
        if (cont & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x00000800ul) {
        if (opp_low & 0x00040000ul) {
            if (opp_low & 0x02000000ul) {
                if (my_bits.high & 0x00000001ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x02000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00040000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x00001000ul) {
        if (opp_low & 0x00100000ul) {
            if (opp_low & 0x10000000ul) {
                if (opp_high & 0x00000010ul) {
                    if (opp_high & 0x00001000ul) {
                        if (opp_high & 0x00100000ul) {
                            if (my_bits.high & 0x10000000ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.high & 0x00100000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00001000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000010ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x10000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00100000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x00002000ul) {
        if (opp_low & 0x00400000ul) {
            if (my_bits.low & 0x80000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00400000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_a4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x02000000ul) & my_bits.low;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfe000000ul;
        flip_count = pop_count[flipped_discs >> 25];
        if (flip_count > 0)
            return TRUE;
    }
    /* Up right */
    if (opp_low & 0x00020000ul) {
        if (opp_low & 0x00000400ul) {
            if (my_bits.low & 0x00000008ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00000400ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00000001ul) {
        if (opp_high & 0x00000100ul) {
            if (opp_high & 0x00010000ul) {
                if (my_bits.high & 0x01000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00010000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000100ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00010000ul) {
        if (opp_low & 0x00000100ul) {
            if (my_bits.low & 0x00000001ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00000100ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00000002ul) {
        if (opp_high & 0x00000400ul) {
            if (opp_high & 0x00080000ul) {
                if (my_bits.high & 0x10000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00080000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000400ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_h4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_low & 0x40000000ul) {
        unsigned int cont = contiguous6[(opp_low >> 25) & 63];
        if ((cont << 24) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00000040ul) {
        if (opp_high & 0x00002000ul) {
            if (opp_high & 0x00100000ul) {
                if (my_bits.high & 0x08000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00100000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00002000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00000080ul) {
        if (opp_high & 0x00008000ul) {
            if (opp_high & 0x00800000ul) {
                if (my_bits.high & 0x80000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00800000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00008000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00800000ul) {
        if (opp_low & 0x00008000ul) {
            if (my_bits.low & 0x00000080ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00008000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x00400000ul) {
        if (opp_low & 0x00002000ul) {
            if (my_bits.low & 0x00000010ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00002000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_a5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00000002ul) & my_bits.high & 0x000000fcul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffffeul;
        flip_count = pop_count[flipped_discs >> 1];
        if (flip_count > 0)
            return TRUE;
    }
    /* Up right */
    if (opp_low & 0x02000000ul) {
        if (opp_low & 0x00040000ul) {
            if (opp_low & 0x00000800ul) {
                if (my_bits.low & 0x00000010ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00000800ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00040000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00000100ul) {
        if (opp_high & 0x00010000ul) {
            if (my_bits.high & 0x01000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00010000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x01000000ul) {
        if (opp_low & 0x00010000ul) {
            if (opp_low & 0x00000100ul) {
                if (my_bits.low & 0x00000001ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00000100ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00010000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00000200ul) {
        if (opp_high & 0x00040000ul) {
            if (my_bits.high & 0x08000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00040000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_h5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_high & 0x00000040ul) {
        unsigned int cont = contiguous6[(opp_high >> 1) & 63];
        if (cont & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00004000ul) {
        if (opp_high & 0x00200000ul) {
            if (my_bits.high & 0x10000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00200000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00008000ul) {
        if (opp_high & 0x00800000ul) {
            if (my_bits.high & 0x80000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00800000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x80000000ul) {
        if (opp_low & 0x00800000ul) {
            if (opp_low & 0x00008000ul) {
                if (my_bits.low & 0x00000080ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00008000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00800000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x40000000ul) {
        if (opp_low & 0x00200000ul) {
            if (opp_low & 0x00001000ul) {
                if (my_bits.low & 0x00000008ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00001000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00200000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_d8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x10000000ul) & my_bits.high;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xf0000000ul;
        flip_count = pop_count[flipped_discs >> 28];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_high & 0x04000000ul) {
        unsigned int cont = contiguous2[(opp_high >> 25) & 3];
        if ((cont << 24) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Up right */
    if (opp_high & 0x00100000ul) {
        if (opp_high & 0x00002000ul) {
            if (opp_high & 0x00000040ul) {
                if (my_bits.low & 0x80000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000040ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00002000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00080000ul) {
        if (opp_high & 0x00000800ul) {
            if (opp_high & 0x00000008ul) {
                if (opp_low & 0x08000000ul) {
                    if (opp_low & 0x00080000ul) {
                        if (opp_low & 0x00000800ul) {
                            if (my_bits.low & 0x00000008ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.low & 0x00000800ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00080000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x08000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000008ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000800ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00040000ul) {
        if (opp_high & 0x00000200ul) {
            if (my_bits.high & 0x00000001ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000200ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_e8(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x20000000ul) & my_bits.high;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xe0000000ul;
        flip_count = pop_count[flipped_discs >> 29];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_high & 0x08000000ul) {
        unsigned int cont = contiguous3[(opp_high >> 25) & 7];
        if ((cont << 24) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Up right */
    if (opp_high & 0x00200000ul) {
        if (opp_high & 0x00004000ul) {
            if (my_bits.high & 0x00000080ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00004000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00100000ul) {
        if (opp_high & 0x00001000ul) {
            if (opp_high & 0x00000010ul) {
                if (opp_low & 0x10000000ul) {
                    if (opp_low & 0x00100000ul) {
                        if (opp_low & 0x00001000ul) {
                            if (my_bits.low & 0x00000010ul) {
                                return TRUE;
                            }
                        }
                        else if (my_bits.low & 0x00001000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00100000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x10000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000010ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00001000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00080000ul) {
        if (opp_high & 0x00000400ul) {
            if (opp_high & 0x00000002ul) {
                if (my_bits.low & 0x01000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000002ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000400ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_c2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00000800ul) & my_bits.low & 0x0000f000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffff800ul;
        flip_count = pop_count[flipped_discs >> 11];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_low & 0x00000200ul) {
        if (my_bits.low & 0x00000100ul) {
            return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x00020000ul) {
        if (my_bits.low & 0x01000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x00040000ul) {
        if (opp_low & 0x04000000ul) {
            if (opp_high & 0x00000004ul) {
                if (opp_high & 0x00000400ul) {
                    if (opp_high & 0x00040000ul) {
                        if (my_bits.high & 0x04000000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00040000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000400ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000004ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x04000000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x00080000ul) {
        if (opp_low & 0x10000000ul) {
            if (opp_high & 0x00000020ul) {
                if (opp_high & 0x00004000ul) {
                    if (my_bits.high & 0x00800000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00004000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000020ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x10000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_f2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    if (opp_low & 0x00004000ul) {
        if (my_bits.low & 0x00008000ul) {
            return TRUE;
        }
    }
    /* Left */
    if (opp_low & 0x00001000ul) {
        unsigned int cont = contiguous4[(opp_low >> 9) & 15];
        if ((cont << 8) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x00100000ul) {
        if (opp_low & 0x08000000ul) {
            if (opp_high & 0x00000004ul) {
                if (opp_high & 0x00000200ul) {
                    if (my_bits.high & 0x00010000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000200ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000004ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x08000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x00200000ul) {
        if (opp_low & 0x20000000ul) {
            if (opp_high & 0x00000020ul) {
                if (opp_high & 0x00002000ul) {
                    if (opp_high & 0x00200000ul) {
                        if (my_bits.high & 0x20000000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00200000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00002000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000020ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x20000000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x00400000ul) {
        if (my_bits.low & 0x80000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_b3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00040000ul) & my_bits.low & 0x00f80000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffc0000ul;
        flip_count = pop_count[flipped_discs >> 18];
        if (flip_count > 0)
            return TRUE;
    }
    /* Up right */
    if (opp_low & 0x00000400ul) {
        if (my_bits.low & 0x00000008ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x02000000ul) {
        if (opp_high & 0x00000002ul) {
            if (opp_high & 0x00000200ul) {
                if (opp_high & 0x00020000ul) {
                    if (my_bits.high & 0x02000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00020000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000200ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000002ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00000200ul) {
        if (my_bits.low & 0x00000002ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x04000000ul) {
        if (opp_high & 0x00000008ul) {
            if (opp_high & 0x00001000ul) {
                if (opp_high & 0x00200000ul) {
                    if (my_bits.high & 0x40000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00200000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00001000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000008ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_g3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_low & 0x00200000ul) {
        unsigned int cont = contiguous5[(opp_low >> 17) & 31];
        if ((cont << 16) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x20000000ul) {
        if (opp_high & 0x00000010ul) {
            if (opp_high & 0x00000800ul) {
                if (opp_high & 0x00040000ul) {
                    if (my_bits.high & 0x02000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00040000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000800ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000010ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x40000000ul) {
        if (opp_high & 0x00000040ul) {
            if (opp_high & 0x00004000ul) {
                if (opp_high & 0x00400000ul) {
                    if (my_bits.high & 0x40000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00400000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00004000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000040ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00004000ul) {
        if (my_bits.low & 0x00000040ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x00002000ul) {
        if (my_bits.low & 0x00000010ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_b6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00000400ul) & my_bits.high & 0x0000f800ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffc00ul;
        flip_count = pop_count[flipped_discs >> 10];
        if (flip_count > 0)
            return TRUE;
    }
    /* Up right */
    if (opp_high & 0x00000004ul) {
        if (opp_low & 0x08000000ul) {
            if (opp_low & 0x00100000ul) {
                if (opp_low & 0x00002000ul) {
                    if (my_bits.low & 0x00000040ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00002000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00100000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x08000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00020000ul) {
        if (my_bits.high & 0x02000000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00000002ul) {
        if (opp_low & 0x02000000ul) {
            if (opp_low & 0x00020000ul) {
                if (opp_low & 0x00000200ul) {
                    if (my_bits.low & 0x00000002ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00000200ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00020000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x02000000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00040000ul) {
        if (my_bits.high & 0x08000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_g6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_high & 0x00002000ul) {
        unsigned int cont = contiguous5[(opp_high >> 9) & 31];
        if ((cont << 8) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00200000ul) {
        if (my_bits.high & 0x10000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00400000ul) {
        if (my_bits.high & 0x40000000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00000040ul) {
        if (opp_low & 0x40000000ul) {
            if (opp_low & 0x00400000ul) {
                if (opp_low & 0x00004000ul) {
                    if (my_bits.low & 0x00000040ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00004000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00400000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x40000000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00000020ul) {
        if (opp_low & 0x10000000ul) {
            if (opp_low & 0x00080000ul) {
                if (opp_low & 0x00000400ul) {
                    if (my_bits.low & 0x00000002ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00000400ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00080000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x10000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_c7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00080000ul) & my_bits.high & 0x00f00000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfff80000ul;
        flip_count = pop_count[flipped_discs >> 19];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_high & 0x00020000ul) {
        if (my_bits.high & 0x00010000ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_high & 0x00000800ul) {
        if (opp_high & 0x00000010ul) {
            if (opp_low & 0x20000000ul) {
                if (opp_low & 0x00400000ul) {
                    if (my_bits.low & 0x00008000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00400000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x20000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000010ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00000400ul) {
        if (opp_high & 0x00000004ul) {
            if (opp_low & 0x04000000ul) {
                if (opp_low & 0x00040000ul) {
                    if (opp_low & 0x00000400ul) {
                        if (my_bits.low & 0x00000004ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00000400ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00040000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x04000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000004ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00000200ul) {
        if (my_bits.high & 0x00000001ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_f7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    if (opp_high & 0x00400000ul) {
        if (my_bits.high & 0x00800000ul) {
            return TRUE;
        }
    }
    /* Left */
    if (opp_high & 0x00100000ul) {
        unsigned int cont = contiguous4[(opp_high >> 17) & 15];
        if ((cont << 16) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Up right */
    if (opp_high & 0x00004000ul) {
        if (my_bits.high & 0x00000080ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00002000ul) {
        if (opp_high & 0x00000020ul) {
            if (opp_low & 0x20000000ul) {
                if (opp_low & 0x00200000ul) {
                    if (opp_low & 0x00002000ul) {
                        if (my_bits.low & 0x00000020ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00002000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00200000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x20000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000020ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00001000ul) {
        if (opp_high & 0x00000008ul) {
            if (opp_low & 0x04000000ul) {
                if (opp_low & 0x00020000ul) {
                    if (my_bits.low & 0x00000100ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00020000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x04000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000008ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_d2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00001000ul) & my_bits.low & 0x0000e000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffff000ul;
        flip_count = pop_count[flipped_discs >> 12];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_low & 0x00000400ul) {
        unsigned int cont = contiguous2[(opp_low >> 9) & 3];
        if ((cont << 8) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x00040000ul) {
        if (opp_low & 0x02000000ul) {
            if (my_bits.high & 0x00000001ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x02000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x00080000ul) {
        if (opp_low & 0x08000000ul) {
            if (opp_high & 0x00000008ul) {
                if (opp_high & 0x00000800ul) {
                    if (opp_high & 0x00080000ul) {
                        if (my_bits.high & 0x08000000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00080000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00000800ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000008ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x08000000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x00100000ul) {
        if (opp_low & 0x20000000ul) {
            if (opp_high & 0x00000040ul) {
                if (my_bits.high & 0x00008000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000040ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x20000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_e2(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00002000ul) & my_bits.low & 0x0000c000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xffffe000ul;
        flip_count = pop_count[flipped_discs >> 13];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_low & 0x00000800ul) {
        unsigned int cont = contiguous3[(opp_low >> 9) & 7];
        if ((cont << 8) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x00080000ul) {
        if (opp_low & 0x04000000ul) {
            if (opp_high & 0x00000002ul) {
                if (my_bits.high & 0x00000100ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000002ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x04000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x00100000ul) {
        if (opp_low & 0x10000000ul) {
            if (opp_high & 0x00000010ul) {
                if (opp_high & 0x00001000ul) {
                    if (opp_high & 0x00100000ul) {
                        if (my_bits.high & 0x10000000ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.high & 0x00100000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00001000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000010ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x10000000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x00200000ul) {
        if (opp_low & 0x40000000ul) {
            if (my_bits.high & 0x00000080ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x40000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_b4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x04000000ul) & my_bits.low;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfc000000ul;
        flip_count = pop_count[flipped_discs >> 26];
        if (flip_count > 0)
            return TRUE;
    }
    /* Up right */
    if (opp_low & 0x00040000ul) {
        if (opp_low & 0x00000800ul) {
            if (my_bits.low & 0x00000010ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00000800ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00000002ul) {
        if (opp_high & 0x00000200ul) {
            if (opp_high & 0x00020000ul) {
                if (my_bits.high & 0x02000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00020000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000200ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00020000ul) {
        if (opp_low & 0x00000200ul) {
            if (my_bits.low & 0x00000002ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00000200ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00000004ul) {
        if (opp_high & 0x00000800ul) {
            if (opp_high & 0x00100000ul) {
                if (my_bits.high & 0x20000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00100000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000800ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_g4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_low & 0x20000000ul) {
        unsigned int cont = contiguous5[(opp_low >> 25) & 31];
        if ((cont << 24) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00000020ul) {
        if (opp_high & 0x00001000ul) {
            if (opp_high & 0x00080000ul) {
                if (my_bits.high & 0x04000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00080000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00001000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00000040ul) {
        if (opp_high & 0x00004000ul) {
            if (opp_high & 0x00400000ul) {
                if (my_bits.high & 0x40000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00400000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00004000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00400000ul) {
        if (opp_low & 0x00004000ul) {
            if (my_bits.low & 0x00000040ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00004000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x00200000ul) {
        if (opp_low & 0x00001000ul) {
            if (my_bits.low & 0x00000008ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00001000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_b5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00000004ul) & my_bits.high & 0x000000f8ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffffcul;
        flip_count = pop_count[flipped_discs >> 2];
        if (flip_count > 0)
            return TRUE;
    }
    /* Up right */
    if (opp_low & 0x04000000ul) {
        if (opp_low & 0x00080000ul) {
            if (opp_low & 0x00001000ul) {
                if (my_bits.low & 0x00000020ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00001000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00080000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00000200ul) {
        if (opp_high & 0x00020000ul) {
            if (my_bits.high & 0x02000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00020000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x02000000ul) {
        if (opp_low & 0x00020000ul) {
            if (opp_low & 0x00000200ul) {
                if (my_bits.low & 0x00000002ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00000200ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00020000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00000400ul) {
        if (opp_high & 0x00080000ul) {
            if (my_bits.high & 0x10000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00080000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_g5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Left */
    if (opp_high & 0x00000020ul) {
        unsigned int cont = contiguous5[(opp_high >> 1) & 31];
        if (cont & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00002000ul) {
        if (opp_high & 0x00100000ul) {
            if (my_bits.high & 0x08000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00100000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00004000ul) {
        if (opp_high & 0x00400000ul) {
            if (my_bits.high & 0x40000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00400000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x40000000ul) {
        if (opp_low & 0x00400000ul) {
            if (opp_low & 0x00004000ul) {
                if (my_bits.low & 0x00000040ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00004000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00400000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x20000000ul) {
        if (opp_low & 0x00100000ul) {
            if (opp_low & 0x00000800ul) {
                if (my_bits.low & 0x00000004ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00000800ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00100000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_d7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00100000ul) & my_bits.high & 0x00e00000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfff00000ul;
        flip_count = pop_count[flipped_discs >> 20];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_high & 0x00040000ul) {
        unsigned int cont = contiguous2[(opp_high >> 17) & 3];
        if ((cont << 16) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Up right */
    if (opp_high & 0x00001000ul) {
        if (opp_high & 0x00000020ul) {
            if (opp_low & 0x40000000ul) {
                if (my_bits.low & 0x00800000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x40000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000020ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00000800ul) {
        if (opp_high & 0x00000008ul) {
            if (opp_low & 0x08000000ul) {
                if (opp_low & 0x00080000ul) {
                    if (opp_low & 0x00000800ul) {
                        if (my_bits.low & 0x00000008ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00000800ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00080000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x08000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000008ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00000400ul) {
        if (opp_high & 0x00000002ul) {
            if (my_bits.low & 0x01000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000002ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_e7(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00200000ul) & my_bits.high & 0x00c00000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xffe00000ul;
        flip_count = pop_count[flipped_discs >> 21];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_high & 0x00080000ul) {
        unsigned int cont = contiguous3[(opp_high >> 17) & 7];
        if ((cont << 16) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Up right */
    if (opp_high & 0x00002000ul) {
        if (opp_high & 0x00000040ul) {
            if (my_bits.low & 0x80000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000040ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00001000ul) {
        if (opp_high & 0x00000010ul) {
            if (opp_low & 0x10000000ul) {
                if (opp_low & 0x00100000ul) {
                    if (opp_low & 0x00001000ul) {
                        if (my_bits.low & 0x00000010ul) {
                            return TRUE;
                        }
                    }
                    else if (my_bits.low & 0x00001000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00100000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x10000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000010ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00000800ul) {
        if (opp_high & 0x00000004ul) {
            if (opp_low & 0x02000000ul) {
                if (my_bits.low & 0x00010000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x02000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000004ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_c3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00080000ul) & my_bits.low & 0x00f00000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfff80000ul;
        flip_count = pop_count[flipped_discs >> 19];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_low & 0x00020000ul) {
        if (my_bits.low & 0x00010000ul) {
            return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x02000000ul) {
        if (my_bits.high & 0x00000001ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_low & 0x00000800ul) {
        if (my_bits.low & 0x00000010ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x04000000ul) {
        if (opp_high & 0x00000004ul) {
            if (opp_high & 0x00000400ul) {
                if (opp_high & 0x00040000ul) {
                    if (my_bits.high & 0x04000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00040000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000400ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000004ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00000400ul) {
        if (my_bits.low & 0x00000004ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x08000000ul) {
        if (opp_high & 0x00000010ul) {
            if (opp_high & 0x00002000ul) {
                if (opp_high & 0x00400000ul) {
                    if (my_bits.high & 0x80000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00400000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00002000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000010ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x00000200ul) {
        if (my_bits.low & 0x00000001ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_f3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    if (opp_low & 0x00400000ul) {
        if (my_bits.low & 0x00800000ul) {
            return TRUE;
        }
    }
    /* Left */
    if (opp_low & 0x00100000ul) {
        unsigned int cont = contiguous4[(opp_low >> 17) & 15];
        if ((cont << 16) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x10000000ul) {
        if (opp_high & 0x00000008ul) {
            if (opp_high & 0x00000400ul) {
                if (opp_high & 0x00020000ul) {
                    if (my_bits.high & 0x01000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00020000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000400ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000008ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_low & 0x00004000ul) {
        if (my_bits.low & 0x00000080ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x20000000ul) {
        if (opp_high & 0x00000020ul) {
            if (opp_high & 0x00002000ul) {
                if (opp_high & 0x00200000ul) {
                    if (my_bits.high & 0x20000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00200000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00002000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000020ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00002000ul) {
        if (my_bits.low & 0x00000020ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x40000000ul) {
        if (my_bits.high & 0x00000080ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x00001000ul) {
        if (my_bits.low & 0x00000008ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_c6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00000800ul) & my_bits.high & 0x0000f000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffff800ul;
        flip_count = pop_count[flipped_discs >> 11];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_high & 0x00000200ul) {
        if (my_bits.high & 0x00000100ul) {
            return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00020000ul) {
        if (my_bits.high & 0x01000000ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_high & 0x00000008ul) {
        if (opp_low & 0x10000000ul) {
            if (opp_low & 0x00200000ul) {
                if (opp_low & 0x00004000ul) {
                    if (my_bits.low & 0x00000080ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00004000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00200000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x10000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00040000ul) {
        if (my_bits.high & 0x04000000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00000004ul) {
        if (opp_low & 0x04000000ul) {
            if (opp_low & 0x00040000ul) {
                if (opp_low & 0x00000400ul) {
                    if (my_bits.low & 0x00000004ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00000400ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00040000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x04000000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00080000ul) {
        if (my_bits.high & 0x10000000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00000002ul) {
        if (my_bits.low & 0x01000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_f6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    if (opp_high & 0x00004000ul) {
        if (my_bits.high & 0x00008000ul) {
            return TRUE;
        }
    }
    /* Left */
    if (opp_high & 0x00001000ul) {
        unsigned int cont = contiguous4[(opp_high >> 9) & 15];
        if ((cont << 8) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00100000ul) {
        if (my_bits.high & 0x08000000ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_high & 0x00000040ul) {
        if (my_bits.low & 0x80000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00200000ul) {
        if (my_bits.high & 0x20000000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00000020ul) {
        if (opp_low & 0x20000000ul) {
            if (opp_low & 0x00200000ul) {
                if (opp_low & 0x00002000ul) {
                    if (my_bits.low & 0x00000020ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00002000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00200000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x20000000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00400000ul) {
        if (my_bits.high & 0x80000000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00000010ul) {
        if (opp_low & 0x08000000ul) {
            if (opp_low & 0x00040000ul) {
                if (opp_low & 0x00000200ul) {
                    if (my_bits.low & 0x00000001ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00000200ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00040000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x08000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_d3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00100000ul) & my_bits.low & 0x00e00000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfff00000ul;
        flip_count = pop_count[flipped_discs >> 20];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_low & 0x00040000ul) {
        unsigned int cont = contiguous2[(opp_low >> 17) & 3];
        if ((cont << 16) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x04000000ul) {
        if (opp_high & 0x00000002ul) {
            if (my_bits.high & 0x00000100ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000002ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_low & 0x00001000ul) {
        if (my_bits.low & 0x00000020ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x08000000ul) {
        if (opp_high & 0x00000008ul) {
            if (opp_high & 0x00000800ul) {
                if (opp_high & 0x00080000ul) {
                    if (my_bits.high & 0x08000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00080000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000800ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000008ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00000800ul) {
        if (my_bits.low & 0x00000008ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x10000000ul) {
        if (opp_high & 0x00000020ul) {
            if (opp_high & 0x00004000ul) {
                if (my_bits.high & 0x00800000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00004000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000020ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x00000400ul) {
        if (my_bits.low & 0x00000002ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_e3(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x00200000ul) & my_bits.low & 0x00c00000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xffe00000ul;
        flip_count = pop_count[flipped_discs >> 21];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_low & 0x00080000ul) {
        unsigned int cont = contiguous3[(opp_low >> 17) & 7];
        if ((cont << 16) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_low & 0x08000000ul) {
        if (opp_high & 0x00000004ul) {
            if (opp_high & 0x00000200ul) {
                if (my_bits.high & 0x00010000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00000200ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000004ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_low & 0x00002000ul) {
        if (my_bits.low & 0x00000040ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_low & 0x10000000ul) {
        if (opp_high & 0x00000010ul) {
            if (opp_high & 0x00001000ul) {
                if (opp_high & 0x00100000ul) {
                    if (my_bits.high & 0x10000000ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.high & 0x00100000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00001000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000010ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00001000ul) {
        if (my_bits.low & 0x00000010ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_low & 0x20000000ul) {
        if (opp_high & 0x00000040ul) {
            if (my_bits.high & 0x00008000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000040ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x00000800ul) {
        if (my_bits.low & 0x00000004ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_c4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x08000000ul) & my_bits.low;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xf8000000ul;
        flip_count = pop_count[flipped_discs >> 27];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_low & 0x02000000ul) {
        if (my_bits.low & 0x01000000ul) {
            return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00000002ul) {
        if (my_bits.high & 0x00000100ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_low & 0x00080000ul) {
        if (opp_low & 0x00001000ul) {
            if (my_bits.low & 0x00000020ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00001000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00000004ul) {
        if (opp_high & 0x00000400ul) {
            if (opp_high & 0x00040000ul) {
                if (my_bits.high & 0x04000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00040000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000400ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00040000ul) {
        if (opp_low & 0x00000400ul) {
            if (my_bits.low & 0x00000004ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00000400ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00000008ul) {
        if (opp_high & 0x00001000ul) {
            if (opp_high & 0x00200000ul) {
                if (my_bits.high & 0x40000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00200000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00001000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x00020000ul) {
        if (my_bits.low & 0x00000100ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_f4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    if (opp_low & 0x40000000ul) {
        if (my_bits.low & 0x80000000ul) {
            return TRUE;
        }
    }
    /* Left */
    if (opp_low & 0x10000000ul) {
        unsigned int cont = contiguous4[(opp_low >> 25) & 15];
        if ((cont << 24) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00000010ul) {
        if (opp_high & 0x00000800ul) {
            if (opp_high & 0x00040000ul) {
                if (my_bits.high & 0x02000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00040000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000800ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_low & 0x00400000ul) {
        if (my_bits.low & 0x00008000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00000020ul) {
        if (opp_high & 0x00002000ul) {
            if (opp_high & 0x00200000ul) {
                if (my_bits.high & 0x20000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00200000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00002000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00200000ul) {
        if (opp_low & 0x00002000ul) {
            if (my_bits.low & 0x00000020ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00002000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00000040ul) {
        if (my_bits.high & 0x00008000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x00100000ul) {
        if (opp_low & 0x00000800ul) {
            if (my_bits.low & 0x00000004ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00000800ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_c5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00000008ul) & my_bits.high & 0x000000f0ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffff8ul;
        flip_count = pop_count[flipped_discs >> 3];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_high & 0x00000002ul) {
        if (my_bits.high & 0x00000001ul) {
            return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00000200ul) {
        if (my_bits.high & 0x00010000ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_low & 0x08000000ul) {
        if (opp_low & 0x00100000ul) {
            if (opp_low & 0x00002000ul) {
                if (my_bits.low & 0x00000040ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00002000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00100000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00000400ul) {
        if (opp_high & 0x00040000ul) {
            if (my_bits.high & 0x04000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00040000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x04000000ul) {
        if (opp_low & 0x00040000ul) {
            if (opp_low & 0x00000400ul) {
                if (my_bits.low & 0x00000004ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00000400ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00040000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00000800ul) {
        if (opp_high & 0x00100000ul) {
            if (my_bits.high & 0x20000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00100000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x02000000ul) {
        if (my_bits.low & 0x00010000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_f5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    if (opp_high & 0x00000040ul) {
        if (my_bits.high & 0x00000080ul) {
            return TRUE;
        }
    }
    /* Left */
    if (opp_high & 0x00000010ul) {
        unsigned int cont = contiguous4[(opp_high >> 1) & 15];
        if (cont & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00001000ul) {
        if (opp_high & 0x00080000ul) {
            if (my_bits.high & 0x04000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00080000ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_low & 0x40000000ul) {
        if (my_bits.low & 0x00800000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00002000ul) {
        if (opp_high & 0x00200000ul) {
            if (my_bits.high & 0x20000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00200000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x20000000ul) {
        if (opp_low & 0x00200000ul) {
            if (opp_low & 0x00002000ul) {
                if (my_bits.low & 0x00000020ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00002000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00200000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00004000ul) {
        if (my_bits.high & 0x00800000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x10000000ul) {
        if (opp_low & 0x00080000ul) {
            if (opp_low & 0x00000400ul) {
                if (my_bits.low & 0x00000002ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00000400ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00080000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_d6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00001000ul) & my_bits.high & 0x0000e000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffff000ul;
        flip_count = pop_count[flipped_discs >> 12];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_high & 0x00000400ul) {
        unsigned int cont = contiguous2[(opp_high >> 9) & 3];
        if ((cont << 8) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00040000ul) {
        if (my_bits.high & 0x02000000ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_high & 0x00000010ul) {
        if (opp_low & 0x20000000ul) {
            if (opp_low & 0x00400000ul) {
                if (my_bits.low & 0x00008000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00400000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x20000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00080000ul) {
        if (my_bits.high & 0x08000000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00000008ul) {
        if (opp_low & 0x08000000ul) {
            if (opp_low & 0x00080000ul) {
                if (opp_low & 0x00000800ul) {
                    if (my_bits.low & 0x00000008ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00000800ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00080000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x08000000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00100000ul) {
        if (my_bits.high & 0x20000000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00000004ul) {
        if (opp_low & 0x02000000ul) {
            if (my_bits.low & 0x00010000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x02000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_e6(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00002000ul) & my_bits.high & 0x0000c000ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xffffe000ul;
        flip_count = pop_count[flipped_discs >> 13];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_high & 0x00000800ul) {
        unsigned int cont = contiguous3[(opp_high >> 9) & 7];
        if ((cont << 8) & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00080000ul) {
        if (my_bits.high & 0x04000000ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_high & 0x00000020ul) {
        if (opp_low & 0x40000000ul) {
            if (my_bits.low & 0x00800000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x40000000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00100000ul) {
        if (my_bits.high & 0x10000000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_high & 0x00000010ul) {
        if (opp_low & 0x10000000ul) {
            if (opp_low & 0x00100000ul) {
                if (opp_low & 0x00001000ul) {
                    if (my_bits.low & 0x00000010ul) {
                        return TRUE;
                    }
                }
                else if (my_bits.low & 0x00001000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00100000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x10000000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00200000ul) {
        if (my_bits.high & 0x40000000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_high & 0x00000008ul) {
        if (opp_low & 0x04000000ul) {
            if (opp_low & 0x00020000ul) {
                if (my_bits.low & 0x00000100ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00020000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x04000000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_d4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x10000000ul) & my_bits.low;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xf0000000ul;
        flip_count = pop_count[flipped_discs >> 28];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_low & 0x04000000ul) {
        unsigned int cont = contiguous2[(opp_low >> 25) & 3];
        if ((cont << 24) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00000004ul) {
        if (opp_high & 0x00000200ul) {
            if (my_bits.high & 0x00010000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000200ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_low & 0x00100000ul) {
        if (opp_low & 0x00002000ul) {
            if (my_bits.low & 0x00000040ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00002000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00000008ul) {
        if (opp_high & 0x00000800ul) {
            if (opp_high & 0x00080000ul) {
                if (my_bits.high & 0x08000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00080000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000800ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00080000ul) {
        if (opp_low & 0x00000800ul) {
            if (my_bits.low & 0x00000008ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00000800ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00000010ul) {
        if (opp_high & 0x00002000ul) {
            if (opp_high & 0x00400000ul) {
                if (my_bits.high & 0x80000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00400000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00002000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x00040000ul) {
        if (opp_low & 0x00000200ul) {
            if (my_bits.low & 0x00000001ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00000200ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_e4(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_low + 0x20000000ul) & my_bits.low;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xe0000000ul;
        flip_count = pop_count[flipped_discs >> 29];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_low & 0x08000000ul) {
        unsigned int cont = contiguous3[(opp_low >> 25) & 7];
        if ((cont << 24) & my_bits.low) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00000008ul) {
        if (opp_high & 0x00000400ul) {
            if (opp_high & 0x00020000ul) {
                if (my_bits.high & 0x01000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00020000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00000400ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_low & 0x00200000ul) {
        if (opp_low & 0x00004000ul) {
            if (my_bits.low & 0x00000080ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00004000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00000010ul) {
        if (opp_high & 0x00001000ul) {
            if (opp_high & 0x00100000ul) {
                if (my_bits.high & 0x10000000ul) {
                    return TRUE;
                }
            }
            else if (my_bits.high & 0x00100000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00001000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x00100000ul) {
        if (opp_low & 0x00001000ul) {
            if (my_bits.low & 0x00000010ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00001000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00000020ul) {
        if (opp_high & 0x00004000ul) {
            if (my_bits.high & 0x00800000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00004000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x00080000ul) {
        if (opp_low & 0x00000400ul) {
            if (my_bits.low & 0x00000002ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00000400ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_d5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00000010ul) & my_bits.high & 0x000000e0ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xfffffff0ul;
        flip_count = pop_count[flipped_discs >> 4];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_high & 0x00000004ul) {
        unsigned int cont = contiguous2[(opp_high >> 1) & 3];
        if (cont & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00000400ul) {
        if (opp_high & 0x00020000ul) {
            if (my_bits.high & 0x01000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00020000ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_low & 0x10000000ul) {
        if (opp_low & 0x00200000ul) {
            if (opp_low & 0x00004000ul) {
                if (my_bits.low & 0x00000080ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00004000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00200000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00000800ul) {
        if (opp_high & 0x00080000ul) {
            if (my_bits.high & 0x08000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00080000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x08000000ul) {
        if (opp_low & 0x00080000ul) {
            if (opp_low & 0x00000800ul) {
                if (my_bits.low & 0x00000008ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00000800ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00080000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00001000ul) {
        if (opp_high & 0x00200000ul) {
            if (my_bits.high & 0x40000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00200000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x04000000ul) {
        if (opp_low & 0x00020000ul) {
            if (my_bits.low & 0x00000100ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00020000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

static int
CheckFlips_bitboard_e5(const BitBoard my_bits,
    const BitBoard opp_bits) {
    register unsigned long opp_low;
    register unsigned long opp_high;
    int flip_count;
    unsigned int first_flip;

    opp_low = opp_bits.low;
    opp_high = opp_bits.high;

    /* Right */
    first_flip = (opp_high + 0x00000020ul) & my_bits.high & 0x000000c0ul;
    if (first_flip) {
        unsigned int flipped_discs;
        flipped_discs = (first_flip - 1) & 0xffffffe0ul;
        flip_count = pop_count[flipped_discs >> 5];
        if (flip_count > 0)
            return TRUE;
    }
    /* Left */
    if (opp_high & 0x00000008ul) {
        unsigned int cont = contiguous3[(opp_high >> 1) & 7];
        if (cont & my_bits.high) {
            flip_count = pop_count[cont];
            if (flip_count > 0)
                return TRUE;
        }
    }
    /* Down left */
    if (opp_high & 0x00000800ul) {
        if (opp_high & 0x00040000ul) {
            if (my_bits.high & 0x02000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00040000ul) {
            return TRUE;
        }
    }
    /* Up right */
    if (opp_low & 0x20000000ul) {
        if (opp_low & 0x00400000ul) {
            if (my_bits.low & 0x00008000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00400000ul) {
            return TRUE;
        }
    }
    /* Down */
    if (opp_high & 0x00001000ul) {
        if (opp_high & 0x00100000ul) {
            if (my_bits.high & 0x10000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00100000ul) {
            return TRUE;
        }
    }
    /* Up */
    if (opp_low & 0x10000000ul) {
        if (opp_low & 0x00100000ul) {
            if (opp_low & 0x00001000ul) {
                if (my_bits.low & 0x00000010ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00001000ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00100000ul) {
            return TRUE;
        }
    }
    /* Down right */
    if (opp_high & 0x00002000ul) {
        if (opp_high & 0x00400000ul) {
            if (my_bits.high & 0x80000000ul) {
                return TRUE;
            }
        }
        else if (my_bits.high & 0x00400000ul) {
            return TRUE;
        }
    }
    /* Up left */
    if (opp_low & 0x08000000ul) {
        if (opp_low & 0x00040000ul) {
            if (opp_low & 0x00000200ul) {
                if (my_bits.low & 0x00000001ul) {
                    return TRUE;
                }
            }
            else if (my_bits.low & 0x00000200ul) {
                return TRUE;
            }
        }
        else if (my_bits.low & 0x00040000ul) {
            return TRUE;
        }
    }

    return FALSE;
}

int(*CheckFlips_bitboard[64])(const BitBoard, const BitBoard) = {
    CheckFlips_bitboard_a1,
    CheckFlips_bitboard_b1,
    CheckFlips_bitboard_c1,
    CheckFlips_bitboard_d1,
    CheckFlips_bitboard_e1,
    CheckFlips_bitboard_f1,
    CheckFlips_bitboard_g1,
    CheckFlips_bitboard_h1,

    CheckFlips_bitboard_a2,
    CheckFlips_bitboard_b2,
    CheckFlips_bitboard_c2,
    CheckFlips_bitboard_d2,
    CheckFlips_bitboard_e2,
    CheckFlips_bitboard_f2,
    CheckFlips_bitboard_g2,
    CheckFlips_bitboard_h2,

    CheckFlips_bitboard_a3,
    CheckFlips_bitboard_b3,
    CheckFlips_bitboard_c3,
    CheckFlips_bitboard_d3,
    CheckFlips_bitboard_e3,
    CheckFlips_bitboard_f3,
    CheckFlips_bitboard_g3,
    CheckFlips_bitboard_h3,

    CheckFlips_bitboard_a4,
    CheckFlips_bitboard_b4,
    CheckFlips_bitboard_c4,
    CheckFlips_bitboard_d4,
    CheckFlips_bitboard_e4,
    CheckFlips_bitboard_f4,
    CheckFlips_bitboard_g4,
    CheckFlips_bitboard_h4,

    CheckFlips_bitboard_a5,
    CheckFlips_bitboard_b5,
    CheckFlips_bitboard_c5,
    CheckFlips_bitboard_d5,
    CheckFlips_bitboard_e5,
    CheckFlips_bitboard_f5,
    CheckFlips_bitboard_g5,
    CheckFlips_bitboard_h5,

    CheckFlips_bitboard_a6,
    CheckFlips_bitboard_b6,
    CheckFlips_bitboard_c6,
    CheckFlips_bitboard_d6,
    CheckFlips_bitboard_e6,
    CheckFlips_bitboard_f6,
    CheckFlips_bitboard_g6,
    CheckFlips_bitboard_h6,

    CheckFlips_bitboard_a7,
    CheckFlips_bitboard_b7,
    CheckFlips_bitboard_c7,
    CheckFlips_bitboard_d7,
    CheckFlips_bitboard_e7,
    CheckFlips_bitboard_f7,
    CheckFlips_bitboard_g7,
    CheckFlips_bitboard_h7,

    CheckFlips_bitboard_a8,
    CheckFlips_bitboard_b8,
    CheckFlips_bitboard_c8,
    CheckFlips_bitboard_d8,
    CheckFlips_bitboard_e8,
    CheckFlips_bitboard_f8,
    CheckFlips_bitboard_g8,
    CheckFlips_bitboard_h8
};
