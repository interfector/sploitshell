/*
* sploitshell, an exploit maker shell
* Copyright (C) 2010 nex
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sploitshell.h>

char* prompts[] = {
	"(\033[31mshellsploit\033[0m)-$ ",
	"(\033[1;32msploit\033[1;31m@\033[1;32mshell\033[0m) % ",
	"(\033[1;34mexploit-shell\033[0m) >> ",
	"\033[4msplua\033[0m exploit( %s ) >> "
};

int now_prompt;

char*
current_prompt(int r)
{
	char* line;

	now_prompt = r;

	if(strcmp(config[5].value,"random"))
	{
		line = malloc( strlen(config[5].value) + 25 );

		if(session_opened)
			sprintf(line, config[5].value, session_opened );
		else
			sprintf(line, config[5].value, "current" );

		return line;
	} else {
		if( r == 3 )
		{
			line = malloc( strlen(prompts[ r ]) + 10 );

			if(session_opened)
				sprintf(line, prompts[ r ], session_opened );
			else
				sprintf(line, prompts[ r ], "current" );

			return line;
		}

		return prompts[ r ];
	}
}

void
resource_file( char* file )
{
	char* home = getenv("HOME");
	char* path;

	if( file )
	{
		config[2].value = xstrdup( file );
		load( NULL );
	}

	if(strcmp(config[4].value, "$HOME/.splua"))
		path = xstrdup( config[4].value );
	else {
		path = malloc( strlen(home) + 10 );
		sprintf(path, "%s/.splua/", home);
	}

	if(access(path, F_OK))
		printf("\033[31m* Shellcode's path not found.\033[0m\n");

	free(path);
}

void
hexDump(unsigned char* buf, int dlen)
{
	char	c[17];
	int	i, ct;

	for (i = 0; i < dlen; ++i)
	{
		if(i != 0 && (i % 2) == 0)
			putchar(' ');

		if (i != 0 && (i % 16) == 0)
		{
			c[16] = '\0';

			printf("  %s\n", c);
		}

		ct = buf[i] & 0xff;

		c[i % 16] = (ct >= ' ' && ct <= '~') ? ct : '.';

		printf("%02x", ct);
	}

	c[i % 16] = '\0';

	for (; i % 16; ++i)
		printf("   ");

	printf("\t%s\n", c);
}

void
init()
{
	rl_completion_entry_function = generator;

	srand(time(NULL));

	sploit_env = malloc( (ENV_BLOCK * sizeof(struct sploitVar)) );
}

void
banner(void)
{
	printf(
		"         _     _ _      _        _ _ \n"
		" ____ __| |___(_) |_ __| |_  ___| | |\n"
		"(_-< '_ \\ / _ \\ |  _(_-< ' \\/ -_) | |\n"
		"/__/ .__/_\\___/_|\\__/__/_||_\\___|_|_|\n"
		"   |_|                               \n\n");
}

char*
generator(const char* text, int state)
{
	static int list_index = 0, len;
	char *name;

	if (!state)
	{
     	list_index = 0;
     	len = strlen (text);
	}

	while ((name = commands[list_index]))
	{
     	list_index++;

     	if (strncmp (name, text, len) == 0)
          	return xstrdup(name);
	}

	return NULL;
}

char*
xstrdup(char* s)
{
	char *r;

	r = xmalloc ((strlen (s) + 1));
	strcpy(r, s);

	return r;
}

void*
xmalloc(int size)
{
	void *buf;

	buf = malloc (size);

	if (!buf)
	{
		fprintf (stderr, "[ERROR] Out of memory.\n");
     	exit (1);
	}

	return buf;
}

int
lineParse(char* line, shCtx* ctx)
{
	int i;
	int core_size = sizeof(core) / sizeof(sploitCtx);
	char* ptr;

	ptr = strtok(line," ");

	ctx->argc = 0;
	ctx->args = NULL;

	if(!ptr)
	{
		ctx->argc = 1;
		ctx->args = xmalloc( sizeof(char*) );
		ctx->args[0] = xstrdup( line );
	} else {

		if(!ctx->args)
			ctx->args = xmalloc( (CTX_BLOCK * sizeof(char*)) + 1);

		ctx->argc = 1;
		ctx->args[ctx->argc - 1] = xstrdup( ptr );

		while((ptr = strtok(NULL, " ")))
		{
			if( !(ctx->argc % CTX_BLOCK) )
				ctx->args = realloc( ctx->args, (CTX_BLOCK * ctx->argc));

			ctx->args[ctx->argc++] = xstrdup( ptr );
		}
	}

	for(i = 0;i < core_size;i++)
		if(!strcmp(core[i].name, ctx->args[0]))
			return core[i].exec( ctx );

	return -1;
}

void
sploitAddVar( struct sploitVar var )
{
	if(!(env_cur % ENV_BLOCK))
		sploit_env = realloc( sploit_env, (ENV_BLOCK + env_cur) * sizeof(struct sploitVar) );

	var.id = env_cur;
	sploit_env[ env_cur++ ] = var;
}

void
sploitPrint( struct sploitVar var )
{
	int i,len = 0;

	char* msgs[] = { "#%d NOP sled, %d bytes",
				  "#%d JMP %d bytes, to #%d",
				  "#%d JMP %d bytes, to 0x%x",
				  "#%d EIP,  0x%x",
				  "#%d Shellcode, %d bytes",
				  "#%d Assembly code, %d bytes" };

	char* message;

	switch(var.type)
	{
		case sploit_nopsled:
			message = malloc(strlen(msgs[0]) + 10);
			sprintf(message, msgs[0], var.id, strlen(var.data));

			len = strlen(message);
			break;
		case sploit_jump:
			message = malloc(strlen(msgs[1]) + 25);

			if( env_cur > var.addr )
				sprintf(message, msgs[1], var.id, strlen(var.data), var.addr);
			else
				sprintf(message, msgs[2], var.id, strlen(var.data), var.addr);

			if(var.set == 0)
				sprintf(message, "#%d JMP , NULL for now", var.id );

			len = strlen(message);
			break;
		case sploit_eip:
			message = malloc(strlen(msgs[3]) + 15);
			sprintf(message, msgs[3], var.id, var.addr);

			len = strlen(message);
			break;
		case sploit_shellcode:
			message = malloc(strlen(msgs[4]) + 10);
			sprintf(message, msgs[4], var.id, var.addr);

			len = strlen(message);
			break;
		case sploit_assembly:
			message = malloc(strlen(msgs[5]) + 15);
			sprintf(message, msgs[5], var.id, var.addr);

			len = strlen(message);
			break;
		default:
			return;
			break;
	}

	for(i = 0;i < len + 6;i++)
		putchar('#');

	printf("\n#");

	for(i = 0;i < len + 4;i++)
		putchar(' ');

	puts("#");

	printf("#  %s  #\n", message );

	putchar('#');

	for(i = 0;i < len + 4;i++)
		putchar(' ');

	puts("#");

	for(i = 0;i < len + 6;i++)
		putchar('#');

	putchar('\n');

	free(message);
}

int
menu( char** choices, int size )
{
	int i;
	char line[BUFSIZ];

	while(1)
	{
		for(i = 0;i < size;i++)
			printf("%d) %s\n", i + 1, choices[i] );

		if(!fgets(line, BUFSIZ, stdin))
			return -1;

		sscanf( line, "%d", &i );

		if( i >= 1 && i <= size )
			return i;
	}

	return -1;
}

int
getLen( char* text )
{
	int i;
	char line[BUFSIZ];

	printf( "%s", text );

	if(!fgets(line, BUFSIZ, stdin))
		return -1;

	sscanf( line, "%d", &i );

	return i;
}

char*
getLine(char *text)
{
	char* line = malloc( 256 );

	printf( "%s", text);

	if(!fgets( line, 256, stdin ))
	{
		printf("* Error reading from stdin.\n");

		return NULL;
	}

	line[strlen(line)-1] = 0;

	return line;
}

int
help(shCtx* dummy)
{
	puts(HELP_MSG);

	return 0;
}

int
nopsled(shCtx* ctx)
{
	char string[] = "@CABHKIJ";
	char* choices[] = { "Random string", "Given byte" };
	int choice, i, len;

	struct sploitVar sled = { 0, sploit_nopsled, NULL, 0, 0 };

	choice = menu( choices, 2 );

	if( choice < 0 )
		return -1;

	if( choice == 1 )
	{
		len = getLen("Lenght of sled: ");

		sled.data = malloc( len + 1);

		for(i = 0;i < len;i++)
			sled.data[i] = string[ rand() % 8 ];

	} else if (choice == 2) {
		len = getLen("Lenght of sled: ");

		sled.data = malloc( len + 1);

		char c;
		char* line = (char*)getLine("Given byte: ");

		if(strlen(line) > 2 && !strncmp(line,"0x",2))
			c = strtol( line, NULL, 16 );
		else
			c = line[0];

		memset( sled.data, c, len );
	}

	sploitAddVar( sled );

	return 0;
}

int
save(shCtx* ctx)
{
	int i = 0, max = -1;
	DIR* dir;
	struct dirent * s_dir;

	char* pathname;
	FILE* fp;

	int dummy;

	if(!strcmp(config[1].value,"default"))
	{

		if(!(dir = opendir(".")))
			pdie("opendir");

		while((s_dir = readdir(dir)))
		{
			if(!strcmp(s_dir->d_name,".") || !strcmp(s_dir->d_name,".."))
				continue;

			if(!strncmp(s_dir->d_name, "session", 7))
				i = atoi( xstrdup( s_dir->d_name + 8 ) );

			if( i > max )
				max = i;
		}

		max++;

		closedir(dir);

		pathname = malloc(15);
		sprintf(pathname, "session.%d", max);
	} else
		pathname = xstrdup(config[1].value);

	if(!(fp = fopen(pathname, "wb")))
	{
		fprintf(stderr,"* Error opening the file.\n");

		return 1;
	}

	dummy = fwrite( &env_cur, sizeof(int), 1, fp);

	for(i = 0;i < (sizeof(config) / sizeof(struct envp));i++)
		dummy = fwrite( &config[i], sizeof(struct envp), 1, fp);

	for(i = 0;i < env_cur;i++)
			dummy = fwrite( &sploit_env[i], sizeof(struct sploitVar), 1, fp);

	for(i = 0;i < (sizeof(config) / sizeof(struct envp));i++)
	{
		dummy = fwrite( config[i].name, strlen(config[i].name) , 1, fp);
		fputc( 0, fp);
	}

	for(i = 0;i < (sizeof(config) / sizeof(struct envp));i++)
	{
		dummy = fwrite( config[i].value, strlen(config[i].value) , 1, fp);
		fputc( 0, fp);
	}

	for(i = 0;i < env_cur;i++)
	{
		if(sploit_env[i].id != -1)
		{
			if(sploit_env[i].type == sploit_assembly)
				dummy = fwrite( sploit_env[i].data, sploit_env[i].addr , 1, fp);
			else
				dummy = fwrite( sploit_env[i].data, strlen(sploit_env[i].data) , 1, fp);
			fputc( 0, fp);
		}
	}

	fclose( fp );

	printf("* Session saved at \'%s\'.\n", pathname );

	free(pathname);

	pathname = xstrdup(".session.last");

	if(!(fp = fopen(pathname, "wb")))
	{
		fprintf(stderr,"* Error opening the file.\n");
		return 1;
	}

	dummy = fwrite( &env_cur, sizeof(int), 1, fp);

	for(i = 0;i < (sizeof(config) / sizeof(struct envp));i++)
		dummy = fwrite( &config[i], sizeof(struct envp), 1, fp);

	for(i = 0;i < env_cur;i++)
			dummy = fwrite( &sploit_env[i], sizeof(struct sploitVar), 1, fp);

	for(i = 0;i < (sizeof(config) / sizeof(struct envp));i++)
	{
		dummy = fwrite( config[i].name, strlen(config[i].name) , 1, fp);
		fputc( 0, fp);
	}

	for(i = 0;i < (sizeof(config) / sizeof(struct envp));i++)
	{
		dummy = fwrite( config[i].value, strlen(config[i].value) , 1, fp);
		fputc( 0, fp);
	}

	for(i = 0;i < env_cur;i++)
	{
		if(sploit_env[i].id != -1)
		{
			if(sploit_env[i].type == sploit_assembly)
				dummy = fwrite( sploit_env[i].data, sploit_env[i].addr , 1, fp);
			else
				dummy = fwrite( sploit_env[i].data, strlen(sploit_env[i].data) , 1, fp);
			fputc( 0, fp);
		}
	}

	fclose( fp );

	return 0;
}

int
set(shCtx* ctx)
{
	int i,j,size = 0;
	int conf_size = sizeof(config) / sizeof(struct envp);
	char* tmp;

	if(ctx->argc < 3)
	{
		fprintf(stderr, "usage: set <name> <value>\n"
					 "where \'name\' is the key of the configuration to change and\n"
					 "where \'value\' is the new value to assign.\n"
				);

		return 1;
	}

	for(i = 0;i < conf_size;i++)
	{
		if(!strcmp( ctx->args[1], config[i].name))
		{
			if(ctx->argc > 3)
			{
				for(j = 3;j < ctx->argc;j++)
					size += strlen(ctx->args[j]);

				tmp = malloc( size );
				sprintf(tmp, "%s ", ctx->args[2] );

				for(j = 3;j < ctx->argc;j++)
				{
					strcat( tmp, ctx->args[j] );
					strcat( tmp, " ");
				}

				ctx->args[2] = xstrdup( tmp );

				free( tmp );
			}

			if(!strcmp(ctx->args[1],"prompt") && !strcmp(ctx->args[2],"current"))
				ctx->args[2] = xstrdup( prompts[now_prompt] );

			config[i].value = xstrdup( ctx->args[2] );
		}
	}

	return 0;
}

int
show(shCtx* ctx)
{
	int i;
	int conf_size = sizeof(config) / sizeof(struct envp);

	if(ctx->argc < 2)
	{
		fprintf(stderr, "usage: show <arg>, where \'arg\' is:\n"
					 "config  - To show the configuration of the session.\n"
					 "exploit - To show the current scheme of the exploit.\n"
					 "id      - To show the content of an ID.\n"
				);

		return 1;
	}

	if(!strcmp( ctx->args[1], "config" ))
	{
		for(i = 0;i < conf_size;i++)
			printf("%s    =>    \"%s\"\n", config[i].name, config[i].value);
	} else if(!strcmp( ctx->args[1], "exploit" ))
	{
		for(i = 0;i < env_cur;i++)
			if(sploit_env[i].id != -1)
				sploitPrint( sploit_env[i] );
	} else if(!strcmp( ctx->args[1], "id" ))
	{
		int size,id;

		if(!ctx->args[2])
		{
			printf("* ID missing.\n");

			return 1;
		}

		id = atoi(ctx->args[2]);

		if( id < 0 || env_cur <= id || sploit_env[id].id == -1)
		{
			printf("* ID not found.\n");

			return 1;
		}

		size = strlen(sploit_env[id].data);

		if(sploit_env[id].type == sploit_assembly || sploit_env[id].type == sploit_shellcode)
			size = sploit_env[id].addr;

		if( sploit_env[id].type == sploit_eip )
			size = 4;

		hexDump( (unsigned char*)sploit_env[id].data, size );
	}

	return 0;
}

int
load(shCtx* ctx)
{
	int i,len;
	FILE* fp;
	struct sploitVar var = { 0, 0, NULL, 0 };
	char *buf;
	long cur,end;

	int dummy;

	if(!(fp = fopen(config[2].value, "rb")))
	{
		fprintf(stderr,"* Error opening the file.\n");

		return 1;
	}

	session_opened = xstrdup( config[2].value );

	dummy = fread( &len, sizeof(int), 1, fp);

	for(i = 0;i < (sizeof(config) / sizeof(struct envp));i++)
		dummy = fread(&config[i], sizeof(struct envp), 1, fp);

	for(i = 0;i < len;i++)
	{
		dummy = fread(&var, sizeof(struct sploitVar), 1, fp);

		if(var.id == -1)
			continue;

		var.data = 0;
		sploitAddVar( var );
	}

	cur = ftell( fp );
	fseek( fp, 0, SEEK_END );
	end = ftell( fp );

	end -= cur;

	fseek( fp, cur, SEEK_SET );

	buf = malloc( end );

	dummy = fread( buf, end, 1, fp);

	for(i = 0;i < (sizeof(config) / sizeof(struct envp));i++)
	{
		config[i].name = xstrdup( buf );
		buf = (strchr( buf, '\0' )) + 1;
	}

	for(i = 0;i < (sizeof(config) / sizeof(struct envp));i++)
	{
		config[i].value = xstrdup( buf );
		buf = (strchr( buf, '\0' )) + 1;
	}

	for(i = 0;i < env_cur;i++)
	{
		sploit_env[i].data = xstrdup(buf);
		buf = (strchr( buf, '\0' )) + 1;
	}

	fclose( fp );

	printf("* Session load.\n");

	config[1].value = xstrdup( session_opened );

	return 0;
}

int
jump(shCtx* ctx)
{
	char* line;
	char* choices[] = { "Given ID", "Given address", "NULL for now" };

	int choice = menu( choices, 3 );

	struct sploitVar jump = { 0, sploit_jump, "", 0, 1 };

	switch(choice)
	{
		case 1:
			jump.addr = getLen("ID: ");

			if(jump.addr > env_cur)
			{
				printf("* Error, no ID found.\n");

				jump.set = 0;
			}
			break;
		case 2:
			line = getLine("Address: ");

			sscanf( line, "0x%x", &jump.addr );
			break;
		case 3:
			jump.set = 0;
			break;
		default:
			jump.set = 0;
			break;
	}

	jump.data = xstrdup("\xeb\xff");

	sploitAddVar( jump );

	return 0;
}

int
assembly(shCtx* ctx)
{
	char* choices[] = { "From file", "From stdin in C format ( \\x?? )" };
	int choice = menu( choices, 2 );
	FILE* fp;
	char* path;

	int size,dummy;

	struct sploitVar assembly = { 0, sploit_assembly, NULL, 0, 0 };

	if(choice == 1)
	{
		path = getLine("Path: ");

		if(!(fp = fopen(path, "r")))
		{
			fprintf(stderr, "* Error opening the file.\n");

			free( path );

			return 1;
		}

		free( path );

		fseek( fp, 0, SEEK_END );
		size = (int) ftell( fp );
		fseek( fp, 0, SEEK_SET );

		assembly.data = malloc( size );
		assembly.addr = size;

		dummy = fread( assembly.data, size, 1, fp );
	} else {
		path = getLine("Assembly in C format ( \\x?? )\n");

		size = strlen(path) / 4;

		assembly.data = malloc( size );
		assembly.addr = size;

		for(dummy = 0;dummy < size;dummy++)
			assembly.data[dummy] = strtol( strndup( path + (dummy * 4) + 2, 2 ), NULL, 16);
	}

	sploitAddVar( assembly );

	return 0;
}

int
generate(shCtx* ctx)
{
	int i,j,dummy,size;
	FILE* fp;

	if(!(fp = fopen(config[3].value, "w" )))
	{
		fprintf(stderr, "* Error opening the file.\n");

		return 1;
	}

	for(i = 0;i < env_cur;i++)
	{
		if(!strcmp(config[0].value, "raw"))
		{
			if(sploit_env[i].type == sploit_assembly )
				dummy = fwrite( sploit_env[i].data, sploit_env[i].addr, 1, fp );
			else if (sploit_env[i].type == sploit_eip )
				dummy = fwrite( &sploit_env[i].addr, sizeof(unsigned int), 1, fp );
			else
				dummy = fwrite( sploit_env[i].data, strlen(sploit_env[i].data), 1, fp );
		} else {
			size = strlen(sploit_env[i].data);

			if(sploit_env[i].type == sploit_assembly )
				size = sploit_env[i].addr;

			if( sploit_env[i].type == sploit_eip )
			{
				sploit_env[i].data = malloc( sizeof(unsigned int) );
				memcpy( sploit_env[i].data, &sploit_env[i].addr, sizeof(unsigned int) );

				size = sizeof(unsigned int);
			}

			for( j = 0;j < size;j++ )
				fprintf(fp, "\\x%.2x", (unsigned char) sploit_env[i].data[j] );
		}
	}

	fclose( fp );

	printf("* Exploit generated with success.\n");

	return 0;
}

int
delete(shCtx* ctx)
{
	int id;

	if(ctx->argc < 2)
	{
		printf("* ID missing.\n");

		return 1;
	}

	id = atoi(ctx->args[1]);

	if( id < 0 || env_cur <= id || sploit_env[id].id == -1)
	{
		printf("* ID not found.\n");

		return 1;
	}

	sploit_env[id].id = -1;

	if(sploit_env[id].type != sploit_eip)
		free( sploit_env[id].data );

	sploit_env[id].type = 0;
	sploit_env[id].addr = 0;
	sploit_env[id].set = -1;

	for(; id < env_cur;id++)
		if( sploit_env[id].type == sploit_jump )
			sploit_env[id].addr--;

	return 0;
}

int
move(shCtx* ctx)
{
	int id,id2,i;

	struct sploitVar tmp;

	if(ctx->argc < 3)
	{
		printf("IDs missing.\n");

		return 1;
	}

	id = atoi(ctx->args[1]);
	id2 = atoi(ctx->args[2]);

	if( id < 0 || id2 < 0 || (env_cur <= id || sploit_env[id].id == -1) || (env_cur <= id2 || sploit_env[id].id == -1))
	{
		printf("* ID not found.\n");

		return 1;
	}
/*
	for(i = 0;i < env_cur;i++)
	{
		if( sploit_env[i].type == sploit_jump )
		{
			if( sploit_env[i].addr == id2 )
				sploit_env[i].addr = id;
			else if ( sploit_env[i].addr == id )
				sploit_env[i].addr = id2;
		}
	}
*/
	memcpy( &tmp, &sploit_env[id], sizeof(struct sploitVar));

	memcpy( &sploit_env[id], &sploit_env[id2], sizeof(struct sploitVar));

	sploit_env[id].id = id;

	memcpy( &sploit_env[id2], &tmp, sizeof(struct sploitVar));

	sploit_env[id2].id = id2;

	return 0;
}

int
modify(shCtx* ctx)
{
	char string[] = "@CABHKIJ";
	int i,id,choice,len;

	char* nop_ch[] = { "Random string", "Given byte" };
	char* asm_ch[] = { "From file", "From stdin in C format ( \\x?? )" };
	char* jmp_ch[] = { "Given ID", "Given address", "NULL	for now" };

	if(ctx->argc < 2)
	{
		printf("* ID missing.\n");

		return 1;
	}

	id = atoi(ctx->args[1]);

	if( id < 0 || env_cur <= id || sploit_env[id].id == -1)
	{
		printf("* ID not found.\n");

		return 1;
	}

	switch(sploit_env[id].type)
	{
		case sploit_nopsled:
			choice = menu( nop_ch, 2 );

			len = getLen("Lenght of sled: ");

			free( sploit_env[id].data );

			sploit_env[id].data = malloc( len + 1);
			memset( sploit_env[id].data, 0, len + 1);

			if(choice == 1)
			{
				for(i = 0;i < len;i++)
					sploit_env[id].data[i] = string[ rand() % 8 ];
			} else {
				char* line = (char*)getLine("Given byte: ");
				char c;

				if(strlen(line) > 2 && !strncmp(line,"0x",2))
					c = strtol( line, NULL, 16 );
				else
					c = line[0];

				memset( sploit_env[id].data, c, len );
			}

			break;
		case sploit_assembly:
			choice = menu( asm_ch, 2 );

			if(choice == 1)
			{
				FILE* fp;
				long size;
				char* path = getLine("File path: ");

				if(!(fp = fopen(path,"r")))
				{
					fprintf(stderr, "* Error opening the file.\n");

					return 1;
				}

				fseek( fp, 0, SEEK_END);
				size = ftell( fp );
				fseek( fp, 0, SEEK_SET);

				free( sploit_env[id].data );

				sploit_env[id].data = malloc( size + 1);
				sploit_env[id].addr = size;

				memset( sploit_env[id].data, 0, size + 1);

				len = fread( sploit_env[id].data, size, 1, fp);

				fclose( fp );
			} else {
				int size;
				char* line = getLine("Assembly in C format ( \\x?? )\n");

				size = strlen(line) / 4;

				free( sploit_env[id].data );

				sploit_env[id].data = malloc( size + 1);
				sploit_env[id].addr = size;

				memset( sploit_env[id].data, 0, size + 1);

				for(i = 0;i < size;i++)
					sploit_env[id].data[i] = strtol( strndup( line + (i * 4) + 2, 2 ), NULL, 16);
			}

			break;
		case sploit_jump:
			choice = menu( jmp_ch, 3 );

			sploit_env[id].set = 1;

			if(choice == 1)
			{
				sploit_env[id].addr = getLen("ID: ");

				if(sploit_env[id].addr > env_cur)
				{
					printf("* Error, no ID found.\n");

					sploit_env[id].set = 0;
				}
			} else if (choice == 2) {
				char* addr = getLine("Address: ");

				sscanf(addr, "0x%x", &sploit_env[id].addr);
				break;
			} else {
				sploit_env[id].set = 0;
			}

			break;
		case sploit_shellcode:
			shellcode( NULL );

			free( sploit_env[id].data );

			sploit_env[id].data = xstrdup( sploit_env[env_cur - 1].data );
			sploit_env[id].addr = sploit_env[env_cur - 1].addr;

			env_cur--;
			break;
		default:	
			break;
	}

	return 0;
}

int
shellcode(shCtx* ctx)
{
	char* shellpath;
	DIR* dp;
	FILE* fp;

	int i;
	long size;

	char* os_ch[] = { "unix", "win32" };
	char* mode_ch[] = { "remote", "local" };

	int os_c, mode_c;

	struct dirent *dir;

	char* home = getenv("HOME");

	char** shellcode_ch;
	int shellcode_size = 0;

	int shell_choice;

	char buf[BUFSIZ] = { 0 };

	struct sploitVar shellcode = { 0, sploit_shellcode, NULL, 0, 0 };

	os_c = menu( os_ch, sizeof(os_ch) / sizeof(os_ch[0]) );
	mode_c = menu( mode_ch, sizeof(mode_ch) / sizeof(mode_ch[0]) );

	if(strcmp(config[4].value, "$HOME/.splua"))
	{
		shellpath = malloc( strlen(config[4].value) + strlen(os_ch[os_c - 1]) + strlen(mode_ch[mode_c - 1]) + 5 );
		sprintf( shellpath, "%s/%s/%s/", config[4].value, os_ch[os_c - 1], mode_ch[mode_c - 1] );
	} else {
		shellpath = malloc( strlen(home) + strlen(os_ch[os_c - 1]) + strlen(mode_ch[mode_c - 1]) + 15 );
		sprintf( shellpath, "%s/.splua/%s/%s/", home, os_ch[os_c - 1], mode_ch[mode_c - 1] );
	}

	if(!(dp = opendir( shellpath )))
	{
		fprintf(stderr, "* Error opening the directory.\n");

		return 1;
	}

	shellcode_ch = malloc( sizeof(char*) * ENV_BLOCK );

	while(( dir = readdir( dp )))
	{
		if(!strcmp( dir->d_name, "." ) || !strcmp( dir->d_name, ".."))
			continue;

		if(!(shellcode_size % ENV_BLOCK))
			shellcode_ch = realloc( shellcode_ch, sizeof(char*) * (shellcode_size + ENV_BLOCK) );

		shellcode_ch[ shellcode_size++ ] = xstrdup( dir->d_name );
	}

	closedir( dp );

	shell_choice = menu( shellcode_ch, shellcode_size );

	sprintf( buf, "%s/%s", shellpath, shellcode_ch[ shell_choice - 1 ] );

	free( shellcode_ch );
	free( shellpath );

	if(!(fp = fopen( buf, "r" )))
	{
		fprintf(stderr, "* Error opening the file.\n");

		return 1;
	}

	fseek( fp, 0, SEEK_END );
	size = ftell( fp );
	fseek( fp, 0, SEEK_SET );

	memset( buf, 0, BUFSIZ );

	shell_choice = fread( buf, size, 1, fp );

	shellpath = strchr(buf, '\n') + 1;

	shell_choice = write( 1, buf, (int)(shellpath - buf));

	size = (size - (int)(shellpath - buf)) / 4;

	shellcode.data = malloc( size + 1 );

	for(i = 0;i < size;i++)
		shellcode.data[i] = strtol( strndup( shellpath + (i * 4) + 2, 2), NULL, 16 );

	shellcode.addr = size;

	sploitAddVar( shellcode );

	return 1;
}

int
eip(shCtx* ctx)
{
	unsigned int addr;
	struct sploitVar eip = { 0, sploit_eip, NULL, 0, 0 };

	if( ctx->argc < 2 )
	{
		printf("* Address missing.\n");

		return 1;
	}

	sscanf( ctx->args[1], "0x%x", &addr );

	eip.data = malloc( 4 );

	memcpy( eip.data, &addr, 4 );

	eip.addr = addr;

	sploitAddVar( eip );

	return 0;
}
