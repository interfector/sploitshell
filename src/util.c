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
		if( !load( NULL ))
			session_opened = xstrdup( file );
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
				  "#%d EIP  0x%x",
				  "#%d Payload, %d bytes",
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
			sprintf(message, msgs[4], var.id, strlen(var.data));

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
	printf( "%s", text);

	return fgets( malloc( 256 ), 256, stdin );
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
	char* choices[] = { "Random string", "Given string" };
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

		char c = *(char*)getLine("Given byte: ");

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
		if(sploit_env[i].type == sploit_assembly)
			dummy = fwrite( sploit_env[i].data, sploit_env[i].addr , 1, fp);
		else
			dummy = fwrite( sploit_env[i].data, strlen(sploit_env[i].data) , 1, fp);
		fputc( 0, fp);
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
		if(sploit_env[i].type == sploit_assembly)
			dummy = fwrite( sploit_env[i].data, sploit_env[i].addr , 1, fp);
		else
			dummy = fwrite( sploit_env[i].data, strlen(sploit_env[i].data) , 1, fp);
		fputc( 0, fp);
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

		if( env_cur < id )
		{
			printf("* ID not found.\n");

			return 1;
		}

		size = strlen(sploit_env[id].data);

		if(sploit_env[id].type == sploit_assembly)
			size = sploit_env[id].addr;

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

	config[1].value = xstrdup( config[2].value );

	dummy = fread( &len, sizeof(int), 1, fp);

	for(i = 0;i < (sizeof(config) / sizeof(struct envp));i++)
		dummy = fread(&config[i], sizeof(struct envp), 1, fp);

	for(i = 0;i < len;i++)
	{
		dummy = fread(&var, sizeof(struct sploitVar), 1, fp);

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
			jump.addr = getLen("ID:");

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
