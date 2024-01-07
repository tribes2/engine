//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _LINUXCONSOLE_H_
#define _LINUECONSOLE_H_

#ifndef _CONSOLE_H_
#include "console/console.h"
#endif
#ifndef _EVENT_H_
#include "Platform/event.h"
#endif

#define NUM_HISTORY 32

class LinuxConsole
{
	ConsoleEvent postEvent;
	bool console_enabled;
	bool split_window;
	int rows, cols;
	int curpos;
	char line[1024];
	char *history[NUM_HISTORY];
	int current_history;

	void set_title(const char *title);
	void move_bol(void);
	void clear_eol(void);
	void clear_line(void);
	void move_to(int row, int col);
	void move_back(void);
	void set_scroll(int hi, int lo, int col);

	void forward_char(void);
	void backward_char(void);
	void forward_eol(void);
	void backward_bol(void);
	void backspace_char(void);
	void delete_char(void);
	void delete_eol(void);
	void delete_line(void);

	void insert_char(char ch);
	void replace_char(char ch);
	void replace_line(const char *text);

	void init_history(void);
	void add_history(const char *text);
	void incr_history(void);
	void decr_history(void);
	void prev_history(void);
	void next_history(void);
	void free_history(void);

	void tab_complete(void);

	void display_prompt(void);

public:
	LinuxConsole( void );
	~LinuxConsole( void );

	void enable( bool enabled );
	bool isEnabled( void ) { return console_enabled; };
	void process( void );
	void processConsoleLine( ConsoleLogEntry::Level level, const char* line );
	static void create( );
	static void destroy( );

	void check_winsize(void);
};

extern LinuxConsole* linuxConsole;

#endif
