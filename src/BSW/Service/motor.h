/*
 * motor.h
 *
 *  Created on: 2025. 6. 27.
 *      Author: USER
 */

#ifndef BSW_DRIVER_MOTOR_H_
#define BSW_DRIVER_MOTOR_H_

#include "Ifx_Types.h"
#include "IfxGpt12.h"
#include "IfxPort.h"

#include "bluetooth.h"
#include "gtm_atom_pwm.h"

void motorInit(void);

///* 1: 정방향, 2: 역방향 */
void motorMovChA(int dir);
void motorStopChA(void);
///* 1: 정방향, 0: 역방향 */
void motorMovChAPwm(int duty, int dir);
///* 1: 정방향, 2: 역방향 */
void motorMovChB(int dir);
void motorStopChB(void);
///* 1: 정방향, 0: 역방향 */
void motorMovChBPwm(int duty, int dir);
void motorKeypadPwm(char c, int duty);

void motorSoftBraking(int duty);
void motorHardBraking(int duty);

void motorMoveForward(int duty);
void motorMoveReverse (int duty);
void motorStop(void);

#endif /* BSW_DRIVER_MOTOR_H_ */
