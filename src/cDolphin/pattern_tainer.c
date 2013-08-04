
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "utils.h"
#include "eval.h"
#include "coeffs.h"
#include "move.h"
#include "pattern.h"
#include "pattern_tainer.h"

int mirror[6561];
int flip52[59049];
int mirror7[2187];
int mirror6[729];
int mirror5[243];
int mirror4[81];
int mirror3[27];
int mirror82x[59049];
int identity10[59049];
int flip33[19683];
int mirror33[19683];

InfoItem constant;
InfoItem parity;
InfoItem afile[6561], bfile[6561], cfile[6561], dfile[6561];
InfoItem corner52[59049];
InfoItem diag8[6561], diag7[2187], diag6[729], diag5[243], diag4[81];
InfoItem corner33[19683];
InfoItem afile2x[59049];
int global_parity;

__declspec(align(64)) static unsigned short pattern_mask[512];
__declspec(align(64)) static unsigned int pattern_mask2[4];

void
init_pattern_mask2( void ) {
	int i, mask;
	unsigned long scan_bit;
	int power, value;
	for ( mask = 0; mask < 512; mask++ ) {
		scan_bit = 1;
		power = 1;
		value = 0;
		for ( i = 0; i < 9; i++ ) {
			if ( (mask & scan_bit) == scan_bit )
				value += power;
			power *= 3;
			scan_bit <<= 1;
		}
		pattern_mask[mask] = value;
	}

	for ( mask = 0; mask < 4; mask++ ) {
		scan_bit = 1;
		power = 6561;
		value = 0;
		for ( i = 0; i < 2; i++ ) {
			if ( (mask & scan_bit) == scan_bit )
				value += power;
			power *= 3;
			scan_bit <<= 1;
		}
		pattern_mask2[mask] = value;
	}
}

void pattern_setup( void ) {
	int i, j, k;
	int mirror_pattern;
	int power3;
	int flip8[6561], flip5[243], flip3[27];
	int row[10];

	/* Build the pattern tables for 8*1-patterns */

	for (i = 0; i < 8; i++)
		row[i] = 0;

	for (i = 0; i < 6561; i++) {
		mirror_pattern = 0;
		power3 = 1;
		for (j = 7; j >= 0; j--) {
			mirror_pattern += row[j] * power3;
			power3 *= 3;
		}
		/* Create the symmetry map */
		mirror[i] = MIN( i, mirror_pattern );
		flip8[i] = mirror_pattern;

		/* Next configuration */
		j = 0;
		do {  /* The odometer principle */
			row[j]++;
			if (row[j] == 3)
				row[j] = 0;
			j++;
		} while (row[j - 1] == 0 && j < 8);
	}

	/* Build the tables for 7*1-patterns */

	for (i = 0; i < 7; i++)
		row[i] = 0;
	for (i = 0; i < 2187; i++) {
		mirror_pattern = 0;
		power3 = 1;
		for (j = 6; j >= 0; j--) {
			mirror_pattern += row[j] * power3;
			power3 *= 3;
		}
		mirror7[i] = MIN(i, mirror_pattern);

		/* Next configuration */
		j = 0;
		do {  /* The odometer principle */
			row[j]++;
			if (row[j] == 3)
				row[j] = 0;
			j++;
		} while (row[j - 1] == 0 && j < 7);
	}

	/* Build the tables for 6*1-patterns */

	for (i = 0; i < 6; i++)
		row[i] = 0;
	for (i = 0; i < 729; i++) {
		mirror_pattern = 0;
		power3 = 1;
		for (j = 5; j >= 0; j--) {
			mirror_pattern += row[j] * power3;
			power3 *= 3;
		}
		mirror6[i] = MIN(i, mirror_pattern);

		/* Next configuration */
		j = 0;
		do {  /* The odometer principle */
			row[j]++;
			if (row[j] == 3)
				row[j] = 0;
			j++;
		} while (row[j - 1] == 0 && j < 6);
	}

	/* Build the tables for 5*1-patterns */

	for (i = 0; i < 5; i++)
		row[i] = 0;
	for (i = 0; i < 243; i++) {
		mirror_pattern = 0;
		power3 = 1;
		for (j = 4; j >= 0; j--) {
			mirror_pattern += row[j] * power3;
			power3 *= 3;
		}
		mirror5[i] = MIN(i, mirror_pattern);
		flip5[i] = mirror_pattern;

		/* Next configuration */
		j = 0;
		do {  /* The odometer principle */
			row[j]++;
			if ( row[j] == 3 )
				row[j] = 0;
			j++;
		} while (row[j - 1] == 0 && j < 5);
	}

	/* Build the tables for 4*1-patterns */

	for ( i = 0; i < 4; i++ )
		row[i] = 0;
	for ( i = 0; i < 81; i++ ) {
		mirror_pattern = 0;
		power3 = 1;
		for ( j = 3; j >= 0; j-- ) {
			mirror_pattern += row[j] * power3;
			power3 *= 3;
		}
		mirror4[i] = MIN( i, mirror_pattern );

		/* Next configuration */
		j = 0;
		do {  /* The odometer principle */
			row[j]++;
			if ( row[j] == 3 )
				row[j] = 0;
			j++;
		} while ( (row[j - 1] == 0) && (j < 4) );
	}

	/* Build the tables for 3*1-patterns */

	for ( i = 0; i < 3; i++ )
		row[i] = 0;
	for ( i = 0; i < 27; i++ ) {
		mirror_pattern = 0;
		power3 = 1;
		for ( j = 2; j >= 0; j-- ) {
			mirror_pattern += row[j] * power3;
			power3 *= 3;
		}
		mirror3[i] = MIN( i, mirror_pattern );
		flip3[i] = mirror_pattern;

		/* Next configuration */
		j = 0;
		do {  /* The odometer principle */
			row[j]++;
			if ( row[j] == 3 )
				row[j] = 0;
			j++;
		} while ( (row[j - 1] == 0) && (j < 3) );
	}

	/* Build the tables for 5*2-patterns */

	for ( i = 0; i < 243; i++ ) {
		for ( j = 0; j < 243; j++ )
			flip52[243 * i + j] = 243 * flip5[i] + flip5[j];
	}
	for ( i = 0; i < 59049; i++ )
		identity10[i] = i;

	/* Build the tables for 3*3-patterns */

	for ( i = 0; i < 27; i++ ) {
		for ( j = 0; j < 27; j++ )
			for ( k = 0; k < 27; k++ )
				flip33[729 * i + 27 * j + k] =
					729 * flip3[i] + 27 * flip3[j] + flip3[k];
	}

	for ( i = 0; i < 9; i++ )
		row[i] = 0;

	for ( i = 0; i < 19683; i++ ) {
		mirror_pattern =
			row[0] + 3 * row[3] + 9 * row[6] +
			27 * row[1] + 81 * row[4] + 243 * row[7] +
			729 * row[2] + 2187 * row[5] + 6561 * row[8];
		mirror33[i] = MIN( i, mirror_pattern );

		/* Next configuration */
		j = 0;
		do {  /* The odometer principle */
			row[j]++;
			if ( row[j] == 3 )
				row[j] = 0;
			j++;
		} while ( (row[j - 1] == 0) && (j < 9) );
	}

	/* Build the tables for edge2X-patterns */

	for ( i = 0; i < 6561; i++ ) {
		for ( j = 0; j < 3; j++ )
			for ( k = 0; k < 3; k++ )
				mirror82x[i + 6561 * j + 19683 * k] =
					MIN( flip8[i] + 6561 * k + 19683 * j, i + 6561 * j + 19683 * k );
	}

	/* Reset the coefficients for the different patterns */

	constant.solution = 0.0;
	parity.solution = 0.0;
	for ( i = 0; i < 59049; i++ ) {
		afile2x[i].pattern = i;
		corner52[i].pattern = i;
	}
	for ( i = 0; i < 19683; i++ ) {
		corner33[i].pattern = i;
	}
	for ( i = 0; i < 6561; i++ ) {
		afile[i].pattern = i;
		bfile[i].pattern = i;
		cfile[i].pattern = i;
		dfile[i].pattern = i;
		diag8[i].pattern = i;
	}
	for ( i = 0; i < 2187; i++ ) {
		diag7[i].pattern = i;
	}
	for ( i = 0; i < 729; i++ ) {
		diag6[i].pattern = i;
	}
	for ( i = 0; i < 243; i++ ) {
		diag5[i].pattern = i;
	}
	for ( i = 0; i < 81; i++ ) {
		diag4[i].pattern = i;
	}
}

void initialize_non_patterns( const char *base ) {
	char file_name[32];
	FILE *stream;

	sprintf( file_name, "%s.s%d", base, 6 );
	stream = fopen( file_name, "r" );
	if ( stream == NULL ) {
		parity.solution = 0.0;
		constant.solution = 0.0;
	}
	else {
		fscanf( stream, "%lf", &constant.solution );
		fscanf( stream, "%lf", &parity.solution );
		fclose( stream );
	}
}

void initialize_solution( const char *base,
						 InfoItem *item,
						 int count ) {
	char file_name[32];
	float *vals;
	int i;
	int *freq;
	FILE *stream;

	sprintf( file_name, "%s.b%d", base, 6 );
	stream = fopen( file_name, "rb" );
	if ( stream == NULL ) {
		for ( i = 0; i < count; i++ ) {
			item[i].solution = 0.0;
		}
	}
	else {
		vals = (float *)malloc(count * sizeof( float ) );
		freq = (int *)malloc(count * sizeof( int ) );
		fread( vals, sizeof( float ), count, stream );
		fread( freq, sizeof( int ), count, stream );
		fclose( stream );
		for ( i = 0; i < count; i++ ) {
			if ( freq[i] > 0 ) {
				item[i].solution = vals[i];
				if ( fabs( vals[i] ) > 100.0 )
					printf( "%s, i=%d, strange value %.2f, freq=%d\n",
						base, i, vals[i], freq[i] );
			}
			else
				item[i].solution = 0.0;
		}
		free( freq );
		free( vals );
	}
}

double
calc_pattern_score( int color,
				   const BitBoard my_bits,
				   const BitBoard opp_bits ) {
	double score;
	int pattern0;
	unsigned long my_discs, opp_discs;
	unsigned long my_discs2, opp_discs2;
    unsigned int tmp;

	/* The constant feature and the parity feature */

	score = constant.solution;
	if ( disks_played & 1 )
		score += parity.solution * global_parity;

	// afile2x: A1-A8, B2, B7
	opp_discs = ((((opp_bits.low & 0x01010101ul) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
	my_discs = ((((my_bits.low & 0x01010101ul) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;

	pattern0 = 29524 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	opp_discs2 = ((opp_bits.low & 0x00000200ul) >> 9) | ((opp_bits.high & 0x00020000ul) >> 16);
	my_discs2 = ((my_bits.low & 0x00000200ul) >> 9) | ((my_bits.high & 0x00020000ul) >> 16);
	pattern0 -= pattern_mask2[my_discs2];
	pattern0 += pattern_mask2[opp_discs2];

	score += afile2x[mirror82x[pattern0]].solution;

	// afile2x: H1-H8, G2, G7
	opp_discs = (((((opp_bits.low & 0x80808080ul) >> 4) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
	my_discs = (((((my_bits.low & 0x80808080ul) >> 4) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;

	pattern0 = 29524 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	opp_discs2 = ((opp_bits.low & 0x00004000ul) >> 14) | ((opp_bits.high & 0x00400000ul) >> 21);
	my_discs2 = ((my_bits.low & 0x00004000ul) >> 14) | ((my_bits.high & 0x00400000ul) >> 21);
	pattern0 -= pattern_mask2[my_discs2];
	pattern0 += pattern_mask2[opp_discs2];

	score += afile2x[mirror82x[pattern0]].solution;

	// afile2x: A1-H1, B2, G2
	opp_discs = opp_bits.low & 0x000000fful;
	my_discs = my_bits.low & 0x000000fful;

	pattern0 = 29524 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	opp_discs2 = ((opp_bits.low & 0x00000200ul) >> 9) | ((opp_bits.low & 0x00004000ul) >> 13);
	my_discs2 = ((my_bits.low & 0x00000200ul) >> 9) | ((my_bits.low & 0x00004000ul) >> 13);
	pattern0 -= pattern_mask2[my_discs2];
	pattern0 += pattern_mask2[opp_discs2];

	score += afile2x[mirror82x[pattern0]].solution;

	// afile2x: A8-H8, B7, G7
	opp_discs = (opp_bits.high & 0xff000000ul) >> 24;
	my_discs = (my_bits.high & 0xff000000ul) >> 24;

	pattern0 = 29524 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	opp_discs2 = ((opp_bits.high & 0x00020000ul) >> 17) | ((opp_bits.high & 0x00400000ul) >> 21);
	my_discs2 = ((my_bits.high & 0x00020000ul) >> 17) | ((my_bits.high & 0x00400000ul) >> 21);
	pattern0 -= pattern_mask2[my_discs2];
	pattern0 += pattern_mask2[opp_discs2];

	score += afile2x[mirror82x[pattern0]].solution;

	// bfile: B1-B8
	opp_discs = ((((opp_bits.low & 0x02020202ul) | ((opp_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
	my_discs = ((((my_bits.low & 0x02020202ul) | ((my_bits.high & 0x02020202ul) << 4)) * 0x00204081ul) & 0x3fc00000ul) >> 22;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += bfile[mirror[pattern0]].solution;

	// bfile: G1-G8
	opp_discs = (((((opp_bits.low & 0x40404040ul) >> 4) | (opp_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;
	my_discs = (((((my_bits.low & 0x40404040ul) >> 4) | (my_bits.high & 0x40404040ul)) * 0x00204081ul) & 0x7f800000ul) >> 23;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += bfile[mirror[pattern0]].solution;

	// bfile: A2-H2
	opp_discs = (opp_bits.low & 0x0000ff00ul) >> 8;
	my_discs = (my_bits.low & 0x0000ff00ul) >> 8;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += bfile[mirror[pattern0]].solution;

	// bfile: A7-H7
	opp_discs = (opp_bits.high & 0x00ff0000ul) >> 16;
	my_discs = (my_bits.high & 0x00ff0000ul) >> 16;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += bfile[mirror[pattern0]].solution;

	// cfile: C1-C8
	opp_discs = ((((opp_bits.low & 0x04040404ul) | ((opp_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;
	my_discs = ((((my_bits.low & 0x04040404ul) | ((my_bits.high & 0x04040404ul) << 4)) * 0x00204081ul) & 0x7f800000ul) >> 23;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += cfile[mirror[pattern0]].solution;

	// cfile: F1-F8
	opp_discs = (((((opp_bits.low & 0x20202020ul) >> 4) | (opp_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;
	my_discs = (((((my_bits.low & 0x20202020ul) >> 4) | (my_bits.high & 0x20202020ul)) * 0x00204081ul) & 0x3fc00000ul) >> 22;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += cfile[mirror[pattern0]].solution;

	// cfile: A3-H3
	opp_discs = (opp_bits.low & 0x00ff0000ul) >> 16;
	my_discs = (my_bits.low & 0x00ff0000ul) >> 16;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += cfile[mirror[pattern0]].solution;

	// cfile: A6-H6
	opp_discs = (opp_bits.high & 0x0000ff00ul) >> 8;
	my_discs = (my_bits.high & 0x0000ff00ul) >> 8;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += cfile[mirror[pattern0]].solution;

	// dfile: D1-D8
	opp_discs = ((((opp_bits.low & 0x08080808ul) | ((opp_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0xff000000ul) >> 24;
	my_discs = ((((my_bits.low & 0x08080808ul) | ((my_bits.high & 0x08080808ul) << 4)) * 0x00204081ul) & 0xff000000ul) >> 24;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += dfile[mirror[pattern0]].solution;

	// dfile: E1-E8
	opp_discs = (((((opp_bits.low & 0x10101010ul) >> 4) | (opp_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
	my_discs = (((((my_bits.low & 0x10101010ul) >> 4) | (my_bits.high & 0x10101010ul)) * 0x00204081ul) & 0x1fe00000ul) >> 21;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += dfile[mirror[pattern0]].solution;

	// dfile: A4-H4
	opp_discs = (opp_bits.low & 0xff000000ul) >> 24;
	my_discs = (my_bits.low & 0xff000000ul) >> 24;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += dfile[mirror[pattern0]].solution;

	// dfile: A5-H5
	opp_discs = opp_bits.high & 0x000000fful;
	my_discs = my_bits.high & 0x000000fful;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += dfile[mirror[pattern0]].solution;

	// diag8: A1-H8
	opp_discs = ((((opp_bits.low & 0x08040201ul) | (opp_bits.high & 0x80402010ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
	my_discs = ((((my_bits.low & 0x08040201ul) | (my_bits.high & 0x80402010ul)) * 0x01010101ul) & 0xff000000ul) >> 24;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag8[mirror[pattern0]].solution;

	// diag8: H1-A8
	opp_discs = ((((opp_bits.low & 0x10204080ul) | (opp_bits.high & 0x01020408ul)) * 0x01010101ul) & 0xff000000ul) >> 24;
	my_discs = ((((my_bits.low & 0x10204080ul) | (my_bits.high & 0x01020408ul)) * 0x01010101ul) & 0xff000000ul) >> 24;

	pattern0 = 3280 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag8[mirror[pattern0]].solution;

	// diag7: B1-H7
	opp_discs = ((((opp_bits.low & 0x10080402ul) | (opp_bits.high & 0x00804020ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;
	my_discs = ((((my_bits.low & 0x10080402ul) | (my_bits.high & 0x00804020ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;

	pattern0 = 1093 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag7[mirror7[pattern0]].solution;

	// diag7: A2-G8
	opp_discs = ((((opp_bits.low & 0x04020100ul) | (opp_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;
	my_discs = ((((my_bits.low & 0x04020100ul) | (my_bits.high & 0x40201008ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;

	pattern0 = 1093 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag7[mirror7[pattern0]].solution;

	// diag7: G1-A7
	opp_discs = ((((opp_bits.low & 0x08102040ul) | (opp_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;
	my_discs = ((((my_bits.low & 0x08102040ul) | (my_bits.high & 0x00010204ul)) * 0x01010101ul) & 0x7f000000ul) >> 24;

	pattern0 = 1093 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag7[mirror7[pattern0]].solution;

	// diag7: H2-B8
	opp_discs = ((((opp_bits.low & 0x20408000ul) | (opp_bits.high & 0x02040810ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;
	my_discs = ((((my_bits.low & 0x20408000ul) | (my_bits.high & 0x02040810ul)) * 0x01010101ul) & 0xfe000000ul) >> 25;

	pattern0 = 1093 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag7[mirror7[pattern0]].solution;

	// diag6: C1-H6
	opp_discs = ((((opp_bits.low & 0x20100804ul) | (opp_bits.high & 0x00008040ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;
	my_discs = ((((my_bits.low & 0x20100804ul) | (my_bits.high & 0x00008040ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;

	pattern0 = 364 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag6[mirror6[pattern0]].solution;

	// diag6: A3-F8
	opp_discs = ((((opp_bits.low & 0x02010000ul) | (opp_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;
	my_discs = ((((my_bits.low & 0x02010000ul) | (my_bits.high & 0x20100804ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;

	pattern0 = 364 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag6[mirror6[pattern0]].solution;

	// diag6: F1-A6
	opp_discs = ((((opp_bits.low & 0x04081020ul) | (opp_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;
	my_discs = ((((my_bits.low & 0x04081020ul) | (my_bits.high & 0x00000102ul)) * 0x01010101ul) & 0x3f000000ul) >> 24;

	pattern0 = 364 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag6[mirror6[pattern0]].solution;

	// diag6: H3-C8
	opp_discs = ((((opp_bits.low & 0x40800000ul) | (opp_bits.high & 0x04081020ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;
	my_discs = ((((my_bits.low & 0x40800000ul) | (my_bits.high & 0x04081020ul)) * 0x01010101ul) & 0xfc000000ul) >> 26;

	pattern0 = 364 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag6[mirror6[pattern0]].solution;

	// diag5: D1-H5
	opp_discs = ((((opp_bits.low & 0x40201008ul) | (opp_bits.high & 0x00000080ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
	my_discs = ((((my_bits.low & 0x40201008ul) | (my_bits.high & 0x00000080ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;

	pattern0 = 121 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag5[mirror5[pattern0]].solution;

	// diag5: A4-E8
	opp_discs = ((((opp_bits.low & 0x01000000ul) | (opp_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
	my_discs = ((((my_bits.low & 0x01000000ul) | (my_bits.high & 0x10080402ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;

	pattern0 = 121 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag5[mirror5[pattern0]].solution;

	// diag5: E1-A5
	opp_discs = ((((opp_bits.low & 0x02040810ul) | (opp_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;
	my_discs = ((((my_bits.low & 0x02040810ul) | (my_bits.high & 0x00000001ul)) * 0x01010101ul) & 0x1f000000ul) >> 24;

	pattern0 = 121 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag5[mirror5[pattern0]].solution;

	// diag5: H4-D8
	opp_discs = ((((opp_bits.low & 0x80000000ul) | (opp_bits.high & 0x08102040ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;
	my_discs = ((((my_bits.low & 0x80000000ul) | (my_bits.high & 0x08102040ul)) * 0x01010101ul) & 0xf8000000ul) >> 27;

	pattern0 = 121 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag5[mirror5[pattern0]].solution;

	// diag4: E1-H4
	opp_discs = (((opp_bits.low & 0x80402010ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
	my_discs = (((my_bits.low & 0x80402010ul) * 0x01010101ul) & 0xf0000000ul) >> 28;

	pattern0 = 40 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag4[mirror4[pattern0]].solution;

	// diag4: A5-D8
	opp_discs = (((opp_bits.high & 0x08040201ul) * 0x01010101ul) & 0x0f000000ul) >> 24;
	my_discs = (((my_bits.high & 0x08040201ul) * 0x01010101ul) & 0x0f000000ul) >> 24;

	pattern0 = 40 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag4[mirror4[pattern0]].solution;

	// diag4: D1-A4
	opp_discs = (((opp_bits.low & 0x01020408ul) * 0x01010101ul) & 0x0f000000ul) >> 24;
	my_discs = (((my_bits.low & 0x01020408ul) * 0x01010101ul) & 0x0f000000ul) >> 24;

	pattern0 = 40 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag4[mirror4[pattern0]].solution;

	// diag4: H5-E8
	opp_discs = (((opp_bits.high & 0x10204080ul) * 0x01010101ul) & 0xf0000000ul) >> 28;
	my_discs = (((my_bits.high & 0x10204080ul) * 0x01010101ul) & 0xf0000000ul) >> 28;

	pattern0 = 40 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += diag4[mirror4[pattern0]].solution;

	// corner33: A1-C1, A2-C2, A3-C3
	opp_discs = ((opp_bits.low & 0x00000007ul) | ((opp_bits.low & 0x00000700ul) >> 5) | ((opp_bits.low & 0x00070000ul) >> 10));
	my_discs = ((my_bits.low & 0x00000007ul) | ((my_bits.low & 0x00000700ul) >> 5) | ((my_bits.low & 0x00070000ul) >> 10));

	pattern0 = 9841 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += corner33[mirror33[pattern0]].solution;

	// corner33: A8-C8, A7-C7, A6-C6
	opp_discs = (((opp_bits.high & 0x07000000ul) >> 24) | ((opp_bits.high & 0x00070000ul) >> 13) | ((opp_bits.high & 0x00000700ul) >> 2));
	my_discs = (((my_bits.high & 0x07000000ul) >> 24) | ((my_bits.high & 0x00070000ul) >> 13) | ((my_bits.high & 0x00000700ul) >> 2));

	pattern0 = 9841 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += corner33[mirror33[pattern0]].solution;

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

	score += corner33[mirror33[pattern0]].solution;

	// corner33: H8-F8, H7-F7, H6-F6
	opp_discs = (((opp_bits.high & 0x0000c000ul) >> 14) | ((opp_bits.high & 0x00e00000ul) >> 19) | ((opp_bits.high & 0xe0000000ul) >> 24));
	opp_discs = (((opp_discs * 0x0802LU & 0x22110LU) | (opp_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;
	opp_discs |= (opp_bits.high & 0x00002000ul) >> 5;
	my_discs = (((my_bits.high & 0x0000c000ul) >> 14) | ((my_bits.high & 0x00e00000ul) >> 19) | ((my_bits.high & 0xe0000000ul) >> 24));
	my_discs = (((my_discs * 0x0802LU & 0x22110LU) | (my_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;
	my_discs |= (my_bits.high & 0x00002000ul) >> 5;

	pattern0 = 9841 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	score += corner33[mirror33[pattern0]].solution;

	// corner52: A1-E1, A2-E2
	opp_discs = (opp_bits.low & 0x0000001ful) | ((opp_bits.low & 0x00000f00ul) >> 3);
	my_discs = (my_bits.low & 0x0000001ful) | ((my_bits.low & 0x00000f00ul) >> 3);

	pattern0 = 29524 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	pattern0 -= ((my_bits.low & 0x00001000ul) >> 12) * 19683;
	pattern0 += ((opp_bits.low & 0x00001000ul) >> 12) * 19683;

	//score += corner52[flip52[pattern0]].solution;
	score += corner52[pattern0].solution;

	// corner52: A8-E8, A7-E7
	opp_discs = ((opp_bits.high & 0x1f000000ul) >> 24) | ((opp_bits.high & 0x000f0000ul) >> 11);
	my_discs = ((my_bits.high & 0x1f000000ul) >> 24) | ((my_bits.high & 0x000f0000ul) >> 11);

	pattern0 = 29524 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	pattern0 -= ((my_bits.high & 0x00100000ul) >> 20) * 19683;
	pattern0 += ((opp_bits.high & 0x00100000ul) >> 20) * 19683;

	//score += corner52[flip52[pattern0]].solution;
	score += corner52[pattern0].solution;

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

	//score += corner52[flip52[pattern0]].solution;
	score += corner52[pattern0].solution;

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

	//score += corner52[flip52[pattern0]].solution;
	score += corner52[pattern0].solution;

	// corner52: A1-A5, B1-B5
	opp_discs = ((((opp_bits.low & 0x01010101ul) | (((opp_bits.high & 0x00000001ul) << 4) | ((opp_bits.low & 0x00020202ul) << 11))) * 0x00204081ul) & 0x1fe00000ul) >> 21;
	my_discs = ((((my_bits.low & 0x01010101ul) | (((my_bits.high & 0x00000001ul) << 4) | ((my_bits.low & 0x00020202ul) << 11))) * 0x00204081ul) & 0x1fe00000ul) >> 21;

	pattern0 = 29524 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	opp_discs2 = ((opp_bits.low & 0x02000000ul) >> 25) | (opp_bits.high & 0x00000002ul);
	my_discs2 = ((my_bits.low & 0x02000000ul) >> 25) | (my_bits.high & 0x00000002ul);
	pattern0 -= pattern_mask2[my_discs2];
	pattern0 += pattern_mask2[opp_discs2];

	//score += corner52[flip52[pattern0]].solution;
	score += corner52[pattern0].solution;

	// corner52: H1-H5, G1-G5
	opp_discs = (((((opp_bits.low & 0x80808080ul) >> 4) | ((opp_bits.high & 0x00000080ul) | ((opp_bits.low & 0x00404040ul) << 9))) * 0x00204081ul) & 0xff000000ul) >> 24;
	my_discs = (((((my_bits.low & 0x80808080ul) >> 4) | ((my_bits.high & 0x00000080ul) | ((my_bits.low & 0x00404040ul) << 9))) * 0x00204081ul) & 0xff000000ul) >> 24;

	pattern0 = 29524 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	opp_discs2 = ((opp_bits.low & 0x40000000ul) >> 30) | ((opp_bits.high & 0x00000040ul) >> 5);
	my_discs2 = ((my_bits.low & 0x40000000ul) >> 30) | ((my_bits.high & 0x00000040ul) >> 5);
	pattern0 -= pattern_mask2[my_discs2];
	pattern0 += pattern_mask2[opp_discs2];

	//score += corner52[flip52[pattern0]].solution;
	score += corner52[pattern0].solution;

	// corner52: A8-A4, B8-B4
	opp_discs = (((((opp_bits.low & 0x01000000ul) | ((opp_bits.high & 0x02020200ul) >> 9)) | ((opp_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;
	opp_discs = (((opp_discs * 0x0802LU & 0x22110LU) | (opp_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;
	my_discs = (((((my_bits.low & 0x01000000ul) | ((my_bits.high & 0x02020200ul) >> 9)) | ((my_bits.high & 0x01010101ul) << 4)) * 0x00204081ul) & 0x1fe00000ul) >> 21;	
	my_discs = (((my_discs * 0x0802LU & 0x22110LU) | (my_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;

	pattern0 = 29524 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	opp_discs2 = ((opp_bits.high & 0x00000002ul) >> 1) | ((opp_bits.low & 0x02000000ul) >> 24);
	my_discs2 = ((my_bits.high & 0x00000002ul) >> 1) | ((my_bits.low & 0x02000000ul) >> 24);
	pattern0 -= pattern_mask2[my_discs2];
	pattern0 += pattern_mask2[opp_discs2];

	//score += corner52[flip52[pattern0]].solution;
	score += corner52[pattern0].solution;

	// corner52: H8-H4, G8-G4
	opp_discs = ((((((opp_bits.low & 0x80000000ul) >> 4) | ((opp_bits.high & 0x40404000ul) >> 11)) | (opp_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
	opp_discs = (((opp_discs * 0x0802LU & 0x22110LU) | (opp_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;
	my_discs = ((((((my_bits.low & 0x80000000ul) >> 4) | ((my_bits.high & 0x40404000ul) >> 11)) | (my_bits.high & 0x80808080ul)) * 0x00204081ul) & 0xff000000ul) >> 24;
	my_discs = (((my_discs * 0x0802LU & 0x22110LU) | (my_discs * 0x8020LU & 0x88440LU)) * 0x10101LU >> 16) & 0x000000ffLU;

	pattern0 = 29524 - pattern_mask[my_discs];
	pattern0 += pattern_mask[opp_discs];

	opp_discs2 = ((opp_bits.high & 0x00000040ul) >> 6) | ((opp_bits.low & 0x40000000ul) >> 29);
	my_discs2 = ((my_bits.high & 0x00000040ul) >> 6) | ((my_bits.low & 0x40000000ul) >> 29);
	pattern0 -= pattern_mask2[my_discs2];
	pattern0 += pattern_mask2[opp_discs2];

	//score += corner52[flip52[pattern0]].solution;
	score += corner52[pattern0].solution;

	return score;
}

void
initialize_pattern_coffes( void ) {

	pattern_setup();
	init_pattern_mask2();

	global_parity = 0;
	initialize_non_patterns( "main" );

	initialize_solution("afile2x", afile2x, 59049);
	initialize_solution("bfile", bfile, 6561);
	initialize_solution("cfile", cfile, 6561);
	initialize_solution("dfile", dfile, 6561);
	initialize_solution("diag8", diag8, 6561);
	initialize_solution("diag7", diag7, 2187);
	initialize_solution("diag6", diag6, 729);
	initialize_solution("diag5", diag5, 243);
	initialize_solution("diag4", diag4, 81);
	initialize_solution("corner33", corner33, 19683);
	initialize_solution("corner52", corner52, 59049);
}
