#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <stdarg.h>

HWND ghconsole=0;
int _open_osfhandle(HANDLE,int);
static DWORD(WINAPI *SetConsoleIcon)(HICON)=0;
#define _O_TEXT         0x4000  /* file mode is text (translated) */

HANDLE get_console_handle()
{
	static HWND (WINAPI *GetConsoleWindow)(void)=0;
	if(GetConsoleWindow==0){
		HMODULE hmod=LoadLibrary(TEXT("kernel32.dll"));
		if(hmod!=0){
			GetConsoleWindow=(HWND (WINAPI *)(void))GetProcAddress(hmod,"GetConsoleWindow");
			if(GetConsoleWindow!=0){
				ghconsole=GetConsoleWindow();
			}
			SetConsoleIcon=(DWORD(WINAPI *)(HICON))GetProcAddress(hmod,"SetConsoleIcon");
		}
	}
	return ghconsole;
}

void open_console()
{
	HWND hcon;
	FILE *hf;
	static BYTE consolecreated=FALSE;
	static int hcrt=0;

	if(consolecreated==TRUE)
	{
		if(ghconsole!=0)
			ShowWindow(ghconsole,SW_SHOW);
		hcon=(HWND)GetStdHandle(STD_INPUT_HANDLE);
		FlushConsoleInputBuffer(hcon);
		return;
	}
	AllocConsole();
	hcrt=_open_osfhandle(GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT);

	fflush(stdin);
	hf=_fdopen(hcrt,"w");
	*stdout=*hf;
	setvbuf(stdout,NULL,_IONBF,0);
	consolecreated=TRUE;
}

__int64 get_time()
{
	__int64 result=0;
	LARGE_INTEGER tmp;
	static LARGE_INTEGER freq={0};
	static int have_freq=0;
	QueryPerformanceCounter(&tmp);
	if(!have_freq){
		QueryPerformanceFrequency(&freq);
		have_freq=1;
	}
	if(0!=freq.QuadPart){
		result=(tmp.QuadPart*1000)/freq.QuadPart;
	}
	return result;

}

int tick_measure(int end)
{
	int result=0;
	static LARGE_INTEGER tick={0};
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	if(end){
		static LARGE_INTEGER freq={0};
		static int have_freq=0;
		LARGE_INTEGER val;
		val.QuadPart=tmp.QuadPart-tick.QuadPart;
		if(!have_freq){
			QueryPerformanceFrequency(&freq);
			have_freq=1;
			if(0==freq.QuadPart)
				return 0;

		}
		val.QuadPart=(val.QuadPart*1000)/freq.QuadPart;
		result=(int)val.QuadPart;
	}
	tick.QuadPart=tmp.QuadPart;
	return result;
}




int do_wait()
{
	const char *p=getenv("PROMPT");
	if(0==p){
		printf("press any key\n");
		getch();
	}
	return 0;
}
int get_key()
{
	int result=0;
	static HANDLE hcon=0;
	if(0==hcon)
		hcon=GetStdHandle(STD_INPUT_HANDLE);
	if(INVALID_HANDLE_VALUE==hcon)
		return 0;
	{
		INPUT_RECORD rec={0};
		DWORD count=0;
		if(PeekConsoleInput(hcon,&rec,1,&count)){
			if(count==1){
				ReadConsoleInput(hcon,&rec,1,&count);
				if(rec.EventType==KEY_EVENT){
					KEY_EVENT_RECORD *ke=(KEY_EVENT_RECORD*)&rec.Event.KeyEvent;
					if(ke->bKeyDown){
						result=(int)ke->wVirtualKeyCode;
					}
				}
			}
		}
	}
	return result;
}

int getkey_wait()
{
	int result=0;
	static HANDLE hcon=0;
	if(0==hcon)
		hcon=GetStdHandle(STD_INPUT_HANDLE);
	if(INVALID_HANDLE_VALUE==hcon)
		return 0;
	while(1){
		INPUT_RECORD rec={0};
		DWORD count=0;
		ReadConsoleInput(hcon,&rec,1,&count);
		if(rec.EventType==KEY_EVENT){
			KEY_EVENT_RECORD *ke=(KEY_EVENT_RECORD*)&rec.Event.KeyEvent;
			if(ke->bKeyDown){
				result=(int)ke->wVirtualKeyCode;
				break;
			}
		}
	}
	return result;
}
static int get_flen(FILE *f)
{
	int result;
	int pos;
	pos=ftell(f);
	fseek(f,0,SEEK_END);
	result=ftell(f);
	fseek(f,pos,SEEK_SET);
	return result;
}
int key_state(int key)
{
	int val;
	val=GetKeyState(key);
	if(val&0x8000)
		return TRUE;
	else
		return FALSE;
}

int main(int argc,char **argv)
{
	int test_wave_player(void *);
	HANDLE hcon;
	hcon=get_console_handle();
	if(hcon){
		HANDLE hdesk=GetDesktopWindow();
		RECT rect={0};
		GetWindowRect(hdesk,&rect);
		SetWindowPos(hcon,NULL,0,rect.bottom/2,0,0,SWP_NOZORDER|SWP_NOSIZE|SWP_SHOWWINDOW);
	}
	//test_shit();
	start_compiler_thread();
	_beginthread(&test_wave_player,0,0);
	test_game();
}



int WINAPI WinMain(HINSTANCE hinst,HINSTANCE hprev,LPSTR cmd_line,int cmd_show)
{
	open_console();
	main(0,0);
	//do_wait();
	return 0;
}