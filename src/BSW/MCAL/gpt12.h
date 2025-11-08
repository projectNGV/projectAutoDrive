/*
 * gpt12.h
 *
 *  Created on: 2025. 6. 25.
 *      Author: USER
 */

#ifndef BSW_DRIVER_GPT12_H_
#define BSW_DRIVER_GPT12_H_

#include "Ifx_Types.h"
#include "IfxCpu.h"
#include "IfxScuWdt.h"

#include "IfxGpt12.h"
#include "priority.h"
#include "led.h"
#include "buzzerport.h"

//#define BEEP_INITIAL_INTERVAL 8000      // 초기 주기 (느리게 시작)
#define BEEP_MIN_INTERVAL     1000      // 최소 주기 (0.1초 정도)
#define BEEP_INTERVAL_STEP    2000      // 인터럽트마다 줄이는 양

void gpt1_init ();
void gpt2_init (void);
void gpt12Init(void);

void Gpt1_Interrupt_Enable (void);
void Gpt1_Interrupt_Disable (void);
void Gpt2_Interrupt_Enable(void);
void Gpt2_Interrupt_Disable(void);

#endif /* BSW_DRIVER_GPT12_H_ */
