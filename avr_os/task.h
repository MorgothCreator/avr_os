/*
 * task.h
 *
 * Created: 5/10/2015 11:51:13 PM
 *  Author: eu
 */ 


#ifndef TASK_H_
#define TASK_H_

#include <stdbool.h>
#include "avr_os.h"

#ifndef MAX_THREADS_NR
#define MAX_THREADS_NR	10
#endif

#ifndef TASK_ISR
#define TASK_ISR	TCC0_CCA_vect
#endif


typedef struct
{
	void *app_addr;
	void *stack_addr;
	void *directory_addr;
	unsigned int pid;
	unsigned int watchdog;
	unsigned char task_status;
	bool request_to_close;
}thread_data;

extern unsigned int thread_count;
extern unsigned int oppened_threads_nr;
extern thread_data task_table[];
extern thread_data *current_task_data;
extern void *kernel_stak;

#endif /* TASK_H_ */