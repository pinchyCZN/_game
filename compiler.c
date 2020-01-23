#include <allegro5/allegro.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libtcc.h"

ALLEGRO_THREAD *g_compiler_thread;

int get_entity(int id,void *);
__int64 get_time();
int are_keys_down(int *list,int count);

static int add(int a, int b)
{
	return a + b;
}

static const char *hello()
{
	return "hello!\n";
}

static int my_printf(const char *fmt,...)
{
	return 0;
}

static char my_program[] =
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

static int compile_program(TCCState *state,char *code_str)
{
	int result=FALSE;
	TCCState *s=state;
	if(0==state || 0==code_str){
		return result;
	}
	tcc_add_include_path(s,"..\\tcc\\");
	tcc_add_include_path(s,"..\\tcc\\include\\");
	tcc_add_include_path(s,"..\\allegro\\include");
	tcc_add_include_path(s,"..\\tcc\\win32\\include");
	tcc_add_library_path(s,"..\\tcc\\win32\\lib");
	tcc_set_output_type(s, TCC_OUTPUT_MEMORY);
	if (tcc_compile_string(s, code_str) == -1)
		return result;

	tcc_add_symbol(s, "get_entity", get_entity);
	tcc_add_symbol(s, "get_time", get_time);
	tcc_add_symbol(s, "are_keys_down", are_keys_down);
	tcc_add_symbol(s, "memset", memset);

	if (tcc_relocate(s, TCC_RELOCATE_AUTO) < 0){
		return result;
	}
	result=TRUE;
	return result;
}

static int get_code_file(WCHAR **fname)
{
	int result=FALSE;
	const int tmp_len=4096;
	WCHAR *tmp;
	tmp=calloc(tmp_len+1,sizeof(WCHAR));
	if(tmp){
		GetCurrentDirectoryW(tmp_len,tmp);
		if(tmp[0]){
			_snwprintf(tmp,tmp_len,L"%s\\move_code.c",tmp);
			fname[0]=tmp;
			result=TRUE;
		}
	}
	return result;
}
static int get_flen(FILE *f)
{
	int result=0;
	if(f){
		int pos;
		pos=ftell(f);
		fseek(f,0,SEEK_END);
		result=ftell(f);
		fseek(f,pos,SEEK_SET);
	}
	return result;
}
static int read_file(char **str)
{
	int result=FALSE;
	WCHAR *fname=0;
	get_code_file(&fname);
	if(fname){
		FILE *f=_wfopen(fname,L"rb");
		if(f){
			int len;
			char *tmp;
			len=get_flen(f);
			tmp=calloc(len+1,1);
			if(tmp){
				fread(tmp,1,len,f);
				*str=tmp;
				result=TRUE;
			}
			fclose(f);
		}
	}
	return result;
}


static void *g_script_func=0;
static CRITICAL_SECTION g_mutex={0};
static int g_mutex_ready=FALSE;
static void *compile_thread(ALLEGRO_THREAD *athread,void *arg)
{
	TCCState *state=0;
	WCHAR *path=0;
	int path_len=1024;
	HANDLE fn=INVALID_HANDLE_VALUE;
	path=calloc(path_len,2);
	InitializeCriticalSection(&g_mutex);
	g_mutex_ready=TRUE;
	if(0==path)
		goto EXIT_THREAD;
	GetCurrentDirectoryW(path_len,path);
	fn=FindFirstChangeNotification(path,FALSE,FILE_NOTIFY_CHANGE_FILE_NAME|FILE_NOTIFY_CHANGE_LAST_WRITE);
	if(INVALID_HANDLE_VALUE==fn){
		goto EXIT_THREAD;
	}
	goto READ_FILE;
	while(1){
		int res;
		res=FindNextChangeNotification(fn);
		if(!res){
			printf("next change notification failed\n");
			break;
		}
		res=WaitForSingleObject(fn,INFINITE);
		if(WAIT_OBJECT_0==res){
READ_FILE:
			EnterCriticalSection(&g_mutex);
			g_script_func=0;
			if(state){
				tcc_delete(state);
				state=0;
			}
			if(0==state){
				state=tcc_new();
			}
			if(state){
				char *str=0;
				read_file(&str);
				if(str){
					printf("compiling program\n");
					res=compile_program(state,str);
					if(res){
						g_script_func=tcc_get_symbol(state,"move_player1");
					}
					free(str);
				}
			}
			LeaveCriticalSection(&g_mutex);
		}else{
			printf("ERROR WAIT\n");
			break;
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
	typedef int (*MOVECODE)();
	MOVECODE move_code;
	if(!g_mutex_ready)
		return 0;
	EnterCriticalSection(&g_mutex);
	if(g_script_func){
		move_code=(MOVECODE)g_script_func;
		move_code();
	}
	LeaveCriticalSection(&g_mutex);
	return 0;
}

int start_compiler_thread()
{
	g_compiler_thread=al_create_thread(&compile_thread,0);
	al_start_thread(g_compiler_thread);
	return TRUE;
}