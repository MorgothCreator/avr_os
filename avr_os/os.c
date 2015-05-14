/*
 * os.c
 *
 * Created: 5/11/2015 1:44:03 AM
 *  Author: eu
 */ 

#include <avr/io.h>
#include <stdbool.h>
#include "os.h"
#include "task.h"

void os_init(void)
{
	
}

bool os_task_add(void *address)
{
	return false;
}

bool os_task_close(unsigned int pid)
{
	unsigned int task_cnt;
	for (task_cnt = 0; task_cnt < MAX_THREADS_NR; task_cnt++)
	{
		if(task_table[task_cnt].pid == pid)
		{
			task_table[task_cnt].request_to_close = true;
			task_table[task_cnt].watchdog = 0;
		}
	}
	return true;
}

bool os_task_remove(unsigned int pid)
{
	return false;
}