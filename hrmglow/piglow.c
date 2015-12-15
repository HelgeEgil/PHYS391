#include "piglow.h"
#include <math.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <piGlow.h>
#include <unistd.h>

#include <string.h>
#include <curses.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MESSAGE_LINE 6
#define ERROR_LINE 22
#define Q_LINE 20
#define PROMPT_LINE 18

#define INPUT_FIFO_NAME "./hrm_pipe"
#define INPUT_PYTHON_NAME "./hrm_read.py"

volatile sig_atomic_t stop;
volatile int hrm;

int getOneHR() {
    int hr =0;
    int input_filestream = open(INPUT_FIFO_NAME, (O_RDONLY));
    if (input_filestream != -1) {
        unsigned char rx_buffer[256];
        int rx_length = read(input_filestream, (void*) rx_buffer, 255);

        if (rx_length<0) {
        }

        else if (rx_length == 0) {
        }

        else {
           rx_buffer[rx_length] = '\0';
           if (rx_length > 1) {
              hr = atoi(rx_buffer);
           }
        }
    }
    (void) close(input_filestream);
    return hr;
}

void sig_stop(int sig_number) {
   stop = 1;
}

void sig_abort(int sig_number) {
   printf("CTRL-C detected, aborting.\n");
   deactivateAll();
   endwin();
   exit(1);
}

float gauss(float x, float mu, float sigma) {
   return exp( -pow( x - mu, 2) / (2 * pow(sigma,2)) ); // sigma = 75 ms
}

void activateAll() {
   int i;

   for (i=0; i<3; i++) {
      piGlowLeg(i, 100);
   }
}

void deactivateAll() {
   int i;

   for (i=0; i<3; i++) {
      piGlowLeg(i, 0);
   }
}

void pulseAllLEDs(int speed) {
   float t, tLed;
   int timer, led, leg;

   int intensity = 255;
   
   for (timer=0; timer<speed*3.14; timer++) {
      t = timer / (float) speed;
      for (leg=0; leg<3; leg++) {
         for (led=0; led<6; led++) {
            tLed = t - led/6.0;
            if (tLed<0 || tLed>2) tLed = 0;
            piGlow1(leg, (5-led), abs(intensity * pow(sin(tLed * 1.57),3)));
            delay(1);
         }
      }
   }
}

void pulseHR() {
   float t;
   int timer, led, leg, pulse, HR;

   int nPulses = 3;
   float intensity, g;
   int maxIntensity = 30;

   // receive one HR event
   while (1) {
      HR = getOneHR();
      if (!HR) {
         sleep(1);
      }
      else {
         break;
      }
   }

   float dt = 60000 / (float) HR; // ms between pulses
   float sigma = (350-HR) / (float) 3;

   for (timer=0; timer<1000; timer++) {
      for (leg=0; leg<3; leg++) {
         for (led=0; led<6; led++) {
            t = timer - led/6.0;
            if (t<0) t = 0;
            
            intensity = 0;
            for (pulse=0; pulse < nPulses; pulse++) {
               g = gauss(t*31.4, 1.5*sigma + pulse * dt, sigma);
               intensity += g;
            }

            piGlow1(leg, (5-led), maxIntensity * intensity);
            delay(1);
         }
      }
   if (t*31.4 > nPulses*dt-100) break;
   if (stop) break;
   }
}

void loopHRM() {
   while (!stop) {
         pulseHR();
   }

   deactivateAll();
}

void run_python() {
   system("python /home/pi/project/piglow/hrm_read.py");
}

void shine() {
   pulseAllLEDs(10);
}


int main() {
   piGlowSetup(1);
   signal(SIGINT, sig_abort);

   stop = 0;

   shine();

   // start HRM
   printf("START HEART RATE MONITOR PYTHON DAEMON\n");
   int pid1 = fork();
   if (pid1 == 0) {
      run_python();
      _exit(1);
   }

   sleep(3); // to give run_python time to init

   printf("START PIGLOW:\n");

   loopHRM();

   kill(0, SIGINT);

   exit(EXIT_SUCCESS);
}
