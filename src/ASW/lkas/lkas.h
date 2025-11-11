#ifndef __LKAS_H__
#define __LKAS_H__

#include "Ifx_Types.h"

extern boolean g_lkasEnable;
extern boolean g_accEnable;

void LKAS_UpdateSteerBuffer(int value);
void LKAS_Start(void);
void LKAS_Main(void);
void LKAS_Stop(void);
//static boolean LKAS_PopLatest(int* outVal);
void ACC_LKAS_Main(unsigned int distance);


#define ACC_TARGET_SPEED 400
#define ACC_DEACTIVATION_THRESHOLD 500 // 50cm 밖이면 ACC 끄고 고정 속도 주행
#define ACC_ACTIVATION_THRESHOLD 300 // 30m 안으로 들어오면 ACC 속도 제어 시작
#define ACC_BRAKE_DUTY_STEP 5 // FSM 루프당 Duty 변화량
#define ACC_MIN_SPEED 300 // 최소 주행 속도 (정지 방지)

#endif

