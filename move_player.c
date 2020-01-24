#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

#define PLAYER1 1
#define PLAYER2 2

typedef struct{
	int state;
	int frame;
	int sx,sy;
	int sw,sh;
	float dx,dy;
	float dw,dh;
	int h_flip,v_flip;
	float h_scale,v_scale;
	ALLEGRO_BITMAP *bm;
}ANIM;

typedef struct{
	int id;
	int xpos;
	int ypos;
	int zpos;
	float vx;
	float vy;
	float vz;
	__int64 time;
	ANIM anim;
}ENTITY;

typedef struct{
	int keys[4];
	__int64 delta;
	int sx,sy;
	int xskip,yskip;
	int mx,my;
	int modulo;
	int flip_h;
	float scale;
}MOVEMENT;

int move_player1()
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
	0,1, //mx my
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
		anim->dy=-e->ypos;
		anim->dw=anim->sw;
		anim->dh=anim->sh;
		anim->h_scale=3.;
		anim->v_scale=3.;
		e->time=0;
	}
	return result;
}
