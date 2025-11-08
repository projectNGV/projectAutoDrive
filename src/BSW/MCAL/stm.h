/*
 * stm.h
 *
 *  Created on: 2025. 6. 25.
 *      Author: USER
 */

#ifndef BSW_DRIVER_STM_H_
#define BSW_DRIVER_STM_H_

#include "IfxStm.h"
#include "priority.h"


#define STM_CLOCK_HZ            100000000UL // 클럭 주파수 100MHz
#define TICKS_PER_MS    (STM_CLOCK_HZ / 1000UL)
#define AEB_TIMEOUT_MS 3000 // 타임아웃 시간 (ms)
#define AEB_TIMEOUT_TICKS (AEB_TIMEOUT_MS * TICKS_PER_MS)


void stm0IsrHandler(void);
void stm0InterruptInit(void);

void stm0StartTimeout(void);


#endif /* BSW_DRIVER_STM_H_ */
