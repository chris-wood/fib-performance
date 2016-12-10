//
// Created by Christopher Wood on 12/9/16.
//

#include "timer.h"

struct timespec
timerStart(void)
{
    struct timespec start_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
    return start_time;
}

long
timerEnd(struct timespec start_time)
{
    struct timespec end_time;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
    long diffInNanos = ((end_time.tv_sec - start_time.tv_sec) * 10000000000L) + (end_time.tv_nsec - start_time.tv_nsec);
    return diffInNanos;
}