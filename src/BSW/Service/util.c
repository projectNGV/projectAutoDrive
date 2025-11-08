#include "util.h"

void delayMs(int msec){
    uint32 target = MODULE_STM0.TIM0.U;
    target += msec * 100000;
    while(MODULE_STM0.TIM0.U < target);
    return;
}


void delayUs(int usec){
    uint32 target = MODULE_STM0.TIM0.U;
    target += usec * 100;
    while(MODULE_STM0.TIM0.U < target);
    return;
}

uint64 getTime10Ns(void){
    uint64 result;
    result = MODULE_STM0.TIM0.U;
    result |= ((uint64) MODULE_STM0.CAP.U) << 32;
    return result;
}
