//
//  mmu.h
//  TestVGB
//
//  Created by vin on 2018/8/27.
//  Copyright © 2018年 vin. All rights reserved.
//

#ifndef mmu_h
#define mmu_h

#include <stdio.h>

extern unsigned char cart[];   // ROM (Cart 1 & 2)

void memInit(void);
unsigned char read8(unsigned short address);
unsigned short read16(unsigned short address);
void write8(unsigned short address, unsigned char value);
void write16(unsigned short address, unsigned short value);

#endif /* mmu_h */
