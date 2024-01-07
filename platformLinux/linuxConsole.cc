//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include <sys/types.h>
#include <stdio.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "platformLinux/platformLinux.h"
#include "platformLinux/linuxConsole.h"
#include "Platform/event.h"
#include "Platform/gameInterface.h"
#include "loki_utils.h"

LinuxConsole* linuxConsole = 0;

ConsoleFunction( enableWinConsole, void, 2, 2, "enableWinConsole(bool);" )
{
	linuxConsole->enable( dAtob( argv[1] ) );
}

static void handle_sigwinch(int sig)
{
	if ( linuxConsole ) {
		linuxConsole->check_winsize();
	}
}

void LinuxConsole::create( void )
{
	linuxConsole = new LinuxConsole( );
	signal(SIGWINCH, handle_sigwinch);
}

void LinuxConsole::destroy( void )
{
	signal(SIGWINCH, SIG_DFL);
	delete linuxConsole;
	linuxConsole = 0;
}

static void linuxConsoleConsumer(ConsoleLogEntry::Level level, const char* line)
{
	if ( linuxConsole ) {
		linuxConsole->processConsoleLine( level, line );
	}
}

LinuxConsole::LinuxConsole( void ) : console_enabled( false )
{
	init_history();
	Con::addConsumer( linuxConsoleConsumer );
}

LinuxConsole::~LinuxConsole( void )
{
	enable(false);
	free_history();
	Con::removeConsumer( linuxConsoleConsumer );
}

// Terminal manipulation routines

void LinuxConsole::set_title(const char *title)
{
	printf("\033]0;%s\07", title);
	fflush(stdout);
}

void LinuxConsole::move_bol(void)
{
	printf("\r");
}

void LinuxConsole::clear_eol(void)
{
	printf("\033[K");
}

void LinuxConsole::clear_line(void)
{
	move_bol();
	clear_eol();
}

void LinuxConsole::move_to(int row, int col)
{
	printf("\033[%d;%dH", row, col);
}

void LinuxConsole::move_back()
{
	printf("\033[D");
}

void LinuxConsole::set_scroll(int hi, int lo, int col)
{
	if ( ! hi && ! lo ) {
		printf("\033[r");
	} else {
		printf("\033[%d;%dr", hi, lo);
		move_to(lo, col);
	}
}

void LinuxConsole::check_winsize(void)
{
	const char *env;

	struct /* winsize */ {
		unsigned short  ws_row;
		unsigned short  ws_col;
		unsigned short  ws_xpixel;
		unsigned short  ws_ypixel;
	} mywinz;

	if ( ioctl(0, TIOCGWINSZ, &mywinz) == 0 ) {
		if ( mywinz.ws_row )
			rows = mywinz.ws_row;
		if ( mywinz.ws_col )
			cols = mywinz.ws_col;
	}
	if ( (env=getenv("LINES")) != NULL )
		rows=atoi(env);
	if ( (env=getenv("COLUMNS")) != NULL )
		cols=atoi(env);

	/* Now set defaults if we can't find the window size */
	if ( ! rows )  rows=24;
	if ( ! cols )  cols=80;

	// Display the prompt
	display_prompt();
}

void LinuxConsole::enable(bool enabled)
{
	static struct termios saved_termios;

	if ( enabled == console_enabled ) {
		return;
	}
	console_enabled = enabled;

	// Okay, if there's no terminal, we're done
	if ( ! isatty(0) || ! isatty(1) ) {
		return;
	}

	if ( enabled ) {
		// Save the terminal settings and go raw
		const char *title;
		const char *env;
		struct termios raw_termios;
		struct fd_set fdset;
		struct timeval timeout;

		// Clear the input buffer
		delete_line();

		// Save the original terminal settings
		tcgetattr(0, &saved_termios);

		// Set the raw terminal settings
		raw_termios = saved_termios;
		raw_termios.c_iflag = IGNBRK;
		raw_termios.c_oflag &= ~OLCUC;
		raw_termios.c_lflag = ISIG;
		raw_termios.c_cc[VMIN] = 0;
		raw_termios.c_cc[VTIME] = 10;
		tcsetattr(0, TCSADRAIN, &raw_termios);

		// See if the terminal uses vt100 emulation
		split_window = false;
		printf("\033[c");  /* Vt100 test: ESC [ c */
		fflush(stdout);
		FD_ZERO( &fdset );
		FD_SET( 0, &fdset );
		timeout.tv_sec = 0;
		timeout.tv_usec = 500*1000;
		while ( select(1, &fdset, NULL, NULL, &timeout) == 1 ) {
			char ch;

			read(0, &ch, 1);
			if ( ch == '\033' ) {
				split_window = true;
			}
			timeout.tv_sec = 0;
			timeout.tv_usec = 500*1000;
		}

		// Set up the split window if we can
		rows = 0;
		cols = 0;
		if ( split_window ) {
			check_winsize();
		} else {
			// No split window, do line-by-line input
			tcsetattr(0, TCSADRAIN, &saved_termios);
		}

		// Try to set the xterm title bar
		title = Con::getVariable("Con::WindowTitle");
		if ( title && *title ) {
			// See if the terminal uses xterm title
			if ( ((env=getenv("TERM")) != NULL) &&
			     (dStrncmp(env, "xterm", 5) == 0) ) {
				set_title(title);
			}
		}
	} else {
		// Clear the split window, if any
		if ( split_window ) {
#ifdef VT100_SCROLL
			set_scroll(0, 0, 0);
			move_to(rows, 0);
			clear_eol();
#else
			printf("\n");
#endif
			// Restore the terminal settings
			tcsetattr(0, TCSADRAIN, &saved_termios);
		}
	}
}

void LinuxConsole::processConsoleLine( ConsoleLogEntry::Level level, const char* line )
{
	char cleaned[256];

	if ( isEnabled( ) ) {
		// clean string--forums sometimes emit
		// control characters which foo the terminal
		dStrncpy( cleaned, line, 256 );
		cleaned[255] = '\0';
		int length = dStrlen( cleaned );

		for( int i = 0; i < length; i++ ) {

			if( !isprint( cleaned[i] ) ) {
				cleaned[i] = ' ';
			}
		}
		if ( split_window ) {
#ifdef VT100_SCROLL
			const char *prompt = Con::getVariable("Con::Prompt");
			set_scroll(1, rows-1, 0);
			printf( "\n%s", cleaned );
			set_scroll(rows-1, rows, dStrlen(prompt)+curpos+1);
#else
			clear_line();
			printf( "%s\n", cleaned );
			display_prompt();
#endif
			fflush(stdout);
		} else {
			printf( "%s\n", cleaned );
		}
	}
}

void LinuxConsole::forward_char(void)
{
	if ( curpos < dStrlen(line) ) {
		++curpos;
	}
}

void LinuxConsole::backward_char(void)
{
	if ( curpos > 0 ) {
		--curpos;
	}
}

void LinuxConsole::forward_eol(void)
{
	curpos = dStrlen(line);
}

void LinuxConsole::backward_bol(void)
{
	curpos = 0;
}

void LinuxConsole::backspace_char(void)
{
	if ( curpos > 0 ) {
		--curpos;
		memcpy(&line[curpos], &line[curpos+1], sizeof(line)-curpos);
	}
}

void LinuxConsole::delete_char(void)
{
	memcpy(&line[curpos], &line[curpos+1], sizeof(line)-curpos);
}

void LinuxConsole::delete_eol(void)
{
	memset(&line[curpos], 0, sizeof(line)-curpos);
}

void LinuxConsole::delete_line(void)
{
	curpos = 0;
	memset(line, 0, sizeof(line));
}

void LinuxConsole::insert_char(char ch)
{
	if ( line[curpos] != '\0' ) {
		memmove(&line[curpos+1],&line[curpos],(sizeof(line)-curpos-1));
	}
	replace_char(ch);
}

void LinuxConsole::replace_char(char ch)
{
	line[curpos++] = ch;
}

void LinuxConsole::replace_line(const char *text)
{
	delete_line();
	if ( text ) {
		strncpy(line, text, sizeof(line)-1);
		curpos = dStrlen(line);
	}
}

void LinuxConsole::init_history(void)
{
	char histfile[PATH_MAX];
	FILE *hfp;

	memset(history, 0, sizeof(history));
	current_history = 0;

	// Load the history from our history file
	sprintf(histfile, "%s/history.txt", loki_getprefpath());
	hfp = fopen(histfile, "r");
	if ( hfp ) {
		while ( fgets(line, sizeof(line), hfp) ) {
			line[dStrlen(line)-1] = '\0';
			add_history(line);
			incr_history();
		}
		fclose(hfp);
	}
}

void LinuxConsole::add_history(const char *text)
{
	if ( history[current_history] ) {
		free(history[current_history]);
	}
	history[current_history] = dStrdup(text);
}

void LinuxConsole::incr_history(void)
{
	++current_history;
	if ( current_history == NUM_HISTORY ) {
		current_history = 0;
	}
}

void LinuxConsole::decr_history(void)
{
	--current_history;
	if ( current_history < 0 ) {
		current_history += NUM_HISTORY;
	}
}

void LinuxConsole::prev_history(void)
{
	add_history(line);
	decr_history();
	replace_line(history[current_history]);
}

void LinuxConsole::next_history(void)
{
	add_history(line);
	incr_history();
	replace_line(history[current_history]);
}

void LinuxConsole::free_history(void)
{
	char histfile[PATH_MAX];
	FILE *hfp;

	// Save the history to our history file
	sprintf(histfile, "%s/history.txt", loki_getprefpath());
	hfp = fopen(histfile, "w");
	if ( hfp ) {
		int history_mark = current_history;
		do {
			if ( history[current_history] ) {
				fprintf(hfp, "%s\n", history[current_history]);
			}
			incr_history();
		} while ( current_history != history_mark );

		fclose(hfp);
	}

	for ( S32 i=0; i<NUM_HISTORY; ++i ) {
		if ( history[i] ) {
			dFree(history[i]);
			history[i] = 0;
		}
	}
}

void LinuxConsole::tab_complete(void)
{
	const char *newtext;

	newtext = Con::tabComplete(line, curpos, true);
	if ( newtext ) {
		int save_pos = curpos;
		replace_line(newtext);
		curpos = save_pos;
	}
}

void LinuxConsole::display_prompt(void)
{
	if ( split_window ) {
		const char *prompt = Con::getVariable("Con::Prompt");
		clear_line();
		printf("%s%s", prompt, line);
#ifdef VT100_SCROLL
		move_to(rows, dStrlen(prompt)+curpos+1);
#else
		U32 maxpos = dStrlen(line);
		for ( U32 i=(U32)curpos; i<maxpos; ++i ) {
			move_back();
		}
#endif
		fflush(stdout);
	}
}

#define toctrl(X)	(X - '@')

void LinuxConsole::process( void )
{
	char ch;
	struct fd_set fdset;
	struct timeval timeout;
	static enum {
		AT_RESET,
		AT_ESCAPE,
		AT_BRACKET
	} arrow_state = AT_RESET;

	FD_ZERO( &fdset );
	FD_SET( 0, &fdset );
	timeout.tv_sec = 0;
	timeout.tv_usec = 0;
	while ( isEnabled() && (select(1, &fdset, 0, 0, &timeout ) == 1) ) {
		if ( read(0, &ch, 1) <= 0 ) {
			break;
		}
		switch (ch) {
		    case toctrl('A'):
			backward_bol();
			break;
		    case toctrl('E'):
			forward_eol();
			break;
		    case toctrl('B'):
			backward_char();
			break;
		    case toctrl('F'):
			forward_char();
			break;
		    case '\0177':
		    case toctrl('H'):
			backspace_char();
			break;
		    case toctrl('D'):
			delete_char();
			break;
		    case toctrl('K'):
			delete_eol();
			break;
		    case toctrl('U'):
			delete_line();
			break;
		    case toctrl('P'):
			prev_history();
			break;
		    case toctrl('N'):
			next_history();
			break;
		    case toctrl('I'):
			tab_complete();
			break;
		    case '\r':
		    case '\n':
			// Execute command
			if ( curpos > 0 ) {
				S32 eventSize;

				add_history(line);
				incr_history();
				eventSize = ConsoleEventHeaderSize;
				dStrcpy( postEvent.data, line );
				postEvent.size = eventSize + dStrlen(line) + 1;
				Game->postEvent( postEvent );
				delete_line();
			}
			break;

                    // Add character to line
		    default:
		    {
			if ( (ch == '\033') && (arrow_state == AT_RESET) ) {
				arrow_state = AT_ESCAPE;
				break;
			} else
			if ( (ch == '[') && (arrow_state == AT_ESCAPE) ) {
				arrow_state = AT_BRACKET;
				break;
			} else if ( arrow_state == AT_BRACKET ) {
				switch (ch) {
				    // Cursor up
				    case 'A':
					prev_history();
					break;
				    // Cursor down
				    case 'B':
					next_history();
					break;
				    // Cursor right
				    case 'C':
					forward_char();
					break;
				    // Cursor left
				    case 'D':
					backward_char();
					break;
				    // Unknown escape code
				    default:
					break;
				}
				arrow_state = AT_RESET;
				break;
			}
			arrow_state = AT_RESET;
			if ( isprint(ch) && (curpos < (sizeof(line)-1)) ) {
				insert_char(ch);
			}
		    }
		    break;
		}
		display_prompt();
	}
}
