#include "lkas.h"

#include "can.h"
#include "Ifx_Types.h"s
#include "motor.h"
#include "control.h"

#define FORWARD_SPEED 400 // LKAS 하면서 앞으로 가는 속도
#define CONTROL_SPEED 0 // LKAS 하면서 앞으로 가는 속도
#define BUFFER_SIZE 3

#define STEER_FACTOR 2 // Steer 값 조절에 혹시 필요하면 쓰세요
#define STEER_OFFSET -200 // CAN에서 오는 건 0~400 우리가 원하는 건 -200 ~ 200

#define LKAS_START_CAN_ID 0x210

boolean g_lkasEnable;

int steerBuffer[BUFFER_SIZE];
int bufferLeft, bufferRight;

void LKAS_UpdateSteerBuffer (int value)
{
    if (g_lkasEnable)
    {
        steerBuffer[bufferRight] = value;
        bufferRight = (bufferRight + 1) % BUFFER_SIZE;
    }
}

void LKAS_Start ()
{
    myPrintf("LKAS START\n");
    bufferLeft = 0;
    bufferRight = 0;
    g_lkasEnable = TRUE;
    unsigned char txData[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    canSendMsg(LKAS_START_CAN_ID, txData, 1);
    motorMoveForward(FORWARD_SPEED);

}

void LKAS_Stop ()
{
    myPrintf("LKAS STOP\n");
    g_lkasEnable = FALSE;
    unsigned char txData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    canSendMsg(LKAS_START_CAN_ID, txData, 1);
    motorStop();
}

void LKAS_Main ()
{
    if (g_lkasEnable)
    {
        if (bufferLeft != bufferRight)
        {
//            myPrintf("offset = %d\n", steerBuffer[bufferLeft]);
            int mv = (steerBuffer[bufferLeft] * STEER_FACTOR) + STEER_OFFSET;
            bufferLeft = (bufferLeft + 1) % BUFFER_SIZE;
            // -값: 우회전, +값: 좌회전
            if (mv < 20 && mv > -20)
                mv = 0;
            mv = -(mv);
            myPrintf("mv = %d\n", mv);


            if (mv > 0)
            {
                motorMovChAPwm((FORWARD_SPEED + mv), Forward);
                motorMovChBPwm(CONTROL_SPEED, Forward);
                delayMs(300);
                motorMovChAPwm(0, Forward);
                motorMovChBPwm(0, Forward);
                delayMs(100);
                motorMovChAPwm(FORWARD_SPEED, Forward);
                motorMovChBPwm(FORWARD_SPEED, Forward);
                delayMs(300);
                motorMovChAPwm(0, Forward);
                motorMovChBPwm(0, Forward);
                delayMs(200);
            }
            else if (mv < 0)
            {
                mv = -mv;
                motorMovChAPwm(CONTROL_SPEED, Forward);
                motorMovChBPwm((FORWARD_SPEED + mv), Forward);
                delayMs(300);
                motorMovChAPwm(0, Forward);
                motorMovChBPwm(0, Forward);
                delayMs(100);
                motorMovChAPwm(FORWARD_SPEED, Forward);
                motorMovChBPwm(FORWARD_SPEED, Forward);
                delayMs(300);
                motorMovChAPwm(0, Forward);
                motorMovChBPwm(0, Forward);
                delayMs(200);
            }
            else
            {
                motorMovChAPwm(FORWARD_SPEED, Forward);
                motorMovChBPwm(FORWARD_SPEED, Forward);
                delayMs(300);
                motorMovChAPwm(0, Forward);
                motorMovChBPwm(0, Forward);
                delayMs(200);
            }
        }
    }
}
