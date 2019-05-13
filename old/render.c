#include <windows.h>
#include <stdio.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include "glext.h"
//#pragma warning(error:4013) //'function' undefined; assuming extern returning int

int g_screenw=1024;
int g_screenh=758;

float gx=0,gy=0,gz=-10;
float grx=0,gry=0,grz=0;

int g_ztri=-100;
int g_tmp=700;

GLfloat ambient[]={1.0,1.0,1.0,0.0};
GLfloat light_position[]={-5.0,-10.0,10.0,0.0};
GLfloat white_light[]={1.0,1.0,1.0,1.0};

GLuint vertex_aid=0;

double fps=0;
PFNGLGENVERTEXARRAYS glGenVertexArrays;
PFNGLBINDVERTEXARRAY glBindVertexArray;

void perspectiveGL( GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar )
{
	const GLdouble pi = 3.1415926535897932384626433832795;
	GLdouble fW, fH;
	fH = tan( fovY / 360 * pi ) * zNear;
	fW = fH * aspect;
	glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}
void reshape(int w, int h)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	perspectiveGL(25.0,(GLfloat)w/(GLfloat)h,.1,10000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0,0,(GLsizei)w,(GLsizei)h);
}

int print_globs()
{
	printf("%3.2f %3.2f %3.2f , %3.2f %3.2f %3.2f\n",grx,gry,grz,gx,gy,gz);
	return 0;
}

int display_str(char *str,int x,int y)
{
	int i,len;
	int list[3]={GL_TEXTURE_2D,GL_DEPTH_TEST,GL_LIGHTING};
	int setting[3];
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0,g_screenw,g_screenh,0,-1000,1000);

	for(i=0;i<sizeof(list)/sizeof(int);i++){
		setting[i]=0;
		if(glIsEnabled(list[i])){
			glDisable(list[i]);
			setting[i]=1;
		}
	}
	glColor3f(1.0,1.0,1.0);
	glRasterPos3f(x,y+15,0);

	glListBase(1000); 
	len=strlen(str);
	glCallLists(len,GL_UNSIGNED_BYTE,str);
	for(i=0;i<sizeof(list)/sizeof(int);i++){
		if(setting[i])
			glEnable(list[i]);
	}
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	return 0;
}

int gl_init()
{
	
	glClearColor(0.0,0.0,0.0,0.0);
//	glShadeModel(GL_FLAT);
//	glShadeModel(GL_SMOOTH);
	glLightfv(GL_LIGHT0,GL_POSITION,light_position);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,white_light);
	glLightfv(GL_LIGHT0,GL_SPECULAR,white_light);
	glLightfv(GL_LIGHT0,GL_AMBIENT,ambient);
    glMatrixMode(GL_PROJECTION);
    glFrustum(-0.08, 0.08F, -0.06F, 0.06F, 0.1F, 1000.0F);

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_DEPTH_TEST);
//		glLightfv(GL_LIGHT0,GL_POSITION,light_position);

	/*
	glGenVertexArrays=wglGetProcAddress("glGenVertexArrays");
	glBindVertexArray=wglGetProcAddress("glBindVertexArray");
	glGenVertexArrays(1,&vertex_aid);
	glBindVertexArray(vertex_aid);
	*/

/*
    glMatrixMode(GL_PROJECTION);
    glFrustum(-0.5F, 0.5F, -0.5F, 0.5F, 1.0F, 3.0F);

    glMatrixMode(GL_MODELVIEW);
    glTranslatef(0.0F, 0.0F, -2.0F);

    glRotatef(30.0F, 1.0F, 0.0F, 0.0F);
    glRotatef(30.0F, 0.0F, 1.0F, 0.0F);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
*/
	return 0;
}
int test_triangle()
{
	static float theta=0;
	glDisable(GL_TEXTURE_2D);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
//	glLoadIdentity();

    glTranslatef(0,0,0);
//	glRotatef(theta, 0.0f, 1.0f, 1.0f);


	glBegin(GL_TRIANGLES);
	glColor3f(1.0f, 0.0f, 0.0f);   glVertex2f(0.0f,   1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);   glVertex2f(0.87f,  -0.5f);
	glColor3f(0.0f, 0.0f, 1.0f);   glVertex2f(-0.87f, -0.5f);
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
	theta+=1.0;
	return 0;
}
int render_rect(float *rot,float *trans)
{
	static float theta=0;
	int i;
	unsigned char indices[] = { 0, 1, 2, 0, 2, 3 };
	float vertices[] = { 
		0, 0, 0,
		1, 0, 0,
		1, 1, 0,
		0, 1, 0
	};
	float normals[] = { 
		0, 0, 1,
		0, 0, 1,
		0, 0, 1,
		0, 0, 1
	};
	for(i=0;i<4;i++){
		//vertices[ 3 * i + 0 ] *= 10;
		//vertices[ 3 * i + 1 ] *= 10;
	}
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(trans[0], trans[1], trans[2]);
	glRotatef(rot[0], 1.0f, 0.0f, 0.0f);
	glRotatef(rot[1], 0.0f, 1.0f, 0.0f);
	glRotatef(rot[2], 0.0f, 0.0f, 1.0f);
	//glRotatef(theta, 0.0f, 1.0f, 0.0f);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glDisable(GL_TEXTURE_2D);
	glVertexPointer(3,GL_FLOAT,0,vertices);
	glNormalPointer(GL_FLOAT,0,normals);
	glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_BYTE,indices);
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	theta+=1;
	return 0;
}
int render_cube()
{
	float rot[]={0,0,0};
	float trans[]={0,0,0};
	static theta=0;

	trans[0]=-.5;
	trans[1]=-.5;
	trans[2]=.5;
	render_rect(rot,trans); //front

	trans[0]=-.5;
	trans[1]=.5;
	trans[2]=-.5;
	rot[0]=180;
	render_rect(rot,trans); //back
	theta++;

	trans[0]=-.5;
	trans[1]=.5;
	trans[2]=.5;
	rot[0]=-90;
	render_rect(rot,trans); //top

	trans[0]=-.5;
	trans[1]=-.5;
	trans[2]=-.5;
	rot[0]=90;
	render_rect(rot,trans); //bottom

	trans[0]=.5;
	trans[1]=-.5;
	trans[2]=.5;
	rot[0]=0;
	rot[1]=90;
	render_rect(rot,trans); //right

	trans[0]=-.5;
	trans[1]=-.5;
	trans[2]=-.5;
	rot[0]=0;
	rot[1]=-90;
	render_rect(rot,trans); //left
}
int do_gfx()
{

	static float x,y,m,b,h;
	static float position[3],offset[3];

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	glColor3f(1.0,0.0,0.0);

	glMatrixMode(GL_MODELVIEW);
//	glMatrixMode(GL_PROJECTION);
	
	glLoadIdentity();
	glTranslatef(gx,gy,gz);
	glRotatef(grx,1,0,0);
	glRotatef(gry,0,1,0);
	glRotatef(grz,0,0,1);



	dude();


//	t1=GetTickCount();
//	printf("time=%u v=%f\n",GetTickCount()-t1,bike.v[0]);

	glFlush();
	glFinish();

	{
		static DWORD tick=0,t,delta;
		static float avg[60]={0};
		static int avg_count=0;
		float fps=0;
		int i;
		char str[12];
		t=GetTickCount();
		delta=t-tick;
		if(delta!=0)
			fps=((float)1/(float)delta)*(float)1000;
		avg[avg_count++]=fps;
		if(avg_count>=sizeof(avg)/sizeof(float))
			avg_count=0;
		fps=0;
		for(i=0;i<sizeof(avg)/sizeof(float);i++){
			fps+=avg[i];
		}
		fps/=sizeof(avg)/sizeof(float);
		_snprintf(str,sizeof(str),"FPS=%02.1f",fps);
		display_str(str,0,0);
		tick=t;	
	}
//	bezier();
	
	glBindTexture(GL_TEXTURE_2D, 13);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
//	glTexImage2D(GL_TEXTURE_2D, 0, 3, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, texture);
	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();
	glScalef(1.0/256, 1.0/256, 1);
	
	return 0;
}
int gl_swap_buffer(HDC hdc,HGLRC hglrc)
{
	if(hdc==0 || hglrc==0)
		return 0;
	wglMakeCurrent(hdc,hglrc);
	do_gfx();
	SwapBuffers(hdc);
	return 0;
}
