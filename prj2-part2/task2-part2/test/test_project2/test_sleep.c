#include "test2.h"
#include "sched.h"
#include "stdio.h"
#include "syscall.h"

static char blank[] = {"                                                "};

void sleep_task(void)
{
    int i;
    int print_location = 1;
    int sleep_time = 5;

    while (1)
    {
        for (i = 0; i < 20; i++)
        {
            sys_move_cursor(1, print_location);
            printf("> [TASK] This task is to test sleep(). (%d)  \n", i);
        }

        sys_move_cursor(1, print_location);
        printf("> [TASK] This task is sleeping, sleep time is %d.\n", sleep_time);

        /* call syscall sleep() */
        sys_sleep(sleep_time);

        sys_move_cursor(1, print_location);
        printf("%s", blank);
    }
}