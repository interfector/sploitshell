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


#ifndef _SHSPLOIT_H_
#define _SHSPLOIT_H_

#include <readline/readline.h>
#include <readline/history.h>

static char* prompts[] = {
	"(\033[31mshellsploit\033[0m) ",
	"[\033[1;32msploit\033[1;31m@\033[1;32mshell\033[0m] %> ",
	"(\033[33mexploit-shell\033[0m) >> "
};

#define pdie(x) do{ perror(x); exit(1); }while(1)

char* generator(const char*,int);

void* xmalloc(int);
char* xstrdup(char*);

void init(void) __attribute__((constructor));

static char* commands [] ={ "exit", "generate", "nopsled" , "quit", "eip", "jump", "help", "load", "shellcode", "set", "save", "show" };

#define HELP_MSG "nopsled   - Generate a NOP sled.\n" \
			  "jump      - Set a jump to a specific address.\n" \
			  "eip       - Overwrite EIP to point to a specific address.\n" \
			  "shellcode - Set the shellcode to execute.\n" \
			  "load      - Loads a previous session of sploitshell.\n" \
			  "save      - Save the current sessions of sploitshell.\n" \
			  "generate  - Generate the current exploit.\n" \
			  "set       - Set some environment variables.\n" \
			  "show      - Show the current setting.\n" \
			  "help      - Show this help.\n" \
			  "quit|exit - Exit."

typedef enum { sploit_nopsled, sploit_jump, sploit_eip, sploit_shellcode } sploit_type;

struct envp {
	char* name;
	char* value;
};

struct sploitVar {
	int id;
	sploit_type type;

	char* data;
	unsigned int addr;
};

typedef struct {
	char **args;
	int argc;
} shCtx;

typedef struct {
	char* name;
	int (*exec)(shCtx*);
} sploitCtx;

#define CTX_BLOCK 10
#define ENV_BLOCK 5

static struct envp config[] = {
	{ "style", "raw" }, /* styles:   raw,c */
	{ "output", "default" }, /* append the number of session */
	{ "input", ".session.last" },
	{ "generated", "shellcode.out" },
	{ "shellpath", "$HOME/.splua" },
	{ "prompt", "random" }
};

static struct sploitVar* sploit_env = NULL;
static int env_cur = 0;

int help(shCtx*);
int nopsled(shCtx*);
int save(shCtx*);
int set(shCtx*);
int show(shCtx*);
int load(shCtx*);

static sploitCtx core[] = {
	{ "load", load },
	{ "show", show },
	{ "set", set },
	{ "nopsled", nopsled },
	{ "save", save },
	{ "help", help }
};

int lineParse(char*, shCtx* );
void banner(void);

static char* session_opened = NULL;

#endif /* _SHSPLOIT_H_ */
