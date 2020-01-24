#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <stdlib.h>
#include <math.h>
#include "libtcc.h"
#include "entity.h"

ALLEGRO_MUTEX *g_mutex=0;
ALLEGRO_FONT *g_font=0;
ALLEGRO_DISPLAY *g_display=0;
static int g_shutdown=FALSE;
static int key_list[10]={0};


static ENTITY *blobs[1000]={0};
static ALLEGRO_BITMAP *player_bm=0;

//move cursor above window
void exit_cursor()
{
	RECT rect={0};
	HWND hwnd;
	if(0==g_display)
		return;
	hwnd=(HWND)al_get_win_window_handle(g_display);
	GetWindowRect(hwnd,&rect);
	SetCursorPos((rect.left+rect.right)/2,rect.top-5);
	ShowWindow(hwnd,SW_HIDE);
	hwnd=GetConsoleWindow();
	if(hwnd){
		ShowWindow(hwnd,SW_HIDE);
	}
}
void center_cursor()
{
	RECT rect={0};
	HWND hwnd;
	if(0==g_display)
		return;
	hwnd=(HWND)al_get_win_window_handle(g_display);
	GetWindowRect(hwnd,&rect);
	SetCursorPos((rect.left+rect.right)/2,(rect.bottom+rect.top)/2);
	SetFocus(hwnd);
}

void do_exit()
{
	if(g_mutex)
		al_lock_mutex(g_mutex);
	exit_cursor();
	exit(0);
}
void abort_msg(const char *msg)
{
	printf("%s\n",msg);
	do_wait();
	do_exit();
}

static void set_key_down(int key)
{
	int i,count;
	count=_countof(key_list);
	for(i=0;i<count;i++){
		int k=key_list[i];
		if(k==key)
			return;
	}
	for(i=0;i<count;i++){
		int k=key_list[i];
		if(0==k){
			key_list[i]=key;
			break;
		}
	}
}
static void set_key_up(int key)
{
	int i,count;
	count=_countof(key_list);
	for(i=0;i<count;i++){
		int k=key_list[i];
		if(k==key){
			key_list[i]=0;
			return;
		}
	}
}
int is_key_down(int key)
{
	int i,count;
	count=_countof(key_list);
	for(i=0;i<count;i++){
		int k=key_list[i];
		if(k==key){
			return TRUE;
		}
	}
	return FALSE;
}
int are_keys_down(int *list,int count)
{
	int result=0;
	int i;
	for(i=0;i<count;i++){
		int key=list[i];
		if(0==key)
			continue;
		if(is_key_down(key)){
			result++;
		}
	}
	return result;
}
int are_keys_up(int *list,int count)
{
	int result=0;
	int i;
	for(i=0;i<count;i++){
		int key=list[i];
		if(0==key)
			continue;
		if(!is_key_down(key)){
			result++;
		}
	}
	return result;
}

int draw_rect(int x,int y,int w,int h,ALLEGRO_COLOR c)
{
	int i;
	for(i=0;i<w;i++){
		al_draw_pixel(x+i,y,c);
		al_draw_pixel(x+i,y+h,c);
	}
	for(i=0;i<h;i++){
		al_draw_pixel(x,y+i,c);
		al_draw_pixel(x+w,y+i,c);
	}
	return 0;
}

int add_entity(ENTITY *e)
{
	int result=FALSE;
	int i,count;
	count=_countof(blobs);
	for(i=0;i<count;i++){
		ENTITY *tmp;
		tmp=blobs[i];
		if(0==tmp){
			blobs[i]=e;
			result=TRUE;
			break;
		}
	}
	return result;
}
int get_entity(int id,ENTITY **e)
{
	int result=FALSE;
	int i,count;
	count=_countof(blobs);
	for(i=0;i<count;i++){
		ENTITY *tmp;
		tmp=blobs[i];
		if(tmp){
			if(id==tmp->id){
				*e=tmp;
				result=TRUE;
				break;
			}
		}
	}
	return result;
}

int __move_player1()
{
	int result=FALSE;
	ENTITY *e;
	ANIM *anim;
	__int64 tick,delta;
	int i,count;
	int found=FALSE;
	MOVEMENT moves[]={
		{{ALLEGRO_KEY_RIGHT,0,0,0},
		30, //time
		80+17,34, //sx,sy
		17,0, //x/y skip
		4,0, //mx,my
		3,0,3.0}, //mod,flip,scale
		{{ALLEGRO_KEY_LEFT,0,0,0},
		30,
		80+17,34,
		17,0,
		-4,0, //mx,my
		3,1,3.0},
		{{ALLEGRO_KEY_Z,0,0,0},
		300,
		80+17*5,34, //sx,sy
		0,0, //xy skip
		0,-1, //mx my
		3,0,3.0
		},
	};
	if(!get_entity(PLAYER1,&e))
		return result;
	anim=&e->anim;
	anim->sw=16;
	anim->sh=16;
	tick=get_time();
	delta=tick-e->time;
	count=_countof(moves);
	for(i=0;i<count;i++){
		MOVEMENT *m;
		int key_count=_countof(m->keys);
		m=&moves[i];
		if(are_keys_down(m->keys,key_count)){
			if(delta>=m->delta){
				int frame;
				e->xpos+=m->mx;
				e->ypos+=m->my;
				frame=(anim->frame+1)%m->modulo;
				anim->sx=m->sx+(frame*m->xskip);
				anim->sy=m->sy+(frame*m->yskip);
				anim->dx=e->xpos;
				anim->dy=e->ypos;
				anim->h_flip=m->flip_h;
				anim->dw=anim->sw;
				anim->dh=anim->sh;
				anim->h_scale=m->scale;
				anim->v_scale=m->scale;
				anim->frame=frame;
				e->time=tick;
			}
			found=TRUE;
		}
	}
	if(!found){
		anim->sx=80;
		anim->sy=34;
		anim->frame=0;
		anim->dx=e->xpos;
		anim->dy=e->ypos;
		anim->dw=anim->sw;
		anim->dh=anim->sh;
		anim->h_scale=3.;
		anim->v_scale=3.;
		e->time=0;
	}
	return result;
}

void draw_bitmap(ANIM *anim,int ox,int oy)
{
	int flags=0;
	float sx,sy,sw,sh;
	float dx,dy,dw,dh;
	ALLEGRO_BITMAP *bm;
//	sx=80+17+frame*17;
	// sy=34

	if(0==anim->bm)
		return;
	if(anim->h_flip){
		flags|=ALLEGRO_FLIP_HORIZONTAL;
	}else if(anim->v_flip){
		flags|=ALLEGRO_FLIP_VERTICAL;
	}
	sx=anim->sx;
	sy=anim->sy;
	sw=anim->sw;
	sh=anim->sh;
	dx=anim->dx+ox;
	dy=anim->dy+oy;
	dw=anim->dw*anim->h_scale;
	dh=anim->dh*anim->v_scale;
	bm=anim->bm;
	al_draw_scaled_bitmap(bm,sx,sy,sw,sh,dx,dy,dw,dh,flags);
}

void draw_entities(ALLEGRO_DISPLAY *disp)
{
	int i,count;
	int ox,oy;
	__int64 tick;
	tick=get_time();
	ox=al_get_display_height(disp);
	oy=al_get_display_width(disp);
	ox/=2;
	oy/=2;
	count=_countof(blobs);
	for(i=0;i<count;i++){
		ENTITY *e;
		e=blobs[i];
		if(0==e)
			continue;
		draw_bitmap(&e->anim,ox,oy);
	}
}

void *game_thread(ALLEGRO_THREAD *athread,void *arg)
{
	ALLEGRO_DISPLAY *disp;
	int i=0;
	int x=0;
	int y=0;
	int frame=0;
	int left=0;
	disp=(ALLEGRO_DISPLAY*)arg;
	al_set_target_bitmap(al_get_backbuffer(disp));
	printf("thread\n");
	while(1){
		al_lock_mutex(g_mutex);
		al_clear_to_color(al_map_rgb_f(0,0,0));
		al_draw_textf(g_font,al_map_rgb_f(1,1,1),0,0,0,"x=%i y=%i",x,y);
		draw_entities(disp);

		al_flip_display();

		al_unlock_mutex(g_mutex);
		move_player1_script();
		al_wait_for_vsync();

	}
	return 0;
}

int create_p1()
{
	ENTITY *e;
	e=calloc(1,sizeof(ENTITY));
	if(0==e){
		abort_msg("unable to create player 1\n");
	}
	player_bm=al_load_bitmap("data/mario.png");
	if(0==player_bm){
		abort_msg("unable to load player bitmap");
	}
	e->id=PLAYER1;
	e->anim.bm=player_bm;
	e->anim.state=0;
	e->xpos=0;
	e->ypos=0;
	add_entity(e);
	move_player1();
	return TRUE;
}

int test_game()
{
	ALLEGRO_TIMER *timer;
	ALLEGRO_EVENT_QUEUE *queue;
	ALLEGRO_MONITOR_INFO info;
	ALLEGRO_DISPLAY *disp;
	ALLEGRO_THREAD *gthread;

	if(!al_init()) {
		abort_msg("Failed to init Allegro.\n");
	}
	g_mutex=al_create_mutex();

	if(!al_init_image_addon()) {
		abort_msg("Failed to init IIO addon.\n");
	}

	al_init_font_addon();

	g_font = al_load_font("data/a4_font.tga",0,0);
	if(0==g_font){
		abort_msg("unable to load font");
	}

	al_get_num_video_adapters();

	al_get_monitor_info(0,&info);
	disp = al_create_display(1,1);
	g_display=disp;
	if(0==disp){
		abort_msg("error creating display\n");
	}
	if(!al_install_keyboard()) {
		abort_msg("Error installing keyboard.\n");
	}
	queue = al_create_event_queue();
	al_register_event_source(queue,al_get_keyboard_event_source());
	al_register_event_source(queue,al_get_display_event_source(disp));

	al_clear_to_color(al_map_rgb_f(0,0,0));
	al_flip_display();

	al_resize_display(disp,640,480);
	al_clear_to_color(al_map_rgb_f(0,0,0));
	al_flip_display();
	al_set_window_position(disp,info.x2-640,info.y2-480-100);
	center_cursor();

	gthread=al_create_thread(&game_thread,(void*)disp);
	al_start_thread(gthread);

	create_p1();

	while(1){
		ALLEGRO_EVENT event;
		al_wait_for_event(queue,&event);
		switch(event.type) {
		case ALLEGRO_EVENT_KEY_CHAR:
			break;
		case ALLEGRO_EVENT_KEY_DOWN:
			{
				int key=event.keyboard.keycode;
				switch(key){
				case ALLEGRO_KEY_ESCAPE:
					do_exit();
					break;
				case ALLEGRO_KEY_A:
					printf("A\n");
					break;
				}
				printf("key:%i\n",key);
				set_key_down(key);
			}
			break;
		case ALLEGRO_EVENT_KEY_UP:
			{
				int key=event.keyboard.keycode;
				set_key_up(key);
			}
			break;
		}
	}

	return 0;
}