#define _WIN32_WINNT 0x400
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>

HINSTANCE	ghinstance=0;
HACCEL		ghaccel=0;

HGLRC	hGLRC=0;
HWND	ghwindow=0;

void open_console()
{
	char title[MAX_PATH]={0}; 
	HWND hcon; 
	FILE *hf;
	static BYTE consolecreated=FALSE;
	static int hcrt=0;
	
	if(consolecreated==TRUE)
	{
		GetConsoleTitle(title,sizeof(title));
		if(title[0]!=0){
			hcon=FindWindow(NULL,title);
			ShowWindow(hcon,SW_SHOW);
		}
		hcon=(HWND)GetStdHandle(STD_INPUT_HANDLE);
		FlushConsoleInputBuffer(hcon);
		return;
	}
	AllocConsole(); 
	hcrt=_open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE),0x4000);

	fflush(stdin);
	hf=_fdopen(hcrt,"w"); 
	*stdout=*hf; 
	setvbuf(stdout,NULL,_IONBF,0);
	GetConsoleTitle(title,sizeof(title));
	if(title[0]!=0){
		hcon=FindWindow(NULL,title);
		ShowWindow(hcon,SW_SHOW); 
		SetForegroundWindow(hcon);
	}
	consolecreated=TRUE;
}
void hide_console()
{
	char title[MAX_PATH]={0}; 
	HANDLE hcon; 
	
	GetConsoleTitle(title,sizeof(title));
	if(title[0]!=0){
		hcon=FindWindow(NULL,title);
		ShowWindow(hcon,SW_HIDE);
		SetForegroundWindow(hcon);
	}
}

int move_console()
{
	char title[MAX_PATH]={0}; 
	HWND hcon; 
	GetConsoleTitle(title,sizeof(title));
	if(title[0]!=0){
		hcon=FindWindow(NULL,title);
		SetWindowPos(hcon,0,820,0,800,600,SWP_NOZORDER);
	}
	return 0;
}
int find_empty_slot(void **array,int count)
{
	int i,index=-1;
	for(i=0;i<count;i++){
		if(array[i]==0){
			index=i;
			break;
		}
	}
	return index;
}

int setupPixelFormat(HDC hDC)
{
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),  /* size */
        1,                              /* version */
        PFD_SUPPORT_OPENGL |
        PFD_DRAW_TO_WINDOW |
        PFD_DOUBLEBUFFER,               /* support double-buffering */
        PFD_TYPE_RGBA,                  /* color type */
        32,                             /* prefered color depth */
        0, 0, 0, 0, 0, 0,               /* color bits (ignored) */
        0,                              /* no alpha buffer */
        0,                              /* alpha bits (ignored) */
        0,                              /* no accumulation buffer */
        0, 0, 0, 0,                     /* accum bits (ignored) */
        16,                             /* depth buffer */
        0,                              /* no stencil buffer */
        0,                              /* no auxiliary buffers */
        PFD_MAIN_PLANE,                 /* main layer */
        0,                              /* reserved */
        0, 0, 0,                        /* no layer, visible, damage masks */
    };
    int pixelFormat;

    pixelFormat = ChoosePixelFormat(hDC, &pfd);
    if (pixelFormat == 0) {
        MessageBox(WindowFromDC(hDC), "ChoosePixelFormat failed.", "Error",
                MB_ICONERROR | MB_OK);
        exit(1);
    }

    if (SetPixelFormat(hDC, pixelFormat, &pfd) != TRUE) {
        MessageBox(WindowFromDC(hDC), "SetPixelFormat failed.", "Error",
                MB_ICONERROR | MB_OK);
        exit(1);
    }
	return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	static HDC hDC=0;
	static lmb_drag=FALSE;
#ifdef _DEBUG
	if(FALSE)
	if(msg!=WM_PAINT&&msg!=WM_SETCURSOR) //msg!=WM_NCHITTEST&&msg!=WM_ENTERIDLE&&
	{
		static DWORD tick;
		if((GetTickCount()-tick)>500)
			printf("--\n");
		print_msg(msg,lparam,wparam,hwnd);
		tick=GetTickCount();
	}
#endif
	switch(msg){
    case WM_CREATE:
		create_game_thread(hwnd);
        return 0;
	case WM_SIZE:
		{
			extern int g_screenw,g_screenh;
			int w,h;
			w=LOWORD(lparam);
			h=HIWORD(lparam);
			g_screenw=w;
			g_screenh=h;
			reshape(w,h);
		}
		break;

	case WM_LBUTTONUP:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONUP:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONUP:
	case WM_RBUTTONDOWN:
	case WM_MOUSEMOVE:
		set_mouse_data(LOWORD(lparam),HIWORD(lparam),wparam);
		break;
	case WM_MOUSEWHEEL:
		break;
	case WM_KEYDOWN:
		if(wparam==VK_ESCAPE)
			exit(0);
		set_keydown(wparam);
		break;
	case WM_KEYUP:
		set_keyup(wparam);
		break;
	case WM_SYSKEYDOWN:
		return 0;
		break;
	case WM_SYSKEYUP:
		break;
	case WM_ACTIVATE:
		//if(LOWORD(wparam)!=WA_INACTIVE)
		break;
	case WM_APP:
		break;
	case WM_PAINT:
		break;
	case WM_COMMAND:
		switch(LOWORD(wparam)){
		case IDCANCEL:
			exit(0);
			break;
		}
	case WM_CLOSE:
		exit(0);
		break;
	}
	return DefWindowProc(hwnd,msg,wparam,lparam);
}

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,PSTR szCmdLine,int iCmdShow)
{
    WNDCLASS wnd;
	MSG msg;
	const char *class_name="MOTORACER";
	ghinstance=hInstance;

	open_console();
	move_console();
	memset(&wnd,0,sizeof(wnd));
	wnd.style=CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	wnd.lpfnWndProc=WndProc;
	wnd.cbClsExtra=0;
	wnd.cbWndExtra=0;
	wnd.hInstance=ghinstance;
	wnd.hIcon=LoadIcon(NULL,IDI_APPLICATION);
	wnd.hCursor=LoadCursor(NULL,IDC_ARROW);
	wnd.hbrBackground=GetStockObject(BLACK_BRUSH);
	wnd.lpszMenuName=NULL;
	wnd.lpszClassName=class_name;
	RegisterClass(&wnd);
	ghwindow=CreateWindow(class_name,class_name,WS_OVERLAPPEDWINDOW|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_VISIBLE,
		0,0,800,600,NULL,NULL,ghinstance,NULL);
	if(!ghwindow){
		MessageBox(NULL,"Could not create main dialog","ERROR",MB_ICONERROR | MB_OK);
		return 0;
	}
	UpdateWindow(ghwindow);
	//ghaccel=LoadAccelerators(ghinstance,MAKEINTRESOURCE(IDR_ACCELERATOR));
	while(GetMessage(&msg,NULL,0,0))
	{
		{
			int _msg,lparam,wparam;
			_msg=msg.message;
			lparam=msg.lParam;
			wparam=msg.wParam;
			if(FALSE)
			//if(_msg!=WM_MOUSEFIRST&&_msg!=WM_NCHITTEST&&_msg!=WM_SETCURSOR&&_msg!=WM_ENTERIDLE&&_msg!=WM_DRAWITEM
			//	&&_msg!=WM_CTLCOLORBTN&&_msg!=WM_CTLCOLOREDIT&&_msg!=WM_PAINT)
			if(_msg!=WM_PAINT)
			{
				static DWORD tick=0;
				if((GetTickCount()-tick)>500)
					printf("--\n");
				printf("*");
				print_msg(_msg,lparam,wparam);
				tick=GetTickCount();
			}
		}
		if(ghaccel!=0)
			TranslateAccelerator(ghwindow,ghaccel,&msg);
		DispatchMessage(&msg);
//		if(!IsDialogMessage(ghwindow,&msg)){
//			TranslateMessage(&msg);
//		}
	}
	return msg.wParam;
}