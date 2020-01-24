#pragma once
#include <allegro5/bitmap.h>

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
