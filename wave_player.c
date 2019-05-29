#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>
#include <stdarg.h>


static HWAVEOUT g_hwave_out=0;
static int g_sample_rate=0;
static int g_bytes=0;
static int g_channels=0;
static double g_speed=1.0;

typedef struct{
	BYTE *data;
	int data_len;
	WAVEHDR hdr;
}WAVE_BUF;

static WAVE_BUF wave_buf_list[3]={0};

static CRITICAL_SECTION g_mutex={0};
static int g_mutex_ready=0;

typedef struct{
	BYTE *data;
	int data_len;
	int sample_rate;
	int channels;
	int bytes;
	int do_loop;
	int loop_start;
	int loop_end;
	int play;
	double current_frame;
	double speed;
}WAVE_SAMPLE;

WAVE_SAMPLE g_sample_list[200]={0};

typedef struct{
	char *fname;
	BYTE *wav_data;
	int wav_len;
	int sample_rate;
	int channels;
	int bits;
}WAVE_FILE;

static int enter_mutex()
{
	if(g_mutex_ready){
		EnterCriticalSection(&g_mutex);
		return TRUE;
	}
	return FALSE;
}
static int leave_mutex()
{
	if(g_mutex_ready){
		LeaveCriticalSection(&g_mutex);
		return TRUE;
	}
	return FALSE;
}

static int fread_data(FILE *f,BYTE *buf,int len)
{
	int result=FALSE;
	int x;
	x=fread(buf,1,len,f);
	if(x==len)
		result=TRUE;
	return result;
}
static int log_error(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	vprintf(fmt,ap);
	return 0;
}
static int get32bit(BYTE *buf)
{
	int *ptr=(int*)buf;
	return ptr[0];
}
static int get16bit(BYTE *buf)
{
	unsigned short *ptr=(unsigned short*)buf;
	return ptr[0];
}

static int read_wave_file(WAVE_FILE *wfile)
{
	int result=FALSE;
	FILE *f;
	const char *fname;
	BYTE buf[128];
	int val;
	int sample_rate=0,channels=0,bits=0;
	enum{FIND_FMT,FIND_DATA};
	int state=FIND_FMT;
	fname=wfile->fname;
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
			int offset,delta;
			offset=ftell(f);
			delta=get32bit(buf+4);
			if(0==strncmp(buf,"fmt ",sizeof('fmt '))){
				state=FIND_DATA;
				if(fread_data(f,buf,16)){
					sample_rate=get32bit(buf+4);
					channels=get16bit(buf+2);
					bits=get16bit(buf+14);
				}
			}
			fseek(f,offset,SEEK_SET);
			fseek(f,delta,SEEK_CUR);
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
				wfile->wav_data=tmp;
				wfile->wav_len=val;
				wfile->sample_rate=sample_rate;
				wfile->channels=channels;
				wfile->bits=bits;
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

static SHORT clamp_short(LONG val)
{
	SHORT result=val;
	if(val>32767)
		result=32767;
	else if(val<-32768)
		result=-32768;
	return result;
}

static int check_loop(WAVE_SAMPLE *sample,double frame_size)
{
	int result=FALSE;
	double cpos;
	cpos=sample->current_frame;
	if(sample->do_loop){
		double start,end;
		start=sample->loop_start/frame_size;
		if(sample->loop_end)
			end=sample->loop_end/frame_size;
		else
			end=sample->data_len/frame_size;
		if(cpos>=end){
			cpos=start;
		}else if(cpos<start){
			cpos=end-1;
			if(cpos<0)
				cpos=0;
		}
	}else{
		double end;
		end=sample->data_len/frame_size;
		if(cpos>=end)
			sample->play=0;
		else if(cpos<0)
			sample->play=0;
	}
	sample->current_frame=cpos;
	return result;
}

static int clamp_current_frame(WAVE_SAMPLE *sample,double frame_size,double frame_count)
{
	int play=sample->play;
	if(sample->do_loop){
		check_loop(sample,frame_size);
	}else{
		if(sample->current_frame>=frame_count)
			play=FALSE;
		else if(sample->current_frame<0)
			play=FALSE;
		sample->play=play;
	}
	return TRUE;
}

static int mix_sample(BYTE *dst,int dst_size,WAVE_SAMPLE *sample)
{
	int i,count;
	double frame_size;
	int iframe_size;
	double frame_count;
	iframe_size=sample->bytes*sample->channels;
	if(iframe_size<=0)
		return FALSE;
	frame_size=iframe_size;
	frame_count=sample->data_len/frame_size;
	clamp_current_frame(sample,frame_size,frame_count);
	if(!(sample->play))
		return FALSE;
	count=dst_size;
	for(i=0;i<count;){
		SHORT *wdst,*wsrc;
		LONG a,b;
		int pos;
		int q;
		wdst=(SHORT*)(dst+i);
		pos=sample->current_frame*frame_size;
		q=pos%iframe_size;
		if(q)
			pos-=q;
		int src_index=0;
		//left channel
		wsrc=(SHORT*)(sample->data+pos);
		a=wdst[0];
		b=wsrc[src_index];
		a+=b;
		a=clamp_short(a);
		wdst[0]=a;
		if(sample->channels>1)
			src_index++;
		//right channel
		a=wdst[1];
		b=wsrc[src_index];
		a+=b;
		a=clamp_short(a);
		wdst[1]=a;
		sample->current_frame+=g_speed;
		check_loop(sample,frame_size);
		if(!sample->play)
			break;
		sample->current_frame=fmod(sample->current_frame,frame_count);
		i+=g_bytes*g_channels;
	}
	return TRUE;
}

static int mix_samples(BYTE *dst,int dst_size,WAVE_SAMPLE *list,int list_count)
{
	int i;
	memset(dst,0,dst_size);
	for(i=0;i<list_count;i++){
		WAVE_SAMPLE *sample;
		sample=&list[i];
		if(sample->play){
			mix_sample(dst,dst_size,sample);
		}
	}
	return TRUE;
}

void CALLBACK audio_callback(HWAVEOUT hwo,UINT msg,LPDWORD instance,LPDWORD param1,LPDWORD param2)
{
	LPWAVEHDR wh;
	BYTE *tmp;
	int tmp_len;
	int i;
	int sample_count;
	int wh_size=sizeof(WAVEHDR);
	wh=(LPWAVEHDR)param1;
	if(0==wh)
		return;
	tmp=wh->lpData;
	tmp_len=wh->dwBufferLength;
	if(0)
	{
		int i;
		i=tick_measure(1);
		printf("%i\n",i);
	}
	enter_mutex();
	//waveOutUnprepareHeader(hwo,wh,wh_size);
	waveOutPrepareHeader(hwo,wh,wh_size);
	sample_count=sizeof(g_sample_list)/sizeof(WAVE_SAMPLE);
	mix_samples(tmp,tmp_len,g_sample_list,sample_count);
	waveOutWrite(hwo,wh,wh_size);
	leave_mutex();
}

static int add_sample(WAVE_SAMPLE *wav)
{
	int result=FALSE;
	int i,count;
	enter_mutex();
	count=sizeof(g_sample_list)/sizeof(WAVE_SAMPLE);
	for(i=0;i<count;i++){
		WAVE_SAMPLE *tmp;
		tmp=&g_sample_list[i];
		if(!tmp->play){
			*tmp=*wav;
			tmp->play=1;
			break;
		}
	}
	leave_mutex();
	return result;
}

static int begin_audio_buffers(HWAVEOUT hwo)
{
	int result=FALSE;
	int i,count;
	int buf_count;
	count=sizeof(wave_buf_list)/sizeof(WAVE_BUF);
	for(i=0;i<count;i++){
		WAVE_BUF *wb;
		BYTE *tmp;
		int x,buf_size;
		wb=&wave_buf_list[i];
		if(wb->data){
			free(wb->data);
			wb->data=0;
			wb->data_len=0;
			memset(&wb->hdr,0,sizeof(wb->hdr));
		}
		x=g_bytes;
		buf_size=(g_sample_rate*g_channels*x*30)/1000;
		tmp=calloc(buf_size,1);
		if(tmp){
			wb->data=tmp;
			wb->data_len=buf_size;
			wb->hdr.dwBufferLength=buf_size;
			wb->hdr.lpData=tmp;
			wb->hdr.dwUser=i;
		}
	}
	buf_count=0;
	for(i=0;i<count;i++){
		WAVE_BUF *wb;
		wb=&wave_buf_list[i];
		if(wb->data){
			int wh_size=sizeof(wb->hdr);
			WAVEHDR *phdr=&wb->hdr;
			buf_count++;
			waveOutPrepareHeader(hwo,phdr,wh_size);
			waveOutWrite(hwo,phdr,wh_size);
		}
	}
	if(buf_count==count)
		result=TRUE;
	return result;
}

int start_audio(int sample_rate,int bits,int channels)
{
	int result=FALSE;
	WAVEFORMATEX wfmt={0};
	if(!g_mutex_ready){
		InitializeCriticalSection(&g_mutex);
		g_mutex_ready=1;
	}
	if(g_hwave_out){
		return TRUE;
	}
	g_sample_rate=sample_rate;
	g_bytes=bits>>3;
	g_channels=channels;
	wfmt.cbSize=sizeof(wfmt);
	wfmt.nBlockAlign=4;
	wfmt.nChannels=channels;
	wfmt.nSamplesPerSec=sample_rate;
	wfmt.wBitsPerSample=bits;
	wfmt.wFormatTag=1;
	waveOutOpen(&g_hwave_out,WAVE_MAPPER,&wfmt,(SIZE_T)&audio_callback,0,CALLBACK_FUNCTION);
	if(g_hwave_out){
		result=begin_audio_buffers(g_hwave_out);
	}
	return result;
}

int stop_audio()
{
	if(g_hwave_out){
		enter_mutex();
		waveOutClose(g_hwave_out);
		leave_mutex();
		g_hwave_out=NULL;
		return TRUE;
	}
	return FALSE;
}


WAVE_FILE mywavs[100]={0};

int load_wavs()
{
	int result=FALSE;
	WIN32_FIND_DATAA wfd={0};
	HANDLE hfind;
	int index=0;
	hfind=FindFirstFileA("wavs\\*.wav",&wfd);
	if(!hfind){
		return result;
	}
	result=TRUE;
	do{
		WAVE_FILE *wf;
		char tmp[255];
		if(strstr(wfd.cFileName,"example"))
			continue;
		_snprintf(tmp,sizeof(tmp),"wavs\\%s",wfd.cFileName);
		wf=&mywavs[index];
		wf->fname=tmp;
		if(read_wave_file(wf)){
			wf->fname=strdup(wfd.cFileName);
			index++;
			if(wf->channels!=2){
				printf("%s %i\n",wfd.cFileName,wf->channels);
			}
			if(wf->bits!=16){
				printf("%s %i\n",wfd.cFileName,wf->bits);
			}
			if(wf->sample_rate != 44100){
				printf("%s %i\n",wfd.cFileName,wf->sample_rate);
			}
		}else{
			wf->fname=0;
		}
	}while(FindNextFileA(hfind,&wfd));
	FindClose(hfind);
	printf("%i waves loaded\n",index);
	return result;
}

int play_index(int i,int loop)
{
	int count;
	WAVE_FILE *wf;
	WAVE_SAMPLE ws={0};
	count=sizeof(mywavs)/sizeof(WAVE_FILE);
	if(i<0 || i>=count)
		return 0;
	wf=&mywavs[i];
	if(0==wf->fname){
		printf("nothing at %i\n",i);
		return 0;
	}
	printf("playing %s [%i] %s\n",wf->fname,i,loop?"looping":"");
	ws.bytes=wf->bits>>3;
	ws.channels=wf->channels;
	ws.data=wf->wav_data;
	ws.data_len=wf->wav_len;
	ws.play=1;
	ws.sample_rate=wf->sample_rate;
	ws.speed=1;
	ws.do_loop=loop;
	if(g_speed<0){
		double frame_count=0;
		if(ws.bytes){
			frame_count=ws.data_len/(ws.bytes*ws.channels);
			ws.current_frame=frame_count-1;
		}
	}
	add_sample(&ws);
	count=sizeof(g_sample_list)/sizeof(WAVE_SAMPLE);
	int total=0;
	enter_mutex();
	for(i=0;i<count;i++){
		if(g_sample_list[i].play)
			total++;
	}
	leave_mutex();
	printf("total playing:%i\n",total);
	return 0;
}

int clear_all()
{
	int i,count;
	count=sizeof(g_sample_list) / sizeof(WAVE_SAMPLE);
	printf("stopping all\n");
	enter_mutex();
	for(i=0;i<count;i++){
		g_sample_list[i].play=0;
	}
	leave_mutex();
	return 0;
}

int mod_speed(double mod)
{
	g_speed+=mod;
	printf("speed=%f\n",g_speed);
	return 0;
}

int test_wave_player()
{
	load_wavs();

	start_audio(44100,16,2);
	printf("sleeping\npress esc to quit\n");
	while(1){
		static double last=0;
		int key;
		double amount=.1;
		static int index=0;
		key=getkey_wait();
		if(0x1b==key)
			break;
		if('Z'==key){
			int x=sizeof(mywavs) / sizeof(WAVE_FILE);
			index--;
			index%=x;
			if(index<0)
				index=x+index;
			play_index(index,0);
		}
		if('X'==key){
			index++;
			index%=sizeof(mywavs) / sizeof(WAVE_FILE);
			play_index(index,0);
		}
		if(VK_OEM_3==key){
			clear_all();
		}
		double mod=.1;
		if(key_down(VK_CONTROL))
			mod=.01;

		if(VK_LEFT==key){ //left
			mod_speed(-mod);
		}
		if(VK_RIGHT==key){ //right
			mod_speed(mod);
		}
		if(' '==key){
			play_index(index,0);
		}
		if('C'==key){
			play_index(index,1);
		}
		{
			printf("key=%02X,%c\n",key,key);
			//printf("speed=%f\n",g_speed);
			last=g_speed;
		}
	}
	stop_audio();
	return 0;
}


