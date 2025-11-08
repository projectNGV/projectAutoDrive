#ifndef BSW_MCAL_LEDPORT_H_
#define BSW_MCAL_LEDPORT_H_

#include "IfxPort.h"

void ledSetRight(int on);
void ledSetLeft(int on);
void ledSetRear(int on);
void ledPortInit(void);
void ledToggleRight(void);
void ledToggleLeft(void);
void ledToggleRear(void);

#endif /* BSW_MCAL_LEDPORT_H_ */
