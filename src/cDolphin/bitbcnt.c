/*
   File:           bitbcnt.cpp

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about bitboard get the filps disc count.
*/

#include <stdlib.h>

#include "utils.h"
#include "bitboard.h"
#include "bitbcnt.h"

/*
  This block generated on Sun Feb  4 15:14:00 2001
*/

static int right_count[128] = {
    0, 0, 1, 0, 2, 0, 1, 0,
    3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0,
    3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0,
    3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0,
    3, 0, 1, 0, 2, 0, 1, 0,
    6, 0, 1, 0, 2, 0, 1, 0,
    3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0,
    3, 0, 1, 0, 2, 0, 1, 0,
    5, 0, 1, 0, 2, 0, 1, 0,
    3, 0, 1, 0, 2, 0, 1, 0,
    4, 0, 1, 0, 2, 0, 1, 0,
    3, 0, 1, 0, 2, 0, 1, 0
};

static int left_count[128] = {
    0, 6, 5, 5, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2,
    2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
};

static int
CountFlips_bitboard_a1(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 1) & 127];

    /* Down */
    if ((my_bits.low & 0x00000100ul) == 0) {
        if ((my_bits.low & 0x00010000ul) == 0) {
            if ((my_bits.low & 0x01000000ul) == 0) {
                if ((my_bits.high & 0x00000001ul) == 0) {
                    if ((my_bits.high & 0x00000100ul) == 0) {
                        if ((my_bits.high & 0x00010000ul) == 0) {
                            if ((my_bits.high & 0x01000000ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.low & 0x00000200ul) == 0) {
        if ((my_bits.low & 0x00040000ul) == 0) {
            if ((my_bits.low & 0x08000000ul) == 0) {
                if ((my_bits.high & 0x00000010ul) == 0) {
                    if ((my_bits.high & 0x00002000ul) == 0) {
                        if ((my_bits.high & 0x00400000ul) == 0) {
                            if ((my_bits.high & 0x80000000ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_h1(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.low << 0) & 127];

    /* Down left */
    if ((my_bits.low & 0x00004000ul) == 0) {
        if ((my_bits.low & 0x00200000ul) == 0) {
            if ((my_bits.low & 0x10000000ul) == 0) {
                if ((my_bits.high & 0x00000008ul) == 0) {
                    if ((my_bits.high & 0x00000400ul) == 0) {
                        if ((my_bits.high & 0x00020000ul) == 0) {
                            if ((my_bits.high & 0x01000000ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.low & 0x00008000ul) == 0) {
        if ((my_bits.low & 0x00800000ul) == 0) {
            if ((my_bits.low & 0x80000000ul) == 0) {
                if ((my_bits.high & 0x00000080ul) == 0) {
                    if ((my_bits.high & 0x00008000ul) == 0) {
                        if ((my_bits.high & 0x00800000ul) == 0) {
                            if ((my_bits.high & 0x80000000ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_a8(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 25) & 127];

    /* Up right */
    if ((my_bits.high & 0x00020000ul) == 0) {
        if ((my_bits.high & 0x00000400ul) == 0) {
            if ((my_bits.high & 0x00000008ul) == 0) {
                if ((my_bits.low & 0x10000000ul) == 0) {
                    if ((my_bits.low & 0x00200000ul) == 0) {
                        if ((my_bits.low & 0x00004000ul) == 0) {
                            if ((my_bits.low & 0x00000080ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.high & 0x00010000ul) == 0) {
        if ((my_bits.high & 0x00000100ul) == 0) {
            if ((my_bits.high & 0x00000001ul) == 0) {
                if ((my_bits.low & 0x01000000ul) == 0) {
                    if ((my_bits.low & 0x00010000ul) == 0) {
                        if ((my_bits.low & 0x00000100ul) == 0) {
                            if ((my_bits.low & 0x00000001ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_h8(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.high & 2130706432) >> 24];

    /* Up */
    if ((my_bits.high & 0x00800000ul) == 0) {
        if ((my_bits.high & 0x00008000ul) == 0) {
            if ((my_bits.high & 0x00000080ul) == 0) {
                if ((my_bits.low & 0x80000000ul) == 0) {
                    if ((my_bits.low & 0x00800000ul) == 0) {
                        if ((my_bits.low & 0x00008000ul) == 0) {
                            if ((my_bits.low & 0x00000080ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00400000ul) == 0) {
        if ((my_bits.high & 0x00002000ul) == 0) {
            if ((my_bits.high & 0x00000010ul) == 0) {
                if ((my_bits.low & 0x08000000ul) == 0) {
                    if ((my_bits.low & 0x00040000ul) == 0) {
                        if ((my_bits.low & 0x00000200ul) == 0) {
                            if ((my_bits.low & 0x00000001ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_b1(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 2) & 63];

    /* Down */
    if ((my_bits.low & 0x00000200ul) == 0) {
        if ((my_bits.low & 0x00020000ul) == 0) {
            if ((my_bits.low & 0x02000000ul) == 0) {
                if ((my_bits.high & 0x00000002ul) == 0) {
                    if ((my_bits.high & 0x00000200ul) == 0) {
                        if ((my_bits.high & 0x00020000ul) == 0) {
                            if ((my_bits.high & 0x02000000ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.low & 0x00000400ul) == 0) {
        if ((my_bits.low & 0x00080000ul) == 0) {
            if ((my_bits.low & 0x10000000ul) == 0) {
                if ((my_bits.high & 0x00000020ul) == 0) {
                    if ((my_bits.high & 0x00004000ul) == 0) {
                        if ((my_bits.high & 0x00800000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_g1(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.low << 1) & 127];

    /* Down left */
    if ((my_bits.low & 0x00002000ul) == 0) {
        if ((my_bits.low & 0x00100000ul) == 0) {
            if ((my_bits.low & 0x08000000ul) == 0) {
                if ((my_bits.high & 0x00000004ul) == 0) {
                    if ((my_bits.high & 0x00000200ul) == 0) {
                        if ((my_bits.high & 0x00010000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.low & 0x00004000ul) == 0) {
        if ((my_bits.low & 0x00400000ul) == 0) {
            if ((my_bits.low & 0x40000000ul) == 0) {
                if ((my_bits.high & 0x00000040ul) == 0) {
                    if ((my_bits.high & 0x00004000ul) == 0) {
                        if ((my_bits.high & 0x00400000ul) == 0) {
                            if ((my_bits.high & 0x40000000ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_a2(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 9) & 127];

    /* Down */
    if ((my_bits.low & 0x00010000ul) == 0) {
        if ((my_bits.low & 0x01000000ul) == 0) {
            if ((my_bits.high & 0x00000001ul) == 0) {
                if ((my_bits.high & 0x00000100ul) == 0) {
                    if ((my_bits.high & 0x00010000ul) == 0) {
                        if ((my_bits.high & 0x01000000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.low & 0x00020000ul) == 0) {
        if ((my_bits.low & 0x04000000ul) == 0) {
            if ((my_bits.high & 0x00000008ul) == 0) {
                if ((my_bits.high & 0x00001000ul) == 0) {
                    if ((my_bits.high & 0x00200000ul) == 0) {
                        if ((my_bits.high & 0x40000000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_h2(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.low & 32512) >> 8];

    /* Down left */
    if ((my_bits.low & 0x00400000ul) == 0) {
        if ((my_bits.low & 0x20000000ul) == 0) {
            if ((my_bits.high & 0x00000010ul) == 0) {
                if ((my_bits.high & 0x00000800ul) == 0) {
                    if ((my_bits.high & 0x00040000ul) == 0) {
                        if ((my_bits.high & 0x02000000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.low & 0x00800000ul) == 0) {
        if ((my_bits.low & 0x80000000ul) == 0) {
            if ((my_bits.high & 0x00000080ul) == 0) {
                if ((my_bits.high & 0x00008000ul) == 0) {
                    if ((my_bits.high & 0x00800000ul) == 0) {
                        if ((my_bits.high & 0x80000000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_a7(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 17) & 127];

    /* Up right */
    if ((my_bits.high & 0x00000200ul) == 0) {
        if ((my_bits.high & 0x00000004ul) == 0) {
            if ((my_bits.low & 0x08000000ul) == 0) {
                if ((my_bits.low & 0x00100000ul) == 0) {
                    if ((my_bits.low & 0x00002000ul) == 0) {
                        if ((my_bits.low & 0x00000040ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.high & 0x00000100ul) == 0) {
        if ((my_bits.high & 0x00000001ul) == 0) {
            if ((my_bits.low & 0x01000000ul) == 0) {
                if ((my_bits.low & 0x00010000ul) == 0) {
                    if ((my_bits.low & 0x00000100ul) == 0) {
                        if ((my_bits.low & 0x00000001ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_h7(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.high & 8323072) >> 16];

    /* Up */
    if ((my_bits.high & 0x00008000ul) == 0) {
        if ((my_bits.high & 0x00000080ul) == 0) {
            if ((my_bits.low & 0x80000000ul) == 0) {
                if ((my_bits.low & 0x00800000ul) == 0) {
                    if ((my_bits.low & 0x00008000ul) == 0) {
                        if ((my_bits.low & 0x00000080ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00004000ul) == 0) {
        if ((my_bits.high & 0x00000020ul) == 0) {
            if ((my_bits.low & 0x10000000ul) == 0) {
                if ((my_bits.low & 0x00080000ul) == 0) {
                    if ((my_bits.low & 0x00000400ul) == 0) {
                        if ((my_bits.low & 0x00000002ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_b8(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 26) & 63];

    /* Up right */
    if ((my_bits.high & 0x00040000ul) == 0) {
        if ((my_bits.high & 0x00000800ul) == 0) {
            if ((my_bits.high & 0x00000010ul) == 0) {
                if ((my_bits.low & 0x20000000ul) == 0) {
                    if ((my_bits.low & 0x00400000ul) == 0) {
                        if ((my_bits.low & 0x00008000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.high & 0x00020000ul) == 0) {
        if ((my_bits.high & 0x00000200ul) == 0) {
            if ((my_bits.high & 0x00000002ul) == 0) {
                if ((my_bits.low & 0x02000000ul) == 0) {
                    if ((my_bits.low & 0x00020000ul) == 0) {
                        if ((my_bits.low & 0x00000200ul) == 0) {
                            if ((my_bits.low & 0x00000002ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_g8(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.high & 1056964608) >> 23];

    /* Up */
    if ((my_bits.high & 0x00400000ul) == 0) {
        if ((my_bits.high & 0x00004000ul) == 0) {
            if ((my_bits.high & 0x00000040ul) == 0) {
                if ((my_bits.low & 0x40000000ul) == 0) {
                    if ((my_bits.low & 0x00400000ul) == 0) {
                        if ((my_bits.low & 0x00004000ul) == 0) {
                            if ((my_bits.low & 0x00000040ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00200000ul) == 0) {
        if ((my_bits.high & 0x00001000ul) == 0) {
            if ((my_bits.high & 0x00000008ul) == 0) {
                if ((my_bits.low & 0x04000000ul) == 0) {
                    if ((my_bits.low & 0x00020000ul) == 0) {
                        if ((my_bits.low & 0x00000100ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_b2(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 10) & 63];

    /* Down */
    if ((my_bits.low & 0x00020000ul) == 0) {
        if ((my_bits.low & 0x02000000ul) == 0) {
            if ((my_bits.high & 0x00000002ul) == 0) {
                if ((my_bits.high & 0x00000200ul) == 0) {
                    if ((my_bits.high & 0x00020000ul) == 0) {
                        if ((my_bits.high & 0x02000000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.low & 0x00040000ul) == 0) {
        if ((my_bits.low & 0x08000000ul) == 0) {
            if ((my_bits.high & 0x00000010ul) == 0) {
                if ((my_bits.high & 0x00002000ul) == 0) {
                    if ((my_bits.high & 0x00400000ul) == 0) {
                        if ((my_bits.high & 0x80000000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_g2(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.low & 16128) >> 7];

    /* Down left */
    if ((my_bits.low & 0x00200000ul) == 0) {
        if ((my_bits.low & 0x10000000ul) == 0) {
            if ((my_bits.high & 0x00000008ul) == 0) {
                if ((my_bits.high & 0x00000400ul) == 0) {
                    if ((my_bits.high & 0x00020000ul) == 0) {
                        if ((my_bits.high & 0x01000000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.low & 0x00400000ul) == 0) {
        if ((my_bits.low & 0x40000000ul) == 0) {
            if ((my_bits.high & 0x00000040ul) == 0) {
                if ((my_bits.high & 0x00004000ul) == 0) {
                    if ((my_bits.high & 0x00400000ul) == 0) {
                        if ((my_bits.high & 0x40000000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_b7(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 18) & 63];

    /* Up right */
    if ((my_bits.high & 0x00000400ul) == 0) {
        if ((my_bits.high & 0x00000008ul) == 0) {
            if ((my_bits.low & 0x10000000ul) == 0) {
                if ((my_bits.low & 0x00200000ul) == 0) {
                    if ((my_bits.low & 0x00004000ul) == 0) {
                        if ((my_bits.low & 0x00000080ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.high & 0x00000200ul) == 0) {
        if ((my_bits.high & 0x00000002ul) == 0) {
            if ((my_bits.low & 0x02000000ul) == 0) {
                if ((my_bits.low & 0x00020000ul) == 0) {
                    if ((my_bits.low & 0x00000200ul) == 0) {
                        if ((my_bits.low & 0x00000002ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_g7(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.high & 4128768) >> 15];

    /* Up */
    if ((my_bits.high & 0x00004000ul) == 0) {
        if ((my_bits.high & 0x00000040ul) == 0) {
            if ((my_bits.low & 0x40000000ul) == 0) {
                if ((my_bits.low & 0x00400000ul) == 0) {
                    if ((my_bits.low & 0x00004000ul) == 0) {
                        if ((my_bits.low & 0x00000040ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00002000ul) == 0) {
        if ((my_bits.high & 0x00000010ul) == 0) {
            if ((my_bits.low & 0x08000000ul) == 0) {
                if ((my_bits.low & 0x00040000ul) == 0) {
                    if ((my_bits.low & 0x00000200ul) == 0) {
                        if ((my_bits.low & 0x00000001ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_c1(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 3) & 31];

    /* Left */
    flipped += left_count[(my_bits.low << 5) & 127];

    /* Down left */
    if ((my_bits.low & 0x00000200ul) == 0) {
        if ((my_bits.low & 0x00010000ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.low & 0x00000400ul) == 0) {
        if ((my_bits.low & 0x00040000ul) == 0) {
            if ((my_bits.low & 0x04000000ul) == 0) {
                if ((my_bits.high & 0x00000004ul) == 0) {
                    if ((my_bits.high & 0x00000400ul) == 0) {
                        if ((my_bits.high & 0x00040000ul) == 0) {
                            if ((my_bits.high & 0x04000000ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.low & 0x00000800ul) == 0) {
        if ((my_bits.low & 0x00100000ul) == 0) {
            if ((my_bits.low & 0x20000000ul) == 0) {
                if ((my_bits.high & 0x00000040ul) == 0) {
                    if ((my_bits.high & 0x00008000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_f1(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 6) & 3];

    /* Left */
    flipped += left_count[(my_bits.low << 2) & 127];

    /* Down left */
    if ((my_bits.low & 0x00001000ul) == 0) {
        if ((my_bits.low & 0x00080000ul) == 0) {
            if ((my_bits.low & 0x04000000ul) == 0) {
                if ((my_bits.high & 0x00000002ul) == 0) {
                    if ((my_bits.high & 0x00000100ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.low & 0x00002000ul) == 0) {
        if ((my_bits.low & 0x00200000ul) == 0) {
            if ((my_bits.low & 0x20000000ul) == 0) {
                if ((my_bits.high & 0x00000020ul) == 0) {
                    if ((my_bits.high & 0x00002000ul) == 0) {
                        if ((my_bits.high & 0x00200000ul) == 0) {
                            if ((my_bits.high & 0x20000000ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.low & 0x00004000ul) == 0) {
        if ((my_bits.low & 0x00800000ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_a3(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 17) & 127];

    /* Up right */
    if ((my_bits.low & 0x00000200ul) == 0) {
        if ((my_bits.low & 0x00000004ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.low & 0x01000000ul) == 0) {
        if ((my_bits.high & 0x00000001ul) == 0) {
            if ((my_bits.high & 0x00000100ul) == 0) {
                if ((my_bits.high & 0x00010000ul) == 0) {
                    if ((my_bits.high & 0x01000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00000100ul) == 0) {
        if ((my_bits.low & 0x00000001ul) != 0)
            flipped += 1;
    }
    /* Down right */
    if ((my_bits.low & 0x02000000ul) == 0) {
        if ((my_bits.high & 0x00000004ul) == 0) {
            if ((my_bits.high & 0x00000800ul) == 0) {
                if ((my_bits.high & 0x00100000ul) == 0) {
                    if ((my_bits.high & 0x20000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_h3(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.low & 8323072) >> 16];

    /* Down left */
    if ((my_bits.low & 0x40000000ul) == 0) {
        if ((my_bits.high & 0x00000020ul) == 0) {
            if ((my_bits.high & 0x00001000ul) == 0) {
                if ((my_bits.high & 0x00080000ul) == 0) {
                    if ((my_bits.high & 0x04000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.low & 0x80000000ul) == 0) {
        if ((my_bits.high & 0x00000080ul) == 0) {
            if ((my_bits.high & 0x00008000ul) == 0) {
                if ((my_bits.high & 0x00800000ul) == 0) {
                    if ((my_bits.high & 0x80000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00008000ul) == 0) {
        if ((my_bits.low & 0x00000080ul) != 0)
            flipped += 1;
    }
    /* Up left */
    if ((my_bits.low & 0x00004000ul) == 0) {
        if ((my_bits.low & 0x00000020ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_a6(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 9) & 127];

    /* Up right */
    if ((my_bits.high & 0x00000002ul) == 0) {
        if ((my_bits.low & 0x04000000ul) == 0) {
            if ((my_bits.low & 0x00080000ul) == 0) {
                if ((my_bits.low & 0x00001000ul) == 0) {
                    if ((my_bits.low & 0x00000020ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00010000ul) == 0) {
        if ((my_bits.high & 0x01000000ul) != 0)
            flipped += 1;
    }
    /* Up */
    if ((my_bits.high & 0x00000001ul) == 0) {
        if ((my_bits.low & 0x01000000ul) == 0) {
            if ((my_bits.low & 0x00010000ul) == 0) {
                if ((my_bits.low & 0x00000100ul) == 0) {
                    if ((my_bits.low & 0x00000001ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00020000ul) == 0) {
        if ((my_bits.high & 0x04000000ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_h6(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.high & 32512) >> 8];

    /* Down left */
    if ((my_bits.high & 0x00400000ul) == 0) {
        if ((my_bits.high & 0x20000000ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.high & 0x00800000ul) == 0) {
        if ((my_bits.high & 0x80000000ul) != 0)
            flipped += 1;
    }
    /* Up */
    if ((my_bits.high & 0x00000080ul) == 0) {
        if ((my_bits.low & 0x80000000ul) == 0) {
            if ((my_bits.low & 0x00800000ul) == 0) {
                if ((my_bits.low & 0x00008000ul) == 0) {
                    if ((my_bits.low & 0x00000080ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00000040ul) == 0) {
        if ((my_bits.low & 0x20000000ul) == 0) {
            if ((my_bits.low & 0x00100000ul) == 0) {
                if ((my_bits.low & 0x00000800ul) == 0) {
                    if ((my_bits.low & 0x00000004ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_c8(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 27) & 31];

    /* Left */
    flipped += left_count[(my_bits.high & 50331648) >> 19];

    /* Up right */
    if ((my_bits.high & 0x00080000ul) == 0) {
        if ((my_bits.high & 0x00001000ul) == 0) {
            if ((my_bits.high & 0x00000020ul) == 0) {
                if ((my_bits.low & 0x40000000ul) == 0) {
                    if ((my_bits.low & 0x00800000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.high & 0x00040000ul) == 0) {
        if ((my_bits.high & 0x00000400ul) == 0) {
            if ((my_bits.high & 0x00000004ul) == 0) {
                if ((my_bits.low & 0x04000000ul) == 0) {
                    if ((my_bits.low & 0x00040000ul) == 0) {
                        if ((my_bits.low & 0x00000400ul) == 0) {
                            if ((my_bits.low & 0x00000004ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00020000ul) == 0) {
        if ((my_bits.high & 0x00000100ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_f8(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 30) & 3];

    /* Left */
    flipped += left_count[(my_bits.high & 520093696) >> 22];

    /* Up right */
    if ((my_bits.high & 0x00400000ul) == 0) {
        if ((my_bits.high & 0x00008000ul) != 0)
            flipped += 1;
    }
    /* Up */
    if ((my_bits.high & 0x00200000ul) == 0) {
        if ((my_bits.high & 0x00002000ul) == 0) {
            if ((my_bits.high & 0x00000020ul) == 0) {
                if ((my_bits.low & 0x20000000ul) == 0) {
                    if ((my_bits.low & 0x00200000ul) == 0) {
                        if ((my_bits.low & 0x00002000ul) == 0) {
                            if ((my_bits.low & 0x00000020ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00100000ul) == 0) {
        if ((my_bits.high & 0x00000800ul) == 0) {
            if ((my_bits.high & 0x00000004ul) == 0) {
                if ((my_bits.low & 0x02000000ul) == 0) {
                    if ((my_bits.low & 0x00010000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_d1(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 4) & 15];

    /* Left */
    flipped += left_count[(my_bits.low << 4) & 127];

    /* Down left */
    if ((my_bits.low & 0x00000400ul) == 0) {
        if ((my_bits.low & 0x00020000ul) == 0) {
            if ((my_bits.low & 0x01000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.low & 0x00000800ul) == 0) {
        if ((my_bits.low & 0x00080000ul) == 0) {
            if ((my_bits.low & 0x08000000ul) == 0) {
                if ((my_bits.high & 0x00000008ul) == 0) {
                    if ((my_bits.high & 0x00000800ul) == 0) {
                        if ((my_bits.high & 0x00080000ul) == 0) {
                            if ((my_bits.high & 0x08000000ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.low & 0x00001000ul) == 0) {
        if ((my_bits.low & 0x00200000ul) == 0) {
            if ((my_bits.low & 0x40000000ul) == 0) {
                if ((my_bits.high & 0x00000080ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_e1(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 5) & 7];

    /* Left */
    flipped += left_count[(my_bits.low << 3) & 127];

    /* Down left */
    if ((my_bits.low & 0x00000800ul) == 0) {
        if ((my_bits.low & 0x00040000ul) == 0) {
            if ((my_bits.low & 0x02000000ul) == 0) {
                if ((my_bits.high & 0x00000001ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.low & 0x00001000ul) == 0) {
        if ((my_bits.low & 0x00100000ul) == 0) {
            if ((my_bits.low & 0x10000000ul) == 0) {
                if ((my_bits.high & 0x00000010ul) == 0) {
                    if ((my_bits.high & 0x00001000ul) == 0) {
                        if ((my_bits.high & 0x00100000ul) == 0) {
                            if ((my_bits.high & 0x10000000ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.low & 0x00002000ul) == 0) {
        if ((my_bits.low & 0x00400000ul) == 0) {
            if ((my_bits.low & 0x80000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_a4(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 25) & 127];

    /* Up right */
    if ((my_bits.low & 0x00020000ul) == 0) {
        if ((my_bits.low & 0x00000400ul) == 0) {
            if ((my_bits.low & 0x00000008ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00000001ul) == 0) {
        if ((my_bits.high & 0x00000100ul) == 0) {
            if ((my_bits.high & 0x00010000ul) == 0) {
                if ((my_bits.high & 0x01000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00010000ul) == 0) {
        if ((my_bits.low & 0x00000100ul) == 0) {
            if ((my_bits.low & 0x00000001ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00000002ul) == 0) {
        if ((my_bits.high & 0x00000400ul) == 0) {
            if ((my_bits.high & 0x00080000ul) == 0) {
                if ((my_bits.high & 0x10000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_h4(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.low & 2130706432) >> 24];

    /* Down left */
    if ((my_bits.high & 0x00000040ul) == 0) {
        if ((my_bits.high & 0x00002000ul) == 0) {
            if ((my_bits.high & 0x00100000ul) == 0) {
                if ((my_bits.high & 0x08000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00000080ul) == 0) {
        if ((my_bits.high & 0x00008000ul) == 0) {
            if ((my_bits.high & 0x00800000ul) == 0) {
                if ((my_bits.high & 0x80000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00800000ul) == 0) {
        if ((my_bits.low & 0x00008000ul) == 0) {
            if ((my_bits.low & 0x00000080ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x00400000ul) == 0) {
        if ((my_bits.low & 0x00002000ul) == 0) {
            if ((my_bits.low & 0x00000010ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_a5(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 1) & 127];

    /* Up right */
    if ((my_bits.low & 0x02000000ul) == 0) {
        if ((my_bits.low & 0x00040000ul) == 0) {
            if ((my_bits.low & 0x00000800ul) == 0) {
                if ((my_bits.low & 0x00000010ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00000100ul) == 0) {
        if ((my_bits.high & 0x00010000ul) == 0) {
            if ((my_bits.high & 0x01000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x01000000ul) == 0) {
        if ((my_bits.low & 0x00010000ul) == 0) {
            if ((my_bits.low & 0x00000100ul) == 0) {
                if ((my_bits.low & 0x00000001ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00000200ul) == 0) {
        if ((my_bits.high & 0x00040000ul) == 0) {
            if ((my_bits.high & 0x08000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_h5(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.high << 0) & 127];

    /* Down left */
    if ((my_bits.high & 0x00004000ul) == 0) {
        if ((my_bits.high & 0x00200000ul) == 0) {
            if ((my_bits.high & 0x10000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00008000ul) == 0) {
        if ((my_bits.high & 0x00800000ul) == 0) {
            if ((my_bits.high & 0x80000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x80000000ul) == 0) {
        if ((my_bits.low & 0x00800000ul) == 0) {
            if ((my_bits.low & 0x00008000ul) == 0) {
                if ((my_bits.low & 0x00000080ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x40000000ul) == 0) {
        if ((my_bits.low & 0x00200000ul) == 0) {
            if ((my_bits.low & 0x00001000ul) == 0) {
                if ((my_bits.low & 0x00000008ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_d8(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 28) & 15];

    /* Left */
    flipped += left_count[(my_bits.high & 117440512) >> 20];

    /* Up right */
    if ((my_bits.high & 0x00100000ul) == 0) {
        if ((my_bits.high & 0x00002000ul) == 0) {
            if ((my_bits.high & 0x00000040ul) == 0) {
                if ((my_bits.low & 0x80000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.high & 0x00080000ul) == 0) {
        if ((my_bits.high & 0x00000800ul) == 0) {
            if ((my_bits.high & 0x00000008ul) == 0) {
                if ((my_bits.low & 0x08000000ul) == 0) {
                    if ((my_bits.low & 0x00080000ul) == 0) {
                        if ((my_bits.low & 0x00000800ul) == 0) {
                            if ((my_bits.low & 0x00000008ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00040000ul) == 0) {
        if ((my_bits.high & 0x00000200ul) == 0) {
            if ((my_bits.high & 0x00000001ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_e8(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 29) & 7];

    /* Left */
    flipped += left_count[(my_bits.high & 251658240) >> 21];

    /* Up right */
    if ((my_bits.high & 0x00200000ul) == 0) {
        if ((my_bits.high & 0x00004000ul) == 0) {
            if ((my_bits.high & 0x00000080ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.high & 0x00100000ul) == 0) {
        if ((my_bits.high & 0x00001000ul) == 0) {
            if ((my_bits.high & 0x00000010ul) == 0) {
                if ((my_bits.low & 0x10000000ul) == 0) {
                    if ((my_bits.low & 0x00100000ul) == 0) {
                        if ((my_bits.low & 0x00001000ul) == 0) {
                            if ((my_bits.low & 0x00000010ul) != 0)
                                flipped += 6;
                        }
                        else {
                            flipped += 5;
                        }
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00080000ul) == 0) {
        if ((my_bits.high & 0x00000400ul) == 0) {
            if ((my_bits.high & 0x00000002ul) == 0) {
                if ((my_bits.low & 0x01000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_c2(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 11) & 31];

    /* Left */
    flipped += left_count[(my_bits.low & 768) >> 3];

    /* Down left */
    if ((my_bits.low & 0x00020000ul) == 0) {
        if ((my_bits.low & 0x01000000ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.low & 0x00040000ul) == 0) {
        if ((my_bits.low & 0x04000000ul) == 0) {
            if ((my_bits.high & 0x00000004ul) == 0) {
                if ((my_bits.high & 0x00000400ul) == 0) {
                    if ((my_bits.high & 0x00040000ul) == 0) {
                        if ((my_bits.high & 0x04000000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.low & 0x00080000ul) == 0) {
        if ((my_bits.low & 0x10000000ul) == 0) {
            if ((my_bits.high & 0x00000020ul) == 0) {
                if ((my_bits.high & 0x00004000ul) == 0) {
                    if ((my_bits.high & 0x00800000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_f2(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 14) & 3];

    /* Left */
    flipped += left_count[(my_bits.low & 7936) >> 6];

    /* Down left */
    if ((my_bits.low & 0x00100000ul) == 0) {
        if ((my_bits.low & 0x08000000ul) == 0) {
            if ((my_bits.high & 0x00000004ul) == 0) {
                if ((my_bits.high & 0x00000200ul) == 0) {
                    if ((my_bits.high & 0x00010000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.low & 0x00200000ul) == 0) {
        if ((my_bits.low & 0x20000000ul) == 0) {
            if ((my_bits.high & 0x00000020ul) == 0) {
                if ((my_bits.high & 0x00002000ul) == 0) {
                    if ((my_bits.high & 0x00200000ul) == 0) {
                        if ((my_bits.high & 0x20000000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.low & 0x00400000ul) == 0) {
        if ((my_bits.low & 0x80000000ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_b3(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 18) & 63];

    /* Up right */
    if ((my_bits.low & 0x00000400ul) == 0) {
        if ((my_bits.low & 0x00000008ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.low & 0x02000000ul) == 0) {
        if ((my_bits.high & 0x00000002ul) == 0) {
            if ((my_bits.high & 0x00000200ul) == 0) {
                if ((my_bits.high & 0x00020000ul) == 0) {
                    if ((my_bits.high & 0x02000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00000200ul) == 0) {
        if ((my_bits.low & 0x00000002ul) != 0)
            flipped += 1;
    }
    /* Down right */
    if ((my_bits.low & 0x04000000ul) == 0) {
        if ((my_bits.high & 0x00000008ul) == 0) {
            if ((my_bits.high & 0x00001000ul) == 0) {
                if ((my_bits.high & 0x00200000ul) == 0) {
                    if ((my_bits.high & 0x40000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_g3(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.low & 4128768) >> 15];

    /* Down left */
    if ((my_bits.low & 0x20000000ul) == 0) {
        if ((my_bits.high & 0x00000010ul) == 0) {
            if ((my_bits.high & 0x00000800ul) == 0) {
                if ((my_bits.high & 0x00040000ul) == 0) {
                    if ((my_bits.high & 0x02000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.low & 0x40000000ul) == 0) {
        if ((my_bits.high & 0x00000040ul) == 0) {
            if ((my_bits.high & 0x00004000ul) == 0) {
                if ((my_bits.high & 0x00400000ul) == 0) {
                    if ((my_bits.high & 0x40000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00004000ul) == 0) {
        if ((my_bits.low & 0x00000040ul) != 0)
            flipped += 1;
    }
    /* Up left */
    if ((my_bits.low & 0x00002000ul) == 0) {
        if ((my_bits.low & 0x00000010ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_b6(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 10) & 63];

    /* Up right */
    if ((my_bits.high & 0x00000004ul) == 0) {
        if ((my_bits.low & 0x08000000ul) == 0) {
            if ((my_bits.low & 0x00100000ul) == 0) {
                if ((my_bits.low & 0x00002000ul) == 0) {
                    if ((my_bits.low & 0x00000040ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00020000ul) == 0) {
        if ((my_bits.high & 0x02000000ul) != 0)
            flipped += 1;
    }
    /* Up */
    if ((my_bits.high & 0x00000002ul) == 0) {
        if ((my_bits.low & 0x02000000ul) == 0) {
            if ((my_bits.low & 0x00020000ul) == 0) {
                if ((my_bits.low & 0x00000200ul) == 0) {
                    if ((my_bits.low & 0x00000002ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00040000ul) == 0) {
        if ((my_bits.high & 0x08000000ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_g6(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.high & 16128) >> 7];

    /* Down left */
    if ((my_bits.high & 0x00200000ul) == 0) {
        if ((my_bits.high & 0x10000000ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.high & 0x00400000ul) == 0) {
        if ((my_bits.high & 0x40000000ul) != 0)
            flipped += 1;
    }
    /* Up */
    if ((my_bits.high & 0x00000040ul) == 0) {
        if ((my_bits.low & 0x40000000ul) == 0) {
            if ((my_bits.low & 0x00400000ul) == 0) {
                if ((my_bits.low & 0x00004000ul) == 0) {
                    if ((my_bits.low & 0x00000040ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00000020ul) == 0) {
        if ((my_bits.low & 0x10000000ul) == 0) {
            if ((my_bits.low & 0x00080000ul) == 0) {
                if ((my_bits.low & 0x00000400ul) == 0) {
                    if ((my_bits.low & 0x00000002ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_c7(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 19) & 31];

    /* Left */
    flipped += left_count[(my_bits.high & 196608) >> 11];

    /* Up right */
    if ((my_bits.high & 0x00000800ul) == 0) {
        if ((my_bits.high & 0x00000010ul) == 0) {
            if ((my_bits.low & 0x20000000ul) == 0) {
                if ((my_bits.low & 0x00400000ul) == 0) {
                    if ((my_bits.low & 0x00008000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.high & 0x00000400ul) == 0) {
        if ((my_bits.high & 0x00000004ul) == 0) {
            if ((my_bits.low & 0x04000000ul) == 0) {
                if ((my_bits.low & 0x00040000ul) == 0) {
                    if ((my_bits.low & 0x00000400ul) == 0) {
                        if ((my_bits.low & 0x00000004ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00000200ul) == 0) {
        if ((my_bits.high & 0x00000001ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_f7(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 22) & 3];

    /* Left */
    flipped += left_count[(my_bits.high & 2031616) >> 14];

    /* Up right */
    if ((my_bits.high & 0x00004000ul) == 0) {
        if ((my_bits.high & 0x00000080ul) != 0)
            flipped += 1;
    }
    /* Up */
    if ((my_bits.high & 0x00002000ul) == 0) {
        if ((my_bits.high & 0x00000020ul) == 0) {
            if ((my_bits.low & 0x20000000ul) == 0) {
                if ((my_bits.low & 0x00200000ul) == 0) {
                    if ((my_bits.low & 0x00002000ul) == 0) {
                        if ((my_bits.low & 0x00000020ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00001000ul) == 0) {
        if ((my_bits.high & 0x00000008ul) == 0) {
            if ((my_bits.low & 0x04000000ul) == 0) {
                if ((my_bits.low & 0x00020000ul) == 0) {
                    if ((my_bits.low & 0x00000100ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_d2(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 12) & 15];

    /* Left */
    flipped += left_count[(my_bits.low & 1792) >> 4];

    /* Down left */
    if ((my_bits.low & 0x00040000ul) == 0) {
        if ((my_bits.low & 0x02000000ul) == 0) {
            if ((my_bits.high & 0x00000001ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.low & 0x00080000ul) == 0) {
        if ((my_bits.low & 0x08000000ul) == 0) {
            if ((my_bits.high & 0x00000008ul) == 0) {
                if ((my_bits.high & 0x00000800ul) == 0) {
                    if ((my_bits.high & 0x00080000ul) == 0) {
                        if ((my_bits.high & 0x08000000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.low & 0x00100000ul) == 0) {
        if ((my_bits.low & 0x20000000ul) == 0) {
            if ((my_bits.high & 0x00000040ul) == 0) {
                if ((my_bits.high & 0x00008000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_e2(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 13) & 7];

    /* Left */
    flipped += left_count[(my_bits.low & 3840) >> 5];

    /* Down left */
    if ((my_bits.low & 0x00080000ul) == 0) {
        if ((my_bits.low & 0x04000000ul) == 0) {
            if ((my_bits.high & 0x00000002ul) == 0) {
                if ((my_bits.high & 0x00000100ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.low & 0x00100000ul) == 0) {
        if ((my_bits.low & 0x10000000ul) == 0) {
            if ((my_bits.high & 0x00000010ul) == 0) {
                if ((my_bits.high & 0x00001000ul) == 0) {
                    if ((my_bits.high & 0x00100000ul) == 0) {
                        if ((my_bits.high & 0x10000000ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.low & 0x00200000ul) == 0) {
        if ((my_bits.low & 0x40000000ul) == 0) {
            if ((my_bits.high & 0x00000080ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_b4(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 26) & 63];

    /* Up right */
    if ((my_bits.low & 0x00040000ul) == 0) {
        if ((my_bits.low & 0x00000800ul) == 0) {
            if ((my_bits.low & 0x00000010ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00000002ul) == 0) {
        if ((my_bits.high & 0x00000200ul) == 0) {
            if ((my_bits.high & 0x00020000ul) == 0) {
                if ((my_bits.high & 0x02000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00020000ul) == 0) {
        if ((my_bits.low & 0x00000200ul) == 0) {
            if ((my_bits.low & 0x00000002ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00000004ul) == 0) {
        if ((my_bits.high & 0x00000800ul) == 0) {
            if ((my_bits.high & 0x00100000ul) == 0) {
                if ((my_bits.high & 0x20000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_g4(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.low & 1056964608) >> 23];

    /* Down left */
    if ((my_bits.high & 0x00000020ul) == 0) {
        if ((my_bits.high & 0x00001000ul) == 0) {
            if ((my_bits.high & 0x00080000ul) == 0) {
                if ((my_bits.high & 0x04000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00000040ul) == 0) {
        if ((my_bits.high & 0x00004000ul) == 0) {
            if ((my_bits.high & 0x00400000ul) == 0) {
                if ((my_bits.high & 0x40000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00400000ul) == 0) {
        if ((my_bits.low & 0x00004000ul) == 0) {
            if ((my_bits.low & 0x00000040ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x00200000ul) == 0) {
        if ((my_bits.low & 0x00001000ul) == 0) {
            if ((my_bits.low & 0x00000008ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_b5(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 2) & 63];

    /* Up right */
    if ((my_bits.low & 0x04000000ul) == 0) {
        if ((my_bits.low & 0x00080000ul) == 0) {
            if ((my_bits.low & 0x00001000ul) == 0) {
                if ((my_bits.low & 0x00000020ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00000200ul) == 0) {
        if ((my_bits.high & 0x00020000ul) == 0) {
            if ((my_bits.high & 0x02000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x02000000ul) == 0) {
        if ((my_bits.low & 0x00020000ul) == 0) {
            if ((my_bits.low & 0x00000200ul) == 0) {
                if ((my_bits.low & 0x00000002ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00000400ul) == 0) {
        if ((my_bits.high & 0x00080000ul) == 0) {
            if ((my_bits.high & 0x10000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_g5(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Left */
    flipped += left_count[(my_bits.high << 1) & 127];

    /* Down left */
    if ((my_bits.high & 0x00002000ul) == 0) {
        if ((my_bits.high & 0x00100000ul) == 0) {
            if ((my_bits.high & 0x08000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00004000ul) == 0) {
        if ((my_bits.high & 0x00400000ul) == 0) {
            if ((my_bits.high & 0x40000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x40000000ul) == 0) {
        if ((my_bits.low & 0x00400000ul) == 0) {
            if ((my_bits.low & 0x00004000ul) == 0) {
                if ((my_bits.low & 0x00000040ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x20000000ul) == 0) {
        if ((my_bits.low & 0x00100000ul) == 0) {
            if ((my_bits.low & 0x00000800ul) == 0) {
                if ((my_bits.low & 0x00000004ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_d7(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 20) & 15];

    /* Left */
    flipped += left_count[(my_bits.high & 458752) >> 12];

    /* Up right */
    if ((my_bits.high & 0x00001000ul) == 0) {
        if ((my_bits.high & 0x00000020ul) == 0) {
            if ((my_bits.low & 0x40000000ul) == 0) {
                if ((my_bits.low & 0x00800000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.high & 0x00000800ul) == 0) {
        if ((my_bits.high & 0x00000008ul) == 0) {
            if ((my_bits.low & 0x08000000ul) == 0) {
                if ((my_bits.low & 0x00080000ul) == 0) {
                    if ((my_bits.low & 0x00000800ul) == 0) {
                        if ((my_bits.low & 0x00000008ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00000400ul) == 0) {
        if ((my_bits.high & 0x00000002ul) == 0) {
            if ((my_bits.low & 0x01000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_e7(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 21) & 7];

    /* Left */
    flipped += left_count[(my_bits.high & 983040) >> 13];

    /* Up right */
    if ((my_bits.high & 0x00002000ul) == 0) {
        if ((my_bits.high & 0x00000040ul) == 0) {
            if ((my_bits.low & 0x80000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.high & 0x00001000ul) == 0) {
        if ((my_bits.high & 0x00000010ul) == 0) {
            if ((my_bits.low & 0x10000000ul) == 0) {
                if ((my_bits.low & 0x00100000ul) == 0) {
                    if ((my_bits.low & 0x00001000ul) == 0) {
                        if ((my_bits.low & 0x00000010ul) != 0)
                            flipped += 5;
                    }
                    else {
                        flipped += 4;
                    }
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.high & 0x00000800ul) == 0) {
        if ((my_bits.high & 0x00000004ul) == 0) {
            if ((my_bits.low & 0x02000000ul) == 0) {
                if ((my_bits.low & 0x00010000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_c3(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 19) & 31];

    /* Left */
    flipped += left_count[(my_bits.low & 196608) >> 11];

    /* Down left */
    if ((my_bits.low & 0x02000000ul) == 0) {
        if ((my_bits.high & 0x00000001ul) != 0)
            flipped += 1;
    }
    /* Up right */
    if ((my_bits.low & 0x00000800ul) == 0) {
        if ((my_bits.low & 0x00000010ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.low & 0x04000000ul) == 0) {
        if ((my_bits.high & 0x00000004ul) == 0) {
            if ((my_bits.high & 0x00000400ul) == 0) {
                if ((my_bits.high & 0x00040000ul) == 0) {
                    if ((my_bits.high & 0x04000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00000400ul) == 0) {
        if ((my_bits.low & 0x00000004ul) != 0)
            flipped += 1;
    }
    /* Down right */
    if ((my_bits.low & 0x08000000ul) == 0) {
        if ((my_bits.high & 0x00000010ul) == 0) {
            if ((my_bits.high & 0x00002000ul) == 0) {
                if ((my_bits.high & 0x00400000ul) == 0) {
                    if ((my_bits.high & 0x80000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x00000200ul) == 0) {
        if ((my_bits.low & 0x00000001ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_f3(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 22) & 3];

    /* Left */
    flipped += left_count[(my_bits.low & 2031616) >> 14];

    /* Down left */
    if ((my_bits.low & 0x10000000ul) == 0) {
        if ((my_bits.high & 0x00000008ul) == 0) {
            if ((my_bits.high & 0x00000400ul) == 0) {
                if ((my_bits.high & 0x00020000ul) == 0) {
                    if ((my_bits.high & 0x01000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up right */
    if ((my_bits.low & 0x00004000ul) == 0) {
        if ((my_bits.low & 0x00000080ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.low & 0x20000000ul) == 0) {
        if ((my_bits.high & 0x00000020ul) == 0) {
            if ((my_bits.high & 0x00002000ul) == 0) {
                if ((my_bits.high & 0x00200000ul) == 0) {
                    if ((my_bits.high & 0x20000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00002000ul) == 0) {
        if ((my_bits.low & 0x00000020ul) != 0)
            flipped += 1;
    }
    /* Down right */
    if ((my_bits.low & 0x40000000ul) == 0) {
        if ((my_bits.high & 0x00000080ul) != 0)
            flipped += 1;
    }
    /* Up left */
    if ((my_bits.low & 0x00001000ul) == 0) {
        if ((my_bits.low & 0x00000008ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_c6(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 11) & 31];

    /* Left */
    flipped += left_count[(my_bits.high & 768) >> 3];

    /* Down left */
    if ((my_bits.high & 0x00020000ul) == 0) {
        if ((my_bits.high & 0x01000000ul) != 0)
            flipped += 1;
    }
    /* Up right */
    if ((my_bits.high & 0x00000008ul) == 0) {
        if ((my_bits.low & 0x10000000ul) == 0) {
            if ((my_bits.low & 0x00200000ul) == 0) {
                if ((my_bits.low & 0x00004000ul) == 0) {
                    if ((my_bits.low & 0x00000080ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00040000ul) == 0) {
        if ((my_bits.high & 0x04000000ul) != 0)
            flipped += 1;
    }
    /* Up */
    if ((my_bits.high & 0x00000004ul) == 0) {
        if ((my_bits.low & 0x04000000ul) == 0) {
            if ((my_bits.low & 0x00040000ul) == 0) {
                if ((my_bits.low & 0x00000400ul) == 0) {
                    if ((my_bits.low & 0x00000004ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00080000ul) == 0) {
        if ((my_bits.high & 0x10000000ul) != 0)
            flipped += 1;
    }
    /* Up left */
    if ((my_bits.high & 0x00000002ul) == 0) {
        if ((my_bits.low & 0x01000000ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_f6(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 14) & 3];

    /* Left */
    flipped += left_count[(my_bits.high & 7936) >> 6];

    /* Down left */
    if ((my_bits.high & 0x00100000ul) == 0) {
        if ((my_bits.high & 0x08000000ul) != 0)
            flipped += 1;
    }
    /* Up right */
    if ((my_bits.high & 0x00000040ul) == 0) {
        if ((my_bits.low & 0x80000000ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.high & 0x00200000ul) == 0) {
        if ((my_bits.high & 0x20000000ul) != 0)
            flipped += 1;
    }
    /* Up */
    if ((my_bits.high & 0x00000020ul) == 0) {
        if ((my_bits.low & 0x20000000ul) == 0) {
            if ((my_bits.low & 0x00200000ul) == 0) {
                if ((my_bits.low & 0x00002000ul) == 0) {
                    if ((my_bits.low & 0x00000020ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00400000ul) == 0) {
        if ((my_bits.high & 0x80000000ul) != 0)
            flipped += 1;
    }
    /* Up left */
    if ((my_bits.high & 0x00000010ul) == 0) {
        if ((my_bits.low & 0x08000000ul) == 0) {
            if ((my_bits.low & 0x00040000ul) == 0) {
                if ((my_bits.low & 0x00000200ul) == 0) {
                    if ((my_bits.low & 0x00000001ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_d3(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 20) & 15];

    /* Left */
    flipped += left_count[(my_bits.low & 458752) >> 12];

    /* Down left */
    if ((my_bits.low & 0x04000000ul) == 0) {
        if ((my_bits.high & 0x00000002ul) == 0) {
            if ((my_bits.high & 0x00000100ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up right */
    if ((my_bits.low & 0x00001000ul) == 0) {
        if ((my_bits.low & 0x00000020ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.low & 0x08000000ul) == 0) {
        if ((my_bits.high & 0x00000008ul) == 0) {
            if ((my_bits.high & 0x00000800ul) == 0) {
                if ((my_bits.high & 0x00080000ul) == 0) {
                    if ((my_bits.high & 0x08000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00000800ul) == 0) {
        if ((my_bits.low & 0x00000008ul) != 0)
            flipped += 1;
    }
    /* Down right */
    if ((my_bits.low & 0x10000000ul) == 0) {
        if ((my_bits.high & 0x00000020ul) == 0) {
            if ((my_bits.high & 0x00004000ul) == 0) {
                if ((my_bits.high & 0x00800000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x00000400ul) == 0) {
        if ((my_bits.low & 0x00000002ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_e3(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 21) & 7];

    /* Left */
    flipped += left_count[(my_bits.low & 983040) >> 13];

    /* Down left */
    if ((my_bits.low & 0x08000000ul) == 0) {
        if ((my_bits.high & 0x00000004ul) == 0) {
            if ((my_bits.high & 0x00000200ul) == 0) {
                if ((my_bits.high & 0x00010000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up right */
    if ((my_bits.low & 0x00002000ul) == 0) {
        if ((my_bits.low & 0x00000040ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.low & 0x10000000ul) == 0) {
        if ((my_bits.high & 0x00000010ul) == 0) {
            if ((my_bits.high & 0x00001000ul) == 0) {
                if ((my_bits.high & 0x00100000ul) == 0) {
                    if ((my_bits.high & 0x10000000ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00001000ul) == 0) {
        if ((my_bits.low & 0x00000010ul) != 0)
            flipped += 1;
    }
    /* Down right */
    if ((my_bits.low & 0x20000000ul) == 0) {
        if ((my_bits.high & 0x00000040ul) == 0) {
            if ((my_bits.high & 0x00008000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x00000800ul) == 0) {
        if ((my_bits.low & 0x00000004ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_c4(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 27) & 31];

    /* Left */
    flipped += left_count[(my_bits.low & 50331648) >> 19];

    /* Down left */
    if ((my_bits.high & 0x00000002ul) == 0) {
        if ((my_bits.high & 0x00000100ul) != 0)
            flipped += 1;
    }
    /* Up right */
    if ((my_bits.low & 0x00080000ul) == 0) {
        if ((my_bits.low & 0x00001000ul) == 0) {
            if ((my_bits.low & 0x00000020ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00000004ul) == 0) {
        if ((my_bits.high & 0x00000400ul) == 0) {
            if ((my_bits.high & 0x00040000ul) == 0) {
                if ((my_bits.high & 0x04000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00040000ul) == 0) {
        if ((my_bits.low & 0x00000400ul) == 0) {
            if ((my_bits.low & 0x00000004ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00000008ul) == 0) {
        if ((my_bits.high & 0x00001000ul) == 0) {
            if ((my_bits.high & 0x00200000ul) == 0) {
                if ((my_bits.high & 0x40000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x00020000ul) == 0) {
        if ((my_bits.low & 0x00000100ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_f4(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 30) & 3];

    /* Left */
    flipped += left_count[(my_bits.low & 520093696) >> 22];

    /* Down left */
    if ((my_bits.high & 0x00000010ul) == 0) {
        if ((my_bits.high & 0x00000800ul) == 0) {
            if ((my_bits.high & 0x00040000ul) == 0) {
                if ((my_bits.high & 0x02000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up right */
    if ((my_bits.low & 0x00400000ul) == 0) {
        if ((my_bits.low & 0x00008000ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.high & 0x00000020ul) == 0) {
        if ((my_bits.high & 0x00002000ul) == 0) {
            if ((my_bits.high & 0x00200000ul) == 0) {
                if ((my_bits.high & 0x20000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00200000ul) == 0) {
        if ((my_bits.low & 0x00002000ul) == 0) {
            if ((my_bits.low & 0x00000020ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00000040ul) == 0) {
        if ((my_bits.high & 0x00008000ul) != 0)
            flipped += 1;
    }
    /* Up left */
    if ((my_bits.low & 0x00100000ul) == 0) {
        if ((my_bits.low & 0x00000800ul) == 0) {
            if ((my_bits.low & 0x00000004ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_c5(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 3) & 31];

    /* Left */
    flipped += left_count[(my_bits.high << 5) & 127];

    /* Down left */
    if ((my_bits.high & 0x00000200ul) == 0) {
        if ((my_bits.high & 0x00010000ul) != 0)
            flipped += 1;
    }
    /* Up right */
    if ((my_bits.low & 0x08000000ul) == 0) {
        if ((my_bits.low & 0x00100000ul) == 0) {
            if ((my_bits.low & 0x00002000ul) == 0) {
                if ((my_bits.low & 0x00000040ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00000400ul) == 0) {
        if ((my_bits.high & 0x00040000ul) == 0) {
            if ((my_bits.high & 0x04000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x04000000ul) == 0) {
        if ((my_bits.low & 0x00040000ul) == 0) {
            if ((my_bits.low & 0x00000400ul) == 0) {
                if ((my_bits.low & 0x00000004ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00000800ul) == 0) {
        if ((my_bits.high & 0x00100000ul) == 0) {
            if ((my_bits.high & 0x20000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x02000000ul) == 0) {
        if ((my_bits.low & 0x00010000ul) != 0)
            flipped += 1;
    }

    return flipped;
}

static int
CountFlips_bitboard_f5(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 6) & 3];

    /* Left */
    flipped += left_count[(my_bits.high << 2) & 127];

    /* Down left */
    if ((my_bits.high & 0x00001000ul) == 0) {
        if ((my_bits.high & 0x00080000ul) == 0) {
            if ((my_bits.high & 0x04000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up right */
    if ((my_bits.low & 0x40000000ul) == 0) {
        if ((my_bits.low & 0x00800000ul) != 0)
            flipped += 1;
    }
    /* Down */
    if ((my_bits.high & 0x00002000ul) == 0) {
        if ((my_bits.high & 0x00200000ul) == 0) {
            if ((my_bits.high & 0x20000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x20000000ul) == 0) {
        if ((my_bits.low & 0x00200000ul) == 0) {
            if ((my_bits.low & 0x00002000ul) == 0) {
                if ((my_bits.low & 0x00000020ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00004000ul) == 0) {
        if ((my_bits.high & 0x00800000ul) != 0)
            flipped += 1;
    }
    /* Up left */
    if ((my_bits.low & 0x10000000ul) == 0) {
        if ((my_bits.low & 0x00080000ul) == 0) {
            if ((my_bits.low & 0x00000400ul) == 0) {
                if ((my_bits.low & 0x00000002ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_d6(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 12) & 15];

    /* Left */
    flipped += left_count[(my_bits.high & 1792) >> 4];

    /* Down left */
    if ((my_bits.high & 0x00040000ul) == 0) {
        if ((my_bits.high & 0x02000000ul) != 0)
            flipped += 1;
    }
    /* Up right */
    if ((my_bits.high & 0x00000010ul) == 0) {
        if ((my_bits.low & 0x20000000ul) == 0) {
            if ((my_bits.low & 0x00400000ul) == 0) {
                if ((my_bits.low & 0x00008000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00080000ul) == 0) {
        if ((my_bits.high & 0x08000000ul) != 0)
            flipped += 1;
    }
    /* Up */
    if ((my_bits.high & 0x00000008ul) == 0) {
        if ((my_bits.low & 0x08000000ul) == 0) {
            if ((my_bits.low & 0x00080000ul) == 0) {
                if ((my_bits.low & 0x00000800ul) == 0) {
                    if ((my_bits.low & 0x00000008ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00100000ul) == 0) {
        if ((my_bits.high & 0x20000000ul) != 0)
            flipped += 1;
    }
    /* Up left */
    if ((my_bits.high & 0x00000004ul) == 0) {
        if ((my_bits.low & 0x02000000ul) == 0) {
            if ((my_bits.low & 0x00010000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_e6(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 13) & 7];

    /* Left */
    flipped += left_count[(my_bits.high & 3840) >> 5];

    /* Down left */
    if ((my_bits.high & 0x00080000ul) == 0) {
        if ((my_bits.high & 0x04000000ul) != 0)
            flipped += 1;
    }
    /* Up right */
    if ((my_bits.high & 0x00000020ul) == 0) {
        if ((my_bits.low & 0x40000000ul) == 0) {
            if ((my_bits.low & 0x00800000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00100000ul) == 0) {
        if ((my_bits.high & 0x10000000ul) != 0)
            flipped += 1;
    }
    /* Up */
    if ((my_bits.high & 0x00000010ul) == 0) {
        if ((my_bits.low & 0x10000000ul) == 0) {
            if ((my_bits.low & 0x00100000ul) == 0) {
                if ((my_bits.low & 0x00001000ul) == 0) {
                    if ((my_bits.low & 0x00000010ul) != 0)
                        flipped += 4;
                }
                else {
                    flipped += 3;
                }
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00200000ul) == 0) {
        if ((my_bits.high & 0x40000000ul) != 0)
            flipped += 1;
    }
    /* Up left */
    if ((my_bits.high & 0x00000008ul) == 0) {
        if ((my_bits.low & 0x04000000ul) == 0) {
            if ((my_bits.low & 0x00020000ul) == 0) {
                if ((my_bits.low & 0x00000100ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_d4(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 28) & 15];

    /* Left */
    flipped += left_count[(my_bits.low & 117440512) >> 20];

    /* Down left */
    if ((my_bits.high & 0x00000004ul) == 0) {
        if ((my_bits.high & 0x00000200ul) == 0) {
            if ((my_bits.high & 0x00010000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up right */
    if ((my_bits.low & 0x00100000ul) == 0) {
        if ((my_bits.low & 0x00002000ul) == 0) {
            if ((my_bits.low & 0x00000040ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00000008ul) == 0) {
        if ((my_bits.high & 0x00000800ul) == 0) {
            if ((my_bits.high & 0x00080000ul) == 0) {
                if ((my_bits.high & 0x08000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00080000ul) == 0) {
        if ((my_bits.low & 0x00000800ul) == 0) {
            if ((my_bits.low & 0x00000008ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00000010ul) == 0) {
        if ((my_bits.high & 0x00002000ul) == 0) {
            if ((my_bits.high & 0x00400000ul) == 0) {
                if ((my_bits.high & 0x80000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x00040000ul) == 0) {
        if ((my_bits.low & 0x00000200ul) == 0) {
            if ((my_bits.low & 0x00000001ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_e4(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.low >> 29) & 7];

    /* Left */
    flipped += left_count[(my_bits.low & 251658240) >> 21];

    /* Down left */
    if ((my_bits.high & 0x00000008ul) == 0) {
        if ((my_bits.high & 0x00000400ul) == 0) {
            if ((my_bits.high & 0x00020000ul) == 0) {
                if ((my_bits.high & 0x01000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up right */
    if ((my_bits.low & 0x00200000ul) == 0) {
        if ((my_bits.low & 0x00004000ul) == 0) {
            if ((my_bits.low & 0x00000080ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00000010ul) == 0) {
        if ((my_bits.high & 0x00001000ul) == 0) {
            if ((my_bits.high & 0x00100000ul) == 0) {
                if ((my_bits.high & 0x10000000ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x00100000ul) == 0) {
        if ((my_bits.low & 0x00001000ul) == 0) {
            if ((my_bits.low & 0x00000010ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00000020ul) == 0) {
        if ((my_bits.high & 0x00004000ul) == 0) {
            if ((my_bits.high & 0x00800000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x00080000ul) == 0) {
        if ((my_bits.low & 0x00000400ul) == 0) {
            if ((my_bits.low & 0x00000002ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_d5(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 4) & 15];

    /* Left */
    flipped += left_count[(my_bits.high << 4) & 127];

    /* Down left */
    if ((my_bits.high & 0x00000400ul) == 0) {
        if ((my_bits.high & 0x00020000ul) == 0) {
            if ((my_bits.high & 0x01000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up right */
    if ((my_bits.low & 0x10000000ul) == 0) {
        if ((my_bits.low & 0x00200000ul) == 0) {
            if ((my_bits.low & 0x00004000ul) == 0) {
                if ((my_bits.low & 0x00000080ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00000800ul) == 0) {
        if ((my_bits.high & 0x00080000ul) == 0) {
            if ((my_bits.high & 0x08000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x08000000ul) == 0) {
        if ((my_bits.low & 0x00080000ul) == 0) {
            if ((my_bits.low & 0x00000800ul) == 0) {
                if ((my_bits.low & 0x00000008ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00001000ul) == 0) {
        if ((my_bits.high & 0x00200000ul) == 0) {
            if ((my_bits.high & 0x40000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x04000000ul) == 0) {
        if ((my_bits.low & 0x00020000ul) == 0) {
            if ((my_bits.low & 0x00000100ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

static int
CountFlips_bitboard_e5(const BitBoard my_bits) {
    unsigned int flipped = 0;

    /* Right */
    flipped += right_count[(my_bits.high >> 5) & 7];

    /* Left */
    flipped += left_count[(my_bits.high << 3) & 127];

    /* Down left */
    if ((my_bits.high & 0x00000800ul) == 0) {
        if ((my_bits.high & 0x00040000ul) == 0) {
            if ((my_bits.high & 0x02000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up right */
    if ((my_bits.low & 0x20000000ul) == 0) {
        if ((my_bits.low & 0x00400000ul) == 0) {
            if ((my_bits.low & 0x00008000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Down */
    if ((my_bits.high & 0x00001000ul) == 0) {
        if ((my_bits.high & 0x00100000ul) == 0) {
            if ((my_bits.high & 0x10000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up */
    if ((my_bits.low & 0x10000000ul) == 0) {
        if ((my_bits.low & 0x00100000ul) == 0) {
            if ((my_bits.low & 0x00001000ul) == 0) {
                if ((my_bits.low & 0x00000010ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }
    /* Down right */
    if ((my_bits.high & 0x00002000ul) == 0) {
        if ((my_bits.high & 0x00400000ul) == 0) {
            if ((my_bits.high & 0x80000000ul) != 0)
                flipped += 2;
        }
        else {
            flipped += 1;
        }
    }
    /* Up left */
    if ((my_bits.low & 0x08000000ul) == 0) {
        if ((my_bits.low & 0x00040000ul) == 0) {
            if ((my_bits.low & 0x00000200ul) == 0) {
                if ((my_bits.low & 0x00000001ul) != 0)
                    flipped += 3;
            }
            else {
                flipped += 2;
            }
        }
        else {
            flipped += 1;
        }
    }

    return flipped;
}

int(*CountFlips_bitboard[64])(const BitBoard my_bits) = {

    CountFlips_bitboard_a1,
    CountFlips_bitboard_b1,
    CountFlips_bitboard_c1,
    CountFlips_bitboard_d1,
    CountFlips_bitboard_e1,
    CountFlips_bitboard_f1,
    CountFlips_bitboard_g1,
    CountFlips_bitboard_h1,

    CountFlips_bitboard_a2,
    CountFlips_bitboard_b2,
    CountFlips_bitboard_c2,
    CountFlips_bitboard_d2,
    CountFlips_bitboard_e2,
    CountFlips_bitboard_f2,
    CountFlips_bitboard_g2,
    CountFlips_bitboard_h2,

    CountFlips_bitboard_a3,
    CountFlips_bitboard_b3,
    CountFlips_bitboard_c3,
    CountFlips_bitboard_d3,
    CountFlips_bitboard_e3,
    CountFlips_bitboard_f3,
    CountFlips_bitboard_g3,
    CountFlips_bitboard_h3,

    CountFlips_bitboard_a4,
    CountFlips_bitboard_b4,
    CountFlips_bitboard_c4,
    CountFlips_bitboard_d4,
    CountFlips_bitboard_e4,
    CountFlips_bitboard_f4,
    CountFlips_bitboard_g4,
    CountFlips_bitboard_h4,

    CountFlips_bitboard_a5,
    CountFlips_bitboard_b5,
    CountFlips_bitboard_c5,
    CountFlips_bitboard_d5,
    CountFlips_bitboard_e5,
    CountFlips_bitboard_f5,
    CountFlips_bitboard_g5,
    CountFlips_bitboard_h5,

    CountFlips_bitboard_a6,
    CountFlips_bitboard_b6,
    CountFlips_bitboard_c6,
    CountFlips_bitboard_d6,
    CountFlips_bitboard_e6,
    CountFlips_bitboard_f6,
    CountFlips_bitboard_g6,
    CountFlips_bitboard_h6,

    CountFlips_bitboard_a7,
    CountFlips_bitboard_b7,
    CountFlips_bitboard_c7,
    CountFlips_bitboard_d7,
    CountFlips_bitboard_e7,
    CountFlips_bitboard_f7,
    CountFlips_bitboard_g7,
    CountFlips_bitboard_h7,

    CountFlips_bitboard_a8,
    CountFlips_bitboard_b8,
    CountFlips_bitboard_c8,
    CountFlips_bitboard_d8,
    CountFlips_bitboard_e8,
    CountFlips_bitboard_f8,
    CountFlips_bitboard_g8,
    CountFlips_bitboard_h8

};
