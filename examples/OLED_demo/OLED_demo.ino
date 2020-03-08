#include <Arduino.h>
#include <font6x8.h>
#include <font8x16.h>
#include <ssd1306xled.h>
#include "image.h"
#include "image_two.h"

// // default I2C address used is 0X3C, change as per your need
// #ifdef SSD1306_SA
//   #undef SSD1306_SA
//   #define SSD1306_SA 0X3C
// #else
//   #define SSD1306_SA 0X3C
// #endif

// // default PIN configuration is as below
// // VCC ---- vcc
// // GND ---- gnd
// // SCL ---- pb2
// // SDA ---- pb0

// // change them if you need
// #ifdef SSD1306_SCL
//   #undef SSD1306_SCL
//   #define SSD1306_SCL PB2
// #else
//   #define SSD1306_SCL PB2
// #endif

// #ifdef SSD1306_SDA
//   #undef SSD1306_SDA
//   #define SSD1306_SDA PB0
// #else
//   #define SSD1306_SDA PB2
// #endif


void setup() {
   _delay_ms(40);
  SSD1306.ssd1306_init();
}

void loop() {
  uint8_t p = 0xff;
  for (uint8_t i = 0; i < 4; i++){
    p = (p >> i);
    SSD1306.ssd1306_fillscreen(~p);
    _delay_ms(100);
  }
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_setpos(0, 1);
  SSD1306.ssd1306_string_font6x8("That's the");
  SSD1306.ssd1306_setpos(43, 3);
  SSD1306.ssd1306_string_font6x8("project");
  SSD1306.ssd1306_setpos(0, 3);
  SSD1306.ssd1306_string_font6x8("The platform that gives you everything you need for your first microcontroller project");
  SSD1306.ssd1306_setpos(12, 7);
  SSD1306.ssd1306_string_font6x8("http://tinusaur.org");
  _delay_ms(6000);
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_draw_bmp(0, 0, 128, 8, logo);
  _delay_ms(4000);
  SSD1306.ssd1306_fillscreen(0);
  SSD1306.ssd1306_draw_bmp(0, 0, 128, 8, logo_two);
  _delay_ms(4000);
}