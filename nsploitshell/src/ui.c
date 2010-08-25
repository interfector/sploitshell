
#define _GNU_SOURCE

#include <string.h>
#include <ui.h>

void
curses_init(void)
{
	initscr();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);

	cdkscreen = initCDKScreen( stdscr );

	initCDKColor();

	refreshScreen();
}

void
curses_end(void)
{
	destroyCDKScreen( cdkscreen );
	endCDK();
}

int
NCMessageBox(int flag, char* text, ...)
{
	
	WINDOW *msg_win, *button;
	int ch;
	int x,y,len;
	int height = 8, width;

	char* msgs[] = { "Information", "Warning", "ERROR", NULL };

	char buf[BUFSIZ];
	va_list va;

	va_start( va, text);

	vsnprintf( buf, BUFSIZ, text, va );

	va_end( va );

	len = strlen( buf );

	if( msgs[flag] != NULL && strlen(msgs[flag]) > len)
		len = strlen(msgs[flag]);

	width = len + 4;

	x = (COLS - width) / 2;
	y = (LINES - height) / 2;

	msg_win = newwin(height, width, y, x);
	box(msg_win, 0 , 0);
	wrefresh(msg_win);

	if(msgs[flag])
		mvprintw( y , x + 2, msgs[flag] );

	mvprintw( y + 2, x + 2, buf );

	button = newwin( 3, 8, y + 4, x + (len / 2) - 2 );
	box(button, 0 , 0);
	wrefresh(button);

	mvprintw( y + 5 , x + (len / 2) + 1, "OK" );

	while((ch = getch()) != 10);

	wborder(button, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wrefresh(button);
	delwin(button);

	wborder(msg_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	wrefresh(msg_win);
	delwin(msg_win);

	return len;
	
/*
	char* bt[] = { "OK" };

	popupDialog( cdkscreen, &text, 1, bt, 1 );
*/
}

char*
getCDKAssembly( )
{
	CDKMENTRY* widget;
	int size,i;
	char* code;
	char* line;

	widget = newCDKMentry( cdkscreen, CENTER, CENTER, 
					   "Assembly code in C format ( \\x?? )\n",
					   "", A_BOLD, '.', vMIXED,
					   40, 20, 20, 0, 1, 0);

	refreshScreen();

	activateCDKMentry( widget, 0 );

	line = strdup( widget->info );

	destroyCDKMentry( widget );

	refreshScreen();

	size = strlen(line) / 4;

	code = malloc( size );

	for(i = 0;i < size;i++)
		code[i] = strtol( strndup( line + (i * 4) + 2, 2 ), NULL, 16 );

	free( line );

	return code;
}

int
setCDKPref( )
{
	return 0;
}

