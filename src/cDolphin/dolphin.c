/*
   File:           dolphin.cpp

   Created:        2007-07-10

   Modified:       none

   Author:         GuoXiongHui (wokss@163.com)

   Contents:       The module which controls the operation of standalone Dolphin.
*/

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h>

#include "utils.h"
#include "global.h"
#include "colour.h"
#include "board.h"
#include "bitboard.h"
#include "counter.h"
#include "hash.h"
#include "move.h"
#include "pv.h"
#include "midgame.h"
#include "bitbmob.h"
#include "bitbchk.h"
#include "pattern.h"
#include "coeffs.h"
#include "probcut.h"
#include "myrandom.h"
#include "game.h"
#include "display.h"
#include "timer.h"
#include "eval.h"
#include "error.h"
#include "currency.h"
#include "dolphin.h"
#include "learn.h"
#include "osfbook.h"
#include "pattern_tainer.h"

#ifdef DOLPHIN_GGS
#include "..\Dolphin_GGS\types.h"
using namespace std;
#endif

#ifdef _WIN32
#define strcasecmp _stricmp
#endif

//#define SCR_DOLPHIN

#ifdef SCR_DOLPHIN
#define SCRIPT_ONLY               TRUE
#else
#define SCRIPT_ONLY               FALSE
#endif

#if SCRIPT_ONLY
#undef DEFAULT_USE_BOOK
#define DEFAULT_USE_BOOK          FALSE
#else
#undef DEFAULT_USE_BOOK
#define DEFAULT_USE_BOOK          TRUE
#endif

/* Get rid of some ugly warnings by disallowing usage of the
   macro version of tolower (not time-critical anyway). */
#ifdef tolower
#undef tolower
#endif

/* Local variables */

#if !SCRIPT_ONLY
static double slack = DEFAULT_SLACK;
static double dev_bonus = 0.0;
static int low_thresh = 0;
static int high_thresh = 0;
static int rand_move_freq = 0;
static int tournament = FALSE;
static int tournament_levels;
static int deviation_depth, cutoff_empty;
static int one_position_only = FALSE;
static int use_timer = FALSE;
static int only_analyze = FALSE;
static int thor_max_games;
static int tournament_depth[MAX_TOURNAMENT_SIZE][3];
static int wld_depth[3], exact_depth[3];
#endif

static char *log_file_name;
static double player_time[3], player_increment[3];
static int depth_info[3];
static int wait;
static int use_book = DEFAULT_USE_BOOK;
static int wld_only = DEFAULT_WLD_ONLY;
static int use_learning;
static int use_thor;

int g_rand_terminal;

/* ------------------- Function prototypes ---------------------- */

#ifndef DOLPHIN_GGS

int main( int argc, char *argv[] )
{
	const char *game_file_name = NULL;
	const char *script_in_file;
	const char *script_out_file;
#if !SCRIPT_ONLY
	const char *move_sequence = NULL;
	const char *move_file_name = NULL;
#endif
	int arg_index;
	int help;
	int hash_bits;
	int use_random;
#if !SCRIPT_ONLY
	int repeat = 1;
#endif
	int run_script;
	int script_optimal_line = DEFAULT_DISPLAY_LINE;
	time_t timer;

#if SCRIPT_ONLY
	printf( "\nscrZebra (c) 1997-2004 Gunnar Andersson, compile "
		"date %s at %s\n\n"
		"scrDolphin (c) 2007-2008 modified by XiongHui Guo, BitBoard version.\n\n", __DATE__, __TIME__ );
#else
	printf( "\nZebra (c) 1997-2004 Gunnar Andersson, compile "
		"date %s at %s\n\n"
		"Dolphin (c) 2007-2008 modified by XiongHui Guo, BitBoard version.\n\n", __DATE__, __TIME__ );
#endif

	use_random = DEFAULT_RANDOM;
	wait = DEFAULT_WAIT;
	echo = DEFAULT_ECHO;
	display_pv = DEFAULT_DISPLAY_PV;
	use_learning = FALSE;
	use_thor = DEFAULT_USE_THOR;
	depth_info[CHESS_BLACK] = depth_info[CHESS_WHITE] = -1;
	hash_bits = DEFAULT_HASH_BITS;
	game_file_name = NULL;
	log_file_name = NULL;
	run_script = FALSE;
	script_in_file = script_out_file = FALSE;
	player_time[CHESS_BLACK] = player_time[CHESS_WHITE] = INFINIT_TIME;
	player_increment[CHESS_BLACK] = player_increment[CHESS_WHITE] = 0.0;

	g_stdout = stdout;

	// get arg
	for ( arg_index = 1, help = FALSE; (arg_index < argc) && !help;
		arg_index++ ) {
		if ( !strcasecmp( argv[arg_index], "-e" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			echo = atoi( argv[arg_index] );
		}
		else if ( !strcasecmp( argv[arg_index], "-h" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			hash_bits = atoi( argv[arg_index] );
		}
#if !SCRIPT_ONLY
		else if ( !strcasecmp( argv[arg_index], "-l" ) ) {
			tournament = FALSE;
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			depth_info[CHESS_BLACK] = atoi( argv[arg_index] );
			if ( depth_info[CHESS_BLACK] > 0 ) {
				if ( arg_index + 2 >= argc ) {
					help = TRUE;
					continue;
				}
				arg_index++;
				exact_depth[CHESS_BLACK] = atoi( argv[arg_index] );
				arg_index++;
				wld_depth[CHESS_BLACK] = atoi( argv[arg_index] );
			}
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			depth_info[CHESS_WHITE] = atoi( argv[arg_index] );
			if ( depth_info[CHESS_WHITE] > 0 ) {
				if ( arg_index + 2 >= argc ) {
					help = TRUE;
					continue;
				}
				arg_index++;
				exact_depth[CHESS_WHITE] = atoi( argv[arg_index] );
				arg_index++;
				wld_depth[CHESS_WHITE] = atoi( argv[arg_index] );
			}
		}
		else if ( !strcasecmp( argv[arg_index], "-t" ) ) {
			int i, j;

			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			tournament = TRUE;
			tournament_levels = atoi( argv[arg_index] );
			if ( arg_index + 3 * tournament_levels >= argc ) {
				help = TRUE;
				continue;
			}
			for ( i = 0; i < tournament_levels; i++ )
				for ( j = 0; j < 3; j++ ) {
					arg_index++;
					tournament_depth[i][j] = atoi( argv[arg_index] );
				}
		}
		else if ( !strcasecmp( argv[arg_index], "-w" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			wait = atoi( argv[arg_index] );
		}
		else if ( !strcasecmp( argv[arg_index], "-p" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			display_pv = atoi( argv[arg_index] );
		}
		else if ( !strcasecmp( argv[arg_index], "?" ) )
			help = 1;
		else if ( !strcasecmp( argv[arg_index], "-g" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			game_file_name = argv[arg_index];
		}
		else if ( !strcasecmp( argv[arg_index], "-r" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			use_random = atoi( argv[arg_index] );
		}
#endif
		else if ( !strcasecmp( argv[arg_index], "-b" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			use_book = atoi( argv[arg_index] );
		}
#if !SCRIPT_ONLY
		else if ( !strcasecmp( argv[arg_index], "-time" ) ) {
			if ( arg_index + 4 >= argc ) {
				help = TRUE;
				continue;
			}
			arg_index++;
			player_time[CHESS_BLACK] = atoi( argv[arg_index] );
			arg_index++;
			player_increment[CHESS_BLACK] = atoi( argv[arg_index] );
			arg_index++;
			player_time[CHESS_WHITE] = atoi( argv[arg_index] );
			arg_index++;
			player_increment[CHESS_WHITE] = atoi( argv[arg_index] );
			use_timer = TRUE;
		}
		else if ( !strcasecmp(argv[arg_index], "-learn" ) ) {
			if ( arg_index + 2 >= argc ) {
				help = TRUE;
				continue;
			}
			arg_index++;
			deviation_depth = atoi( argv[arg_index] );
			arg_index++;
			cutoff_empty = atoi( argv[arg_index] );
			use_learning = TRUE;
		}
		else if ( !strcasecmp( argv[arg_index], "-slack" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			slack = atof( argv[arg_index] );
		}
		else if ( !strcasecmp( argv[arg_index], "-dev" ) ) {
			if ( arg_index + 3 >= argc ) {
				help = TRUE;
				continue;
			}
			arg_index++;
			low_thresh = atoi( argv[arg_index] );
			arg_index++;
			high_thresh = atoi( argv[arg_index] );
			arg_index++;
			dev_bonus = atof( argv[arg_index] );
		}
		else if ( !strcasecmp( argv[arg_index], "-log" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			log_file_name = argv[arg_index];
		}
		/*
		else if ( !strcasecmp( argv[arg_index], "-private" ) )
		set_game_mode( PRIVATE_GAME );
		else if ( !strcasecmp( argv[arg_index], "-public" ) )
		set_game_mode( PUBLIC_GAME );
		else if ( !strcasecmp( argv[arg_index], "-keepdraw" ) )
		set_draw_mode( NEUTRAL );
		else if ( !strcasecmp( argv[arg_index], "-draw2black" ) )
		set_draw_mode( BLACK_WINS );
		else if ( !strcasecmp( argv[arg_index], "-draw2white" ) )
		set_draw_mode( WHITE_WINS );
		else if ( !strcasecmp( argv[arg_index], "-draw2none" ) )
		set_draw_mode( OPPONENT_WINS );
		//*/
		else if ( !strcasecmp( argv[arg_index], "-test" ) )
			one_position_only = TRUE;
		else if ( !strcasecmp( argv[arg_index], "-seq" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			move_sequence = argv[arg_index];
		}
		else if ( !strcasecmp( argv[arg_index], "-seqfile" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			move_file_name = argv[arg_index];
		}
		else if ( !strcasecmp( argv[arg_index], "-repeat" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			repeat = atoi( argv[arg_index] );
		}
		else if ( !strcasecmp( argv[arg_index], "-thor" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			use_thor = TRUE;
			thor_max_games = atoi( argv[arg_index] );
		}
		else if ( !strcasecmp( argv[arg_index], "-analyze" ) )
			only_analyze = TRUE;
		else if ( !strcasecmp( argv[arg_index], "-randmove" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			rand_move_freq = atoi( argv[arg_index] );
			if ( rand_move_freq < 0 ) {
				help = TRUE;
				continue;
			}
		}
#endif
		else if ( !strcasecmp( argv[arg_index], "-wld" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			wld_only = atoi( argv[arg_index] );
		}
		else if ( !strcasecmp( argv[arg_index], "-line" ) ) {
			if ( ++arg_index == argc ) {
				help = TRUE;
				continue;
			}
			script_optimal_line = atoi( argv[arg_index] );
		}
		else if ( !strcasecmp( argv[arg_index], "-script" ) ) {
			if ( arg_index + 2 >= argc ) {
				help = TRUE;
				continue;
			}
			arg_index++;
			script_in_file = argv[arg_index];
			arg_index++;
			script_out_file = argv[arg_index];
			run_script = TRUE;
		}
		else
			help = TRUE;
		if ( arg_index >= argc )
			help = TRUE;
	}

#if SCRIPT_ONLY
	if ( !run_script )
		help = TRUE;
#endif

#if SCRIPT_ONLY
  #ifdef _DEBUG
	if ( 1 || !help ) {
  #else
	if ( help || (argc <= 1) ) {
  #endif
		generate_random_endgame( "test.pos", 40, 0, 1, 5000, 23 );
		goto GAME_END;
	}
#endif

	if ( !help && (argc <= 1) ) {
		special_test( game_file_name );
		goto GAME_END;
	}

	if ( help ) {
#if SCRIPT_ONLY
		puts( "Usage:" );
		puts( "  scrDolphin [-e ...] [-h ...] [-wld ...] [-line ...] [-b ...] "
			"-script ..." );
		puts( "" );
		puts( "  -e <echo?>" );
		printf( "    Toggles screen output on/off (default %d).\n\n",
			DEFAULT_ECHO );
		puts( "  -h <bits in hash key>" );
		printf( "    Size of hash table is 2^{this value} (default %d).\n\n",
			DEFAULT_HASH_BITS );
		puts( "  -script <script file> <output file>" );
		puts( "    Solves all positions in script file for exact score.\n" );
		puts( "  -wld <only solve WLD?>" );
		printf( "    Toggles WLD only solve on/off (default %d).\n\n",
			DEFAULT_WLD_ONLY );
		puts( "  -line <output line?>" );
		printf( "    Toggles output of optimal line on/off (default %d).\n\n",
			DEFAULT_DISPLAY_LINE );
		puts( "  -b <use book?>" );
		printf( "    Toggles usage of opening book on/off (default %d).\n",
			DEFAULT_USE_BOOK );
		puts( "" );
#else
		puts( "Usage:" );
		puts( "  Dolphin [-b -e -g -h -l -p -t -time -w -learn -slack -dev -log" );
		puts( "         -keepdraw -draw2black -draw2white -draw2none" );
		puts( "         -private -public -test -seq -thor -script -analyze ?" );
		puts( "         -repeat -seqfile]" );
		puts( "" );
		puts( "Flags:" );
		puts( "  ? " );
		puts( "    Displays this text." );
		puts( "" );
		puts( "  -b <use book?>" );
		printf( "    Toggles usage of opening book on/off (default %d).\n",
			DEFAULT_USE_BOOK );
		puts( "" );
		puts( "  -e <echo?>" );
		printf( "    Toggles screen output on/off (default %d).\n",
			DEFAULT_ECHO );
		puts( "" );
		puts( "  -g <game file>" );
		puts( "" );
		puts( "  -h <bits in hash key>" );
		printf( "    Size of hash table is 2^{this value} (default %d).\n",
			DEFAULT_HASH_BITS );
		puts( "" );
		puts( "  -l <black depth> [<black exact depth> <black WLD depth>]" );
		puts( "     <white depth> [<white exact depth> <white WLD depth>]" );
		printf( "    Sets the search depth. If <black depth> or <white depth> " );
		printf( "are set to 0, a\n" );
		printf( "    human player is assumed. In this case the other " );
		printf( "parameters must be omitted.\n" );
		printf( "    <* exact depth> specify the number of moves before the " );
		printf( "(at move 60) when\n" );
		printf( "    the exact game-theoretical value is calculated. <* WLD " );
		printf( "depth> are used\n" );
		puts( "    analogously for the calculation of Win/Loss/Draw." );
		puts( "" );
		puts( "  -p <display principal variation?>" );
		printf( "    Toggles output of principal variation on/off (default %d).\n",
			DEFAULT_DISPLAY_PV );
		puts( "" );
		puts( "  -r <use randomization?>" );
		printf( "    Toggles randomization on/off (default %d)\n",
			DEFAULT_RANDOM );
		puts( "" );
		puts( "  -t <number of levels> <(first) depth> ... <(last) wld depth>" );
		puts( "" );
		puts( "  -time <black time> <black increment> "
			"<white time> <white increment>" );
		puts( "    Tournament mode; the format for the players is as above." );
		puts( "" );
		puts( "  -w <wait?>" );
		printf( "    Toggles wait between moves on/off (default %d).\n",
			DEFAULT_WAIT );
		puts( "" );
		puts( "  -learn <depth> <cutoff>" );
		puts( "    Learn the game with <depth> deviations up to <cutoff> empty." );
		puts( "" );
		puts( "  -slack <disks>" );
		printf( "    Dolphin's opening randomness is <disks> disks (default %f).\n",
			DEFAULT_SLACK );
		puts( "" );
		puts( "  -dev <low> <high> <bonus>" );
		puts( "    Give deviations before move <high> a <bonus> disk bonus but" );
		puts( "    don't give any extra bonus for deviations before move <low>." );
		puts( "" );
		puts( "  -log <file name>" );
		puts( "    Append all game results to the specified file." );
		puts( "" );
		puts( "  -private" );
		puts( "    Treats all draws as losses for both sides." );
		puts( "" );
		puts( "  -public" );
		puts( "    No tweaking of draw scores." );
		puts( "" );
		puts( "  -keepdraw" );
		puts( "    Book draws are counted as draws." );
		puts( "" );
		puts( "  -draw2black" );
		puts( "    Book draws scored as 32-31 for black." );
		puts( "" );
		puts( "  -draw2white" );
		puts( "    Book draws scored as 32-31 for white." );
		puts( "" );
		puts( "  -draw2none" );
		puts( "    Book draws scored as 32-31 for the opponent." );
		puts( "" );
		puts( "  -test" );
		puts( "    Only evaluate one position, then exit." );
		puts( "" );
		puts( "  -seq <move sequence>" );
		puts( "    Forces the game to start with a predefined move sequence;" );
		puts( "    e.g. f4d6c3." );
		puts( "" );
		puts( "  -seqfile <filename" );
		puts( "    Specifies a file from which move sequences are read." );
		puts( "" );
		puts( "  -thor <game count>" );
		puts( "    Look for each position in the Thor database; "
			"list the first <game count>." );
		puts( "" );
		puts( "  -script <script file> <output file>" );
		puts( "    Solves all positions in script file for exact score." );
		puts( "" );
		puts( "  -wld <only solve WLD?>" );
		printf( "    Toggles WLD only solve on/off (default %d).\n\n",
			DEFAULT_WLD_ONLY );
		puts( "  -analyze" );
		puts( "    Used in conjunction with -seq; all positions are analyzed." );
		puts( "  -repeat <#iterations>" );
		puts( "    Repeats the operation the specified number of times. " );
#endif
		puts( "" );
		exit( EXIT_FAILURE );
	}

	if ( hash_bits < 1 ) {
		printf( "Hash table key must contain at least 1 bit\n" );
		exit( EXIT_FAILURE );
	}

	global_setup( use_random, hash_bits );
	//init_thor_database();

	if ( use_book ) {
		init_learn( "book.bin", TRUE );
		printf( "use book.\n" );
	}

	if ( use_random && !SCRIPT_ONLY ) {
		time( &timer );
		my_srandom( (int)timer );
	}
	else
		my_srandom( 1 );

	do_some_tests();

#if !SCRIPT_ONLY
	if ( !tournament && !run_script ) {
		while ( depth_info[CHESS_BLACK] < 0 ) {
			printf( "Black parameters: " );
			scanf( "%d", &depth_info[CHESS_BLACK] );
			if ( depth_info[CHESS_BLACK] > 0 )
				scanf( "%d %d", &exact_depth[CHESS_BLACK], &wld_depth[CHESS_BLACK] );
		}
		while ( depth_info[CHESS_WHITE] < 0 ) {
			printf( "White parameters: " );
			scanf( "%d", &depth_info[CHESS_WHITE] );
			if ( depth_info[CHESS_WHITE] > 0 )
				scanf( "%d %d", &exact_depth[CHESS_WHITE], &wld_depth[CHESS_WHITE] );
		}
	}

#ifndef _DEBUG
	//SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
#endif

	if ( one_position_only )
		toggle_smart_buffer_management( FALSE );
#endif

	if ( run_script ) {
		run_endgame_script( script_in_file, script_out_file,
			script_optimal_line );
	}

#if !SCRIPT_ONLY
	else {
		/*
		if ( tournament )
			play_tournament( move_sequence );
		else {
			if ( only_analyze )
				analyze_game( move_sequence );
			else
				play_game( game_file_name, move_sequence, move_file_name, repeat );
		}
		//*/
		play_game( game_file_name, move_sequence, move_file_name, repeat );
	}
#endif

	global_terminate();

#ifndef _DEBUG
	//SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
#endif

GAME_END:
	system( "pause" );
	return EXIT_SUCCESS;
}

#endif

static void
special_test( const char *file_name ) {
	int i;
	DWORD dwUsedTime;
	int depth, color;
	int score, move;
	BitBoard b, w;
	char leaf_buffer[32], tree_buffer[32];
	char total_buffer[32];
	EvaluationType eval_info;
	CounterType nodes;
	double fnodes;
	double nps;

	depth = 24;
	color = CHESS_BLACK;

	score = 0;
	move = _NULL_MOVE;

	disks_played = 0;

	global_setup( TRUE, 22 );
	hash_setup( TRUE, TRUE );

	game_init( file_name, &color );

	BITBOARD_CLEAR( b );
	BITBOARD_CLEAR( w );

	if ( file_name == NULL ) {
		bitboard_set_bit( &b, E4);
		bitboard_or_bit ( &b, D5);
		bitboard_set_bit( &w, D4);
		bitboard_or_bit ( &w, E5);

		for ( i = 0; i < 64; i++ )
			board[i] = CHESS_EMPTY;
		board[E4] = CHESS_BLACK;
		board[D5] = CHESS_BLACK;
		board[D4] = CHESS_WHITE;
		board[E5] = CHESS_WHITE;

		piece_count[CHESS_BLACK][0] = 2;
		piece_count[CHESS_WHITE][0] = 2;
	}
	else {
		set_bitboards( board, color, &b, &w );
	}

	//hash_set_transformation( my_random(), my_random() );
	hash_set_transformation( 0, 0 );
	clear_ponder_move();

	for ( i = 0; i < 60; i++ ) {
		black_moves[i] = PASS_MOVE;
		white_moves[i] = PASS_MOVE;
	}

	/*
	black_hash1 = my_random();
	black_hash2 = my_random();
	white_hash1 = my_random();
	white_hash2 = my_random();
	//*/

	remove_coeffs( disks_played );

	generate_all( b, w, color );

	if ( color == CHESS_BLACK )
		score_sheet_row++;

	clear_panic_abort();

	//set_move_list( black_moves, white_moves, score_sheet_row );
	set_times( 1200, 3 );

	//if ( echo )
	//	dumpch();

	start_move( 1200, 3, disks_played + 4 );
	determine_move_time( 1200, 3, disks_played + 4 );

	toggle_experimental( FALSE );

	toggle_smart_buffer_management( FALSE );

	//display_board(stdout, board, color, TRUE, FALSE, TRUE);

	eval_info = create_eval_info( PASS_EVAL, UNSOLVED_POSITION,
				   (int)0.0, 0.0, 0, FALSE );

	display_board( stdout, board, color, TRUE, FALSE, TRUE );

	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

	dwUsedTime = GetTickCount();
	//inherit_move_lists( disks_played + depth );
	//score = root_tree_search(b, w, 0, depth, -32767, 32767, CHESS_BLACK, TRUE, TRUE, FALSE);
	//score = tree_search(b, w, 0, depth, -32767, 32767, CHESS_BLACK, TRUE, TRUE, FALSE);
	//move = middle_game(b, w, depth, CHESS_BLACK, TRUE, &eval_info);
	move = compute_move(b, w, color, TRUE, 1200, 3, FALSE, FALSE, depth, 22, 24, FALSE, &eval_info);
	dwUsedTime = GetTickCount() - dwUsedTime;

	score = eval_info.score;

	SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

	//hash_free();
	//clear_coeffs();

	global_terminate();

	//sum_counter( &total_nodes, &leaf_nodes, &tree_nodes );
	sub_counter( &nodes, &tree_nodes, &leaf_nodes );
	//adjust_counter( &tree_nodes );
	//unsigned __int64 u64_total_nodes;
	//u64_total_nodes = ((unsigned __int64)tree_nodes.high * DECIMAL_BASIS) + tree_nodes.low;
	//u64_total_nodes = counter_int64( &tree_nodes );

	Int64ToCurrency( counter_int64( &leaf_nodes ), leaf_buffer );
	Int64ToCurrency( counter_int64( &nodes ), tree_buffer );
	//Int64ToCurrency( u64_total_nodes, total_buffer );
	Int64ToCurrency( counter_int64( &tree_nodes ), total_buffer );

	fnodes = counter_value( &tree_nodes );
	nps = (fnodes / 1000.0f) / ((double) dwUsedTime / 1000.0f + 0.001f);

	printf("\r\n");

	printf("\r\n");
	//printf("leaf nodes: %d, tree nodes: %d, total nodes: %I64u.", leaf_nodes.low, nodes.low, u64_total_nodes);
	printf("leaf nodes: %s, tree nodes: %s, total nodes: %s.", leaf_buffer, tree_buffer, total_buffer);
	/*
	printf("\r\nleaf nodes: %s, tree nodes: %s, total nodes: %I64u.",
		Int32ToCurrency2( leaf_nodes.low ),
		Int32ToCurrency2( nodes.low ), u64_total_nodes);
	//*/
	/*
	printf("\r\nleaf nodes: %s, tree nodes: %s, total nodes: %s.",
		DecimalToCurrency2( (double)leaf_nodes.low ),
		DecimalToCurrency2( (double)nodes.low ),
		DecimalToCurrency2( (double)u64_total_nodes ) );
	//*/
	printf("\r\n");

	printf("\r\n");
	printf("depth: %d, move: %c%c, score: %0.2f.\r\n\r\nused time: %d ms, nps: %0.3f kn/s.", depth, TO_SQUARE( move ),
		(double) score / 128.0f, dwUsedTime, nps);
	printf("\r\n");

	printf("\r\n");
}

#if !SCRIPT_ONLY
/*
   PLAY_GAME
   Administrates the game between two players, humans or computers.
*/

static void
play_game( const char *file_name,
		  const char *move_string,
		  const char *move_file_name,
		  int repeat ) {
	EvaluationType eval_info;
	const char *black_name;
	const char *white_name;
	//const char *opening_name;
	double node_val, leaf_val, tree_val;
	CounterType tmp_nodes;
	double move_start, move_stop;
	//double database_start, database_stop;
	double total_search_time = 0.0;
	int i;
	int color;
	int curr_move;
	int timed_search;
	int rand_color = CHESS_BLACK;
	int black_hash1, black_hash2, white_hash1, white_hash2;
	int provided_move_count;
	int col, row;
	//int thor_position_count;
	int provided_move[61];
	char move_vec[121];
	char line_buffer[1000];
	BitBoard my_bits, opp_bits;
	BitBoard tmp_bits;
	time_t timer;
	FILE *log_file;
	FILE *move_file;
    char leaf_buffer[32]; char tree_buffer[32], total_buffer[32];

	if ( move_file_name != NULL )
		move_file = fopen( move_file_name, "r" );
	else
		move_file = NULL;

START:

	/* Decode the predefined move sequence */

	if ( move_file != NULL ) {
		char *newline_pos;

		fgets( line_buffer, sizeof line_buffer, move_file );
		newline_pos = strchr( line_buffer, '\n' );
		if ( newline_pos != NULL )
			*newline_pos = 0;
		move_string = line_buffer;
	}

	if ( move_string == NULL )
		provided_move_count = 0;
	else {
		provided_move_count = strlen(move_string) / 2;
		if ( (provided_move_count > 60) || (strlen( move_string ) % 2 == 1) )
			fatal_error( "Invalid move string provided" );
		for ( i = 0; i < provided_move_count; i++ ) {
			col = tolower( move_string[2 * i] ) - 'a' + 1;
			row = move_string[2 * i + 1] - '0';
			if ( (col < 1) || (col > 8) || (row < 1) || (row > 8) )
				fatal_error( "Unexpected character in move string" );
			provided_move[i] = 8 * (row - 1) + (col - 1);
		}
	}

	/* Set up the position and the search engine */

	game_init( file_name, &color );
	hash_setup( TRUE, TRUE );
	//clear_stored_game();

	//hash_set_transformation( 0, 0 );
	clear_ponder_move();

	set_bitboards( board, color, &my_bits, &opp_bits );

	slack = DEFAULT_SLACK;

	///*
	if ( echo && use_book )
		printf( "Book randomness: %.2f disks\n", slack );
	set_slack( (int)floor( slack * 128.0 ) );
	toggle_human_openings( FALSE );

	if ( use_learning )
		set_learning_parameters( deviation_depth, cutoff_empty );
	reset_book_search();
	set_deviation_value( low_thresh, high_thresh, dev_bonus );
	//*/

	/*
	if ( use_thor ) {

#if 1
		/-* No error checking done as it's only for testing purposes *-/

		database_start = get_real_timer();
		(void) read_player_database( "thor\\wthor.jou");
		(void) read_tournament_database( "thor\\wthor.trn" );
		(void) read_game_database( "thor\\wth_2001.wtb" );
		(void) read_game_database( "thor\\wth_2000.wtb" );
		(void) read_game_database( "thor\\wth_1999.wtb" );
		(void) read_game_database( "thor\\wth_1998.wtb" );
		(void) read_game_database( "thor\\wth_1997.wtb" );
		(void) read_game_database( "thor\\wth_1996.wtb" );
		(void) read_game_database( "thor\\wth_1995.wtb" );
		(void) read_game_database( "thor\\wth_1994.wtb" );
		(void) read_game_database( "thor\\wth_1993.wtb" );
		(void) read_game_database( "thor\\wth_1992.wtb" );
		(void) read_game_database( "thor\\wth_1991.wtb" );
		(void) read_game_database( "thor\\wth_1990.wtb" );
		(void) read_game_database( "thor\\wth_1989.wtb" );
		(void) read_game_database( "thor\\wth_1988.wtb" );
		(void) read_game_database( "thor\\wth_1987.wtb" );
		(void) read_game_database( "thor\\wth_1986.wtb" );
		(void) read_game_database( "thor\\wth_1985.wtb" );
		(void) read_game_database( "thor\\wth_1984.wtb" );
		(void) read_game_database( "thor\\wth_1983.wtb" );
		(void) read_game_database( "thor\\wth_1982.wtb" );
		(void) read_game_database( "thor\\wth_1981.wtb" );
		(void) read_game_database( "thor\\wth_1980.wtb" );
		database_stop = get_real_timer();

#if FULL_ANALYSIS
		frequency_analysis( get_total_game_count() );
#endif
		printf( "Loaded %d games in %.3f s.\n", get_total_game_count(),
			database_stop - database_start );
		printf( "Each Thor game occupies %d bytes.\n", get_thor_game_size() );
#else
		{
			/-* This code only used for the generation of screen saver analyses *-/

			GameInfoType thor_game;
			char db_name[100], output_name[100];
			int j;
			int year = 1999;
			int move_count;
			int moves[100];
			FILE *stream;

			database_start = get_real_timer();
			(void) read_player_database( "thor\\wthor.jou");
			(void) read_tournament_database( "thor\\wthor.trn" );
			sprintf( db_name, "thor\\wth_%d.wtb", year );
			(void) read_game_database( db_name );
			database_stop = get_real_timer();
			printf( "Loaded %d games in %.3f s.\n", get_total_game_count(),
				database_stop - database_start );

			database_search( board, color );
			thor_position_count = get_match_count();
			printf( "%d games match the initial position\n", thor_position_count );

			sprintf( output_name, "wc%d.dbs", year );
			stream = fopen( output_name, "w" );
			if ( stream != NULL ) {
				for ( i = 0; i < thor_position_count; i++ ) {
					thor_game = get_thor_game( i );
					if ( strcmp( thor_game.tournament, "Championnat du Monde" ) == 0 ) {
						fprintf( stream, "%d\n", year );
						fprintf( stream, "%s\n", thor_game.black_name );
						fprintf( stream, "%s\n", thor_game.white_name );
						get_thor_game_moves( i, &move_count, moves );
						for ( j = 0; j < move_count; j++ )
							fprintf( stream, "%c%c", TO_SQUARE( moves[j] ) );
						fputs( "\n", stream );
					}
				}
				fclose( stream );
			}
		}
#endif
	}
	//*/

	if ( depth_info[CHESS_BLACK] == 0 )
		black_name = "Player";
	else
		black_name = "Dolphin";
	if ( depth_info[CHESS_WHITE] == 0 )
		white_name = "Player";
	else
		white_name = "Dolphin";
	set_names( black_name, white_name );
	set_move_list( black_moves, white_moves, score_sheet_row );
	set_evals( 0.0, 0.0 );

	for ( i = 0; i < 60; i++ ) {
		black_moves[i] = PASS_MOVE;
		white_moves[i] = PASS_MOVE;
	}

	move_vec[0] = 0;

	black_hash1 = my_random();
	black_hash2 = my_random();
	white_hash1 = my_random();
	white_hash2 = my_random();

	while ( game_in_progress( my_bits, opp_bits, color ) ) {
		remove_coeffs( disks_played );
		if ( SEPARATE_TABLES ) {  /* Computer players won't share hash tables */
			if ( color == CHESS_BLACK ) {
				g_hash1 ^= black_hash1;
				g_hash2 ^= black_hash2;
			}
			else {
				g_hash1 ^= white_hash1;
				g_hash2 ^= white_hash2;
			}
		}
		generate_all( my_bits, opp_bits, color );

		if ( color == CHESS_BLACK )
			score_sheet_row++;

		if ( move_count[disks_played] != 0 ) {
			move_start = get_real_timer();
			clear_panic_abort();

			if ( echo ) {
				set_move_list( black_moves, white_moves, score_sheet_row );
				set_times( (int)floor( player_time[CHESS_BLACK] ),
					(int)floor( player_time[CHESS_WHITE] ) );
				/*
				opening_name = find_opening_name();
				if ( opening_name != NULL )
					printf( "\nOpening: %s\n", opening_name );
				if ( use_thor ) {
					database_start = get_real_timer();
					database_search( board, color );
					thor_position_count = get_match_count();
					database_stop = get_real_timer();
					total_search_time += database_stop - database_start;
					printf( "%d matching games  (%.3f s search time, %.3f s total)\n",
						thor_position_count, database_stop - database_start,
						total_search_time );
					if ( thor_position_count > 0 ) {
						printf( "%d black wins, %d draws, %d white wins\n",
							get_black_win_count(), get_draw_count(),
							get_white_win_count() );
						printf( "Median score %d-%d", get_black_median_score(),
							64 - get_black_median_score() );
						printf( ", average score %.2f-%.2f\n", get_black_average_score(),
							64.0 - get_black_average_score() );
					}
					print_thor_matches( stdout, thor_max_games );
				}
				//*/
				display_board( stdout, board, color, TRUE, use_timer, TRUE );
			}
			//dump_position( color );
			//dump_game_score( color );

			/* Check what the Thor opening statistics has to say */

			//(void) choose_thor_opening_move( board, color, echo );

			//if ( echo && wait )
			//	dumpch();
			if ( disks_played >= provided_move_count ) {
				if ( depth_info[color] == 0 ) {
					///*
					if ( use_book && display_pv ) {
						fill_move_alternatives( my_bits, opp_bits, color, 0 );
						if ( echo )
							print_move_alternatives( color );
					}
					//*/
					puts( "" );
					curr_move = get_move( color );
				}
				else {
					start_move( player_time[color],
						player_increment[color],
						disks_played + 4 );
					determine_move_time( player_time[color],
						player_increment[color],
						disks_played + 4 );
					timed_search = (depth_info[color] >= 60);
					toggle_experimental( FALSE );

					curr_move =
						compute_move( my_bits, opp_bits, color, TRUE, (int)player_time[color],
							(int)player_increment[color], timed_search,
							use_book, depth_info[color],
							exact_depth[color], wld_depth[color],
							FALSE, &eval_info );
					if ( color == CHESS_BLACK )
						set_evals( produce_compact_eval( eval_info ), 0.0 );
					else
						set_evals( 0.0, produce_compact_eval( eval_info ) );
					if ( eval_info.is_book && (rand_move_freq > 0) &&
						(color == rand_color) &&
						((my_random() % rand_move_freq) == 0) ) {
						puts( "Engine override: Random move selected." );
						rand_color = OPP_COLOR( rand_color );
						curr_move =
							move_list[disks_played][my_random() % move_count[disks_played]];
					}
				}
			}
			else {
				curr_move = provided_move[disks_played];
				if ( !move_is_valid( my_bits, opp_bits, curr_move ) )
					fatal_error( "Invalid move %c%c in move sequence",
						TO_SQUARE( curr_move ) );
			}

			move_stop = get_real_timer();
			if ( player_time[color] != INFINIT_TIME )
				player_time[color] -= (move_stop - move_start);

			//store_move( disks_played, curr_move );

			sprintf( move_vec + 2 * disks_played, "%c%c", TO_SQUARE( curr_move ) );
			(void) make_move( &my_bits, &opp_bits, color, curr_move, TRUE );
			if ( color == CHESS_BLACK )
				black_moves[score_sheet_row] = curr_move;
			else {
				if ( white_moves[score_sheet_row] != PASS_MOVE )
					score_sheet_row++;
				white_moves[score_sheet_row] = curr_move;
			}
		}
		else {
			if ( color == CHESS_BLACK )
				black_moves[score_sheet_row] = PASS_MOVE;
			else
				white_moves[score_sheet_row] = PASS_MOVE;
			if ( depth_info[color] == 0 ) {
				puts( "You must pass - please press Enter" );
				dumpch();
			}
		}

		color = OPP_COLOR( color );
		tmp_bits = my_bits;
		my_bits = opp_bits;
		opp_bits = tmp_bits;

		if ( one_position_only )
			break;
	}

	if ( !echo && !one_position_only ) {
		printf( "\n");
		printf( "Black level: %d\n", depth_info[CHESS_BLACK] );
		printf( "White level: %d\n", depth_info[CHESS_WHITE] );
	}

	if ( color == CHESS_BLACK )
		score_sheet_row++;
	//dump_game_score( color );

	if ( echo && !one_position_only ) {
		set_move_list( black_moves, white_moves, score_sheet_row );
		/*
		if ( use_thor ) {
			database_start = get_real_timer();
			database_search( board, color );
			thor_position_count = get_match_count();
			database_stop = get_real_timer();
			total_search_time += database_stop - database_start;
			printf( "%d matching games  (%.3f s search time, %.3f s total)\n",
				thor_position_count, database_stop - database_start,
				total_search_time );
			if ( thor_position_count > 0 ) {
				printf( "%d black wins,  %d draws,  %d white wins\n",
					get_black_win_count(), get_draw_count(),
					get_white_win_count() );
				printf( "Median score %d-%d\n", get_black_median_score(),
					64 - get_black_median_score() );
				printf( ", average score %.2f-%.2f\n", get_black_average_score(),
					64.0 - get_black_average_score() );
			}
			print_thor_matches( stdout, thor_max_games );
		}
		//*/
		set_times( (int)floor( player_time[CHESS_BLACK] ), (int)floor( player_time[CHESS_WHITE] ) );
		display_board( stdout, board, color, TRUE, use_timer, TRUE );
	}

	leaf_val = counter_value( &leaf_nodes );
	node_val = counter_value( &tree_nodes );
	sub_counter( &tmp_nodes, &tree_nodes, &leaf_nodes );
	tree_val = counter_value( &tmp_nodes );
	DecimalToCurrency( leaf_val, leaf_buffer );
	DecimalToCurrency( tree_val, tree_buffer );
	DecimalToCurrency( node_val, total_buffer );

	printf( "\nBlack: %d   White: %d\n", disc_count( my_bits, opp_bits, (color == CHESS_BLACK) ),
		disc_count( my_bits, opp_bits, (color == CHESS_WHITE) ) );
	printf( "\n" );
	printf( "Leaf Nodes: %s, Tree Nodes: %s, Total Nodes: %s.\n",
		leaf_buffer, tree_buffer, total_buffer );
	printf( "\n" );
	printf( "Move: %c%c, Score: %0.2f, Total time: %.3f s.\n", TO_SQUARE( curr_move ),
		eval_info.score / 128.0f, total_time );
	printf( "\n" );

	if ( (log_file_name != NULL) && !one_position_only )  {
		log_file = fopen( log_file_name, "a" );
		if ( log_file != NULL ) {
			timer = time( NULL );
			fprintf( log_file, "# %s#     %2d - %2d\n", ctime( &timer ),
				disc_count( my_bits, opp_bits, (color == CHESS_BLACK) ),
				disc_count( my_bits, opp_bits, (color == CHESS_WHITE) ) );
			fprintf( log_file, "%s\n", move_vec );
			fclose( log_file );
		}
	}

	repeat--;

	toggle_abort_check( FALSE );
	/*
	if ( use_learning && !one_position_only )
		learn_game( disks_played, (depth[CHESS_BLACK] != 0) && (depth[CHESS_WHITE] != 0),
			repeat == 0 );
	//*/
	toggle_abort_check( TRUE );

	if ( repeat > 0 )
		goto START;

	if ( move_file != NULL )
		fclose( move_file );
}

#ifdef DOLPHIN_GGS

/*
   INIT_GAME_GGS
   Init the game on the GGS server.
*/

void
global_setup_ggs( int use_random,
				 int hash_bits ) {
	int color;
	time_t timer;

	use_random = DEFAULT_RANDOM;
	wait = DEFAULT_WAIT;
	echo = DEFAULT_ECHO;
	display_pv = DEFAULT_DISPLAY_PV;
	use_learning = FALSE;
	use_thor = DEFAULT_USE_THOR;
	depth_info[CHESS_BLACK] = depth_info[CHESS_WHITE] = -1;
	log_file_name = NULL;
	player_time[CHESS_BLACK] = player_time[CHESS_WHITE] = INFINIT_TIME;
	player_increment[CHESS_BLACK] = player_increment[CHESS_WHITE] = 0.0;
	use_timer = TRUE;
	use_book = FALSE;

	g_stdout = stdout;
	if ( hash_bits <= 0 )
		hash_bits = DEFAULT_HASH_BITS;

	global_setup( use_random, hash_bits );
	//init_thor_database();

	if ( use_book )
		init_learn( "book.bin", TRUE );

	if ( use_random && !SCRIPT_ONLY ) {
		time( &timer );
		my_srandom( (int)timer );
	}
	else
		my_srandom( 1 );

	game_init( NULL, &color );
	hash_setup( TRUE, TRUE );
}

/*
   PLAY_GAME_GGS
   Administrates the game on the GGS server.
*/

int
play_game_ggs( int color,
			  int my_time,
			  int my_incr,
			  int opp_time,
			  int opp_incr,
			  int book,
			  int mid,
			  int exact,
			  int wld,
			  const char *gameid,
			  EvaluationType &eval_info ) {
	const char *black_name;
	const char *white_name;
	//const char *opening_name;
	double node_val, leaf_val, tree_val;
	CounterType tmp_nodes;
	double move_start, move_stop;
	//double database_start, database_stop;
	double total_search_time = 0.0;
	int i;
	int curr_move;
	int timed_search;
	int rand_color = CHESS_BLACK;
	int black_hash1, black_hash2, white_hash1, white_hash2;
	int provided_move_count;
	int b_discs, w_discs, e_discs;
	//int thor_position_count;
	int provided_move[61];
	char move_vec[121];
	BitBoard my_bits, opp_bits;
	BitBoard tmp_bits;
	time_t timer;
	FILE *log_file;
	char info_buffer[128];
    char leaf_buffer[32];
	char tree_buffer[32];
	char total_buffer[32];
	char nps_buffer[32];
	const char *color_str;

	tournament = TRUE;
	tournament_levels = 0;
	tournament_depth[0][0] = 0;

	depth_info[CHESS_BLACK] = mid;
	exact_depth[CHESS_BLACK] = exact;
	wld_depth[CHESS_BLACK] = wld;

	depth_info[CHESS_WHITE] = mid;
	exact_depth[CHESS_WHITE] = exact;
	wld_depth[CHESS_WHITE] = wld;

	use_book = book;
	use_timer = TRUE;
	if ( color == CHESS_BLACK ) {
		player_time[CHESS_BLACK] = my_time;
		player_increment[CHESS_BLACK] = my_incr;

		player_time[CHESS_WHITE] = opp_time;
		player_increment[CHESS_WHITE] = opp_incr;
	}
	else {
		player_time[CHESS_WHITE] = my_time;
		player_increment[CHESS_WHITE] = my_incr;

		player_time[CHESS_BLACK] = opp_time;
		player_increment[CHESS_BLACK] = opp_incr;
	}

	provided_move_count = 0;
	one_position_only = true;

	/* Set up the position and the search engine */

	game_init_ggs( color );
	//hash_setup_ggs( TRUE );
	//clear_stored_game();

	//hash_set_transformation( 0, 0 );
	clear_ponder_move();

	set_bitboards( board, color, &my_bits, &opp_bits );

	eval_info = create_eval_info( PASS_EVAL, UNSOLVED_POSITION,
		0, 0.0, 0, FALSE );

	///*
	if ( echo && use_book )
		printf( "Book randomness: %.2f disks\n", slack );
	set_slack( (int)floor( slack * 128.0 ) );
	toggle_human_openings( FALSE );

	if ( use_learning )
		set_learning_parameters( deviation_depth, cutoff_empty );
	reset_book_search();
	set_deviation_value( low_thresh, high_thresh, dev_bonus );
	//*/

	/*
	if ( use_thor ) {

#if 1
		/-* No error checking done as it's only for testing purposes *-/

		database_start = get_real_timer();
		(void) read_player_database( "thor\\wthor.jou");
		(void) read_tournament_database( "thor\\wthor.trn" );
		(void) read_game_database( "thor\\wth_2001.wtb" );
		(void) read_game_database( "thor\\wth_2000.wtb" );
		(void) read_game_database( "thor\\wth_1999.wtb" );
		(void) read_game_database( "thor\\wth_1998.wtb" );
		(void) read_game_database( "thor\\wth_1997.wtb" );
		(void) read_game_database( "thor\\wth_1996.wtb" );
		(void) read_game_database( "thor\\wth_1995.wtb" );
		(void) read_game_database( "thor\\wth_1994.wtb" );
		(void) read_game_database( "thor\\wth_1993.wtb" );
		(void) read_game_database( "thor\\wth_1992.wtb" );
		(void) read_game_database( "thor\\wth_1991.wtb" );
		(void) read_game_database( "thor\\wth_1990.wtb" );
		(void) read_game_database( "thor\\wth_1989.wtb" );
		(void) read_game_database( "thor\\wth_1988.wtb" );
		(void) read_game_database( "thor\\wth_1987.wtb" );
		(void) read_game_database( "thor\\wth_1986.wtb" );
		(void) read_game_database( "thor\\wth_1985.wtb" );
		(void) read_game_database( "thor\\wth_1984.wtb" );
		(void) read_game_database( "thor\\wth_1983.wtb" );
		(void) read_game_database( "thor\\wth_1982.wtb" );
		(void) read_game_database( "thor\\wth_1981.wtb" );
		(void) read_game_database( "thor\\wth_1980.wtb" );
		database_stop = get_real_timer();

#if FULL_ANALYSIS
		frequency_analysis( get_total_game_count() );
#endif
		printf( "Loaded %d games in %.3f s.\n", get_total_game_count(),
			database_stop - database_start );
		printf( "Each Thor game occupies %d bytes.\n", get_thor_game_size() );
#else
		{
			/-* This code only used for the generation of screen saver analyses *-/

			GameInfoType thor_game;
			char db_name[100], output_name[100];
			int j;
			int year = 1999;
			int move_count;
			int moves[100];
			FILE *stream;

			database_start = get_real_timer();
			(void) read_player_database( "thor\\wthor.jou");
			(void) read_tournament_database( "thor\\wthor.trn" );
			sprintf( db_name, "thor\\wth_%d.wtb", year );
			(void) read_game_database( db_name );
			database_stop = get_real_timer();
			printf( "Loaded %d games in %.3f s.\n", get_total_game_count(),
				database_stop - database_start );

			database_search( board, color );
			thor_position_count = get_match_count();
			printf( "%d games match the initial position\n", thor_position_count );

			sprintf( output_name, "wc%d.dbs", year );
			stream = fopen( output_name, "w" );
			if ( stream != NULL ) {
				for ( i = 0; i < thor_position_count; i++ ) {
					thor_game = get_thor_game( i );
					if ( strcmp( thor_game.tournament, "Championnat du Monde" ) == 0 ) {
						fprintf( stream, "%d\n", year );
						fprintf( stream, "%s\n", thor_game.black_name );
						fprintf( stream, "%s\n", thor_game.white_name );
						get_thor_game_moves( i, &move_count, moves );
						for ( j = 0; j < move_count; j++ )
							fprintf( stream, "%c%c", TO_SQUARE( moves[j] ) );
						fputs( "\n", stream );
					}
				}
				fclose( stream );
			}
		}
#endif
	}
	//*/

	//if ( depth[CHESS_BLACK] == 0 )
	if ( color != CHESS_BLACK )
		black_name = "Player";
	else
		black_name = "Dolphin";
	//if ( depth[CHESS_WHITE] == 0 )
	if ( color == CHESS_BLACK )
		white_name = "Player";
	else
		white_name = "Dolphin";
	set_names( black_name, white_name );
	set_move_list( black_moves, white_moves, score_sheet_row );
	set_evals( 0.0, 0.0 );

	for ( i = 0; i < 60; i++ ) {
		black_moves[i] = PASS_MOVE;
		white_moves[i] = PASS_MOVE;
	}

	if ( (g_output_channelv || g_output_chatwindowv) && g_odk_stream ) {

		b_discs = disc_count( my_bits, opp_bits, (color == CHESS_BLACK) );
		w_discs = disc_count( my_bits, opp_bits, (color == CHESS_WHITE) );
		e_discs = 64 - b_discs - w_discs;

		if ( color == CHESS_BLACK )
			color_str = "Black";
		else
			color_str = "White";

		if ( g_output_channelv && !g_channel_namev.empty() ) {
			//g_odk_stream->Channel( g_channel_namev.c_str(),
			//	"--------------------" );

			if ( gameid && (strcmp(gameid, "") != 0) ) {
				sprintf( info_buffer, "Game [%s] : %s",
					gameid, color_str );
			}
			else
				sprintf( info_buffer, "Game [unknown] : %s", color_str );
			g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );

			//sprintf( info_buffer, "Disks Played: %d", b_discs + w_discs );
			//g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );

			sprintf( info_buffer, "Time: %02d:%02d / %02d:%02d",
				my_time / 60, my_time % 60,
				opp_time / 60, opp_time % 60 );
			g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );

			sprintf( info_buffer, "Played: %d / %d emp.", b_discs + w_discs, e_discs );
			g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );

			g_odk_stream->Channel( g_channel_namev.c_str(),
				"--------------------" );
		}

		if ( g_output_chatwindowv && gameid && (strcmp(gameid, "") != 0) ) {
			g_odk_stream->ChatWindow( gameid,
				"--------------------" );

			sprintf( info_buffer, "Game [%s] : %s",
				gameid, color_str );
			g_odk_stream->ChatWindow( gameid, info_buffer );

			//sprintf( info_buffer, "Disks Played: %d", b_discs + w_discs );
			//g_odk_stream->ChatWindow( gameid, info_buffer );

			sprintf( info_buffer, "Time: %02d:%02d / %02d:%02d",
				my_time / 60, my_time % 60,
				opp_time / 60, opp_time % 60 );
			g_odk_stream->ChatWindow( gameid, info_buffer );

			sprintf( info_buffer, "Played: %d / %d emp.", b_discs + w_discs, e_discs );
			g_odk_stream->ChatWindow( gameid, info_buffer );

			g_odk_stream->ChatWindow( gameid,
				"--------------------" );
		}
	}

	move_vec[0] = 0;

	black_hash1 = my_random();
	black_hash2 = my_random();
	white_hash1 = my_random();
	white_hash2 = my_random();

	curr_move = PASS_MOVE;

	while ( game_in_progress( my_bits, opp_bits, color ) ) {
		remove_coeffs( disks_played );
		if ( SEPARATE_TABLES ) {  /* Computer players won't share hash tables */
			if ( color == CHESS_BLACK ) {
				g_hash1 ^= black_hash1;
				g_hash2 ^= black_hash2;
			}
			else {
				g_hash1 ^= white_hash1;
				g_hash2 ^= white_hash2;
			}
		}
		generate_all( my_bits, opp_bits, color );

		if ( color == CHESS_BLACK )
			score_sheet_row++;

		if ( move_count[disks_played] != 0 ) {
			move_start = get_real_timer();
			clear_panic_abort();

			if ( echo ) {
				set_move_list( black_moves, white_moves, score_sheet_row );
				set_times( (int)floor( player_time[CHESS_BLACK] ),
					(int)floor( player_time[CHESS_WHITE] ) );
				/*
				opening_name = find_opening_name();
				if ( opening_name != NULL )
					printf( "\nOpening: %s\n", opening_name );
				if ( use_thor ) {
					database_start = get_real_timer();
					database_search( board, color );
					thor_position_count = get_match_count();
					database_stop = get_real_timer();
					total_search_time += database_stop - database_start;
					printf( "%d matching games  (%.3f s search time, %.3f s total)\n",
						thor_position_count, database_stop - database_start,
						total_search_time );
					if ( thor_position_count > 0 ) {
						printf( "%d black wins, %d draws, %d white wins\n",
							get_black_win_count(), get_draw_count(),
							get_white_win_count() );
						printf( "Median score %d-%d", get_black_median_score(),
							64 - get_black_median_score() );
						printf( ", average score %.2f-%.2f\n", get_black_average_score(),
							64.0 - get_black_average_score() );
					}
					print_thor_matches( stdout, thor_max_games );
				}
				//*/
				display_board( stdout, board, color, TRUE, use_timer, TRUE );
			}
			//dump_position( color );
			//dump_game_score( color );

			/* Check what the Thor opening statistics has to say */

			//(void) choose_thor_opening_move( board, color, echo );

			//if ( echo && wait )
			//	dumpch();
			if ( disks_played >= provided_move_count ) {
				if ( depth_info[color] == 0 ) {
					///*
					if ( use_book && display_pv ) {
						fill_move_alternatives( my_bits, opp_bits, color, 0 );
						if ( echo )
							print_move_alternatives( color );
					}
					//*/
					puts( "" );
					curr_move = get_move( color );
				}
				else {
					start_move( player_time[color],
						player_increment[color],
						disks_played + 4 );
					determine_move_time( player_time[color],
						player_increment[color],
						disks_played + 4 );
					timed_search = (depth_info[color] >= 60);
					toggle_experimental( FALSE );

					curr_move =
						compute_move( my_bits, opp_bits, color, TRUE, (int)player_time[color],
							(int)player_increment[color], timed_search,
							use_book, depth_info[color],
							exact_depth[color], wld_depth[color],
							FALSE, &eval_info );
					if ( color == CHESS_BLACK )
						set_evals( produce_compact_eval( eval_info ), 0.0 );
					else
						set_evals( 0.0, produce_compact_eval( eval_info ) );
					// Adjust wld eval
					adjust_wld_eval( &eval_info );

					if ( eval_info.is_book && (rand_move_freq > 0) &&
						(color == rand_color) &&
						((my_random() % rand_move_freq) == 0) ) {
						puts( "Engine override: Random move selected." );
						rand_color = OPP_COLOR( rand_color );
						curr_move =
							move_list[disks_played][my_random() % move_count[disks_played]];
					}
				}
			}
			else {
				curr_move = provided_move[disks_played];
				if ( !move_is_valid( my_bits, opp_bits, curr_move ) )
					fatal_error( "Invalid move %c%c in move sequence",
						TO_SQUARE( curr_move ) );
			}

			move_stop = get_real_timer();
			if ( player_time[color] != INFINIT_TIME )
				player_time[color] -= (move_stop - move_start);

			//store_move( disks_played, curr_move );

			sprintf( move_vec + 2 * disks_played, "%c%c", TO_SQUARE( curr_move ) );
			(void) make_move( &my_bits, &opp_bits, color, curr_move, TRUE );
			if ( color == CHESS_BLACK )
				black_moves[score_sheet_row] = curr_move;
			else {
				if ( white_moves[score_sheet_row] != PASS_MOVE )
					score_sheet_row++;
				white_moves[score_sheet_row] = curr_move;
			}
		}
		else {
			if ( color == CHESS_BLACK )
				black_moves[score_sheet_row] = PASS_MOVE;
			else
				white_moves[score_sheet_row] = PASS_MOVE;
			if ( depth_info[color] == 0 ) {
				puts( "You must pass - please press Enter" );
				dumpch();
			}
		}

		color = OPP_COLOR( color );
		tmp_bits = my_bits;
		my_bits = opp_bits;
		opp_bits = tmp_bits;

		if ( one_position_only )
			break;
	}

	if ( !echo && !one_position_only ) {
		printf( "\n");
		printf( "Black level: %d\n", depth_info[CHESS_BLACK] );
		printf( "White level: %d\n", depth_info[CHESS_WHITE] );
	}

	if ( color == CHESS_BLACK )
		score_sheet_row++;
	//dump_game_score( color );

	if ( echo && !one_position_only ) {
		set_move_list( black_moves, white_moves, score_sheet_row );
		/*
		if ( use_thor ) {
			database_start = get_real_timer();
			database_search( board, color );
			thor_position_count = get_match_count();
			database_stop = get_real_timer();
			total_search_time += database_stop - database_start;
			printf( "%d matching games  (%.3f s search time, %.3f s total)\n",
				thor_position_count, database_stop - database_start,
				total_search_time );
			if ( thor_position_count > 0 ) {
				printf( "%d black wins,  %d draws,  %d white wins\n",
					get_black_win_count(), get_draw_count(),
					get_white_win_count() );
				printf( "Median score %d-%d\n", get_black_median_score(),
					64 - get_black_median_score() );
				printf( ", average score %.2f-%.2f\n", get_black_average_score(),
					64.0 - get_black_average_score() );
			}
			print_thor_matches( stdout, thor_max_games );
		}
		//*/
		set_times( (int)floor( player_time[CHESS_BLACK] ), (int)floor( player_time[CHESS_WHITE] ) );
		display_board( stdout, board, color, TRUE, use_timer, TRUE );
	}

	leaf_val = counter_value( &leaf_nodes );
	node_val = counter_value( &tree_nodes );
	sub_counter( &tmp_nodes, &tree_nodes, &leaf_nodes );
	tree_val = counter_value( &tmp_nodes );
	DecimalToCurrency( leaf_val, leaf_buffer );
	DecimalToCurrency( tree_val, tree_buffer );
	DecimalToCurrency( node_val, total_buffer );

	DecimalToCurrency( counter_value( &tree_nodes ) / total_time, nps_buffer );

	b_discs = disc_count( my_bits, opp_bits, (color == CHESS_BLACK) );
	w_discs = disc_count( my_bits, opp_bits, (color == CHESS_WHITE) );
	e_discs = 64 - b_discs - w_discs;
	printf( "\nB: %d  W: %d  Empties: %d  Move: %c%c, Eval: %0.2f.\n", b_discs,
		w_discs, e_discs, TO_SQUARE( curr_move ), eval_info.score / 128.0f );
	printf( "Nodes: %s, Time: %.3f s, Nodes/Sec = %s nps \n", total_buffer,
		total_time, nps_buffer );
	printf( "\n" );

	// Send to match chat windows's format
	// "/os: .52.1 A 1452 shines: ghjgjk"
	// 1452 is rating

	if ( (g_output_channelv || g_output_chatwindowv) && g_odk_stream ) {
		int max_pv_depth;
		int pv_move;

		if ( g_output_channelv && !g_channel_namev.empty() ) {

			if ( eval_info.type == PASS_EVAL ) {
				sprintf( info_buffer, "Move:  Pass Move" );
				g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );

				sprintf( info_buffer, "Depth: 0" );
				g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );

				sprintf( info_buffer, "Nodes: 0" );
				g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );
			}
			else {
				if ( eval_info.type == INTERRUPTED_EVAL
					|| eval_info.type == SELECTIVE_EVAL )
					sprintf( info_buffer, "Depth: %d (*)", eval_info.search_depth );
				else
					sprintf( info_buffer, "Depth: %d", eval_info.search_depth );
				g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );

				sprintf( info_buffer, "Move: %c%c", TO_SQUARE( curr_move ) );
				g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );

				sprintf( info_buffer, "Eval: %0.2f", eval_info.score / 128.0f );
				g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );

				sprintf( info_buffer, "Time: %.3f sec.", total_time );
				g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );

				sprintf( info_buffer, "Nodes: %s", total_buffer );
				g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );

				//sprintf( info_buffer, "Nodes/Sec = %s nps", nps_buffer );
				sprintf( info_buffer, "Speed: %s nps", nps_buffer );
				g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );

				max_pv_depth = MIN(5, full_pv_depth);
				sprintf( info_buffer, "PV: " );
				for ( i = 0; i < max_pv_depth; i++ ) {
					pv_move = full_pv[i];
					if ( pv_move == PASS_MOVE )
						strcat( info_buffer, "--" );
					else
						sprintf( info_buffer, "%s%c%c", info_buffer, TO_SQUARE( pv_move ) );
					if ( i != (max_pv_depth - 1) )
						strcat( info_buffer, " " );
				}
				g_odk_stream->Channel( g_channel_namev.c_str(), info_buffer );
			}

			g_odk_stream->Channel( g_channel_namev.c_str(),
				"=====================" );
		}

		if ( g_output_chatwindowv && gameid && (strcmp(gameid, "") != 0) ) {

			if ( eval_info.type == PASS_EVAL ) {
				sprintf( info_buffer, "Move: Pass Move" );
				g_odk_stream->ChatWindow( gameid, info_buffer );

				/*
				sprintf( info_buffer, "Depth: 0" );
				g_odk_stream->ChatWindow( gameid, info_buffer );

				sprintf( info_buffer, "Nodes: 0" );
				g_odk_stream->ChatWindow( gameid, info_buffer );
				//*/
			}
			else {
				if ( eval_info.type == INTERRUPTED_EVAL
					|| eval_info.type == SELECTIVE_EVAL )
					sprintf( info_buffer, "Depth: %d (*)", eval_info.search_depth );
				else
					sprintf( info_buffer, "Depth: %d", eval_info.search_depth );
				g_odk_stream->ChatWindow( gameid, info_buffer );

				sprintf( info_buffer, "Move: %c%c", TO_SQUARE( curr_move ) );
				g_odk_stream->ChatWindow( gameid, info_buffer );

				sprintf( info_buffer, "Eval: %0.2f", eval_info.score / 128.0f );
				g_odk_stream->ChatWindow( gameid, info_buffer );

				sprintf( info_buffer, "Time: %.3f sec.", total_time );
				g_odk_stream->ChatWindow( gameid, info_buffer );

				sprintf( info_buffer, "Nodes: %s", total_buffer );
				g_odk_stream->ChatWindow( gameid, info_buffer );

				//sprintf( info_buffer, "Nodes/Sec = %s nps", nps_buffer );
				sprintf( info_buffer, "Speed: %s nps", nps_buffer );
				g_odk_stream->ChatWindow( gameid, info_buffer );

				max_pv_depth = MIN(5, full_pv_depth);
				sprintf( info_buffer, "PV: " );
				for ( i = 0; i < max_pv_depth; i++ ) {
					pv_move = full_pv[i];
					if ( pv_move == PASS_MOVE )
						strcat( info_buffer, "--" );
					else
						sprintf( info_buffer, "%s%c%c", info_buffer, TO_SQUARE( pv_move ) );
					if ( i != (max_pv_depth - 1) )
						strcat( info_buffer, " " );
				}
				g_odk_stream->ChatWindow( gameid, info_buffer );
			}

			g_odk_stream->ChatWindow( gameid,
				"====================" );
		}
	}

	eval_info.confidence = total_time;

	if ( (log_file_name != NULL) && !one_position_only )  {
		log_file = fopen( log_file_name, "a" );
		if ( log_file != NULL ) {
			timer = time( NULL );
			fprintf( log_file, "# %s#     %2d - %2d\n", ctime( &timer ),
				disc_count( my_bits, opp_bits, (color == CHESS_BLACK) ),
				disc_count( my_bits, opp_bits, (color == CHESS_WHITE) ) );
			fprintf( log_file, "%s\n", move_vec );
			fclose( log_file );
		}
	}

	toggle_abort_check( FALSE );
	/*
	if ( use_learning && !one_position_only )
		learn_game( disks_played, (depth[CHESS_BLACK] != 0) && (depth[CHESS_WHITE] != 0),
			TRUE );
	//*/
	toggle_abort_check( TRUE );

	return curr_move;
}

#endif  // DOLPHIN_GGS

#endif  // SCRIPT_ONLY

/*
  RUN_ENDGAME_SCRIPT
*/

#define BUFFER_SIZE           256

static void
run_endgame_script( const char *in_file_name,
				   const char *out_file_name,
				   int display_line ) {
	CounterType script_nodes;
	EvaluationType eval_info;
	char *comment;
	char buffer[BUFFER_SIZE];
	char board_string[BUFFER_SIZE], stm_string[BUFFER_SIZE];
	double start_time, stop_time;
	double search_start, search_stop, max_search;
	int i, j;
	int row, col, pos;
	int book, mid, exact, wld;
	int my_time, my_incr;
	int color, move;
	int score;
	int timed_search;
	int scanned, token;
	int position_count;
	FILE *script_stream;
	FILE *output_stream;
	BitBoard my_bits, opp_bits;
	BitBoard tmp_bits;
	char total_buffer[32], nps_buffer[32];

	/* Open the files and get the number of positions */

	script_stream = fopen( in_file_name, "r" );
	if ( script_stream == NULL ) {
		printf( "\nCan't open script file '%s' - aborting\n\n", in_file_name );
		exit( EXIT_FAILURE );
	}

	output_stream = fopen( out_file_name, "w" );
	if ( output_stream == NULL ) {
		printf( "\nCan't create output file '%s' - aborting\n\n", out_file_name );
		exit( EXIT_FAILURE );
	}
	if ( output_stream ) {
		fprintf( output_stream, "--------------------------------------------------------------------------------\r\n" );
		fprintf( output_stream, "No.  Empty  Move  Value    Time     Total Nodes  Nodes/Sec.  PV Sequence\r\n" );
	}
	fclose( output_stream );

	/* Initialize display subsystem and search parameters */

	set_names( "", "" );
	set_move_list( black_moves, white_moves, score_sheet_row );
	set_evals( 0.0, 0.0 );

	for ( i = 0; i < 60; i++ ) {
		black_moves[i] = PASS_MOVE;
		white_moves[i] = PASS_MOVE;
	}

	my_time = 100000000;
	my_incr = 0;
	timed_search = FALSE;
	book = use_book;
	mid = 60;
	if ( wld_only )
		exact = 0;
	else
		exact = 60;
	wld = 60;

	toggle_status_log( FALSE );

	reset_counter( &script_nodes );
	position_count = 0;
	max_search = -0.0;
	start_time = get_real_timer();

	/* Scan through the script file */

	for ( i = 0; ; i++ ) {
		int pass_count = 0;

		/* Check if the line is a comment or an end marker */

		fgets( buffer, BUFFER_SIZE, script_stream );
		if ( feof( script_stream ) )
			break;
		if ( buffer[0] == '%' ) {  /* Comment */
			output_stream = fopen( out_file_name, "a" );
			if ( output_stream == NULL ) {
				printf( "\nCan't append to output file '%s' - aborting\n\n",
					out_file_name );
				exit( EXIT_FAILURE );
			}
			fputs( buffer, output_stream );
			fclose( output_stream );
			if ( strstr( buffer, "% End of the endgame script" ) == buffer )
				break;
			else
				continue;
		}

		if ( feof( script_stream ) ) {
			printf( "\nEOF encountered when reading position #%d - aborting\n\n",
				i + 1 );
			exit( EXIT_FAILURE );
		}

		/* Parse the script line containing board and side to move */

		game_init( NULL, &color );
		set_slack( (int)0.0 );
		toggle_human_openings( FALSE );
		reset_book_search();
		set_deviation_value( 0, 60, 0.0 );
		hash_setup( TRUE, TRUE );
		position_count++;

		scanned = sscanf( buffer, "%s %s", board_string, stm_string );
		if ( scanned != 2 ) {
			printf( "\nError parsing line %d - aborting\n\n", i + 1 );
			exit( EXIT_FAILURE );
		}

		if ( strlen( stm_string ) != 1 ) {
			printf( "\nAmbiguous side to move on line %d - aborting\n\n", i + 1 );
			exit( EXIT_FAILURE );
		}
		switch ( stm_string[0] ) {
		case 'O':
		case '0':
		case 'o':
		case 'W':
		case 'w':
			color = CHESS_WHITE;
			break;
		case '*':
		case 'X':
		case 'x':
		case 'B':
		case 'b':
			color = CHESS_BLACK;
			break;
		default:
			printf( "\nBad side-to-move indicator on line %d - aborting\n\n",
				i + 1 );
		}

		if ( strlen( board_string ) != 64 ) {
			printf( "\nBoard on line %d doesn't contain 64 positions - aborting\n\n",
				i + 1 );
			exit( EXIT_FAILURE );
		}

		token = 0;
		for ( row = 0; row < 8; row++ ) {
			for ( col = 0; col < 8; col++ ) {
				pos = 8 * row + col;
				switch ( board_string[token] ) {
				case '*':
				case 'X':
				case 'x':
				case 'B':
				case 'b':
					board[pos] = CHESS_BLACK;
					break;
				case 'O':
				case '0':
				case 'o':
				case 'W':
				case 'w':
					board[pos] = CHESS_WHITE;
					break;
				case '-':
				case '.':
					board[pos] = CHESS_EMPTY;
					break;
				default:
					printf( "\nBad character '%c' in board on line %d - aborting\n\n",
						board_string[token], i + 1 );
					break;
				}
				token++;
			}
		}

		set_bitboards( board, color, &my_bits, &opp_bits );
		//disks_played = disc_count2( CHESS_BLACK ) + disc_count2( CHESS_WHITE ) - 4;
		disks_played = disc_count( my_bits, opp_bits, (color == CHESS_BLACK) ) +
			disc_count( my_bits, opp_bits, (color == CHESS_WHITE) ) - 4;

		//hash_set_transformation( my_random(), my_random() );
		clear_ponder_move();

		/* Search the position */

		if ( echo ) {
			set_move_list( black_moves, white_moves, score_sheet_row );
			display_board( stdout, board, color, TRUE, FALSE, TRUE );
		}

		search_start = get_real_timer();
		start_move( my_time, my_incr, disks_played + 4 );
		determine_move_time( my_time, my_incr, disks_played + 4 );

		pass_count = 0;
		move = compute_move( my_bits, opp_bits, color, TRUE, my_time, my_incr, timed_search,
			book, mid, exact, wld, TRUE, &eval_info );

		if ( move == PASS_MOVE ) {
			pass_count++;
			color = OPP_COLOR( color );
			tmp_bits = my_bits;
			my_bits = opp_bits;
			opp_bits = tmp_bits;
			move = compute_move( my_bits, opp_bits, color, TRUE, my_time, my_incr, timed_search,
				book, mid, exact, wld, TRUE, &eval_info );
			if ( move == PASS_MOVE ) {  /* Both pass, game over. */
				int my_discs = disc_count( my_bits, opp_bits, TRUE );
				int opp_discs = disc_count( my_bits, opp_bits, FALSE );
				if ( my_discs > opp_discs )
					my_discs = 64 - opp_discs;
				else if ( opp_discs > my_discs )
					opp_discs = 64 - my_discs;
				else
					my_discs = opp_discs = 32;
				eval_info.score = 128 * (my_discs - opp_discs);
				pass_count++;
			}
		}

		score = eval_info.score / 128;
		search_stop = get_real_timer();
		if ( search_stop - search_start > max_search )
			max_search = search_stop - search_start;
		add_counter( &script_nodes, &tree_nodes );

		output_stream = fopen( out_file_name, "a" );
		if ( output_stream == NULL ) {
			printf( "\nCan't append to output file '%s' - aborting\n\n",
				out_file_name );
			exit( EXIT_FAILURE );
		}
		if ( wld_only ) {
			if ( color == CHESS_BLACK ) {
				if ( score > 0 )
					fputs( "Black win", output_stream );
				else if ( score == 0 )
					fputs( "Draw", output_stream );
				else
					fputs( "White win", output_stream );
			}
			else {
				if ( score > 0 )
					fputs( "White win", output_stream );
				else if ( score == 0 )
					fputs( "Draw", output_stream );
				else
					fputs( "Black win", output_stream );
			}
		}
		else {
			if ( color == CHESS_BLACK )
				fprintf( output_stream, "%2d  %2d - %2d",
					i,
					32 + (score / 2), 32 - (score / 2) );
			else
				fprintf( output_stream, "%2d  %2d - %2d",
					i,
					32 - (score / 2), 32 + (score / 2) );
		}
		if ( display_line && (pass_count != 2) ) {
			fputs( "   ", output_stream );
			if ( pass_count == 1 )
				fputs( " --", output_stream );
			for ( j = 0; j < full_pv_depth; j++ ) {
				fputs( " ", output_stream );
				display_move( output_stream, full_pv[j] );
			}
		}
		comment = strstr( buffer, "%" );
		if ( comment != NULL )  /* Copy comment to output file */
			fprintf( output_stream, "      %s", comment );
		else
			fputs( "\n", output_stream );
		fclose( output_stream );

		if ( echo )
			puts( "\n" );
	}

	/* Clean up and terminate */

	fclose( script_stream );

	stop_time = get_real_timer();

	output_stream = fopen( out_file_name, "a" );
	if ( output_stream == NULL ) {
		printf( "\nCan't append to output file '%s' - aborting\n\n",
			out_file_name );
		exit( EXIT_FAILURE );
	}

	DecimalToCurrency( counter_value( &script_nodes ), total_buffer );
	DecimalToCurrency( counter_value( &script_nodes ) / ( stop_time - start_time ), nps_buffer );
	fprintf( output_stream, "--------------------------------------------------------------------------------\r\n" );
	fprintf( output_stream, "Total Nodes = %s   Total Time = %.3f sec   Nodes/Sec = %s nps",
		total_buffer, stop_time - start_time, nps_buffer );
	fclose( output_stream );

	printf( "Total positions solved:   %d\n", position_count );
	printf( "Total time:               %.1f s\n", stop_time - start_time );
	printf( "Total nodes:              %.0f\n", counter_value( &script_nodes ) );
	puts( "" );
	printf( "Average time for solve:   %.1f s\n",
		(stop_time - start_time) / position_count );
	printf( "Maximum time for solve:   %.1f s\n", max_search );
	puts( "" );
	printf( "Average speed:            %.0f nps\n",
		counter_value( &script_nodes ) / (stop_time - start_time ) );
	puts( "" );
}

#if SCRIPT_ONLY

static void
generate_random_endgame(const char *game_file,
						int rnd_discs,
						int empties_disc,
						int mid_depth,
						int max_position,
						int hash_bits) {
	CounterType script_nodes;
	EvaluationType eval_info;
	double start_time, stop_time;
	double search_start, search_stop, max_search;
	int i, j;
	//int row, col, pos;
	int book, mid, exact, wld;
	int my_time, my_incr;
	int color, move;
	int rnd_depth;
	int pass_count;
	int score;
	int randNum;
	int timed_search;
	int position_count;
	BitBoard my_bits, opp_bits;
	BitBoard tmp_bits;
	time_t timer;
	int use_random;
	FILE *stream;
	double fscore1, fscore2, error_dif;
	double error1[10100], error2[10100];
	double error_sum1, error_sum2;
	double error_avg1, error_avg2;
	double error_abs1, error_abs2;
	double error_mse1, error_mse2;
	int nscore;

	int bIsEndGame;
	int my_discs, opp_discs;

	BitBoard my_end_bits, opp_end_bits;
	int color_end;
	int pv_move, pass_count_end;
	int my_end_discs, opp_end_discs;
	EvaluationType eval_info_end;
    int end_loop_count;

    int depth, t;
    int score_end;

    unsigned int hi_mask, lo_mask;

	if (game_file == NULL)
		return;

	stream = fopen( game_file, "wb" );
	if ( stream == NULL )
		printf( "Error creating '%s' .\n", game_file );

	use_random = DEFAULT_RANDOM;
	wait = DEFAULT_WAIT;
	echo = DEFAULT_ECHO;
	display_pv = DEFAULT_DISPLAY_PV;
	use_learning = FALSE;
	use_thor = DEFAULT_USE_THOR;
	depth_info[CHESS_BLACK] = depth_info[CHESS_WHITE] = -1;
	log_file_name = NULL;
	player_time[CHESS_BLACK] = player_time[CHESS_WHITE] = INFINIT_TIME;
	player_increment[CHESS_BLACK] = player_increment[CHESS_WHITE] = 0.0;
	use_book = FALSE;
	wld_only = FALSE;

	g_stdout = stdout;
	if ( hash_bits <= 0 )
		hash_bits = DEFAULT_HASH_BITS;

	global_setup( use_random, hash_bits );
	//init_thor_database();

	if ( use_book ) {
		init_learn( "book.bin", TRUE );
		printf( "use book.\n" );
	}

	if ( use_random ) {
		srand( (unsigned)time( NULL ) );
		time( &timer );
		my_srandom( (int)timer );
	}
	else
		my_srandom( 1 );

	game_init( NULL, &color );
	hash_setup( TRUE, TRUE );

	/* Initialize display subsystem and search parameters */

	set_names( "", "" );
	set_move_list( black_moves, white_moves, score_sheet_row );
	set_evals( 0.0, 0.0 );

	for ( i = 0; i < 60; i++ ) {
		black_moves[i] = PASS_MOVE;
		white_moves[i] = PASS_MOVE;
	}

	my_time = 100000000;
	my_incr = 0;
	timed_search = FALSE;
	book = use_book;
	//mid = 1;
	mid = mid_depth;
	exact = 64 - 4 - rnd_discs;
	wld = 0;

	toggle_status_log( FALSE );

	position_count = 0;
	max_search = -0.0;
	start_time = get_real_timer();

	initialize_pattern_coffes();

	//srand( 1 );
	//my_srandom( 1 );

    error_dif = 0.0;
	for ( i = 0; i < max_position; i++ ) {
		error1[i] = 0.0;
		error2[i] = 0.0;
	}
	error_sum1 = 0.0; error_sum2 = 0.0;
	error_avg1 = 0.0; error_avg2 = 0.0;
	error_abs1 = 0.0; error_abs2 = 0.0;
	error_mse1 = 0.0; error_mse2 = 0.0;

	while (position_count < max_position) {
		reset_counter( &script_nodes );

		game_init(NULL, &color);
		toggle_human_openings( FALSE );
		hash_setup( TRUE, FALSE );

		clear_pv();

		set_bitboards( board, color, &my_bits, &opp_bits );
		//disks_played = disc_count2( CHESS_BLACK ) + disc_count2( CHESS_WHITE ) - 4;
		disks_played = disc_count( my_bits, opp_bits, (color == CHESS_BLACK) ) +
			disc_count( my_bits, opp_bits, (color == CHESS_WHITE) ) - 4;

		//hash_set_transformation( my_random(), my_random() );
		clear_ponder_move();

		/* Search the position */

		if ( echo ) {
			set_move_list( black_moves, white_moves, score_sheet_row );
			//display_board( stdout, board, color, TRUE, FALSE, TRUE );
		}

		search_start = get_real_timer();
		start_move( my_time, my_incr, disks_played + 4 );
		determine_move_time( my_time, my_incr, disks_played + 4 );

		randNum = rand() ^ my_random();
		for ( i = 0; i < MAX_SEARCH_DEPTH; i++ )
			pv[62][i] = -1;

		g_rand_terminal = FALSE;
		score = generate_random_game( my_bits, opp_bits, color, 0, rnd_discs );
		rnd_depth = 0;
		for ( j = 0; j < rnd_discs; j++ ) {
			move = pv[62][j];
			color = pv[63][j];
			if ( move == -1 )
				break;
			if ( color == CHESS_BLACK )
				make_move_no_hash( &my_bits, &opp_bits, color, move );
			else
				make_move_no_hash( &opp_bits, &my_bits, color, move );
			rnd_depth++;
		}

		color = OPP_COLOR( color );
		if ( color == CHESS_WHITE ) {
			tmp_bits = my_bits;
			my_bits = opp_bits;
			opp_bits = tmp_bits;
		}
		set_boards(color, my_bits, opp_bits, board);
		disks_played = disc_count( my_bits, opp_bits, (color == CHESS_BLACK) ) +
			disc_count( my_bits, opp_bits, (color == CHESS_WHITE) ) - 4;

		if ( echo ) {
			display_board( stdout, board, color, TRUE, FALSE, TRUE );
		}

		if ( rnd_depth < rnd_discs ) {
			printf( "\nMidgame terminal at %d discs!\n\n", rnd_depth + 4 );
			continue;
		}

		nscore = pattern_evaluation( color, my_bits, opp_bits );
		fscore1 = ((double)nscore / 128.0);
		fscore2 = calc_pattern_score( color, my_bits, opp_bits );

		clear_ponder_move();

		search_start = get_real_timer();
		start_move( my_time, my_incr, disks_played + 4 );
		determine_move_time( my_time, my_incr, disks_played + 4 );

		pass_count = 0;
		move = compute_move( my_bits, opp_bits, color, TRUE, my_time, my_incr, timed_search,
			book, mid, exact, wld, TRUE, &eval_info );

        bIsEndGame = FALSE;
		if ( move == PASS_MOVE ) {
			pass_count++;
			color = OPP_COLOR( color );
			tmp_bits = my_bits;
			my_bits = opp_bits;
			opp_bits = tmp_bits;
			move = compute_move( my_bits, opp_bits, color, TRUE, my_time, my_incr, timed_search,
				book, mid, exact, wld, TRUE, &eval_info );
			if ( move == PASS_MOVE ) {  /* Both pass, game over. */
				color = OPP_COLOR( color );
				tmp_bits = my_bits;
				my_bits = opp_bits;
				opp_bits = tmp_bits;
				my_discs  = disc_count( my_bits, opp_bits, TRUE );
				opp_discs = disc_count( my_bits, opp_bits, FALSE );
				if ( my_discs > opp_discs )
					my_discs = 64 - opp_discs;
				else if ( opp_discs > my_discs )
					opp_discs = 64 - my_discs;
				else
					my_discs = opp_discs = 32;
				eval_info.score = 128 * (my_discs - opp_discs);
				pass_count++;
				if ((my_discs + opp_discs) >= 63)
					bIsEndGame = TRUE;
			}
		}

		eval_info_end.score = 0;
		my_end_bits = my_bits;
		opp_end_bits = opp_bits;
		color_end = color;

		end_loop_count = 0;
		while ( TRUE ) {
			pass_count_end = 0;
			my_end_discs  = disc_count( my_end_bits, opp_end_bits, TRUE );
			opp_end_discs = disc_count( my_end_bits, opp_end_bits, FALSE );
			/*
			if (my_end_discs + opp_end_discs >= 64)
				break;
			//*/
			pv_move = compute_move( my_end_bits, opp_end_bits, color_end, TRUE, my_time, my_incr, timed_search,
				book, mid, exact, wld, TRUE, &eval_info_end );
			if ( pv_move == PASS_MOVE ) {
				pass_count_end++;
				color_end = OPP_COLOR( color_end );
				tmp_bits = my_end_bits;
				my_end_bits = opp_end_bits;
				opp_end_bits = tmp_bits;
				pv_move = compute_move( my_end_bits, opp_end_bits, color_end, TRUE, my_time, my_incr, timed_search,
					book, mid, exact, wld, TRUE, &eval_info_end );
				if ( pv_move == PASS_MOVE ) {  /* Both pass, game over. */
					color_end = OPP_COLOR( color_end );
					tmp_bits = my_end_bits;
					my_end_bits = opp_end_bits;
					opp_end_bits = tmp_bits;
					// count discs
					my_end_discs  = disc_count( my_end_bits, opp_end_bits, TRUE );
					opp_end_discs = disc_count( my_end_bits, opp_end_bits, FALSE );
					if ( my_end_discs > opp_end_discs )
						my_end_discs = 64 - opp_end_discs;
					else if ( opp_end_discs > my_end_discs )
						opp_end_discs = 64 - my_end_discs;
					else
						my_end_discs = opp_end_discs = 32;
					eval_info_end.score = 128 * (my_end_discs - opp_end_discs);
					pass_count_end++;
					break;
				}
			}
			//printf( "\npv_move = " );
			if (full_pv_depth > 0) {
				for (depth=0; depth<full_pv_depth; depth++) {
					pv_move = full_pv[depth];
					if (pv_move != PASS_MOVE) {
						if (make_move(&my_end_bits, &opp_end_bits, color_end, pv_move, TRUE) > 0) {
							color_end = OPP_COLOR( color_end );
							tmp_bits = my_end_bits;
							my_end_bits = opp_end_bits;
							opp_end_bits = tmp_bits;
						}
						else break;
					}
					else {
						color_end = OPP_COLOR( color_end );
						tmp_bits = my_end_bits;
						my_end_bits = opp_end_bits;
						opp_end_bits = tmp_bits;
					}
					//printf( "%d ", pv_move );
				}
				//printf( "\n" );
			}
			else {
				if (pv_move != PASS_MOVE) {
					if (make_move(&my_end_bits, &opp_end_bits, color_end, pv_move, TRUE) > 0) {
						color_end = OPP_COLOR( color_end );
						tmp_bits = my_end_bits;
						my_end_bits = opp_end_bits;
						opp_end_bits = tmp_bits;
					}
				}
			}
			clear_pv();
			end_loop_count++;
			if (end_loop_count >= 80)
				break;
		}

		score_end = eval_info_end.score / 128;
		if (pass_count_end == 1)
			score_end = -score_end;
		for (t=0; t<64; t++)
			board[t] = CHESS_EMPTY;
		set_boards( color_end, my_end_bits, opp_end_bits, board );
		if ( echo ) {
			display_board( stdout, board, color_end, TRUE, FALSE, TRUE );
		}

		score = eval_info.score / 128;
		if (pass_count == 1)
			score = -score;
		printf( "\n s1 = %+0.2f (%+0.2f/%+d), s2 = %+0.2f (%+0.2f/%+d).\n",
			fscore1, (fscore1 - (double)score), score,
			fscore2, (fscore2 - (double)score), score );
		
		search_stop = get_real_timer();
		if ( search_stop - search_start > max_search )
			max_search = search_stop - search_start;
		add_counter( &script_nodes, &tree_nodes );

		error1[position_count] = fscore1 - (double)score;
		error2[position_count] = fscore2 - (double)score;

		if ( (move != PASS_MOVE || bIsEndGame) && (pass_count_end >= 2)) {
			my_end_discs  = disc_count( my_end_bits, opp_end_bits, TRUE );
			opp_end_discs = disc_count( my_end_bits, opp_end_bits, FALSE );
			//if (my_end_discs + opp_end_discs < 64)
			//	system( "pause" );
			// current positions
			disks_played = disc_count( my_bits, opp_bits, (color == CHESS_BLACK) ) +
				disc_count( my_bits, opp_bits, (color == CHESS_WHITE) ) - 4;
			if ( color != CHESS_BLACK ) {
				tmp_bits = my_bits;
				my_bits = opp_bits;
				opp_bits = tmp_bits;
			}
			for ( i = 0; i < 4; i++ ) {
				hi_mask = (my_bits.low & 0xF0) >> 4;
				hi_mask += ' ';
				lo_mask = my_bits.low & 0x0F;
				lo_mask += ' ';
				fprintf( stream, "%c%c", (unsigned char)hi_mask, (unsigned char)lo_mask );
				hi_mask = (opp_bits.low & 0xF0) >> 4;
				hi_mask += ' ';
				lo_mask = opp_bits.low & 0x0F;
				lo_mask += ' ';
				fprintf( stream, "%c%c", (unsigned char)hi_mask, (unsigned char)lo_mask );
				my_bits.low >>= 8;
				opp_bits.low >>= 8;
			}
			for ( i = 0; i < 4; i++ ) {
				hi_mask = (my_bits.high & 0xF0) >> 4;
				hi_mask += ' ';
				lo_mask = my_bits.high & 0x0F;
				lo_mask += ' ';
				fprintf( stream, "%c%c", (unsigned char)hi_mask, (unsigned char)lo_mask );
				hi_mask = (opp_bits.high & 0xF0) >> 4;
				hi_mask += ' ';
				lo_mask = opp_bits.high & 0x0F;
				lo_mask += ' ';
				fprintf( stream, "%c%c", (unsigned char)hi_mask, (unsigned char)lo_mask );
				my_bits.high >>= 8;
				opp_bits.high >>= 8;
			}
			if ( color == CHESS_BLACK )
				fprintf( stream, " * ");
			else if ( color == CHESS_WHITE )
				fprintf( stream, " O ");
			else
				fprintf( stream, " ? ");
			fprintf( stream, "%d %d\r\n", disks_played, score );

			// end game positions
			disks_played = disc_count( my_end_bits, opp_end_bits, (color_end == CHESS_BLACK) ) +
				disc_count( my_end_bits, opp_end_bits, (color_end == CHESS_WHITE) ) - 4;
			if ( color_end != CHESS_BLACK ) {
				tmp_bits = my_end_bits;
				my_end_bits = opp_end_bits;
				opp_end_bits = tmp_bits;
			}
			//unsigned char hi_mask, lo_mask;
			for ( i = 0; i < 4; i++ ) {
				hi_mask = (my_end_bits.low & 0xF0) >> 4;
				hi_mask += ' ';
				lo_mask = my_end_bits.low & 0x0F;
				lo_mask += ' ';
				fprintf( stream, "%c%c", (unsigned char)hi_mask, (unsigned char)lo_mask );
				hi_mask = (opp_end_bits.low & 0xF0) >> 4;
				hi_mask += ' ';
				lo_mask = opp_end_bits.low & 0x0F;
				lo_mask += ' ';
				fprintf( stream, "%c%c", (unsigned char)hi_mask, (unsigned char)lo_mask );
				my_end_bits.low >>= 8;
				opp_end_bits.low >>= 8;
			}
			for ( i = 0; i < 4; i++ ) {
				hi_mask = (my_end_bits.high & 0xF0) >> 4;
				hi_mask += ' ';
				lo_mask = my_end_bits.high & 0x0F;
				lo_mask += ' ';
				fprintf( stream, "%c%c", (unsigned char)hi_mask, (unsigned char)lo_mask );
				hi_mask = (opp_end_bits.high & 0xF0) >> 4;
				hi_mask += ' ';
				lo_mask = opp_end_bits.high & 0x0F;
				lo_mask += ' ';
				fprintf( stream, "%c%c", (unsigned char)hi_mask, (unsigned char)lo_mask );
				my_end_bits.high >>= 8;
				opp_end_bits.high >>= 8;
			}
			if ( color_end == CHESS_BLACK )
				fprintf( stream, " * ");
			else if ( color_end == CHESS_WHITE )
				fprintf( stream, " O ");
			else
				fprintf( stream, " ? ");
			fprintf( stream, "%d %d\r\n", disks_played, score_end );

			if ( ((position_count+1) % 50) == 0 ) {
				error_sum1 = 0.0;
				error_sum2 = 0.0;
				for ( i = 0; i <= position_count; i++ ) {
					error_sum1 += error1[i];
					error_sum2 += error2[i];
				}

				error_avg1 = error_sum1 / (double)(position_count + 1);
				error_avg2 = error_sum2 / (double)(position_count + 1);

				error_mse1 = 0.0;
				error_mse2 = 0.0;
				error_abs1 = 0.0;
				error_abs2 = 0.0;
				for ( i = 0; i <= position_count; i++ ) {
					error_mse1 += error1[i] * error1[i];
					error_mse2 += error2[i] * error2[i];
					error_abs1 += fabs(error1[i]);
					error_abs2 += fabs(error2[i]);
				}
				error_mse1 = (error_mse1 / (double)(position_count + 1));
				error_mse2 = (error_mse2 / (double)(position_count + 1));
				error_abs1 = error_abs1 / (double)(position_count + 1);
				error_abs2 = error_abs2 / (double)(position_count + 1);

				printf( "\nposition_count = %d.\n", position_count + 1);
				printf( "error_avg1: %0.6f, error_abs1: %0.6f, error_mse1: %0.6f\n"
					"error_avg2: %0.6f, error_abs2: %0.6f, error_mse2: %0.6f\n",
					error_avg1, error_abs1, error_mse1,
					error_avg2, error_abs2, error_mse2 );
				//getch();
			}
		}

		position_count++;
		if ( (position_count % 64) == 63 ) {
			fflush( stream );
			if ( (position_count % 256) == 255 ) {
				time( &timer );
				my_srandom( (int)timer );
				srand( (unsigned)time( NULL ) );
			}
		}
	}
	stop_time = get_real_timer();

	if ( stream )
		fclose( stream );
}

#endif

#if !SCRIPT_ONLY
/*
   DUMP_POSITION
   Saves the current board position to disk.
*/

static void
dump_position( int color ) {
	int i, j;
	FILE *stream;

	stream = fopen( "current.gam", "w" );
	if ( stream == NULL )
		fatal_error( "File creation error when writing CURRENT.GAM\n" );

	for ( i = 0; i < 8; i++ ) {
		for ( j = 0; j < 8; j++ ) {
			switch ( board[8 * i + j] ) {
			case CHESS_BLACK:
				fputc( 'X', stream );
				break;

			case CHESS_WHITE:
				fputc( 'O', stream );
				break;

			case CHESS_EMPTY:
				fputc( '-', stream );
				break;

			default:  /* This really can't happen but shouldn't cause a crash */
				fputc( '?', stream );
				break;
			}
		}
	}
	fputs( "\n", stream );
	if ( color == CHESS_BLACK )
		fputs( "Black", stream );
	else
		fputs( "White", stream );
	fputs( " to move\nThis file was automatically generated\n", stream );

	fclose( stream );
}

/*
  DUMP_GAME_SCORE
  Writes the current game score to disk.
*/

static void
dump_game_score( int color ) {
	FILE *stream;
	int i;

	stream = fopen( "current.mov", "w" );
	if ( stream == NULL )
		fatal_error( "File creation error when writing CURRENT.MOV\n" );

	for ( i = 0; i <= score_sheet_row; i++ ) {
		fprintf( stream, "   %2d.    ", i + 1 );
		if ( black_moves[i] == PASS_MOVE )
			fputs( "- ", stream );
		else
			fprintf( stream, "%c%c", TO_SQUARE( black_moves[i] ) );
		fputs( "  ", stream );
		if ( (i < score_sheet_row) ||
			((i == score_sheet_row) && (color == CHESS_BLACK)) ) {
			if ( white_moves[i] == PASS_MOVE )
				fputs( "- ", stream );
			else
				fprintf( stream, "%c%c", TO_SQUARE( white_moves[i] ) );
		}
		fputs( "\n", stream );
	}
	fclose( stream );
}

#endif
