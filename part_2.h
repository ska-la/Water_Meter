//--- made by SKA ---
#include <Arduino.h>
#include "kPad_i2c.h"

//----------------------- LCD 16x2 -----------

#define SCREEN_WIDTH  16
#define SCREEN_HEIGHT 2
#define LCD_PORT      0x27
/*
//----------------------- LCD 20x4 -----------
#define SCREEN_WIDTH  20
#define SCREEN_HEIGHT 4
#define LCD_PORT      0xa0
*/
#if defined(SCREEN_WIDTH) && SCREEN_WIDTH == 16
#define VSCREEN_WIDTH 40
#else
#define VSCREEN_WIDTH 20
#endif

#if defined(SCREEN_HEIGHT) && SCREEN_HEIGHT == 2
#define TST_STR_CH B00000011
#else
#define TST_STR_CH B00001111
#endif

#define SER_IN 0
#define SER_OUT 1

//---------------------- количество отсчётов для усреднения показаний скорости пролива ----------
#define SMOOTH_LENGTH 5

extern LiquidCrystal_I2C lcd;
extern int nMenu;
extern byte cur_posi;
extern byte menu_posi;
extern boolean inMenu;
extern boolean cur_mov;
extern char my_cur;
extern byte str_ch;
extern char vBuff[];
extern char smbl;
extern boolean single_tag;

extern kPad kp;

extern volatile unsigned long pulseCount;
extern volatile unsigned int pulsePerSecond;
extern volatile boolean reDraw;

extern String pulseWeight[];
extern String workMenu[];

extern String preDataInput;
extern String postDataInput;
extern String resultData;
extern String clearAsk;

extern float floatPulse;
extern unsigned int pulseMass[];

extern float startValue;
extern float finalValue;

extern char strPulse[];
extern char strValue[];

extern unsigned long startDelay;

void vbuff_init(void);

void menu_init(byte);

void cur_clear(void);

void cur_draw(void);

void cur_down(void);

void cur_up(void);

void p_move(boolean);

void p_to_i(byte);

void preRunStr(void);

void menuReturn(void);

void input_sel(void);

void do_config(void);

void do_count(void);

void do_preData(void);

void do_postData(void);

void do_calculate(void);

void do_reset(void);

void input_data(void);

void meter_interrupt();

