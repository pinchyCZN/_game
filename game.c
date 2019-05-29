#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <stdlib.h>
#include <math.h>


typedef struct{
	int frame;
}ANIM;

typedef struct{
	int xpos;
	int ypos;
	int zpos;
	float vx;
	float vy;
	float vz;
	ANIM anim;
}ENTITY;


int abort_msg(const char *msg)
{
	printf("%s\n",msg);
	exit(0);
}

int test_game()
{
	ALLEGRO_TIMER *timer;
	ALLEGRO_EVENT_QUEUE *queue;
	ALLEGRO_MONITOR_INFO info;
	ALLEGRO_DISPLAY *disp;

	if(!al_init()) {
		abort_msg("Failed to init Allegro.\n");
	}

	if(!al_init_image_addon()) {
		abort_msg("Failed to init IIO addon.\n");
	}

	al_init_font_addon();

	al_get_num_video_adapters();

	al_get_monitor_info(0,&info);
	disp = al_create_display(640,480);
	if(0==disp){
		abort_msg("error creating display\n");
	}
	if(!al_install_keyboard()) {
		abort_msg("Error installing keyboard.\n");
	}
	queue = al_create_event_queue();
	al_register_event_source(queue,al_get_keyboard_event_source());

	while(1){
		ALLEGRO_EVENT event;
		al_wait_for_event(queue,&event);
		switch(event.type) {
		case ALLEGRO_EVENT_KEY_CHAR:
			{
				int key=event.keyboard.keycode;
				printf("key:%i\n",key);
			}
			break;
		}
	}

	return 0;
}