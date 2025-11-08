
#ifndef ASW_CONTROL_H_
#define ASW_CONTROL_H_

#include "bluetooth.h"
#include "motor.h"
#include "tof.h"
#include "led.h"
#include "autopark.h"

#define Forward 1
#define Backward 0

typedef struct {
    int baseDuty;       // 사용자 설정 Duty
    int currentDuty;    // 현재 Duty
    char currentDir;    // 현재 주행 방향
    char prevDir;       // 이전 주행 방향
    char lastKeyInput;  // 방금 받은 키보드 입력
} MotorState;

void moveForward(int duty);
void moveBackward(int duty);
void turnLeftInPlace(int duty);
void turnRightInPlace(int duty);
void moveForwardLeft(int duty);
void moveForwardRight(int duty);
void moveBackwardkLeft(int duty);
void moveBackwardRight(int duty);

void handleDirectionCommand(char cmd, MotorState* state);
void handleDutyCommand(char cmd, MotorState* state);
void handleBrakeCommand(MotorState* state);
void motorUpdateState(MotorState* state);
void motorRunCommand (MotorState* state);

#endif /* ASW_CONTROL_H_ */
