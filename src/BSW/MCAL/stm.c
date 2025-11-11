#include "stm.h"
#include "fsm.h"

//extern volatile bool aebFlag;
extern volatile bool obstacleDetectedFlag;

IFX_INTERRUPT(stm0IsrHandler, 0, ISR_PRIORITY_STM0);
void stm0IsrHandler(){
    //if(aebFlag == true)
        obstacleDetectedFlag = true;
    MODULE_STM0.ICR.B.CMP0EN = 0; // CMP0 Interrupt disable
    MODULE_STM0.ISCR.B.CMP0IRR = 1U; // clear CMP0 Interrupt Flag
}


void stm0InterruptInit(void){
    STM0_CMCON.B.MSIZE0 = 31; // 32비트 전체 비교
    STM0_CMCON.B.MSTART0 = 0; // STM 하위 32비트와 비교
    MODULE_STM0.ICR.B.CMP0OS = 0; // 인터럽트 출력 STMIR0 설정

    // 인터럽트 컨트롤러 설정
    MODULE_SRC.STM.STM[0].SR[0].B.TOS = 0; // CPU 0
    MODULE_SRC.STM.STM[0].SR[0].B.SRPN = ISR_PRIORITY_STM0;
    MODULE_SRC.STM.STM[0].SR[0].B.CLRR = 1; // clear Service Request Flag
    MODULE_SRC.STM.STM[0].SR[0].B.SRE = 1; // enable Service Request

//    MODULE_STM0.ISCR.B.CMP0IRR = 1U; // clear CMP0 Interrupt Flag
//    MODULE_STM0.ICR.B.CMP0EN = 1U; // CMP0 Interrupt Enable

    //MODULE_STM0.CMP[0].U = (uint32)(MODULE_STM0.TIM0.U + AEB_TIMEOUT_TICKS); // 첫 인터럽트 시점 설정
}

void stm0StartTimeout(void) {
    obstacleDetectedFlag = false;
    MODULE_STM0.CMP[0].U = (uint32)(MODULE_STM0.TIM0.U + AEB_TIMEOUT_TICKS);
    MODULE_STM0.ISCR.B.CMP0IRR = 1U;
    MODULE_STM0.ICR.B.CMP0EN = 1U;
}
