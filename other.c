#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>


/*
In visual studio 2015, stdin, stderr, stdout are defined as follow :

#define stdin  (__acrt_iob_func(0))
#define stdout (__acrt_iob_func(1))
#define stderr (__acrt_iob_func(2))

But previously, they were defined as:

#define stdin  (&__iob_func()[0])
#define stdout (&__iob_func()[1])
#define stderr (&__iob_func()[2])
*/

void *my_iob()
{
	static FILE _iob[3];
	_iob[0]=*stdin;
	_iob[1]=*stdout;
	_iob[2]=*stderr;
	return _iob;
}

void * _imp____iob_func=&my_iob;
void *_imp___snwprintf=&_snwprintf;

void *_imp___vsnprintf=&vsnprintf;
void *_imp__sprintf=&sprintf;
void *_imp__fprintf=&fprintf;
void *_imp___snprintf=&snprintf;
void *_imp__printf=&printf;

/*
int _imp___snprintf(void* buf,size_t len,const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	return vsnprintf(buf,len,fmt,ap);
}

int _imp__printf(const char *fmt,...)
{
	va_list ap;
	va_start(ap,fmt);
	return vprintf(fmt,ap);
}
*/