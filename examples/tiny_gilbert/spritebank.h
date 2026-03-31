//        >>>>>  T-I-N-Y  G-I-L-B-E-R-T  for ATTINY85  GPL v3 <<<<
//                  Copyright (c) Daniel Champagne 2017
//                         all right reserved
//                 Contact EMAIL: phoenixbozo@gmail.com
//           https://sites.google.com/view/arduino-collection

//  Tiny Gilbert is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

//the code work at 16 MHZ internal
//and use ssd1306xled Library for SSD1306 oled display 128x64

#include <avr/pgmspace.h>

typedef struct DriftSprite{
  uint8_t DriftGrid[2][2];
  int8_t x4decalage;
  int8_t y8decalage;
  int16_t MainPositionOnGridH;
  int16_t MainPositionOnGridV;
}DriftSprite;

const uint8_t sprite1  [] PROGMEM =  {0xFF, 0x99,0xE7, 0x99, };  //bloc1 gauche
const uint8_t sprite2  [] PROGMEM =  {0x81, 0xE7, 0x99, 0xFF,};  //bloc1 droite
const uint8_t sprite3  [] PROGMEM = {0x01, 0x2B, 0x55, 0x2B,};    //bloc2 gauche
const uint8_t sprite4  [] PROGMEM = {0x55, 0x2B, 0x55, 0xFF,};    //bloc2 droie
const uint8_t sprite7  [] PROGMEM =  {0x55, 0xAA, 0x55, 0xAA,};  //demi mosaic
const uint8_t sprite8  [] PROGMEM =  {0xF0, 0xFF, 0xFC, 0xC0};  //pik
const uint8_t sprite11 [] PROGMEM =  {0x02, 0xA5, 0xFD, 0x02,}; //cle1
const uint8_t sprite12 [] PROGMEM =  {0x02, 0xFD, 0xA5, 0x02, };//cle2
const uint8_t sprite13 [] PROGMEM =  {0x00, 0xFE, 0xFB, 0xF1,}; //porte gauche
const uint8_t sprite14 [] PROGMEM =  {0xF1, 0xDB, 0xFE, 0x00, };//porte droite
const uint8_t sprite15 [] PROGMEM =  {0x55, 0x57, 0x7D, 0xD5}; //brick gauche
const uint8_t sprite16 [] PROGMEM =  {0x55, 0x57, 0x75, 0xDD };//brick droite
const uint8_t sprite20 [] PROGMEM =  {0x02, 0x0E, 0xDB, 0x7F, };//Gilbert right frame 0
const uint8_t sprite21 [] PROGMEM =  {0xFF, 0x9F, 0x0E, 0x00, };//Gilbert right frame 0b
const uint8_t sprite22 [] PROGMEM =  {0x04, 0x1C, 0xB6, 0x7E, };//Gilbert right frame 1
const uint8_t sprite23 [] PROGMEM =  {0xFE, 0x3E, 0x1C, 0x00, };//Gilbert left frame 1b
const uint8_t sprite24 [] PROGMEM =  {0x04, 0x1C, 0x36, 0xFE, };//Gilbert left frame 2
const uint8_t sprite25 [] PROGMEM =  {0x7E, 0xBE, 0x1C, 0x00, };//Gilbert left frame 2b
const uint8_t sprite26 [] PROGMEM =  {0x00, 0x0E, 0x9F, 0xFF, };//Gilbert right frame 3
const uint8_t sprite27 [] PROGMEM =  {0x7F, 0xDB, 0x0E, 0x02, };//Gilbert right frame 3b
const uint8_t sprite28 [] PROGMEM =  {0x00, 0x1C, 0x3E, 0xFE, };//Gilbert right frame 4
const uint8_t sprite29 [] PROGMEM =  {0x7E, 0xB6, 0x1C, 0x04,  };//Gilbert left frame 4b
const uint8_t sprite30 [] PROGMEM =  {0x00, 0x1C, 0xBE, 0x7E, };//Gilbert left frame 5
const uint8_t sprite31 [] PROGMEM =  {0xFE, 0x36, 0x1C, 0x04,  };//Gilbert left frame 5b
const uint8_t Level0 [] PROGMEM =   {0,1,2,4,4,2,5,2,11,9,8,10,10,2,12,4,6,7,0,0,0};
const uint8_t Level1 [] PROGMEM =   {0,1,2,3,7,7,6,5,8,9,2,1,3,4,6,7,1,11,0,0,0};
const uint8_t Level2 [] PROGMEM =   {0,1,1,2,3,4,3,3,2,12,2,6,5,7,11,1,12,0,0,0};
const uint8_t Level3 [] PROGMEM =   {0,11,10,10,8,7,6,10,6,5,3,2,0,0,0};
const uint8_t Level4 [] PROGMEM =   {0,3,10,4,7,3,6,4,6,3,2,12,3,2,11,12,4,5,1,2,2,12,1,5,5,12,0,0,0};
const uint8_t Level5 [] PROGMEM =   {0,1,12,2,10,11,10,8,12,2,3,3,12,3,3,0,0,0};
const uint8_t Level6 [] PROGMEM =   {0,1,2,12,12,3,12,3,12,1,2,6,11,0,0,0};
const uint8_t Level7 [] PROGMEM =   {0,12,2,1,1,3,3,3,3,12,1,1,1,8,7,11,0,0,0};
const uint8_t Level8 [] PROGMEM =   {0,11,12,1,2,3,6,3,12,3,12,3,6,12,0,0,0};
const uint8_t Level9 [] PROGMEM =   {0,12,11,12,2,12,3,7,12,8,10,2,12,2,12,2,3,4,7,12,0,0,0};
const uint8_t KeyinLevel [] PROGMEM =   {10,11,11,4,19,9,8,12,9,11};
const uint8_t map1couche2[] PROGMEM={  3, 4, 3, 4,/*0*/ 3, 4, 3, 4,/*1*/ 3, 4, 3, 4,/*2*/ 3, 4, 3, 4,/*3*/ 3, 4, 3, 4,/*4*/ 3, 4, 3, 4,/*5*/ 3, 4, 3, 4,/*6*/ 3, 4, 3, 4,/*7*/ 3, 4, 3, 4,/*8*/ 3, 4, 3, 4,/*9*/ 0, 0, 0, 0,/*10*/ 3, 4, 3, 4,/*11*/ 3, 4, 3, 4/*12*/};
const uint8_t map1couche3[] PROGMEM= { 3, 4, 3, 4,/*0*/ 0, 0, 0, 0,/*1*/ 0, 0, 0, 0,/*2*/ 0, 0,11, 0,/*3*/ 0, 0, 0,11,/*4*/ 0, 0, 0, 0,/*5*/ 0, 0, 0, 0,/*6*/ 0, 0, 3, 4,/*7*/ 3, 4, 3, 4,/*8*/ 3, 4, 0, 0,/*9*/ 0, 0, 0, 0,/*10*/ 3, 4, 3, 4,/*11*/ 0, 0, 0, 0/*12*/};
const uint8_t map1couche4[] PROGMEM= { 3, 4, 3, 4,/*0*/ 0, 0, 0, 0,/*1*/ 0, 0, 0, 0,/*2*/ 0, 0, 0, 0,/*3*/ 0, 0, 0, 0,/*4*/ 0,11, 0, 0,/*5*/ 0, 0, 0, 0,/*6*/ 0, 0, 0, 0,/*7*/ 3, 4, 3, 4,/*8*/ 3, 4, 0, 0,/*9*/ 0, 0, 0, 0,/*10*/ 0, 0, 0, 0,/*11*/ 0, 0, 0, 0/*12*/};
const uint8_t map1couche5[] PROGMEM= { 3, 4, 3, 4,/*0*/ 0, 0, 0, 0,/*1*/ 0, 0, 0, 0,/*2*/ 0, 0, 0, 8,/*3*/ 0, 0, 3, 4,/*4*/ 0, 0,11, 0,/*5*/ 3, 4, 3, 4,/*6*/ 3, 4, 3, 4,/*7*/ 0, 0, 3, 4,/*8*/ 0, 0, 0, 0,/*9*/ 3, 4, 3, 4,/*10*/ 0, 0, 0, 0,/*11*/11, 0, 0, 0/*12*/};
const uint8_t map1couche6[] PROGMEM= { 3, 4, 3, 4,/*0*/ 0,11, 0, 0,/*1*/ 0, 0, 0, 0,/*2*/ 3, 4, 3, 4,/*3*/ 3, 4, 3, 4,/*4*/ 0, 0, 3, 4,/*5*/ 0, 0, 0, 0,/*6*/ 0, 0, 0, 0,/*7*/ 0, 0, 3, 4,/*8*/ 0, 0, 0,11,/*9*/ 0, 0, 0, 0,/*10*/ 0,13,14, 0,/*11*/ 0, 0, 0, 0/*12*/};
const uint8_t map1couche7[] PROGMEM= { 3, 4, 3, 4,/*0*/ 0, 0, 0, 8,/*1*/ 3, 4, 3, 4,/*2*/ 3, 4, 3, 4,/*3*/ 3, 4, 3, 4,/*4*/ 0, 0, 0, 0,/*5*/ 0, 0, 0, 0,/*6*/ 0, 0, 0, 0,/*7*/ 0,11, 0, 0,/*8*/ 0, 0,11, 0,/*9*/ 0, 0, 0, 0,/*10*/15,16,15,16,/*11*/ 0, 0, 0, 0/*12*/};
const uint8_t map1couche8[] PROGMEM= {15,16,15,16,/*0*/15,16,15,16,/*1*/15,16,15,16,/*2*/15,16,15,16,/*3*/15,16,15,16,/*4*/15,16,15,16,/*5*/ 0, 0, 0, 0,/*6*/15, 0, 0,16,/*7*/15,16,15,16,/*8*/15,16,15,16,/*9*/15,16,15,16,/*10*/15,16,15,16,/*11*/ 0, 0, 0, 0/*12*/};
const uint8_t  start [] PROGMEM= {
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0xBF, 0xBF, 0x9F, 0xFF, 0xDF, 0xDF, 0xDF, 0xFF, 0xBF,
0x7F, 0x7B, 0xFB, 0x73, 0x07, 0x07, 0x1F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFB, 0xFB,
0xFB, 0x03, 0xFB, 0xFB, 0xFB, 0xFF, 0x0D, 0xFF, 0x0F, 0xDF, 0xEF, 0xEF, 0x1F, 0xFF, 0xEF, 0x1F,
0xFF, 0x7F, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0x1F, 0x07, 0x07, 0x03, 0x00, 0x46, 0xC3, 0xE3, 0xE1, 0xF1, 0xF1, 0xF1, 0xF9, 0xF0, 0xF9, 0x98,
0x98, 0xD8, 0xF8, 0xF8, 0xF8, 0xF8, 0xC8, 0xE0, 0x00, 0x80, 0xE0, 0xF0, 0xF0, 0xFF, 0xFF, 0x7F,
0xBD, 0xBE, 0xBF, 0xBF, 0xFF, 0xFD, 0xFE, 0xDF, 0xFC, 0x1F, 0xFF, 0xFF, 0x1E, 0xFD, 0xFF, 0xEF,
0xF0, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF,
0xFF, 0xFE, 0xFE, 0xFC, 0xFC, 0xF8, 0xE1, 0xE3, 0x87, 0x07, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
0x0F, 0x0F, 0x0F, 0x07, 0x07, 0xC3, 0xE3, 0xF0, 0xF8, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xF0, 0xEF,
0xDF, 0xDF, 0x5D, 0x5D, 0x61, 0x7F, 0x5E, 0x40, 0x7F, 0x40, 0x7F, 0x7F, 0x40, 0x5E, 0x5E, 0x5E,
0x61, 0x7F, 0x61, 0x5A, 0x5A, 0x5A, 0x59, 0x7F, 0x60, 0x5E, 0x7E, 0x7E, 0xFE, 0xC0, 0xDE, 0xFF,
0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFC, 0xF8, 0x80, 0x00, 0x00, 0x04, 0x4C,
0x68, 0x78, 0x78, 0x70, 0x7C, 0xB9, 0x77, 0x77, 0x6F, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
0xFF, 0xFF, 0x00, 0x7F, 0x01, 0x01, 0x49, 0x55, 0x25, 0x01, 0x09, 0x7D, 0x09, 0x01, 0x79, 0x15,
0x79, 0x01, 0x7D, 0x35, 0x59, 0x01, 0x09, 0x7D, 0x09, 0x01, 0x7F, 0x00, 0xFF, 0xFF, 0xFF, 0xFF
};
