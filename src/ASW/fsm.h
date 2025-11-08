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
    STATE_LKAS
} VehicleState;

void handleStateMachine(MotorState* motorState);


#endif /* ASW_FSM_H_ */
