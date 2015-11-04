#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curses.h>

#define MESSAGE_LINE 6
#define ERROR_LINE 22
#define Q_LINE 20
#define PROMPT_LINE 18

#define RED_LED = 0 // fix, ovviamente
#define BLUE_LED = 1
#define GREEN_LED = 2

char *main_menu[] = {
   "Activate single LED",
   "Deactivate single LED",
   "Check LED status",
   "RAINBOW TIME",
   "quit",
   0,
};

char *led_choice[] = {
   "Red LED",
   "Blue LED",
   "Green LED",
   "quit",
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
//      txt_ptr++;
      mvprintw(start_row + current_row, start_col, "%s", txt_ptr);
      if (current_row == current_highlight) attroff(A_STANDOUT);
      current_row++;
      option_ptr++;
   }

   mvprintw(start_row + current_row + 3, start_col, "Move highlight then press Return ");
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

   if (selected_row >= max_row)
      selected_row = 0;
   clear();
   refresh();
   mvprintw(start_screenrow - 2, start_screencol, greet);
   keypad(stdscr, TRUE);
   cbreak();
   noecho();
   key = 0;
   while (key != 'q' && key != KEY_ENTER && key != '\n') {
      if (key == KEY_UP) {
         if (selected_row == 0)
            selected_row = max_row - 1;
         else
            selected_row--;
      }
      if (key == KEY_DOWN) {
         if (selected_row == (max_row - 1))
            selected_row = 0;
         else
            selected_row++;
      }
      selected = *choices[selected_row];
      draw_menu(choices, selected_row, start_screenrow, start_screencol);
      key = getch();
   }
   keypad(stdscr, FALSE);
   nocbreak();
   echo();

   if (key == 'q')
      selected = 'q';

   return (selected);
}

void activate_led(int led) {
   switch (choice) {
      case 'R':
         digitalWrite(RED_LED, HIGH);
         break;
      case 'B':
         digitalWrite(BLUE_LED, HIGH);
         break;
      case 'G':
         digitalWrite(GREEN_LED, HIGH);
         break;
   }
}

void deactivate_led(int led) {
   switch (choice) {
      case 'R':
         digitalWrite(RED_LED, LOW);
         break;
      case 'B':
         digitalWrite(BLUE_LED, LOW);
         break;
      case 'G':
         digitalWrite(GREEN_LED, LOW);
         break;
   }
}

void rainbow_time() {
   digitalWrite(RED_LED, LOW);
   digitalWrite(BLUE_LED, LOW);
   digitalWrite(GREEN_LED, LOW);


   // TODO create break possibility
   for (;;) {
      digitalWrite(RED_LED, HIGH);
      delay(150);
      digitalWrite(GREEN_LED, LOW);
      delay(150);
      digitalWrite(BLUE_LED, HIGH);
      delay(150);
      digitalWrite(RED_LED, LOW);
      delay(150);
      digitalWrite(GREEN_LED, HIGH);
      delay(150);
      digitalWrite(BLUE_LED, LOW);
   }


int get_led_status(int led) {
   int status;
   switch (choice) {
      case 'R':
         status = digitalRead(RED_LED, LOW);
         break;
      case 'B':
         status = digitalRead(BLUE_LED, LOW);
         break;
      case 'G':
         status = digitalRead(GREEN_LED, LOW);
         break;
   }
   return status;
}

int main() {
   int choice, led;
   int status;
   initscr();
   wiringPiSetup();
   pinMode(0, OUTPUT);

   do {
      choice = getchoice("Options: ", main_menu);
      switch (choice) {
         case 'q':
            break;
         case 'A':
            getchoice("Which LED to activate? ", led_choice);
            activate_led(led);
            break;
         case 'D':
            led = getchoice("Which LED to deactivate? ", led_choice);
            deactivate_led(led);
            break;
         case 'C':
            getchoice("Check status of which LED? ", led_choice);
            status = get_led_status(led);
            break;
         case 'R':
            rainbow_time();
            break;
      }  
   } while (choice != 'q');
   endwin();
   exit(EXIT_SUCCESS);
}
