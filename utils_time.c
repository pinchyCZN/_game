#include <windows.h>

__int64 get_time()
{
	__int64 result=0;
	LARGE_INTEGER tmp;
	static LARGE_INTEGER freq={0};
	static int have_freq=0;
	QueryPerformanceCounter(&tmp);
	if(!have_freq){
		QueryPerformanceFrequency(&freq);
		have_freq=1;
	}
	if(0!=freq.QuadPart){
		result=(tmp.QuadPart*1000)/freq.QuadPart;
	}
	return result;

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
