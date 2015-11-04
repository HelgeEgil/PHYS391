#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#define ST_BASE (0x3F003000) // For BCM2836 chipset
#define TIMER_OFFSET (4)

// from wiringPI.com
void delayMicrosecondsWiringPi(unsigned long howLong) {
   struct timeval tNow, tLong, tEnd;

   gettimeofday(&tNow, NULL);
   tLong.tv_sec  = howLong / 1000000;
   tLong.tv_usec = howLong % 1000000;

   // so that &tNow != &tEnd?
   timeradd(&tNow, &tLong, &tEnd);

   while (timercmp(&tNow, &tEnd, <))
      gettimeofday(&tNow, NULL);
}

void delayMicrosecondsNanosleep(unsigned long howLong) {
   struct timespec nanotime, dummy;
   nanotime.tv_sec  = (howLong / 1000000);
   nanotime.tv_nsec = (howLong % 1000000) * 1000;
   nanosleep(&nanotime, &dummy);
}

int main(int argc, char *argv[]) {
   long long int t, d, prev, *timer; // 64 bit timer
   int fd, i;
   void *st_base; // byte ptr to simplify offset math


   fd = open("/dev/mem", O_RDONLY);
   if (fd == -1) {
      fprintf(stderr, "open() failed.\n");
      return 255;
   }

   st_base = mmap(NULL, 4096, PROT_READ, MAP_SHARED, fd, ST_BASE);
   if (st_base == MAP_FAILED) {
      fprintf(stderr, "mmap() failed.\n");
      return 254;
   }

   // set up pointer based on mapped page
   timer = (long long int *) ((char *) st_base + TIMER_OFFSET);

   printf("Sleeping using nanosleep method: \n");

   prev = *timer;
   delayMicrosecondsNanosleep(0);
   for (i=0; i<10; i++) {
      d = 20 * i;
      t = *timer;
      printf("Sleeping for %lld us: Actual time %lld     \n", d, t - prev);
      prev = t;
      delayMicrosecondsNanosleep(d);
   }

   printf("Sleeping using gettimeofday method: \n");

   prev = *timer;
   delayMicrosecondsWiringPi(0);
   for (i=0; i<10; i++) {
      d = 20 * i;
      t = *timer;
      printf("Sleeping for %lld us: Actual time: %lld      \n", d, t-prev);
      prev = t;
      delayMicrosecondsWiringPi(d);
   }

   return 0;
}
