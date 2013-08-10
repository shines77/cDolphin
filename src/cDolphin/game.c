/*
   File:           game.c

   Created:        2007-07-10

   Modified:       2013-08-10

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       All the routines needed to play a game.
*/

#if !defined(_WIN32) && !defined( _WIN32_WCE ) && !defined( __linux__ ) && !defined( __CYGWIN__ )
#include "dir.h"
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include "direct.h"
#endif

#ifndef _WIN32_WCE
#include <assert.h>
#include <time.h>
#endif

#include "utils.h"
#include "colour.h"
#include "board.h"
#include "bitboard.h"
#include "global.h"
#include "counter.h"
#include "hash.h"
#include "display.h"
#include "eval.h"
#include "game.h"
#include "coeffs.h"
#include "midgame.h"
#include "endgame.h"
#include "pv.h"
#include "move.h"
#include "myrandom.h"
#include "pattern.h"
#include "probcut.h"
#include "search.h"
#include "timer.h"
#include "bitbmob.h"
#include "stable.h"
#include "osfbook.h"
#include "bitbtest.h"
#include "bitbtest1.h"
#include "bitbtest2.h"
#include "bitbtest4.h"
#include "bitbtest5.h"
#include "bitbtest6.h"

/* The pre-ordering depth used when guessing the opponent's move */
#define PONDER_DEPTH             8

/* The depth to abandon the book move and try to solve the position
   in time control games. */
#define FORCE_BOOK_SOLVE         30

/* The somewhat arbitrary point where endgame typically commences
   when Zebra is running on today's hardware */
#define TYPICAL_SOLVE            27

/* The recommended extra depth gained by switching to the endgame searcher;
   i.e., use the endgame solver with n+ENDGAME_OFFSET empty instead of the
   midgame module with depth n. */
#define ENDGAME_OFFSET           7                 /* WAS: 4 */

/* Same as ENDGAME_OFFSET but forces a solve to begin */
#define ENDGAME_FORCE_OFFSET     3

/* Solve earlier for lopsided positions? */
#define ADAPTIVE_SOLVE_DEPTH     FALSE

/* The depth where no time is to be gained by using the fact that
   there is only one move available. */
#ifdef _WIN32_WCE
#define DISABLE_FORCED_MOVES     70
#else
#define DISABLE_FORCED_MOVES     60
#endif

/* The file to which compute_move() writes its status reports */
#define LOG_FILE_NAME            "dolphin.log"

/* The maximum length of any system path. */
#define MAX_PATH_LENGTH          2048

static const char *forced_opening = NULL;
static char log_file_path[MAX_PATH_LENGTH];
static double last_time_used;
static int max_depth_reached;
#ifdef _WIN32_WCE
static int use_log_file = FALSE;
#else
static int use_log_file = TRUE;
#endif
static int play_human_openings = TRUE;
static int play_thor_match_openings = TRUE;
//static int game_evaluated_count;
static int komi = 0;
//static int prefix_move = 0;
static int endgame_performed[3];
//static EvaluatedMove evaluated_list[60];

/* Flag bits and shifts*/
#define NULL_MOVE                 0
#define BLACK_TO_MOVE             1
#define WHITE_TO_MOVE             2
#define WLD_SOLVED                4
#define NOT_TRAVERSED             8
#define FULL_SOLVED               16
#define PRIVATE_NODE              32
#define DEPTH_SHIFT               10

/*
  TOGGLE_STATUS_LOG
  Enable/disable the use of logging all the output that the
  text version of Zebra would output to the screen.
*/

void
toggle_status_log( int write_log ) {
	use_log_file = write_log;
}

#define TEST_BOARD_MAX  128

#if 1

void
do_some_tests() {
	// none
}

#else

void
do_some_tests() {
	BitBoard my_bits_t[TEST_BOARD_MAX], opp_bits_t[TEST_BOARD_MAX];
	int board2[64];
	BitBoard my_bits, opp_bits;
	BitBoard bb_flips1, bb_flips2, bb_flips3, bb_flips4;
	int flipped1, flipped2;
	int start_time;
	int count;
	int i, j;
    int diff_count = 0;
    //int diff_count2 = 0;
	int diff_count3 = 0;
    //int mob1, mob2;
	int sc1, sc2;

	srand( (unsigned)time( NULL ) );

	for ( i = 0; i < TEST_BOARD_MAX; i++ ) {
		for ( j = 0; j < 64; j++ )
			board2[j] = rand() % 3;
		/*
		board2[0] = 0;
		board2[7] = 0;
		board2[56] = 0;
		board2[63] = 0;
		board2[1] = 0;
		board2[7] = 0;
		//*/

		board2[27] = rand() % 2 + 1;
		board2[28] = rand() % 2 + 1;
		board2[35] = rand() % 2 + 1;
		board2[36] = rand() % 2 + 1;

		set_bitboards(board2, CHESS_BLACK, &my_bits, &opp_bits);
		my_bits_t[i] = my_bits;
		opp_bits_t[i] = opp_bits;
	}

#if 0
	count = 0;
	for ( i = 0; i < 1000000; i++ ) {
		for ( j = 0; j < 64; j++ )
			board2[j] = rand() % 3;
		board2[27] = rand() % 2 + 1;
		board2[28] = rand() % 2 + 1;
		board2[35] = rand() % 2 + 1;
		board2[36] = rand() % 2 + 1;
		set_bitboards(board2, CHESS_BLACK, &my_bits, &opp_bits);

		/*
		my_bits.low = 0x11111111;
		my_bits.high = 0x22222222;
		opp_bits.low = 0x33333333;
		opp_bits.high = 0x44444444;
		//*/

		mob1 = bitboard_mobility(my_bits, opp_bits);
		mob2 = bitboard_mobility1(my_bits, opp_bits);

		///*
		if (mob1 != mob2) {
			diff_count2++;
		}
		//*/
		count++;
	}

	printf("mobility total test: %d, different count: %d.\n\n", count, diff_count2);
	//system( "pause" );
	exit(0);

#endif

#if 0
	///*
	count = 0;
	for ( i = 0; i < 1000000; i++ ) {
		for ( j = 0; j < 64; j++ )
			board2[j] = rand() % 3;
		/*
		board2[0] = 0;
		board2[7] = 0;
		board2[56] = 0;
		board2[63] = 0;
		//*/
		board2[27] = rand() % 2 + 1;
		board2[28] = rand() % 2 + 1;
		board2[35] = rand() % 2 + 1;
		board2[36] = rand() % 2 + 1;
		set_bitboards(board2, CHESS_BLACK, &my_bits, &opp_bits);
		/*
		// A1
		my_bits.low = 0x30788804;
		my_bits.high = 0xa101461c;
		opp_bits.low = 0x4b073780;
		opp_bits.high = 0x040438c3;

		// A1
		my_bits.low = 0x760cfa08;
		my_bits.high = 0x006024c6;
		opp_bits.low = 0x09e004b2;
		opp_bits.high = 0xaf1b4838;
		//*/

		/*
		BitBoard move_bit;
		bitboard_set_bit(&move_bit, A1);
		BITBOARD_ANDNOT(my_bits, move_bit);
		BITBOARD_ANDNOT(opp_bits, move_bit);
		//*/

		flipped1 = TestFlips_bitboard[A1](my_bits, opp_bits);
		//bb_flips1.high = bb_flips.high | my_bits.high;
		//bb_flips1.low = bb_flips.low | my_bits.low;
		//bitboard_or_bit( &bb_flips1, B1 );
		bb_flips1 = bb_flips;
		flipped2 = TestFlips_bitboard6[A1](my_bits, opp_bits);
		bb_flips2 = bb_flips;

		///*
		if (flipped1 != flipped2 || bb_flips1.high != bb_flips2.high || bb_flips1.low != bb_flips2.low) {
			diff_count++;
		}
		//*/
		count++;
	}

	printf("total test: %d, different count: %d.\n\n", count, diff_count);
	//system( "pause" );
	exit(0);
	//*/
#endif

#if 1
	count = 0;
	start_time = clock();
	for ( i = 0; i < 100000; i++ ) {
		for ( j = 0; j < 64; j++ )
			board2[j] = rand() % 3;
		/*
		board2[0] = 0;
		board2[7] = 0;
		board2[56] = 0;
		board2[63] = 0;
		//*/
		board2[27] = rand() % 2 + 1;
		board2[28] = rand() % 2 + 1;
		board2[35] = rand() % 2 + 1;
		board2[36] = rand() % 2 + 1;
		set_bitboards(board2, CHESS_BLACK, &my_bits, &opp_bits);
		/*
		my_bits.low = 0x40404040ul;
		my_bits.high = 0x40404040ul;
		opp_bits.low = 0x00000000ul;
		opp_bits.high = 0x00000000ul;

		my_bits.low = 0x8323f4c4;
		my_bits.high = 0x113698c4;
		opp_bits.low = 0x7c540b32;
		opp_bits.high = 0x6e81651a;
		//*/

		sc1 = pattern_evaluation(CHESS_BLACK, my_bits, opp_bits);
		sc2 = pattern_evaluation2(CHESS_BLACK, my_bits, opp_bits);

		if (sc1 != sc2) {
			diff_count3++;
		}
		count++;
	}

	printf("pattern total test: %d, different count: %d.\n\n", count, diff_count3);
	printf("use time: %d ms.\n", clock() - start_time);
	printf("\n");
	//system( "pause" );
	exit(0);
#endif

	count = 0;
	//srand( 100 );
	start_time = clock();
	bb_flips3.low = 0;
	bb_flips3.high = 0;
	for ( i = 0; i < 50000003; i++ ) {
		my_bits = my_bits_t[i & (TEST_BOARD_MAX - 1)];
		opp_bits = opp_bits_t[i & (TEST_BOARD_MAX - 1)];

		flipped1 = TestFlips_bitboard4[A1](my_bits, opp_bits);
		bb_flips1 = bb_flips;
		bb_flips3.low ^= bb_flips1.low;
		bb_flips3.high ^= bb_flips1.high;

		count++;
	}

	printf("low: 0x%08X, high: 0x%08X\n", bb_flips3.low, bb_flips3.high);
	printf("use time: %d ms.\n", clock() - start_time);
	printf("\n");

	count = 0;
	start_time = clock();
	bb_flips4.low = 0;
	bb_flips4.high = 0;
	for ( i = 0; i < 50000003; i++ ) {
		my_bits = my_bits_t[i & (TEST_BOARD_MAX - 1)];
		opp_bits = opp_bits_t[i & (TEST_BOARD_MAX - 1)];

		flipped2 = TestFlips_bitboard1[A1](my_bits.high, my_bits.low, opp_bits.high, opp_bits.low);
		bb_flips2 = bb_flips;
		bb_flips4.low ^= bb_flips2.low;
		bb_flips4.high ^= bb_flips2.high;

		count++;
	}

	printf("low: 0x%08X, high: 0x%08X\n", bb_flips4.low, bb_flips4.high);
	printf("use time: %d ms.\n", clock() - start_time);
	printf("\n");

	count = 0;
	start_time = clock();
	bb_flips4.low = 0;
	bb_flips4.high = 0;
	for ( i = 0; i < 50000003; i++ ) {
		my_bits = my_bits_t[i & (TEST_BOARD_MAX - 1)];
		opp_bits = opp_bits_t[i & (TEST_BOARD_MAX - 1)];

		flipped2 = TestFlips_bitboard6[A1](my_bits, opp_bits);
		bb_flips2 = bb_flips;
		bb_flips4.low ^= bb_flips2.low;
		bb_flips4.high ^= bb_flips2.high;

		count++;
	}

	printf("low: 0x%08X, high: 0x%08X\n", bb_flips4.low, bb_flips4.high);
	printf("use time: %d ms.\n", clock() - start_time);
	printf("\n");

	count = 0;
	start_time = clock();
	bb_flips4.low = 0;
	bb_flips4.high = 0;
	for ( i = 0; i < 50000003; i++ ) {
		my_bits = my_bits_t[i & (TEST_BOARD_MAX - 1)];
		opp_bits = opp_bits_t[i & (TEST_BOARD_MAX - 1)];

		flipped2 = TestFlips_bitboard5[A1](my_bits, opp_bits);
		bb_flips2 = bb_flips;
		bb_flips4.low ^= bb_flips2.low;
		bb_flips4.high ^= bb_flips2.high;

		count++;
	}

	printf("low: 0x%08X, high: 0x%08X\n", bb_flips4.low, bb_flips4.high);
	printf("use time: %d ms.\n", clock() - start_time);
	printf("\n");

	system( "pause" );
	exit(0);
}

#endif

/*
   GLOBAL_SETUP
   Initialize the different sub-systems.
*/

void
global_setup( int use_random, int hash_bits ) {
	FILE *log_file;
	time_t timer;

	/* Clear the log file. No error handling done. */

#ifdef __linux__
	strcpy( log_file_path, LOG_FILE_NAME );
#else
	char directory_buffer[MAX_PATH_LENGTH];
	_getcwd( directory_buffer, MAX_PATH_LENGTH );
	if ( directory_buffer[strlen( directory_buffer ) - 1] == '\\' )
		sprintf( log_file_path, "%s%s", directory_buffer, LOG_FILE_NAME );
	else
		sprintf( log_file_path, "%s\\%s", directory_buffer, LOG_FILE_NAME );
#endif

	if ( use_log_file ) {
		log_file = fopen( log_file_path, "w" );
		if ( log_file != NULL ) {
			time( &timer );
			fprintf( log_file, "%s %s\n", "Log file created", ctime( &timer ) );
			fprintf( log_file, "%s %s %s\n", "Engine compiled", __DATE__, __TIME__ );
			fclose( log_file );
		}
	}

	if ( use_random ) {
		time( &timer );
		my_srandom( (int)timer );
	}
	else
		my_srandom( 1 );

    init_mmx();
	init_disc_set_table();

	hash_init( hash_bits );
	init_square_mask();
	init_neighborhood();
	init_disc_mask();
	init_disc_mask_4();
	init_disc_mask_5();
	init_disc_mask_6();

	init_moves();
	init_patterns();
	init_pattern_mask();
	init_coeffs();
	init_timer();
	init_probcut();
	init_stable();
	setup_search();
}

/*
   GLOBAL_TERMINATE
   Free all dynamically allocated memory.
*/

void
global_terminate( void ) {
	hash_free();
	clear_coeffs();
	//clear_osf();
}

/*
   SETUP_GAME
   Prepares the board.
*/

static void
setup_game( const char *file_name, int *color ) {
	char buffer[128];
	int i, j;
	int pos, token;
	FILE *stream;
	BitBoard my_bits, opp_bits;

	for ( i = 0; i < 8; i++ ) {
		for ( j = 0; j < 8; j++ ) {
			pos = 8 * i + j;
			board[pos] = CHESS_EMPTY;
		}
	}

	if ( file_name == NULL ) {
		board[E4] = board[D5] = CHESS_BLACK;
		board[D4] = board[E5] = CHESS_WHITE;
		*color = CHESS_BLACK;
	}
	else {
		stream = fopen( file_name, "r" );
		if ( stream == NULL ) {
			printf( "%s '%s'\n", "Cannot open game file", file_name );
			exit( EXIT_FAILURE );
		}
		fgets( buffer, 66, stream );
		token = 0;
		for ( i = 0; i < 8; i++ ) {
			for ( j = 0; j < 8; j++ ) {
				pos = 8 * i + j;
				switch ( buffer[token] ) {
				case '*':
				case 'X':
				case 'B':
				case 'b':
					board[pos] = CHESS_BLACK;
					break;
				case 'O':
				case '0':
				case 'W':
				case 'w':
					board[pos] = CHESS_WHITE;
					break;
				case '-':
				case '.':
					board[pos] = CHESS_EMPTY;
					break;
				default:
#ifdef TEXT_BASED
					printf( "%s '%c' %s\n", "Unrecognized character",
						buffer[pos], "in game file");
					exit( EXIT_FAILURE );
#endif
					break;
				}
				token++;
			}
		}

		fgets( buffer, 10, stream );
		if ( buffer[0] == 'B' || buffer[0] == 'b' || buffer[0] == 'X' ||
			buffer[0] == 'x' || buffer[0] == '*' )
			*color = CHESS_BLACK;
		else if ( buffer[0] == 'W' || buffer[0] == 'w' || buffer[0] == 'O' ||
			buffer[0] == 'o' || buffer[0] == '0' )
			*color = CHESS_WHITE;
		else {
			printf( "%s '%c' %s\n", "Unrecognized character", buffer[0],
				"in game file");
			exit( EXIT_FAILURE );
		}
	}
	//disks_played = disc_count2( CHESS_BLACK ) + disc_count2( CHESS_WHITE ) - 4;

	set_bitboards( board, *color, &my_bits, &opp_bits );
	//disks_played = disc_count2( CHESS_BLACK ) + disc_count2( CHESS_WHITE ) - 4;
	disks_played = disc_count( my_bits, opp_bits, (*color == CHESS_BLACK) ) +
		disc_count( my_bits, opp_bits, (*color == CHESS_WHITE) ) - 4;

	hash_determine_values2( board, *color );

	/* Make the game score look right */

	if ( *color == CHESS_BLACK )
		score_sheet_row = -1;
	else {
		black_moves[0] = PASS_MOVE;
		score_sheet_row = 0;
	}
}

#ifdef DOLPHIN_GGS

/*
   SETUP_GAME_GGS
   Prepares the board.
*/

static void
setup_game_ggs( int color ) {
	BitBoard my_bits, opp_bits;
	set_bitboards( board, color, &my_bits, &opp_bits );

	//disks_played = disc_count2( CHESS_BLACK ) + disc_count2( CHESS_WHITE ) - 4;
	disks_played = disc_count( my_bits, opp_bits, (color == CHESS_BLACK) ) +
		disc_count( my_bits, opp_bits, (color == CHESS_WHITE) ) - 4;

	hash_determine_values2( board, color );

	/* Make the game score look right */

	if ( color == CHESS_BLACK )
		score_sheet_row = -1;
	else {
		black_moves[0] = PASS_MOVE;
		score_sheet_row = 0;
	}
}

#endif

/*
   GAME_INIT
   Prepare the relevant data structures so that a game
   can be played. The position is read from the file
   specified by FILE_NAME.
*/

void
game_init( const char *file_name, int *color ) {
	setup_game( file_name, color );
	setup_search();
	setup_midgame();
	setup_end();
	init_eval();

	clear_ponder_times();

	reset_counter( &total_nodes );
	reset_counter( &total_evaluations );

	total_time = 0.0;
	max_depth_reached = 0;
	last_time_used = 0.0;
	endgame_performed[CHESS_BLACK] = endgame_performed[CHESS_WHITE] = FALSE;
}

#ifdef DOLPHIN_GGS

/*
   GAME_INIT_GGS
   Prepare the relevant data structures so that a game
   can be played. The position is read from the file
   specified by FILE_NAME.
*/

void
game_init_ggs( int color ) {
	setup_game_ggs( color );
	setup_search();
	setup_midgame();
	setup_end();
	init_eval();

	clear_ponder_times();

	reset_counter( &total_nodes );
	reset_counter( &total_evaluations );

	total_time = 0.0;
	max_depth_reached = 0;
	last_time_used = 0.0;
	endgame_performed[CHESS_BLACK] = endgame_performed[CHESS_WHITE] = FALSE;
}

#endif

/*
  SET_KOMI
  Set the endgame komi value.
*/

void
set_komi( int in_komi ) {
	komi = in_komi;
}

/*
  TOGGLE_HUMAN_OPENINGS
  Specifies whether the Thor statistics should be queried for
  openings moves before resorting to the usual opening book.
*/

void
toggle_human_openings( int toggle ) {
	play_human_openings = toggle;
}

/*
  TOGGLE_THOR_MATCH_OPENINGS
  Specifies whether matching Thor games are used as opening book
  before resorting to the usual opening book.
*/

void
toggle_thor_match_openings( int toggle ) {
	play_thor_match_openings = toggle;
}

/*
  SET_FORCED_OPENING
  Specifies an opening line that Zebra is forced to follow when playing.
*/

void
set_forced_opening( const char *opening_str ) {
	forced_opening = opening_str;
}

int
compute_move( BitBoard my_bits,
			 BitBoard opp_bits,
			 int color,
			 int update_all,
			 int my_time,
			 int my_incr,
			 int timed_depth,
			 int book,
			 int mid,
			 int exact,
			 int wld,
			 int search_forced,
			 EvaluationType *eval_info ) {
	FILE *log_file;
	EvaluationType book_eval_info;
	EvaluationType end_eval_info;
	EvaluationType mid_eval_info;
	char *eval_str;
	double midgame_diff;
	enum { INTERRUPTED_MOVE, BOOK_MOVE, MIDGAME_MOVE, ENDGAME_MOVE } move_type;
	int i;
	int curr_move, midgame_move;
	int empties;
	int midgame_depth, interrupted_depth, max_depth;
	int book_move_found;
	int endgame_reached;
	int offset;

	log_file = NULL;
	if ( use_log_file )
		log_file = fopen( log_file_path, "a" );

	//display_board( log_file, board, color, FALSE, FALSE, FALSE );

	///*
	if ( log_file )
		display_board2( log_file, board, color, FALSE, FALSE, FALSE );
	//*/

	set_times( (int)floor( (double)my_time ), (int)floor( (double)my_time ) );
	set_evals ( 0.0f, 0.0f );

	/* Initialize various components of the move system */

	if ( color == CHESS_BLACK ) {
		piece_count[CHESS_BLACK][disks_played] = disc_count( my_bits, opp_bits, TRUE );
		piece_count[CHESS_WHITE][disks_played] = disc_count( my_bits, opp_bits, FALSE );
	}
	else {
		piece_count[CHESS_BLACK][disks_played] = disc_count( my_bits, opp_bits, FALSE );
		piece_count[CHESS_WHITE][disks_played] = disc_count( my_bits, opp_bits, TRUE );
	}
	init_moves();
	generate_all( my_bits, opp_bits, color );
	hash_determine_values( my_bits, opp_bits, color );

	calculate_perturbation();

	if ( log_file ) {
		fprintf( log_file, "%d %s: ", move_count[disks_played], "moves generated" );
		for ( i = 0; i < move_count[disks_played]; i++ )
			fprintf( log_file, "%c%c ", TO_SQUARE( move_list[disks_played][i] ) );
		fputs( "\n", log_file );
	}

	if ( update_all ) {
		reset_counter( &leaf_nodes );
		reset_counter( &tree_nodes );
		reset_counter( &total_nodes );
	}

	for ( i = 0; i < 64; i++ )
		evals[disks_played][i] = 0;
	max_depth_reached = 1;
	empties = 60 - disks_played;
	reset_buffer_display();

	determine_move_time( my_time, my_incr, disks_played + 4 );
	if ( get_ponder_move() == _NULL_MOVE )
		clear_ponder_times();

	remove_coeffs( disks_played );

	/* No feasible moves? */

	if ( move_count[disks_played] == 0 ) {
		*eval_info = create_eval_info( PASS_EVAL, UNSOLVED_POSITION,
			(int)0.0, 0.0, 0, FALSE );
		set_current_eval( *eval_info );
		if ( echo ) {
			eval_str = produce_eval_text( *eval_info, FALSE );
			send_status( "-->         " );
			send_status( "%-8s  ", eval_str );
			display_status( stdout, FALSE );
			free( eval_str );
		}
		if ( log_file ) {
			fprintf( log_file, "%s: %s\n", "Best move", "pass" );
			fclose( log_file );
		}
		last_time_used = 0.0;
		clear_pv();
		return PASS_MOVE;
	}

	/* If there is only one move available:
		Don't waste any time, unless told so or very close to the end,
		searching the position. */

	if ( (empties > DISABLE_FORCED_MOVES) &&
		(move_count[disks_played] == 1) &&
		!search_forced ) {  /* Forced move */
		*eval_info = create_eval_info( FORCED_EVAL, UNSOLVED_POSITION,
			(int)0.0, 0.0, 0, FALSE );
		set_current_eval( *eval_info );
		if ( echo ) {
			eval_str = produce_eval_text( *eval_info, FALSE );
			send_status( "-->         " );
			send_status( "%-8s  ", eval_str );
			free( eval_str );
			send_status( "%c%c ", TO_SQUARE( move_list[disks_played][0] ) );
			display_status( stdout, FALSE );
		}
		if ( log_file ) {
			fprintf( log_file, "%s: %c%c  (%s)\n", "Best move",
				TO_SQUARE(move_list[disks_played][0]), "forced" );
			fclose( log_file );
		}
		last_time_used = 0.0;
		return move_list[disks_played][0];
	}

	/* Mark the search as interrupted until a successful search
		has been performed. */

	move_type = INTERRUPTED_MOVE;
	interrupted_depth = 0;
	curr_move = move_list[disks_played][0];

	/* Check the opening book for midgame moves */

	book_move_found = FALSE;
	midgame_move = PASS_MOVE;

	///*
	if ( forced_opening != NULL ) {
		/* Check if the position fits the currently forced opening */
		curr_move = check_forced_opening( color, forced_opening );
		if ( curr_move != PASS_MOVE ) {
			book_eval_info = create_eval_info( UNDEFINED_EVAL, UNSOLVED_POSITION,
				0, 0.0, 0, TRUE );
			midgame_move = curr_move;
			book_move_found = TRUE;
			move_type = BOOK_MOVE;
			if ( echo ) {
				send_status( "-->   Forced opening move        " );
				if ( get_ponder_move() != _NULL_MOVE )
					send_status( "{%c%c} ", TO_SQUARE( get_ponder_move() ) );
				send_status( "%c%c", TO_SQUARE( curr_move ) );
				display_status( stdout, FALSE );
			}
			clear_pv();
			pv_depth[0] = 1;
			pv[0][0] = curr_move;
		}
	}
	//*/

	/*
	if ( !book_move_found && play_thor_match_openings ) {

		///-* Optionally use the Thor database as opening book. *-/

		int threshold = 2;
		database_search( board, color );
		if ( get_match_count() >= threshold ) {
			int game_index = (my_random() >> 8) % get_match_count();
			curr_move = get_thor_game_move( game_index, disks_played );

			if ( valid_move( curr_move, color ) ) {
				book_eval_info = create_eval_info( UNDEFINED_EVAL, UNSOLVED_POSITION,
					0, 0.0, 0, TRUE );
				midgame_move = curr_move;
				book_move_found = TRUE;
				move_type = BOOK_MOVE;
				if ( echo ) {
					send_status( "-->   %s        ", "Thor database" );
					if ( get_ponder_move() != _NULL_MOVE )
						send_status( "{%c%c} ", TO_SQUARE( get_ponder_move() ) );
					send_status( "%c%c", TO_SQUARE( curr_move ) );
					display_status( stdout, FALSE );
				}
				clear_pv();
				pv_depth[0] = 1;
				pv[0][0] = curr_move;
			}
			else {
				printf( "Thor book move %d is invalid!", curr_move );
				exit( EXIT_FAILURE );
			}
		}
	}
	//*/

	/*
	if ( !book_move_found && play_human_openings && book ) {
		///-* Check Thor statistics for a move *-/
		curr_move = choose_thor_opening_move( board, color, FALSE );
		if ( curr_move != PASS ) {
			book_eval_info = create_eval_info( UNDEFINED_EVAL, UNSOLVED_POSITION,
				0, 0.0, 0, TRUE );
			midgame_move = curr_move;
			book_move_found = TRUE;
			move_type = BOOK_MOVE;
			if ( echo ) {
				send_status( "-->   %s        ", THOR_TEXT );
				if ( get_ponder_move() != _NULL_MOVE )
					send_status( "{%c%c} ", TO_SQUARE( get_ponder_move() ) );
				send_status( "%c%c", TO_SQUARE( curr_move ) );
				display_status( stdout, FALSE );
			}
			clear_pv();
			pv_depth[0] = 1;
			pv[0][0] = curr_move;
		}
	}
	//*/

	///*
	if ( !book_move_found && book ) {  /* Check ordinary opening book */
		int flags = 0;

		if ( empties <= FORCE_BOOK_SOLVE ) {
			if ( empties <= wld )
				flags = WLD_SOLVED;
			if ( empties <= exact )
				flags = FULL_SOLVED;
		}
		fill_move_alternatives( my_bits, opp_bits, color, flags );
		curr_move = get_book_move( my_bits, opp_bits, color, update_all, &book_eval_info );
		if ( curr_move != PASS_MOVE ) {
			set_current_eval( book_eval_info );
			midgame_move = curr_move;
			book_move_found = TRUE;
			move_type = BOOK_MOVE;
			display_status( stdout, FALSE );
		}
	}
	//*/

	/* Use iterative deepening in the midgame searches until the endgame
		is reached. If an endgame search already has been performed,
		make a much more shallow midgame search. Also perform much more
		shallow searches when there is no time limit and hence no danger
		starting to solve only to get interrupted. */

	if ( !timed_depth && (empties <= MAX( exact, wld ) ) )
		mid = MAX( MIN( MIN( mid, empties - 7 ), 28 ), 2 );

	endgame_reached = !timed_depth && endgame_performed[color];

	g_save_hash1 = g_hash1;
	g_save_hash2 = g_hash2;

	if ( !book_move_found && !endgame_reached ) {
		clear_panic_abort();
		clear_midgame_abort();
		toggle_midgame_abort_check( update_all );
		toggle_midgame_hash_usage( TRUE, TRUE );

		if ( timed_depth )
			max_depth = 64;
		else if ( empties <= MAX( exact, wld ) )
			max_depth = MAX( MIN( MIN( mid, empties - 12 ), 18 ), 2 );
		else
			max_depth = mid;
		midgame_depth = MIN( 2, max_depth );
		do {
			max_depth_reached = midgame_depth;
			midgame_move = middle_game( my_bits, opp_bits, midgame_depth, color,
				update_all, &mid_eval_info );
			set_current_eval( mid_eval_info );
			midgame_diff = 1.3 * mid_eval_info.score / 128.0;
			if ( color == CHESS_BLACK )
				midgame_diff -= komi;
			else
				midgame_diff += komi;
			if ( timed_depth ) {  /* Check if the endgame zone has been reached */
				offset = ENDGAME_OFFSET;

				/* These constants were chosen rather arbitrarily but intend
					to make Zebra solve earlier if the position is lopsided. */

				if ( is_panic_abort() )
					offset--;

#if ADAPTIVE_SOLVE_DEPTH
				if ( fabs( midgame_diff ) > 4.0 )
					offset++;
				if ( fabs( midgame_diff ) > 8.0 )
					offset++;
				if ( fabs( midgame_diff ) > 16.0 )
					offset++;
				if ( fabs( midgame_diff ) > 32.0 )
					offset++;
				if ( fabs( midgame_diff ) > 48.0 )
					offset++;
				if ( fabs( midgame_diff ) > 56.0 )
					offset++;
#endif

				if ( endgame_performed[color] )
					offset += 2;
				if ( ((midgame_depth + offset + TYPICAL_SOLVE) >= 2 * empties) ||
					(midgame_depth + ENDGAME_OFFSET >= empties) )
					endgame_reached = TRUE;
			}
			midgame_depth++;
		} while ( !is_panic_abort() &&
			!is_midgame_abort() &&
			!force_return &&
			(midgame_depth <= max_depth) &&
			(midgame_depth + disks_played <= 61) &&
			!endgame_reached );

		if ( echo )
			display_status( stdout, FALSE );

		if ( abs( mid_eval_info.score ) == abs( SEARCH_ABORT ) ) {
			move_type = INTERRUPTED_MOVE;
			interrupted_depth = midgame_depth - 1;  /* compensate for increment */
		}
		else
			move_type = MIDGAME_MOVE;
	}

	curr_move = midgame_move;

	/* If the endgame has been reached, solve the position */

	if ( !force_return ) {
		if ( (timed_depth && endgame_reached) ||
			(timed_depth && book_move_found &&
			(disks_played >= 60 - FORCE_BOOK_SOLVE)) ||
			(!timed_depth && (empties <= MAX( exact, wld ))) ) {
			max_depth_reached = empties;
			clear_panic_abort();
			if ( timed_depth )
				curr_move = end_game( my_bits, opp_bits, color,
					(disks_played < 60 - exact), FALSE, book, komi,
					&end_eval_info );
			else {
				if ( empties <= exact )
					curr_move = end_game( my_bits, opp_bits, color, FALSE,
						FALSE, book, komi, &end_eval_info );
				else
					curr_move = end_game( my_bits, opp_bits, color, TRUE,
						FALSE, book, komi, &end_eval_info);
			}
			set_current_eval( end_eval_info );
			if ( abs( root_eval ) == abs( SEARCH_ABORT ) )
				move_type = INTERRUPTED_MOVE;
			else
				move_type = ENDGAME_MOVE;
			if ( update_all )
				endgame_performed[color] = TRUE;
		}
	}

	switch ( move_type ) {
	case INTERRUPTED_MOVE:
		*eval_info = create_eval_info( INTERRUPTED_EVAL, UNSOLVED_POSITION,
			(int)0.0, 0.0, interrupted_depth, FALSE );
		clear_status();
		send_status( "--> *%2d", interrupted_depth );
		eval_str = produce_eval_text( *eval_info, TRUE );
		send_status( "%10s  ", eval_str );
		free( eval_str );
		send_status_nodes( counter_value( &tree_nodes ) );
		send_status_pv( pv[0], interrupted_depth );
		send_status_time( get_elapsed_time() );
		if ( get_elapsed_time() != 0.0 ) {
			send_status( "%6.0f %s",
				counter_value( &tree_nodes ) / (get_elapsed_time() + 0.001),
				"nps" );
		}
		break;
	/*
	case BOOK_MOVE:
		*eval_info = book_eval_info;
		break;
	//*/
	case MIDGAME_MOVE:
		*eval_info = mid_eval_info;
		break;
	case ENDGAME_MOVE:
		*eval_info = end_eval_info;
		break;
	}

	set_current_eval( *eval_info );

	last_time_used = get_elapsed_time();
	if ( update_all ) {
		total_time += last_time_used;
		add_counter( &total_evaluations, &leaf_nodes );
		add_counter( &total_nodes, &tree_nodes );
	}

	clear_panic_abort();

	/* Write the contents of the status buffer to the log file. */

	if ( move_type == BOOK_MOVE ) {
		eval_str = produce_eval_text( *eval_info, FALSE );
		if ( log_file )
			fprintf( log_file, "%s: %c%c  %s\n", "Move chosen",
				TO_SQUARE( curr_move ), eval_str );
		free( eval_str );
	}
	else if ( log_file )
		display_status( log_file, TRUE );

	/* Write the principal variation, if available, to the log file
		and, optionally, to screen. */

	if ( get_ponder_move() == _NULL_MOVE ) {
		complete_pv( my_bits, opp_bits, color );
		if ( display_pv && echo )
			display_optimal_line( stdout );
		if ( log_file )
			display_optimal_line( log_file );
	}

	if ( log_file )
		fclose( log_file );

	return curr_move;
}
