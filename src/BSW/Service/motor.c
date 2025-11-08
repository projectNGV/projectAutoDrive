#include "motor.h"

void motorInit(void)
{
    MODULE_P02.IOCR4.B.PC7 = 0x10;  // A Break
    MODULE_P10.IOCR0.B.PC1 = 0x10;  // A DIR
    MODULE_P02.IOCR4.B.PC6 = 0x10;  // B Break
    MODULE_P10.IOCR0.B.PC2 = 0x10;  // B DIR

    gtmAtomPwmInit();

    gtmAtomPwmASetDutyCycle(0);
    gtmAtomPwmBSetDutyCycle(0);
}

void motorStopChA(void)
{
    MODULE_P02.OUT.B.P7 = 1;   /* 모터 Brake 신호 인가 (1: 정지, 0: PWM-A에 따라 동작) */
}


///* 1: 정방향, 0: 역방향 */
void motorMovChAPwm(int duty, int dir)
{
    gtmAtomPwmASetDutyCycle(duty);
    if(dir)
    {
        MODULE_P10.OUT.B.P1 = 1; /* 모터 회전 방향 (1: 앞, 0: 뒤) */
    }
    else {
        MODULE_P10.OUT.B.P1 = 0; /* 모터 회전 방향 (1: 앞, 0: 뒤) */
    }

    MODULE_P02.OUT.B.P7 = 0;   /* 모터 Brake 해제 (1: 정지, 0: PWM-A에 따라 동작) */
}

void motorStopChB(void)
{
    MODULE_P02.OUT.B.P6 = 1;   /* 모터 Brake 신호 인가 (1: 정지, 0: PWM-A에 따라 동작) */
}


///* 1: 정방향, 0: 역방향 */
void motorMovChBPwm(int duty, int dir)
{
    gtmAtomPwmBSetDutyCycle(duty);

    if(dir)
    {
        MODULE_P10.OUT.B.P2 = 1; /* 모터 회전 방향 (1: 앞, 0: 뒤) */
    }
    else {
        MODULE_P10.OUT.B.P2 = 0; /* 모터 회전 방향 (1: 앞, 0: 뒤) */
    }

    MODULE_P02.OUT.B.P6 = 0;   /* 모터 Brake 해제 (1: 정지, 0: PWM-A에 따라 동작) */
}

void motorSoftBraking(int duty)
{
    while (duty > 0)
    {
        duty = (duty >= 100) ? duty - 100 : 0;
        gtmAtomPwmASetDutyCycle(duty);
        gtmAtomPwmBSetDutyCycle(duty);
    }

    motorStop();  // 마지막에 완전 정지
}

void motorHardBraking(int duty)
{
    // 모터 회전 방향: 역회전
    MODULE_P10.OUT.B.P1 = 0;
    MODULE_P10.OUT.B.P2 = 0;

    gtmAtomPwmASetDutyCycle(duty);
    gtmAtomPwmBSetDutyCycle(duty);

    delayMs(200);

    motorStop();  // 마지막에 완전 정지
}

void motorMoveForward(int duty)
{
    MODULE_P10.OUT.B.P1 = 1;
    MODULE_P10.OUT.B.P2 = 1;

    MODULE_P02.OUT.B.P7 = 0;
    MODULE_P02.OUT.B.P6 = 0;

    gtmAtomPwmASetDutyCycle(duty);
    gtmAtomPwmBSetDutyCycle(duty);
}

void motorMoveReverse (int duty)
{
    MODULE_P10.OUT.B.P1 = 0;
    MODULE_P10.OUT.B.P2 = 0;


    MODULE_P02.OUT.B.P7 = 0;
    MODULE_P02.OUT.B.P6 = 0;


    gtmAtomPwmASetDutyCycle(duty);
    gtmAtomPwmBSetDutyCycle(duty);
}

void motorStop(void){
    MODULE_P02.OUT.B.P7 = 1;
    MODULE_P02.OUT.B.P6 = 1;

    gtmAtomPwmASetDutyCycle(0);
    gtmAtomPwmBSetDutyCycle(0);
}

