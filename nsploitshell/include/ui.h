#ifndef _SPLUA_UI_

#define _SPLUA_UI_

#include <cdk/cdk.h>
#include <ncurses.h>
#include <stdarg.h>

#define MSG_INFO	0
#define MSG_WARN	1
#define MSG_ERR	2

/* CDK and menu initialization */

void curses_init( void ) __attribute__((constructor));
void curses_end( void )  __attribute__((destructor));
void initCDKMenu( void* );
void aboutCDKShell( void );

/* Session management function */
int loadCDKSession( void );
//int saveCDKSession( void );

/* CDK sploitshell function */

int NCMessageBox( int, char*, ... ); /* flag, text */
int getCDKAssembly( void );
int getCDKNOPsled( void );
int getCDKShellcode( void );
int getCDKJump( void );
int getCDKEip( void );

#define ui_error( ... ) NCMessageBox(MSG_ERR, __VA_ARGS__ )

typedef enum { FILE_OPEN,
			ID_NFOUND,
			ID_MISS,
			ARG_MISS,
			SHPATH_NFOUND,
			LOAD_ERR,
			EMPTY_STR
		   } errors_t;

static char*  msg_error[] =  { 
	"Error opening the file", 
	"ID not found", 
	"ID missing", 
	"Argument missing", 
	"Shellcode path not found", 
	"Error loading the session file",
	"Empty string!"
};

#define refreshScreen() refreshCDKScreen( cdkscreen )

CDKSCREEN *cdkscreen;

#endif /* _SPLUA_UI_ */
