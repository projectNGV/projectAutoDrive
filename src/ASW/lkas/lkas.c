#include "lkas.h"

#include "can.h"
#include "Ifx_Types.h"
#include "motor.h"
#include "control.h"

#define FORWARD_SPEED 400 // LKAS 하면서 앞으로 가는 속도
#define CONTROL_SPEED 0 // LKAS 하면서 앞으로 가는 속도
#define BUFFER_SIZE 3

#define STEER_FACTOR 2 // Steer 값 조절에 혹시 필요하면 쓰세요
#define STEER_OFFSET -200 // CAN에서 오는 건 0~400 우리가 원하는 건 -200 ~ 200

#define LKAS_START_CAN_ID 0x210
#define LKAS_START_CAN_ID 0x211


boolean g_lkasEnable;

int steerBuffer[BUFFER_SIZE];
int bufferLeft, bufferRight;


////***********************************////
extern MotorState motorState;
boolean g_accEnable = false;

////***********************************////

void LKAS_UpdateSteerBuffer (int value)
{
    if (g_lkasEnable)
    {
        steerBuffer[bufferRight] = value;
        bufferRight = (bufferRight + 1) % BUFFER_SIZE;
    }
}

void LKAS_Start (void)
{
    myPrintf("LKAS START\n");
    bufferLeft = 0;
    bufferRight = 0;
    g_lkasEnable = TRUE;
    unsigned char txData[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    canSendMsg(LKAS_START_CAN_ID, txData, 8);
    motorMoveForward(FORWARD_SPEED);

}

void LKAS_Stop (void)
{
    myPrintf("LKAS STOP\n");
    g_lkasEnable = FALSE;
    g_accEnable = false;
    unsigned char txData[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    canSendMsg(LKAS_START_CAN_ID, txData, 8);
    motorStop();
}

void LKAS_Main (void)
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


void ACC_LKAS_Main(unsigned int distance){ // ACC + LKAS
    int steer_mv = 0;
    int currentDuty = motorState.currentDuty;

    // ACC 속도 제어 로직 (Duty 조정)
    if (distance < ACC_ACTIVATION_THRESHOLD) {
        // 목표 거리보다 가까우면 감속
        currentDuty -= ACC_BRAKE_DUTY_STEP;
        if (currentDuty < ACC_MIN_SPEED)
            currentDuty = ACC_MIN_SPEED; // 최소 속도 유지

        // AEB 거리 (AEB_DISTANCE_MM)보다 가까워지면 FSM에서 처리됨
    }
    else if (currentDuty < ACC_TARGET_SPEED) {
        // 전방이 충분히 멀면 증속 (LKAS_Main에서 FORWARD_SPEED가 ACC_TARGET_SPEED와 같다고 가정)
        currentDuty += ACC_BRAKE_DUTY_STEP;
        if (currentDuty > ACC_TARGET_SPEED)
            currentDuty = ACC_TARGET_SPEED;
    }

    motorState.currentDuty = currentDuty; // 새로운 속도 저장

    // 2. LKAS 조향 제어 로직
    if (bufferLeft != bufferRight) {
        int latestIdx = bufferRight - 1;
        if (latestIdx < 0)
            latestIdx += BUFFER_SIZE;

        int latestRaw = steerBuffer[latestIdx];
        bufferLeft = bufferRight; // 버퍼 리셋

        steer_mv = (latestRaw * STEER_FACTOR) + STEER_OFFSET;
        if (steer_mv < 20 && steer_mv > -20)
            steer_mv = 0;
    }

    // 최종 모터 실행 (ACC 속도 + LKAS 조향 통합)
    // 조향 값(steer_mv)을 현재 속도(currentDuty)에 적용
    motorMovChAPwm(currentDuty - steer_mv, Forward); // 좌측 채널
    motorMovChBPwm(currentDuty + steer_mv, Forward); // 우측 채널
}




//#include "lkas.h"
//
//#include "can.h"
//#include "Ifx_Types.h"
//#include "motor.h"
//#include "control.h"
//
//#define FORWARD_SPEED 300 // LKAS 하면서 앞으로 가는 속도
//#define CONTROL_SPEED 0 // LKAS 하면서 앞으로 가는 속도
//#define BUFFER_SIZE 8
//
//#define STEER_FACTOR 2 // Steer 값 조절에 혹시 필요하면 쓰세요
//#define STEER_OFFSET -200 // CAN에서 오는 건 0~400 우리가 원하는 건 -200 ~ 200
//
//#define LKAS_START_CAN_ID 0x210
//
//boolean g_lkasEnable;
//
//int steerBuffer[BUFFER_SIZE];
//int bufferLeft, bufferRight;
//
//void LKAS_UpdateSteerBuffer (int value)
//{
//    if (g_lkasEnable)
//    {
//        steerBuffer[bufferRight] = value;
//        bufferRight = (bufferRight + 1) % BUFFER_SIZE;
//    }
//}
//
//static boolean LKAS_PopLatest(int* outVal)
//{
//    if (bufferLeft == bufferRight) return FALSE; // 비었음
//
//    // 마지막으로 기록된 인덱스(가장 최신)
//    int latestIdx = bufferRight - 1;
//    if (latestIdx < 0) latestIdx += BUFFER_SIZE;
//
//    *outVal = steerBuffer[latestIdx];
//
//    // 백로그는 모두 폐기 → 다음 읽기는 새로 들어올 값 기준
//    bufferLeft = bufferRight;
//    return TRUE;
//}
//
//
//void LKAS_Start ()
//{
//    myPrintf("LKAS START\n");
//    bufferLeft = 0;
//    bufferRight = 0;
//    g_lkasEnable = TRUE;
//    unsigned char txData[8] = {1, 0, 0, 0, 0, 0, 0, 0};
//    canSendMsg(LKAS_START_CAN_ID, txData, 1);
//    motorMoveForward(FORWARD_SPEED);
//
//}
//
//void LKAS_Stop ()
//{
//    myPrintf("LKAS STOP\n");
//    g_lkasEnable = FALSE;
//    unsigned char txData[8] = {0, 0, 0, 0, 0, 0, 0, 0};
//    canSendMsg(LKAS_START_CAN_ID, txData, 1);
//    motorStop();
//}
//
//void LKAS_Main ()
//{
//    if (g_lkasEnable)
//    {
//        int latestRaw;
//        if (LKAS_PopLatest(&latestRaw))   // ★ 변경: 최신 값만 사용
//        {
//            // myPrintf("offset = %d\n", latestRaw);
//            int mv = (latestRaw * STEER_FACTOR) + STEER_OFFSET;
//
//            // -값: 우회전, +값: 좌회전  (기존 주석/부호 정의 유지)
//            if (mv < 20 && mv > -20)
//                mv = 0;
//
//            myPrintf("mv = %d\n", mv);
//
//            if (mv > 0)
//            {
//                motorMovChAPwm((FORWARD_SPEED + mv), Forward);
//                motorMovChBPwm(CONTROL_SPEED, Forward);
//            }
//            else if (mv < 0)
//            {
//                int amp = -mv;
//                motorMovChAPwm(CONTROL_SPEED, Forward);
//                motorMovChBPwm((FORWARD_SPEED + amp), Forward);
//            }
//            else
//            {
//                motorMovChAPwm(FORWARD_SPEED, Forward);
//                motorMovChBPwm(FORWARD_SPEED, Forward);
//            }
//
//            // 기존 시퀀스 유지
//            delayMs(500);
//            motorMovChAPwm(0, Forward);
//            motorMovChBPwm(0, Forward);
//            delayMs(500);
//            motorMovChAPwm(FORWARD_SPEED, Forward);
//            motorMovChBPwm(FORWARD_SPEED, Forward);
//            delayMs(100);
//        }
//    }
//}
//
