#include "timer.h"
#include "interrupt.h"
#include "cpu.h"

struct timer timer;

void setDiv(unsigned char value)
{
    value = 0;
    timer.div = value; // setting div to anything makes it 0
}

unsigned int getDiv(void)
{
    return timer.div;
}

void setTima(unsigned char value)
{
    timer.tima = value;
}

unsigned int getTima(void)
{
    return timer.tima;
}

void setTma(unsigned char value)
{
    timer.tma = value;
}

unsigned int getTma(void)
{
    return timer.tma;
}

void setTac(unsigned char value)
{
    // revisit this
    int speeds[] = {1, 64, 16, 4};
    timer.tac = value;
    timer.started = value & 4;
    timer.speed = speeds[value & 3];
}

unsigned int getTac(void)
{
    return timer.tac;
}

void tick(void)
{
    timer.tick += 1;

    if (timer.tick == 16) {
        timer.div += 1;
        timer.tick = 0;
    }

    if (!timer.started)
        return;
    
    if (timer.tick == timer.speed) {
        timer.tima += 1;
        timer.tick = 0;
    }

    if (timer.tima == 0x100) {
        interrupt.flags |= TIMER;
        timer.tima = timer.tma;
    }
}

void timerCycle(void)
{
    static unsigned int time = 0;
    static unsigned int change = 0;
    unsigned int delta = getCycles() - time;
    time = getCycles();

    change += delta * 4;

    if (change >= 16) {
        tick();
        change -= 16;
    }
}


