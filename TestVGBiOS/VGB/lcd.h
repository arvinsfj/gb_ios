//
//  lcd.h
//  TestVGB
//
//  Created by vin on 2018/8/24.
//  Copyright © 2018年 vin. All rights reserved.
//

#ifndef lcd_h
#define lcd_h

struct LCD {
    int windowX;
    int windowY;
    int scrollX;
    int scrollY;
    int line;
    int frame;
    int lyCompare;
};

// LCD Control
struct LCDC {
    int lcdDisplay; // 7
    int windowTileMap; // 6
    int windowDisplay; // 5
    int tileDataSelect; // 4
    int tileMapSelect; // 3
    int spriteSize; // 2
    int spriteDisplay; // 1
    int bgWindowDisplay; // 0
};

// LCD STAT
struct LCDS {
    int lyInterrupt;
    int oamInterrupt;
    int vblankInterrupt;
    int hblankInterrupt;
    int lyFlag;
    int modeFlag;
};

struct sprite {
    int y;
    int x;
    int patternNum;
    int flags;
};

void setLCDC(unsigned char value);
void setLCDS(unsigned char value);
void setBGPalette(unsigned char value);
void setSpritePalette1(unsigned char value);
void setSpritePalette2(unsigned char value);
void setScrollX(unsigned char value);
void setScrollY(unsigned char value);
void setWindowX(unsigned char value);
void setWindowY(unsigned char value);
void setLyCompare(unsigned char value);

unsigned char getLCDC(void);
unsigned char getLCDS(void);
unsigned char getScrollX(void);
unsigned char getScrollY(void);
unsigned char getWindowX(void);
unsigned char getWindowY(void);
int getLine(void);

int lcdCycle(void);

#endif /* lcd_h */
