#include "port.h"

const GpioPin LED_PINS[LED_NUM] = {
        [LED_1] = {&MODULE_P00, 5},
        [LED_2] = {&MODULE_P00, 6}
};

const GpioPin BUZZER_PINS[BUZZER_NUM] = {
        [BUZZER_1] = {&MODULE_P00, 5},
        [BUZZER_2] = {&MODULE_P00, 6}
};

const GpioPin BUTTON_PINS[BUTTON_NUM] = {
        [BUTTON_1] = {&MODULE_P00, 7}
};

void portInit ()
{
    for (int i = 0; i < LED_NUM; i++)
    {
        IfxPort_setPinMode(LED_PINS[i].port, LED_PINS[i].pinIndex, IfxPort_Mode_outputPushPullGeneral);
    }

    for (int i = 0; i < BUZZER_NUM; i++)
    {
        IfxPort_setPinMode(BUZZER_PINS[i].port, BUZZER_PINS[i].pinIndex, IfxPort_Mode_outputPushPullGeneral);
    }

    for (int i = 0; i < BUTTON_NUM; i++){
        IfxPort_setPinMode(BUZZER_PINS[i].port, BUZZER_PINS[i].pinIndex, IfxPort_Mode_inputPullUp);
    }
}

void controlLed(LedPins led, int on){
    if(on) IfxPort_setPinState(LED_PINS[led].port, LED_PINS[led].pinIndex, IfxPort_State_high);
    else IfxPort_setPinState(LED_PINS[led].port, LED_PINS[led].pinIndex, IfxPort_State_low);
}

void toggleLed(LedPins led){
    IfxPort_setPinState(LED_PINS[led].port, LED_PINS[led].pinIndex, IfxPort_State_toggled);
}
