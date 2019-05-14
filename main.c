#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <conio.h>

HWND ghconsole=0;
int _open_osfhandle(long,int);
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
	hcrt=_open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT);

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
int wave_pos=0;

#define BUF_LEN 1024
WORD buf1[BUF_LEN];
WORD buf2[BUF_LEN];
int reverse=0;

int fill_buf(WORD *buf,int buf_len)
{
	int pos=0;
	int i;
	WORD *src;
	int src_len=wave_len/2;
	src=(WORD*)wave_data;
	for(i=0;i<buf_len;i++){
		int offset;
		if(reverse)
			pos=wave_pos-i;
		else
			pos=wave_pos+i;
		offset=pos%src_len;
		if(offset<0)
			offset=src_len+offset;
		buf[i]=src[offset];
	}
	pos=(pos+1)%src_len;
	if(pos<0)
		pos=-pos;
	wave_pos=pos;
	return 0;
}

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
	fill_buf((WORD*)tmp,tmp_len/2);
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
int read_wave_file()
{
	int result=FALSE;
	FILE *f;
	f=fopen("example.wav","rb");
	if(0==f){
		printf("unable top open file\n");
		goto F_ERROR;
	}
	wave_len=get_flen(f);
	wave_data=malloc(wave_len);
	if(0==wave_data){
		printf("unable to allocate space for wave file\n");
		goto F_ERROR;
	}
	fread(wave_data,1,wave_len,f);
	result=TRUE;
F_ERROR:
	if(f)
		fclose(f);
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
	if(!read_wave_file())
		return 0;

	wf.wFormatTag=1;
	wf.nBlockAlign=4;
	wf.wBitsPerSample=16;
	wf.cbSize=sizeof(wf);
	wf.nChannels=2;
	wf.nSamplesPerSec=8000;
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
			audio_callback(hwo,0,0,tmp,0);
		}
	}else{
		printf("wave out failed\n");
	}
	printf("sleeping\npress esc to quit\n");
	while(1){
		static int last_rev=0;
		int key;
		Sleep(100);
		key=get_key();
		if(0x1b==key)
			break;

		if(key_down(VK_LEFT))
			reverse=1;
		else
			reverse=0;
		if(last_rev!=reverse){
			printf("rev=%i\n",reverse);
			last_rev=reverse;
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