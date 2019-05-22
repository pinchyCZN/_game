#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <stdarg.h>

HWND ghconsole=0;
int _open_osfhandle(HANDLE,int);
static DWORD(WINAPI *SetConsoleIcon)(HICON)=0;
#define _O_TEXT         0x4000  /* file mode is text (translated) */

void open_console()
{
	HWND hcon;
	FILE *hf;
	static BYTE consolecreated=FALSE;
	static int hcrt=0;
	static HWND (WINAPI *GetConsoleWindow)(void)=0;

	if(consolecreated==TRUE)
	{
		if(ghconsole!=0)
			ShowWindow(ghconsole,SW_SHOW);
		hcon=(HWND)GetStdHandle(STD_INPUT_HANDLE);
		FlushConsoleInputBuffer(hcon);
		return;
	}
	AllocConsole();
	hcrt=_open_osfhandle(GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT);

	fflush(stdin);
	hf=_fdopen(hcrt,"w");
	*stdout=*hf;
	setvbuf(stdout,NULL,_IONBF,0);
	consolecreated=TRUE;
	if(GetConsoleWindow==0){
		HMODULE hmod=LoadLibrary(TEXT("kernel32.dll"));
		if(hmod!=0){
			GetConsoleWindow=(HWND (WINAPI *)(void))GetProcAddress(hmod,"GetConsoleWindow");
			if(GetConsoleWindow!=0){
				ghconsole=GetConsoleWindow();
			}
			SetConsoleIcon=(DWORD(WINAPI *)(HICON))GetProcAddress(hmod,"SetConsoleIcon");
		}
	}

}


int tick_measure(int end)
{
	int result=0;
	static LARGE_INTEGER tick={0};
	LARGE_INTEGER tmp;
	QueryPerformanceCounter(&tmp);
	if(end){
		static LARGE_INTEGER freq={0};
		static int have_freq=0;
		LARGE_INTEGER val;
		val.QuadPart=tmp.QuadPart-tick.QuadPart;
		if(!have_freq){
			QueryPerformanceFrequency(&freq);
			have_freq=1;
			if(0==freq.QuadPart)
				return 0;

		}
		val.QuadPart=(val.QuadPart*1000)/freq.QuadPart;
		result=(int)val.QuadPart;
	}
	tick.QuadPart=tmp.QuadPart;
	return result;
}



BYTE *wave_data=0;
int wave_len=0;
double wave_pos=0;
double wave_speed=0;

#define BUF_LEN 4096
WORD buf1[BUF_LEN];
WORD buf2[BUF_LEN];

int fread_data(FILE *f,BYTE *buf,int len)
{
	int result=FALSE;
	int x;
	x=fread(buf,1,len,f);
	if(x==len)
		result=TRUE;
	return result;
}
int log_error(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	vprintf(fmt,ap);
	return 0;
}
int get32bit(BYTE *buf)
{
	int *ptr=(int*)buf;
	return ptr[0];
}
int get16bit(BYTE *buf)
{
	unsigned short *ptr=(unsigned short*)buf;
	return ptr[0];
}

int read_wave_file(const char *fname,BYTE **dst,int *dst_len)
{
	int result=FALSE;
	FILE *f;
	BYTE buf[128];
	int val;
	enum{FIND_FMT,FIND_DATA};
	int state=FIND_FMT;
	f=fopen(fname,"rb");
	if(0==f){
		printf("unable to open file:%s\n",fname);
		return result;
	}
	if(!fread_data(f,buf,12)){
		log_error("unable to read file:%s\n",fname);
	}
	if(0!=strncmp(buf,"RIFF",sizeof('RIFF'))){
		printf("invalid wave header\n");
		goto WAVE_ERROR;
	}
	if(0!=strncmp(buf + 8,"WAVE",sizeof('WAVE'))){
		printf("invalid wave header\n");
		goto WAVE_ERROR;
	}
	while(1){
		if(!fread_data(f,buf,8)){
			log_error("unable to read file:%s\n",fname);
		}
		if(FIND_FMT==state){
			if(0==strncmp(buf,"fmt ",sizeof('fmt '))){
				state=FIND_DATA;
			}
			val=get32bit(buf+4);
			fseek(f,val,SEEK_CUR);
		}else if(FIND_DATA==state){
			val=get32bit(buf + 4);
			if(0 == strncmp(buf,"data",sizeof('data'))){
				BYTE *tmp;
				tmp=malloc(val);
				if(0==tmp){
					log_error("unable to allocate space [%u] for wave file %s\n",val,fname);
					goto WAVE_ERROR;
				}
				if(!fread_data(f,tmp,val)){
					log_error("unable to read all data [%u] from wave file %s\n",val,fname);
					free(tmp);
					goto WAVE_ERROR;
				}
				*dst=tmp;
				*dst_len=val;
				result=TRUE;
				break;
			}
			fseek(f,val,SEEK_CUR);
		}else{
			log_error("state error\n");
			goto WAVE_ERROR;
		}
	}
WAVE_ERROR:
	fclose(f);
	return result;
}

int write_wave(DWORD *dest,int dest_count,DWORD *src,int src_count,double *src_pos,double speed)
{
	int i;
	double fpos;
	int pos;
	int count;
	if(0==src_count){
		memset(dest,0,dest_count*2);
		return 0;
	}
	count=min(dest_count,src_count);
	fpos=*src_pos;
	fpos=fmod(fpos,src_count);
	for(i=0;i<count;i++){
		pos=fpos;
		if(pos>src_count){
			pos%=src_count;
		}else if(pos<0){
			pos=-pos;
			pos%=src_count;
			pos=src_count-pos;
		}
		dest[i]=src[pos];
		fpos+=speed;
	}
	*src_pos=fpos;
	if(i<dest_count){
		for( ;i<dest_count;i++){
			dest[i]=0;
		}
	}
	return 0;
}

//int mix_wave(WORD *dest,int dest_count,DWORD *src,)

void CALLBACK audio_callback(HWAVEOUT hwo,UINT msg,LPDWORD instance,LPDWORD param1,LPDWORD param2)
{
	LPWAVEHDR wh;
	BYTE *tmp;
	int tmp_len;
	int i;
	int wh_size=sizeof(WAVEHDR);
	wh=(LPWAVEHDR)param1;
	if(0==wh)
		return;
	tmp=wh->lpData;
	tmp_len=wh->dwBufferLength;
	write_wave((DWORD*)tmp,tmp_len/4,(DWORD*)wave_data,wave_len/4,&wave_pos,wave_speed);
	if(0)
	{
		int i;
		i=tick_measure(1);
		printf("%i\n",i);
	}
	waveOutUnprepareHeader(hwo,wh,wh_size);
	waveOutPrepareHeader(hwo,wh,wh_size);
	waveOutWrite(hwo,wh,wh_size);
}

int do_wait()
{
	const char *p=getenv("PROMPT");
	if(0==p){
		printf("press any key\n");
		getch();
	}
	return 0;
}
int get_key()
{
	int result=0;
	if(kbhit()){
		int x=getch();
		if(0xE0==x)
			x=getch();
		result=x;
	}
	return result;
}
int getkey_wait()
{
	int key;
	key=getch();
	if(0xE0==key)
		key=getch();
	return key;
}
int get_flen(FILE *f)
{
	int result;
	int pos;
	pos=ftell(f);
	fseek(f,0,SEEK_END);
	result=ftell(f);
	fseek(f,pos,SEEK_SET);
	return result;
}
int key_down(int key)
{
	int val;
	val=GetKeyState(key);
	if(val&0x8000)
		return TRUE;
	else
		return FALSE;
}

int main(int argc,char **argv)
{

	HWAVEOUT hwo;
	WAVEFORMATEX wf={0};
	WAVEHDR wh[2]={0};
	if(!read_wave_file("example.wav",&wave_data,&wave_len))
		return 0;

	wf.wFormatTag=1;
	wf.nBlockAlign=4;
	wf.wBitsPerSample=16;
	wf.cbSize=sizeof(wf);
	wf.nChannels=2;
	wf.nSamplesPerSec=44100;
	waveOutOpen(&hwo,WAVE_MAPPER,&wf,(SIZE_T)&audio_callback,0,CALLBACK_FUNCTION);
	if(hwo){
		int i,count;
		BYTE *blist[2]={(LPBYTE)&buf1,(LPBYTE)&buf2};
		count=sizeof(wh)/sizeof(WAVEHDR);
		for(i=0;i<count;i++){
			LPWAVEHDR tmp;
			tmp=&wh[i];
			wh[i].dwBufferLength=sizeof(buf1);
			wh[i].lpData=(LPSTR)blist[i&1];
			wh[i].dwUser=i+1;
			audio_callback(hwo,0,0,(LPDWORD)tmp,0);
		}
	}else{
		printf("wave out failed\n");
	}
	printf("sleeping\npress esc to quit\n");
	while(1){
		static double last=0;
		int key;
		double amount=.1;
		key=getkey_wait();
		if(0x1b==key)
			break;
		if(key_down(VK_CONTROL))
			amount=.01;
		if(key_down(VK_LEFT))
			wave_speed-=amount;
		else if(key_down(VK_RIGHT))
			wave_speed+=amount;
		//if(last!=wave_speed)
		{
			printf("key=%i\n",key);
			printf("speed=%f\n",wave_speed);
			last=wave_speed;
		}
	}
	waveOutClose(hwo);
	return 0;
}



int WINAPI WinMain(HINSTANCE hinst,HINSTANCE hprev,LPSTR cmd_line,int cmd_show)
{
	open_console();
	main(0,0);
	//do_wait();
	return 0;
}