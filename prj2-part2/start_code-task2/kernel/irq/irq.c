#include "irq.h"
#include "time.h"
#include "sched.h"
#include "string.h"

static void irq_timer()
{
    // TODO clock interrupt handler.
    // scheduler, time counter in here to do, emmmmmm maybe.
    time_elapsed += (INT_TIME + INT_TIME);
    screen_reflush();
    scheduler();
}

void other_exception_handler()
{
    // TODO other exception handler
}

void interrupt_helper(uint32_t status, uint32_t cause)
{
    uint32_t execode = cause & 0x0000007c;
	uint32_t IP = cause & 0x0000ff00;
	if(execode == 0x00000000 && IP == 0x00008000)
		irq_timer();
	else
		irq_timer();//other_exception_handler();
	return ;
    // TODO interrupt handler.
    // Leve3 exception Handler.
    // read CP0 register to analyze the type of interrupt.
}

