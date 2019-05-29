#include <stdio.h>
#include <stdlib.h>


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

void *_imp____iob_func=&my_iob;
void *_imp___snwprintf=&_snwprintf;

void *_imp___vsnprintf=&vsnprintf;
void *_imp__sprintf=&sprintf;
void *_imp__fprintf=&fprintf;
void *_imp___snprintf=&snprintf;
void *_imp__printf=&printf;
void *_imp__vfprintf=&vfprintf;
