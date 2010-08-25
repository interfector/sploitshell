#ifndef _SPLUA_UI_

#define _SPLUA_UI_

#include <ncurses.h>
#include <stdarg.h>

#define MSG_INFO	0
#define MSG_WARN	1
#define MSG_ERR	2

void curses_init(void) __attribute__((constructor));
void curses_end(void)  __attribute__((destructor));

int NCMessageBox( int, char*, ... ); /* flag, text */

#endif /* _SPLUA_UI_ */
