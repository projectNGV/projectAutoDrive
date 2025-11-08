#include "buzzerport.h"

void buzzerInit(void)
{
    // P10.4를 푸시풀 출력으로 설정 (0x10 = output push-pull)
    MODULE_P10.IOCR4.B.PC4 = 0x10;

    // 기본값: OFF
    MODULE_P10.OUT.B.P4 = 0;
}

void buzzerOn(void)
{
    MODULE_P10.OUT.B.P4 = 1;
}

void buzzerOff(void)
{
    MODULE_P10.OUT.B.P4 = 0;
}

void buzzerToggle(void)
{
    MODULE_P10.OUT.B.P4 ^= 1;
}
