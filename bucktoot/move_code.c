#ifndef ALLEGRO_HAVE__BOOL
#define ALLEGRO_HAVE__BOOL
#endif
#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif
#include <allegro5/keycodes.h>
#include "entity.h"
#include "utils_time.h"
#include "utils.h"
#include "game.h"

MOVEMENT run_right=
	{{ALLEGRO_KEY_RIGHT,0,0,0},
	30, //time
	80+17,34, //sx,sy
	17,0, //x/y skip
	4,0, //mx,my
	3,0,3.0}; //mod,flip,scale
MOVEMENT run_left=
	{{ALLEGRO_KEY_LEFT,0,0,0},
	30,
	80+17,34,
	17,0,
	-4,0, //mx,my
	3,1,3.0};
	
void run_animation(ENTITY *e,int left)
{
	__int64 tick,delta;
	MOVEMENT *m=&run_right;
	ANIM *anim=&e->anim;
	int frame;
	tick=get_time();
	delta=tick-e->time;
	if(delta<15){
		return;
	}
	e->time=tick;
	if(left){
		m=&run_left;
	}
	e->vx+=10;
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
	return;
}

void player_idle(ENTITY *e)
{
	ANIM *anim=&e->anim;
	anim->sx=80;
	anim->sy=34;
	anim->frame=0;
	anim->dx=e->xpos;
	anim->dy=-e->ypos;
	anim->dw=anim->sw;
	anim->dh=anim->sh;
	anim->h_scale=3.;
	anim->v_scale=3.;
}

int move_player1()
{
	int result=FALSE;
	ENTITY *e;
	ANIM *anim;
	__int64 tick,delta;
	int i,count;
	int found=FALSE;
	/*
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
	0,1, //mx my
	3,0,3.0
	},
	};
	*/
	tick=get_time();
	if(!get_entity(PLAYER1,&e))
		return result;
	anim=&e->anim;
	anim->sw=16;
	anim->sh=16;
	tick=get_time();
	delta=tick-e->time;

	printf("----%i\n",e->state);
	switch(e->state){
	case PS_IDLE:
		if(is_key_down(ALLEGRO_KEY_RIGHT)){
			e->state=PS_RUN;
			run_animation(e,0);
		}else if(is_key_down(ALLEGRO_KEY_LEFT)){
			e->state=PS_RUN;
			run_animation(e,1);
		}else if(is_key_down(ALLEGRO_KEY_Z)){
			e->state=PS_JUMP;
		}else{
			player_idle(e);
		}
		break;
	case PS_RUN:
		if(is_key_down(ALLEGRO_KEY_RIGHT)){
			e->state=PS_RUN;
			run_animation(e,0);
		}else if(is_key_down(ALLEGRO_KEY_LEFT)){
			e->state=PS_RUN;
			run_animation(e,1);
		}else{
			e->state=PS_IDLE;
			player_idle(e);
		}
		break;
	}
	count=0;
	for(i=0;i<count;i++){
		MOVEMENT *m=0;
		int key_count=_countof(m->keys);
		if(are_keys_down(m->keys,key_count)){
			if(delta>=m->delta)
			{
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
	/*
	if(!found){
		//if(delta>100)
		anim->sx=80;
		anim->sy=34;
		anim->frame=0;
		anim->dx=e->xpos;
		anim->dy=-e->ypos;
		//printf("%i\n",anim->dy);
		anim->dw=anim->sw;
		anim->dh=anim->sh;
		anim->h_scale=3.;
		anim->v_scale=3.;
	}
	*/
	return result;

}