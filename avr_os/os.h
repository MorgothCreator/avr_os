/*
 * os.h
 *
 * Created: 5/11/2015 1:44:11 AM
 *  Author: eu
 */ 


#ifndef OS_H_
#define OS_H_

#include <stdbool.h>

#ifndef THREAD_WATCHDOG
#define THREAD_WATCHDOG	1000
#endif

typedef struct
{
	unsigned int stak_size;
	unsigned int attribute;
	unsigned int version;
	signed char priority;
	unsigned char name[11];
}task_header;

void os_init(void);
bool os_task_add(void *address);
bool os_task_close(unsigned int pid);
bool os_task_remove(unsigned int pid);

#endif /* OS_H_ */