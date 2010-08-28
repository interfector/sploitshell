
#define _GNU_SOURCE

#include <stdlib.h>
#include <string.h>
#include <sploitshell.h>
#include <ui.h>

char *menulist[MAX_MENU_ITEMS][MAX_SUB_ITEMS] = { { "File", "Open session", "Save session", "------------", "Exit" },
										{ "Edit", "Insert NOP sled", "Insert EIP", "Insert Assembly code",
											"Insert shellcode", "Insert jump", "--------------", "Configuration" },
										{ "View", "Exploit scheme", "Generated shellcode" },
										{ "Exploit", "Connect to & Exec", "Generate & Save" },
										{ "About", "About" } };
int submenusize[5] = { 5, 8, 3, 3, 2 };
int menuloc[5] = { LEFT, LEFT, LEFT, LEFT, RIGHT };

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

	srand(time(NULL));
	
	sploit_env = malloc( (ENV_BLOCK * sizeof(struct sploitVar)) );
}

void
curses_end(void)
{
	destroyCDKScreen( cdkscreen );
	endCDK();
}

void
initCDKMenu( void* dummy )
{
	CDKMENU* menu;
	int selected,submenu;

	menu = newCDKMenu( cdkscreen, menulist, 5, submenusize, menuloc,
			TOP, A_BOLD, A_REVERSE );

	if(!menu)
	{
		destroyCDKScreen( cdkscreen );
		endCDK();

		printf("Couldn't create CDKMenu...\n");

		exit( 1 );
	}

	while(1)
	{
		drawCDKMenu( menu, 1 );

		refreshScreen();

		selected = activateCDKMenu( menu, 0 );

		if (menu->exitType == vNORMAL )
		{
			submenu = (selected % 100) + 1;

			switch( selected / 100 )
			{
				case 0:
					if(submenu == 1)
						loadCDKSession( );
					else if (submenu == 2 )
						; // saveCDKSession( );
					else if (submenu == 4 )
						exit( 0 );
					break;
				case 1:
					if(submenu == 1)
						getCDKNOPsled( );
					else if (submenu == 2)
						getCDKEip( );
					else if (submenu == 3 )
						getCDKAssembly( );
					else if (submenu == 4 )
						getCDKShellcode( );
					else if (submenu == 5 )
						getCDKJump( );
					else if (submenu == 7 )
						; // setCDKPref( );
					break;
				case 2:
					break;
				case 3:
					break;
				case 4:
					if(submenu == 1)
						aboutCDKShell( );
					break;
				default:
					break;
			}
		}

		eraseCDKMenu( menu );
	}
}

void
aboutCDKShell( )
{
	char* msg[] = { "This is an interactive interface for exploit creation,",
				 "this is the ncurses version of SploitShell and it",
				 "implements the CDK ( Curses Development Kit ) to use",
				 "more ncurses widget.",
				 "This program is released under GPL v3.0 and it's",
				 "developped by nex.",
				 "------------------------------------------------",
				 "For BUGS or something else contact me at",
				 "                                        ",
				 "nex [AT] autistici [DOT] org" };

	popupLabel( cdkscreen, msg, sizeof(msg) / sizeof(msg[0]) );
}

void
resource_file( char* file )
{
	char* home = getenv("HOME");
	char* path;

	if( file )
	{
		config[2].value = strdup( file );
		loadCDKSession( );
	}

	if(strcmp(config[4].value, "$HOME/.splua"))
		path = strdup( config[4].value );
	else {
		path = malloc( strlen(home) + 10 );
		sprintf(path, "%s/.splua/", home);
	}

	if(access(path, F_OK))
		printf("\033[31m* Shellcode's path not found.\033[0m\n");

	free(path);
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

	refreshScreen();

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

	if(msgs[flag])
	{
		memset( buf, 0, BUFSIZ );
		memset( buf, ' ', strlen(msgs[flag]));
		mvprintw( y , x + 2, buf );
	}

	memset( buf, ' ', BUFSIZ );

	mvprintw( y + 2, x + 2, buf );

	refreshScreen();

	return len;
	
/*
	char* bt[] = { "OK" };
	CDKDIALOG* dialog;

	dialog = newCDKDialog( cdkscreen, CENTER, CENTER, &text, 1, bt, 1, A_REVERSE, 1, 1, 0 );

	activateCDKDialog( dialog, 0 );

	refreshScreen();
*/
}

int
getCDKAssembly( )
{
	char* item[] = { "From file", "From stdin in C format ( \\x?? )" };
	CDKRADIO* radio;
	CDKMENTRY* widget;
	int selected;
	int size,i;
	char* line;

	FILE* fp;

	struct sploitVar assembly = { 0, sploit_assembly, NULL, 0, 0 };

	radio = newCDKRadio( cdkscreen, CENTER, CENTER, NONE, 8, 30,
						"Chose assembly typing mode...\n",
						item, sizeof(item) / sizeof(item[0]),
						'@' | A_REVERSE,
						1, A_REVERSE, 1, 0 );

	if(!radio)
	{
		destroyCDKScreen( cdkscreen );
		endCDK();

		printf("Couldn't create CDKRadio...\n");

		exit( 1 );
	}

	selected = activateCDKRadio( radio, 0 );

	if( radio->exitType == vESCAPE_HIT )
	{
		destroyCDKRadio( radio );
		return 1;
	}

	destroyCDKRadio( radio );

	if(selected)
	{
		widget = newCDKMentry( cdkscreen, CENTER, CENTER, 
					   "Assembly code in C format ( \\x?? )\n",
					   "", A_BOLD, '.', vMIXED,
					   40, 20, 20, 0, 1, 0);

		if(!widget)
		{
			destroyCDKScreen( cdkscreen );
			endCDK();

			printf("Couldn't create CDKMentry...\n");

			exit( 1 );
		}

		refreshScreen();

		activateCDKMentry( widget, 0 );

		line = strdup( widget->info );

		destroyCDKMentry( widget );

		refreshScreen();

		size = strlen(line) / 4;

		assembly.data = malloc( size );
		assembly.addr = size;

		for(i = 0;i < size;i++)
			assembly.data[i] = strtol( strndup( line + (i * 4) + 2, 2 ), NULL, 16 );

		free( line );
	} else {
		line = selectFile( cdkscreen, "Select assembly...\n" );

		if(!line)
			return 1;

		if(!(fp = fopen(line,"r")))
		{
			ui_error( msg_error[FILE_OPEN] );

			return 1;
		}

		fseek( fp, 0, SEEK_END );
		assembly.addr = ftell( fp );
		fseek( fp, 0, SEEK_SET );

		assembly.data = malloc( assembly.addr + 1 );
		i = fread( assembly.data, assembly.addr, 1, fp );
	}

	sploitAddVar( assembly );

	return 0;
}

int
setCDKPref( )
{
	return 0;
}

int
getCDKShellcode( )
{
	char path[ BUFSIZ ];
	char* shellpath = "/home/nex/.splua";
	char* sh_name;
	FILE* fp;

	int dummy;
	int size;

	struct sploitVar shellcode = { 0, sploit_shellcode, NULL, 0, 0 };

	if( !getcwd( path, BUFSIZ ) )
	{
		ui_error( "Error getting current directory" );

		return 1;
	}

	if(strcmp(config[4].value,"$HOME/.splua"))
		dummy = chdir( config[4].value );
	else
		dummy = chdir( shellpath );

	sh_name = selectFile( cdkscreen, "Select shellcode\n" );

	if(!sh_name)
		return 1;

	dummy = chdir( path );

	if(!(fp = fopen(sh_name, "r" )))
	{
		ui_error( msg_error[FILE_OPEN] );

		return 1;
	}

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	memset( path, BUFSIZ, 0 );

	dummy = fread( path, size, 1, fp );

	sh_name = strchr( path, '\n' ) + 1;

	shellpath = strndup( path, (int)( sh_name - path ) - 1);

	popupLabel( cdkscreen, &shellpath, 1 );

	free( shellpath );

	size = (size - (int)( sh_name - path ) ) / 4;

	shellcode.data = malloc ( size + 1 );

	for(dummy = 0;dummy < size;dummy++)
		shellcode.data[dummy] = strtol( strndup( sh_name + (dummy * 4) + 2, 2 ), NULL, 16 );

	shellcode.addr = size;

	sploitAddVar( shellcode );

	return 0;
}

int
getCDKNOPsled( )
{
	char* item[] = { "Random string", "Given byte" };
	CDKRADIO* radio;

	int selected = 0,i,size;
	char* string;
	char random[] = "@CABHKIJ";
	struct sploitVar nopsled = { 0, sploit_nopsled, NULL, 0, 0 };

	radio = newCDKRadio( cdkscreen, CENTER, CENTER, NONE, 8, 30,
					"Chose NOP sled type...\n",
					item, sizeof(item) / sizeof(item[0]),
					'@' | A_REVERSE,
					1, A_REVERSE, 1, 0 );

	if(!radio)
	{
		destroyCDKScreen( cdkscreen );
		endCDK();

		printf("Couldn't create CDKRadio...\n");

		exit( 1 );
	}

	selected = activateCDKRadio( radio, 0 );

	if( radio->exitType == vESCAPE_HIT )
	{
		destroyCDKRadio( radio );
		return 1;
	}

	destroyCDKRadio( radio );

	string = getString( cdkscreen, "NOP sled\n", "Lenght: ", "" );

	if(!string)
		return 1;

	refreshScreen();

	for(i = 0;i < strlen(string);i++)
	{
		if( string[i] > '9' || string[i] < '0' )
		{
			ui_error( "Only number accepted!" );

			return 1;
		}
	}

	sscanf( string, "%d", &size );

	nopsled.data = malloc( size + 1 );

	memset( nopsled.data, 0, size + 1 );

	if( selected == 0 )
	{
		for(i = 0;i < size;i++)
			nopsled.data[i] = random[ rand() % 8 ];
	} else {
		char c;
		string = getString( cdkscreen, "NOP sled\n", "Byte: ", "" );

		if(!string || !string[0])
		{
			ui_error( msg_error[EMPTY_STR] );

			return 1;
		}

		if(strlen(string) > 2 && !strncmp(string,"0x",2))
			sscanf(string, "0x%x", (unsigned int*)&c );
		else
			c = string[0];

		memset( nopsled.data, c, size );
	}

	sploitAddVar( nopsled );

	return 0;
}

int
getCDKJump( )
{
	CDKRADIO* radio;
	int selected;
	char* item[] = { "Given ID", "Given address", "NULL for now" };

	char* string;

	struct sploitVar jump = { 0, sploit_jump, NULL, 0, 1 };

	radio = newCDKRadio( cdkscreen, CENTER, CENTER, NONE, 8, 30,
			"Chose jump type...\n", item, sizeof(item) / sizeof(item[0]),
			'@' | A_REVERSE, 1, A_REVERSE, 1, 0 );

	if(!radio)
	{
		destroyCDKScreen( cdkscreen );
		endCDK();

		printf("Couldn't create CDKRadio...\n");

		exit( 1 );
	}

	selected = activateCDKRadio( radio, 0 );

	if( radio->exitType == vESCAPE_HIT )
	{
		destroyCDKRadio( radio );
		return 1;
	}

	destroyCDKRadio( radio );

	switch(selected)
	{
		case 0:
			string = getString( cdkscreen, "Jumps\n", "ID: ", "0" );

			if(!string)
				return 1;

			sscanf( string, "%d", &jump.addr );

			if( jump.addr > env_cur )
			{
				ui_error( msg_error[ID_NFOUND] );

				jump.set = 0;
			}

			break;
		case 1:
			string = getString( cdkscreen, "Jumps\n", "Address: ", "0x00000000" );

			if(!string)
				return 1;

			sscanf( string, "0x%x", &jump.addr );
		case 2:
		default:
			jump.set = 0;
			break;
	}

	jump.data = strdup("\xeb\xff");

	sploitAddVar( jump );

	return 1;
}

int
getCDKEip( )
{
	struct sploitVar eip = { 0, sploit_eip, NULL, 0, 0 };
	char* string;

	string = getString( cdkscreen, "EIP Address...\n", "Address: ", "0x00000000" );

	if(!string)
		return 1;

	sscanf( string, "0x%x", &eip.addr );

	eip.data = malloc( 4 );
	memcpy( eip.data, &eip.addr, sizeof( int ) );

	sploitAddVar( eip );

	return 0;
}

int
loadCDKSession( )
{
	char* file;

	file = selectFile( cdkscreen, "Select session file...\n" );

	if(!file)
		return 1;

	return 0;
}
