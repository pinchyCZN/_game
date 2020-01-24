#include <windows.h>
#include <stdio.h>

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
int get_flen(FILE *f)
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
