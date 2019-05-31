#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <stdlib.h>
#include <math.h>

ALLEGRO_MUTEX *g_mutex=0;
ALLEGRO_FONT *g_font;
static int g_shutdown=FALSE;
static int key_list[4]={0};
static int key_pressed=0;
static int key_released=0;

#define PLAYER1 1
#define PLAYER2 2

typedef struct{
	int state;
	int sx,sy;
	int w,h;
	int xskip;
	int max_frame;
	int draw_flag;
}ANIM_DRAW;

typedef struct{
	int state;
	int frame;
	ALLEGRO_BITMAP *bm;
	ANIM_DRAW draw[4];
}ANIM;

typedef struct{
	int id;
	int xpos;
	int ypos;
	int zpos;
	float vx;
	float vy;
	float vz;
	ANIM anim;
}ENTITY;

static ENTITY *blobs[1000]={0};
static ALLEGRO_BITMAP *player_bm=0;

void do_exit()
{
	if(g_mutex)
		al_lock_mutex(g_mutex);
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
		if(is_key_down(list[i])){
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
		if(!is_key_down(list[i])){
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
			if(0==tmp->id){
				*e=tmp;
				result=TRUE;
				break;
			}
		}
	}
	return result;
}

int move_player1()
{
	int result=FALSE;
	ENTITY *e;
	if(!get_entity(PLAYER1,&e))
		return result;
	//e->
}

void draw_mario(int x,int y,int frame,double scale,int left)
{
	int w,h;
	int sx,sy;
	int flags=0;
	w=16;
	h=16;
	sy=34;
	sx=80+17+frame*17;
	if(left){
		flags|=ALLEGRO_FLIP_HORIZONTAL;
	}
	al_draw_scaled_bitmap(player_bm,sx,sy,w,h,x,y,w*scale,h*scale,flags);
}

void draw_entities()
{
	int i,count;
	count=_countof(blobs);
	for(i=0;i<count;i++){
		ENTITY *e;
		e=blobs[i];
		if(0==e)
			continue;
		//e->anim.
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
		move_player1();
		draw_entities();


		al_flip_display();

		al_unlock_mutex(g_mutex);
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
//	e->anim.state
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

	gthread=al_create_thread(&game_thread,(void*)disp);
	al_start_thread(gthread);

	create_p1();

	while(1){
		ALLEGRO_EVENT event;
		al_wait_for_event(queue,&event);
		switch(event.type) {
		case ALLEGRO_EVENT_KEY_CHAR:
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
				key_pressed=TRUE;
			}
			break;
		case ALLEGRO_EVENT_KEY_UP:
			{
				int key=event.keyboard.keycode;
				set_key_up(key);
				key_released=TRUE;
			}
			break;
		}
	}

	return 0;
}