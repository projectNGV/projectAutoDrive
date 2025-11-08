#include "stm.h"

IFX_INTERRUPT(Stm0IsrHandler, 0, ISR_PRIORITY_STM0);
void stm0IsrHandler(){
    MODULE_STM0.CMP[0].B.CMPVAL = MODULE_STM0.TIM0.U + 100000000;
}


void stmInterruptInit(){
    MODULE_STM0.CMCON.B.MSIZE0 = 0x1F;
    MODULE_STM0.CMCON.B.MSTART0 = 0;

    MODULE_STM0.ICR.B.CMP0OS = 1; // IR(SR) 선택

//    Ifx_SRC_SRCR_Bits* src = (Ifx_SRC_SRCR_Bits*) &MODULE_SRC.STM.STM[0].SR[1].B;
//    src->SRPN = ISR_PRIORITY_STM0;
//    src->TOS = 0;
//    src->CLRR = 1;
//    src->SRE = 1;
//
//    MODULE_STM0.ICR.B.CMP0EN = 1;
//    MODULE_STM0.CMP[0].B.CMPVAL = (Ifx_UReg_32Bit) MODULE_STM0.TIM0.U + 100000000;

}
