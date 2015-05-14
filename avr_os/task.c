/*
 * task.c
 *
 * Created: 5/10/2015 11:51:22 PM
 *  Author: eu
 */ 
#include <avr/io.h>
#include <stdio.h>
#include <avr/interrupt.h>

#include "task.h"

unsigned int thread_count = 0;
unsigned int oppened_threads_nr = 0;
thread_data task_table[MAX_THREADS_NR];
thread_data *current_task_data = NULL;
void *kernel_stak;

ISR(TASK_ISR, ISR_NAKED)
{
/*
 * Save all registers, special registers and SREG.
 */
	__asm__ __volatile__
	(
	"push r0 \n\t"
	"push r1 \n\t"
	"push r2 \n\t"
	"push r3 \n\t"
	"push r4 \n\t"
	"push r5 \n\t"
	"push r6 \n\t"
	"push r7 \n\t"
	"push r8 \n\t"
	"push r9 \n\t"
	"push r10 \n\t"
	"push r11 \n\t"
	"push r12 \n\t"
	"push r13 \n\t"
	"push r14 \n\t"
	"push r15 \n\t"
	"push r16 \n\t"
	"push r17 \n\t"
	"push r18 \n\t"
	"push r19 \n\t"
	"push r20 \n\t"
	"push r21 \n\t"
	"push r22 \n\t"
	"push r23 \n\t"
	"push r24 \n\t"
	"push r25 \n\t"
	"push r26 \n\t"
	"push r27 \n\t"
	"push r28 \n\t"
	"push r29 \n\t"
	"push r30 \n\t"
	"push r31 \n\t"
	"in r16, %5 \n\t"
	"push r16 \n\t"
	"in r16, %0 \n\t"
	"push r16 \n\t"
	"in r16, %1 \n\t"
	"push r16 \n\t"
	"in r16, %2 \n\t"
	"push r16 \n\t"
	"in r16, %3 \n\t"
	"push r16 \n\t"
	"in r16, %4 \n\t"
	"push r16 \n\t"
	:
	: "I" (_SFR_IO_ADDR(EIND)),
	  "I" (_SFR_IO_ADDR(RAMPZ)),
	  "I" (_SFR_IO_ADDR(RAMPY)),
	  "I" (_SFR_IO_ADDR(RAMPX)),
	  "I" (_SFR_IO_ADDR(RAMPD)),
	  "I" (_SFR_IO_ADDR(SREG))
	);
/*
 * Save STAK pointer.
 */
	void *__current_thread_stack_addr = current_task_data->stack_addr;
	__asm__ __volatile__
	(
	"in r18, %1 \n\t"
	"in r19, %2 \n\t"
	"st %a0+, R16 \n\t"
	"st %a0, R17 \n\t"
	:
	: "e" (__current_thread_stack_addr),
	  "I" (_SFR_IO_ADDR(SPL)),
	  "I" (_SFR_IO_ADDR(SPH))
	);
/*
 * Change the current task with the next task, is a round robin function.
 */
	do
	{
		thread_count++;
		if(thread_count >= MAX_THREADS_NR)
		{
			thread_count = 0;
		}
	} while (!task_table[thread_count].stack_addr);
	__current_thread_stack_addr = (void *)&task_table[thread_count].stack_addr;
	current_task_data->stack_addr = __current_thread_stack_addr;
	
	
/*
 * Restore STAK pointer.
 */
	__asm__ __volatile__
	(
	"ld R17, -%a0 \n\t"
	"ld R16, %a0 \n\t"
	"out %2, R19 \n\t"
	"out %1, R18 \n\t"
	:
	: "e" (__current_thread_stack_addr),
	  "I" (_SFR_IO_ADDR(SPL)),
	  "I" (_SFR_IO_ADDR(SPH))
	);

/*
 * Restore all registers, special registers and SREG.
 */
	__asm__ __volatile__
	(
	"pop r16 \n\t"
	"out %4, r16 \n\t"
	"pop r16 \n\t"
	"out %3, r16 \n\t"
	"pop r16 \n\t"
	"out %2, r16 \n\t"
	"pop r16 \n\t"
	"out %1, r16 \n\t"
	"pop r16 \n\t"
	"out %0, r16 \n\t"
	"pop r16 \n\t"
	"out %5, r16 \n\t"
	"pop r31 \n\t"
	"pop r30 \n\t"
	"pop r29 \n\t"
	"pop r28 \n\t"
	"pop r27 \n\t"
	"pop r26 \n\t"
	"pop r25 \n\t"
	"pop r24 \n\t"
	"pop r23 \n\t"
	"pop r22 \n\t"
	"pop r21 \n\t"
	"pop r20 \n\t"
	"pop r19 \n\t"
	"pop r18 \n\t"
	"pop r17 \n\t"
	"pop r16 \n\t"
	"pop r15 \n\t"
	"pop r14 \n\t"
	"pop r13 \n\t"
	"pop r12 \n\t"
	"pop r11 \n\t"
	"pop r10 \n\t"
	"pop r9 \n\t"
	"pop r8 \n\t"
	"pop r7 \n\t"
	"pop r6 \n\t"
	"pop r5 \n\t"
	"pop r4 \n\t"
	"pop r3 \n\t"
	"pop r2 \n\t"
	"pop r1 \n\t"
	"pop r0 \n\t"
	:
	: "I" (_SFR_IO_ADDR(EIND)),
	  "I" (_SFR_IO_ADDR(RAMPZ)),
	  "I" (_SFR_IO_ADDR(RAMPY)),
	  "I" (_SFR_IO_ADDR(RAMPX)),
	  "I" (_SFR_IO_ADDR(RAMPD)),
	  "I" (_SFR_IO_ADDR(SREG))
	);
/*
 * This is an naked ISR, force return.
 */
	__asm__ __volatile__
	(
	"reti"
	);
}
