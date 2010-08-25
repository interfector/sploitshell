#include <sploitshell.h>
#include <ui.h>

int
main(int argc,char **argv)
{
	char* code = getCDKAssembly( );

	setCDKPref( );

	ui_error( msg_error[LOAD_ERR] );

	return 0;
}
