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

#define MESSAGE_LINE 6
#define ERROR_LINE 22
#define Q_LINE 20
#define PROMPT_LINE 18

volatile sig_atomic_t stop;

char *main_menu[] = {
   "1) Make my raspberry pi shine",
   "2) Couple PiGlow to Heart Rate Monitor",
   "3) Couple PiGlow to multiple Heart Rate Monitors",
   "4) Quit",
   0,
}; 

char *oneHR_menu[] = {
   "1) Find HRM (not yet)",
   "2) Start connection",
   "3) Stop connection",
   "4) Return",
   0,
};

void draw_menu(char *options[], int current_highlight, int start_row, int start_col) {
   int current_row = 0;
   char **option_ptr;
   char *txt_ptr;

   option_ptr = options;

   while (*option_ptr) {
      if (current_row == current_highlight) attron(A_STANDOUT);
      txt_ptr = options[current_row];
      mvprintw(start_row + current_row, start_col, "%s", txt_ptr);
      if (current_row == current_highlight) attroff(A_STANDOUT);
      current_row++;
      option_ptr++;
   }

   mvprintw(start_row + current_row + 3, start_col, "Choose option and press Return ");
   refresh();
}

int getchoice(char *greet, char *choices[]) {
   static int selected_row = 0;
   int max_row = 0;
   int start_screenrow = MESSAGE_LINE, start_screencol = 10;
   char **option;
   int selected;
   int key = 0;

   option = choices;
   while (*option) {
      max_row++;
      option++;
   }

   if (selected_row >= max_row) { selected_row = 0; }
   clear();
   refresh();

   mvprintw(start_screenrow - 2, start_screencol, greet);
   keypad(stdscr, TRUE);
   cbreak();
   noecho();
   key = 0;

   while (key != 'q' && key != KEY_ENTER && key != '\n') {
      if (key == KEY_UP) {
         if (selected_row == 0) { selected_row = max_row - 1; }
         else { selected_row--; }
      }
      if (key == KEY_DOWN) {
         if (selected_row == (max_row - 1)) { selected_row = 0; }
         else { selected_row++; }

      }

      selected = *choices[selected_row];
      draw_menu(choices, selected_row, start_screenrow, start_screencol);
      key = getch();

   }

   keypad(stdscr, FALSE);
   nocbreak();
   echo();

   if (key == 'q') { selected = 'q'; }

   return (selected);

}

void sig_stop(int sig_number) {
   stop = 1;
   (void) signal(SIGINT, sig_abort);
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

void pulseLeg(int leg) {
   int i;
   for (i=0; i<200; i++) {
      piGlowLeg(leg, i);
      delay(1);
   }
   delay(10);
   for (i=200; i>=0; i--) {
      piGlowLeg(leg, i);
      delay(1);
   }
}

void pulseLegFast(int leg) {
   int i;
   for (i=0; i<20; i++) {
      piGlowLeg(leg, i*10);
      delay(1);
   }
   delay(10);
   for (i=20; i>=0; i--) {
      piGlowLeg(leg, i*10);
      delay(1);
   }
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

void pulseAll(int delay_ms) {
   activateAll();
   delay(delay_ms);
   deactivateAll();
}

void pulseLegFancy(int leg, int speed) {
   int timer;
   float t, tLed;
   int led;

   int intensity = 20;
   
   for (timer=0; timer<speed*15; timer++) {
      t = timer / (float) speed;
      for (led=0; led<6; led++) {
         tLed = t - led/6.0;
         if (tLed<0) tLed = 0;
         piGlow1(leg, (5-led), abs(intensity * pow(sin(tLed * 1.57),2)));
         delay(1);
      }
   }
}

void pulseAllFancy(int speed) {
   float t, tLed;
   int timer, led, leg;

   int intensity = 20;
   
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

void pulseHR(int HR) {
   float t;
   int timer, led, leg, pulse;

   int nPulses = 8;

   float intensity, g;
   int maxIntensity = 30;
  
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
   }
}

void pulseHROneLeg(int HR, int leg) {
   float t;
   int timer, led, pulse;

   int nPulses = 8;

   float intensity, g;
   int maxIntensity = 30;
  
   float dt = 60000 / (float) HR; // ms between pulses
   float sigma = (350-HR) / (float) 3;


   for (timer=0; timer<1000; timer++) {
      for (led=0; led<6; led++) {
         t = timer - led/6.0;
         if (t<0) t = 0;
         
         intensity = 0;
         for (pulse=0; pulse < nPulses; pulse++) {
            g = gauss(t*31.4, 1.5*sigma + pulse * dt, sigma);
            intensity += g;
         }

         piGlow1(leg, (5-led), maxIntensity * intensity);
         delay(3);
         
      }
   if (t*31.4 > nPulses*dt-100) break;
   }
}

int getHR() {
   return 120;
}

int getManyHR(int i) {
   int HR;
   if (i == 1) HR = 135;
   if (i == 2) HR = 86;

   return HR;   
}

void oneHRM() {
   int HR;

   while (!stop) {
      HR = getHR();
      pulseHR(HR);
   }

   deactivateAll();
}

void manyHRM() {
   int HR1, HR2;

   while (!stop) {
      HR1 = getManyHR(1);
      HR2 = getManyHR(2);

      pid_t i = fork();
      if (i==0) { pulseHROneLeg(HR1, 2); }
      else { pulseHROneLeg(HR2, 1); }
   }
   deactivateAll();
}

void shine() {
   pulseAllFancy(10);
}

void drawOneHR_menu() {
   int choice, i;
   do {
      choice = getchoice("Options: ", oneHR_menu);
      switch (choice) {
         case 'q':
            break;
         case '1':
            break;
         case '2':
            i = fork();
            signal(SIGINT, sig_stop);
            if (i == 0) oneHRM();
            break;
         case '3':
            kill(0, SIGINT);
            break;
         case '4':
            break;
      }
   } while (choice != 'q');
}

void drawManyHR_menu() {
   int choice, i;

   do {
      choice = getchoice("Options: ", oneHR_menu);
      switch (choice) {
         case 'q':
            break;
         case '1':
            break;
         case '2':
            signal(SIGINT, sig_stop);
            i = fork();
            if (i == 0) manyHRM();
            break;
         case '3':
            kill(0, SIGINT);
            break;
         case '4':
            break;
      }
   } while (choice != 'q');
}

int main() {
   piGlowSetup(1);
   signal(SIGINT, sig_abort);

   stop = 0;

   int choice;
   initscr();

   do {
      choice = getchoice("Options: ", main_menu);
      switch (choice) {
         case 'q':
            break;
         case '1':
            shine();
            break;
         case '2':
            drawOneHR_menu();
            break;
         case '3':
            drawManyHR_menu();
            break;
         case '4':
            endwin();
            exit(EXIT_SUCCESS);
            break;
      }
   } while (choice != 'q');
   endwin();
   exit(EXIT_SUCCESS);
}
