// >>>>>  T-I-N-Y  S-P-A-C-E   I-N-V-A-D-E-R-S for ATTINY85  GPL v3 <<<<
//                  Programmer: Daniel Champagne 2018
//                 Contact EMAIL: phoenixbozo@gmail.com
//           https://sites.google.com/view/arduino-collection

//  Tiny Space Invaders is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.

//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

//  the code work at 16MHZ internal
//  and use ssd1306xled Library for SSD1306 oled display 128x64

//  Code formatted and adjusted to work with skeleton handheld by @davdarko
//  for element14presents episode

#include <ssd1306xled.h>
#include "spritebank.h"

#define MAXLEVELSHIELDED 3

// var public
uint8_t Live = 0;
uint8_t ShieldRemoved = 0;
uint8_t MONSTERrest = 0;
uint8_t LEVELS = 0;
uint8_t SpeedShootMonster = 0;
uint8_t ShipDead = 0;
uint8_t ShipPos = 64;
// fin var public

void setup() {
  SSD1306.ssd1306_init();
  pinMode(1, INPUT);
  DDRB = DDRB | 0b00010000;
  pinMode(A0, INPUT);
}

void LoadMonstersLevels(int8_t Levels, SPACE *space) {
  uint8_t x, y;
  for (y = 0; y < 4; y++) {
    for (x = 0; x < 6; x++) {
      space->MonsterGrid[y][x] = pgm_read_byte(&MonstersLevels[(Levels * 24) + ((y) * 6) + (x)]);
    }
  }
}

void loop() {
#define SHOOTS 2
  uint8_t Decompte = 0;
  uint8_t VarPot = 60;
  uint8_t MyShootReady = SHOOTS;
  SPACE space;
NEWGAME:;
  Live = 3;
  LEVELS = 0;
  while (1) {
    Tiny_Flip(1, &space);
    if (digitalRead(1) == 0) {
      Sound(100, 125);
      Sound(50, 125);
      _delay_ms(500);
      goto BYPASS2;
    }
  }
NEWLEVEL:
  _delay_ms(1000);
BYPASS2:
  VarResetNewLevel(&space);
  SpeedControle(&space);
  goto Bypass;
RestartLevel:
  _delay_ms(1000);
  if (Live > 0) {
    Live--;
  } else {
    goto NEWGAME;
  }
Bypass:
  ShipDead = 0;
  Decompte = 0;
  while (1) {
    if (MONSTERrest == 0) {
      Sound(110, 255); _delay_ms(40); Sound(130, 255); _delay_ms(40); Sound(100, 255);
      _delay_ms(40); Sound(1, 155); _delay_ms(20); Sound(60, 255); Sound(60, 255);
      if (LEVELS < 9) {
        LEVELS++;
      }
      goto NEWLEVEL;
    }
    if ((((space.MonsterGroupeYpos) + (space.MonsterFloorMax + 1)) == 7) && (Decompte == 0)) {
      ShipDead = 1;
    }
    if (SpeedShootMonster <= ((9 - LEVELS))) {
      SpeedShootMonster++;
    } else {
      SpeedShootMonster = 0;
      MonsterShootGenerate(&space);
    }
    space.ScrBackV = (ShipPos / 14) + 52;
    Tiny_Flip(0, &space);
    space.oneFrame = !space.oneFrame;
    RemoveExplodOnMonsterGrid(&space);
    MonsterShootupdate(&space);
    UFOUpdate(&space);
    if (((space.MonsterGroupeXpos >= 26) && (space.MonsterGroupeXpos <= 28)) && (space.MonsterGroupeYpos == 2) && (space.DecalageY8 == 4)) {
      space.UFOxPos = 127;
    }
    if (VarPot > (ShipPos + 2)) {
      ShipPos = ShipPos + ((VarPot - ShipPos) / 3);
    }
    if (VarPot < (ShipPos - 2)) {
      ShipPos = ShipPos - ((ShipPos - VarPot) / 3);
    }
    if (ShipDead != 1) {
      if (space.frame < space.frameMax) {
        space.frame++;
      } else {
        GRIDMonsterFloorY(&space);
        space.anim = !space.anim;
        if (space.anim == 0) {
          Sound(100, 1);
        } else {
          Sound(200, 1);
        } MonsterRefreshMove(&space);
        space.frame = 0;
      }
      
      //VarPot=map(analogRead(A3),0,1023,0,114);
      // left
      if ((analogRead(A3) >= 340) && (analogRead(A3) < 350)) {
        if (VarPot > 5) {
          VarPot = VarPot - 6;
        }
      }
      // right
      if ((analogRead(A3) > 95) && (analogRead(A3) < 105)) {
        if (VarPot < 108) {
          VarPot = VarPot + 6;
        }
      }
      if ((digitalRead(1) == 0) && (MyShootReady == SHOOTS)) {
        Sound(200, 4);
        MyShootReady = 0;
        space.MyShootBall = 6;
        space.MyShootBallxpos = ShipPos + 6;
      }
    } else {
      Sound(80, 1); Sound(100, 1);
      Decompte++;
      if (Decompte >= 30) {
        _delay_ms(600);
        if (((space.MonsterGroupeYpos) + (space.MonsterFloorMax + 1)) == 7) {
          goto NEWGAME;
        } else {
          goto RestartLevel;
        }
      }
    }
    if (space.MyShootBall == -1) {
      if (MyShootReady < SHOOTS) {
        MyShootReady++;
      }
    }
  }
}
////////////////////////////////// main end /////////////////////////////////

void SpeedControle(SPACE *space) {
  uint8_t xx = 00, yy = 0;
  MONSTERrest = 0;
  for (yy = 0; yy <= 3; yy++) {
    for (xx = 0; xx <= 5; xx++) {
      if ((space->MonsterGrid[yy][xx] != -1) && ((space->MonsterGrid[yy][xx] <= 5)) ) {
        MONSTERrest++;
      }
    }
  } space->frameMax = (MONSTERrest / 8 );
}

void GRIDMonsterFloorY(SPACE *space) {
  uint8_t y, x;
  space->MonsterFloorMax = 3;
  for (y = 3; y != 0; y--) {
    for (x = 0; x != 5; x++) {
      if (space->MonsterGrid[y][x] != -1) {
        goto FIN;
      }
    }
    space->MonsterFloorMax = space->MonsterFloorMax - 1;
} FIN:;
}

uint8_t LivePrint(uint8_t x, uint8_t y) {
#define XLIVEWIDE ((5*Live)-1)
  if (((0 >= (x - XLIVEWIDE))) && (y == 7)) {
    return pgm_read_byte(&LIVE[(x)]);
  }
  return 0x00;
}

void Tiny_Flip(uint8_t render0_picture1, SPACE *space) {
  uint8_t y, x;
  uint8_t MYSHIELD = 0x00;
  for (y = 0; y < 8; y++)
  {
    SSD1306.ssd1306_send_command(0xb0 + y); // page0 - page1
    SSD1306.ssd1306_send_command(0x00);   // low column start address
    SSD1306.ssd1306_send_command(0x10);   // high column start address
    SSD1306.ssd1306_send_data_start();
    for (x = 0; x < 128; x++)
    {
      if (render0_picture1 == 0) {
        if (ShieldRemoved == 0) {
          MYSHIELD = MyShield(x, y, space);
        } else {
          MYSHIELD = 0x00;
        }
        SSD1306.ssd1306_send_byte(background(x, y, space) | LivePrint(x, y) | Vesso(x, y, space) | UFOWrite(x, y, space) | Monster(x, y, space) | MyShoot(x, y, space) | MonsterShoot(x, y, space) | MYSHIELD);
      } else {
        SSD1306.ssd1306_send_byte(pgm_read_byte(&intro[x + (y * 128)]));
      }
    }
    if (render0_picture1 == 0) {
      if (ShieldRemoved == 0) {
        ShieldDestroy(0, space->MyShootBallxpos, space->MyShootBall, space);
      }
      SSD1306.ssd1306_send_data_stop();
    }
  }
  if (render0_picture1 == 0) {
    if ((space->MonsterGroupeYpos < (2 + (4 - (space->MonsterFloorMax + 1))))/*&&(LEVELS<=MAXLEVELSHIELDED)*/) {} else {
      if (ShieldRemoved != 1) {
        space->Shield[0] = 0x00;
        space->Shield[1] = 0x00;
        space->Shield[2] = 0x00;
        space->Shield[3] = 0x00;
        space->Shield[4] = 0x00;
        space->Shield[5] = 0x00;
        ShieldRemoved = 1;
      }
    }
  }
}

uint8_t UFOWrite(uint8_t x, uint8_t y, SPACE *space) {
  if ((space->UFOxPos != -120) && (y == 0) && ((space->UFOxPos <= (x)) && (space->UFOxPos >= (x - 14)))) {
    return pgm_read_byte(&Monsters[(x - space->UFOxPos) + (6 * 14) + (space->oneFrame * 14)]);
  }
  return 0x00;
}

void UFOUpdate(SPACE *space) {
  if (space->UFOxPos != -120) {
    space->UFOxPos = space->UFOxPos - 2;
    if (space->UFOxPos <= -20) {
      space->UFOxPos = -120;
    }
  }
}

void ShipDestroyByMonster(SPACE *space) {
  if ((space->MonsterShoot[1][0] >= 14) && (space->MonsterShoot[1][0] <= 15) && (space->MonsterShoot[0][0] >= ShipPos) && (space->MonsterShoot[0][0] <= ShipPos + 14)) {
    ShipDead = 1;
  }
}

void MonsterShootupdate(SPACE *space) {
  if (space->MonsterShoot[1][0] != 16) {
    ShipDestroyByMonster(space);
    if (ShieldDestroy(1, space->MonsterShoot[0][0], space->MonsterShoot[1][0] / 2, space)) {
      space->MonsterShoot[1][0] = 16;
    } else {
      space->MonsterShoot[1][0] = space->MonsterShoot[1][0] + 1;
    }
  }
}

void MonsterShootGenerate(SPACE *space) {
  uint8_t a = random() % 3;
  uint8_t b = random() % 6;
  if (b >= 5) {
    b = 5;
  }
  if (space->MonsterShoot[1][0] == 16) {
    if (space->MonsterGrid[a][b] != -1) {
      space->MonsterShoot[0][0] = (space->MonsterGroupeXpos + 7) + (b * 14);
      space->MonsterShoot[1][0] = (((space->MonsterGroupeYpos) + a) * 2) + 1;
    }
  }
}

uint8_t MonsterShoot(uint8_t x, uint8_t y, SPACE *space) {
  if ((((space->MonsterShoot[1][0]) / 2) == y) && (space->MonsterShoot[0][0] == x) ) {
    if (((space->MonsterShoot[1][0 ]) % 2) == 0) {
      return 0b00001111;
    } else {
      return 0b11110000;
    }
  }
  return 0x00;
}

uint8_t ShieldDestroy(uint8_t Origine, uint8_t VarX, uint8_t VarY, SPACE *space) {
#define OFFSETXSHIELD -1
  if (VarY == 6) {
    if (((VarX >= (20 + OFFSETXSHIELD)) && (VarX <= (27 + OFFSETXSHIELD)))) {
      if ((BOOLREAD(0, (VarX - (20 + OFFSETXSHIELD)), space))) {
        ShieldDestroyWrite(0, (VarX - (20 + OFFSETXSHIELD)), space, Origine);
        return 1;
      }
    }
    if (((VarX >= (28 + OFFSETXSHIELD)) && (VarX <= (35 + OFFSETXSHIELD)))) {
      if ((BOOLREAD(1, (VarX - (28 + OFFSETXSHIELD)), space))) {
        ShieldDestroyWrite(1, (VarX - (28 + OFFSETXSHIELD)), space, Origine);
        return 1;
      }
    }
    if (((VarX >= (55 + OFFSETXSHIELD)) && (VarX <= (62 + OFFSETXSHIELD)))) {
      if ((BOOLREAD(2, (VarX - (55 + OFFSETXSHIELD)), space))) {
        ShieldDestroyWrite(2, (VarX - (55 + OFFSETXSHIELD)), space, Origine);
        return 1;
      }
    }
    if (((VarX >= (63 + OFFSETXSHIELD)) && (VarX <= (70 + OFFSETXSHIELD)))) {
      if ((BOOLREAD(3, (VarX - (63 + OFFSETXSHIELD)), space))) {
        ShieldDestroyWrite(3, (VarX - (63 + OFFSETXSHIELD)), space, Origine);
        return 1;
      }
    }
    if (((VarX >= (90 + OFFSETXSHIELD)) && (VarX <= (97 + OFFSETXSHIELD)))) {
      if ((BOOLREAD(4, (VarX - (90 + OFFSETXSHIELD)), space))) {
        ShieldDestroyWrite(4, (VarX - (90 + OFFSETXSHIELD)), space, Origine);
        return 1;
      }
    }
    if (((VarX >= (98 + OFFSETXSHIELD)) && (VarX <= (105 + OFFSETXSHIELD)))) {
      if ((BOOLREAD(5, (VarX - (98 + OFFSETXSHIELD)), space))) {
        ShieldDestroyWrite(5, (VarX - (98 + OFFSETXSHIELD)), space, Origine);
        return 1;
      }
    }
  }
  return 0;
}


void ShieldDestroyWrite(uint8_t BOOLWRITE, uint8_t line, SPACE *space, uint8_t Origine) {
  switch (line) {
    case 0: space->Shield[BOOLWRITE] = space->Shield[BOOLWRITE] - 128; if (Origine == 0) {
        space->MyShootBall = -1;
      } break;
    case 1: space->Shield[BOOLWRITE] = space->Shield[BOOLWRITE] - 64; if (Origine == 0) {
        space->MyShootBall = -1;
      } break;
    case 2: space->Shield[BOOLWRITE] = space->Shield[BOOLWRITE] - 32; if (Origine == 0) {
        space->MyShootBall = -1;
      } break;
    case 3: space->Shield[BOOLWRITE] = space->Shield[BOOLWRITE] - 16; if (Origine == 0) {
        space->MyShootBall = -1;
      } break;
    case 4: space->Shield[BOOLWRITE] = space->Shield[BOOLWRITE] - 8; if (Origine == 0) {
        space->MyShootBall = -1;
      } break;
    case 5: space->Shield[BOOLWRITE] = space->Shield[BOOLWRITE] - 4; if (Origine == 0) {
        space->MyShootBall = -1;
      } break;
    case 6: space->Shield[BOOLWRITE] = space->Shield[BOOLWRITE] - 2; if (Origine == 0) {
        space->MyShootBall = -1;
      } break;
    case 7: space->Shield[BOOLWRITE] = space->Shield[BOOLWRITE] - 1; if (Origine == 0) {
        space->MyShootBall = -1;
      } break;
    default: break;
  }
}


uint8_t MyShield(uint8_t x, uint8_t y, SPACE *space) {
#define OFFSETXSHIELD -1
  if (((x >= (20 + OFFSETXSHIELD)) && (x <= (27 + OFFSETXSHIELD))) && (y == 6)) {
    if ((BOOLREAD(0, (x - (20 + OFFSETXSHIELD)), space))) {
      return ShieldBlitz(0, (x - (20 + OFFSETXSHIELD)));
    } else {
      return 0x00;
    }
  }
  if (((x >= (28 + OFFSETXSHIELD)) && (x <= (35 + OFFSETXSHIELD))) && (y == 6)) {
    if ((BOOLREAD(1, (x - (28 + OFFSETXSHIELD)), space))) {
      return ShieldBlitz(1, (x - (28 + OFFSETXSHIELD)));
    } else {
      return 0x00;
    }
  }
  if (((x >= (55 + OFFSETXSHIELD)) && (x <= (62 + OFFSETXSHIELD))) && (y == 6)) {
    if ((BOOLREAD(2, (x - (55 + OFFSETXSHIELD)), space))) {
      return ShieldBlitz(0, (x - (55 + OFFSETXSHIELD)));
    } else {
      return 0x00;
    }
  }
  if (((x >= (63 + OFFSETXSHIELD)) && (x <= (70 + OFFSETXSHIELD))) && (y == 6)) {
    if ((BOOLREAD(3, (x - (63 + OFFSETXSHIELD)), space))) {
      return ShieldBlitz(1, (x - (63 + OFFSETXSHIELD)));
    } else {
      return 0x00;
    }
  }
  if (((x >= (90 + OFFSETXSHIELD)) && (x <= (97 + OFFSETXSHIELD))) && (y == 6)) {
    if ((BOOLREAD(4, (x - (90 + OFFSETXSHIELD)), space))) {
      return ShieldBlitz(0, (x - (90 + OFFSETXSHIELD)));
    } else {
      return 0x00;
    }
  }
  if (((x >= (98 + OFFSETXSHIELD)) && (x <= (105 + OFFSETXSHIELD))) && (y == 6)) {
    if ((BOOLREAD(5, (x - (98 + OFFSETXSHIELD)), space))) {
      return ShieldBlitz(1, (x - (98 + OFFSETXSHIELD)));
    } else {
      return 0x00;
    }
  }
  return 0x00;
}

uint8_t ShieldBlitz(uint8_t Part, uint8_t LineSH ) {
  uint8_t Var0 = 0;
  switch (LineSH) {
    case 0: if (Part == 0) {
        Var0 = 0b11110000;
      } else {
        Var0 = 0b00001111;
      } break;
    case 1: if (Part == 0) {
        Var0 = 0b11111100;
      } else {
        Var0 = 0b00001111;
      } break;
    case 2:
    case 3:
    case 4:
    case 5: Var0 = 0b00001111; break;
    case 6: if (Part == 1) {
        Var0 = 0b11111100;
      } else {
        Var0 = 0b00001111;
      } break;
    case 7: if (Part == 1) {
        Var0 = 0b11110000;
      } else {
        Var0 = 0b00001111;
      } break;
    default: Var0 = 0b00000000; break;
  }
  return Var0;
}

uint8_t BOOLREAD(uint8_t SHnum, uint8_t LineSH, SPACE *space ) {
  uint8_t Var0 = 0;
  switch (LineSH) {
    case 0: Var0 = 0b10000000; break;
    case 1: Var0 = 0b01000000; break;
    case 2: Var0 = 0b00100000; break;
    case 3: Var0 = 0b00010000; break;
    case 4: Var0 = 0b00001000; break;
    case 5: Var0 = 0b00000100; break;
    case 6: Var0 = 0b00000010; break;
    case 7: Var0 = 0b00000001; break;
    default: Var0 = 0b00000000; break;
  }
  if ((space->Shield[SHnum]&Var0) != 0) {
    return 1;
  } else {
    return 0;
  }
}

void RemoveExplodOnMonsterGrid(SPACE *space) {
  uint8_t x = 0, y = 0;
  for (y = 0; y <= 3; y++) { //était a 5
    for (x = 0; x <= 5; x++) {
      if (space->MonsterGrid[y][x] >= 11) {
        space->MonsterGrid[y][x] = -1;
      }
      if (space->MonsterGrid[y][x] >= 8) {
        space->MonsterGrid[y][x] = space->MonsterGrid[y][x] + 1;
      }
    }
  }
}

uint8_t background(uint8_t x, uint8_t y, SPACE *space) {
  uint8_t scr = (space->ScrBackV + x);
  if ((scr) > 127) {
    scr = (space->ScrBackV + x) - 128;
  }
  return 0xff - pgm_read_byte(&back[((y) * 128) + ((scr))]);
}

uint8_t Vesso(uint8_t x, uint8_t y, SPACE *space) {
  if (((x - ShipPos) >= 0) && ((x - ShipPos) < 13) && (y == 7)) {
    if (ShipDead == 0) {
      return pgm_read_byte(&vesso[(x - ShipPos)]);
    } else {
      return pgm_read_byte(&vesso[(x - ShipPos) + (12 * space->oneFrame)]);
    }
  }
  return 0;
}

void UFO_Attack_Check(uint8_t x, SPACE *space) {
  if (space->MyShootBall == 0) {
    if ((space->MyShootBallxpos >= space->UFOxPos) && (space->MyShootBallxpos <= (space->UFOxPos + 14))) {
      for (x = 1; x < 100; x++) {
        Sound(x, 1);
      }
      if (Live < 3) Live++;
      space->UFOxPos = -120;
    }
  }
}

uint8_t MyShoot(uint8_t x, uint8_t y, SPACE *space) {
  if ((space->MyShootBallxpos == x) && (y == ((space->MyShootBall)))) {
    if (space->MyShootBall > -1) {
      space->MyShootBallFrame = !space->MyShootBallFrame;
    } else {
      return 0x00;
    }
    if (space->MyShootBallFrame == 1) {
      space->MyShootBall--;
    }
    Monster_Attack_Check(space);
    UFO_Attack_Check(x, space);
    return pgm_read_byte(&SHOOT[(space->MyShootBallFrame)]);
  }
  return 0x00;
}

void Monster_Attack_Check(SPACE *space) {
  int8_t Varx = 0, Vary = 0;
#define Xmouin (space->MonsterGroupeXpos)
#define Ymouin ((space->MonsterGroupeYpos)*8)//-space->DecalageY8
#define XPlus (Xmouin+84)
#define YPlus (Ymouin+(4*8))
#define MYSHOOTX (space->MyShootBallxpos)
#define MYSHOOTY ((space->MyShootBall*8)+(((space->MyShootBallFrame)+1)*4))
  if ((MYSHOOTX >= Xmouin) && (MYSHOOTX <= XPlus) && (MYSHOOTY >= (Ymouin)) && (MYSHOOTY <= YPlus)) {
    //enter in the monster zone
    Vary = (round((MYSHOOTY - Ymouin) / 8));
    Varx = (round((MYSHOOTX - Xmouin) / 14));
    if (Varx < 0) {
      Varx = 0;
    }
    if (Vary < 0) {
      Vary = 0;
    }
    if (Varx > 5) {
      goto End;
    }
    if (Vary > 3) {
      goto End;
    }
    if ((space->MonsterGrid[Vary][Varx] > -1) && (space->MonsterGrid[Vary][Varx] < 6)) {
      Sound(50, 10);
      space->MonsterGrid[Vary][Varx] = 8;
      space->MyShootBall = -1;
      SpeedControle(space);
    }
    //fin monster zone
  }
End:;
}

int8_t OuDansLaGrilleMonster(uint8_t x, uint8_t y, SPACE *space) {
  if (x < space->MonsterGroupeXpos) {
    return -1;
  }
  if (y < space->MonsterGroupeYpos) {
    return -1;
  }
  space->PositionDansGrilleMonsterX = (x - space->MonsterGroupeXpos) / 14;
  space->PositionDansGrilleMonsterY = (y - space->MonsterGroupeYpos);
  if ((space->PositionDansGrilleMonsterX) > 5)  {
    return -1;
  }
  if ((space->PositionDansGrilleMonsterY) > 4)  {
    return -1;
  }
  return 0;
}

uint8_t SplitSpriteDecalageY(uint8_t Input, uint8_t UPorDOWN, SPACE *space) {
  if (UPorDOWN) {
    return Input << space->DecalageY8;
  } else {
    return Input >> (8 - space->DecalageY8);
  }
}

uint8_t Murge_Split_UP_DOWN(uint8_t x, SPACE *space) {
  int8_t SpriteType = -1;
  int8_t ANIMs = -1;
  uint8_t Murge1 = 0;
  uint8_t Murge2 = 0;
  if (space->DecalageY8 == 0) {
    SpriteType = space->MonsterGrid[space->PositionDansGrilleMonsterY][space->PositionDansGrilleMonsterX];
    if (SpriteType < 8) {
      ANIMs = (space->anim * 14);
    } else {
      ANIMs = 0;
    }
    if (SpriteType == -1) {
      return 0x00;
    }
    return pgm_read_byte(&Monsters[(WriteMonster14(x - space->MonsterGroupeXpos) + SpriteType * 14) + ANIMs]);
  } else { //debut
    if (space->PositionDansGrilleMonsterY == 0) {
      SpriteType = space->MonsterGrid[space->PositionDansGrilleMonsterY][space->PositionDansGrilleMonsterX];
      if (SpriteType < 8) {
        ANIMs = (space->anim * 14);
      } else {
        ANIMs = 0;
      }
      if (SpriteType != -1) {
        Murge2 = SplitSpriteDecalageY(pgm_read_byte(&Monsters[(WriteMonster14(x - space->MonsterGroupeXpos) + SpriteType * 14) + ANIMs]), 1, space);
      } else {
        Murge2 = 0x00;
      }
      return Murge2;
    } else {
      SpriteType = space->MonsterGrid[space->PositionDansGrilleMonsterY - 1][space->PositionDansGrilleMonsterX];
      if (SpriteType < 8) {
        ANIMs = (space->anim * 14);
      } else {
        ANIMs = 0;
      }
      if (SpriteType != -1) {
        Murge1 = SplitSpriteDecalageY(pgm_read_byte(&Monsters[(WriteMonster14(x - space->MonsterGroupeXpos) + SpriteType * 14) + ANIMs]), 0, space);
      } else {
        Murge1 = 0x00;
      }
      SpriteType = space->MonsterGrid[space->PositionDansGrilleMonsterY][space->PositionDansGrilleMonsterX];
      if (SpriteType < 8) {
        ANIMs = (space->anim * 14);
      } else {
        ANIMs = 0;
      }
      if (SpriteType != -1) {
        Murge2 = SplitSpriteDecalageY(pgm_read_byte(&Monsters[(WriteMonster14(x - space->MonsterGroupeXpos) + SpriteType * 14) + ANIMs]), 1, space);
      } else {
        Murge2 = 0x00;
      }
      return Murge1 | Murge2;
    }
  } //fin
}

uint8_t WriteMonster14(uint8_t x) {
  while (1) {
    if ((x - 14) >= 0) {
      x = x - 14;
    } else {
      break;
    }
  } return x;
}

uint8_t Monster(uint8_t x, uint8_t y, SPACE *space) {
  if (OuDansLaGrilleMonster(x, y, space) != -1) {
  } else {
    return 0x00; //quiter la fonction si pas dans la grille
  }
  return  Murge_Split_UP_DOWN(x, space);
  return 0x00;
}//end Monster();

uint8_t MonsterRefreshMove(SPACE *space) {
  if (space->Direction == 1) {
    if ((space->MonsterGroupeXpos < space->MonsterOffsetDroite)) {
      space->MonsterGroupeXpos = space->MonsterGroupeXpos + 2;
      return 0;
    } else {
      if (space->DecalageY8 < 7) {
        space->DecalageY8 = space->DecalageY8 + 4;
        if (space->DecalageY8 > 7) {
          space->DecalageY8 = 7;
        }
      } else {
        space->MonsterGroupeYpos++;
        space->DecalageY8 = 0;
      }
      space->Direction = 0; return 0;
    }
  } else {
    if ((space->MonsterGroupeXpos > space->MonsterOffsetGauche)) {
      space->MonsterGroupeXpos = space->MonsterGroupeXpos - 2;
      return 0;
    } else {
      if (space->DecalageY8 < 7) {
        space->DecalageY8 = space->DecalageY8 + 4;
        if (space->DecalageY8 > 7) {
          space->DecalageY8 = 7;
        }
      } else {
        space->MonsterGroupeYpos++;
        space->DecalageY8 = 0;
      }
      space->Direction = 1; return 0;
    }
  }
}


void Sound(uint8_t freq, uint8_t dur) {
  for (uint8_t t = 0; t < dur; t++) {
    if (freq != 0) PORTB = PORTB | 0b00010000;
    for (uint8_t t = 0; t < (255 - freq); t++) {
      _delay_us(1);
    }
    PORTB = PORTB & 0b11101111;
    for (uint8_t t = 0; t < (255 - freq); t++) {
      _delay_us(1);
    }
  }
}

void VarResetNewLevel(SPACE *space) {
  //space->ScrBackV=0;
  ShieldRemoved = 0;
  SpeedShootMonster = 0;
  MONSTERrest = 24;
  LoadMonstersLevels(LEVELS, space);
  space->Shield[0] = 255;
  space->Shield[1] = 255;
  space->Shield[2] = 255;
  space->Shield[3] = 255;
  space->Shield[4] = 255;
  space->Shield[5] = 255;
  space->MonsterShoot[0][0] = 16;
  space->MonsterShoot[1][0] = 16;
  space->UFOxPos = -120;

  space->MyShootBall = -1;
  space->MyShootBallxpos = 0;
  space->MyShootBallFrame = 0;
  space->anim = 0;
  space->frame = 0;
  space->PositionDansGrilleMonsterX = 0;
  space->PositionDansGrilleMonsterY = 0;
  space->MonsterFloorMax = 3;
  space->MonsterOffsetGauche = 0;
  space->MonsterOffsetDroite = 44;
  space->MonsterGroupeXpos = 0;
  if (LEVELS > 3) {
    space->MonsterGroupeYpos = 1;
  } else {
    space->MonsterGroupeYpos = 0;
  }
  space->DecalageY8 = 0;
  space->frameMax = 8;
  space->Direction = 1; //1 right 0 gauche
  space->oneFrame = 0;
}
