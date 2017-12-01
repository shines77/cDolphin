/*
   File:           pattern.c

   Created:        2007-07-10

   Modified:       2013-08-10

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about the patterns.
*/

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"
#include "colour.h"
#include "board.h"
#include "pattern.h"

/* Global variables */

int pow3[10] = { 1, 3, 9, 27, 81, 243, 729, 2187, 6561, 19683 };

/* Connections between the squares and the bit masks */

int row_no[64];
int row_index[64];
int col_no[64];
int col_index[64];

int color_pattern[3];

/* The patterns describing the current state of the board. */

int row_pattern[8];
int col_pattern[8];

/* Symmetry maps */

int flip8[6561];

/* Bit masks which represent dependencies between discs and patterns */

unsigned int depend_lo[64];
unsigned int depend_hi[64];

/* Bit masks that show what patterns have been modified */

unsigned int modified_lo = 0;
unsigned int modified_hi = 0;

/*
   TRANSFORMATION_SET_UP
   Calculate the various symmetry and color transformations.
*/

static void
transformation_setup(void) {
    int i, j;
    int row[10];
    int mirror_pattern;

    /* Build the pattern tables for 8*1-patterns */

    for (i = 0; i < 8; i++)
        row[i] = 0;

    for (i = 0; i < 6561; i++) {
        /* Create the symmetry map */
        mirror_pattern = 0;
        for (j = 0; j < 8; j++)
            mirror_pattern += row[j] * pow3[7 - j];

        flip8[i] = mirror_pattern;

        /* Next configuration */
        j = 0;
        do {  /* The odometer principle */
            row[j]++;
            if (row[j] == 3)
                row[j] = 0;
            j++;
        } while ((row[j - 1] == 0) && (j < 8));
    }

    /* output map mirror data for test */
    //output_mirror_data(&flip8[0], 6561, "flip8_org");
}

/*
  ADD_SINGLE
  Mark board position POS as depending on pattern # MASK.
*/

static void
add_single(int mask, int pos) {
#if _DEBUG
    if (mask < 0 || mask >= 64)
        printf("ERROR: add_single(): mask < 0 || mask >= 64, mask = %d\n", mask);
    if (pos < 0 || pos >= 64)
        printf("ERROR: add_single(): pos < 0 || pos >= 64, pos = %d\n", pos);
#endif
    if (mask < 32)
        depend_lo[pos] |= 1 << mask;
    else
        depend_hi[pos] |= 1 << (mask - 32);
}

/*
  ADD_MULTIPLE
  Mark board positions POS, POS+STEP, ..., POS+(COUNT-1)STEP as
  depending on pattern # MASK.
*/

static void
add_multiple(int mask, int pos, int count, int step) {
    int i;

    for (i = 0; i < count; i++)
        add_single(mask, pos + i * step);
}

/*
  PATTERN_DEPENDENCY
  Fill the dependency masks for each square with the bit masks
  for the patterns which it depends.
  Note: The definitions of the patterns and their corresponding name
        must match the order given in endmacro.c.
*/

static void
pattern_dependency(void) {

    /* A-file+2X: a1-a8 + b2,b7 */

    add_multiple(AFILEX1, A1, 8, 8);
    add_single(AFILEX1, B2);
    add_single(AFILEX1, B7);

    /* A-file+2X: h1-h8 + g2,g7 */

    add_multiple(AFILEX2, H1, 8, 8);
    add_single(AFILEX2, G2);
    add_single(AFILEX2, G7);

    /* A-file+2X: a1-h1 + b2,g2 */

    add_multiple(AFILEX3, A1, 8, 1);
    add_single(AFILEX3, B2);
    add_single(AFILEX3, G2);

    /* A-file+2X: a8-h8 + b7,g7 */

    add_multiple(AFILEX4, A8, 8, 1);
    add_single(AFILEX4, B7);
    add_single(AFILEX4, G7);

    /* B-file: b1-b8 */

    add_multiple(BFILE1, B1, 8, 8);

    /* B-file: g1-g8 */

    add_multiple(BFILE2, G1, 8, 8);

    /* B-file: a2-h2 */

    add_multiple(BFILE3, A2, 8, 1);

    /* B-file: a7-h7 */

    add_multiple(BFILE4, A7, 8, 1);

    /* C-file: c1-c8 */

    add_multiple(CFILE1, C1, 8, 8);

    /* C-file: f1-f8 */

    add_multiple(CFILE2, F1, 8, 8);

    /* C-file: a3-h3 */

    add_multiple(CFILE3, A3, 8, 1);

    /* C-file: a6-h6 */

    add_multiple(CFILE4, A6, 8, 1);

    /* D-file: d1-d8 */

    add_multiple(DFILE1, D1, 8, 8);

    /* D-file: e1-e8 */

    add_multiple(DFILE2, E1, 8, 8);

    /* D-file: a4-h4 */

    add_multiple(DFILE3, A4, 8, 1);

    /* D-file: a5-h5 */

    add_multiple(DFILE4, A5, 8, 1);

    /* Diag8: a1-h8 */

    add_multiple(DIAG8_1, A1, 8, 9);

    /* Diag8: h1-a8 */

    add_multiple(DIAG8_2, H1, 8, 7);

    /* Diag7: b1-h7 */

    add_multiple(DIAG7_1, B1, 7, 9);

    /* Diag7: a2-g8 */

    add_multiple(DIAG7_2, A2, 7, 9);

    /* Diag7: a7-g1 */

    add_multiple(DIAG7_3, A7, 7, -7);     /* fixed bug by shines, 2013/08/09 */

    /* Diag7: b8-h2 */

    add_multiple(DIAG7_4, B8, 7, -7);     /* fixed bug by shines, 2013/08/09 */

    /* Diag6: c1-h6 */

    add_multiple(DIAG6_1, C1, 6, 9);

    /* Diag6: a3-f8 */

    add_multiple(DIAG6_2, A3, 6, 9);

    /* Diag6: a6-f1 */

    add_multiple(DIAG6_3, A6, 6, -7);     /* fixed bug by shines, 2013/08/09 */

    /* Diag6: c8-h3 */

    add_multiple(DIAG6_4, C8, 6, -7);     /* fixed bug by shines, 2013/08/09 */

    /* Diag5: d1-h5 */

    add_multiple(DIAG5_1, D1, 5, 9);

    /* Diag5: a4-e8 */

    add_multiple(DIAG5_2, A4, 5, 9);

    /* Diag5: a5-e1 */

    add_multiple(DIAG5_3, A5, 5, -7);     /* fixed bug by shines, 2013/08/09 */

    /* Diag5: d8-h4 */

    add_multiple(DIAG5_4, D8, 5, -7);     /* fixed bug by shines, 2013/08/09 */

    /* Diag4: e1-h4 */

    add_multiple(DIAG4_1, E1, 4, 9);

    /* Diag4: a5-d8 */

    add_multiple(DIAG4_2, A5, 4, 9);

    /* Diag4: a4-d1 */

    add_multiple(DIAG4_3, A4, 4, -7);     /* fixed bug by shines, 2013/08/09 */

    /* Diag4: e8-h5 */

    add_multiple(DIAG4_4, E8, 4, -7);     /* fixed bug by shines, 2013/08/09 */

    /* Corner3x3: a1-c1 + a2-c2 + a3-c3 */

    add_multiple(CORNER33_1, A1, 3, 1);
    add_multiple(CORNER33_1, A2, 3, 1);
    add_multiple(CORNER33_1, A3, 3, 1);

    /* Corner3x3: a8-c8 + a7-c7 + a6-c6 */

    add_multiple(CORNER33_2, A8, 3, 1);
    add_multiple(CORNER33_2, A7, 3, 1);
    add_multiple(CORNER33_2, A6, 3, 1);

    /* Corner3x3: f1-h1 + f2-h2 + f3-h3 */

    add_multiple(CORNER33_3, H1, 3, -1);
    add_multiple(CORNER33_3, H2, 3, -1);
    add_multiple(CORNER33_3, H3, 3, -1);

    /* Corner3x3: f8-h8 + f7-h7 + f6-h6 */

    add_multiple(CORNER33_4, H8, 3, -1);
    add_multiple(CORNER33_4, H7, 3, -1);
    add_multiple(CORNER33_4, H6, 3, -1);

    /* Corner4x2: a1-d1 + a2-d2 */

    add_multiple(CORNER42_1, A1, 4, 1);
    add_multiple(CORNER42_1, A2, 4, 1);

    /* Corner4x2: a8-d8 + a7-d7 */

    add_multiple(CORNER42_2, A8, 4, 1);
    add_multiple(CORNER42_2, A7, 4, 1);

    /* Corner4x2: e1-h1 + e2-h2 */

    add_multiple(CORNER42_3, H1, 4, -1);
    add_multiple(CORNER42_3, H2, 4, -1);

    /* Corner4x2: e8-h8 + e7-h7 */

    add_multiple(CORNER42_4, H8, 4, -1);
    add_multiple(CORNER42_4, H7, 4, -1);

    /* Corner4x2: a1-a4 + b1-b4 */

    add_multiple(CORNER42_5, A1, 4, 8);
    add_multiple(CORNER42_5, B1, 4, 8);

    /* Corner4x2: h1-h4 + g1-g4 */

    add_multiple(CORNER42_6, H1, 4, 8);
    add_multiple(CORNER42_6, G1, 4, 8);

    /* Corner4x2: a8-a5 + b8-b5 */

    add_multiple(CORNER42_7, A8, 4, -8);
    add_multiple(CORNER42_7, B8, 4, -8);

    /* Corner4x2: h8-h5 + g8-g5 */

    add_multiple(CORNER42_8, H8, 4, -8);
    add_multiple(CORNER42_8, G8, 4, -8);
}

/*
   INIT_PATTERNS
   Pre-computes some tables needed for fast pattern access.
*/

void
init_patterns(void) {
    int i, j;
    int pos;

    transformation_setup();

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            pos = 8 * i + j;
            row_no[pos] = i;
            row_index[pos] = j;
            col_no[pos] = j;
            col_index[pos] = i;
        }
    }

    pattern_dependency();

    /* These values needed for compatibility with the old book format */

    color_pattern[CHESS_EMPTY] = EMPTY_PATTERN;
    color_pattern[CHESS_BLACK] = BLACK_PATTERN;
    color_pattern[CHESS_WHITE] = WHITE_PATTERN;
}

/*
   COMPUTE_LINE_PATTERNS
   Translate the current board configuration into patterns.
*/

void==
compute_line_patterns(int *in_board) {
    int i, j;
    int pos;
    int mask;

    for (i = 0; i < 8; i++) {
        row_pattern[i] = 0;
        col_pattern[i] = 0;
    }

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            pos = 8 * i + j;
            if (in_board[pos] == CHESS_EMPTY)
                mask = EMPTY_PATTERN;
            else
                mask = color_pattern[in_board[pos]];
            row_pattern[row_no[pos]] += mask * pow3[row_index[pos]];
            col_pattern[col_no[pos]] += mask * pow3[col_index[pos]];
        }
    }
}
