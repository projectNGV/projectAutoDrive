#ifndef ASW_FSM_H_
#define ASW_FSM_H_

#include "control.h"
#include "aeb.h"
#include "autopark.h"

typedef enum {
    STATE_IDLE,
    STATE_MANUAL_DRIVE,
    STATE_EMERGENCY_STOP,
    STATE_AUTO_PARK,
    STATE_LKAS,
    STATE_LKAS_STOPPED,   // LKAS 중 AEB로 정지된 상태
    STATE_LANE_CHANGE,     // 장애물 회피 (차선 변경) 수행 상태
    STATE_GET_RIGHT_SIGNS
} VehicleState;

void handleStateMachine(MotorState* motorState);


#endif /* ASW_FSM_H_ */
