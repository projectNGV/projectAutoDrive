#include "control.h"
#include "lkas.h"

/*********************************************************************************************************************/
// 모터 주행 제어 함수들
/*********************************************************************************************************************/

// 8 차량을 전진시킴 (양쪽 바퀴 모두 정방향 회전)
void moveForward(int duty)
{
    motorMovChAPwm(duty, Forward);  // 왼쪽 바퀴 정방향
    motorMovChBPwm(duty, Forward);  // 오른쪽 바퀴 정방향
}

// 2 차량을 후진시킴 (양쪽 바퀴 모두 역방향 회전)
void moveBackward(int duty)
{
    motorMovChAPwm(duty, Backward);  // 왼쪽 바퀴 역방향
    motorMovChBPwm(duty, Backward);  // 오른쪽 바퀴 역방향
}

// 4 차량을 제자리에서 왼쪽으로 회전시킴 (좌측 바퀴 역방향, 우측 바퀴 정방향)
void turnLeftInPlace(int duty)
{
    motorMovChAPwm(duty, Backward);
    motorMovChBPwm(duty, Forward);
}

// 6 차량을 제자리에서 오른쪽으로 회전시킴 (좌측 바퀴 정방향, 우측 바퀴 역방향)
void turnRightInPlace(int duty)
{
    motorMovChAPwm(duty, Forward);
    motorMovChBPwm(duty, Backward);
}

// 7 전진하면서 좌회전
void moveForwardLeft(int duty)
{
    motorMovChBPwm(duty, Forward);
    motorMovChAPwm(duty/32, Forward);
}

// 9 전진하면서 우회전
void moveForwardRight(int duty)
{
    motorMovChAPwm(duty, Forward);
    motorMovChBPwm(duty/32, Forward);
}

// 1 후진하면서 좌회전
void moveBackwardkLeft(int duty)
{
    motorMovChBPwm(duty, Backward);
    motorStopChA();
}

// 3 후진하면서 우회전
void moveBackwardRight(int duty)
{
    motorMovChAPwm(duty, Backward);
    motorStopChB();
}

/*********************************************************************************************************************/
// 키보드 입력 처리 함수
/*********************************************************************************************************************/

// 방향 키(1~9)를 입력받아 주행 방향을 설정하고, 현재 듀티를 baseDuty로 설정
void handleDirectionCommand(char cmd, MotorState* state)
{
    state->currentDir = cmd;               // 현재 방향 저장
    state->currentDuty = state->baseDuty;  // 기본 설정 듀티값 적용
}

// 속도 조절용 키(a~k)를 통해 baseDuty 설정
void handleDutyCommand(char cmd, MotorState* state)
{
    switch (cmd) {
        case 'a': state->baseDuty = 300; break;   // 가장 느린 속도
        case 's': state->baseDuty = 400; break;
        case 'd': state->baseDuty = 500; break;
        case 'f': state->baseDuty = 600; break;
        case 'g': state->baseDuty = 700; break;
        case 'h': state->baseDuty = 800; break;
        case 'j': state->baseDuty = 900; break;
        case 'k': state->baseDuty = 1000; break; // 가장 빠른 속도
        default: break;  // 예외 처리 (잘못된 입력 무시)
    }
}

// 브레이크 입력 ('B')이 들어오면 감속 수행
void handleBrakeCommand(MotorState* state)
{
    motorSoftBraking(state->currentDuty);  // 현재 Duty 기준으로 감속 동작 수행
}

/*********************************************************************************************************************/
// 상태 업데이트 함수
/*********************************************************************************************************************/

// 키 입력에 따라 모터 상태(state)를 업데이트 (입력 기반 명령 판단)
void motorUpdateState(MotorState* state)
{
    char cmd = state->lastKeyInput;  // 최근 키 입력 가져오기

    // 숫자키 1~9 입력 → 방향 설정
    if (cmd >= '1' && cmd <= '9' && cmd != 'B')
    {
        handleDirectionCommand(cmd, state);
    }

    // 속도 조절 키 입력 → baseDuty 설정
    else if (cmd == 'a' || cmd == 's' || cmd == 'd' || cmd == 'f' ||
             cmd == 'g' || cmd == 'h' || cmd == 'j' || cmd == 'k')
    {
        handleDutyCommand(cmd, state);
    }

    // 감속 명령 (키 뗐을 때)
    else if (cmd == 'B')
    {
        handleBrakeCommand(state);        // 감속 실행
        ledStopAll();
    }

    else if (cmd == 'p')
    {
        autoPark();                                 // 주차 절차 수행 (장애물 회피, 후진 등 포함 가능)
        while(state->lastKeyInput != '8');
        ledStopAll();
    }

    // TOF 센서 On/Off 전환
    else if (cmd == 't')
    {
        tofOnOff();               // 센서 끄기/켜기 전환
        state->lastKeyInput = '5'; // 입력 초기화 (정지 상태로 변경)
    }
}

/*********************************************************************************************************************/
// 실제 모터 구동 함수
/*********************************************************************************************************************/

// 실제로 모터를 회전시키는 함수
// direction 값에 따라 좌우 바퀴에 정/역방향 PWM을 인가
void motorRunCommand (MotorState* state)
{
    char cmd = state->lastKeyInput;    // 현재 입력된 키
    int duty = state->currentDuty;     // 현재 속도(Duty)

    // 방향키가 입력되었을 경우, 마지막 방향 상태(prevDir)를 갱신
    if (cmd != 'B' && cmd != '5')  // 감속 or 정지가 아닐 경우
    {
        state->prevDir = cmd;
    }

    // 방향에 따라 모터 및 LED 제어
    switch (cmd)
    {
        case '8' :
            ledStopAll();
            moveForward(duty);
            break;

        case '2' :
            ledStopAll();
            moveBackward(duty);
            break;

        case '4' :
            turnLeftInPlace(duty);
            ledSetRight(0);
            ledStartBlinking(LED_LEFT);     // 좌측 깜빡이 점등
            break;

        case '6' :
            turnRightInPlace(duty);
            ledSetLeft(0);
            ledStartBlinking(LED_RIGHT);    // 우측 깜빡이 점등
            break;

        case '5' :
            motorStop();                    // 정지
            ledStopAll();
            break;

        case '7' :
            moveForwardLeft(duty);
            ledSetRight(0);
            ledStartBlinking(LED_LEFT);     // 좌측 깜빡이 점등
            break;

        case '9' :
            moveForwardRight(duty);
            ledSetLeft(0);
            ledStartBlinking(LED_RIGHT);    // 우측 깜빡이 점등
            break;

        case '1' :
            ledStopAll();
            moveBackwardkLeft(duty);
            break;

        case '3' :
            ledStopAll();
            moveBackwardRight(duty);
            break;
    }
}
