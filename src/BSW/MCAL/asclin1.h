#ifndef BSW_MCAL_ASCLIN1_H_
#define BSW_MCAL_ASCLIN1_H_

#include "IfxAsclin.h"
#include "IfxAsclin_bf.h"
#include "priority.h"
#include "control.h"

#define RX_BUFFER_SIZE 32

void asclin1InitUart(void);
void asclin1OutUart(const unsigned char chr);
unsigned char asclin1InUart(void);
int asclin1PollUart(unsigned char *chr);


#endif /* BSW_MCAL_ASCLIN1_H_ */
