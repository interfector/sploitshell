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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sploitshell.h>

extern char* current_prompt( int );
extern void resource_file( char* );

int
main(int argc,char **argv)
{
	char *line,*temp;
	shCtx ctx;
	int i = 0;

	int prompt_size = sizeof(prompts) / sizeof(prompts[0]);
	int p_rand = rand() % prompt_size;

	banner();

	resource_file( argv[1] );

	while( (line = readline( current_prompt( p_rand ) )) )
	{
		rl_bind_key('\t',rl_complete);

		if(line[0] == 0 || line[0] == '#')
			continue;

		if(!strncmp(line, "quit", 4) || !strncmp(line, "exit", 4))
			exit( 0 );

		if(strchr(line, '#'))
			*(char*)strchr(line, '#') = 0;

		temp = xstrdup(line);

		if((i = lineParse( line, &ctx ) < 0 ))
		{
			printf("* Error, command not found.\n");
			continue;
		}

     	if (temp[0] != 0)
          	add_history(temp);
	}
	
	free( sploit_env );
	free(line);

	return 0;
}
