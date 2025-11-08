#include "buzzer.h"

extern volatile uint16 g_beepInterval;

void buzzerParkingWarning (int rearDistance)
{
    if (rearDistance <= 150)
    {
        Gpt1_Interrupt_Disable();  // 인터럽트 끄고
        buzzerOn();                // 지속 울림
//        delayMs(2000);             // 2초 지속
//        buzzerOff();               // 울림 없음
    }
    else if (rearDistance <= 300)
    {
        Gpt1_Interrupt_Enable();
        g_beepInterval = 3000;     // 빠르게 삐삐삐삐
    }
    else if (rearDistance <= 500)
    {
        Gpt1_Interrupt_Enable();
        g_beepInterval = 8000;     // 느리게 삐–삐–삐
    }
    else
    {
        Gpt1_Interrupt_Disable();
        buzzerOff();               // 울림 없음
    }
}
