//
//  lcd.c
//  TestVGB
//
//  Created by vin on 2018/8/24.
//  Copyright © 2018年 vin. All rights reserved.
//

#include "lcd.h"

#include "cpu.h"
#include "interrupt.h"
#include "mmu.h"

struct LCD LCD;
struct LCDC LCDC;
struct LCDS LCDS;


int bgPalette[] = {3,2,1,0};
int spritePalette1[] = {0, 1, 2, 3};
int spritePalette2[] = {0, 1, 2, 3};
unsigned int colours[4] = {0xFFFFFF, 0xC0C0C0, 0x808080, 0x000000};

//////////////////////////////////////////////////

// 获取或设置lcd寄存器
void setLCDC(unsigned char value)
{
    LCDC.lcdDisplay = (!!(value & 0x80));
    LCDC.windowTileMap = (!!(value & 0x40));
    LCDC.windowDisplay = (!!(value & 0x20));
    LCDC.tileDataSelect = (!!(value & 0x10));
    LCDC.tileMapSelect = (!!(value & 0x08));
    LCDC.spriteSize = (!!(value & 0x04));
    LCDC.spriteDisplay = (!!(value & 0x02));
    LCDC.bgWindowDisplay = (!!(value & 0x01));
}

unsigned char getLCDC(void)
{
    return ((LCDC.lcdDisplay << 7) | (LCDC.windowTileMap << 6) | (LCDC.windowDisplay << 5) | (LCDC.tileDataSelect << 4) | (LCDC.tileMapSelect << 3) | (LCDC.spriteSize << 2) | (LCDC.spriteDisplay << 1) | (LCDC.bgWindowDisplay));
}

void setLCDS(unsigned char value)
{
    LCDS.lyInterrupt = (!!(value & 0x40));
    LCDS.oamInterrupt = ((value & 0x20) >> 5);
    LCDS.vblankInterrupt = ((value & 0x10) >> 4);
    LCDS.hblankInterrupt = ((value & 0x08) >> 3);
    LCDS.lyFlag = ((value & 0x04) >> 2);
    LCDS.modeFlag = ((value & 0x03));
}

unsigned char getLCDS(void)
{
    return ((LCDS.lyInterrupt << 6) | (LCDS.oamInterrupt << 5) | (LCDS.vblankInterrupt << 4) | (LCDS.hblankInterrupt << 3) | (LCDS.lyFlag << 2) | (LCDS.modeFlag));
}

void setBGPalette(unsigned char value)
{
    bgPalette[3] = ((value >> 6) & 0x03);
    bgPalette[2] = ((value >> 4) & 0x03);
    bgPalette[1] = ((value >> 2) & 0x03);
    bgPalette[0] = ((value) & 0x03);
}

void setSpritePalette1(unsigned char value)
{
    spritePalette1[3] = ((value >> 6) & 0x03);
    spritePalette1[2] = ((value >> 4) & 0x03);
    spritePalette1[1] = ((value >> 2) & 0x03);
    spritePalette1[0] = 0;
}

void setSpritePalette2(unsigned char value)
{
    spritePalette2[3] = ((value >> 6) & 0x03);
    spritePalette2[2] = ((value >> 4) & 0x03);
    spritePalette2[1] = ((value >> 2) & 0x03);
    spritePalette2[0] = 0;
}

void setScrollX(unsigned char value)
{
    LCD.scrollX = value;
}

unsigned char getScrollX(void)
{
    return LCD.scrollX;
}

void setScrollY(unsigned char value)
{
    LCD.scrollY = value;
}

unsigned char getScrollY(void)
{
    return LCD.scrollY;
}

void setWindowX(unsigned char value)
{
    LCD.windowX = value;
}

void setWindowY(unsigned char value)
{
    LCD.windowY = value;
}

int getLine(void)
{
    return LCD.line;
}

void setLyCompare(unsigned char value)
{
    LCD.lyCompare = (LCD.line == value);
}

/////////////////////////////////////////////////////////////////////////

void sortSprites(struct sprite* sprite, int c)
{
    // blessed insertion sort
    static struct sprite s;
    for (int i = 0; i < c; i++) {
        for (int j = 0; j < c-1; j++) {
            if (sprite[j].x < sprite[j+1].x) {
                s = sprite[j+1];
                sprite[j+1] = sprite[j];
                sprite[j] = s;
            }
        }
    }
}

void drawBgWindow(unsigned int *buf, int line)
{
    unsigned int mapSelect, tileMapOffset, tileNum, tileAddr, currX, currY;
    unsigned char buf1, buf2, mask, colour;
    
    for(int x = 0; x < 160; x++) // for the x size of the window (160x144)
    {
        if(line >= LCD.windowY && LCDC.windowDisplay && line - LCD.windowY < 144) {
            // wind
            currX = x;
            currY = line - LCD.windowY;
            mapSelect = LCDC.windowTileMap;
            
        } else {
            // background
            if (!LCDC.bgWindowDisplay) {
                buf[line*160 + x] = 0; // if not window or background, make it white
                return;
            }
            currX = (x + LCD.scrollX) % 256; // mod 256 since if it goes off the screen, it wraps around
            currY = (line + LCD.scrollY) % 256;
            mapSelect = LCDC.tileMapSelect;
        }
        
        // map window to 32 rows of 32 bytes
        tileMapOffset = (currY/8)*32 + currX/8;
        
        tileNum = read8(0x9800 + mapSelect*0x400 + tileMapOffset);
        if(LCDC.tileDataSelect) {
            tileAddr = 0x8000 + (tileNum*16);
        } else {
            tileAddr = 0x9000 + (((signed int)tileNum)*16); // pattern 0 lies at 0x9000
        }
        
        buf1 = read8(tileAddr + (currY%8)*2); // 2 bytes represent the line
        buf2 = read8(tileAddr + (currY%8)*2 + 1);
        mask = 128>>(currX%8);
        colour = (!!(buf2&mask)<<1) | !!(buf1&mask);
        buf[line*160 + x] = colours[bgPalette[colour]];
    }
}

void drawSprites(unsigned int *buf, int line, int blocks, struct sprite *sprite)
{
    unsigned int buf1, buf2, tileAddr, spriteRow, x;
    unsigned char mask, colour; int *pal;
    
    for(int i = 0; i < blocks; i++)
    {
        // off screen
        if(sprite[i].x < -7) continue;
        
        spriteRow = sprite[i].flags & 0x40 ? (LCDC.spriteSize ? 15 : 7)-(line - sprite[i].y) : line -sprite[i].y;
        
        // similar to background
        tileAddr = 0x8000 + (sprite[i].patternNum*16) + spriteRow*2;
        
        buf1 = read8(tileAddr);
        buf2 = read8(tileAddr+1);
        
        // draw each pixel
        for(x = 0; x < 8; x++) {
            // out of bounds check
            if((sprite[i].x + x) >= 160) continue;
            
            if((sprite[i].x + x) >= 160) continue;
            
            mask = sprite[i].flags & 0x20 ? 128>>(7-x) : 128>>x;
            colour = ((!!(buf2&mask))<<1) | !!(buf1&mask);
            
            if(colour == 0) continue;
            
            pal = (sprite[i].flags & 0x10) ? spritePalette2 : spritePalette1;
            
            // only render over colour 0
            if(sprite[i].flags & 0x80) {
                unsigned int temp = buf[line*160+(x + sprite[i].x)];
                if(temp != colours[bgPalette[0]]) continue;
            }
            buf[line*160+(x + sprite[i].x)] = colours[pal[colour]]; // for testing
        }
    }
}

unsigned int* getPixels(void);

void renderLine(int line)
{
    int c = 0; // block counter
    struct sprite sprite[10]; // max 10 sprites per line
    unsigned int *buf = getPixels();//获取像素数组RGBA
    int y = 0;
    
    // OAM is divided into 40 4-byte blocks each - corresponding to a sprite
    for (int i = 0; i < 40; i++)
    {
        y = read8(0xFE00 + (i*4)) - 16;
        if (line < y || line >= y + 8 + (8*LCDC.spriteSize)) continue;
        
        sprite[c].y = y;
        sprite[c].x = read8(0xFE00 + (i*4) + 1) - 8;
        sprite[c].patternNum = read8(0xFE00 + (i*4) + 2);
        sprite[c].flags = read8(0xFE00 + (i*4) + 3);
        
        if (++c == 10) break; // max 10 sprites per line
    }
    
    if (c) sortSprites(sprite, c);
    
    drawBgWindow(buf, line);
    drawSprites(buf, line, c, sprite);
}

///////////////////////////////////////////////////////////////////////

void wnd_draw(uint8_t* pixels);
int wnd_updateEvent(void);

// lcd循环
int lcdCycle()
{
    int cycles = getCycles();
    static int prevLine;
    
    int this_frame;
    int end = 0;
    
    this_frame = cycles % (70224/4); // 70224 clks per screen
    LCD.line = this_frame / (456/4); // 465 clks per line
    
    if (this_frame < 204/4)
    LCDS.modeFlag = 2;  // OAM
    else if (this_frame < 284/4)
    LCDS.modeFlag = 3;  // VRA
    else if (this_frame < 456/4)
    LCDS.modeFlag = 0;  // HBlank
    
    // Done all lines
    if (LCD.line >= 144)
    LCDS.modeFlag = 1;  // VBlank
    
    if (LCD.line != prevLine && LCD.line < 144) {
        renderLine(LCD.line);
    }
    
    if (LCDS.lyInterrupt && LCD.line == LCD.lyCompare) {
        interrupt.flags |= LCDSTAT;
    }
    
    if (prevLine == 143 && LCD.line == 144) {
        // draw the entire frame
        interrupt.flags |= VBLANK;
        wnd_draw(NULL);
        if(wnd_updateEvent()) end = 1;
    }
    
    if (end) return 0;
    
    prevLine = LCD.line;
    
    return 1;
}


/*
void showLogo(unsigned char* rom, unsigned int* buf);

int lcdCycle1(int timeStart)
{
    int end = 0;
    
    unsigned int *buf = sdlPixels();
    //设置SDL背景
    for (int i = 0; i < 160*144; ++i) {
        buf[i] = ((0x01<<24) | (0xFF<<16) | (0x00<<8) | 0xFF);//ARGB
    }
    //显示任天堂logo位图
    showLogo(rom.romBytes, buf);
    
    sdlUpdateFrame();
    if(sdlUpdateEvent()) end = 1;
    //
    float deltaT = (float)1000 / (59.7) - (float)(SDL_GetTicks() - timeStart);
    if (deltaT > 0) SDL_Delay(deltaT);
    
    return end?0:1;
}

void showLogo(unsigned char* rom, unsigned int* buf)
{
    int length = 0x30*8*sizeof(char);
    unsigned char* logo = malloc(length);
    memset(logo, 0, length);
    //0104-0133 - Nintendo Logo
    for (int i = 0; i < 0x30; ++i) {
        char byte = rom[0x0104+i];
        for (int j = 0; j < 8; ++j) {
            logo[i*8+(7-j)] = (byte & (1 << j))? 0x50 : 0xFF;//注意像素位置跟位的位置是相反的，高位在低位置像素上。
        }
    }
    
    //设置SDL像素
    int x = 0, y = 0;
    for (int i = 0; i < 24; ++i) {
        x = (i * 4) % 48;
        y = ((i * 4) / 48) * 4;
        for (int j = 0; j < 16; ++j) {
            unsigned char byte = logo[i*16+j];
            buf[x+(j%4)+(y+(j/4))*160] = ((0x01<<24) | (byte<<16) | (byte<<8) | byte);//ARGB
        }
    }
    free(logo);
}
*/
