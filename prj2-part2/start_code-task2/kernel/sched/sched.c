#include "lock.h"
#include "time.h"
#include "stdio.h"
#include "sched.h"
#include "queue.h"
#include "screen.h"

pcb_t pcb[NUM_MAX_TASK];

/* current running task PCB */
pcb_t *current_running;

/* global process id */
pid_t process_id = 1;



static void check_sleeping()
{
    pcb_t *checkme = sleep_queue.head;
	pcb_t *next;
	if(queue_is_empty(&sleep_queue)){
		return ;
	}
	while(checkme != NULL){
		if(get_timer() >= checkme->sleep_due){
			next = (pcb_t *)queue_remove(&sleep_queue, (void*)checkme);
			checkme->sleep_due = -1;
			checkme->status = TASK_READY;
			queue_push(&ready_queue, checkme);
			checkme = next;
		}else{
			checkme = checkme->next;
		}
	}
}

void scheduler(void)
{
    if(!queue_is_empty(&sleep_queue)){
		check_sleeping();
	}
    current_running->cursor_x = screen_cursor_x;
    current_running->cursor_y = screen_cursor_y;
    if(current_running->status == TASK_RUNNING || current_running->status == TASK_READY) {
        current_running->status = TASK_READY;
        queue_push(&ready_queue, current_running);
    }

    int maxpriority = -1;
    int temptime = get_timer();
    int tempprio;
    int i =1;

    for(; i<= 7;i++) {
        if(pcb[i].status !=TASK_READY) continue;
        tempprio = prio[i][0] + temptime - prio[i][1];
        if(tempprio > maxpriority) {
            maxpriority = tempprio;
            current_running = &pcb[i];
        }
    }
    prio[current_running->pid - 1][1]= temptime;

    //current_running = &pcb[7];

    //printk("test\n");
    queue_remove(&ready_queue, current_running);
    current_running->status = TASK_RUNNING;

    screen_cursor_x = current_running->cursor_x;
    screen_cursor_y = current_running->cursor_y;
}

void do_sleep(uint32_t sleep_time)
{
    current_running->status = TASK_BLOCKED;
	current_running->sleep_due = get_timer() + sleep_time;
	//current_running->user_context.cp0_epc = current_running->user_context.cp0_epc + 4;
	queue_push(&sleep_queue, (void*)current_running);
	scheduler();
	return ;
    // TODO sleep(seconds)
}

void do_block(queue_t * queue)
{
    // block the current_running task into the queue
    current_running->status = TASK_BLOCKED;
    queue_push(queue, (void*)current_running);
    scheduler();
}

void do_unblock_one(queue_t *queue)
{
    // unblock the head task from the queue
    pcb_t *unblock_one;
    unblock_one = queue_dequeue(queue);
    unblock_one->status = TASK_READY;
    queue_push(&ready_queue, unblock_one);
}

void do_unblock_all(queue_t *queue)
{
    // unblock all task in the queue
}
