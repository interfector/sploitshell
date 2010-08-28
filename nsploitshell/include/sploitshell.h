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

#include <stdio.h>

#define pdie(x) do{ perror(x); exit(1); }while(1)

typedef enum { sploit_nopsled, sploit_jump, sploit_eip, sploit_shellcode, sploit_assembly } sploit_type;

struct envp {
	char* name;
	char* value;
};

struct sploitVar {
	int id;
	sploit_type type;

	char* data;
	unsigned int addr;

	int set;
};

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

static char* session_opened = NULL;

void sploitAddVar( struct sploitVar );
void hexDump(unsigned char*,int);

#endif /* _SHSPLOIT_H_ */
