#ifndef __LKAS_H__
#define __LKAS_H__

#include "Ifx_Types.h"

extern boolean g_lkasEnable;

void LKAS_UpdateSteerBuffer(int value);
void LKAS_Start();
void LKAS_Main();
void LKAS_Stop();
//static boolean LKAS_PopLatest(int* outVal);

#endif

