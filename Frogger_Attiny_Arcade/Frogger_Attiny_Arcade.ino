#include <EEPROM.h>
#include "font6x8AJ2.h"
#include <avr/pgmspace.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define CLICKDELAY 120 
#define MOVEBASE 1000 

#define DIGITAL_WRITE_HIGH(PORT) PORTB |= (1 << PORT)
#define DIGITAL_WRITE_LOW(PORT) PORTB &= ~(1 << PORT)

#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))

#define SSD1306XLED_H
#define SSD1306_SCL   PORTB4
#define SSD1306_SDA   PORTB3
#define SSD1306_SA    0x78

void ssd1306_init(void);
void ssd1306_xfer_start(void);
void ssd1306_xfer_stop(void);
void ssd1306_send_byte(uint8_t byte);
void ssd1306_send_command(uint8_t command);
void ssd1306_send_data_start(void);
void ssd1306_send_data_stop(void);
void ssd1306_setpos(uint8_t x, uint8_t y);
void ssd1306_fillscreen(uint8_t fill_Data);
void ssd1306_char_f6x8(uint8_t x, uint8_t y, const char ch[]);
void ssd1306_draw_bmp(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t bitmap[]);

void sendBlock(byte, bool);
void sendByte(byte, bool);
void beep(int,int);
void system_sleep(void);
void doNumber (int,int,int);
void playFrogger(void);
void levelUp(int);
void moveBlocks(void);
void initScreen(void);
void drawDocks(void);
void drawLives(void);
void displayTitle(void);
void resetDock(byte);
void checkCollision(void);

int watchDog;
boolean stopAnimate;
int lives;
bool frogDocks[5];
bool flipFlop;
bool flipFlopShift;
byte frogColumn;
byte frogRow;
byte frogLeftLimit;
byte frogRightLimit;
byte level;
byte blockShiftL;
byte blockShiftR;
int interimStep;
int moveDelay;
int dockedFrogs;
unsigned long clickBase;
boolean clickLock;
int score;
int topScore;
boolean newHigh;
boolean mute = 0;
byte grid[6][16];
byte frogMode;
bool moveForward=0;
bool moveLeft=0;
bool moveRight=0;

ISR(PCINT0_vect){
  if (clickLock == 0) {
    moveLeft = 1;
    clickLock = 1;
    clickBase = millis();
  }
}

void playerIncFrogger(){
  if (clickLock == 0) {
    moveRight = 1;
    clickLock = 1;
    clickBase = millis();
  }
}

void displayTitle(void) {
  int incr = 0;
  for(int lxn = 2; lxn < 7; lxn++) {
    ssd1306_setpos(85,lxn); 
    ssd1306_send_data_start();
    for(int lxn2 = 0; lxn2 < 40; lxn2++) {
      ssd1306_send_byte(pgm_read_byte(&titleBmp[incr]));      
      incr++;
    }
    ssd1306_send_data_stop();                    
  }
}

void setup() {
  DDRB = 0b00000010;
  PCMSK = 0b00000001;
  GIMSK |= 0b00100000;
  sei();
}

void loop() { 
  ssd1306_init();
  ssd1306_fillscreen(0x00);
  ssd1306_char_f6x8(0, 2, "F R O G G E R");
  ssd1306_char_f6x8(0, 4, "andy jackson");
  ssd1306_setpos(0,1); 
  for (int incr = 0; incr < 80; incr++) {
    ssd1306_send_data_start();
    ssd1306_send_byte(B00111000);
    ssd1306_send_data_stop();                    
  }
  displayTitle();
  delay(2000);
  ssd1306_init();
  ssd1306_fillscreen(0x00);
  playFrogger(); 
  topScore = EEPROM.read(0);
  topScore = topScore << 8;
  topScore = topScore |  EEPROM.read(1);
  newHigh = 0;
  if (score > topScore) { 
    topScore = score;
    EEPROM.write(1,score & 0xFF); 
    EEPROM.write(0,(score>>8) & 0xFF); 
    newHigh = 1;
  }
  ssd1306_fillscreen(0x00);
  ssd1306_char_f6x8(11, 1, "----------------");
  ssd1306_char_f6x8(11, 2, "G A M E  O V E R");
  ssd1306_char_f6x8(11, 3, "----------------");
  ssd1306_char_f6x8(37, 5, "SCORE:");
  doNumber(75, 5, score);
  if (!newHigh) {
    ssd1306_char_f6x8(21, 7, "HIGH SCORE:");
    doNumber(88, 7, topScore);
  }
  delay(1000);
  if (newHigh) {
    ssd1306_fillscreen(0x00);
    ssd1306_char_f6x8(10, 1, "----------------");
    ssd1306_char_f6x8(10, 3, " NEW HIGH SCORE ");
    ssd1306_char_f6x8(10, 7, "----------------");
    doNumber(50,5,topScore);
    for (int i = 700; i>200; i = i - 50){
      beep(30,i);
    }
    delay(1200);    
  } 
  system_sleep();
}
