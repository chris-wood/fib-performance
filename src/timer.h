//
// Created by Christopher Wood on 12/9/16.
//

#ifndef FIB_PERF_TIMER_H
#define FIB_PERF_TIMER_H

#include <time.h>

struct timespec timerStart(void);
long timerEnd(struct timespec start_time);

#endif //FIB_PERF_TIMER_H
