#include <windows.h>
static HWND ghwnd=0;
static int mousex=0,mousey=0;
static int mbuttons=0;

int set_mouse_data(int x,int y,int buttons)
{
	mousex=x;
	mousey=y;
	mbuttons=buttons;
	return 0;
}
int game_thread()
{
	while(1){
		if(ghwnd!=0)
			InvalidateRect(ghwnd,0,0);
		Sleep(15);
	}
}
int create_game_thread(HWND hwnd)
{
	ghwnd=hwnd;
	_beginthread(game_thread,0,0);
}
int get_mousex()
{ return mousex; }