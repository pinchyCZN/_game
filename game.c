#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>

static int mousex=0,mousey=0;
static int mbuttons=0;
static keyboard[0xFF]={0};

typedef struct{
	int x,y,z;
	int vx,vy,vz;
	int wx,wy,wz;
	int state;
	int state2;
}ENTITY;
int set_mouse_data(int x,int y,int buttons)
{
	mousex=x;
	mousey=y;
	mbuttons=buttons;
	return 0;
}
int set_keydown(int key)
{
	if(key>=0 && key<sizeof(keyboard))
		keyboard[key]=1;
}
int set_keyup(int key)
{
	if(key>=0 && key<sizeof(keyboard))
		keyboard[key]=0;
}

int game_thread(HWND hwnd)
{
	HDC hdc=0;
	HGLRC hglrc=0;
	hdc=GetDC(hwnd);
	if(hdc)
		setupPixelFormat(hdc);
	hglrc=wglCreateContext(hdc);
	if(hglrc)
		wglMakeCurrent(hdc,hglrc);
	if(hdc){
		SelectObject(hdc,GetStockObject(SYSTEM_FONT));
		wglUseFontBitmaps(hdc,0,255,1000);
	}
	gl_init();


	while(1){
		dude();
		if(hdc!=0 && hglrc!=0){
			wglMakeCurrent(hdc,hglrc);
			do_gfx();
			SwapBuffers(hdc);
		}
		Sleep(15);
	}
}
int create_game_thread(HWND hwnd)
{
	_beginthread(game_thread,0,hwnd);
}

int render_entity(ENTITY *e)
{
	float rot[3]={0},trans[3]={0};
	if(e==0)
		return 0;
	trans[0]=e->x/10.;
	trans[1]=e->y/10.;
	trans[2]=e->z/10.;
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(trans[0], trans[1], trans[2]);
	render_cube();
	glPopMatrix();

	//render_rect(&rot,&trans);
}
int dude()
{
	static ENTITY p1={0};
	if(keyboard[VK_LEFT])
		p1.x--;
	if(keyboard[VK_RIGHT])
		p1.x++;
	if(keyboard[VK_CONTROL])
		p1.y++;
	else{
		p1.y--;
		if(p1.y<0)
			p1.y=0;

	}
	render_entity(&p1);

}

