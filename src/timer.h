#ifdef __cplusplus
extern "C" {
#endif


#ifndef FIB_PERF_TIMER_H
#define FIB_PERF_TIMER_H

#include <time.h>

typedef struct timespec Timestamp;

Timestamp timerStart(void);
long timerEnd(Timestamp startTime);

#endif //FIB_PERF_TIMER_H

#ifdef __cplusplus
}
#endif
