#ifndef BSW_SERVICE_LEDH
#define BSW_SERVICE_LEDH

#include "ledport.h"

typedef enum {
    LED_RIGHT,
    LED_LEFT,
    LED_REAR,
    LED_BOTH,
    LED_NONE
} LedPosition;

void ledInit(void);                            // 전체 초기화
void ledStartBlinking(LedPosition pos);        // 지정 LED 깜빡이기 시작
void ledStopAll(void);                         // 모든 LED OFF
void ledUpdateBlinking(void);                  // 주기 호출

#endif /* BSW_SERVICE_LEDH */
