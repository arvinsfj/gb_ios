//
//  rom.c
//  TestVGB
//
//  Created by vin on 2018/8/24.
//  Copyright © 2018年 vin. All rights reserved.
//

/*
 MBC TYPE:
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
 
 =====================
 
 ROM SIZE:
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
 
 =====================
 
 External RAM SIZE:
 00h - None
 01h - 2 KBytes
 02h - 8 Kbytes
 03h - 32 KBytes (4 banks of 8KBytes each)
 */

#include "rom.h"
#include "mmu.h"

#define ROM_TITLE_OFFSET 0x134
#define ROM_TYPE_OFFSET 0x147
#define ROM_SIZE_OFFSET 0x148
#define ROM_RAM_OFFSET 0x149
#define HEADER_SIZE 0x14F

struct rom rom;

void checkRom(unsigned char* rom, long length);

void romInit(const char* filename)
{
    long len = 0, i = 0;
    FILE *file;
    unsigned char header[HEADER_SIZE];
    
    file = fopen(filename,"rb");
    fseek(file, 0, SEEK_END);
    len = ftell(file); 
    
    //读取整个Cartridge到romBytes中
    rom.romBytes = malloc(len);
    rewind(file);
    fread(rom.romBytes, len, 1, file);
    
    //读取Cartridge Header
    rewind(file);
    fread(header, 1, HEADER_SIZE, file);//Cartridge Header size: 0x14F
    
    rom.romType = header[ROM_TYPE_OFFSET];//Cartridge Type
    for (i = 0; i<16; i++) {
        rom.gameTitle[i] = header[ROM_TITLE_OFFSET+i];//Game Title
    }
    rom.romSize = header[ROM_SIZE_OFFSET];//ROM size
    rom.romSize = pow(2,rom.romSize+1) * 16;
    
    rom.ramSize = header[ROM_RAM_OFFSET];//External RAM size
    rom.ramSize = pow(4, rom.ramSize)/2;
    
    printf("Title: %s\n", rom.gameTitle);
    printf("MBC: %d\n", rom.romType);
    printf("ROM SIZE: %d KB\n", rom.romSize);
    printf("ERAM SIZE: %d KB\n\n", rom.ramSize);
    
    //检验rom
    checkRom(rom.romBytes, len);
    
    //由于MMC为00的情况，可以直接拷贝ROM内容到0000-7FFF区域
    memcpy(&cart[0x0000], &rom.romBytes[0x0000], 0x8000);
    
    fclose(file);
}

void checkRom(unsigned char* rom, long length)
{
    printf("CHRCK HEADER: \n=================\n");
    //0100-014F
    //0100-0103 - Entry Point
    printf("Entry Point: ");
    printf("0x%0.2X|0x%0.2X|0x%0.2X|0x%0.2X\n", rom[0x0100],rom[0x0101],rom[0x0102],rom[0x0103]);
    
    //0104-0133 - Nintendo Logo
    printf("Nintendo Logo: \n");
    for (int i = 0; i < 0x30; ++i) {
        if (i>0 && i%16==0) printf("\n");
        printf("0x%0.2X|", rom[0x0104+i]);
    }
    printf("\n");
    
    //0134-0143 - Title
    printf("Title: ");
    for (int i = 0; i < 16; ++i) {
        printf("%c", rom[0x0134+i]);
    }
    printf("\n");
    
    //013F-0142 - Manufacturer Code
    printf("Manufacturer Code: ");
    printf("%c%c%c%c\n", rom[0x013F],rom[0x0140],rom[0x0141],rom[0x0142]);
    
    //0143 - CGB Flag
    printf("CGB Flag: ");
    printf("0x%0.2X\n", rom[0x0143]);
    
    //0144-0145 - New Licensee Code
    printf("New Licensee Code: ");
    printf("%c%c\n", rom[0x0144],rom[0x0145]);
    
    //0146 - SGB Flag
    printf("SGB Flag: ");
    printf("0x%0.2X\n", rom[0x0146]);
    
    //0147 - Cartridge Type
    printf("Cartridge Type: ");
    printf("0x%0.2X\n", rom[0x0147]);
    
    //0148 - ROM Size
    printf("ROM Size: ");
    printf("0x%0.2X\n", rom[0x0148]);
    
    //0149 - RAM Size
    printf("ROM Size: ");
    printf("0x%0.2X\n", rom[0x0149]);
    
    //014A - Destination Code
    printf("Destination Code: ");
    printf("0x%0.2X\n", rom[0x014A]);
    
    //014B - Old Licensee Code
    printf("Old Licensee Code: ");
    printf("0x%0.2X\n", rom[0x014B]);
    
    //014C - Mask ROM Version number
    printf("Mask ROM Version number: ");
    printf("0x%0.2X\n", rom[0x014C]);
    
    //014D - Header Checksum
    printf("Header Checksum: ");
    printf("0x%0.2X - ", rom[0x014D]);
    //算法：x=0:FOR i=0134h TO 014Ch:x=x-MEM[i]-1:NEXT
    int sum = 0;
    for (int i = 0x0134; i <= 0x014C; ++i) {
        sum = sum - rom[i] - 1;
    }
    printf("0x%0.2X\n", sum);
    
    //014E-014F - Global Checksum
    printf("Global Checksum: ");
    printf("0x%0.2X|0x%0.2X - ", rom[0x014E],rom[0x014F]);
    //算法：除了014E-014F2个字节，其他所有字节相加
    sum = 0;
    for (int i = 0; i < length; ++i) {
        if (i != 0x014E && i != 0x014F) {
            sum += rom[i];
        }
    }
    printf("0x%0.2X\n", sum);
    
    printf("=================\n\n");
}
