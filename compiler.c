#include <allegro5/allegro.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libtcc.h"

ALLEGRO_THREAD *g_compiler_thread;

int get_entity(int id,void *);
__int64 get_time();
int are_keys_down(int *list,int count);

int add(int a, int b)
{
	return a + b;
}

const char *hello()
{
	return "hello!\n";
}

int my_printf(const char *fmt,...)
{
	return 0;
}

char my_program[] =
/*
"#include <allegro5/allegro.h>\n"
"int func(int x){\n"
"return 0\n"
"\n"
"}\n"
*/
"int func(int x){ return x+1; }\n"
;


int test_shit()
{
	TCCState *s;
	int i;
	int (*func)(int);
	int argc=0;
	char **argv=0;

	s = tcc_new();
	if (!s) {
		fprintf(stderr, "Could not create tcc state\n");
		exit(1);
	}

	/* if tcclib.h and libtcc1.a are not installed, where can we find them */
	for (i = 1; i < argc; ++i) {
		char *a = argv[i];
		if (a[0] == '-') {
			if (a[1] == 'B')
				tcc_set_lib_path(s, a+2);
			else if (a[1] == 'I')
				tcc_add_include_path(s, a+2);
			else if (a[1] == 'L')
				tcc_add_library_path(s, a+2);
		}
	}
	tcc_add_include_path(s,"..\\tcc\\");
	tcc_add_include_path(s,"..\\tcc\\include\\");
	tcc_add_include_path(s,"..\\allegro\\include");
	tcc_add_include_path(s,"..\\tcc\\win32\\include");
	tcc_add_library_path(s,"..\\tcc\\win32\\lib");

	/* MUST BE CALLED before any compilation */
	tcc_set_output_type(s, TCC_OUTPUT_MEMORY);

	if (tcc_compile_string(s, my_program) == -1)
		return 1;

	/* as a test, we add symbols that the compiled program can use.
	You may also open a dll with tcc_add_dll() and use symbols from that */
	tcc_add_symbol(s, "add", add);
	tcc_add_symbol(s, "hello", hello);
	tcc_add_symbol(s, "printf", my_printf);

	/* relocate the code */
	if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0){
		do_wait();
		exit(0);
		return 1;
	}

	/* get entry symbol */
	func = tcc_get_symbol(s, "foo");
	if (!func)
		return 1;

	/* run the code */
	func(32);

	/* delete the state */
	tcc_delete(s);

	return 0;
}

int compile_program(TCCState *state,char *code_str)
{
	int result=FALSE;
	TCCState *s=state;
	tcc_add_include_path(s,"..\\tcc\\");
	tcc_add_include_path(s,"..\\tcc\\include\\");
	tcc_add_include_path(s,"..\\allegro\\include");
	tcc_add_include_path(s,"..\\tcc\\win32\\include");
	tcc_add_library_path(s,"..\\tcc\\win32\\lib");
	tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
	if (tcc_compile_string(s, my_program) == -1)
		return result;

	tcc_add_symbol(s, "get_entity", get_entity);
	tcc_add_symbol(s, "get_time", get_time);
	tcc_add_symbol(s, "are_keys_down", are_keys_down);

	if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0){
		return result;
	}
	result=TRUE;
	return result;
}



void *g_script_func=0;
void *compile_thread(ALLEGRO_THREAD *athread,void *arg)
{
	TCCState *state=0;
	WCHAR *path=0;
	int path_len=1024;
	HANDLE fn=INVALID_HANDLE_VALUE;
	path=calloc(path_len,2);
	if(0==path)
		goto EXIT_THREAD;
	GetCurrentDirectoryW(path_len,path);
	fn=FindFirstChangeNotification(path,FALSE,FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE);
	if(INVALID_HANDLE_VALUE==fn)
		goto EXIT_THREAD;

	while(1){
		if(0==state){
			state=tcc_new();
		}
		if(state){

		}
	}
EXIT_THREAD:
	free(path);
	if(INVALID_HANDLE_VALUE!=fn)
		FindCloseChangeNotification(fn);

	return 0;
}

int move_player1_script()
{

	return 0;
}

int start_compiler_thread()
{
	g_compiler_thread=al_create_thread(&compile_thread,0);
	al_start_thread(g_compiler_thread);
	return TRUE;
}