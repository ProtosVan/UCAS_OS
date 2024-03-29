/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *         The kernel's entry, where most of the initialization work is done.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE. 
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "irq.h"
#include "test.h"
#include "stdio.h"
#include "sched.h"
#include "screen.h"
#include "common.h"
#include "syscall.h"

/* ready queue to run */
queue_t ready_queue;

/* block queue to wait */
queue_t block_queue[2];

// sleep queue
queue_t sleep_queue;

int prio[16][2];

int exception_handler[32];

static void init_pcb()
{
	queue_init(&ready_queue);
	queue_init(&block_queue[0]);
	queue_init(&block_queue[1]);
	queue_init(&sleep_queue);
	int i;
	int j;
	current_running = &pcb[0];
	process_id = 1;
	pcb[0].pid = 1;
	pcb[0].status = TASK_EXITED;
	pcb[0].cursor_x = 0;
	pcb[0].cursor_y = 0;
	for(i = 1; i <= 31; i++){
		pcb[0].kernel_context.regs[i] = 0;
	}
	pcb[0].kernel_context.regs[29] = 0xa0f01000;
	pcb[0].kernel_context.cp0_status = 0x8002;
    pcb[0].kernel_context.hi = 0;
    pcb[0].kernel_context.lo = 0;
    pcb[0].kernel_context.cp0_badvaddr = 0;
    pcb[0].kernel_context.cp0_cause = 0;
    pcb[0].kernel_context.cp0_epc = 0xa0800200;
	for(j = 1; j <= num_final_test; j++){

		pcb[j].pid = j + 1;
		pcb[j].status = TASK_READY;
		pcb[j].cursor_x = 0;
		pcb[j].cursor_y = 0;
		for(i = 1; i <= 31; i++){
			pcb[j].user_context.regs[i] = 0;
		}
		pcb[j].user_context.regs[29] = 0xa0f00000 - j * 0x1000; //sp
		pcb[j].user_context.cp0_status = 0x8002;
    	pcb[j].user_context.hi = 0;
    	pcb[j].user_context.lo = 0;
    	pcb[j].user_context.cp0_badvaddr = 0;
    	pcb[j].user_context.cp0_cause = 0;
   		pcb[j].user_context.cp0_epc = final_test[j - 1]->entry_point;
		pcb[j].cursor_x = 0;
		pcb[j].cursor_y = 0;
		pcb[j].sleep_due = -1;
		pcb[j].status = TASK_READY;
		prio[j][0] = j;
		prio[j][1] = 0;
		queue_push(&ready_queue, &pcb[j]);
	}
}

static void init_exception_handler()
{
	int i;
	exception_handler[0] = (uint32_t)&(handle_int);
	for(i = 1; i < 32; i++) {
		if(i != 8)
			exception_handler[i] = (uint32_t)&(handle_other);
		else 
			exception_handler[i] = (uint32_t)&(handle_syscall);
	}
}

static void init_exception()
{	
	CLOSE_INTER();
	uint8_t *begin = (void *)exception_handler_begin;
	uint8_t *end	= (void *)exception_handler_end;
	memcpy((void *)0x80000180, begin, (end-begin));

	init_exception_handler();
	// 1. Get CP0_STATUS
	// 2. Disable all interrupt
	// 3. Copy the level 2 exception handling code to 0x80000180
	// 4. reset CP0_COMPARE & CP0_COUNT register
}

static void init_syscall(void)
{
	// init system call table.
	int i = 0;
	for(i = 0; i < NUM_SYSCALLS; i++){
		syscall[i] = 0;
	}
	syscall[SYSCALL_SLEEP] = (int (*)())&(do_sleep);
	
	syscall[SYSCALL_BLOCK] = (void (*))&(do_block);
	syscall[SYSCALL_UNBLOCK_ONE] = (int (*)())&(do_unblock_one);
	syscall[SYSCALL_UNBLOCK_ALL] =(int (*)())&(do_unblock_all);
	
	syscall[SYSCALL_WRITE] = (int (*)())&(screen_write);


	syscall[SYSCALL_CURSOR] = (int (*)())&(screen_move_cursor);
	syscall[SYSCALL_REFLUSH] = (int (*)())&(screen_reflush);
	
	syscall[SYSCALL_MUTEX_LOCK_INIT] = (int (*)())&(do_mutex_lock_init);
	syscall[SYSCALL_MUTEX_LOCK_ACQUIRE] = (int (*)())&(do_mutex_lock_acquire);
	syscall[SYSCALL_MUTEX_LOCK_RELEASE] = (int (*)())&(do_mutex_lock_release);
}

// jump from bootloader.
// The beginning of everything >_< ~~~~~~~~~~~~~~
void __attribute__((section(".entry_function"))) _start(void)
{
	// Close the cache, no longer refresh the cache 
	// when making the exception vector entry copy
	asm_start();

	// init interrupt (^_^)
	init_exception();
	printk("> [INIT] Interrupt processing initialization succeeded.\n");

	// init system call table (0_0)
	init_syscall();
	printk("> [INIT] System call initialized successfully.\n");

	// init Process Control Block (-_-!)
	init_pcb();
	printk("> [INIT] PCB initialization succeeded.\n");

	// init screen (QAQ)
	init_screen();

	printk("> [INIT] SCREEN initialization succeeded.\n");
	// TODO Enable interrupt
	OPEN_INTER();
	while (1)
	{
		// (QAQQQQQQQQQQQ)
		// If you do non-preemptive scheduling, you need to use it to surrender control
		

	};
	return;
}
