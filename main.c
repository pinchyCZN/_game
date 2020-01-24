#include <windows.h>
#include <process.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <stdarg.h>
#include "utils.h"





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
	//_beginthread(&test_wave_player,0,0);
	test_game();
}



int WINAPI WinMain(HINSTANCE hinst,HINSTANCE hprev,LPSTR cmd_line,int cmd_show)
{
	open_console();
	main(0,0);
	//do_wait();
	return 0;
}