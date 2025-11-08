#ifndef BSW_SERVICE_TOF_H_
#define BSW_SERVICE_TOF_H_

#include "can.h"
#include "motor.h"
#include "util.h"
#include "stdbool.h"
#include "IfxSrc_reg.h"
#include "aeb.h"

#define TOF_DEFAULT_VALUE_MM   5000

void tofInit (void);
void tofUpdateFromCAN (unsigned char *rxData);
unsigned int tofGetValue (void);
void tofOnOff (void);


#endif /* BSW_SERVICE_TOF_H_ */
