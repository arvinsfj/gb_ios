//
//  interrupt.h
//  TestVGB
//
//  Created by vin on 2018/8/27.
//  Copyright © 2018年 vin. All rights reserved.
//

#ifndef interrupt_h
#define interrupt_h

#include <stdio.h>

#define VBLANK    (1 << 0)
#define LCDSTAT   (1 << 1)
#define TIMER     (1 << 2)
#define SERIAL    (1 << 3)
#define JOYPAD    (1 << 4)

struct interrupt {
    unsigned char master;
    unsigned char enable;
    unsigned char flags;
    unsigned char pending;
};

extern struct interrupt interrupt;

void interruptCycle(void);

#endif /* interrupt_h */
