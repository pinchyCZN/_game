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