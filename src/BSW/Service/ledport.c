#include "ledport.h"

void ledSetRight(int on)
{
    MODULE_P13.OUT.B.P2 = on ? 1 : 0; // RIGHT (P13.2)
}

void ledSetLeft(int on)
{
    MODULE_P13.OUT.B.P1 = on ? 1 : 0; // LEFT (P13.1)
}

void ledSetRear(int on)
{
    MODULE_P02.OUT.B.P0 = on ? 1 : 0; // REAR (P2.0)
}

void ledPortInit(void)
{
    MODULE_P13.IOCR0.B.PC2 = 0x10;
    MODULE_P13.IOCR0.B.PC1 = 0x10;
    MODULE_P02.IOCR0.B.PC0 = 0x10;

    ledSetRight(0);
    ledSetLeft(0);
    ledSetRear(0);
}

void ledToggleRight(void)
{
    MODULE_P13.OUT.B.P2 ^= 1;
}

void ledToggleLeft(void)
{
    MODULE_P13.OUT.B.P1 ^= 1;
}

void ledToggleRear(void)
{
    MODULE_P02.OUT.B.P0 ^= 1;
}
