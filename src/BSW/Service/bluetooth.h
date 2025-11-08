/*
 * Bluetooth.h
 *
 *  Created on: 2025. 6. 29.
 *      Author: USER
 */

#ifndef BSW_IO_BLUETOOTH_H_
#define BSW_IO_BLUETOOTH_H_

#include <stdarg.h>

#include "uart.h"
#include "util.h"
#include "priority.h"
#include "asclin1.h"

void bluetoothInit(void);
void bluetoothSetName(char *name);
void bluetoothSetPwd(char *pwd);
void bluetoothSetBaud(int baudrate);
void bluetoothAtCommand(char *cmd);
char bluetoothRecvByteBlocked(void);
char bluetoothRecvByteNonBlocked(void);
void bluetoothSendByteBlocked(unsigned char ch);
void bluetoothPrintf(const char *fmt, ...);
void bluetoothScanf(const char *fmt, ...);

static void remove_null(char *s);


#endif /* BSW_IO_BLUETOOTH_H_ */
