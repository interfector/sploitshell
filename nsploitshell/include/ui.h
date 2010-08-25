#ifndef _SPLUA_UI_

#define _SPLUA_UI_

#include <cdk/cdk.h>
#include <ncurses.h>
#include <stdarg.h>

#define MSG_INFO	0
#define MSG_WARN	1
#define MSG_ERR	2

void curses_init(void) __attribute__((constructor));
void curses_end(void)  __attribute__((destructor));

int NCMessageBox( int, char*, ... ); /* flag, text */
char* getCDKAssembly( void );

#define ui_error( ... ) NCMessageBox(MSG_ERR, __VA_ARGS__ )

typedef enum { FILE_OPEN,
			ID_NFOUND,
			ID_MISS,
			ARG_MISS,
			SHPATH_NFOUND,
			LOAD_ERR
		   } errors_t;

static char*  msg_error[] =  { 
	"Error opening the file", 
	"ID not found", 
	"ID missing", 
	"Argument missing", 
	"Shellcode path not found", 
	"Error loading the session file"
};

#define refreshScreen() refreshCDKScreen( cdkscreen )

CDKSCREEN *cdkscreen;

#endif /* _SPLUA_UI_ */
