/*
 * eru.h
 *
 *  Created on: 2025. 6. 24.
 *      Author: USER
 */

#ifndef BSW_MCAL_ERU_H_
#define BSW_MCAL_ERU_H_

#include "IfxPort.h"
#include "IfxScuWdt.h"
#include "IfxSrc.h"

#include "priority.h"


void scuEruInt0Handler();
void scuEruInit();

#endif
