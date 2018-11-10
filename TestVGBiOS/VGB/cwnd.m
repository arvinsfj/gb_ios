//
//  cwnd.c
//  TestNes
//
//  Created by arvin on 2017/8/16.
//  Copyright © 2017年 com.fuwo. All rights reserved.
//

#import <UIKit/UIKit.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "hqx.h"

uint32_t   RGBtoYUV[16777216];
uint32_t   YUV1, YUV2;

HQX_API void HQX_CALLCONV hqxInit(void)
{
    /* Initalize RGB to YUV lookup table */
    uint32_t c, r, g, b, y, u, v;
    for (c = 0; c < 16777215; c++) {
        r = (c & 0xFF0000) >> 16;
        g = (c & 0x00FF00) >> 8;
        b = c & 0x0000FF;
        y = (uint32_t)(0.299*r + 0.587*g + 0.114*b);
        u = (uint32_t)(-0.169*r - 0.331*g + 0.5*b) + 128;
        v = (uint32_t)(0.5*r - 0.419*g - 0.081*b) + 128;
        RGBtoYUV[c] = (y << 16) + (u << 8) + v;
    }
}

#define    WIDTH        160
#define    HEIGHT       144
#define    MAG          1   //magnification;

static uint8_t pic_mem_orgl[WIDTH * HEIGHT * 4];
static uint8_t pic_mem_frnt[WIDTH * HEIGHT * 4 * MAG * MAG];
static uint32_t frame_counter;
static double time_frame0;
static uint8_t ctrl0[2] = {0, 0};

unsigned int* getPixels(void)
{
    return (unsigned int*)pic_mem_orgl;
}

int wnd_init(const char *filename)
{
    hqxInit();
    
    frame_counter = 0;
    time_frame0 =  CFAbsoluteTimeGetCurrent();
    
    return 0;
}

void byte2image(uint8_t *bytes, uint64_t width, uint64_t height)
{
    CGColorSpaceRef colorRef = CGColorSpaceCreateDeviceRGB();
    CGContextRef ctxRef = CGBitmapContextCreate(bytes, width, height, 8, width*4, colorRef, kCGImageAlphaPremultipliedLast);//RGBA
    CGImageRef imgRef = CGBitmapContextCreateImage(ctxRef);
    UIImage* image = [UIImage imageWithCGImage:imgRef];
    if (image) {
        [[NSNotificationCenter defaultCenter] postNotificationName:@"gb_video" object:nil userInfo:@{@"video":image}];
    }
    CGColorSpaceRelease(colorRef);
    CGImageRelease(imgRef);
    CGContextRelease(ctxRef);
}

void wnd_draw(uint8_t* pixels)
{
    if (MAG == 1) {
        memcpy(pic_mem_frnt, pic_mem_orgl, WIDTH*HEIGHT*4);
    }
    if (MAG == 2) {
        hq2x_32((uint32_t*)pic_mem_orgl, (uint32_t*)pic_mem_frnt, WIDTH, HEIGHT);
    }
    if (MAG == 3) {
        hq3x_32((uint32_t*)pic_mem_orgl, (uint32_t*)pic_mem_frnt, WIDTH, HEIGHT);
    }
    if (MAG == 4) {
        hq4x_32((uint32_t*)pic_mem_orgl, (uint32_t*)pic_mem_frnt, WIDTH, HEIGHT);
    }
    
    ++frame_counter;
    double delay = (frame_counter*0.016667+time_frame0) - CFAbsoluteTimeGetCurrent();
    if (delay > 0) {
        [NSThread sleepForTimeInterval:delay];//多余的时间还给系统
    }
    
    byte2image(pic_mem_frnt, WIDTH, HEIGHT);
}

void wnd_key2btn(int key, char isDown)
{
    uint8_t btn = 0;
    switch (key) {
        case 7:
            btn = 1 << 7;//START
            break;
        case 6:
            btn = 1 << 6;//SELECT
            break;
        case 5:
            btn = 1 << 5;//B
            break;
        case 4:
            btn = 1 << 4;//A
            break;
        case 3:
            btn = 1 << 3;//Down
            break;
        case 2:
            btn = 1 << 2;//UP
            break;
        case 1:
            btn = 1 << 1;//LEFT
            break;
        case 0:
            btn = 1 << 0;//RIGHT
            break;
    }
    if (isDown){
        ctrl0[0] |= btn;
        ctrl0[1] |= btn;
    }else if (!isDown){
        ctrl0[0] &= ~btn;
        ctrl0[1] &= ~btn;
    }
}

int wnd_updateEvent(void)
{
    return 0;
}

unsigned int getButton(void)
{
    char ctl = ctrl0[0];
    return (ctl&0xF0)>>4;
}

unsigned int getDirection(void)
{
    char ctl = ctrl0[0];
    return (ctl&0x0F)>>0;
}


