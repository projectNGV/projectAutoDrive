#include "aeb.h"

/*********************************************************************************************************************/

// AEB (Autonomous Emergency Braking) 작동 여부를 나타내는 플래그
// 장애물이 일정 거리 이내에 감지되면 true로 설정되고, 긴급 제동이 수행됨
volatile bool aebFlag = false;

// TOF 센서 작동 여부를 나타내는 플래그 (외부에서 선언됨)
// true일 때만 거리 측정을 정상적으로 수행함
extern volatile bool tofFlag;

// 모터 상태를 담고 있는 구조체 변수 (현재 Duty, 방향 등 포함)
extern MotorState motorState;

/*********************************************************************************************************************/

// 긴급 경고 부저 함수
// 비상 상황 발생 시 부저를 켜고 끄는 패턴으로 경고음을 출력
void emergencyBuzzer (void)
{
    buzzerOn();        // 1차 삐—
    delayMs(100);
    buzzerOff();

    delayMs(150);      // 짧은 공백

    buzzerOn();        // 2차 삐—
    delayMs(100);
    buzzerOff();
}

// 긴급 제동 수행 함수
// AEB가 작동하면 현재 속도로 후진한 후 정지시킴
void performEmergencyStop(void)
{
    motorMoveReverse(motorState.currentDuty);  // 현재 듀티만큼 후진
    delayMs(REVERSE_TIME_MS);                  // 일정 시간 후진 유지

    motorState.currentDuty = 0;                // 속도를 0으로 초기화
    motorStop();                               // 차량 완전 정지
}

// AEB 플래그 업데이트 함수
// TOF 센서의 거리 값(g_TofValue)을 기반으로 aebFlag 값을 업데이트
// g_TofValue: 현재 측정된 전방 거리(mm)
void updateAebFlagByTof (unsigned int g_TofValue)
{
    // TOF 센서가 꺼진 상태면 AEB도 무조건 OFF
    if(tofFlag == false)
    {
        aebFlag = false;        // AEB 해제
        g_TofValue = 5000;      // 거리 값을 충분히 멀다고 가정 (초기화용)
    }
    else
    {
        // AEB가 아직 작동하지 않은 상태일 때
        if (!aebFlag)
        {
            // 일정 거리 이내라면 기본 듀티를 500으로 낮춤 (속도 제한)
            if (g_TofValue <= DUTY_LIMIT_DISTANCE_MM)
            {
                motorState.baseDuty = 500;
            }

            // 충돌 위험 거리 이내일 경우 AEB 작동 플래그 설정
            if (g_TofValue < AEB_DISTANCE_MM)
            {
                aebFlag = true;
            }
        }

        // AEB가 이미 작동 중일 때 (차량이 멈춰있는 상태)
        else
        {
            // 안전 거리 이상 확보된 경우 AEB 해제
            if (g_TofValue >= SAFETY_DISTANCE_MM)
            {
                aebFlag = false;
            }
        }
    }
}
