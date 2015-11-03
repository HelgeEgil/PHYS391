#include <sys/time.h>
#include <stddef.h>
#include <signal.h>
#include <stdio.h>

// int setitimer(int which, const struct itimerval *new_value, struct itimerval *old_value);

// 1. arg: The type of timer
//    SIGALRM is only sent during Real Time timer, use which = ITIMER_REAL
//    Other: ITIMER_VIRTUAL (runs only when the process is in execution)
//    Other: ITIMER_PROF (runs during profiling)

// 3. arg: Read the values of an existing timer

// it_interval: time between signals
// it_value: time for first signal

void ping() {
   printf("SIGALRM called...\n");
}

void ctrlc() {
   printf("Cannot kill me...\n");
   (void) signal(SIGINT, SIG_DFL);
}

void main() {
   struct itimerval timer;
   timer.it_interval.tv_usec = 11000; // 0.75 sec
   timer.it_interval.tv_sec = 0; // 0.75 sec
   timer.it_value.tv_usec = 11000;
   timer.it_value.tv_sec = 0;

   signal(SIGALRM, ping);
   signal(SIGINT, ctrlc);

   setitimer(ITIMER_REAL, &timer, NULL);

   while(1);
}
