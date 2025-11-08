#include "systeminit.h"


void systemInit(){
    bluetoothInit();
    motorInit();
    asclin0InitUart();
    tofInit();
    uartInit();
    ultrasonicInit();
    ledInit();
    gpt12Init();
    buzzerInit();
}
