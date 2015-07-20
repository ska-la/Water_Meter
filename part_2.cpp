//--- made by SKA ---
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "part_2.h"

//---------------------- очистка видеобуфера ---------
void vbuff_init() {
  for (byte j=0;j < SCREEN_HEIGHT;j++) {
    for (byte i=0;i < VSCREEN_WIDTH;i++) {
        vBuff[ i + j*VSCREEN_WIDTH ] = ' ';
    }
    bitSet(str_ch,j);
  }
}

//------------------------ очистка места где стоит указатель-курсор ------------------
void cur_clear() {
  lcd.write(' ');
  lcd.setCursor(0,cur_posi);
}

//---------------------------- рисуем указатель-курсор ---
void cur_draw() {
  lcd.setCursor(0,cur_posi);
  lcd.write(my_cur);
  lcd.setCursor(0,cur_posi);
}

//------------------------ курсор вниз --------------
void cur_down(void) {
  lcd.setCursor(0,cur_posi++);
  menu_posi++;
}

//------------------------ курсор вверх --------------
void cur_up(void) {
  lcd.setCursor(0,cur_posi--);
  menu_posi--;
}

//--------------------- перемещаем указатель-курсор вверх или вниз ---------
void p_move(boolean c_m) {
  if ( inMenu ) return;
  cur_clear();
  cur_mov = true;
  if ( (menu_posi == 0) && c_m ) {
    cur_down();
    return;
  }
  if ( (menu_posi == nMenu-1) && !c_m ) {
    cur_up();
    return;
  }
  if ( (menu_posi > 0) && (menu_posi < nMenu-1) ) {
    switch (cur_posi) {
      case 0:
        if ( c_m ) {
          cur_down();
        } else {
          menu_posi--;
          vbuff_init();
          menu_init(menu_posi);
        }
        break;
      case SCREEN_HEIGHT-1:
        if ( !c_m ) {
          cur_up();
        } else {
          menu_posi++;
          vbuff_init();
          menu_init(menu_posi-SCREEN_HEIGHT+1);
        }
        break;
      default:
        if ( c_m ) {
          cur_down();
        }
        else {
          cur_up();
        }
    }
  }
}

//------------------------- устанавливаем курсор на нужный пункт меню --------------
void p_to_i(byte b) {
byte i_diff;
  if ( inMenu ) return;
  b--;
  cur_clear();
  cur_mov = true;
  if ( b < nMenu ) {
    if ( b > menu_posi ) {
      i_diff = b - menu_posi;
      if ( cur_posi + i_diff > SCREEN_HEIGHT-1 ) {
        menu_posi = b;
        cur_posi = SCREEN_HEIGHT-1;
        vbuff_init();
        menu_init(menu_posi-SCREEN_HEIGHT+1);
      } else {
        cur_posi += i_diff;
        menu_posi = b;
      }
      return;
    }
    if ( b < menu_posi ) {
      i_diff = menu_posi - b;
      if ( cur_posi - i_diff < 0 ) {
        menu_posi = b;
        cur_posi = 0;
        vbuff_init();
        menu_init(menu_posi);
      } else {
        cur_posi -= i_diff;
        menu_posi = b;
      }
    }
  }
}

//------------------------- анализ ввода с клавиатуры ----------
//------------------------- привязка действий к клавишам -------
void input_sel() {
byte b_i_s = (byte) smbl & B00001111;
  switch(smbl) {
    case 'A':
      p_move(0);
      break;
    case 'C':
      p_move(1);
      break;
    case '1':
      p_to_i(b_i_s);
      break;
    case '2':
      p_to_i(b_i_s);
      break;
    case '3':
      p_to_i(b_i_s);
      break;
    case '4':
      p_to_i(b_i_s);
      break;
    case '5':
      p_to_i(b_i_s);
      break;
    case '6':
      p_to_i(b_i_s);
      break;
    case 'B':
      if ( inMenu ) {
        if ( menu_posi == 1 ) {
          startValue = String(strValue).toFloat();
          lcd.setCursor(7,1);
          lcd.write('*');
          lcd.noCursor();
          lcd.noBlink();
        }
        if ( menu_posi == 3 ) {
          finalValue = String(strValue).toFloat();
          lcd.setCursor(7,1);
          lcd.write('*');
          lcd.noCursor();
          lcd.noBlink();
        }
        if ( menu_posi == 5 ) {
          pulseCount = 0;
          startValue = 0;
          finalValue = 0;
          byte j = 0;
          do {
            pulseMass[j] = 0;
            j++ ;
          } while ( j < SMOOTH_LENGTH );
          lcd.setCursor(0,1);
          lcd.print("Yes");
        }
      }
      preRunStr();
      break;
    case 'D':
      menuReturn();
      single_tag = false;
      reDraw = ! reDraw;
      if ( menu_posi == 2 ) {
        detachInterrupt(0);
      }
      if ( menu_posi == 1 || menu_posi == 3 ) {
        lcd.noCursor();
        lcd.noBlink();
      }
      break;
    case '#':
      if ( menu_posi == 2 ) {
        detachInterrupt(0);
      }
      break;
    default:
      break;
  }
}

//----------------- настройка конфигурации счётчика --------
void do_config() {
  if ( ! single_tag ) {
    lcd.setCursor(0,0);
    lcd.print(pulseWeight[0]);
    lcd.setCursor(0,1);
    lcd.print(strPulse);
    lcd.setCursor(8,1);
    lcd.print(pulseWeight[1]);
    single_tag = true;
  }
}

//----------------------- отображение подсчёта импульсов ----------
void do_count() {
static byte smoothDelay = 0;
float averagePulses = 0;
byte i = 0;
  if ( ! single_tag ) {
    pulseCount = 0;
    pulsePerSecond = 0;
    attachInterrupt(0,meter_interrupt,RISING);
    startDelay = millis();
    lcd.setCursor(0,0);
    lcd.print( String( (float) 0, 4) );
    lcd.setCursor(7,0);
    lcd.print(workMenu[0]);
    lcd.setCursor(7,1);
    lcd.print(workMenu[1]);
    single_tag = true;
  }
  if ( reDraw ) {
    pulseMass[ smoothDelay ] = pulsePerSecond;
    smoothDelay++ ;
    if ( smoothDelay == SMOOTH_LENGTH ) {;
      smoothDelay = 0;
    }
    do {
      averagePulses += pulseMass[i];
      i++;
    } while ( i < SMOOTH_LENGTH );
    averagePulses /= 5 ;
    lcd.setCursor(0,0);
//------------------------ в кубометрах -------------
//    lcd.print( String( pulsePerSecond * 0.001, 3) );
//------------------------ кубометры / час -------------
//    lcd.print( String( (float) ( pulsePerSecond * 3600 / 100000 ), 4) );
    lcd.print( String( (float) ( ( averagePulses * floatPulse ) * 3600 ), 4) );
//------------------------ в литрах --------------
//    lcd.print( pulsePerSecond, DEC );
    pulsePerSecond = 0;
    lcd.setCursor(0,1);
//------------------------ в кубометрах -------------
//    lcd.print ( String( pulseCount * 0.001, 3 ) );
//------------------------ в литрах --------------
    lcd.print ( String( (float) pulseCount/100, 2 ) );
//------------------------ в литрах --------------
//    lcd.print ( pulseCount, DEC );
    reDraw = ! reDraw;
  }
}

//-------------------- ввод начального показания счётчика -----------
void do_preData() {
  if ( ! single_tag ) {
    smbl = 0x00;
    lcd.setCursor(0,0);
    lcd.print(preDataInput);
    lcd.setCursor(0,1);
    lcd.print( String ( startValue , 4 ) );
    lcd.setCursor(0,1);
    lcd.blink();
    lcd.cursor();
    single_tag = true;
  }
    input_data();
}

//-------------------- ввод данных с клавиатуры -----------
void input_data() {
static int powExp = 1;
    if ( byte(smbl) >= 0x30 && byte(smbl) <= 0x39 ) {
        strValue[1-powExp] = smbl;
      lcd.write(smbl);
      powExp--;
      if ( powExp == 0 ) {
        powExp--;
      }
      if ( powExp == -5 ) {
        powExp = 1;
      }
      lcd.setCursor( byte(1 - powExp) , 1 );
    }
}

//-------------------- ввод конечного показания счётчика -----------
void do_postData() {
  if ( ! single_tag ) {
    smbl = 0x00;
    lcd.setCursor(0,0);
    lcd.print(postDataInput);
    lcd.setCursor(0,1);
    lcd.print( String ( finalValue , 4 ) );
    lcd.setCursor(0,1);
    lcd.blink();
    lcd.cursor();
    single_tag = true;
  }
    input_data();
}

//---------------------- вычисление результата --------------
void do_calculate() {
float resultVal = 0;
  if ( ! single_tag ) {
    if ( pulseCount > 0 && startValue > 0 && finalValue > startValue ) {
//      resultVal = finalValue - startValue ;
      resultVal = ( ( ( finalValue - startValue ) / ( pulseCount * floatPulse ) ) - 1 ) * 100;
    }
    lcd.setCursor(0,0);
    lcd.print(resultData);
    lcd.setCursor(0,1);
    lcd.print( String ( resultVal , 4) );
/*    lcd.print ( String(startValue,4) );
    lcd.setCursor(0,1);
    lcd.print ( String(finalValue,4) );*/
    single_tag = true;
  }
}

//------------------- сброс всех введённых показаний и вычислений ------------
void do_reset() {
  if ( ! single_tag ) {
    lcd.setCursor(0,0);
    lcd.print(clearAsk);
    single_tag = true;
  }
}


