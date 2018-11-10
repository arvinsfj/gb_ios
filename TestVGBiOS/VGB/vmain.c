//
//  main.c
//  TestVGB
//
//  Created by vin on 2018/8/24.
//  Copyright © 2018年 vin. All rights reserved.
//

/*
Game Boy Technical Data

CPU          - 8-bit (Similar to the Z80 processor)
Clock Speed  - 4.194304MHz (4.295454MHz for SGB, max. 8.4MHz for CGB)
Work RAM     - 8K Byte (32K Byte for CGB)
Video RAM    - 8K Byte (16K Byte for CGB)
Screen Size  - 2.6"
Resolution   - 160x144 (20x18 tiles)
Max sprites  - Max 40 per screen, 10 per line
Sprite sizes - 8x8 or 8x16
Palettes     - 1x4 BG, 2x3 OBJ (for CGB: 8x4 BG, 8x3 OBJ)
Colors       - 4 grayshades (32768 colors for CGB)
Horiz Sync   - 9198 KHz (9420 KHz for SGB)
Vert Sync    - 59.73 Hz (61.17 Hz for SGB)
Sound        - 4 channels with stereo sound
Power        - DC6V 0.7W (DC3V 0.7W for GB Pocket, DC3V 0.6W for CGB)
*/

#include <stdio.h>

#include "lcd.h"
#include "rom.h"
#include "interrupt.h"
#include "timer.h"
#include "cpu.h"

int wnd_init(const char *filename);

int vmain(int argc, const char* argv)
{
    
    // 组件初始化
    romInit(argv);
    cpuInit();
    wnd_init("");
    
    while (1) {
        // 组件执行循环
        cpuCycle();
        interruptCycle();
        timerCycle();
        if (!lcdCycle()) break;
    }
    
    // 组件退出清理
    
    return 0;
}
