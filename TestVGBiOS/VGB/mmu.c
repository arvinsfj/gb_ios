//
//  mmu.c
//  TestVGB
//
//  Created by vin on 2018/8/27.
//  Copyright © 2018年 vin. All rights reserved.
//

/*
0000-3FFF   16KB ROM Bank 00     (in cartridge, fixed at bank 00)
4000-7FFF   16KB ROM Bank 01..NN (in cartridge, switchable bank number)
8000-9FFF   8KB Video RAM (VRAM) (switchable bank 0-1 in CGB Mode)
A000-BFFF   8KB External RAM     (in cartridge, switchable bank, if any)

C000-CFFF   4KB Work RAM Bank 0 (WRAM)
D000-DFFF   4KB Work RAM Bank 1 (WRAM)  (switchable bank 1-7 in CGB Mode)
E000-FDFF   Same as C000-DDFF (ECHO)    (typically not used)

FE00-FE9F   Sprite Attribute Table (OAM)
FEA0-FEFF   Not Usable
FF00-FF7F   I/O Ports
FF80-FFFE   High RAM (HRAM)
FFFF        Interrupt Enable Register

jump vectors：
0000,0008,0010,0018,0020,0028,0030,0038   for RST commands
0040,0048,0050,0058,0060                  for Interrupts（VBL,LCD,Timer,Serial,Joypad）
*/

#include "mmu.h"

#include "lcd.h"
#include "rom.h"
#include "interrupt.h"
#include "timer.h"

unsigned char cart[0x8000];   // ROM (Cart 1 & 2)
unsigned char vram[0x2000];  // video RAM
unsigned char sram[0x2000];  // switchable RAM
unsigned char wram[0x2000];  // working RAM
unsigned char oam[0x100];     // Sprite Attribute Memory
unsigned char io[0x100];     // Input/Output - Not sure if I need 0x100, 0x40 may suffice
unsigned char hram[0x80];    // High RAM

char jb = 0, jd = 0;

void memInit(void)
{
    memset(sram, 0, sizeof(sram));
    memset(io, 0, sizeof(io));
    memset(vram, 0, sizeof(vram));
    memset(oam, 0, sizeof(oam));
    memset(wram, 0, sizeof(wram));
    memset(hram, 0, sizeof(hram));
    //
    write8(0xFF10, 0x80);
    write8(0xFF11, 0xBF);
    write8(0xFF12, 0xF3);
    write8(0xFF14, 0xBF);
    write8(0xFF16, 0x3F);
    write8(0xFF19, 0xBF);
    write8(0xFF1A, 0x7F);
    write8(0xFF1B, 0xFF);
    write8(0xFF1C, 0x9F);
    write8(0xFF1E, 0xBF);
    write8(0xFF20, 0xFF);
    write8(0xFF23, 0xBF);
    write8(0xFF24, 0x77);
    write8(0xFF25, 0xF3);
    write8(0xFF26, 0xF1);
    write8(0xFF40, 0x91);
    write8(0xFF47, 0xFC);
    write8(0xFF48, 0xFF);
    write8(0xFF49, 0xFF);
}

unsigned int getButton(void);
unsigned int getDirection(void);

unsigned char read8(unsigned short address)
{
    if (0x0000 <= address && address <= 0x7FFF)
        return cart[address];
    else if (0x8000 <= address && address <= 0x9FFF)
        return vram[address - 0x8000];
    else if (0xA000 <= address && address <= 0xBFFF)
        return sram[address - 0xA000];
    else if (0xC000 <= address && address <= 0xDFFF)
        return wram[address - 0xC000];
    else if (0xE000 <= address && address <= 0xFDFF) //echo of wram
        return wram[address - 0xE000];
    else if (0xFE00 <= address && address <= 0xFEFF)
        return oam[address - 0xFE00];
    else if (address == 0xFF00) {
        unsigned char mask = 0;
        if (!jb) mask = getButton();
        if (!jd) mask = getDirection();
        return (0xC0 | (0xF ^ mask) | ((jb) | (jd)));
    }
    else if (address == 0xFF04)
        return getDiv();
    else if (address == 0xFF05)
        return getTima();
    else if (address == 0xFF06)
        return getTma();
    else if (address == 0xFF04)
        return getTac();
    else if (address == 0xFF0F)
        return interrupt.flags;
    else if (address == 0xFF40)
        return getLCDC();
    else if (address == 0xFF41)
        return getLCDS();
    else if (address == 0xFF42)
        return getScrollY();
    else if (address == 0xFF43)
        return getScrollX();
    else if (address == 0xFF44)
        return getLine();
    else if(0xFF00 <= address && address <= 0xFF7F) // maybe only up to 0xFF4F
        return io[address - 0xFF00];
    else if (0xFF80 <= address && address <= 0xFFFE)
        return hram[address - 0xFF80];
    else if (address == 0xFFFF)
        return interrupt.enable;
    
    return 0;
}

unsigned short read16(unsigned short address)
{
    return (read8(address) | (read8(address+1) << 8));
}

void write8(unsigned short address, unsigned char value)
{
    // can't write to ROM
    if (0x8000 <= address && address <= 0x9FFF)
        vram[address - 0x8000] = value;
    else if (0xA000 <= address && address <= 0xBFFF)
        sram[address - 0xA000] = value;
    else if (0xC000 <= address && address <= 0xDFFF)
        wram[address - 0xC000] = value;
    else if (0xE000 <= address && address <= 0xFDFF)
        wram[address - 0xE000] = value;
    else if (0xFE00 <= address && address <= 0xFEFF)
        oam[address - 0xFE00] = value;
    else if (address == 0xFF04)
        setDiv(value);
    else if (address == 0xFF05)
        setTima(value);
    else if (address == 0xFF06)
        setTma(value);
    else if (address == 0xFF04)
        setTac(value);
    else if (address == 0xFF40)
        setLCDC(value);
    else if (address == 0xFF41)
        setLCDS(value);
    else if (address == 0xFF42)
        setScrollY(value);
    else if (address == 0xFF43)
        setScrollX(value);
    else if (address == 0xFF45)
        setLyCompare(value);
    else if(address == 0xff46){
        for(int i = 0; i < 160; i++) write8(0xfe00 + i, read8((value << 8) + i));
    }
    else if (address == 0xFF47)
        setBGPalette(value);
    else if (address == 0xFF48)
        setSpritePalette1(value);
    else if (address == 0xFF49)
        setSpritePalette2(value);
    else if (address == 0xFF4A)
        setWindowY(value);
    else if (address == 0xFF4B)
        setWindowX(value);
    else if (address == 0xFF00) {
        jb = value & 0x20; jd = value & 0x10; 
    }
    else if(0xFF00 <= address && address <= 0xFF7F)
        io[address - 0xFF00] = value;
    else if (0xFF80 <= address && address <= 0xFFFE)
        hram[address - 0xFF80] = value;
    else if (address == 0xFF0F)
        interrupt.flags = value;
    else if (address == 0xFFFF)
        interrupt.enable = value;
}

void write16(unsigned short address, unsigned short value)
{
    write8(address,(value & 0x00FF));
    write8(address+1,(value & 0xFF00) >> 8);
}
