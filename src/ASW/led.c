#include "led.h"

/*********************************************************************************************************************/

// 현재 토글(깜빡임) 대상이 되는 LED의 위치를 저장하는 전역 변수
// LED_NONE일 경우, 어떤 LED도 깜빡이지 않음
static LedPosition activeLed = LED_NONE;

/*********************************************************************************************************************/

// LED 초기화 함수: 각 LED 핀을 출력 모드로 설정하고 OFF 상태로 초기화
void ledInit(void)
{
    ledPortInit();          // 하드웨어 포트 설정 (GPIO 출력 모드 등)
    activeLed = LED_NONE;   // 초기에는 어떤 LED도 깜빡이지 않도록 설정
}

// 깜빡일 LED를 지정하는 함수
// 지정된 LED는 ledUpdateBlinking() 호출 시마다 상태가 토글됨
void ledStartBlinking(LedPosition pos)
{
    activeLed = pos;  // 지정된 위치의 LED를 깜빡이 대상으로 설정
}

// 모든 LED의 깜빡임을 중단하고 OFF 상태로 설정
void ledStopAll(void)
{
    activeLed = LED_NONE;   // 깜빡임 대상 해제
    ledSetRight(0);         // 오른쪽 LED OFF
    ledSetLeft(0);          // 왼쪽 LED OFF
    ledSetRear(0);          // 후진등 OFF
}

// ledUpdateBlinking() 주기적으로 호출되어 지정된 LED를 토글(ON↔OFF)하여 깜빡임 효과를 만듦
void ledUpdateBlinking(void)
{
    switch (activeLed)
    {
        case LED_RIGHT :
            ledToggleRight();  // 우측 LED 상태 반전
            break;

        case LED_LEFT :
            ledToggleLeft();   // 좌측 LED 상태 반전
            break;

        case LED_REAR :
            ledToggleRear();   // 후진 LED 상태 반전
            break;

        case LED_BOTH :
            ledToggleRight();  // 양쪽 동시에 깜빡임
            ledToggleLeft();
            break;

        case LED_NONE :
        default :
            ledStopAll();      // 깜빡임 대상 없으면 모든 LED OFF
            break;
    }
}
