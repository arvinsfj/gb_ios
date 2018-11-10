//
//  interrupt.c
//  TestVGB
//
//  Created by vin on 2018/8/27.
//  Copyright © 2018年 vin. All rights reserved.
//

#include "interrupt.h"
#include "cpu.h"

struct interrupt interrupt;

void interruptCycle()
{
    if (interrupt.pending == 1) {
        interrupt.pending -= 1;
        return;
    }
    // if everything is enabled and there is a flag set
    if (interrupt.master && interrupt.enable && interrupt.flags) {
        // get which interrupt is currently being executed
        unsigned char inter = interrupt.enable & interrupt.flags;
        
        if (inter & VBLANK) {
            interrupt.flags &= ~VBLANK; // turn off the flag
            cpuInterrupt(0x40);
        }
        
        if (inter & LCDSTAT) {
            interrupt.flags &= ~LCDSTAT;
            cpuInterrupt(0x48);
        }
        
        if (inter & TIMER) {
            interrupt.flags &= ~TIMER;
            cpuInterrupt(0x50);
        }
        
        if (inter & SERIAL) {
            interrupt.flags &= ~SERIAL;
            cpuInterrupt(0x58);
        }
        
        if (inter & JOYPAD) {
            interrupt.flags &= ~JOYPAD;
            cpuInterrupt(0x60);
        }
    }
}
