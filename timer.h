#pragma once

#define TIMER_TYPE ITIMER_REAL
#define TIMER_SIGNAL SIGALRM

// returns number of microseconds (usec) since the latests interrupt
extern int timer_cnt(void);

extern void timer_init(int ms, void (*hnd)(void));


