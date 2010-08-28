#include <sploitshell.h>
#include <ui.h>

void
hexDump(unsigned char* buf, int dlen)
{
	char	c[17];
	int	i, ct;
	
	char** string;
	char bufz[BUFSIZ] = { 0 };
	char* buf2;

	for (i = 0; i < dlen; ++i)
	{
		if(i != 0 && (i % 2) == 0)
			bufz[strlen(bufz)] = ' ';

		if (i != 0 && (i % 16) == 0)
		{
			c[16] = '\0';

			strcat(bufz, "  " );
			strcat(bufz, c );
			bufz[strlen(bufz)] = '\n';
		}

		ct = buf[i] & 0xff;

		c[i % 16] = (ct >= ' ' && ct <= '~') ? ct : '.';

		sprintf( bufz, "%s%02x", bufz, ct );
	}

	c[i % 16] = '\0';

	for (; i % 16; ++i)
		strcat(bufz, "   " ); 

	bufz[strlen(bufz)] = '\t';
	strcat(bufz, c );
	bufz[strlen(bufz)] = '\n';

	for(i = 0, ct = 0;i < strlen(bufz);i++)
		if(bufz[i] == '\n')
			ct++;

	string = malloc( sizeof(char*) * ct );

	buf2 = strtok( bufz, "\n" );

	i = 0;

	string[i++] = strdup( buf2 );

	while((buf2 = strtok(NULL, "\n" )))
		string[i++] = strdup( buf2 );
	
	popupLabel( cdkscreen, string, ct );

	free( string );
}

void
sploitAddVar( struct sploitVar var )
{
	if(!(env_cur % ENV_BLOCK))
		sploit_env = realloc( sploit_env, (ENV_BLOCK + env_cur) * sizeof(struct sploitVar) );

	var.id = env_cur;
	sploit_env[ env_cur++ ] = var;
}
