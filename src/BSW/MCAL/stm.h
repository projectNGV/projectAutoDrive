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

void stm0IsrHandler(void);
void stmInterruptInit();


#endif /* BSW_DRIVER_STM_H_ */
