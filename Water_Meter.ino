//--- made by SKA ---
//--- LCD_I2C + keypad 4x4 ---
//--- работа в комплексе со Струна-М ---

#include <avr/pgmspace.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "kPad_i2c.h"
#include "part_2.h"

LiquidCrystal_I2C lcd(LCD_PORT,SCREEN_WIDTH,SCREEN_HEIGHT);  // set the LCD address, chars and lines

#define PAD_ROWS      4
#define PAD_COLS      4

char sChart[PAD_ROWS][PAD_COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

#define KPAD_I2C_ADDR 0x23                          //--- I2C адрес клавиатуры ---

kPad kp = kPad((char *) sChart,PAD_ROWS,PAD_COLS);

//--- пункты меню ---
const prog_char iMenu1[] PROGMEM = {" 1.CONFIG"};
const prog_char iMenu2[] PROGMEM = {" 2.PRE-DATA"};
const prog_char iMenu3[] PROGMEM = {" 3.WORK"};
const prog_char iMenu4[] PROGMEM = {" 4.POST-DATA"};
const prog_char iMenu5[] PROGMEM = {" 5.CALCULATE"};
const prog_char iMenu6[] PROGMEM = {" 6.RESET"};

//--- подменю (состоят из пунктов меню) ---
const prog_char *sMenu1[] PROGMEM = {
  iMenu1,
  iMenu2,
  iMenu3,
  iMenu4,
  iMenu5,
  iMenu6
};

int nMenu = 6;               //---------- количество пунктов меню
char vBuff[VSCREEN_WIDTH * SCREEN_HEIGHT];    //---------- видео буфер (40 символов x 2 строки или 20 символов х 4 строки) LCD

//------------------------------------------------- Menu CONFIG strings ---------
String pulseWeight[] = { "Input pulse weight:" , "m3/pulse" };

//------------------------------------------------- Menu WORK strings ---------
String workMenu[] = { "m3/hour", "total Litres" };

//------------------------------------------------- Menu PRE-DATA strings ---------
String preDataInput = "Start value (m3)";

//------------------------------------------------- Menu POST-DATA strings ---------
String postDataInput = "Final value (m3)";

//------------------------------------------------- Menu CALCULATE strings ---------
String resultData = "Deviation:";

//------------------------------------------------- Menu RESET strings ---------
String clearAsk = "Clear all data?";

//--------- следующие переменные используются в качестве битовых масок -----------
byte str_ch = 0;    //--------------- содержимое строки видеобуфера изменилось ----------
byte isData = 0;    //--------------- определяет номер бегущей строки данных -----------

//--------- все что касается жизни курсора на LCD -----------
byte cur_posi = 0;          //----------- cursor position in most left LCD column ---------
byte menu_posi = 0;         //----------- current menu item under cursor ---------
char my_cur = B01111110;    //----------- мой курсор (можно выбрать по своему вкусу любой) --------
boolean cur_mov = true;    //----------- было ли перемещение курсора вверх или вниз ---------

//----------------------- признак входа в один из пунктов меню ------------
boolean inMenu = false;

//-------------------------- признак однократного выполнения кода -------------
boolean single_tag = false;

char smbl = 0x00;            //------ символ считываемый с клавиатуры ---

//------------------------------------- счётчики импульсов (общий и в секунду) ---------------
volatile unsigned long pulseCount = 0;
volatile unsigned int pulsePerSecond = 0;

//-------------------------------------- массив (импульсов/сек) предыдущих 5-ти секунд ------------
//unsigned int pulseMass[] = {0,0,0,0,0};
unsigned int pulseMass[SMOOTH_LENGTH] = {0};

//----------------------------- вес импульса (м3) --------------
char strPulse[] = {"0.00001"};
float floatPulse = String ( strPulse ).toFloat();

//----------------------------- показания счётчика (м3) --------------
char strValue[] = {"0.0000"};

//-------------------------------- показания поверяемого счётчика -----------
//-------------------------------- до и после измерения -----------
float startValue = 0.0001;
float finalValue = 0;

//------------------------------- пора перерисовать экран ----------
volatile boolean reDraw = false;

#define DELAY_AMOUNT 1000        //------- задержка в 1 секунду ---------
unsigned long startDelay = 0;

void setup() {
  kp.init(KPAD_I2C_ADDR);
  lcd.init();
  vbuff_init();
  menu_init(menu_posi);
  pinMode( SER_IN, INPUT );
  pinMode( SER_OUT, OUTPUT );
//-------------------------------------------- прерывание от водомера на INT1 ------
//  attachInterrupt(1,meter_interrupt,FALLING);
}

void loop() {
  smbl = kp.read();
  if (smbl) {
      input_sel();
  }
  if ( millis() - startDelay > DELAY_AMOUNT ) {
    reDraw = true;
    startDelay = millis();
  }
  runStr();
  lcd_redraw();
  delay(200);
}

//------------------------ обработчик прерываний от счётчика ---------
void meter_interrupt() {
  pulseCount++;
  pulsePerSecond++;
}

/*
void int_tackle() {
  reDraw = true;
}
*/

//-------------------------------- инициализация меню ---
void menu_init(byte a) {
//---------------------- 'byte а' - это порядковый номер меню в первой строке LCD ----
//---------------------- 'byte j' - цикл по строкам LCD ----------
byte n_str = 0;
  if ( nMenu-a < SCREEN_HEIGHT ) n_str = nMenu-a; else n_str = SCREEN_HEIGHT;
  for ( byte j=0; j < n_str; j++ ) {
        strncpy_P((char *)vBuff + j*VSCREEN_WIDTH, (char *) pgm_read_word(&(sMenu1[a+j])),strlen_P((char *) pgm_read_word(&(sMenu1[a+j]))));
      bitSet(str_ch,j);
  }
}

//------------------- ----перерисовка экрана LCD ---
void lcd_redraw(void) {
  if ( (str_ch & TST_STR_CH) != B00000000 ) {
    for (byte j=0; j < SCREEN_HEIGHT; j++) {
      if ( bitRead(str_ch,j) ) {
        lcd.setCursor(0,j);
        for (byte i=0; i < VSCREEN_WIDTH ; i++)
          lcd.write(vBuff[ j*VSCREEN_WIDTH + i ]);
        bitClear(str_ch,j);
      }
    }
  }
  if ( cur_mov ) {
    cur_draw();
    cur_mov = false;
  }
}

//-------------------------- возврат к меню --------------
void menuReturn(void) {
  if ( ! inMenu ) return;
  isData = 0;
  inMenu = false;
  lcd.noAutoscroll();
  lcd.clear();
  str_ch |= TST_STR_CH ;
  cur_mov = true;
}

//-------------- подготовка LCD к бегущей строке -----------
void preRunStr(void) {
  if ( inMenu ) return;
  str_ch = 0;
  inMenu = true;
  lcd.clear();
  bitSet(isData,0);
}

//-------------- вывод данных пунктов меню -----------
void runStr(void) {
//byte i = 0;
  if ( (isData & TST_STR_CH) != B00000000 ) {
    switch (menu_posi){
      case 0:
        do_config();
        break;
      case 1:
        do_preData();
        break;
      case 2:
        do_count();
        break;
      case 3:
        do_postData();
        break;
      case 4:
        do_calculate();
        break;
      case 5:
        do_reset();
        break;
      default:
        break;
    }
  }
}


