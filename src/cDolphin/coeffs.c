/*
   File:           coeffs.c

   Created:        2007-07-10

   Modified:       2013-08-10

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       Routines about calculate pattern coeffs.
                   Unpacks the coefficient file, computes final-stage
                   pattern values and performs pattern evaluation.
*/

//#define __linux__

#define USE_ZIP_COEFFS_FILE     1

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#endif

#if !defined( _WIN32 ) && !defined( _WIN32_WCE ) && !defined( __linux__ )
#include <dir.h>
#endif

#if defined(USE_ZIP_COEFFS_FILE) && (USE_ZIP_COEFFS_FILE != 0)

#ifdef _WIN32_WCE
#include "zlib/zlib.h"
#elif defined (_MSC_VER)
#include "zlib/zlib.h"
#else
#include <assert.h>
#include <zlib.h>
#endif

/*
#ifdef _DEBUG
#pragma comment (lib, "..\\..\\..\\..\\lib\\windows\\x86\\vc60\\zlib_debug.lib")
#else
#pragma comment (lib, "..\\..\\..\\..\\lib\\windows\\x86\\vc60\\zlib.lib")
#endif
//*/

#else

#include <stdio.h>
typedef FILE *gzFile;

#endif

#if defined( _WIN32 ) || defined( _WIN32_WCE )
#include <assert.h>
#ifndef assert
#define assert ASSERT
#endif
#include <direct.h>
//#include <atltrace.h>
///*
#ifndef _DEBUG
  /*
  #ifdef ATLTRACE
    #undef ATLTRACE
    #undef ATLTRACE2
  #endif
  #ifndef ATLTRACE
    #define ATLTRACE ATL::CTraceFileAndLineInfo(__FILE__, __LINE__)
    #define ATLTRACE2 ATLTRACE
  #endif
  // extern ATL::CTrace TRACE;
  //*/
#ifdef TRACE
#undef TRACE
#endif
//#define TRACE ATLTRACE
#define TRACE
#else
#define TRACE ATLTRACE
#endif
//*/
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "colour.h"
#include "board.h"
#include "bitboard.h"
#include "eval.h"
#include "coeffs.h"
#include "move.h"
#include "pattern.h"
#include "search.h"

//#define USE_BYTE_BOARD

#ifndef USE_BYTE_BOARD
static ALIGN_PREFIX(64) unsigned int wBoard[64] ALIGN_SUFFIX(64);
#ifdef _DEBUG
static ALIGN_PREFIX(64) unsigned int wBoard2[64] ALIGN_SUFFIX(64);
#endif
#else
static ALIGN_PREFIX(64) unsigned char wBoard[64] ALIGN_SUFFIX(64);
#ifdef _DEBUG
static ALIGN_PREFIX(64) unsigned char wBoard2[64] ALIGN_SUFFIX(64);
#endif
#endif

/* An upper limit on the number of coefficient blocks in the arena */
#define MAX_BLOCKS            200

/* The file containing the feature values (really shouldn't be #define'd) */
#if defined(USE_ZIP_COEFFS_FILE) && (USE_ZIP_COEFFS_FILE != 0)
#   define PATTERN_FILE          "coeffs2.bin"
#else
#   define PATTERN_FILE          "coeffs2.dat"
#endif

/* Calculate cycle counts for the eval function? */
#define TIME_EVAL             0

#if defined(_MSC_VER) || defined(__ICL)

#pragma pack(push, 1)
typedef struct tagCoeffSet {
    int permanent;
    int loaded;
    int prev, next;
    int block;
    short parity_constant[2];
    short parity;
    short constant;
    short *afile2x, *bfile, *cfile, *dfile;
    short *diag8, *diag7, *diag6, *diag5, *diag4;
    short *corner33, *corner52;
    short *afile2x_last, *bfile_last, *cfile_last, *dfile_last;
    short *diag8_last, *diag7_last, *diag6_last, *diag5_last, *diag4_last;
    short *corner33_last, *corner52_last;
    char alignment_padding[12];  /* In order to achieve 128-byte alignment */
} CoeffSet;
#pragma pack(pop)

#elif defined(__GNUC__)

typedef struct tagCoeffSet {
    int permanent;
    int loaded;
    int prev, next;
    int block;
    short parity_constant[2];
    short parity;
    short constant;
    short *afile2x, *bfile, *cfile, *dfile;
    short *diag8, *diag7, *diag6, *diag5, *diag4;
    short *corner33, *corner52;
    short *afile2x_last, *bfile_last, *cfile_last, *dfile_last;
    short *diag8_last, *diag7_last, *diag6_last, *diag5_last, *diag4_last;
    short *corner33_last, *corner52_last;
    char alignment_padding[12];  /* In order to achieve 128-byte alignment */
} __attribute__((packed)) CoeffSet;

#else

typedef struct tagCoeffSet {
    int permanent;
    int loaded;
    int prev, next;
    int block;
    short parity_constant[2];
    short parity;
    short constant;
    short *afile2x, *bfile, *cfile, *dfile;
    short *diag8, *diag7, *diag6, *diag5, *diag4;
    short *corner33, *corner52;
    short *afile2x_last, *bfile_last, *cfile_last, *dfile_last;
    short *diag8_last, *diag7_last, *diag6_last, *diag5_last, *diag4_last;
    short *corner33_last, *corner52_last;
    char alignment_padding[12];  /* In order to achieve 128-byte alignment */
} CoeffSet;

#endif

typedef struct tagAllocationBlock {
    short afile2x_block[59049];
    short bfile_block[6561];
    short cfile_block[6561];
    short dfile_block[6561];
    short diag8_block[6561];
    short diag7_block[2187];
    short diag6_block[729];
    short diag5_block[243];
    short diag4_block[81];
    short corner33_block[19683];
    short corner52_block[59049];
} AllocationBlock;

static FILE *g_save_stream = NULL;
static int g_getword_cnt = 0;

static int stage_count = 0;
static int block_count = 0;
static int stage[64];
static int block_allocated[MAX_BLOCKS], block_set[MAX_BLOCKS];
static int eval_map[64];
static AllocationBlock *block_list[MAX_BLOCKS];

static ALIGN_PREFIX(64) CoeffSet coeffs[64] ALIGN_SUFFIX(64);

static ALIGN_PREFIX(64) unsigned int disc_set_tableb[16] ALIGN_SUFFIX(64);
static ALIGN_PREFIX(64) unsigned int disc_set_tablew[16] ALIGN_SUFFIX(64);

static ALIGN_PREFIX(64) unsigned int disc_set_table_bw[256] ALIGN_SUFFIX(64);

static ALIGN_PREFIX(64) unsigned int disc_set_table_bw32[256 * 4] ALIGN_SUFFIX(64);

static ALIGN_PREFIX(64) unsigned short pattern_mask[512] ALIGN_SUFFIX(64);
static ALIGN_PREFIX(64) unsigned int pattern_mask2[4] ALIGN_SUFFIX(64);
//ALIGN_PREFIX(64) static unsigned int pattern_mask3[16] ALIGN_SUFFIX(64);

int min_coffe, max_coffe;

void
init_pattern_mask(void) {
    int i, mask;
    unsigned long scan_bit;
    //unsigned long scan_bit2;
    int power, value;
    for (mask = 0; mask < 512; mask++) {
        scan_bit = 1;
        power = 1;
        value = 0;
        for (i = 0; i < 9; i++) {
            if ((mask & scan_bit) == scan_bit)
                value += power;
            power *= 3;
            scan_bit <<= 1;
        }
        pattern_mask[mask] = value;
    }

    for (mask = 0; mask < 4; mask++) {
        scan_bit = 1;
        power = 6561;
        value = 0;
        for (i = 0; i < 2; i++) {
            if ((mask & scan_bit) == scan_bit)
                value += power;
            power *= 3;
            scan_bit <<= 1;
        }
        pattern_mask2[mask] = value;
    }

    /*
    for ( mask = 0; mask < 16; mask++ ) {
        scan_bit = 1;
        scan_bit2 = 4;
        power = 6561;
        value = 26244;
        for ( i = 0; i < 2; i++ ) {
            if ( (mask & scan_bit) == scan_bit )
                value -= power;
            if ( (mask & scan_bit2) == scan_bit2  ) {
                value += power;
            }
            power *= 3;
            scan_bit <<= 1;
            scan_bit2 <<= 1;
        }
        pattern_mask3[mask] = value;
    }
    //*/
}

/*
   TERMINAL_PATTERNS
   Calculates the patterns associated with a filled board,
   only counting discs.
*/

static void
terminal_patterns(void) {
    double result;
    double value[8][8];
    int i, j, k;
    int row[10];
    int hit[8][8];

    /* Count the number of times each square is counted */

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++)
            hit[i][j] = 0;
    }
    for (i = 0; i < 8; i++) {
        hit[0][i]++;
        hit[i][0]++;
        hit[7][i]++;
        hit[i][7]++;
    }
    for (i = 0; i < 8; i++) {
        hit[1][i]++;
        hit[i][1]++;
        hit[6][i]++;
        hit[i][6]++;
    }
    for (i = 0; i < 8; i++) {
        hit[2][i]++;
        hit[i][2]++;
        hit[5][i]++;
        hit[i][5]++;
    }
    for (i = 0; i < 8; i++) {
        hit[3][i]++;
        hit[i][3]++;
        hit[4][i]++;
        hit[i][4]++;
    }
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            hit[i][j]++;
            hit[i][7 - j]++;
            hit[7 - i][j]++;
            hit[7 - i][7 - j]++;
        }
    }
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 5; j++) {
            hit[i][j]++;
            hit[j][i]++;
            hit[i][7 - j]++;
            hit[j][7 - i]++;
            hit[7 - i][j]++;
            hit[7 - j][i]++;
            hit[7 - i][7 - j]++;
            hit[7 - j][7 - i]++;
        }
    }
    for (i = 0; i < 8; i++) {
        hit[i][i]++;
        hit[i][7 - i]++;
    }
    for (i = 0; i < 7; i++) {
        hit[i][i + 1]++;
        hit[i + 1][i]++;
        hit[i][6 - i]++;
        hit[i + 1][7 - i]++;
    }
    for (i = 0; i < 6; i++) {
        hit[i][i + 2]++;
        hit[i + 2][i]++;
        hit[i][5 - i]++;
        hit[i + 2][7 - i]++;
    }
    for (i = 0; i < 5; i++) {
        hit[i][i + 3]++;
        hit[i + 3][i]++;
        hit[i][4 - i]++;
        hit[i + 3][7 - i]++;
    }
    for (i = 0; i < 4; i++) {
        hit[i][i + 4]++;
        hit[i + 4][i]++;
        hit[i][3 - i]++;
        hit[i + 4][7 - i]++;
    }
    hit[1][1] += 2;
    hit[1][6] += 2;
    hit[6][1] += 2;
    hit[6][6] += 2;

    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++)
            value[i][j] = 1.0 / hit[i][j];
    }

    for (i = 0; i < 10; i++)
        row[i] = 0;

    for (i = 0; i < 59049; i++) {
        result = 0.0;
        for (j = 0; j < 8; j++) {
            if (row[j] == CHESS_BLACK)
                result += value[0][j];
            else if (row[j] == CHESS_WHITE)
                result -= value[0][j];
        }
        if (row[8] == CHESS_BLACK)
            result += value[1][1];
        else if (row[8] == CHESS_WHITE)
            result -= value[1][1];
        if (row[9] == CHESS_BLACK)
            result += value[1][6];
        else if (row[9] == CHESS_WHITE)
            result -= value[1][6];
        coeffs[60].afile2x[i] = (short)floor(result * 128.0 + 0.5);

        result = 0.0;
        for (j = 0; j < 5; j++) {
            for (k = 0; k < 2; k++) {
                if (row[5 * k + j] == CHESS_BLACK)
                    result += value[j][k];
                else if (row[5 * k + j] == CHESS_WHITE)
                    result -= value[j][k];
            }
        }
        coeffs[60].corner52[i] = (short)floor(result * 128.0 + 0.5);
        if (i < 19683) {
            result = 0.0;
            for (j = 0; j < 3; j++) {
                for (k = 0; k < 3; k++) {
                    if (row[3 * j + k] == CHESS_BLACK)
                        result += value[j][k];
                    else if (row[3 * j + k] == CHESS_WHITE)
                        result -= value[j][k];
                }
            }
            coeffs[60].corner33[i] = (short)floor(result * 128.0 + 0.5);
        }
        if (i < 6561) {
            result = 0.0;
            for (j = 0; j < 8; j++) {
                if (row[j] == CHESS_BLACK)
                    result += value[1][j];
                else if (row[j] == CHESS_WHITE)
                    result -= value[1][j];
            }
            coeffs[60].bfile[i] = (short)floor(result * 128.0 + 0.5);

            result = 0.0;
            for (j = 0; j < 8; j++) {
                if (row[j] == CHESS_BLACK)
                    result += value[2][j];
                else if (row[j] == CHESS_WHITE)
                    result -= value[2][j];
            }
            coeffs[60].cfile[i] = (short)floor(result * 128.0 + 0.5);

            result = 0.0;
            for (j = 0; j < 8; j++) {
                if (row[j] == CHESS_BLACK)
                    result += value[3][j];
                else if (row[j] == CHESS_WHITE)
                    result -= value[3][j];
            }
            coeffs[60].dfile[i] = (short)floor(result * 128.0 + 0.5);

            result = 0.0;
            for (j = 0; j < 8; j++) {
                if (row[j] == CHESS_BLACK)
                    result += value[j][j];
                else if (row[j] == CHESS_WHITE)
                    result -= value[j][j];
            }
            coeffs[60].diag8[i] = (short)floor(result * 128.0 + 0.5);
        }
        if (i < 2187) {
            result = 0.0;
            for (j = 0; j < 7; j++) {
                if (row[j] == CHESS_BLACK)
                    result += value[j][j + 1];
                else if (row[j] == CHESS_WHITE)
                    result -= value[j][j + 1];
            }
            coeffs[60].diag7[i] = (short)floor(result * 128.0 + 0.5);
        }
        if (i < 729) {
            result = 0.0;
            for (j = 0; j < 6; j++) {
                if (row[j] == CHESS_BLACK)
                    result += value[j][j + 2];
                else if (row[j] == CHESS_WHITE)
                    result -= value[j][j + 2];
            }
            coeffs[60].diag6[i] = (short)floor(result * 128.0 + 0.5);
        }
        if (i < 243) {
            result = 0.0;
            for (j = 0; j < 5; j++) {
                if (row[j] == CHESS_BLACK)
                    result += value[j][j + 3];
                else if (row[j] == CHESS_WHITE)
                    result -= value[j][j + 3];
            }
            coeffs[60].diag5[i] = (short)floor(result * 128.0 + 0.5);
        }
        if (i < 81) {
            result = 0.0;
            for (j = 0; j < 4; j++) {
                if (row[j] == CHESS_BLACK)
                    result += value[j][j + 4];
                else if (row[j] == CHESS_WHITE)
                    result -= value[j][j + 4];
            }
            coeffs[60].diag4[i] = (short)floor(result * 128.0 + 0.5);
        }

        /* Next configuration */
        j = 0;
        do {  /* The odometer principle */
            row[j]++;
            if (row[j] == 3)
                row[j] = 0;
            j++;
        } while ((row[j - 1] == 0) && (j < 10));
    }
}

#define READ_ERROR_HI       "READ_ERROR_HIGH"
#define READ_ERROR_LO       "READ_ERROR_LOW"

/*
   GET_WORD
   Reads a 16-bit signed integer from a file.
*/

static short
get_word(gzFile stream) {
    union uni_val {
        short signed_val;
        unsigned short unsigned_val;
    } val;

#if defined(USE_ZIP_COEFFS_FILE) && (USE_ZIP_COEFFS_FILE != 0)
    int hi, lo;

    hi = gzgetc(stream);
    //if (hi < 0 || hi > 256)
    //	hi = hi;
    //assert( hi != -1 );
    //if (hi == -1)
    //    TRACE("assert( hi != -1 );\n");
    //if (hi == -1) hi = 0;
#ifdef _DEBUG
    if (hi == -1)
        val.signed_val = -1;
#endif

    lo = gzgetc(stream);
    //if (lo < 0 || lo > 256)
    //	lo = lo;
    //assert( lo != -1 );
    //if (lo == -1)
    //    TRACE("assert( lo != -1 );\n");
#ifdef _DEBUG
    if (lo == -1)
        val.signed_val = -1;
#endif

#else

    int received;
    unsigned char hi, lo;
    hi = 0; lo = 0;

    received = fread(&hi, sizeof(unsigned char), 1, (FILE *)stream);
    if (received != 1) {
        //puts( READ_ERROR_HI );
    }
    received = fread(&lo, sizeof(unsigned char), 1, (FILE *)stream);
    if (received != 1) {
        //puts( READ_ERROR_LO );
    }

#endif

    val.unsigned_val = (hi << 8) + lo;

#if 0
    if (g_save_stream != NULL) {
        fwrite(&hi, sizeof(unsigned char), 1, g_save_stream);
        fwrite(&lo, sizeof(unsigned char), 1, g_save_stream);
        //fwrite(&val.unsigned_val, sizeof(val), 1, g_save_stream);
    }
#endif

    //assert( (lo != -1) || (hi != -1) );
    //TRACE("val.unsigned_val\n");

   //g_getword_cnt++;

    return val.signed_val;
}


/*
   UNPACK_BATCH
   Reads feature values for one specific pattern
*/

static void
unpack_batch(short *item, int *mirror, int count, gzFile stream) {
    int i;
    short *buffer;

    //g_getword_cnt = 0;

    buffer = (short *)malloc(count * sizeof(short));
    if (buffer == NULL) {
        exit(EXIT_FAILURE);
        return;
    }
    memset(buffer, 0, count * sizeof(short));

    /* Unpack the coefficient block where the score is scaled
    so that 512 units corresponds to one disk. */

    for (i = 0; i < count; i++) {
        if ((mirror == NULL) || (mirror[i] == i))
            buffer[i] = get_word(stream) / 4;
        else
            buffer[i] = buffer[mirror[i]];
    }

    for (i = 0; i < count; i++)
        item[i] = buffer[i];

    if (mirror != NULL) {
        for (i = 0; i < count; i++) {
            if (item[i] != item[mirror[i]]) {
                printf("%s @ %d <--> %d of %d\n", "Mirror symmetry error",
                    i, mirror[i], count);
                printf("%d <--> %d\n", item[i], item[mirror[i]]);
                exit(EXIT_FAILURE);
            }
        }
    }

    //printf("getword_cnt = %5d\n", g_getword_cnt);
    free(buffer);
}

#if 0

/*
   UNPACK_BATCH2
   Reads feature values for one specific pattern
*/

static void
unpack_batch2(short *item, int *mirror, int count, char *base, gzFile stream) {
    int i;
    short *buffer;
    float *vals;
    int *freq;
    char file_name[32];
    FILE *stream2;

    buffer = (short *)malloc(count * sizeof(short));
    if (buffer == NULL) {
        exit(EXIT_FAILURE);
        return;
    }

    sprintf(file_name, "%s%s", base, ".b6");
    stream2 = fopen(file_name, "wb");
    if (stream2 == NULL) {
        exit(EXIT_FAILURE);
        return;
    }

    vals = (float *)malloc(count * sizeof(float));
    freq = (int *)malloc(count * sizeof(int));

    /* Unpack the coefficient block where the score is scaled
    so that 512 units corresponds to one disk. */

    for (i = 0; i < count; i++) {
        if ((mirror == NULL) || (mirror[i] == i)) {
            buffer[i] = get_word(stream) / 4;
            if (buffer[i] > max_coffe)
                max_coffe = buffer[i];
            else if (buffer[i] < min_coffe)
                min_coffe = buffer[i];
            freq[i] = 1;
        }
        else {
            buffer[i] = buffer[mirror[i]];
            freq[i] = 0;
        }
    }

    for (i = 0; i < count; i++) {
        item[i] = buffer[i];
        vals[i] = (double)buffer[i] / 128.0;
    }

    fwrite(vals, sizeof(float), count, stream2);
    fwrite(freq, sizeof(int), count, stream2);

    if (mirror != NULL) {
        for (i = 0; i < count; i++) {
            if (item[i] != item[mirror[i]]) {
                printf("%s @ %d <--> %d of %d\n", "Mirror symmetry error",
                    i, mirror[i], count);
                printf("%d <--> %d\n", item[i], item[mirror[i]]);
                exit(EXIT_FAILURE);
            }
        }
    }

    free(buffer);

    free(freq);
    free(vals);
    fclose(stream2);
}

#endif

/* for test only */

int output_mirror_data(int *map_mirror, int count, const char *name) {
    int ret = 0;
    int i;
    FILE *output_stream;
    char sMapMirrorFile[260];

    _getcwd(sMapMirrorFile, sizeof(sMapMirrorFile));
    strcat(sMapMirrorFile, "\\");
    strcat(sMapMirrorFile, name);
    strcat(sMapMirrorFile, ".mirror.txt");

    output_stream = fopen(sMapMirrorFile, "wb");
    if (output_stream == NULL) {
        //fatal_error( "%s '%s'\n", FILE_ERROR, sSavePatternFile );
        printf("%s '%s'\n", "Unable to open output map_mirror file", sMapMirrorFile);
        system("pause");
        exit(EXIT_FAILURE);
    }

    fprintf(output_stream, "name = %s\r\n\r\n", name);
    for (i = 0; i < count; i++) {
        fprintf(output_stream, "%d\r\n", map_mirror[i]);
    }

    if (output_stream != NULL)
        fclose(output_stream);

    return ret;
}

/*
   UNPACK_COEFFS
   Reads all feature values for a certain stage. To take care of
   symmetric patterns, mirror tables are calculated.
*/

static void
unpack_coeffs(gzFile stream) {
    int i, j, k;
    int mirror_pattern;
    int row[10];
    int *map_mirror3;
    int *map_mirror4;
    int *map_mirror5;
    int *map_mirror6;
    int *map_mirror7;
    int *map_mirror8;
    int *map_mirror33;
    int *map_mirror8x2;

    /* Allocate the memory needed for the temporary mirror maps from the
    heap rather than the stack to reduce memory requirements. */

    map_mirror3 = (int *)malloc(27 * sizeof(int));
    if (map_mirror3 == NULL) {
        puts("map_mirror3 malloc error!\n");
        return;
    }
    map_mirror4 = (int *)malloc(81 * sizeof(int));
    if (map_mirror4 == NULL) {
        puts("map_mirror4 malloc error!\n");
        return;
    }
    map_mirror5 = (int *)malloc(243 * sizeof(int));
    if (map_mirror5 == NULL) {
        puts("map_mirror5 malloc error!\n");
        return;
    }
    map_mirror6 = (int *)malloc(729 * sizeof(int));
    if (map_mirror6 == NULL) {
        puts("map_mirror6 malloc error!\n");
        return;
    }
    map_mirror7 = (int *)malloc(2187 * sizeof(int));
    if (map_mirror7 == NULL) {
        puts("map_mirror7 malloc error!\n");
        return;
    }
    map_mirror8 = (int *)malloc(6561 * sizeof(int));
    if (map_mirror8 == NULL) {
        puts("map_mirror8 malloc error!\n");
        return;
    }
    map_mirror33 = (int *)malloc(19683 * sizeof(int));
    if (map_mirror33 == NULL) {
        puts("map_mirror33 malloc error!\n");
        return;
    }
    map_mirror8x2 = (int *)malloc(59049 * sizeof(int));
    if (map_mirror8x2 == NULL) {
        puts("map_mirror8x2 malloc error!\n");
        return;
    }

    /* Build the pattern tables for 8*1-patterns */

    for (i = 0; i < 8; i++)
        row[i] = 0;

    for (i = 0; i < 6561; i++) {
        mirror_pattern = 0;
        for (j = 0; j < 8; j++)
            mirror_pattern += row[j] * pow3[7 - j];
        /* Create the symmetry map */
        map_mirror8[i] = MIN(i, mirror_pattern);

        /* Next configuration */
        j = 0;
        do {  /* The odometer principle */
            row[j]++;
            if (row[j] == 3)
                row[j] = 0;
            j++;
        } while ((row[j - 1] == 0) && (j < 8));
    }

    /* Build the tables for 7*1-patterns */

    for (i = 0; i < 7; i++)
        row[i] = 0;

    for (i = 0; i < 2187; i++) {
        mirror_pattern = 0;
        for (j = 0; j < 7; j++)
            mirror_pattern += row[j] * pow3[6 - j];
        map_mirror7[i] = MIN(i, mirror_pattern);

        /* Next configuration */
        j = 0;
        do {  /* The odometer principle */
            row[j]++;
            if (row[j] == 3)
                row[j] = 0;
            j++;
        } while ((row[j - 1]) == 0 && (j < 7));
    }

    /* Build the tables for 6*1-patterns */

    for (i = 0; i < 6; i++)
        row[i] = 0;

    for (i = 0; i < 729; i++) {
        mirror_pattern = 0;
        for (j = 0; j < 6; j++)
            mirror_pattern += row[j] * pow3[5 - j];
        map_mirror6[i] = MIN(i, mirror_pattern);

        /* Next configuration */
        j = 0;
        do {  /* The odometer principle */
            row[j]++;
            if (row[j] == 3)
                row[j] = 0;
            j++;
        } while ((row[j - 1]) == 0 && (j < 6));
    }

    /* Build the tables for 5*1-patterns */

    for (i = 0; i < 5; i++)
        row[i] = 0;

    for (i = 0; i < 243; i++) {
        mirror_pattern = 0;
        for (j = 0; j < 5; j++)
            mirror_pattern += row[j] * pow3[4 - j];
        map_mirror5[i] = MIN(mirror_pattern, i);

        /* Next configuration */
        j = 0;
        do {  /* The odometer principle */
            row[j]++;
            if (row[j] == 3)
                row[j] = 0;
            j++;
        } while ((row[j - 1] == 0) && (j < 5));
    }

    /* Build the tables for 4*1-patterns */

    for (i = 0; i < 4; i++)
        row[i] = 0;

    for (i = 0; i < 81; i++) {
        mirror_pattern = 0;
        for (j = 0; j < 4; j++)
            mirror_pattern += row[j] * pow3[3 - j];
        map_mirror4[i] = MIN(i, mirror_pattern);

        /* Next configuration */
        j = 0;
        do {  /* The odometer principle */
            row[j]++;
            if (row[j] == 3)
                row[j] = 0;
            j++;
        } while ((row[j - 1]) == 0 && (j < 4));
    }

    /* Build the tables for 3*1-patterns */

    for (i = 0; i < 3; i++)
        row[i] = 0;

    for (i = 0; i < 27; i++) {
        mirror_pattern = 0;
        for (j = 0; j < 3; j++)
            mirror_pattern += row[j] * pow3[2 - j];
        map_mirror3[i] = MIN(i, mirror_pattern);

        /* Next configuration */
        j = 0;
        do {  /* The odometer principle */
            row[j]++;
            if (row[j] == 3)
                row[j] = 0;
            j++;
        } while ((row[j - 1] == 0) && (j < 3));
    }

    /* Build the tables for 5*2-patterns */

    /* --- none needed --- */

    /* Build the tables for edge2X-patterns */

    //memset( map_mirror8x2, 0, 59049 * sizeof( int ) );
    /*
    for (i = 0; i < 59049; i++) {
        map_mirror8x2[i] = -1;
    }
    //*/

    for (i = 0; i < 6561; i++) {
        for (j = 0; j < 3; j++) {
            for (k = 0; k < 3; k++) {
                map_mirror8x2[i + 6561 * j + 19683 * k] =
                    MIN((flip8[i] + 6561 * k + 19683 * j), (i + 6561 * j + 19683 * k));
            }
        }
    }

#if 0
    g_getword_cnt = 0;
    for (i = 0; i < 59049; i++) {
        if (i == map_mirror8x2[i])
            g_getword_cnt++;
    }
    printf("getword_cnt = %d\n", g_getword_cnt);

    /* output map mirror data for test */
    output_mirror_data(&flip8[0], 6561, "flip8");

    /* output map mirror data for test */
    output_mirror_data(map_mirror8x2, 59049, "map_mirror8x2");
#endif

    /* Build the tables for 3*3-patterns */

    for (i = 0; i < 9; i++)
        row[i] = 0;

    for (i = 0; i < 19683; i++) {
        mirror_pattern =
            row[0] + 3 * row[3] + 9 * row[6] +
            27 * row[1] + 81 * row[4] + 243 * row[7] +
            729 * row[2] + 2187 * row[5] + 6561 * row[8];
        map_mirror33[i] = MIN(i, mirror_pattern);

        /* Next configuration */
        j = 0;
        do {  /* The odometer principle */
            row[j]++;
            if (row[j] == 3)
                row[j] = 0;
            j++;
        } while ((row[j - 1] == 0) && (j < 9));
    }

    /* Read and unpack - using symmetries - the coefficient tables. */

    for (i = 0; i < stage_count - 1; i++) {
#if 1
        //printf("i = %d, stage[i] = %d\n", i, stage[i]);
        coeffs[stage[i]].constant = get_word(stream) / 4;
        coeffs[stage[i]].parity = get_word(stream) / 4;
        coeffs[stage[i]].parity_constant[0] = coeffs[stage[i]].constant;
        coeffs[stage[i]].parity_constant[1] =
            coeffs[stage[i]].constant + coeffs[stage[i]].parity;

        //printf("%8s - %13s : count = %5d, ", "afile2x", "map_mirror8x2", 59049);
        unpack_batch(coeffs[stage[i]].afile2x, map_mirror8x2, 59049, stream);

        //printf("%8s - %13s : count = %5d, ", "bfile", "map_mirror8", 6561);
        unpack_batch(coeffs[stage[i]].bfile, map_mirror8, 6561, stream);

        //printf("%8s - %13s : count = %5d, ", "cfile", "map_mirror8", 6561);
        unpack_batch(coeffs[stage[i]].cfile, map_mirror8, 6561, stream);

        //printf("%8s - %13s : count = %5d, ", "dfile", "map_mirror8", 6561);
        unpack_batch(coeffs[stage[i]].dfile, map_mirror8, 6561, stream);

        //printf("%8s - %13s : count = %5d, ", "diag8", "map_mirror8", 6561);
        unpack_batch(coeffs[stage[i]].diag8, map_mirror8, 6561, stream);

        //printf("%8s - %13s : count = %5d, ", "diag7", "map_mirror6", 2187);
        unpack_batch(coeffs[stage[i]].diag7, map_mirror7, 2187, stream);

        //printf("%8s - %13s : count = %5d, ", "diag6", "map_mirror6", 729);
        unpack_batch(coeffs[stage[i]].diag6, map_mirror6, 729, stream);

        //printf("%8s - %13s : count = %5d, ", "diag5", "map_mirror5", 243);
        unpack_batch(coeffs[stage[i]].diag5, map_mirror5, 243, stream);

        //printf("%8s - %13s : count = %5d, ", "diag4", "map_mirror4", 81);
        unpack_batch(coeffs[stage[i]].diag4, map_mirror4, 81, stream);

        //printf("%8s - %13s : count = %5d, ", "corner33", "map_mirror33", 19683);
        unpack_batch(coeffs[stage[i]].corner33, map_mirror33, 19683, stream);

        //printf("%8s - %13s : count = %5d, ", "corner52", "NULL", 59049);
        unpack_batch(coeffs[stage[i]].corner52, NULL, 59049, stream);
#else
        if (stage[i] == 40) {
            coeffs[stage[i]].constant = get_word(stream) / 4;
            coeffs[stage[i]].parity = get_word(stream) / 4;
            coeffs[stage[i]].parity_constant[0] = coeffs[stage[i]].constant;
            coeffs[stage[i]].parity_constant[1] =
                coeffs[stage[i]].constant + coeffs[stage[i]].parity;
            FILE *stream2;
            char file_name[32];
            sprintf(file_name, "main.s6");
            stream2 = fopen(file_name, "w");
            if (stream2) {
                fprintf(stream2, "%.8f\n", (double)coeffs[stage[i]].constant / 128.0);
                fprintf(stream2, "%.8f\n", (double)coeffs[stage[i]].parity / 128.0);
                fclose(stream2);
            }
            min_coffe = 0; max_coffe = 0;
            unpack_batch2(coeffs[stage[i]].afile2x, map_mirror8x2, 59049, "afile2x", stream);
            min_coffe = 0; max_coffe = 0;
            unpack_batch2(coeffs[stage[i]].bfile, map_mirror8, 6561, "bfile", stream);
            min_coffe = 0; max_coffe = 0;
            unpack_batch2(coeffs[stage[i]].cfile, map_mirror8, 6561, "cfile", stream);
            min_coffe = 0; max_coffe = 0;
            unpack_batch2(coeffs[stage[i]].dfile, map_mirror8, 6561, "dfile", stream);
            min_coffe = 0; max_coffe = 0;
            unpack_batch2(coeffs[stage[i]].diag8, map_mirror8, 6561, "diag8", stream);
            min_coffe = 0; max_coffe = 0;
            unpack_batch2(coeffs[stage[i]].diag7, map_mirror7, 2187, "diag7", stream);
            min_coffe = 0; max_coffe = 0;
            unpack_batch2(coeffs[stage[i]].diag6, map_mirror6, 729, "diag6", stream);
            min_coffe = 0; max_coffe = 0;
            unpack_batch2(coeffs[stage[i]].diag5, map_mirror5, 243, "diag5", stream);
            min_coffe = 0; max_coffe = 0;
            unpack_batch2(coeffs[stage[i]].diag4, map_mirror4, 81, "diag4", stream);
            min_coffe = 0; max_coffe = 0;
            unpack_batch2(coeffs[stage[i]].corner33, map_mirror33, 19683, "corner33", stream);
            min_coffe = 0; max_coffe = 0;
            unpack_batch2(coeffs[stage[i]].corner52, NULL, 59049, "corner52", stream);
        }
        else {
            coeffs[stage[i]].constant = get_word(stream) / 4;
            coeffs[stage[i]].parity = get_word(stream) / 4;
            coeffs[stage[i]].parity_constant[0] = coeffs[stage[i]].constant;
            coeffs[stage[i]].parity_constant[1] =
                coeffs[stage[i]].constant + coeffs[stage[i]].parity;
            unpack_batch(coeffs[stage[i]].afile2x, map_mirror8x2, 59049, stream);
            unpack_batch(coeffs[stage[i]].bfile, map_mirror8, 6561, stream);
            unpack_batch(coeffs[stage[i]].cfile, map_mirror8, 6561, stream);
            unpack_batch(coeffs[stage[i]].dfile, map_mirror8, 6561, stream);
            unpack_batch(coeffs[stage[i]].diag8, map_mirror8, 6561, stream);
            unpack_batch(coeffs[stage[i]].diag7, map_mirror7, 2187, stream);
            unpack_batch(coeffs[stage[i]].diag6, map_mirror6, 729, stream);
            unpack_batch(coeffs[stage[i]].diag5, map_mirror5, 243, stream);
            unpack_batch(coeffs[stage[i]].diag4, map_mirror4, 81, stream);
            unpack_batch(coeffs[stage[i]].corner33, map_mirror33, 19683, stream);
            unpack_batch(coeffs[stage[i]].corner52, NULL, 59049, stream);
        }
#endif
    }

    /* Free the mirror tables - the symmetries are now implicit
    in the coefficient tables. */

    free(map_mirror3);
    free(map_mirror4);
    free(map_mirror5);
    free(map_mirror6);
    free(map_mirror7);
    free(map_mirror8);
    free(map_mirror33);
    free(map_mirror8x2);
}

/*
   GENERATE_BATCH
   Interpolates between two stages.
*/

static void
generate_batch(short *target, int count, short *source1, int weight1,
    short *source2, int weight2) {
    int i;
    int total_weight;

    total_weight = weight1 + weight2;
    if (total_weight == 4) {
        for (i = 0; i < count; i++)
            target[i] = (weight1 * source1[i] + weight2 * source2[i]) / 4;
    }
    else if (total_weight == 6) {
        for (i = 0; i < count; i++)
            target[i] = (weight1 * source1[i] + weight2 * source2[i]) / 6;
    }
    else if (total_weight == 8) {
        for (i = 0; i < count; i++)
            target[i] = (weight1 * source1[i] + weight2 * source2[i]) / 8;
    }
    else {
        for (i = 0; i < count; i++)
            target[i] = (weight1 * source1[i] + weight2 * source2[i]) /
            total_weight;
    }
}

/*
   FIND_MEMORY_BLOCK
   Maintains an internal memory handler to boost
   performance and avoid heap fragmentation.
*/

static int
find_memory_block(short **afile2x, short **bfile, short **cfile,
    short **dfile, short **diag8, short **diag7,
    short **diag6, short **diag5, short **diag4,
    short **corner33, short **corner52, int index) {
    int i;
    int found_free, free_block;

    found_free = FALSE;
    free_block = -1;
    for (i = 0; (i < block_count) && !found_free; i++) {
        if (!block_allocated[i]) {
            found_free = TRUE;
            free_block = i;
        }
    }
    if (!found_free) {
        if (block_count < MAX_BLOCKS) {
            block_list[block_count] = (AllocationBlock *)malloc(sizeof(AllocationBlock));
            if (block_list[block_count] == NULL)
                return (int)NULL;
        }
        if ((block_count == MAX_BLOCKS) ||
            (block_list[block_count] == NULL)) {
            //fatal_error( "%s @ #%d\n", MEMORY_ERROR, block_count );
            printf("%s @ #%d\n", "Memory allocation failure", block_count);
            exit(EXIT_FAILURE);
        }
        free_block = block_count;
        block_count++;
    }
    *afile2x = block_list[free_block]->afile2x_block;
    *bfile = block_list[free_block]->bfile_block;
    *cfile = block_list[free_block]->cfile_block;
    *dfile = block_list[free_block]->dfile_block;
    *diag8 = block_list[free_block]->diag8_block;
    *diag7 = block_list[free_block]->diag7_block;
    *diag6 = block_list[free_block]->diag6_block;
    *diag5 = block_list[free_block]->diag5_block;
    *diag4 = block_list[free_block]->diag4_block;
    *corner33 = block_list[free_block]->corner33_block;
    *corner52 = block_list[free_block]->corner52_block;
    block_allocated[free_block] = TRUE;
    block_set[free_block] = index;

    return free_block;
}

/*
   FREE_MEMORY_BLOCK
   Marks a memory block as no longer in use.
*/

static void
free_memory_block(int block) {
    block_allocated[block] = FALSE;
}

/*
   INIT_MEMORY_HANDLER
   Mark all blocks in the memory arena as "not used".
*/

static void
init_memory_handler(void) {
    int i;

    block_count = 0;
    for (i = 0; i < MAX_BLOCKS; i++)
        block_allocated[i] = FALSE;
}

/*
   ALLOCATE_COEFF
   Finds memory for all patterns belonging to a certain stage.
*/

static void
allocate_coeff(int index) {
    coeffs[index].block =
        find_memory_block(&coeffs[index].afile2x, &coeffs[index].bfile,
            &coeffs[index].cfile, &coeffs[index].dfile,
            &coeffs[index].diag8, &coeffs[index].diag7,
            &coeffs[index].diag6, &coeffs[index].diag5, &coeffs[index].diag4,
            &coeffs[index].corner33, &coeffs[index].corner52, index);
}

/*
   LOAD_COEFF
   Performs linear interpolation between the nearest stages to
   obtain the feature values for the stage in question.
   Also calculates the offset pointers to the last elements in each block
   (used for the inverted patterns when white is to move).
*/

static void
load_coeff(int index) {
    int prev, next;
    int weight1, weight2, total_weight;

    if (!coeffs[index].permanent) {
        prev = coeffs[index].prev;
        next = coeffs[index].next;
        if (prev == next) {
            weight1 = 1;
            weight2 = 1;
        }
        else {
            weight1 = next - index;
            weight2 = index - prev;
        }
        total_weight = weight1 + weight2;
        coeffs[index].constant =
            (weight1 * coeffs[prev].constant + weight2 * coeffs[next].constant) /
            total_weight;
        coeffs[index].parity =
            (weight1 * coeffs[prev].parity + weight2 * coeffs[next].parity) /
            total_weight;
        coeffs[index].parity_constant[0] = coeffs[index].constant;
        coeffs[index].parity_constant[1] =
            coeffs[index].constant + coeffs[index].parity;
        allocate_coeff(index);
        generate_batch(coeffs[index].afile2x, 59049,
            coeffs[prev].afile2x, weight1,
            coeffs[next].afile2x, weight2);
        generate_batch(coeffs[index].bfile, 6561,
            coeffs[prev].bfile, weight1,
            coeffs[next].bfile, weight2);
        generate_batch(coeffs[index].cfile, 6561,
            coeffs[prev].cfile, weight1,
            coeffs[next].cfile, weight2);
        generate_batch(coeffs[index].dfile, 6561,
            coeffs[prev].dfile, weight1,
            coeffs[next].dfile, weight2);
        generate_batch(coeffs[index].diag8, 6561,
            coeffs[prev].diag8, weight1,
            coeffs[next].diag8, weight2);
        generate_batch(coeffs[index].diag7, 2187,
            coeffs[prev].diag7, weight1,
            coeffs[next].diag7, weight2);
        generate_batch(coeffs[index].diag6, 729,
            coeffs[prev].diag6, weight1,
            coeffs[next].diag6, weight2);
        generate_batch(coeffs[index].diag5, 243,
            coeffs[prev].diag5, weight1,
            coeffs[next].diag5, weight2);
        generate_batch(coeffs[index].diag4, 81,
            coeffs[prev].diag4, weight1,
            coeffs[next].diag4, weight2);
        generate_batch(coeffs[index].corner33, 19683,
            coeffs[prev].corner33, weight1,
            coeffs[next].corner33, weight2);
        generate_batch(coeffs[index].corner52, 59049,
            coeffs[prev].corner52, weight1,
            coeffs[next].corner52, weight2);
    }

    coeffs[index].afile2x_last = coeffs[index].afile2x + 59048;
    coeffs[index].bfile_last = coeffs[index].bfile + 6560;
    coeffs[index].cfile_last = coeffs[index].cfile + 6560;
    coeffs[index].dfile_last = coeffs[index].dfile + 6560;
    coeffs[index].diag8_last = coeffs[index].diag8 + 6560;
    coeffs[index].diag7_last = coeffs[index].diag7 + 2186;
    coeffs[index].diag6_last = coeffs[index].diag6 + 728;
    coeffs[index].diag5_last = coeffs[index].diag5 + 242;
    coeffs[index].diag4_last = coeffs[index].diag4 + 80;
    coeffs[index].corner33_last = coeffs[index].corner33 + 19682;
    coeffs[index].corner52_last = coeffs[index].corner52 + 59048;

    coeffs[index].loaded = 1;
}

/*
  DISC_COUNT_ADJUSTMENT
*/

static void
eval_adjustment(double disc_adjust, double edge_adjust,
    double corner_adjust, double x_adjust) {
    int i, j, k;
    int adjust;
    int row[10];

    for (i = 0; i < stage_count - 1; i++) {

        /* Bonuses for having more discs */

        for (j = 0; j < 59049; j++) {
            coeffs[stage[i]].afile2x[j] += coeffs[60].afile2x[j] * disc_adjust;
            coeffs[stage[i]].corner52[j] += coeffs[60].corner52[j] * disc_adjust;
        }
        for (j = 0; j < 19683; j++)
            coeffs[stage[i]].corner33[j] += coeffs[60].corner33[j] * disc_adjust;
        for (j = 0; j < 6561; j++) {
            coeffs[stage[i]].bfile[j] += coeffs[60].bfile[j] * disc_adjust;
            coeffs[stage[i]].cfile[j] += coeffs[60].cfile[j] * disc_adjust;
            coeffs[stage[i]].dfile[j] += coeffs[60].dfile[j] * disc_adjust;
            coeffs[stage[i]].diag8[j] += coeffs[60].diag8[j] * disc_adjust;
        }
        for (j = 0; j < 2187; j++)
            coeffs[stage[i]].diag7[j] += coeffs[60].diag7[j] * disc_adjust;
        for (j = 0; j < 729; j++)
            coeffs[stage[i]].diag6[j] += coeffs[60].diag6[j] * disc_adjust;
        for (j = 0; j < 243; j++)
            coeffs[stage[i]].diag5[j] += coeffs[60].diag5[j] * disc_adjust;
        for (j = 0; j < 81; j++)
            coeffs[stage[i]].diag4[j] += coeffs[60].diag4[j] * disc_adjust;

        for (j = 0; j < 10; j++)
            row[j] = 0;

        for (j = 0; j < 59049; j++) {
            adjust = 0;

            /* Bonus for having edge discs */

            for (k = 1; k <= 6; k++) {
                if (row[k] == CHESS_BLACK)
                    adjust += 128.0 * edge_adjust;
                else if (row[k] == CHESS_WHITE)
                    adjust -= 128.0 * edge_adjust;
            }

            /* Bonus for having corners.  The "0.5 *" is because corners are part
            of two A-file+2X patterns. */

            if (row[0] == CHESS_BLACK)
                adjust += 0.5 * 128.0 * corner_adjust;
            else if (row[0] == CHESS_WHITE)
                adjust -= 0.5 * 128.0 * corner_adjust;
            if (row[7] == CHESS_BLACK)
                adjust += 0.5 * 128.0 * corner_adjust;
            else if (row[7] == CHESS_WHITE)
                adjust -= 0.5 * 128.0 * corner_adjust;

            /* Bonus for having X-squares when the adjacent corners are empty.
            Scaling by 0.5 applies here too. */

            if ((row[8] == CHESS_BLACK) && (row[0] == CHESS_EMPTY))
                adjust += 0.5 * 128.0 * x_adjust;
            else if ((row[8] == CHESS_WHITE) && (row[0] == CHESS_EMPTY))
                adjust -= 0.5 * 128.0 * x_adjust;
            if ((row[9] == CHESS_BLACK) && (row[7] == CHESS_EMPTY))
                adjust += 0.5 * 128.0 * x_adjust;
            else if ((row[9] == CHESS_WHITE) && (row[7] == CHESS_EMPTY))
                adjust -= 0.5 * 128.0 * x_adjust;

            coeffs[stage[i]].afile2x[j] += adjust;

            /* Next configuration */
            k = 0;
            do {  /* The odometer principle */
                row[k]++;
                if (row[k] == 3)
                    row[k] = 0;
                k++;
            } while ((row[k - 1] == 0) && (k < 10));
        }
    }
}

/*
   INIT_COEFFS
   Manages the initialization of all relevant tables.
*/

void
init_coeffs(void) {
    int i, j;
    int word1, word2;
    int subsequent_stage;
    int curr_stage;
    gzFile coeff_stream;
    FILE *adjust_stream;
    char sPatternFile[260];
    char sSavePatternFile[260];

    init_memory_handler();

#if defined( _WIN32_WCE )
    /* Special hack for CE. */
    getcwd(sPatternFile, sizeof(sPatternFile));
    strcat(sPatternFile, PATTERN_FILE);
#elif defined( _WIN32 )
    /* Special hack for CE. */
    _getcwd(sPatternFile, sizeof(sPatternFile));
    strcat(sPatternFile, "\\");
    strcat(sPatternFile, PATTERN_FILE);

    _getcwd(sSavePatternFile, sizeof(sSavePatternFile));
    strcat(sSavePatternFile, "\\");
    strcat(sSavePatternFile, "coeffs2.sav");
#elif defined( __linux__ )
    /* Linux don't support current directory. */
    strcpy(sPatternFile, PATTERN_FILE);
#else
    getcwd(sPatternFile, sizeof(sPatternFile));
    strcat(sPatternFile, "/" PATTERN_FILE);
#endif

#if defined(USE_ZIP_COEFFS_FILE) && (USE_ZIP_COEFFS_FILE != 0)
    coeff_stream = gzopen(sPatternFile, "rb");
#else
    coeff_stream = fopen(sPatternFile, "rb");
#endif
    if (coeff_stream == NULL) {
        //fatal_error( "%s '%s'\n", FILE_ERROR, sPatternFile );
        printf("%s '%s'\n", "Unable to open coefficient file", sPatternFile);
        system("pause");
        exit(EXIT_FAILURE);
    }

#if 0
    g_save_stream = fopen(sSavePatternFile, "wb");
    if (g_save_stream == NULL) {
        //fatal_error( "%s '%s'\n", FILE_ERROR, sSavePatternFile );
        printf("%s '%s'\n", "Unable to open coefficient save file", sSavePatternFile);
        system("pause");
        exit(EXIT_FAILURE);
    }
#endif

    /* Check the magic values in the beginning of the file to make sure
    the file format is right */

    word1 = get_word(coeff_stream);
    word2 = get_word(coeff_stream);

    if ((word1 != EVAL_MAGIC1) || (word2 != EVAL_MAGIC2)) {
        //fatal_error( "%s: %s", sPatternFile, CHECKSUM_ERROR );
        printf("%s: %s", sPatternFile, "Wrong checksum, might be an old version");
        exit(EXIT_FAILURE);
    }

    /* Read the different stages for which the evaluation function
    was tuned and mark the other stages with pointers to the previous
    and next stages. */

    /*
    i = sizeof(coeffs);
    j = sizeof(CoeffSet);
    memset((void *)&coeffs[0], 0, sizeof(coeffs));
    coeffs[0].block = 1;
    coeffs[0].parity_constant[0] = 2;
    coeffs[0].parity_constant[1] = 3;
    coeffs[0].parity = 4;
    coeffs[0].constant = 5;
    //*/

    for (i = 0; i <= 60; i++) {
        coeffs[i].permanent = 0;
        coeffs[i].loaded = 0;
    }

    stage_count = get_word(coeff_stream);
    for (i = 0; i < stage_count - 1; i++) {
        stage[i] = get_word(coeff_stream);
        curr_stage = stage[i];
        if (i == 0) {
            for (j = 0; j < stage[0]; j++) {
                coeffs[j].prev = stage[0];
                coeffs[j].next = stage[0];
            }
        }
        else {
            for (j = stage[i - 1]; j < stage[i]; j++) {
                coeffs[j].prev = stage[i - 1];
                coeffs[j].next = stage[i];
            }
        }
        coeffs[curr_stage].permanent = 1;
        allocate_coeff(curr_stage);
    }
    stage[stage_count - 1] = 60;
    for (j = stage[stage_count - 2]; j < 60; j++) {
        coeffs[j].prev = stage[stage_count - 2];
        coeffs[j].next = 60;
    }

    coeffs[60].permanent = 1;
    allocate_coeff(60);

    /* Read the pattern values */

    unpack_coeffs(coeff_stream);

#if defined(USE_ZIP_COEFFS_FILE) && (USE_ZIP_COEFFS_FILE != 0)
    gzclose(coeff_stream);
#else
    fclose(coeff_stream);
#endif

#if 0
    if (g_save_stream != NULL) {
        fclose(g_save_stream);
        g_save_stream = NULL;
    }
#endif

    /* Calculate the patterns which correspond to the board being filled */

    terminal_patterns();
    coeffs[60].constant = 0;
    coeffs[60].parity = 0;
    coeffs[60].parity_constant[0] = coeffs[60].constant;
    coeffs[60].parity_constant[1] = coeffs[60].constant + coeffs[60].parity;

    /* Adjust the coefficients so as to reflect the encouragement for
    having lots of discs */

    adjust_stream = fopen("adjust.txt", "r");
    if (adjust_stream != NULL) {
        double disc_adjust = 0.0;
        double edge_adjust = 0.0;
        double corner_adjust = 0.0;
        double x_adjust = 0.0;

        fscanf(adjust_stream, "%lf %lf %lf %lf", &disc_adjust, &edge_adjust,
            &corner_adjust, &x_adjust);
        eval_adjustment(disc_adjust, edge_adjust, corner_adjust, x_adjust);
        fclose(adjust_stream);
    }

    /* For each of number of disks played, decide on what set of evaluation
    patterns to use.
    The following rules apply:
    - Stages from the tuning are used as evaluation stages
    - Intermediate evaluation stages are introduced 2 stages before
    each tuning stage.
    - Other stages are mapped onto the next evaluation stage
    (which may be either from the tuning or an intermediate stage).
    */

    for (i = 0; i < stage[0]; i++)
        eval_map[i] = stage[0];
    for (i = 0; i < stage_count; i++)
        eval_map[stage[i]] = stage[i];

    for (i = subsequent_stage = 60; i >= stage[0]; i--) {
        if (eval_map[i] == i)
            subsequent_stage = i;
        else if (i == subsequent_stage - 2) {
            eval_map[i] = i;
            subsequent_stage = i;
        }
        else {
            eval_map[i] = subsequent_stage;
        }
    }

    //system( "pause" );
}

/*
   INTI_DISC_SET_TABLE
*/

void
init_disc_set_table(void) {
    int i, j, p;
    unsigned int bit, power, val;
    for (i = 0; i < 16; i++) {
        bit = i;
        power = (1 << 24);
        val = 0;
        for (p = 0; p < 4; p++) {
            if (bit & 0x08)
                val |= power;
            bit <<= 1;
            power >>= 8;
        }
        disc_set_tableb[i] = val;
        disc_set_tablew[i] = val << 1;
    }

    for (i = 0; i < 16; i++) {
        int k = i * 16;
        for (j = 0; j < 16; j++, k++) {
            int bits = disc_set_tableb[i] | disc_set_tablew[j];
            disc_set_table_bw[k] = bits;
            disc_set_table_bw32[k * 4] = bits & 0x03;
            disc_set_table_bw32[k * 4 + 1] = (bits >> 8) & 0x03;
            disc_set_table_bw32[k * 4 + 2] = (bits >> 16) & 0x03;
            disc_set_table_bw32[k * 4 + 3] = (bits >> 24) & 0x03;
        }
    }
}

/*
   GEN_BYTE_BOARD_FROM_BB0
   Transform board from bitboard.
*/

INLINE void
gen_byte_board_from_bb0(const BitBoard my_bits,
    const BitBoard opp_bits,
    unsigned char *my_board,
    int color) {
    register unsigned int bbits, wbits;
    register unsigned int mask;
    unsigned int *pb;
    BitBoard empty_bits;
    BitBoard *black_bits, *white_bits;

    //BITBOARD_FULL_OR( empty_bits, my_bits, opp_bits );
    //BITBOARD_NOT( empty_bits );

    BITBOARD_FULL_NOTOR(empty_bits, my_bits, opp_bits);

    if (color == CHESS_BLACK) {
        black_bits = (BitBoard *)&empty_bits;
        white_bits = (BitBoard *)&opp_bits;
    }
    else {
        black_bits = (BitBoard *)&empty_bits;
        white_bits = (BitBoard *)&my_bits;
    }

    // board
    pb = (unsigned int *)my_board;

    // low bits
    bbits = black_bits->low;
    wbits = white_bits->low;

    mask = ((bbits & 0x0000000f) << 4) | (wbits & 0x0000000f);
    *pb = disc_set_table_bw[mask];
    pb++;
    mask = ((bbits & 0x000000f0)) | ((wbits & 0x000000f0) >> 4);
    *pb = disc_set_table_bw[mask];
    pb++;

    mask = ((bbits & 0x00000f00) >> 4) | ((wbits & 0x00000f00) >> 8);
    *pb = disc_set_table_bw[mask];
    pb++;
    mask = ((bbits & 0x0000f000) >> 8) | ((wbits & 0x0000f000) >> 12);
    *pb = disc_set_table_bw[mask];
    pb++;

    mask = ((bbits & 0x000f0000) >> 12) | ((wbits & 0x000f0000) >> 16);
    *pb = disc_set_table_bw[mask];
    pb++;
    mask = ((bbits & 0x00f00000) >> 16) | ((wbits & 0x00f00000) >> 20);
    *pb = disc_set_table_bw[mask];
    pb++;

    mask = ((bbits & 0x0f000000) >> 20) | ((wbits & 0x0f000000) >> 24);
    *pb = disc_set_table_bw[mask];
    pb++;
    mask = ((bbits & 0xf0000000) >> 24) | ((wbits & 0xf0000000) >> 28);
    *pb = disc_set_table_bw[mask];
    pb++;

    // high bits
    bbits = black_bits->high;
    wbits = white_bits->high;

    mask = ((bbits & 0x0000000f) << 4) | (wbits & 0x0000000f);
    *pb = disc_set_table_bw[mask];
    pb++;
    mask = ((bbits & 0x000000f0)) | ((wbits & 0x000000f0) >> 4);
    *pb = disc_set_table_bw[mask];
    pb++;

    mask = ((bbits & 0x00000f00) >> 4) | ((wbits & 0x00000f00) >> 8);
    *pb = disc_set_table_bw[mask];
    pb++;
    mask = ((bbits & 0x0000f000) >> 8) | ((wbits & 0x0000f000) >> 12);
    *pb = disc_set_table_bw[mask];
    pb++;

    mask = ((bbits & 0x000f0000) >> 12) | ((wbits & 0x000f0000) >> 16);
    *pb = disc_set_table_bw[mask];
    pb++;
    mask = ((bbits & 0x00f00000) >> 16) | ((wbits & 0x00f00000) >> 20);
    *pb = disc_set_table_bw[mask];
    pb++;

    mask = ((bbits & 0x0f000000) >> 20) | ((wbits & 0x0f000000) >> 24);
    *pb = disc_set_table_bw[mask];
    pb++;
    mask = ((bbits & 0xf0000000) >> 24) | ((wbits & 0xf0000000) >> 28);
    *pb = disc_set_table_bw[mask];
    pb++;
}

/*
   GEN_BYTE_BOARD_FROM_BB1
   Transform board from bitboard.
*/

INLINE void
gen_byte_board_from_bb1(const BitBoard my_bits,
    const BitBoard opp_bits,
    unsigned char *my_board,
    int color) {
    unsigned int bbits, wbits;
    unsigned int mask1, mask2;
    unsigned int *pb;
    BitBoard empty_bits;
    BitBoard *black_bits, *white_bits;

    BITBOARD_FULL_NOTOR(empty_bits, my_bits, opp_bits);

    if (color == CHESS_BLACK) {
        black_bits = (BitBoard *)&empty_bits;
        white_bits = (BitBoard *)&opp_bits;
    }
    else {
        black_bits = (BitBoard *)&empty_bits;
        white_bits = (BitBoard *)&my_bits;
    }

    // board
    pb = (unsigned int *)my_board;

    // low bits
    bbits = black_bits->low;
    wbits = white_bits->low;

    mask1 = ((bbits & 0x0000000f) << 4) | (wbits & 0x0000000f);
    mask2 = ((bbits & 0x000000f0)) | ((wbits & 0x000000f0) >> 4);
    *pb = disc_set_table_bw[mask1];
    pb++;
    *pb = disc_set_table_bw[mask2];
    pb++;

    mask1 = ((bbits & 0x00000f00) >> 4) | ((wbits & 0x00000f00) >> 8);
    mask2 = ((bbits & 0x0000f000) >> 8) | ((wbits & 0x0000f000) >> 12);
    *pb = disc_set_table_bw[mask1];
    pb++;
    *pb = disc_set_table_bw[mask2];
    pb++;

    mask1 = ((bbits & 0x000f0000) >> 12) | ((wbits & 0x000f0000) >> 16);
    mask2 = ((bbits & 0x00f00000) >> 16) | ((wbits & 0x00f00000) >> 20);
    *pb = disc_set_table_bw[mask1];
    pb++;
    *pb = disc_set_table_bw[mask2];
    pb++;

    mask1 = ((bbits & 0x0f000000) >> 20) | ((wbits & 0x0f000000) >> 24);
    mask2 = ((bbits & 0xf0000000) >> 24) | ((wbits & 0xf0000000) >> 28);
    *pb = disc_set_table_bw[mask1];
    pb++;
    *pb = disc_set_table_bw[mask2];
    pb++;

    // high bits
    bbits = black_bits->high;
    wbits = white_bits->high;

    mask1 = ((bbits & 0x0000000f) << 4) | (wbits & 0x0000000f);
    mask2 = ((bbits & 0x000000f0)) | ((wbits & 0x000000f0) >> 4);
    *pb = disc_set_table_bw[mask1];
    pb++;
    *pb = disc_set_table_bw[mask2];
    pb++;

    mask1 = ((bbits & 0x00000f00) >> 4) | ((wbits & 0x00000f00) >> 8);
    mask2 = ((bbits & 0x0000f000) >> 8) | ((wbits & 0x0000f000) >> 12);
    *pb = disc_set_table_bw[mask1];
    pb++;
    *pb = disc_set_table_bw[mask2];
    pb++;

    mask1 = ((bbits & 0x000f0000) >> 12) | ((wbits & 0x000f0000) >> 16);
    mask2 = ((bbits & 0x00f00000) >> 16) | ((wbits & 0x00f00000) >> 20);
    *pb = disc_set_table_bw[mask1];
    pb++;
    *pb = disc_set_table_bw[mask2];
    pb++;

    mask1 = ((bbits & 0x0f000000) >> 20) | ((wbits & 0x0f000000) >> 24);
    mask2 = ((bbits & 0xf0000000) >> 24) | ((wbits & 0xf0000000) >> 28);
    *pb = disc_set_table_bw[mask1];
    pb++;
    *pb = disc_set_table_bw[mask2];
    pb++;
}

/*
   GEN_BYTE_BOARD_FROM_BB
   Transform board from bitboard.
*/

INLINE void
gen_byte_board_from_bb(const BitBoard my_bits,
    const BitBoard opp_bits,
    unsigned char *my_board,
    int color) {
    register unsigned int bits;
    register unsigned int mask;
    unsigned int *pbi;
    BitBoard empty_bits;
    BitBoard *black_bits, *white_bits;

    //BITBOARD_FULL_OR( empty_bits, my_bits, opp_bits );
    //BITBOARD_NOT( empty_bits );

    BITBOARD_FULL_NOTOR(empty_bits, my_bits, opp_bits);

    if (color == CHESS_BLACK) {
        black_bits = (BitBoard *)&empty_bits;
        white_bits = (BitBoard *)&opp_bits;
    }
    else {
        black_bits = (BitBoard *)&empty_bits;
        white_bits = (BitBoard *)&my_bits;
    }

    // Black
    pbi = (unsigned int *)my_board;

    // low bits
    bits = black_bits->low;

    mask = (bits & 0x0000000f);
    *pbi = disc_set_tableb[mask];
    pbi++;
    mask = (bits & 0x000000f0) >> 4;
    *pbi = disc_set_tableb[mask];
    pbi++;

    mask = (bits & 0x00000f00) >> 8;
    *pbi = disc_set_tableb[mask];
    pbi++;
    mask = (bits & 0x0000f000) >> 12;
    *pbi = disc_set_tableb[mask];
    pbi++;

    mask = (bits & 0x000f0000) >> 16;
    *pbi = disc_set_tableb[mask];
    pbi++;
    mask = (bits & 0x00f00000) >> 20;
    *pbi = disc_set_tableb[mask];
    pbi++;

    mask = (bits & 0x0f000000) >> 24;
    *pbi = disc_set_tableb[mask];
    pbi++;
    mask = (bits & 0xf0000000) >> 28;
    *pbi = disc_set_tableb[mask];
    pbi++;

    // high bits
    bits = black_bits->high;

    mask = (bits & 0x0000000f);
    *pbi = disc_set_tableb[mask];
    pbi++;
    mask = (bits & 0x000000f0) >> 4;
    *pbi = disc_set_tableb[mask];
    pbi++;

    mask = (bits & 0x00000f00) >> 8;
    *pbi = disc_set_tableb[mask];
    pbi++;
    mask = (bits & 0x0000f000) >> 12;
    *pbi = disc_set_tableb[mask];
    pbi++;

    mask = (bits & 0x000f0000) >> 16;
    *pbi = disc_set_tableb[mask];
    pbi++;
    mask = (bits & 0x00f00000) >> 20;
    *pbi = disc_set_tableb[mask];
    pbi++;

    mask = (bits & 0x0f000000) >> 24;
    *pbi = disc_set_tableb[mask];
    pbi++;
    mask = (bits & 0xf0000000) >> 28;
    *pbi = disc_set_tableb[mask];

    // White
    pbi = (unsigned int *)my_board;

    // low bits
    bits = white_bits->low;

    mask = (bits & 0x0000000f);
    *pbi |= disc_set_tablew[mask];
    pbi++;
    mask = (bits & 0x000000f0) >> 4;
    *pbi |= disc_set_tablew[mask];
    pbi++;

    mask = (bits & 0x00000f00) >> 8;
    *pbi |= disc_set_tablew[mask];
    pbi++;
    mask = (bits & 0x0000f000) >> 12;
    *pbi |= disc_set_tablew[mask];
    pbi++;

    mask = (bits & 0x000f0000) >> 16;
    *pbi |= disc_set_tablew[mask];
    pbi++;
    mask = (bits & 0x00f00000) >> 20;
    *pbi |= disc_set_tablew[mask];
    pbi++;

    mask = (bits & 0x0f000000) >> 24;
    *pbi |= disc_set_tablew[mask];
    pbi++;
    mask = (bits & 0xf0000000) >> 28;
    *pbi |= disc_set_tablew[mask];
    pbi++;

    // high bits
    bits = white_bits->high;

    mask = (bits & 0x0000000f);
    *pbi |= disc_set_tablew[mask];
    pbi++;
    mask = (bits & 0x000000f0) >> 4;
    *pbi |= disc_set_tablew[mask];
    pbi++;

    mask = (bits & 0x00000f00) >> 8;
    *pbi |= disc_set_tablew[mask];
    pbi++;
    mask = (bits & 0x0000f000) >> 12;
    *pbi |= disc_set_tablew[mask];
    pbi++;

    mask = (bits & 0x000f0000) >> 16;
    *pbi |= disc_set_tablew[mask];
    pbi++;
    mask = (bits & 0x00f00000) >> 20;
    *pbi |= disc_set_tablew[mask];
    pbi++;

    mask = (bits & 0x0f000000) >> 24;
    *pbi |= disc_set_tablew[mask];
    pbi++;
    mask = (bits & 0xf0000000) >> 28;
    *pbi |= disc_set_tablew[mask];
}

/*
   GEN_BYTE_BOARD_FROM_BB2
   Transform board from bitboard.
*/

INLINE void
gen_byte_board_from_bb2(const BitBoard my_bits,
    const BitBoard opp_bits,
    unsigned char *my_board,
    int color) {
    register unsigned int bits;
    register int *pbi;
    BitBoard empty_bits;
    BitBoard *BlackBits, *WhiteBits;

    //BITBOARD_FULL_OR( empty_bits, my_bits, opp_bits );
    //BITBOARD_NOT( empty_bits );

    BITBOARD_FULL_NOTOR(empty_bits, my_bits, opp_bits);

    if (color == CHESS_BLACK) {
        BlackBits = (BitBoard *)&empty_bits;
        WhiteBits = (BitBoard *)&opp_bits;
    }
    else {
        BlackBits = (BitBoard *)&empty_bits;
        WhiteBits = (BitBoard *)&my_bits;
    }

    // Black
    pbi = (int *)my_board;

    // low bits
    bits = BlackBits->low;

    *pbi = ((bits & 0x00000001)) | ((bits & 0x00000002) << 7) | ((bits & 0x00000004) << 14) | ((bits & 0x00000008) << 21);
    pbi++;
    *pbi = ((bits & 0x00000010) >> 4) | ((bits & 0x00000020) << 3) | ((bits & 0x00000040) << 10) | ((bits & 0x00000080) << 17);
    pbi++;

    *pbi = ((bits & 0x00000100) >> 8) | ((bits & 0x00000200) >> 1) | ((bits & 0x00000400) << 6) | ((bits & 0x00000800) << 13);
    pbi++;
    *pbi = ((bits & 0x00001000) >> 12) | ((bits & 0x00002000) >> 5) | ((bits & 0x00004000) << 2) | ((bits & 0x00008000) << 9);
    pbi++;

    *pbi = ((bits & 0x00010000) >> 16) | ((bits & 0x00020000) >> 9) | ((bits & 0x00040000) >> 2) | ((bits & 0x00080000) << 5);
    pbi++;
    *pbi = ((bits & 0x00100000) >> 20) | ((bits & 0x00200000) >> 13) | ((bits & 0x00400000) >> 6) | ((bits & 0x00800000) << 1);
    pbi++;

    *pbi = ((bits & 0x01000000) >> 24) | ((bits & 0x02000000) >> 17) | ((bits & 0x04000000) >> 10) | ((bits & 0x08000000) >> 3);
    pbi++;
    *pbi = ((bits & 0x10000000) >> 28) | ((bits & 0x20000000) >> 21) | ((bits & 0x40000000) >> 14) | ((bits & 0x80000000) >> 7);
    pbi++;

    // high bits
    bits = BlackBits->high;

    *pbi = ((bits & 0x00000001)) | ((bits & 0x00000002) << 7) | ((bits & 0x00000004) << 14) | ((bits & 0x00000008) << 21);
    pbi++;
    *pbi = ((bits & 0x00000010) >> 4) | ((bits & 0x00000020) << 3) | ((bits & 0x00000040) << 10) | ((bits & 0x00000080) << 17);
    pbi++;

    *pbi = ((bits & 0x00000100) >> 8) | ((bits & 0x00000200) >> 1) | ((bits & 0x00000400) << 6) | ((bits & 0x00000800) << 13);
    pbi++;
    *pbi = ((bits & 0x00001000) >> 12) | ((bits & 0x00002000) >> 5) | ((bits & 0x00004000) << 2) | ((bits & 0x00008000) << 9);
    pbi++;

    *pbi = ((bits & 0x00010000) >> 16) | ((bits & 0x00020000) >> 9) | ((bits & 0x00040000) >> 2) | ((bits & 0x00080000) << 5);
    pbi++;
    *pbi = ((bits & 0x00100000) >> 20) | ((bits & 0x00200000) >> 13) | ((bits & 0x00400000) >> 6) | ((bits & 0x00800000) << 1);
    pbi++;

    *pbi = ((bits & 0x01000000) >> 24) | ((bits & 0x02000000) >> 17) | ((bits & 0x04000000) >> 10) | ((bits & 0x08000000) >> 3);
    pbi++;
    *pbi = ((bits & 0x10000000) >> 28) | ((bits & 0x20000000) >> 21) | ((bits & 0x40000000) >> 14) | ((bits & 0x80000000) >> 7);

    // White
    pbi = (int *)my_board;

    // low bits
    bits = WhiteBits->low;

    *pbi |= ((bits & 0x00000001) << 1) | ((bits & 0x00000002) << 8) | ((bits & 0x00000004) << 15) | ((bits & 0x00000008) << 22);
    pbi++;
    *pbi |= ((bits & 0x00000010) >> 3) | ((bits & 0x00000020) << 4) | ((bits & 0x00000040) << 11) | ((bits & 0x00000080) << 18);
    pbi++;

    *pbi |= ((bits & 0x00000100) >> 7) | ((bits & 0x00000200)) | ((bits & 0x00000400) << 7) | ((bits & 0x00000800) << 14);
    pbi++;
    *pbi |= ((bits & 0x00001000) >> 11) | ((bits & 0x00002000) >> 4) | ((bits & 0x00004000) << 3) | ((bits & 0x00008000) << 10);
    pbi++;

    *pbi |= ((bits & 0x00010000) >> 15) | ((bits & 0x00020000) >> 8) | ((bits & 0x00040000) >> 1) | ((bits & 0x00080000) << 6);
    pbi++;
    *pbi |= ((bits & 0x00100000) >> 19) | ((bits & 0x00200000) >> 12) | ((bits & 0x00400000) >> 5) | ((bits & 0x00800000) << 2);
    pbi++;

    *pbi |= ((bits & 0x01000000) >> 23) | ((bits & 0x02000000) >> 16) | ((bits & 0x04000000) >> 9) | ((bits & 0x08000000) >> 2);
    pbi++;
    *pbi |= ((bits & 0x10000000) >> 27) | ((bits & 0x20000000) >> 20) | ((bits & 0x40000000) >> 13) | ((bits & 0x80000000) >> 6);
    pbi++;

    // high bits
    bits = WhiteBits->high;

    *pbi |= ((bits & 0x00000001) << 1) | ((bits & 0x00000002) << 8) | ((bits & 0x00000004) << 15) | ((bits & 0x00000008) << 22);
    pbi++;
    *pbi |= ((bits & 0x00000010) >> 3) | ((bits & 0x00000020) << 4) | ((bits & 0x00000040) << 11) | ((bits & 0x00000080) << 18);
    pbi++;

    *pbi |= ((bits & 0x00000100) >> 7) | ((bits & 0x00000200)) | ((bits & 0x00000400) << 7) | ((bits & 0x00000800) << 14);
    pbi++;
    *pbi |= ((bits & 0x00001000) >> 11) | ((bits & 0x00002000) >> 4) | ((bits & 0x00004000) << 3) | ((bits & 0x00008000) << 10);
    pbi++;

    *pbi |= ((bits & 0x00010000) >> 15) | ((bits & 0x00020000) >> 8) | ((bits & 0x00040000) >> 1) | ((bits & 0x00080000) << 6);
    pbi++;
    *pbi |= ((bits & 0x00100000) >> 19) | ((bits & 0x00200000) >> 12) | ((bits & 0x00400000) >> 5) | ((bits & 0x00800000) << 2);
    pbi++;

    *pbi |= ((bits & 0x01000000) >> 23) | ((bits & 0x02000000) >> 16) | ((bits & 0x04000000) >> 9) | ((bits & 0x08000000) >> 2);
    pbi++;
    *pbi |= ((bits & 0x10000000) >> 27) | ((bits & 0x20000000) >> 20) | ((bits & 0x40000000) >> 13) | ((bits & 0x80000000) >> 6);
}

/*
   GEN_BYTE_BOARD_FROM_BB3
   Transform board from bitboard.
*/

INLINE void
gen_byte_board_from_bb3(const BitBoard my_bits,
    const BitBoard opp_bits,
    unsigned char *my_board,
    int color) {
    register unsigned int bits1, bits2;
    register int *pbi;
    BitBoard empty_bits;
    BitBoard *BlackBits, *WhiteBits;

    //BITBOARD_FULL_OR( empty_bits, my_bits, opp_bits );
    //BITBOARD_NOT( empty_bits );

    BITBOARD_FULL_NOTOR(empty_bits, my_bits, opp_bits);

    if (color == CHESS_BLACK) {
        BlackBits = (BitBoard *)&empty_bits;
        WhiteBits = (BitBoard *)&opp_bits;
    }
    else {
        BlackBits = (BitBoard *)&empty_bits;
        WhiteBits = (BitBoard *)&my_bits;
    }

    // Black
    pbi = (int *)my_board;

    // low bits
    bits1 = bits2 = BlackBits->low;

    *pbi = ((bits1 & 0x00000001)) | ((bits2 & 0x00000002) << 7) | ((bits1 & 0x00000004) << 14) | ((bits2 & 0x00000008) << 21);
    pbi++;
    *pbi = ((bits1 & 0x00000010) >> 4) | ((bits2 & 0x00000020) << 3) | ((bits1 & 0x00000040) << 10) | ((bits2 & 0x00000080) << 17);
    pbi++;

    *pbi = ((bits1 & 0x00000100) >> 8) | ((bits2 & 0x00000200) >> 1) | ((bits1 & 0x00000400) << 6) | ((bits2 & 0x00000800) << 13);
    pbi++;
    *pbi = ((bits1 & 0x00001000) >> 12) | ((bits2 & 0x00002000) >> 5) | ((bits1 & 0x00004000) << 2) | ((bits2 & 0x00008000) << 9);
    pbi++;

    *pbi = ((bits1 & 0x00010000) >> 16) | ((bits2 & 0x00020000) >> 9) | ((bits1 & 0x00040000) >> 2) | ((bits2 & 0x00080000) << 5);
    pbi++;
    *pbi = ((bits1 & 0x00100000) >> 20) | ((bits2 & 0x00200000) >> 13) | ((bits1 & 0x00400000) >> 6) | ((bits2 & 0x00800000) << 1);
    pbi++;

    *pbi = ((bits1 & 0x01000000) >> 24) | ((bits2 & 0x02000000) >> 17) | ((bits1 & 0x04000000) >> 10) | ((bits2 & 0x08000000) >> 3);
    pbi++;
    *pbi = ((bits1 & 0x10000000) >> 28) | ((bits2 & 0x20000000) >> 21) | ((bits1 & 0x40000000) >> 14) | ((bits2 & 0x80000000) >> 7);
    pbi++;

    // high bits
    bits1 = bits2 = BlackBits->high;

    *pbi = ((bits1 & 0x00000001)) | ((bits2 & 0x00000002) << 7) | ((bits1 & 0x00000004) << 14) | ((bits2 & 0x00000008) << 21);
    pbi++;
    *pbi = ((bits1 & 0x00000010) >> 4) | ((bits2 & 0x00000020) << 3) | ((bits1 & 0x00000040) << 10) | ((bits2 & 0x00000080) << 17);
    pbi++;

    *pbi = ((bits1 & 0x00000100) >> 8) | ((bits2 & 0x00000200) >> 1) | ((bits1 & 0x00000400) << 6) | ((bits2 & 0x00000800) << 13);
    pbi++;
    *pbi = ((bits1 & 0x00001000) >> 12) | ((bits2 & 0x00002000) >> 5) | ((bits1 & 0x00004000) << 2) | ((bits2 & 0x00008000) << 9);
    pbi++;

    *pbi = ((bits1 & 0x00010000) >> 16) | ((bits2 & 0x00020000) >> 9) | ((bits1 & 0x00040000) >> 2) | ((bits2 & 0x00080000) << 5);
    pbi++;
    *pbi = ((bits1 & 0x00100000) >> 20) | ((bits2 & 0x00200000) >> 13) | ((bits1 & 0x00400000) >> 6) | ((bits2 & 0x00800000) << 1);
    pbi++;

    *pbi = ((bits1 & 0x01000000) >> 24) | ((bits2 & 0x02000000) >> 17) | ((bits1 & 0x04000000) >> 10) | ((bits2 & 0x08000000) >> 3);
    pbi++;
    *pbi = ((bits1 & 0x10000000) >> 28) | ((bits2 & 0x20000000) >> 21) | ((bits1 & 0x40000000) >> 14) | ((bits2 & 0x80000000) >> 7);

    // White
    pbi = (int *)my_board;

    // low bits
    bits1 = bits2 = WhiteBits->low;

    *pbi |= ((bits1 & 0x00000001) << 1) | ((bits2 & 0x00000002) << 8) | ((bits1 & 0x00000004) << 15) | ((bits2 & 0x00000008) << 22);
    pbi++;
    *pbi |= ((bits1 & 0x00000010) >> 3) | ((bits2 & 0x00000020) << 4) | ((bits1 & 0x00000040) << 11) | ((bits2 & 0x00000080) << 18);
    pbi++;

    *pbi |= ((bits1 & 0x00000100) >> 7) | ((bits2 & 0x00000200)) | ((bits1 & 0x00000400) << 7) | ((bits2 & 0x00000800) << 14);
    pbi++;
    *pbi |= ((bits1 & 0x00001000) >> 11) | ((bits2 & 0x00002000) >> 4) | ((bits1 & 0x00004000) << 3) | ((bits2 & 0x00008000) << 10);
    pbi++;

    *pbi |= ((bits1 & 0x00010000) >> 15) | ((bits2 & 0x00020000) >> 8) | ((bits1 & 0x00040000) >> 1) | ((bits2 & 0x00080000) << 6);
    pbi++;
    *pbi |= ((bits1 & 0x00100000) >> 19) | ((bits2 & 0x00200000) >> 12) | ((bits1 & 0x00400000) >> 5) | ((bits2 & 0x00800000) << 2);
    pbi++;

    *pbi |= ((bits1 & 0x01000000) >> 23) | ((bits2 & 0x02000000) >> 16) | ((bits1 & 0x04000000) >> 9) | ((bits2 & 0x08000000) >> 2);
    pbi++;
    *pbi |= ((bits1 & 0x10000000) >> 27) | ((bits2 & 0x20000000) >> 20) | ((bits1 & 0x40000000) >> 13) | ((bits2 & 0x80000000) >> 6);
    pbi++;

    // high bits
    bits1 = bits2 = WhiteBits->high;

    *pbi |= ((bits1 & 0x00000001) << 1) | ((bits2 & 0x00000002) << 8) | ((bits1 & 0x00000004) << 15) | ((bits2 & 0x00000008) << 22);
    pbi++;
    *pbi |= ((bits1 & 0x00000010) >> 3) | ((bits2 & 0x00000020) << 4) | ((bits1 & 0x00000040) << 11) | ((bits2 & 0x00000080) << 18);
    pbi++;

    *pbi |= ((bits1 & 0x00000100) >> 7) | ((bits2 & 0x00000200)) | ((bits1 & 0x00000400) << 7) | ((bits2 & 0x00000800) << 14);
    pbi++;
    *pbi |= ((bits1 & 0x00001000) >> 11) | ((bits2 & 0x00002000) >> 4) | ((bits1 & 0x00004000) << 3) | ((bits2 & 0x00008000) << 10);
    pbi++;

    *pbi |= ((bits1 & 0x00010000) >> 15) | ((bits2 & 0x00020000) >> 8) | ((bits1 & 0x00040000) >> 1) | ((bits2 & 0x00080000) << 6);
    pbi++;
    *pbi |= ((bits1 & 0x00100000) >> 19) | ((bits2 & 0x00200000) >> 12) | ((bits1 & 0x00400000) >> 5) | ((bits2 & 0x00800000) << 2);
    pbi++;

    *pbi |= ((bits1 & 0x01000000) >> 23) | ((bits2 & 0x02000000) >> 16) | ((bits1 & 0x04000000) >> 9) | ((bits2 & 0x08000000) >> 2);
    pbi++;
    *pbi |= ((bits1 & 0x10000000) >> 27) | ((bits2 & 0x20000000) >> 20) | ((bits1 & 0x40000000) >> 13) | ((bits2 & 0x80000000) >> 6);
}

/*
   GEN_BOARD_FROM_BB4
   Transform board from bitboard.
*/
INLINE void
gen_byte_board_from_bb4(const BitBoard my_bits,
    const BitBoard opp_bits,
    unsigned char *my_board,
    int color) {
    int i, mask;
    int bbits, wbits;
    BitBoard *pb, *pw;

    if (color == CHESS_BLACK) {
        pb = (BitBoard *)&my_bits;
        pw = (BitBoard *)&opp_bits;
    }
    else {
        pb = (BitBoard *)&opp_bits;
        pw = (BitBoard *)&my_bits;
    }

    // Low
    mask = 1;
    bbits = pb->low;
    wbits = pw->low;
    for (i = 0; i < 32; i++) {
        if (bbits & mask)
            my_board[i] = CHESS_BLACK;
        else if (wbits & mask)
            my_board[i] = CHESS_WHITE;
        else
            my_board[i] = CHESS_EMPTY;
        mask <<= 1;
    }

    // High
    mask = 1;
    bbits = pb->high;
    wbits = pw->high;
    for (i = 32; i < 64; i++) {
        if (bbits & mask)
            my_board[i] = CHESS_BLACK;
        else if (wbits & mask)
            my_board[i] = CHESS_WHITE;
        else
            my_board[i] = CHESS_EMPTY;
        mask <<= 1;
    }
}

/*
   GEN_BOARD_FROM_BB
   Transform board from bitboard.
*/
INLINE void
gen_board_from_bb(const BitBoard my_bits,
    const BitBoard opp_bits,
    unsigned int *my_board,
    int color) {
    int i, mask;
    int bbits, wbits;
    BitBoard *pb, *pw;

    if (color == CHESS_BLACK) {
        pb = (BitBoard *)&my_bits;
        pw = (BitBoard *)&opp_bits;
    }
    else {
        pb = (BitBoard *)&opp_bits;
        pw = (BitBoard *)&my_bits;
    }

    // Low
    mask = 1;
    bbits = pb->low;
    wbits = pw->low;
    for (i = 0; i < 32; i++) {
        if (bbits & mask)
            my_board[i] = CHESS_BLACK;
        else if (wbits & mask)
            my_board[i] = CHESS_WHITE;
        else
            my_board[i] = CHESS_EMPTY;
        mask <<= 1;
    }

    // High
    mask = 1;
    bbits = pb->high;
    wbits = pw->high;
    for (i = 32; i < 64; i++) {
        if (bbits & mask)
            my_board[i] = CHESS_BLACK;
        else if (wbits & mask)
            my_board[i] = CHESS_WHITE;
        else
            my_board[i] = CHESS_EMPTY;
        mask <<= 1;
    }
}

/*
   GEN_BOARD_FROM_BB1
   Transform board from bitboard.
*/
INLINE void
gen_board_from_bb1(const BitBoard my_bits,
    const BitBoard opp_bits,
    unsigned int *my_board,
    int color) {
    register unsigned int bbits, wbits;
    register unsigned int mask;
    BitBoard empty_bits;
    BitBoard *black_bits, *white_bits;
    unsigned int *pb;

    BITBOARD_FULL_NOTOR(empty_bits, my_bits, opp_bits);

    black_bits = (BitBoard *)&empty_bits;

    if (color == CHESS_BLACK)
        white_bits = (BitBoard *)&opp_bits;
    else
        white_bits = (BitBoard *)&my_bits;

    // board
    pb = (unsigned int *)my_board;

    // low bits
    bbits = black_bits->low;
    wbits = white_bits->low;

    mask = ((bbits & 0x0000000f) << 4) | (wbits & 0x0000000f);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;
    mask = ((bbits & 0x000000f0)) | ((wbits & 0x000000f0) >> 4);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;

    mask = ((bbits & 0x00000f00) >> 4) | ((wbits & 0x00000f00) >> 8);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;
    mask = ((bbits & 0x0000f000) >> 8) | ((wbits & 0x0000f000) >> 12);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;

    mask = ((bbits & 0x000f0000) >> 12) | ((wbits & 0x000f0000) >> 16);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;
    mask = ((bbits & 0x00f00000) >> 16) | ((wbits & 0x00f00000) >> 20);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;

    mask = ((bbits & 0x0f000000) >> 20) | ((wbits & 0x0f000000) >> 24);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;
    mask = ((bbits & 0xf0000000) >> 24) | ((wbits & 0xf0000000) >> 28);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;

    // high bits
    bbits = black_bits->high;
    wbits = white_bits->high;

    mask = ((bbits & 0x0000000f) << 4) | (wbits & 0x0000000f);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;
    mask = ((bbits & 0x000000f0)) | ((wbits & 0x000000f0) >> 4);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;

    mask = ((bbits & 0x00000f00) >> 4) | ((wbits & 0x00000f00) >> 8);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;
    mask = ((bbits & 0x0000f000) >> 8) | ((wbits & 0x0000f000) >> 12);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;

    mask = ((bbits & 0x000f0000) >> 12) | ((wbits & 0x000f0000) >> 16);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;
    mask = ((bbits & 0x00f00000) >> 16) | ((wbits & 0x00f00000) >> 20);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;

    mask = ((bbits & 0x0f000000) >> 20) | ((wbits & 0x0f000000) >> 24);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
    pb += 4;
    mask = ((bbits & 0xf0000000) >> 24) | ((wbits & 0xf0000000) >> 28);
    memcpy((void *)pb, (const void *)&disc_set_table_bw32[mask * 4], sizeof(unsigned int) * 4);
}

#if defined(_MSC_VER) && (_MSC_VER <= 1200)

//#define GEN_BOARD_USE_MMX_REG

#endif

/*
   GEN_BOARD_FROM_BB2
   Transform board from bitboard.
*/
INLINE void
gen_board_from_bb2(const BitBoard my_bits,
    const BitBoard opp_bits,
    unsigned int *my_board,
    int color) {
    BitBoard empty_bits;
    BitBoard *black_bits, *white_bits;

#if defined(_MSC_VER) && (_MSC_VER <= 1200)

#ifdef GEN_BOARD_USE_MMX_REG
    static ALIGN_PREFIX(16) const unsigned __int64 s_shift_bits ALIGN_SUFFIX(16) = 0x0000000080000001UI64;
    static ALIGN_PREFIX(16) const unsigned __int64 s_discs_mask ALIGN_SUFFIX(16) = 0x0000000100000001UI64;
#else
    static ALIGN_PREFIX(16) const unsigned __int64 s_shift_bits_128[] ALIGN_SUFFIX(16) = {
        0x0000000080000001UI64, 0x0000000080000001UI64
    };
    static ALIGN_PREFIX(16) const unsigned __int64 s_discs_mask_128[] ALIGN_SUFFIX(16) = {
        0x0000000100000001UI64, 0x0000000100000001UI64
    };
#endif

#else

#ifdef GEN_BOARD_USE_MMX_REG
    static ALIGN_PREFIX(16) const unsigned __int64 s_shift_bits ALIGN_SUFFIX(16) = 0x0000000080000001ULL;
    static ALIGN_PREFIX(16) const unsigned __int64 s_discs_mask ALIGN_SUFFIX(16) = 0x0000000100000001ULL;
#else
    static ALIGN_PREFIX(16) const unsigned __int64 s_shift_bits_128[] ALIGN_SUFFIX(16) = {
        0x0000000080000001ULL, 0x0000000080000001ULL
    };
    static ALIGN_PREFIX(16) const unsigned __int64 s_discs_mask_128[] ALIGN_SUFFIX(16) = {
        0x0000000100000001ULL, 0x0000000100000001ULL
    };
#endif

#endif  /* defined(_MSC_VER) && (_MSC_VER <= 1200) */

    BITBOARD_FULL_NOTOR(empty_bits, my_bits, opp_bits);

    black_bits = (BitBoard *)&empty_bits;

    if (color == CHESS_BLACK)
        white_bits = (BitBoard *)&opp_bits;
    else
        white_bits = (BitBoard *)&my_bits;

    __asm {
        mov         edi, dword ptr[my_board]

#ifdef GEN_BOARD_USE_MMX_REG
        movq        mm0, s_shift_bits; s_shift_bits = 0x0000000040000001
        movq        mm1, s_discs_mask; s_discs_mask = 0x0000000100000001
#endif
        mov         esi, edi

#ifdef GEN_BOARD_USE_MMX_REG
        movq2dq     xmm0, xmm0
        movq2dq     xmm1, xmm1
#else
        movdqa      xmm0, s_shift_bits_128
        movdqa      xmm1, s_discs_mask_128
#endif
        add         esi, 64

        punpcklqdq  xmm0, xmm0; s_shift_bits_128
        punpcklqdq  xmm1, xmm1; s_discs_mask_128

        ; extract BitBoard.low
        ; ------------------------------------------------------------------

        mov         eax, dword ptr[black_bits]
        mov         edx, dword ptr[white_bits]
        mov         ebx, dword ptr[eax + BitBoard.low]
        mov         ecx, dword ptr[edx + BitBoard.low]
        and         ebx, 0x0000FFFFUL; b_bits_low_low_l = black_bits.low & 0x0000FFFF;
        and         ecx, 0x0000FFFFUL; w_bits_low_low_l = white_bits.low & 0x0000FFFF;

        movd        xmm2, ebx; xmm2 <= = b_bits_low_low_l
        movd        xmm4, ecx; xmm4 <= = w_bits_low_low_l

        shr         ebx, 2; b_bits_low_low_h = b_bits_low_low_l >> 2;
        shr         ecx, 2; w_bits_low_low_h = w_bits_low_low_l >> 2;
        movd        xmm3, ebx; xmm3 <= = b_bits_low_low_h
        movd        xmm5, ecx; xmm5 <= = w_bits_low_low_h

        punpcklqdq  xmm2, xmm3; xmm2 <= = (b_bits_low_low_h << 64) | b_bits_low_low_l
        punpcklqdq  xmm4, xmm5; xmm4 <= = (w_bits_low_low_h << 64) | w_bits_low_low_l

        ; ------------------------------------------------------------------

        mov         ebx, dword ptr[eax + BitBoard.low]
        mov         ecx, dword ptr[edx + BitBoard.low]
        and         ebx, 0xFFFF0000UL; b_bits_low_high_l = black_bits.low & 0xFFFF0000;
        and         ecx, 0xFFFF0000UL; w_bits_low_high_l = white_bits.low & 0xFFFF0000;

        movd        xmm3, ebx; xmm3 <= = b_bits_low_high_l
        movd        xmm5, ecx; xmm5 <= = w_bits_low_high_l

        shr         ebx, 18; b_bits_low_high_h = b_bits_low_high_l >> 2;
        shr         ecx, 18; w_bits_low_high_h = w_bits_low_high_l >> 2;
        psrld       xmm3, 16
        psrld       xmm5, 16
        movd        xmm6, ebx; xmm6 <= = b_bits_low_high_h
        movd        xmm7, ecx; xmm7 <= = w_bits_low_high_h

        punpcklqdq  xmm3, xmm6; xmm3 <= = (b_bits_low_high_h << 64) | b_bits_low_high_l
        punpcklqdq  xmm5, xmm7; xmm5 <= = (w_bits_low_high_h << 64) | w_bits_low_high_l

        ; ------------------------------------------------------------------

        pmuludq     xmm2, xmm0; xmm2 = xmm2 * shift_bits_128
        pmuludq     xmm4, xmm0; xmm4 = xmm4 * shift_bits_128

        movdqa      xmm6, xmm2
        movdqa      xmm7, xmm4

        pmuludq     xmm3, xmm0; xmm3 = xmm3 * shift_bits_128
        pmuludq     xmm5, xmm0; xmm5 = xmm5 * shift_bits_128

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[edi], xmm6

        movdqa      xmm6, xmm3
        movdqa      xmm7, xmm5

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         edi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[esi], xmm6

        ; ------------------------------------------------------------------

        psrld       xmm2, 4
        psrld       xmm4, 4

        movdqa      xmm6, xmm2
        movdqa      xmm7, xmm4

        psrld       xmm3, 4
        psrld       xmm5, 4

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         esi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[edi], xmm6

        movdqa      xmm6, xmm3
        movdqa      xmm7, xmm5

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         edi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[esi], xmm6

        ; ------------------------------------------------------------------

        psrld       xmm2, 4
        psrld       xmm4, 4

        movdqa      xmm6, xmm2
        movdqa      xmm7, xmm4

        psrld       xmm3, 4
        psrld       xmm5, 4

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         esi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[edi], xmm6

        movdqa      xmm6, xmm3
        movdqa      xmm7, xmm5

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         edi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[esi], xmm6

        ; ------------------------------------------------------------------

        psrld       xmm2, 4
        psrld       xmm4, 4

        movdqa      xmm6, xmm2
        movdqa      xmm7, xmm4

        psrld       xmm3, 4
        psrld       xmm5, 4

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         esi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[edi], xmm6

        movdqa      xmm6, xmm3
        movdqa      xmm7, xmm5

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         edi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[esi], xmm6

        ; ------------------------------------------------------------------

        ; add edi 64 bytes and add esi 64 bytes

        ; extract BitBoard.high
        ; ------------------------------------------------------------------

        mov         eax, dword ptr[black_bits]
        mov         edx, dword ptr[white_bits]
        add         edi, 64
        add         esi, 80; 64 + 16 = 80
        mov         ebx, dword ptr[eax + BitBoard.high]
        mov         ecx, dword ptr[edx + BitBoard.high]
        and         ebx, 0x0000FFFFUL; b_bits_high_low_l = black_bits.high & 0x0000FFFF;
        and         ecx, 0x0000FFFFUL; w_bits_high_low_l = white_bits.high & 0x0000FFFF;

        movd        xmm2, ebx; xmm2 <= = b_bits_high_low_l
        movd        xmm4, ecx; xmm4 <= = w_bits_high_low_l

        shr         ebx, 2; b_bits_high_low_h = b_bits_high_low_l >> 2;
        shr         ecx, 2; w_bits_high_low_h = w_bits_high_low_l >> 2;
        movd        xmm3, ebx; xmm3 <= = b_bits_high_low_h
        movd        xmm5, ecx; xmm5 <= = w_bits_high_low_h

        punpcklqdq  xmm2, xmm3; xmm2 <= = (b_bits_high_low_h << 64) | b_bits_high_low_l
        punpcklqdq  xmm4, xmm5; xmm4 <= = (w_bits_high_low_h << 64) | w_bits_high_low_l

        ; ------------------------------------------------------------------

        mov         ebx, dword ptr[eax + BitBoard.high]
        mov         ecx, dword ptr[edx + BitBoard.high]
        and         ebx, 0xFFFF0000UL; b_bits_high_high_l = black_bits.high & 0xFFFF0000;
        and         ecx, 0xFFFF0000UL; w_bits_high_high_l = white_bits.high & 0xFFFF0000;

        movd        xmm3, ebx; xmm3 <= = b_bits_high_high_l
        movd        xmm5, ecx; xmm5 <= = w_bits_high_high_l

        shr         ebx, 18; b_bits_high_high_h = b_bits_high_high_l >> 2;
        shr         ecx, 18; w_bits_high_high_h = w_bits_high_high_l >> 2;
        psrld       xmm3, 16
        psrld       xmm5, 16
        movd        xmm6, ebx; xmm6 <= = b_bits_high_high_h
        movd        xmm7, ecx; xmm7 <= = w_bits_high_high_h

        punpcklqdq  xmm3, xmm6; xmm3 <= = (b_bits_high_high_h << 64) | b_bits_high_high_l
        punpcklqdq  xmm5, xmm7; xmm5 <= = (w_bits_high_high_h << 64) | w_bits_high_high_l

        ; ------------------------------------------------------------------

        pmuludq     xmm2, xmm0; xmm2 = xmm2 * shift_bits_128
        pmuludq     xmm4, xmm0; xmm4 = xmm4 * shift_bits_128

        movdqa      xmm6, xmm2
        movdqa      xmm7, xmm4

        pmuludq     xmm3, xmm0; xmm3 = xmm3 * shift_bits_128
        pmuludq     xmm5, xmm0; xmm5 = xmm5 * shift_bits_128

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[edi], xmm6

        movdqa      xmm6, xmm3
        movdqa      xmm7, xmm5

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         edi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[esi], xmm6

        ; ------------------------------------------------------------------

        psrld       xmm2, 4
        psrld       xmm4, 4

        movdqa      xmm6, xmm2
        movdqa      xmm7, xmm4

        psrld       xmm3, 4
        psrld       xmm5, 4

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         esi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[edi], xmm6

        movdqa      xmm6, xmm3
        movdqa      xmm7, xmm5

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         edi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[esi], xmm6

        ; ------------------------------------------------------------------

        psrld       xmm2, 4
        psrld       xmm4, 4

        movdqa      xmm6, xmm2
        movdqa      xmm7, xmm4

        psrld       xmm3, 4
        psrld       xmm5, 4

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         esi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[edi], xmm6

        movdqa      xmm6, xmm3
        movdqa      xmm7, xmm5

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         edi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[esi], xmm6

        ; ------------------------------------------------------------------

        psrld       xmm2, 4
        psrld       xmm4, 4

        movdqa      xmm6, xmm2
        movdqa      xmm7, xmm4

        psrld       xmm3, 4
        psrld       xmm5, 4

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         esi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[edi], xmm6

        movdqa      xmm6, xmm3
        movdqa      xmm7, xmm5

        pand        xmm6, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pand        xmm7, xmm1; 逻辑与 s_discs_mask, 取 disc_bits
        pslld       xmm7, 1
        add         edi, 16
        por         xmm6, xmm7; black_bits | (white_bits << 1);

        movdqa      xmmword ptr[esi], xmm6

        ; ------------------------------------------------------------------

#ifdef GEN_BOARD_USE_MMX_REG
        emms
#endif
    }

}

/*
   GEN_BOARD_FROM_BB3
   Transform board from bitboard.
*/
INLINE void
gen_board_from_bb3(const BitBoard my_bits,
    const BitBoard opp_bits,
    unsigned int *my_board,
    int color) {
    register unsigned int bit_b, bit_w;
    register unsigned int *pbi;
    BitBoard empty_bits;
    BitBoard *black_bits, *white_bits;

    BITBOARD_FULL_NOTOR(empty_bits, my_bits, opp_bits);

    black_bits = (BitBoard *)&empty_bits;

    if (color == CHESS_BLACK)
        white_bits = (BitBoard *)&opp_bits;
    else
        white_bits = (BitBoard *)&my_bits;

    pbi = (unsigned int *)my_board;

    // low bits
    bit_b = black_bits->low;
    bit_w = white_bits->low;

#if 0
    __asm {
        nop
        emms
    }
#endif

    ///////////////////////////////////////////////////////////////////
    // board[0]-board[7]
    bit_w <<= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);

    // board[8]-board[15]
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);

    // board[16]-board[23]
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);

    // board[24]-board[30]
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);

    // board[31]
    bit_b >>= 1;
    bit_w = (white_bits->low) >> 30;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);

    // high bits
    bit_b = black_bits->high;
    bit_w = white_bits->high;

    ///////////////////////////////////////////////////////////////////
    // board[32]-board[39]
    bit_w <<= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);

    // board[40]-board[47]
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);

    // board[48]-board[55]
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);

    // board[56]-board[62]
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);
    bit_b >>= 1;
    bit_w >>= 1;
    *pbi++ = (bit_b & 0x00000001) | (bit_w & 0x00000002);

    // board[63]
    bit_b >>= 1;
    bit_w = (white_bits->high) >> 30;
    *pbi = (bit_b & 0x00000001) | (bit_w & 0x00000002);
}

/*
   GEN_BOARD_FROM_BB4
   Transform board from bitboard.
*/
INLINE void
gen_board_from_bb4(const BitBoard my_bits,
    const BitBoard opp_bits,
    unsigned int *my_board,
    int color) {
    register unsigned int bit_b1, bit_w1, bit_b2, bit_w2;
    register unsigned int *pbi1, *pbi2;
    BitBoard empty_bits;
    BitBoard *black_bits, *white_bits;

    BITBOARD_FULL_NOTOR(empty_bits, my_bits, opp_bits);

    black_bits = (BitBoard *)&empty_bits;

    if (color == CHESS_BLACK)
        white_bits = (BitBoard *)&opp_bits;
    else
        white_bits = (BitBoard *)&my_bits;

    pbi1 = (unsigned int *)my_board;
    pbi2 = pbi1 + 1;

#if 0
    __asm {
        pause
        pause
        nop
        nop
        emms
    }
#endif

    ///////////////////////////////////////////////////////////////////
    // low bits
    ///////////////////////////////////////////////////////////////////

    bit_b1 = black_bits->low;
    bit_w1 = white_bits->low;
    bit_b2 = bit_b1 >> 1;
    bit_w2 = bit_w1 >> 1;

    ///////////////////////////////////////////////////////////////////
    // board[0]-board[7]
    bit_w1 <<= 1;
    bit_w2 <<= 1;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);

    // board[8]-board[15]
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);

    // board[16]-board[23]
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);

    // board[24]-board[30]
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);

    // board[30]-board[31]
    bit_b1 >>= 2;
    bit_b2 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_w1 = (white_bits->low) >> 29;
    bit_w2 = (white_bits->low) >> 30;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);

    ///////////////////////////////////////////////////////////////////
    // high bits
    ///////////////////////////////////////////////////////////////////

    bit_b1 = black_bits->high;
    bit_w1 = white_bits->high;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 = bit_b1 >> 1;
    bit_w2 = bit_w1 >> 1;

    ///////////////////////////////////////////////////////////////////
    // board[32]-board[39]
    bit_w1 <<= 1;
    bit_w2 <<= 1;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);

    // board[40]-board[47]
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);

    // board[48]-board[55]
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);

    // board[56]-board[62]
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
    bit_b1 >>= 2;
    bit_w1 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_b2 >>= 2;
    bit_w2 >>= 2;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);

    // board[30]-board[31]
    bit_b1 >>= 2;
    bit_b2 >>= 2;
    pbi1 += 2;
    pbi2 += 2;
    bit_w1 = (white_bits->high) >> 29;
    bit_w2 = (white_bits->high) >> 30;
    *pbi1 = (bit_b1 & 0x00000001) | (bit_w1 & 0x00000002);
    *pbi2 = (bit_b2 & 0x00000001) | (bit_w2 & 0x00000002);
}

int board_is_equal32(unsigned int *a_board, unsigned int *b_board)
{
    int i;
    int unequal_pos = -1;
    for (i = 0; i < 64; ++i) {
        if (a_board[i] != b_board[i]) {
            unequal_pos = i;
            break;
        }
    }
    return unequal_pos;
}

/*
   PSADBW PSADBW mm, mm/m64
   PSADBW (Packed Sum of Absolute Differences)
   PSADBW instructions when using 64-bit operands:
   TEMP0 ← ABS(DEST[7-0] ? SRC[7-0]);
   * repeat operation for bytes 2 through 6 *;
   TEMP7 ← ABS(DEST[63-56] ? SRC[63-56]);
   DEST[15:0] ← SUM(TEMP0...TEMP7);
*/

#if 1

/*
   PATTERN_EVALUATION
   Calculates the static evaluation of the position using
   the statistically optimized pattern tables.
*/

int
pattern_evaluation2(int color,
    const BitBoard my_bits,
    const BitBoard opp_bits) {
    int eval_phase;
    short score;

#ifndef USE_BYTE_BOARD
#if _DEBUG
    int unequal_pos;
    BitBoard *_my_bits = NULL;
    BitBoard *_opp_bits = NULL;
#endif  /* _DEBUG */
#endif  /* !USE_BYTE_BOARD */

    INCREMENT_COUNTER(leaf_nodes);

#if TIME_EVAL
    //__int64 t0 = rdtsc();
#endif

    /* Any player wiped out? Game over then... */

#ifndef _DEBUG
    if (piece_count[CHESS_BLACK][disks_played] == 0) {
        if (color == CHESS_BLACK)
            return -(MIDGAME_WIN + 64);
        else
            return +(MIDGAME_WIN + 64);
    }
    else if (piece_count[CHESS_WHITE][disks_played] == 0) {
        if (color == CHESS_BLACK)
            return +(MIDGAME_WIN + 64);
        else
            return -(MIDGAME_WIN + 64);
    }
#endif  /* !_DEBUG */

    /* Load and/or initialize the pattern coefficients */

    eval_phase = eval_map[disks_played];
    if (!coeffs[eval_phase].loaded)
        load_coeff(eval_phase);

    /* The constant feature and the parity feature */

    score = coeffs[eval_phase].parity_constant[disks_played & 1];

#ifdef LOG_EVAL
    fprintf(stream, "parity=%d\n", score);
    fprintf(stream, "disks_played=%d (%d)\n", disks_played,
        disks_played & 1 );
#endif

    /* The pattern features. */
#ifndef USE_BYTE_BOARD
#if _DEBUG
#if 0
    _my_bits = (BitBoard *)&my_bits;
    _opp_bits = (BitBoard *)&opp_bits;
    _my_bits->low = 0x43211234UL;
    _my_bits->high = 0x12344321UL;
    _opp_bits->low = 0x24044042UL;
    _opp_bits->high = 0x40422404UL;
#endif
    gen_board_from_bb(my_bits, opp_bits, wBoard, color);
    gen_board_from_bb1(my_bits, opp_bits, wBoard2, color);
    unequal_pos = board_is_equal32(wBoard, wBoard2);
    if (unequal_pos != -1)
        unequal_pos = 0;
#endif  /* _DEBUG */

#if 0
    gen_board_from_bb(my_bits, opp_bits, wBoard, color);
#else
    gen_board_from_bb1(my_bits, opp_bits, wBoard, color);
#endif

#else
    gen_byte_board_from_bb(my_bits, opp_bits, wBoard, color);
#endif

    if (color == CHESS_BLACK) {

        int pattern0;
        pattern0 = wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[A8];
        pattern0 = 3 * pattern0 + wBoard[A7];
        pattern0 = 3 * pattern0 + wBoard[A6];
        pattern0 = 3 * pattern0 + wBoard[A5];
        pattern0 = 3 * pattern0 + wBoard[A4];
        pattern0 = 3 * pattern0 + wBoard[A3];
        pattern0 = 3 * pattern0 + wBoard[A2];
        pattern0 = 3 * pattern0 + wBoard[A1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].afile2x[pattern0]);
#endif
        score += coeffs[eval_phase].afile2x[pattern0];
        pattern0 = wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[H8];
        pattern0 = 3 * pattern0 + wBoard[H7];
        pattern0 = 3 * pattern0 + wBoard[H6];
        pattern0 = 3 * pattern0 + wBoard[H5];
        pattern0 = 3 * pattern0 + wBoard[H4];
        pattern0 = 3 * pattern0 + wBoard[H3];
        pattern0 = 3 * pattern0 + wBoard[H2];
        pattern0 = 3 * pattern0 + wBoard[H1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].afile2x[pattern0]);
#endif
        score += coeffs[eval_phase].afile2x[pattern0];
        pattern0 = wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[H1];
        pattern0 = 3 * pattern0 + wBoard[G1];
        pattern0 = 3 * pattern0 + wBoard[F1];
        pattern0 = 3 * pattern0 + wBoard[E1];
        pattern0 = 3 * pattern0 + wBoard[D1];
        pattern0 = 3 * pattern0 + wBoard[C1];
        pattern0 = 3 * pattern0 + wBoard[B1];
        pattern0 = 3 * pattern0 + wBoard[A1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].afile2x[pattern0]);
#endif
        score += coeffs[eval_phase].afile2x[pattern0];
        pattern0 = wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[H8];
        pattern0 = 3 * pattern0 + wBoard[G8];
        pattern0 = 3 * pattern0 + wBoard[F8];
        pattern0 = 3 * pattern0 + wBoard[E8];
        pattern0 = 3 * pattern0 + wBoard[D8];
        pattern0 = 3 * pattern0 + wBoard[C8];
        pattern0 = 3 * pattern0 + wBoard[B8];
        pattern0 = 3 * pattern0 + wBoard[A8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].afile2x[pattern0]);
#endif
        score += coeffs[eval_phase].afile2x[pattern0];
        pattern0 = wBoard[B8];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[B6];
        pattern0 = 3 * pattern0 + wBoard[B5];
        pattern0 = 3 * pattern0 + wBoard[B4];
        pattern0 = 3 * pattern0 + wBoard[B3];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[B1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].bfile[pattern0]);
#endif
        score += coeffs[eval_phase].bfile[pattern0];
        pattern0 = wBoard[G8];
        pattern0 = 3 * pattern0 + wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[G6];
        pattern0 = 3 * pattern0 + wBoard[G5];
        pattern0 = 3 * pattern0 + wBoard[G4];
        pattern0 = 3 * pattern0 + wBoard[G3];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[G1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].bfile[pattern0]);
#endif
        score += coeffs[eval_phase].bfile[pattern0];
        pattern0 = wBoard[H2];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[F2];
        pattern0 = 3 * pattern0 + wBoard[E2];
        pattern0 = 3 * pattern0 + wBoard[D2];
        pattern0 = 3 * pattern0 + wBoard[C2];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[A2];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].bfile[pattern0]);
#endif
        score += coeffs[eval_phase].bfile[pattern0];
        pattern0 = wBoard[H7];
        pattern0 = 3 * pattern0 + wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[F7];
        pattern0 = 3 * pattern0 + wBoard[E7];
        pattern0 = 3 * pattern0 + wBoard[D7];
        pattern0 = 3 * pattern0 + wBoard[C7];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[A7];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].bfile[pattern0]);
#endif
        score += coeffs[eval_phase].bfile[pattern0];
        pattern0 = wBoard[C8];
        pattern0 = 3 * pattern0 + wBoard[C7];
        pattern0 = 3 * pattern0 + wBoard[C6];
        pattern0 = 3 * pattern0 + wBoard[C5];
        pattern0 = 3 * pattern0 + wBoard[C4];
        pattern0 = 3 * pattern0 + wBoard[C3];
        pattern0 = 3 * pattern0 + wBoard[C2];
        pattern0 = 3 * pattern0 + wBoard[C1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].cfile[pattern0]);
#endif
        score += coeffs[eval_phase].cfile[pattern0];
        pattern0 = wBoard[F8];
        pattern0 = 3 * pattern0 + wBoard[F7];
        pattern0 = 3 * pattern0 + wBoard[F6];
        pattern0 = 3 * pattern0 + wBoard[F5];
        pattern0 = 3 * pattern0 + wBoard[F4];
        pattern0 = 3 * pattern0 + wBoard[F3];
        pattern0 = 3 * pattern0 + wBoard[F2];
        pattern0 = 3 * pattern0 + wBoard[F1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].cfile[pattern0]);
#endif
        score += coeffs[eval_phase].cfile[pattern0];
        pattern0 = wBoard[H3];
        pattern0 = 3 * pattern0 + wBoard[G3];
        pattern0 = 3 * pattern0 + wBoard[F3];
        pattern0 = 3 * pattern0 + wBoard[E3];
        pattern0 = 3 * pattern0 + wBoard[D3];
        pattern0 = 3 * pattern0 + wBoard[C3];
        pattern0 = 3 * pattern0 + wBoard[B3];
        pattern0 = 3 * pattern0 + wBoard[A3];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].cfile[pattern0]);
#endif
        score += coeffs[eval_phase].cfile[pattern0];
        pattern0 = wBoard[H6];
        pattern0 = 3 * pattern0 + wBoard[G6];
        pattern0 = 3 * pattern0 + wBoard[F6];
        pattern0 = 3 * pattern0 + wBoard[E6];
        pattern0 = 3 * pattern0 + wBoard[D6];
        pattern0 = 3 * pattern0 + wBoard[C6];
        pattern0 = 3 * pattern0 + wBoard[B6];
        pattern0 = 3 * pattern0 + wBoard[A6];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].cfile[pattern0]);
#endif
        score += coeffs[eval_phase].cfile[pattern0];
        pattern0 = wBoard[D8];
        pattern0 = 3 * pattern0 + wBoard[D7];
        pattern0 = 3 * pattern0 + wBoard[D6];
        pattern0 = 3 * pattern0 + wBoard[D5];
        pattern0 = 3 * pattern0 + wBoard[D4];
        pattern0 = 3 * pattern0 + wBoard[D3];
        pattern0 = 3 * pattern0 + wBoard[D2];
        pattern0 = 3 * pattern0 + wBoard[D1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].dfile[pattern0]);
#endif
        score += coeffs[eval_phase].dfile[pattern0];
        pattern0 = wBoard[E8];
        pattern0 = 3 * pattern0 + wBoard[E7];
        pattern0 = 3 * pattern0 + wBoard[E6];
        pattern0 = 3 * pattern0 + wBoard[E5];
        pattern0 = 3 * pattern0 + wBoard[E4];
        pattern0 = 3 * pattern0 + wBoard[E3];
        pattern0 = 3 * pattern0 + wBoard[E2];
        pattern0 = 3 * pattern0 + wBoard[E1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].dfile[pattern0]);
#endif
        score += coeffs[eval_phase].dfile[pattern0];
        pattern0 = wBoard[H4];
        pattern0 = 3 * pattern0 + wBoard[G4];
        pattern0 = 3 * pattern0 + wBoard[F4];
        pattern0 = 3 * pattern0 + wBoard[E4];
        pattern0 = 3 * pattern0 + wBoard[D4];
        pattern0 = 3 * pattern0 + wBoard[C4];
        pattern0 = 3 * pattern0 + wBoard[B4];
        pattern0 = 3 * pattern0 + wBoard[A4];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].dfile[pattern0]);
#endif
        score += coeffs[eval_phase].dfile[pattern0];
        pattern0 = wBoard[H5];
        pattern0 = 3 * pattern0 + wBoard[G5];
        pattern0 = 3 * pattern0 + wBoard[F5];
        pattern0 = 3 * pattern0 + wBoard[E5];
        pattern0 = 3 * pattern0 + wBoard[D5];
        pattern0 = 3 * pattern0 + wBoard[C5];
        pattern0 = 3 * pattern0 + wBoard[B5];
        pattern0 = 3 * pattern0 + wBoard[A5];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].dfile[pattern0]);
#endif
        score += coeffs[eval_phase].dfile[pattern0];
        pattern0 = wBoard[H8];
        pattern0 = 3 * pattern0 + wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[F6];
        pattern0 = 3 * pattern0 + wBoard[E5];
        pattern0 = 3 * pattern0 + wBoard[D4];
        pattern0 = 3 * pattern0 + wBoard[C3];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[A1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag8[pattern0]);
#endif
        score += coeffs[eval_phase].diag8[pattern0];
        pattern0 = wBoard[A8];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[C6];
        pattern0 = 3 * pattern0 + wBoard[D5];
        pattern0 = 3 * pattern0 + wBoard[E4];
        pattern0 = 3 * pattern0 + wBoard[F3];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[H1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag8[pattern0]);
#endif
        score += coeffs[eval_phase].diag8[pattern0];
        pattern0 = wBoard[H7];
        pattern0 = 3 * pattern0 + wBoard[G6];
        pattern0 = 3 * pattern0 + wBoard[F5];
        pattern0 = 3 * pattern0 + wBoard[E4];
        pattern0 = 3 * pattern0 + wBoard[D3];
        pattern0 = 3 * pattern0 + wBoard[C2];
        pattern0 = 3 * pattern0 + wBoard[B1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag7[pattern0]);
#endif
        score += coeffs[eval_phase].diag7[pattern0];
        pattern0 = wBoard[G8];
        pattern0 = 3 * pattern0 + wBoard[F7];
        pattern0 = 3 * pattern0 + wBoard[E6];
        pattern0 = 3 * pattern0 + wBoard[D5];
        pattern0 = 3 * pattern0 + wBoard[C4];
        pattern0 = 3 * pattern0 + wBoard[B3];
        pattern0 = 3 * pattern0 + wBoard[A2];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag7[pattern0]);
#endif
        score += coeffs[eval_phase].diag7[pattern0];
        pattern0 = wBoard[A7];
        pattern0 = 3 * pattern0 + wBoard[B6];
        pattern0 = 3 * pattern0 + wBoard[C5];
        pattern0 = 3 * pattern0 + wBoard[D4];
        pattern0 = 3 * pattern0 + wBoard[E3];
        pattern0 = 3 * pattern0 + wBoard[F2];
        pattern0 = 3 * pattern0 + wBoard[G1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag7[pattern0]);
#endif
        score += coeffs[eval_phase].diag7[pattern0];
        pattern0 = wBoard[B8];
        pattern0 = 3 * pattern0 + wBoard[C7];
        pattern0 = 3 * pattern0 + wBoard[D6];
        pattern0 = 3 * pattern0 + wBoard[E5];
        pattern0 = 3 * pattern0 + wBoard[F4];
        pattern0 = 3 * pattern0 + wBoard[G3];
        pattern0 = 3 * pattern0 + wBoard[H2];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag7[pattern0]);
#endif
        score += coeffs[eval_phase].diag7[pattern0];
        pattern0 = wBoard[H6];
        pattern0 = 3 * pattern0 + wBoard[G5];
        pattern0 = 3 * pattern0 + wBoard[F4];
        pattern0 = 3 * pattern0 + wBoard[E3];
        pattern0 = 3 * pattern0 + wBoard[D2];
        pattern0 = 3 * pattern0 + wBoard[C1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag6[pattern0]);
#endif
        score += coeffs[eval_phase].diag6[pattern0];
        pattern0 = wBoard[F8];
        pattern0 = 3 * pattern0 + wBoard[E7];
        pattern0 = 3 * pattern0 + wBoard[D6];
        pattern0 = 3 * pattern0 + wBoard[C5];
        pattern0 = 3 * pattern0 + wBoard[B4];
        pattern0 = 3 * pattern0 + wBoard[A3];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag6[pattern0]);
#endif
        score += coeffs[eval_phase].diag6[pattern0];
        pattern0 = wBoard[A6];
        pattern0 = 3 * pattern0 + wBoard[B5];
        pattern0 = 3 * pattern0 + wBoard[C4];
        pattern0 = 3 * pattern0 + wBoard[D3];
        pattern0 = 3 * pattern0 + wBoard[E2];
        pattern0 = 3 * pattern0 + wBoard[F1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag6[pattern0]);
#endif
        score += coeffs[eval_phase].diag6[pattern0];
        pattern0 = wBoard[C8];
        pattern0 = 3 * pattern0 + wBoard[D7];
        pattern0 = 3 * pattern0 + wBoard[E6];
        pattern0 = 3 * pattern0 + wBoard[F5];
        pattern0 = 3 * pattern0 + wBoard[G4];
        pattern0 = 3 * pattern0 + wBoard[H3];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag6[pattern0]);
#endif
        score += coeffs[eval_phase].diag6[pattern0];
        pattern0 = wBoard[H5];
        pattern0 = 3 * pattern0 + wBoard[G4];
        pattern0 = 3 * pattern0 + wBoard[F3];
        pattern0 = 3 * pattern0 + wBoard[E2];
        pattern0 = 3 * pattern0 + wBoard[D1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag5[pattern0]);
#endif
        score += coeffs[eval_phase].diag5[pattern0];
        pattern0 = wBoard[E8];
        pattern0 = 3 * pattern0 + wBoard[D7];
        pattern0 = 3 * pattern0 + wBoard[C6];
        pattern0 = 3 * pattern0 + wBoard[B5];
        pattern0 = 3 * pattern0 + wBoard[A4];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag5[pattern0]);
#endif
        score += coeffs[eval_phase].diag5[pattern0];
        pattern0 = wBoard[A5];
        pattern0 = 3 * pattern0 + wBoard[B4];
        pattern0 = 3 * pattern0 + wBoard[C3];
        pattern0 = 3 * pattern0 + wBoard[D2];
        pattern0 = 3 * pattern0 + wBoard[E1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag5[pattern0]);
#endif
        score += coeffs[eval_phase].diag5[pattern0];
        pattern0 = wBoard[D8];
        pattern0 = 3 * pattern0 + wBoard[E7];
        pattern0 = 3 * pattern0 + wBoard[F6];
        pattern0 = 3 * pattern0 + wBoard[G5];
        pattern0 = 3 * pattern0 + wBoard[H4];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag5[pattern0]);
#endif
        score += coeffs[eval_phase].diag5[pattern0];
        pattern0 = wBoard[H4];
        pattern0 = 3 * pattern0 + wBoard[G3];
        pattern0 = 3 * pattern0 + wBoard[F2];
        pattern0 = 3 * pattern0 + wBoard[E1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag4[pattern0]);
#endif
        score += coeffs[eval_phase].diag4[pattern0];
        pattern0 = wBoard[D8];
        pattern0 = 3 * pattern0 + wBoard[C7];
        pattern0 = 3 * pattern0 + wBoard[B6];
        pattern0 = 3 * pattern0 + wBoard[A5];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag4[pattern0]);
#endif
        score += coeffs[eval_phase].diag4[pattern0];
        pattern0 = wBoard[A4];
        pattern0 = 3 * pattern0 + wBoard[B3];
        pattern0 = 3 * pattern0 + wBoard[C2];
        pattern0 = 3 * pattern0 + wBoard[D1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag4[pattern0]);
#endif
        score += coeffs[eval_phase].diag4[pattern0];
        pattern0 = wBoard[E8];
        pattern0 = 3 * pattern0 + wBoard[F7];
        pattern0 = 3 * pattern0 + wBoard[G6];
        pattern0 = 3 * pattern0 + wBoard[H5];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag4[pattern0]);
#endif
        score += coeffs[eval_phase].diag4[pattern0];
        pattern0 = wBoard[C3];
        pattern0 = 3 * pattern0 + wBoard[B3];
        pattern0 = 3 * pattern0 + wBoard[A3];
        pattern0 = 3 * pattern0 + wBoard[C2];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[A2];
        pattern0 = 3 * pattern0 + wBoard[C1];
        pattern0 = 3 * pattern0 + wBoard[B1];
        pattern0 = 3 * pattern0 + wBoard[A1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner33[pattern0]);
#endif
        score += coeffs[eval_phase].corner33[pattern0];
        pattern0 = wBoard[C6];
        pattern0 = 3 * pattern0 + wBoard[B6];
        pattern0 = 3 * pattern0 + wBoard[A6];
        pattern0 = 3 * pattern0 + wBoard[C7];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[A7];
        pattern0 = 3 * pattern0 + wBoard[C8];
        pattern0 = 3 * pattern0 + wBoard[B8];
        pattern0 = 3 * pattern0 + wBoard[A8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner33[pattern0]);
#endif
        score += coeffs[eval_phase].corner33[pattern0];
        pattern0 = wBoard[F3];
        pattern0 = 3 * pattern0 + wBoard[G3];
        pattern0 = 3 * pattern0 + wBoard[H3];
        pattern0 = 3 * pattern0 + wBoard[F2];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[H2];
        pattern0 = 3 * pattern0 + wBoard[F1];
        pattern0 = 3 * pattern0 + wBoard[G1];
        pattern0 = 3 * pattern0 + wBoard[H1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner33[pattern0]);
#endif
        score += coeffs[eval_phase].corner33[pattern0];
        pattern0 = wBoard[F6];
        pattern0 = 3 * pattern0 + wBoard[G6];
        pattern0 = 3 * pattern0 + wBoard[H6];
        pattern0 = 3 * pattern0 + wBoard[F7];
        pattern0 = 3 * pattern0 + wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[H7];
        pattern0 = 3 * pattern0 + wBoard[F8];
        pattern0 = 3 * pattern0 + wBoard[G8];
        pattern0 = 3 * pattern0 + wBoard[H8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner33[pattern0]);
#endif
        score += coeffs[eval_phase].corner33[pattern0];
        pattern0 = wBoard[E2];
        pattern0 = 3 * pattern0 + wBoard[D2];
        pattern0 = 3 * pattern0 + wBoard[C2];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[A2];
        pattern0 = 3 * pattern0 + wBoard[E1];
        pattern0 = 3 * pattern0 + wBoard[D1];
        pattern0 = 3 * pattern0 + wBoard[C1];
        pattern0 = 3 * pattern0 + wBoard[B1];
        pattern0 = 3 * pattern0 + wBoard[A1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52[pattern0]);
#endif
        score += coeffs[eval_phase].corner52[pattern0];
        pattern0 = wBoard[E7];
        pattern0 = 3 * pattern0 + wBoard[D7];
        pattern0 = 3 * pattern0 + wBoard[C7];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[A7];
        pattern0 = 3 * pattern0 + wBoard[E8];
        pattern0 = 3 * pattern0 + wBoard[D8];
        pattern0 = 3 * pattern0 + wBoard[C8];
        pattern0 = 3 * pattern0 + wBoard[B8];
        pattern0 = 3 * pattern0 + wBoard[A8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52[pattern0]);
#endif
        score += coeffs[eval_phase].corner52[pattern0];
        pattern0 = wBoard[D2];
        pattern0 = 3 * pattern0 + wBoard[E2];
        pattern0 = 3 * pattern0 + wBoard[F2];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[H2];
        pattern0 = 3 * pattern0 + wBoard[D1];
        pattern0 = 3 * pattern0 + wBoard[E1];
        pattern0 = 3 * pattern0 + wBoard[F1];
        pattern0 = 3 * pattern0 + wBoard[G1];
        pattern0 = 3 * pattern0 + wBoard[H1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52[pattern0]);
#endif
        score += coeffs[eval_phase].corner52[pattern0];
        pattern0 = wBoard[D7];
        pattern0 = 3 * pattern0 + wBoard[E7];
        pattern0 = 3 * pattern0 + wBoard[F7];
        pattern0 = 3 * pattern0 + wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[H7];
        pattern0 = 3 * pattern0 + wBoard[D8];
        pattern0 = 3 * pattern0 + wBoard[E8];
        pattern0 = 3 * pattern0 + wBoard[F8];
        pattern0 = 3 * pattern0 + wBoard[G8];
        pattern0 = 3 * pattern0 + wBoard[H8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52[pattern0]);
#endif
        score += coeffs[eval_phase].corner52[pattern0];
        pattern0 = wBoard[B5];
        pattern0 = 3 * pattern0 + wBoard[B4];
        pattern0 = 3 * pattern0 + wBoard[B3];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[B1];
        pattern0 = 3 * pattern0 + wBoard[A5];
        pattern0 = 3 * pattern0 + wBoard[A4];
        pattern0 = 3 * pattern0 + wBoard[A3];
        pattern0 = 3 * pattern0 + wBoard[A2];
        pattern0 = 3 * pattern0 + wBoard[A1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52[pattern0]);
#endif
        score += coeffs[eval_phase].corner52[pattern0];
        pattern0 = wBoard[G5];
        pattern0 = 3 * pattern0 + wBoard[G4];
        pattern0 = 3 * pattern0 + wBoard[G3];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[G1];
        pattern0 = 3 * pattern0 + wBoard[H5];
        pattern0 = 3 * pattern0 + wBoard[H4];
        pattern0 = 3 * pattern0 + wBoard[H3];
        pattern0 = 3 * pattern0 + wBoard[H2];
        pattern0 = 3 * pattern0 + wBoard[H1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52[pattern0]);
#endif
        score += coeffs[eval_phase].corner52[pattern0];
        pattern0 = wBoard[B4];
        pattern0 = 3 * pattern0 + wBoard[B5];
        pattern0 = 3 * pattern0 + wBoard[B6];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[B8];
        pattern0 = 3 * pattern0 + wBoard[A4];
        pattern0 = 3 * pattern0 + wBoard[A5];
        pattern0 = 3 * pattern0 + wBoard[A6];
        pattern0 = 3 * pattern0 + wBoard[A7];
        pattern0 = 3 * pattern0 + wBoard[A8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52[pattern0]);
#endif
        score += coeffs[eval_phase].corner52[pattern0];
        pattern0 = wBoard[G4];
        pattern0 = 3 * pattern0 + wBoard[G5];
        pattern0 = 3 * pattern0 + wBoard[G6];
        pattern0 = 3 * pattern0 + wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[G8];
        pattern0 = 3 * pattern0 + wBoard[H4];
        pattern0 = 3 * pattern0 + wBoard[H5];
        pattern0 = 3 * pattern0 + wBoard[H6];
        pattern0 = 3 * pattern0 + wBoard[H7];
        pattern0 = 3 * pattern0 + wBoard[H8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52[pattern0]);
#endif
        score += coeffs[eval_phase].corner52[pattern0];

    }
    else { // color == CHESS_WHITE

        int pattern0;
        pattern0 = wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[A8];
        pattern0 = 3 * pattern0 + wBoard[A7];
        pattern0 = 3 * pattern0 + wBoard[A6];
        pattern0 = 3 * pattern0 + wBoard[A5];
        pattern0 = 3 * pattern0 + wBoard[A4];
        pattern0 = 3 * pattern0 + wBoard[A3];
        pattern0 = 3 * pattern0 + wBoard[A2];
        pattern0 = 3 * pattern0 + wBoard[A1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].afile2x_last[-pattern0]);
#endif
        score += coeffs[eval_phase].afile2x_last[-pattern0];
        pattern0 = wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[H8];
        pattern0 = 3 * pattern0 + wBoard[H7];
        pattern0 = 3 * pattern0 + wBoard[H6];
        pattern0 = 3 * pattern0 + wBoard[H5];
        pattern0 = 3 * pattern0 + wBoard[H4];
        pattern0 = 3 * pattern0 + wBoard[H3];
        pattern0 = 3 * pattern0 + wBoard[H2];
        pattern0 = 3 * pattern0 + wBoard[H1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].afile2x_last[-pattern0]);
#endif
        score += coeffs[eval_phase].afile2x_last[-pattern0];
        pattern0 = wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[H1];
        pattern0 = 3 * pattern0 + wBoard[G1];
        pattern0 = 3 * pattern0 + wBoard[F1];
        pattern0 = 3 * pattern0 + wBoard[E1];
        pattern0 = 3 * pattern0 + wBoard[D1];
        pattern0 = 3 * pattern0 + wBoard[C1];
        pattern0 = 3 * pattern0 + wBoard[B1];
        pattern0 = 3 * pattern0 + wBoard[A1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].afile2x_last[-pattern0]);
#endif
        score += coeffs[eval_phase].afile2x_last[-pattern0];
        pattern0 = wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[H8];
        pattern0 = 3 * pattern0 + wBoard[G8];
        pattern0 = 3 * pattern0 + wBoard[F8];
        pattern0 = 3 * pattern0 + wBoard[E8];
        pattern0 = 3 * pattern0 + wBoard[D8];
        pattern0 = 3 * pattern0 + wBoard[C8];
        pattern0 = 3 * pattern0 + wBoard[B8];
        pattern0 = 3 * pattern0 + wBoard[A8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].afile2x_last[-pattern0]);
#endif
        score += coeffs[eval_phase].afile2x_last[-pattern0];
        pattern0 = wBoard[B8];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[B6];
        pattern0 = 3 * pattern0 + wBoard[B5];
        pattern0 = 3 * pattern0 + wBoard[B4];
        pattern0 = 3 * pattern0 + wBoard[B3];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[B1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].bfile_last[-pattern0]);
#endif
        score += coeffs[eval_phase].bfile_last[-pattern0];
        pattern0 = wBoard[G8];
        pattern0 = 3 * pattern0 + wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[G6];
        pattern0 = 3 * pattern0 + wBoard[G5];
        pattern0 = 3 * pattern0 + wBoard[G4];
        pattern0 = 3 * pattern0 + wBoard[G3];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[G1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].bfile_last[-pattern0]);
#endif
        score += coeffs[eval_phase].bfile_last[-pattern0];
        pattern0 = wBoard[H2];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[F2];
        pattern0 = 3 * pattern0 + wBoard[E2];
        pattern0 = 3 * pattern0 + wBoard[D2];
        pattern0 = 3 * pattern0 + wBoard[C2];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[A2];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].bfile_last[-pattern0]);
#endif
        score += coeffs[eval_phase].bfile_last[-pattern0];
        pattern0 = wBoard[H7];
        pattern0 = 3 * pattern0 + wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[F7];
        pattern0 = 3 * pattern0 + wBoard[E7];
        pattern0 = 3 * pattern0 + wBoard[D7];
        pattern0 = 3 * pattern0 + wBoard[C7];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[A7];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].bfile_last[-pattern0]);
#endif
        score += coeffs[eval_phase].bfile_last[-pattern0];
        pattern0 = wBoard[C8];
        pattern0 = 3 * pattern0 + wBoard[C7];
        pattern0 = 3 * pattern0 + wBoard[C6];
        pattern0 = 3 * pattern0 + wBoard[C5];
        pattern0 = 3 * pattern0 + wBoard[C4];
        pattern0 = 3 * pattern0 + wBoard[C3];
        pattern0 = 3 * pattern0 + wBoard[C2];
        pattern0 = 3 * pattern0 + wBoard[C1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].cfile_last[-pattern0]);
#endif
        score += coeffs[eval_phase].cfile_last[-pattern0];
        pattern0 = wBoard[F8];
        pattern0 = 3 * pattern0 + wBoard[F7];
        pattern0 = 3 * pattern0 + wBoard[F6];
        pattern0 = 3 * pattern0 + wBoard[F5];
        pattern0 = 3 * pattern0 + wBoard[F4];
        pattern0 = 3 * pattern0 + wBoard[F3];
        pattern0 = 3 * pattern0 + wBoard[F2];
        pattern0 = 3 * pattern0 + wBoard[F1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].cfile_last[-pattern0]);
#endif
        score += coeffs[eval_phase].cfile_last[-pattern0];
        pattern0 = wBoard[H3];
        pattern0 = 3 * pattern0 + wBoard[G3];
        pattern0 = 3 * pattern0 + wBoard[F3];
        pattern0 = 3 * pattern0 + wBoard[E3];
        pattern0 = 3 * pattern0 + wBoard[D3];
        pattern0 = 3 * pattern0 + wBoard[C3];
        pattern0 = 3 * pattern0 + wBoard[B3];
        pattern0 = 3 * pattern0 + wBoard[A3];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].cfile_last[-pattern0]);
#endif
        score += coeffs[eval_phase].cfile_last[-pattern0];
        pattern0 = wBoard[H6];
        pattern0 = 3 * pattern0 + wBoard[G6];
        pattern0 = 3 * pattern0 + wBoard[F6];
        pattern0 = 3 * pattern0 + wBoard[E6];
        pattern0 = 3 * pattern0 + wBoard[D6];
        pattern0 = 3 * pattern0 + wBoard[C6];
        pattern0 = 3 * pattern0 + wBoard[B6];
        pattern0 = 3 * pattern0 + wBoard[A6];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].cfile_last[-pattern0]);
#endif
        score += coeffs[eval_phase].cfile_last[-pattern0];
        pattern0 = wBoard[D8];
        pattern0 = 3 * pattern0 + wBoard[D7];
        pattern0 = 3 * pattern0 + wBoard[D6];
        pattern0 = 3 * pattern0 + wBoard[D5];
        pattern0 = 3 * pattern0 + wBoard[D4];
        pattern0 = 3 * pattern0 + wBoard[D3];
        pattern0 = 3 * pattern0 + wBoard[D2];
        pattern0 = 3 * pattern0 + wBoard[D1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].dfile_last[-pattern0]);
#endif
        score += coeffs[eval_phase].dfile_last[-pattern0];
        pattern0 = wBoard[E8];
        pattern0 = 3 * pattern0 + wBoard[E7];
        pattern0 = 3 * pattern0 + wBoard[E6];
        pattern0 = 3 * pattern0 + wBoard[E5];
        pattern0 = 3 * pattern0 + wBoard[E4];
        pattern0 = 3 * pattern0 + wBoard[E3];
        pattern0 = 3 * pattern0 + wBoard[E2];
        pattern0 = 3 * pattern0 + wBoard[E1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].dfile_last[-pattern0]);
#endif
        score += coeffs[eval_phase].dfile_last[-pattern0];
        pattern0 = wBoard[H4];
        pattern0 = 3 * pattern0 + wBoard[G4];
        pattern0 = 3 * pattern0 + wBoard[F4];
        pattern0 = 3 * pattern0 + wBoard[E4];
        pattern0 = 3 * pattern0 + wBoard[D4];
        pattern0 = 3 * pattern0 + wBoard[C4];
        pattern0 = 3 * pattern0 + wBoard[B4];
        pattern0 = 3 * pattern0 + wBoard[A4];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].dfile_last[-pattern0]);
#endif
        score += coeffs[eval_phase].dfile_last[-pattern0];
        pattern0 = wBoard[H5];
        pattern0 = 3 * pattern0 + wBoard[G5];
        pattern0 = 3 * pattern0 + wBoard[F5];
        pattern0 = 3 * pattern0 + wBoard[E5];
        pattern0 = 3 * pattern0 + wBoard[D5];
        pattern0 = 3 * pattern0 + wBoard[C5];
        pattern0 = 3 * pattern0 + wBoard[B5];
        pattern0 = 3 * pattern0 + wBoard[A5];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].dfile_last[-pattern0]);
#endif
        score += coeffs[eval_phase].dfile_last[-pattern0];
        pattern0 = wBoard[H8];
        pattern0 = 3 * pattern0 + wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[F6];
        pattern0 = 3 * pattern0 + wBoard[E5];
        pattern0 = 3 * pattern0 + wBoard[D4];
        pattern0 = 3 * pattern0 + wBoard[C3];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[A1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag8_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag8_last[-pattern0];
        pattern0 = wBoard[A8];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[C6];
        pattern0 = 3 * pattern0 + wBoard[D5];
        pattern0 = 3 * pattern0 + wBoard[E4];
        pattern0 = 3 * pattern0 + wBoard[F3];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[H1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag8_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag8_last[-pattern0];
        pattern0 = wBoard[H7];
        pattern0 = 3 * pattern0 + wBoard[G6];
        pattern0 = 3 * pattern0 + wBoard[F5];
        pattern0 = 3 * pattern0 + wBoard[E4];
        pattern0 = 3 * pattern0 + wBoard[D3];
        pattern0 = 3 * pattern0 + wBoard[C2];
        pattern0 = 3 * pattern0 + wBoard[B1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag7_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag7_last[-pattern0];
        pattern0 = wBoard[G8];
        pattern0 = 3 * pattern0 + wBoard[F7];
        pattern0 = 3 * pattern0 + wBoard[E6];
        pattern0 = 3 * pattern0 + wBoard[D5];
        pattern0 = 3 * pattern0 + wBoard[C4];
        pattern0 = 3 * pattern0 + wBoard[B3];
        pattern0 = 3 * pattern0 + wBoard[A2];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag7_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag7_last[-pattern0];
        pattern0 = wBoard[A7];
        pattern0 = 3 * pattern0 + wBoard[B6];
        pattern0 = 3 * pattern0 + wBoard[C5];
        pattern0 = 3 * pattern0 + wBoard[D4];
        pattern0 = 3 * pattern0 + wBoard[E3];
        pattern0 = 3 * pattern0 + wBoard[F2];
        pattern0 = 3 * pattern0 + wBoard[G1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag7_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag7_last[-pattern0];
        pattern0 = wBoard[B8];
        pattern0 = 3 * pattern0 + wBoard[C7];
        pattern0 = 3 * pattern0 + wBoard[D6];
        pattern0 = 3 * pattern0 + wBoard[E5];
        pattern0 = 3 * pattern0 + wBoard[F4];
        pattern0 = 3 * pattern0 + wBoard[G3];
        pattern0 = 3 * pattern0 + wBoard[H2];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag7_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag7_last[-pattern0];
        pattern0 = wBoard[H6];
        pattern0 = 3 * pattern0 + wBoard[G5];
        pattern0 = 3 * pattern0 + wBoard[F4];
        pattern0 = 3 * pattern0 + wBoard[E3];
        pattern0 = 3 * pattern0 + wBoard[D2];
        pattern0 = 3 * pattern0 + wBoard[C1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag6_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag6_last[-pattern0];
        pattern0 = wBoard[F8];
        pattern0 = 3 * pattern0 + wBoard[E7];
        pattern0 = 3 * pattern0 + wBoard[D6];
        pattern0 = 3 * pattern0 + wBoard[C5];
        pattern0 = 3 * pattern0 + wBoard[B4];
        pattern0 = 3 * pattern0 + wBoard[A3];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag6_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag6_last[-pattern0];
        pattern0 = wBoard[A6];
        pattern0 = 3 * pattern0 + wBoard[B5];
        pattern0 = 3 * pattern0 + wBoard[C4];
        pattern0 = 3 * pattern0 + wBoard[D3];
        pattern0 = 3 * pattern0 + wBoard[E2];
        pattern0 = 3 * pattern0 + wBoard[F1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag6_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag6_last[-pattern0];
        pattern0 = wBoard[C8];
        pattern0 = 3 * pattern0 + wBoard[D7];
        pattern0 = 3 * pattern0 + wBoard[E6];
        pattern0 = 3 * pattern0 + wBoard[F5];
        pattern0 = 3 * pattern0 + wBoard[G4];
        pattern0 = 3 * pattern0 + wBoard[H3];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag6_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag6_last[-pattern0];
        pattern0 = wBoard[H5];
        pattern0 = 3 * pattern0 + wBoard[G4];
        pattern0 = 3 * pattern0 + wBoard[F3];
        pattern0 = 3 * pattern0 + wBoard[E2];
        pattern0 = 3 * pattern0 + wBoard[D1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag5_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag5_last[-pattern0];
        pattern0 = wBoard[E8];
        pattern0 = 3 * pattern0 + wBoard[D7];
        pattern0 = 3 * pattern0 + wBoard[C6];
        pattern0 = 3 * pattern0 + wBoard[B5];
        pattern0 = 3 * pattern0 + wBoard[A4];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag5_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag5_last[-pattern0];
        pattern0 = wBoard[A5];
        pattern0 = 3 * pattern0 + wBoard[B4];
        pattern0 = 3 * pattern0 + wBoard[C3];
        pattern0 = 3 * pattern0 + wBoard[D2];
        pattern0 = 3 * pattern0 + wBoard[E1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag5_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag5_last[-pattern0];
        pattern0 = wBoard[D8];
        pattern0 = 3 * pattern0 + wBoard[E7];
        pattern0 = 3 * pattern0 + wBoard[F6];
        pattern0 = 3 * pattern0 + wBoard[G5];
        pattern0 = 3 * pattern0 + wBoard[H4];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag5_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag5_last[-pattern0];
        pattern0 = wBoard[H4];
        pattern0 = 3 * pattern0 + wBoard[G3];
        pattern0 = 3 * pattern0 + wBoard[F2];
        pattern0 = 3 * pattern0 + wBoard[E1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag4_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag4_last[-pattern0];
        pattern0 = wBoard[D8];
        pattern0 = 3 * pattern0 + wBoard[C7];
        pattern0 = 3 * pattern0 + wBoard[B6];
        pattern0 = 3 * pattern0 + wBoard[A5];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag4_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag4_last[-pattern0];
        pattern0 = wBoard[A4];
        pattern0 = 3 * pattern0 + wBoard[B3];
        pattern0 = 3 * pattern0 + wBoard[C2];
        pattern0 = 3 * pattern0 + wBoard[D1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag4_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag4_last[-pattern0];
        pattern0 = wBoard[E8];
        pattern0 = 3 * pattern0 + wBoard[F7];
        pattern0 = 3 * pattern0 + wBoard[G6];
        pattern0 = 3 * pattern0 + wBoard[H5];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].diag4_last[-pattern0]);
#endif
        score += coeffs[eval_phase].diag4_last[-pattern0];
        pattern0 = wBoard[C3];
        pattern0 = 3 * pattern0 + wBoard[B3];
        pattern0 = 3 * pattern0 + wBoard[A3];
        pattern0 = 3 * pattern0 + wBoard[C2];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[A2];
        pattern0 = 3 * pattern0 + wBoard[C1];
        pattern0 = 3 * pattern0 + wBoard[B1];
        pattern0 = 3 * pattern0 + wBoard[A1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner33_last[-pattern0]);
#endif
        score += coeffs[eval_phase].corner33_last[-pattern0];
        pattern0 = wBoard[C6];
        pattern0 = 3 * pattern0 + wBoard[B6];
        pattern0 = 3 * pattern0 + wBoard[A6];
        pattern0 = 3 * pattern0 + wBoard[C7];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[A7];
        pattern0 = 3 * pattern0 + wBoard[C8];
        pattern0 = 3 * pattern0 + wBoard[B8];
        pattern0 = 3 * pattern0 + wBoard[A8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner33_last[-pattern0]);
#endif
        score += coeffs[eval_phase].corner33_last[-pattern0];
        pattern0 = wBoard[F3];
        pattern0 = 3 * pattern0 + wBoard[G3];
        pattern0 = 3 * pattern0 + wBoard[H3];
        pattern0 = 3 * pattern0 + wBoard[F2];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[H2];
        pattern0 = 3 * pattern0 + wBoard[F1];
        pattern0 = 3 * pattern0 + wBoard[G1];
        pattern0 = 3 * pattern0 + wBoard[H1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner33_last[-pattern0]);
#endif
        score += coeffs[eval_phase].corner33_last[-pattern0];
        pattern0 = wBoard[F6];
        pattern0 = 3 * pattern0 + wBoard[G6];
        pattern0 = 3 * pattern0 + wBoard[H6];
        pattern0 = 3 * pattern0 + wBoard[F7];
        pattern0 = 3 * pattern0 + wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[H7];
        pattern0 = 3 * pattern0 + wBoard[F8];
        pattern0 = 3 * pattern0 + wBoard[G8];
        pattern0 = 3 * pattern0 + wBoard[H8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner33_last[-pattern0]);
#endif
        score += coeffs[eval_phase].corner33_last[-pattern0];
        pattern0 = wBoard[E2];
        pattern0 = 3 * pattern0 + wBoard[D2];
        pattern0 = 3 * pattern0 + wBoard[C2];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[A2];
        pattern0 = 3 * pattern0 + wBoard[E1];
        pattern0 = 3 * pattern0 + wBoard[D1];
        pattern0 = 3 * pattern0 + wBoard[C1];
        pattern0 = 3 * pattern0 + wBoard[B1];
        pattern0 = 3 * pattern0 + wBoard[A1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52_last[-pattern0]);
#endif
        score += coeffs[eval_phase].corner52_last[-pattern0];
        pattern0 = wBoard[E7];
        pattern0 = 3 * pattern0 + wBoard[D7];
        pattern0 = 3 * pattern0 + wBoard[C7];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[A7];
        pattern0 = 3 * pattern0 + wBoard[E8];
        pattern0 = 3 * pattern0 + wBoard[D8];
        pattern0 = 3 * pattern0 + wBoard[C8];
        pattern0 = 3 * pattern0 + wBoard[B8];
        pattern0 = 3 * pattern0 + wBoard[A8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52_last[-pattern0]);
#endif
        score += coeffs[eval_phase].corner52_last[-pattern0];
        pattern0 = wBoard[D2];
        pattern0 = 3 * pattern0 + wBoard[E2];
        pattern0 = 3 * pattern0 + wBoard[F2];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[H2];
        pattern0 = 3 * pattern0 + wBoard[D1];
        pattern0 = 3 * pattern0 + wBoard[E1];
        pattern0 = 3 * pattern0 + wBoard[F1];
        pattern0 = 3 * pattern0 + wBoard[G1];
        pattern0 = 3 * pattern0 + wBoard[H1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52_last[-pattern0]);
#endif
        score += coeffs[eval_phase].corner52_last[-pattern0];
        pattern0 = wBoard[D7];
        pattern0 = 3 * pattern0 + wBoard[E7];
        pattern0 = 3 * pattern0 + wBoard[F7];
        pattern0 = 3 * pattern0 + wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[H7];
        pattern0 = 3 * pattern0 + wBoard[D8];
        pattern0 = 3 * pattern0 + wBoard[E8];
        pattern0 = 3 * pattern0 + wBoard[F8];
        pattern0 = 3 * pattern0 + wBoard[G8];
        pattern0 = 3 * pattern0 + wBoard[H8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52_last[-pattern0]);
#endif
        score += coeffs[eval_phase].corner52_last[-pattern0];
        pattern0 = wBoard[B5];
        pattern0 = 3 * pattern0 + wBoard[B4];
        pattern0 = 3 * pattern0 + wBoard[B3];
        pattern0 = 3 * pattern0 + wBoard[B2];
        pattern0 = 3 * pattern0 + wBoard[B1];
        pattern0 = 3 * pattern0 + wBoard[A5];
        pattern0 = 3 * pattern0 + wBoard[A4];
        pattern0 = 3 * pattern0 + wBoard[A3];
        pattern0 = 3 * pattern0 + wBoard[A2];
        pattern0 = 3 * pattern0 + wBoard[A1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52_last[-pattern0]);
#endif
        score += coeffs[eval_phase].corner52_last[-pattern0];
        pattern0 = wBoard[G5];
        pattern0 = 3 * pattern0 + wBoard[G4];
        pattern0 = 3 * pattern0 + wBoard[G3];
        pattern0 = 3 * pattern0 + wBoard[G2];
        pattern0 = 3 * pattern0 + wBoard[G1];
        pattern0 = 3 * pattern0 + wBoard[H5];
        pattern0 = 3 * pattern0 + wBoard[H4];
        pattern0 = 3 * pattern0 + wBoard[H3];
        pattern0 = 3 * pattern0 + wBoard[H2];
        pattern0 = 3 * pattern0 + wBoard[H1];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52_last[-pattern0]);
#endif
        score += coeffs[eval_phase].corner52_last[-pattern0];
        pattern0 = wBoard[B4];
        pattern0 = 3 * pattern0 + wBoard[B5];
        pattern0 = 3 * pattern0 + wBoard[B6];
        pattern0 = 3 * pattern0 + wBoard[B7];
        pattern0 = 3 * pattern0 + wBoard[B8];
        pattern0 = 3 * pattern0 + wBoard[A4];
        pattern0 = 3 * pattern0 + wBoard[A5];
        pattern0 = 3 * pattern0 + wBoard[A6];
        pattern0 = 3 * pattern0 + wBoard[A7];
        pattern0 = 3 * pattern0 + wBoard[A8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52_last[-pattern0]);
#endif
        score += coeffs[eval_phase].corner52_last[-pattern0];
        pattern0 = wBoard[G4];
        pattern0 = 3 * pattern0 + wBoard[G5];
        pattern0 = 3 * pattern0 + wBoard[G6];
        pattern0 = 3 * pattern0 + wBoard[G7];
        pattern0 = 3 * pattern0 + wBoard[G8];
        pattern0 = 3 * pattern0 + wBoard[H4];
        pattern0 = 3 * pattern0 + wBoard[H5];
        pattern0 = 3 * pattern0 + wBoard[H6];
        pattern0 = 3 * pattern0 + wBoard[H7];
        pattern0 = 3 * pattern0 + wBoard[H8];
#ifdef LOG_EVAL
        fprintf(stream, "pattern=%d\n", pattern0);
        fprintf(stream, "score=%d\n", coeffs[eval_phase].corner52_last[-pattern0]);
#endif
        score += coeffs[eval_phase].corner52_last[-pattern0];
    }

#if TIME_EVAL
    {
        static __int64 sum = 0;
        static int calls = 0;

        calls++;
        sum += (rdtsc() - t0);
        if ((calls & ((1 << 22) - 1)) == 0) {
            printf("%lld cycles / eval\n", sum / calls);
        }
}
#endif

    return score;
}

#endif

#if 0
#else

/*
   PATTERN_EVALUATION2
   Calculates the static evaluation of the position using
   the statistically optimized pattern tables.
*/

int
pattern_evaluation(int color,
    const BitBoard my_bits,
    const BitBoard opp_bits) {
    int eval_phase;
    short score;

    int pattern0;
    unsigned long my_discs, opp_discs;
    unsigned long my_discs2, opp_discs2;
    unsigned int tmp;

    INCREMENT_COUNTER(leaf_nodes);

#if TIME_EVAL
    //__int64 t0 = rdtsc();
#endif

    /* Any player wiped out? Game over then... */

#ifndef _DEBUG
#if 0
    if (piece_count[CHESS_BLACK][disks_played] == 0) {
        if (color == CHESS_BLACK)
            return -(MIDGAME_WIN + 64);
        else
            return +(MIDGAME_WIN + 64);
    }
    else if (piece_count[CHESS_WHITE][disks_played] == 0) {
        if (color == CHESS_BLACK)
            return +(MIDGAME_WIN + 64);
        else
            return -(MIDGAME_WIN + 64);
}
#else

    if ((my_bits.low | my_bits.high) == 0)
        return +(MIDGAME_WIN + 64);
    else if ((opp_bits.low | opp_bits.high) == 0)
        return -(MIDGAME_WIN + 64);

#endif
#endif  // _DEBUG

    /* Load and/or initialize the pattern coefficients */

    eval_phase = eval_map[disks_played];
    if (!coeffs[eval_phase].loaded)
        load_coeff(eval_phase);

    /* The constant feature and the parity feature */

    score = coeffs[eval_phase].parity_constant[disks_played & 1];

    // afile2x: A1-A8, B2, B7
    opp_discs = (((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x01020408ul) >> 24;
    my_discs = (((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x01020408ul) >> 24;

    pattern0 = 29524 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    opp_discs2 = ((opp_bits.low & 0x00000200ul) >> 9) | ((opp_bits.high & 0x00020000ul) >> 16);
    my_discs2 = ((my_bits.low & 0x00000200ul) >> 9) | ((my_bits.high & 0x00020000ul) >> 16);
    pattern0 -= pattern_mask2[my_discs2];
    pattern0 += pattern_mask2[opp_discs2];

    score += coeffs[eval_phase].afile2x[pattern0];

    // afile2x: H1-H8, G2, G7
    opp_discs = ((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) >> 24;
    my_discs = ((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) >> 24;

    pattern0 = 29524 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    opp_discs2 = ((opp_bits.low & 0x00004000ul) >> 14) | ((opp_bits.high & 0x00400000ul) >> 21);
    my_discs2 = ((my_bits.low & 0x00004000ul) >> 14) | ((my_bits.high & 0x00400000ul) >> 21);
    pattern0 -= pattern_mask2[my_discs2];
    pattern0 += pattern_mask2[opp_discs2];

    score += coeffs[eval_phase].afile2x[pattern0];

    // afile2x: A1-H1, B2, G2
    opp_discs = opp_bits.low & 0x000000fful;
    my_discs = my_bits.low & 0x000000fful;

    pattern0 = 29524 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    opp_discs2 = ((opp_bits.low & 0x00000200ul) >> 9) | ((opp_bits.low & 0x00004000ul) >> 13);
    my_discs2 = ((my_bits.low & 0x00000200ul) >> 9) | ((my_bits.low & 0x00004000ul) >> 13);
    pattern0 -= pattern_mask2[my_discs2];
    pattern0 += pattern_mask2[opp_discs2];

    score += coeffs[eval_phase].afile2x[pattern0];

    // afile2x: A8-H8, B7, G7
    opp_discs = opp_bits.high >> 24;
    my_discs = my_bits.high >> 24;

    pattern0 = 29524 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    opp_discs2 = ((opp_bits.high & 0x00020000ul) >> 17) | ((opp_bits.high & 0x00400000ul) >> 21);
    my_discs2 = ((my_bits.high & 0x00020000ul) >> 17) | ((my_bits.high & 0x00400000ul) >> 21);
    pattern0 -= pattern_mask2[my_discs2];
    pattern0 += pattern_mask2[opp_discs2];

    score += coeffs[eval_phase].afile2x[pattern0];

    // bfile: B1-B8
    opp_discs = (((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00810204ul) >> 24;
    my_discs = (((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00810204ul) >> 24;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].bfile[pattern0];

    // bfile: G1-G8
    opp_discs = ((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00408102ul) >> 24;
    my_discs = ((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00408102ul) >> 24;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].bfile[pattern0];

    // bfile: A2-H2
    opp_discs = (opp_bits.low & 0x0000ff00ul) >> 8;
    my_discs = (my_bits.low & 0x0000ff00ul) >> 8;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].bfile[pattern0];

    // bfile: A7-H7
    opp_discs = (opp_bits.high & 0x00ff0000ul) >> 16;
    my_discs = (my_bits.high & 0x00ff0000ul) >> 16;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].bfile[pattern0];

    // cfile: C1-C8
    opp_discs = (((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00408102ul) >> 24;
    my_discs = (((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00408102ul) >> 24;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].cfile[pattern0];

    // cfile: F1-F8
    opp_discs = ((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00810204ul) >> 24;
    my_discs = ((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00810204ul) >> 24;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].cfile[pattern0];

    // cfile: A3-H3
    opp_discs = (opp_bits.low & 0x00ff0000ul) >> 16;
    my_discs = (my_bits.low & 0x00ff0000ul) >> 16;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].cfile[pattern0];

    // cfile: A6-H6
    opp_discs = (opp_bits.high & 0x0000ff00ul) >> 8;
    my_discs = (my_bits.high & 0x0000ff00ul) >> 8;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].cfile[pattern0];

    // dfile: D1-D8
    opp_discs = (((opp_bits.low & 0x08080808ul) | ((opp_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) >> 24;
    my_discs = (((my_bits.low & 0x08080808ul) | ((my_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) >> 24;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].dfile[pattern0];

    // dfile: E1-E8
    opp_discs = ((((opp_bits.low & 0x10101010ul) >> 4) | (opp_bits.high & 0x10101010ul)) * 0x01020408ul) >> 24;
    my_discs = ((((my_bits.low & 0x10101010ul) >> 4) | (my_bits.high & 0x10101010ul)) * 0x01020408ul) >> 24;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].dfile[pattern0];

    // dfile: A4-H4
    opp_discs = opp_bits.low >> 24;
    my_discs = my_bits.low >> 24;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].dfile[pattern0];

    // dfile: A5-H5
    opp_discs = opp_bits.high & 0x000000fful;
    my_discs = my_bits.high & 0x000000fful;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].dfile[pattern0];

    // diag8: A1-H8
    opp_discs = (((opp_bits.low & 0x08040201ul) | (opp_bits.high & 0x80402010ul)) * 0x01010101ul) >> 24;
    my_discs = (((my_bits.low & 0x08040201ul) | (my_bits.high & 0x80402010ul)) * 0x01010101ul) >> 24;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag8[pattern0];

    // diag8: H1-A8
    opp_discs = (((opp_bits.low & 0x10204080ul) | (opp_bits.high & 0x01020408ul)) * 0x01010101ul) >> 24;
    my_discs = (((my_bits.low & 0x10204080ul) | (my_bits.high & 0x01020408ul)) * 0x01010101ul) >> 24;

    pattern0 = 3280 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag8[pattern0];

    // diag7: B1-H7
    opp_discs = (((opp_bits.low & 0x10080402ul) | (opp_bits.high & 0x00804020ul)) * 0x01010101ul) >> 25;
    my_discs = (((my_bits.low & 0x10080402ul) | (my_bits.high & 0x00804020ul)) * 0x01010101ul) >> 25;

    pattern0 = 1093 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag7[pattern0];

    // diag7: A2-G8
    opp_discs = (((opp_bits.low & 0x04020100ul) | (opp_bits.high & 0x40201008ul)) * 0x02020202ul) >> 25;
    my_discs = (((my_bits.low & 0x04020100ul) | (my_bits.high & 0x40201008ul)) * 0x02020202ul) >> 25;

    pattern0 = 1093 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag7[pattern0];

    // diag7: G1-A7
    opp_discs = (((opp_bits.low & 0x08102040ul) | (opp_bits.high & 0x00010204ul)) * 0x02020202ul) >> 25;
    my_discs = (((my_bits.low & 0x08102040ul) | (my_bits.high & 0x00010204ul)) * 0x02020202ul) >> 25;

    pattern0 = 1093 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag7[pattern0];

    // diag7: H2-B8
    opp_discs = (((opp_bits.low & 0x20408000ul) | (opp_bits.high & 0x02040810ul)) * 0x01010101ul) >> 25;
    my_discs = (((my_bits.low & 0x20408000ul) | (my_bits.high & 0x02040810ul)) * 0x01010101ul) >> 25;

    pattern0 = 1093 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag7[pattern0];

    // diag6: C1-H6
    opp_discs = (((opp_bits.low & 0x20100804ul) | (opp_bits.high & 0x00008040ul)) * 0x01010101ul) >> 26;
    my_discs = (((my_bits.low & 0x20100804ul) | (my_bits.high & 0x00008040ul)) * 0x01010101ul) >> 26;

    pattern0 = 364 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag6[pattern0];

    // diag6: A3-F8
    opp_discs = (((opp_bits.low & 0x02010000ul) | (opp_bits.high & 0x20100804ul)) * 0x04040404ul) >> 26;
    my_discs = (((my_bits.low & 0x02010000ul) | (my_bits.high & 0x20100804ul)) * 0x04040404ul) >> 26;

    pattern0 = 364 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag6[pattern0];

    // diag6: F1-A6
    opp_discs = (((opp_bits.low & 0x04081020ul) | (opp_bits.high & 0x00000102ul)) * 0x04040404ul) >> 26;
    my_discs = (((my_bits.low & 0x04081020ul) | (my_bits.high & 0x00000102ul)) * 0x04040404ul) >> 26;

    pattern0 = 364 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag6[pattern0];

    // diag6: H3-C8
    opp_discs = (((opp_bits.low & 0x40800000ul) | (opp_bits.high & 0x04081020ul)) * 0x01010101ul) >> 26;
    my_discs = (((my_bits.low & 0x40800000ul) | (my_bits.high & 0x04081020ul)) * 0x01010101ul) >> 26;

    pattern0 = 364 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag6[pattern0];

    // diag5: D1-H5
    opp_discs = (((opp_bits.low & 0x40201008ul) | (opp_bits.high & 0x00000080ul)) * 0x01010101ul) >> 27;
    my_discs = (((my_bits.low & 0x40201008ul) | (my_bits.high & 0x00000080ul)) * 0x01010101ul) >> 27;

    pattern0 = 121 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag5[pattern0];

    // diag5: A4-E8
    opp_discs = (((opp_bits.low & 0x01000000ul) | (opp_bits.high & 0x10080402ul)) * 0x08080808ul) >> 27;
    my_discs = (((my_bits.low & 0x01000000ul) | (my_bits.high & 0x10080402ul)) * 0x08080808ul) >> 27;

    pattern0 = 121 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag5[pattern0];

    // diag5: E1-A5
    opp_discs = (((opp_bits.low & 0x02040810ul) | (opp_bits.high & 0x00000001ul)) * 0x08080808ul) >> 27;
    my_discs = (((my_bits.low & 0x02040810ul) | (my_bits.high & 0x00000001ul)) * 0x08080808ul) >> 27;

    pattern0 = 121 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag5[pattern0];

    // diag5: H4-D8
    opp_discs = (((opp_bits.low & 0x80000000ul) | (opp_bits.high & 0x08102040ul)) * 0x01010101ul) >> 27;
    my_discs = (((my_bits.low & 0x80000000ul) | (my_bits.high & 0x08102040ul)) * 0x01010101ul) >> 27;

    pattern0 = 121 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag5[pattern0];

    // diag4: E1-H4
    opp_discs = (((opp_bits.low & 0x80402010ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
    my_discs = (((my_bits.low & 0x80402010ul) * 0x01010101ul) & 0xf0000000ul) >> 28;

    pattern0 = 40 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag4[pattern0];

    // diag4: A5-D8
    opp_discs = ((opp_bits.high & 0x08040201ul) * 0x10101010ul) >> 28;
    my_discs = ((my_bits.high & 0x08040201ul) * 0x10101010ul) >> 28;

    pattern0 = 40 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag4[pattern0];

    // diag4: D1-A4
    opp_discs = ((opp_bits.low & 0x01020408ul) * 0x10101010ul) >> 28;
    my_discs = ((my_bits.low & 0x01020408ul) * 0x10101010ul) >> 28;

    pattern0 = 40 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag4[pattern0];

    // diag4: H5-E8
    opp_discs = ((opp_bits.high & 0x10204080ul) * 0x01010101ul) >> 28;
    my_discs = ((my_bits.high & 0x10204080ul) * 0x01010101ul) >> 28;

    pattern0 = 40 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].diag4[pattern0];

    // corner33: A1-C1, A2-C2, A3-C3
    opp_discs = ((opp_bits.low & 0x00000007ul) | ((opp_bits.low & 0x00000700ul) >> 5) | ((opp_bits.low & 0x00070000ul) >> 10));
    my_discs = ((my_bits.low & 0x00000007ul) | ((my_bits.low & 0x00000700ul) >> 5) | ((my_bits.low & 0x00070000ul) >> 10));

    pattern0 = 9841 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].corner33[pattern0];

    // corner33: A8-C8, A7-C7, A6-C6
    opp_discs = (((opp_bits.high & 0x07000000ul) >> 24) | ((opp_bits.high & 0x00070000ul) >> 13) | ((opp_bits.high & 0x00000700ul) >> 2));
    my_discs = (((my_bits.high & 0x07000000ul) >> 24) | ((my_bits.high & 0x00070000ul) >> 13) | ((my_bits.high & 0x00000700ul) >> 2));

    pattern0 = 9841 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].corner33[pattern0];

    // corner33: H1-F1, H2-F2, H3-F3
    tmp = (opp_bits.low & 0x00C0C0C0ul) * 0x00000421ul;
    opp_discs = (((tmp >> 17) & 0x00000049ul)
        | ((tmp >> 15) & 0x00000092ul)
        | ((((opp_bits.low & 0x00202020ul) * 0x00000421ul) >> 13) & 0x00000124ul));

    tmp = (my_bits.low & 0x00C0C0C0ul) * 0x00000421ul;
    my_discs = (((tmp >> 17) & 0x00000049ul)
        | ((tmp >> 15) & 0x00000092ul)
        | ((((my_bits.low & 0x00202020ul) * 0x00000421ul) >> 13) & 0x00000124ul));

    /*
    opp_discs = (((((opp_bits.low & 0x00808080ul) * 0x00000421ul) >> 17) & 0x00000049ul)
        | ((((opp_bits.low & 0x00404040ul) * 0x00000421ul) >> 15) & 0x00000092ul)
        | ((((opp_bits.low & 0x00202020ul) * 0x00000421ul) >> 13) & 0x00000124ul));
    my_discs = (((((my_bits.low & 0x00808080ul) * 0x00000421ul) >> 17) & 0x00000049ul)
        | ((((my_bits.low & 0x00404040ul) * 0x00000421ul) >> 15) & 0x00000092ul)
        | ((((my_bits.low & 0x00202020ul) * 0x00000421ul) >> 13) & 0x00000124ul));
    //*/

    pattern0 = 9841 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].corner33[pattern0];

    // corner33: H8-F8, H7-F7, H6-F6
    opp_discs = (((opp_bits.high & 0x0000c000ul) >> 14) | ((opp_bits.high & 0x00e00000ul) >> 19) | ((opp_bits.high & 0xe0000000ul) >> 24));
    opp_discs = (((opp_discs * 0x0802LU & 0x22110LU) | (opp_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;
    opp_discs |= (opp_bits.high & 0x00002000ul) >> 5;
    my_discs = (((my_bits.high & 0x0000c000ul) >> 14) | ((my_bits.high & 0x00e00000ul) >> 19) | ((my_bits.high & 0xe0000000ul) >> 24));
    my_discs = (((my_discs * 0x0802LU & 0x22110LU) | (my_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;
    my_discs |= (my_bits.high & 0x00002000ul) >> 5;

    pattern0 = 9841 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    score += coeffs[eval_phase].corner33[pattern0];

    // corner52: A1-E1, A2-E2
    opp_discs = (opp_bits.low & 0x0000001ful) | ((opp_bits.low & 0x00000f00ul) >> 3);
    my_discs = (my_bits.low & 0x0000001ful) | ((my_bits.low & 0x00000f00ul) >> 3);

    pattern0 = 29524 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    pattern0 -= ((my_bits.low & 0x00001000ul) >> 12) * 19683;
    pattern0 += ((opp_bits.low & 0x00001000ul) >> 12) * 19683;

    score += coeffs[eval_phase].corner52[pattern0];

    // corner52: A8-E8, A7-E7
    opp_discs = ((opp_bits.high & 0x1f000000ul) >> 24) | ((opp_bits.high & 0x000f0000ul) >> 11);
    my_discs = ((my_bits.high & 0x1f000000ul) >> 24) | ((my_bits.high & 0x000f0000ul) >> 11);

    pattern0 = 29524 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    pattern0 -= ((my_bits.high & 0x00100000ul) >> 20) * 19683;
    pattern0 += ((opp_bits.high & 0x00100000ul) >> 20) * 19683;

    score += coeffs[eval_phase].corner52[pattern0];

    // corner52: H1-D1, H2-D2
    opp_discs = (opp_bits.low & 0x000000f8ul) | ((opp_bits.low & 0x0000e000ul) >> 13);
    opp_discs = (((opp_discs * 0x0802LU & 0x22110LU) | (opp_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;
    my_discs = (my_bits.low & 0x000000f8ul) | ((my_bits.low & 0x0000e000ul) >> 13);
    my_discs = (((my_discs * 0x0802LU & 0x22110LU) | (my_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;

    pattern0 = 29524 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    opp_discs2 = ((opp_bits.low & 0x00001000ul) >> 12) | ((opp_bits.low & 0x00000800ul) >> 10);
    my_discs2 = ((my_bits.low & 0x00001000ul) >> 12) | ((my_bits.low & 0x00000800ul) >> 10);
    pattern0 -= pattern_mask2[my_discs2];
    pattern0 += pattern_mask2[opp_discs2];

    score += coeffs[eval_phase].corner52[pattern0];

    // corner52: H8-D8, H7-D7
    opp_discs = ((opp_bits.high & 0xf8000000ul) >> 24) | ((opp_bits.high & 0x00e00000ul) >> 21);
    opp_discs = (((opp_discs * 0x0802LU & 0x22110LU) | (opp_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;
    my_discs = ((my_bits.high & 0xf8000000ul) >> 24) | ((my_bits.high & 0x00e00000ul) >> 21);
    my_discs = (((my_discs * 0x0802LU & 0x22110LU) | (my_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;

    pattern0 = 29524 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    opp_discs2 = ((opp_bits.high & 0x00100000ul) >> 20) | ((opp_bits.high & 0x00080000ul) >> 18);
    my_discs2 = ((my_bits.high & 0x00100000ul) >> 20) | ((my_bits.high & 0x00080000ul) >> 18);
    pattern0 -= pattern_mask2[my_discs2];
    pattern0 += pattern_mask2[opp_discs2];

    score += coeffs[eval_phase].corner52[pattern0];

    // corner52: A1-A5, B1-B5
    opp_discs = (((opp_bits.low & 0x01010101ul) | (((opp_bits.high & 0x00000001ul) << 4) | ((opp_bits.low & 0x00020202ul) << 11))) * 0x01020408ul) >> 24;
    my_discs = (((my_bits.low & 0x01010101ul) | (((my_bits.high & 0x00000001ul) << 4) | ((my_bits.low & 0x00020202ul) << 11))) * 0x01020408ul) >> 24;

    pattern0 = 29524 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    opp_discs2 = ((opp_bits.low & 0x02000000ul) >> 25) | (opp_bits.high & 0x00000002ul);
    my_discs2 = ((my_bits.low & 0x02000000ul) >> 25) | (my_bits.high & 0x00000002ul);
    pattern0 -= pattern_mask2[my_discs2];
    pattern0 += pattern_mask2[opp_discs2];

    score += coeffs[eval_phase].corner52[pattern0];

    // corner52: H1-H5, G1-G5
    opp_discs = ((((opp_bits.low & 0x80808080ul) >> 4) | ((opp_bits.high & 0x00000080ul) | ((opp_bits.low & 0x00404040ul) << 9))) * 0x00204081ul) >> 24;
    my_discs = ((((my_bits.low & 0x80808080ul) >> 4) | ((my_bits.high & 0x00000080ul) | ((my_bits.low & 0x00404040ul) << 9))) * 0x00204081ul) >> 24;

    pattern0 = 29524 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    opp_discs2 = ((opp_bits.low & 0x40000000ul) >> 30) | ((opp_bits.high & 0x00000040ul) >> 5);
    my_discs2 = ((my_bits.low & 0x40000000ul) >> 30) | ((my_bits.high & 0x00000040ul) >> 5);
    pattern0 -= pattern_mask2[my_discs2];
    pattern0 += pattern_mask2[opp_discs2];

    score += coeffs[eval_phase].corner52[pattern0];

    // corner52: A8-A4, B8-B4
    opp_discs = ((((opp_bits.low & 0x01000000ul) | ((opp_bits.high & 0x02020200ul) >> 9)) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x01020408ul) >> 24;
    opp_discs = (((opp_discs * 0x0802LU & 0x22110LU) | (opp_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;
    my_discs = ((((my_bits.low & 0x01000000ul) | ((my_bits.high & 0x02020200ul) >> 9)) | ((my_bits.high & 0x01010101ul) << 4)) * 0x01020408ul) >> 24;
    my_discs = (((my_discs * 0x0802LU & 0x22110LU) | (my_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;

    pattern0 = 29524 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    opp_discs2 = ((opp_bits.high & 0x00000002ul) >> 1) | ((opp_bits.low & 0x02000000ul) >> 24);
    my_discs2 = ((my_bits.high & 0x00000002ul) >> 1) | ((my_bits.low & 0x02000000ul) >> 24);
    pattern0 -= pattern_mask2[my_discs2];
    pattern0 += pattern_mask2[opp_discs2];

    score += coeffs[eval_phase].corner52[pattern0];

    // corner52: H8-H4, G8-G4
    opp_discs = (((((opp_bits.low & 0x80000000ul) >> 4) | ((opp_bits.high & 0x40404000ul) >> 11)) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) >> 24;
    opp_discs = (((opp_discs * 0x0802LU & 0x22110LU) | (opp_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;
    my_discs = (((((my_bits.low & 0x80000000ul) >> 4) | ((my_bits.high & 0x40404000ul) >> 11)) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) >> 24;
    my_discs = (((my_discs * 0x0802LU & 0x22110LU) | (my_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;

    pattern0 = 29524 - pattern_mask[my_discs];
    pattern0 += pattern_mask[opp_discs];

    opp_discs2 = ((opp_bits.high & 0x00000040ul) >> 6) | ((opp_bits.low & 0x40000000ul) >> 29);
    my_discs2 = ((my_bits.high & 0x00000040ul) >> 6) | ((my_bits.low & 0x40000000ul) >> 29);
    pattern0 -= pattern_mask2[my_discs2];
    pattern0 += pattern_mask2[opp_discs2];

    score += coeffs[eval_phase].corner52[pattern0];

#if TIME_EVAL
    {
        static __int64 sum = 0;
        static int calls = 0;

        calls++;
        sum += (rdtsc() - t0);
        if ((calls & ((1 << 22) - 1)) == 0) {
            printf("%lld cycles / eval\n", sum / calls);
        }
    }
#endif

    return score;
}

#endif

/*
   REMOVE_SPECIFIC_COEFFS
   Removes the interpolated coefficients for a
   specific game phase from memory.
*/

static void
remove_specific_coeffs(int phase) {
    if (coeffs[phase].loaded) {
        if (!coeffs[phase].permanent)
            free_memory_block(coeffs[phase].block);
        coeffs[phase].loaded = 0;
    }
}

/*
   REMOVE_COEFFS
   Removes pattern tables which have gone out of scope from memory.
*/

void
remove_coeffs(int phase) {
    int i;

    for (i = 0; i < phase; i++)
        remove_specific_coeffs(i);
}

/*
   CLEAR_COEFFS
   Remove all coefficients loaded from memory.
*/

void
clear_coeffs(void) {
    int i;

    for (i = 0; i <= 60; i++)
        remove_specific_coeffs(i);
}

#ifdef _MSC_VER
#pragma warning( default : 4244 )
#endif
