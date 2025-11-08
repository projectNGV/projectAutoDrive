#ifndef ASW_EMERGENCYSTOP_AEB_H_
#define ASW_EMERGENCYSTOP_AEB_H_

#include "stdbool.h"
#include "control.h"
#include "buzzer.h"

#define REVERSE_TIME_MS         150
#define DUTY_LIMIT_DISTANCE_MM  950

#define AEB_DISTANCE_MM         350
#define SAFETY_DISTANCE_MM      500

void emergencyBuzzer (void);
void performEmergencyStop(void);
void updateAebFlagByTof (unsigned int g_TofValue);

#endif /* ASW_EMERGENCYSTOP_AEB_H_ */
