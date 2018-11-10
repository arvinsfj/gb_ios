//
//  rom.h
//  TestVGB
//
//  Created by vin on 2018/8/24.
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
 0040,0048,0050,0058,0060                  for Interrupts
 
 =============================
 
 The areas from 0000-7FFF and A000-BFFF may be used to connect external hardware.
 
 The first area is typically used to address ROM (read only, of course), cartridges with Memory Bank Controllers (MBCs) are additionally using this area to output data (write only) to the MBC chip.
 
 The second area is often used to address external RAM, or to address other external hardware (Real Time Clock, etc). External memory is often battery buffered, and may hold saved game positions and high scrore tables (etc.) even when the gameboy is turned of, or when the cartridge is removed.
 
 ==============================
 
 The memory at 0100-014F contains the cartridge header.
 
 An internal information area is located at 0100-014F in each cartridge.
 It contains the following values:
 
 0100-0103 - Entry Point
 
 After displaying the Nintendo Logo, the built-in boot procedure jumps to this address (100h), which should then jump to the actual main program in the cartridge. Usually this 4 byte area contains a NOP instruction, followed by a JP 0150h instruction. But not always.
 
 0104-0133 - Nintendo Logo
 
 These bytes define the bitmap of the Nintendo logo that is displayed when the gameboy gets turned on.
 The gameboys boot procedure verifies the content of this bitmap (after it has displayed it), and LOCKS ITSELF UP if these bytes are incorrect. A CGB verifies only the first 18h bytes of the bitmap, but others (for example a pocket gameboy) verify all 30h bytes.
 
 0134-0143 - Title

 Title of the game in UPPER CASE ASCII. If it is less than 16 characters then the remaining bytes are filled with 00's. When inventing the CGB, Nintendo has reduced the length of this area to 15 characters, and some months later they had the fantastic idea to reduce it to 11 characters only. The new meaning of the ex-title bytes is described below.

 013F-0142 - Manufacturer Code
 
 In older cartridges this area has been part of the Title (see above), in newer cartridges this area contains an 4 character uppercase manufacturer code. Purpose and Deeper Meaning unknown.
 
 0143 - CGB Flag
 
 In older cartridges this byte has been part of the Title (see above). In CGB cartridges the upper bit is used to enable CGB functions. This is required, otherwise the CGB switches itself into Non-CGB-Mode. Typical values are:
 80h - Game supports CGB functions, but works on old gameboys also.
 C0h - Game works on CGB only (physically the same as 80h).
 
 0144-0145 - New Licensee Code
 
 Specifies a two character ASCII licensee code, indicating the company or publisher of the game. These two bytes are used in newer games only (games that have been released after the SGB has been invented). Older games are using the header entry at 014B instead.
 
 0146 - SGB Flag
 
 Specifies whether the game supports SGB functions, common values are:
 00h = No SGB functions (Normal Gameboy or CGB only game)
 03h = Game supports SGB functions
 The SGB disables its SGB functions if this byte is set to another value than 03h.
 
 0147 - Cartridge Type
 
 Specifies which Memory Bank Controller (if any) is used in the cartridge, and if further external hardware exists in the cartridge.
 
 
 00h  ROM ONLY                 13h  MBC3+RAM+BATTERY
 01h  MBC1                     15h  MBC4
 02h  MBC1+RAM                 16h  MBC4+RAM
 03h  MBC1+RAM+BATTERY         17h  MBC4+RAM+BATTERY
 05h  MBC2                     19h  MBC5
 06h  MBC2+BATTERY             1Ah  MBC5+RAM
 08h  ROM+RAM                  1Bh  MBC5+RAM+BATTERY
 09h  ROM+RAM+BATTERY          1Ch  MBC5+RUMBLE
 0Bh  MMM01                    1Dh  MBC5+RUMBLE+RAM
 0Ch  MMM01+RAM                1Eh  MBC5+RUMBLE+RAM+BATTERY
 0Dh  MMM01+RAM+BATTERY        FCh  POCKET CAMERA
 0Fh  MBC3+TIMER+BATTERY       FDh  BANDAI TAMA5
 10h  MBC3+TIMER+RAM+BATTERY   FEh  HuC3
 11h  MBC3                     FFh  HuC1+RAM+BATTERY
 12h  MBC3+RAM
 
 0148 - ROM Size

 Specifies the ROM Size of the cartridge. Typically calculated as "32KB shl N".
 00h -  32KByte (no ROM banking)
 01h -  64KByte (4 banks)
 02h - 128KByte (8 banks)
 03h - 256KByte (16 banks)
 04h - 512KByte (32 banks)
 05h -   1MByte (64 banks)  - only 63 banks used by MBC1
 06h -   2MByte (128 banks) - only 125 banks used by MBC1
 07h -   4MByte (256 banks)
 52h - 1.1MByte (72 banks)
 53h - 1.2MByte (80 banks)
 54h - 1.5MByte (96 banks)
 
 0149 - RAM Size
 
 Specifies the size of the external RAM in the cartridge (if any).
 
 00h - None
 01h - 2 KBytes
 02h - 8 Kbytes
 03h - 32 KBytes (4 banks of 8KBytes each)
 
 When using a MBC2 chip 00h must be specified in this entry, even though the MBC2 includes a built-in RAM of 512 x 4 bits.
 
 014A - Destination Code

 Specifies if this version of the game is supposed to be sold in japan, or anywhere else. Only two values are defined.
 00h - Japanese
 01h - Non-Japanese
 
 014B - Old Licensee Code
 
 Specifies the games company/publisher code in range 00-FFh. A value of 33h signalizes that the New License Code in header bytes 0144-0145 is used instead.
 (Super GameBoy functions won't work if <> $33.)
 
 014C - Mask ROM Version number
 
 Specifies the version number of the game. That is usually 00h.
 
 014D - Header Checksum
 
 Contains an 8 bit checksum across the cartridge header bytes 0134-014C. The checksum is calculated as follows:
 
 x=0:FOR i=0134h TO 014Ch:x=x-MEM[i]-1:NEXT
 
 The lower 8 bits of the result must be the same than the value in this entry. The GAME WON'T WORK if this checksum is incorrect.
 
 014E-014F - Global Checksum
 
 Contains a 16 bit checksum (upper byte first) across the whole cartridge ROM. Produced by adding all bytes of the cartridge (except for the two checksum bytes). The Gameboy doesn't verify this checksum.

 ==============================
 
 MBC
 
 As the gameboys 16 bit address bus offers only limited space for ROM and RAM addressing, many games are using Memory Bank Controllers (MBCs) to expand the available address space by bank switching. These MBC chips are located in the game cartridge (ie. not in the gameboy itself)
 
 None (32KByte ROM only)

 Small games of not more than 32KBytes ROM do not require a MBC chip for ROM banking. The ROM is directly mapped to memory at 0000-7FFFh. Optionally up to 8KByte of RAM could be connected at A000-BFFF, even though that could require a tiny MBC-like circuit, but no real MBC chip.
 
 ================================
 
 Power Up Sequence

 When the GameBoy is powered up, a 256 byte program starting at memory location 0 is executed.
 
 This program is located in a ROM inside the GameBoy. The first thing the program does is read the cartridge locations from $104 to $133 and place this graphic of a Nintendo logo on the screen at the top. This image is then scrolled until it is in the middle of the screen. Two musical notes are then played on the internal speaker.
 
 Again, the cartridge locations $104 to $133 are read but this time they are compared with a table in the internal rom. If any byte fails to compare, then the GameBoy stops comparing bytes and simply halts all operations. If all locations compare the same, then the GameBoy starts adding all of the bytes in the cartridge from $134 to $14d. A value of 25 decimal is added to this total. If the least significant byte of the result is a not a zero, then the GameBoy will stop doing anything. If it is a zero, then the internal ROM is disabled and cartridge program execution begins at location $100 with the following register values:
 
 AF=$01B0
 BC=$0013
 DE=$00D8
 HL=$014D
 Stack Pointer=$FFFE
 [$FF05] = $00   ; TIMA
 [$FF06] = $00   ; TMA
 [$FF07] = $00   ; TAC
 [$FF10] = $80   ; NR10
 [$FF11] = $BF   ; NR11
 [$FF12] = $F3   ; NR12
 [$FF14] = $BF   ; NR14
 [$FF16] = $3F   ; NR21
 [$FF17] = $00   ; NR22
 [$FF19] = $BF   ; NR24
 [$FF1A] = $7F   ; NR30
 [$FF1B] = $FF   ; NR31
 [$FF1C] = $9F   ; NR32
 [$FF1E] = $BF   ; NR33
 [$FF20] = $FF   ; NR41
 [$FF21] = $00   ; NR42
 [$FF22] = $00   ; NR43
 [$FF23] = $BF   ; NR30
 [$FF24] = $77   ; NR50
 [$FF25] = $F3   ; NR51
 [$FF26] = $F1-GB, $F0-SGB ; NR52
 [$FF40] = $91   ; LCDC
 [$FF42] = $00   ; SCY
 [$FF43] = $00   ; SCX
 [$FF45] = $00   ; LYC
 [$FF47] = $FC   ; BGP
 [$FF48] = $FF   ; OBP0
 [$FF49] = $FF   ; OBP1
 [$FF4A] = $00   ; WY
 [$FF4B] = $00   ; WX
 [$FFFF] = $00   ; IE
 
 It is not a good idea to assume the above values will always exist. A later version GameBoy could contain different values than these at reset. Always set these registers on reset rather than assume they are as above.
 
 Please note that GameBoy internal RAM on power up contains random data. All of the GameBoy emulators tend to set all RAM to value $00 on entry.
 
 Cart RAM the first time it is accessed on a real GameBoy contains random data. It will only contain known data if the GameBoy code initializes it to some value.

 ============================
 
 */

#ifndef rom_h
#define rom_h

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

struct rom {
    unsigned char *romBytes;
    char gameTitle[17];
    int romType; // use int for now - change to string
    int romSize;
    int ramSize;
};

extern struct rom rom;

void romInit(const char* filename);

#endif /* rom_h */
