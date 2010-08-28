#include <sploitshell.h>
#include <pthread.h>
#include <ui.h>

int
main(int argc,char **argv)
{
	pthread_t thread;

	pthread_create( &thread, NULL,(void* (*)(void*)) initCDKMenu, NULL );

	while(1) sleep( 1 );

	return 0;
}
