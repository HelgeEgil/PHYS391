void activateAll();
void deactivateAll();
void pulseAll(int delay_ms);
void sig_abort(int sig_number);
void pulseLeg(int leg);
void pulseLegFast(int leg);
void pulseLegFancy(int leg, int speed);
void pulseAllFancy(int speed);
void pulseHR(int HR);
void pulseHROneLeg(int HR, int leg);
int getHR();
int getManyHR(int i);
void oneHRM();
void manyHRM();
void shine();
float gauss(float x, float mu, float sigma);


char *main_menu[];
void draw_menu(char *options[], int current_highlight, int start_row, int start_col);
int getchoice(char *greet, char *choices[]);
