/*********************************************************************************************************************/
/*-----------------------------------------------------Includes------------------------------------------------------*/
/*********************************************************************************************************************/

#include "autopark.h"

#include "asclin1.h"
#include "ultrasonic.h"
#include "buzzer.h"
#include "led.h"
#include "gpt12.h"

#include <stdlib.h>
#include <stdio.h>

/*********************************************************************************************************************/
/*------------------------------------------------------Macros-------------------------------------------------------*/
/*********************************************************************************************************************/

#define MOTOR_STOP_DELAY 500

/*********************************************************************************************************************/
/*-------------------------------------------------Global variables--------------------------------------------------*/
/*********************************************************************************************************************/

extern volatile boolean g_rx_getLine;
extern volatile char g_rx_buffer[RX_BUFFER_SIZE];
extern volatile uint16 g_beepInterval;

/*********************************************************************************************************************/
/*--------------------------------------------Private Variables/Constants--------------------------------------------*/
/*********************************************************************************************************************/

// Parking Distance
static int parkingDistance = 250000;

// Parking Speed
static int parkingSpeedForward = 350;
static int parkingSpeedBackward = 350;

// Parking Tick
static volatile int parkingFoundTick = 4;

// Forward & Rotate 90 Degree Rightward
static int goForwardDelay = 0;
static int rotateDelay = 375;

// Backward Stop Distance
static int stopDistance = 120000;

/*********************************************************************************************************************/
/*------------------------------------------------Function Prototypes------------------------------------------------*/
/*********************************************************************************************************************/

static void foundSpace ();
static void rotate ();
static void goBackWard ();

static void tuneParkingDistance ();
static void tuneParkingSpeed ();
static void tuneParkingFoundTick ();
static void tuneRotate ();
static void tuneStopDistance ();

void autoParkTune ();
void autoPark ();

/*********************************************************************************************************************/
/*---------------------------------------------Function Implementations----------------------------------------------*/
/*********************************************************************************************************************/

static void foundSpace ()
{
    int curTick = 0;
    motorMoveForward(parkingSpeedForward);
    while (curTick < parkingFoundTick)
    {
        if (parkingDistance <= getDistanceByUltra(ULT_LEFT))
        {
            curTick++;
            if (curTick >= parkingFoundTick)
                break;
        }
        else
        {
            curTick = 0;
        }
        delayMs(50);
    }
    motorStop();
}

static void rotate ()
{
    motorMoveForward(parkingSpeedForward);
    delayMs(goForwardDelay);
    motorStop();
    delayMs(MOTOR_STOP_DELAY);
    motorMovChAPwm(0, 1);
    motorMovChBPwm(1000, 0);
    delayMs(rotateDelay);
    motorStop();
}

static void goBackWard ()
{
    motorMoveReverse(parkingSpeedBackward);

    int rearDis = getDistanceByUltra(ULT_REAR);
    g_beepInterval = rearDis / 100;
    buzzerOn();
    Gpt1_Interrupt_Enable ();
    while (rearDis > stopDistance)
    {
        delayMs(50);
        rearDis = getDistanceByUltra(ULT_REAR);
        g_beepInterval = rearDis / 100;
    }
    motorStop();
    Gpt1_Interrupt_Disable();
    buzzerOn();
    ledStartBlinking(LED_BOTH);
    delayMs(2500);
    buzzerOff();

}

static void tuneParkingDistance ()
{
    while (1)
    {
        bluetoothPrintf("?주차 공간 입력 [c] - 왼쪽 초음파 거리, [y] - 확인 (현재거리: %d)\n", parkingDistance);
        while (!g_rx_getLine)
            ;
        if (g_rx_buffer[0] == 'c')
        {
            int leftDis = getDistanceByUltra(ULT_LEFT);
            bluetoothPrintf("현재 초음파 거리: %d\n", leftDis);
        }
        else if (g_rx_buffer[0] == 'y')
        {
            bluetoothPrintf("주차 공간 설정 완료: %d\n", parkingDistance);
            rxBufferFlush();
            break;
        }
        else
        {
            parkingDistance = atoi(g_rx_buffer);
            bluetoothPrintf("parkingDistance 변경 완료: %d\n", parkingDistance);
        }
        rxBufferFlush();
    }
}

static void tuneParkingSpeed ()
{
    while (1)
    {
        bluetoothPrintf("[주차 공간 찾기] 직진 후진 속도 조절\n");
        bluetoothPrintf("?[y] - 확인\t(현재 직진: %d, 현재 후진: %d)\n", parkingSpeedForward, parkingSpeedBackward);
        while (!g_rx_getLine)
            ;
        if (g_rx_buffer[0] == 'y')
        {
            bluetoothPrintf("속도 설정 완료\n");
            bluetoothPrintf("직진: %d\t후진: %d\n", parkingSpeedForward, parkingSpeedBackward);
            rxBufferFlush();
            break;
        }
        else
        {
            sscanf(g_rx_buffer, "%d %d", &parkingSpeedForward, &parkingSpeedBackward);
        }
        rxBufferFlush();

        /**
         * 직진 보여줘야 되는데 몇초나 할지
         */
        motorMoveForward(parkingSpeedForward);
        delayMs(2000);
        motorStop();
        delayMs(MOTOR_STOP_DELAY);
        motorMoveReverse(parkingSpeedBackward);
        delayMs(2000);
        motorStop();
        delayMs(MOTOR_STOP_DELAY);
    }
}

static void tuneParkingFoundTick ()
{

    while (1)
    {
        bluetoothPrintf("?[주차 공간 찾기] Tick 값 설정 [y] - 확인 (현재 Tick: %d)\n", parkingFoundTick);
        while (!g_rx_getLine)
            ;
        if (g_rx_buffer[0] == 'y')
        {
            bluetoothPrintf("Tick 값 설정 완료: %d\n", parkingFoundTick);
            rxBufferFlush();
            break;
        }
        else
        {
            parkingFoundTick = atoi(g_rx_buffer);
        }
        rxBufferFlush();

        foundSpace();
    }

}

static void tuneRotate ()
{
    while (1)
    {
        bluetoothPrintf("[주차] 직진 & 후진 딜레이 조절 (현재 직진 딜레이: %d, 후진 딜레이: %d)\n", goForwardDelay, rotateDelay);
        bluetoothPrintf("?[y] - 확인 [r] - 주차 공간 찾기\n");
        while (!g_rx_getLine)
            ;
        if (g_rx_buffer[0] == 'y')
        {
            bluetoothPrintf("딜레이 설정 완료 직진: %d\t후진: %d\n", goForwardDelay, rotateDelay);
            rxBufferFlush();
            break;
        }
        else if (g_rx_buffer[0] == 'r')
        {
            foundSpace();
        }
        else
        {
            sscanf(g_rx_buffer, "%d %d", &goForwardDelay, &rotateDelay);
        }
        rxBufferFlush();
        rotate();
    }
}

static void tuneStopDistance ()
{
    while (1)
    {
        bluetoothPrintf("[주차] 후진 거리 조절 (현재 후진 거리: %d)\n", stopDistance);
        bluetoothPrintf("?[c] - 뒤쪽 거리 출력\t[y] - 확인 \n");
        while (!g_rx_getLine)
            ;
        if (g_rx_buffer[0] == 'y')
        {
            bluetoothPrintf("후진 거리 설정 완료: %d\n", stopDistance);
            rxBufferFlush();
            break;
        }
        else if (g_rx_buffer[0] == 'c')
        {
            int rearDis = getDistanceByUltra(ULT_REAR);
            bluetoothPrintf("현재 초음파 거리: %d\n", rearDis);
        }
        else
        {
            stopDistance = atoi(g_rx_buffer);
            goBackWard();
        }
        rxBufferFlush();
    }
}

void autoParkTune (void)
{
    boolean isTuned = FALSE;

    while (1)
    {
        bluetoothPrintf("\n");
        bluetoothPrintf("\n");
        bluetoothPrintf("===========현재 값===========\n");
        bluetoothPrintf("1. [주차 공간 찾기] 주차공간: %d\n", parkingDistance);
        bluetoothPrintf("2. [주차 공간 찾기] 전진속도: %d\t", parkingSpeedForward);
        bluetoothPrintf("후진속도: %d\n", parkingSpeedBackward);
        bluetoothPrintf("3. [주차 공간 찾기] Tick: %d\n", parkingFoundTick);
        bluetoothPrintf("4. [주차 90도 들어가기] 전진 딜레이: %d\t", goForwardDelay);
        bluetoothPrintf("4. 후진 우회전 딜레이: %d\n", rotateDelay);
        bluetoothPrintf("5. [주차] 후진 정지 거리: %d\n", stopDistance);
        bluetoothPrintf("?[r] - 시험 주행\t[c]- 확인\t[#]- 재설정\n");
        while (!g_rx_getLine)
            ;
        switch (g_rx_buffer[0])
        {
            case 'r' :
                rxBufferFlush();
                autoPark();
                break;
            case 'c' :
                rxBufferFlush();
                isTuned = true;
                break;
            case '1' :
                rxBufferFlush();
                tuneParkingDistance();
                break;
            case '2' :
                rxBufferFlush();
                tuneParkingSpeed();
                break;
            case '3' :
                rxBufferFlush();
                tuneParkingFoundTick();
                break;
            case '4' :
                rxBufferFlush();
                tuneRotate();
                break;
            case '5' :
                rxBufferFlush();
                tuneStopDistance();
                break;
            default :
                rxBufferFlush();
                bluetoothPrintf("UNKNOWN COMMAND\n");
                break;
        }



        if (isTuned)
            break;
    }
}

void autoPark (void)
{
    foundSpace();
    rotate();
    delayMs(MOTOR_STOP_DELAY);
    goBackWard();
}
