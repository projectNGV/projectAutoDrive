#include "eru.h"

// SCU: System Control Unit 시스템 컨트롤 유닛, ERU: External Request Unit 외부(버튼) 인터럽트
void scuEruInit()
{
    uint16 password = IfxScuWdt_getSafetyWatchdogPasswordInline();
    IfxScuWdt_clearSafetyEndinitInline(password);

    MODULE_P02.IOCR0.B.PC1 = (1 << 1);
    MODULE_SCU.EICR[1].B.EXIS0 = 1;

    MODULE_SCU.EICR[1].B.REN0 = 0;
    MODULE_SCU.EICR[1].B.FEN0 = 1;

    MODULE_SCU.EICR[1].B.EIEN0 = 1;
    MODULE_SCU.EICR[1].B.INP0 = 0;

    MODULE_SCU.IGCR[0].B.IGP0 = 1;

//
//    Ifx_SRC_SRCR_Bits* src = (Ifx_SRC_SRCR_Bits*) &MODULE_SRC.SCU.SCUERU[0].B;
//    src->SRPN = ISR_PRIORITY_ERU_INT0;
//    src->TOS = 0;
//    src->CLRR = 1;
//    src->SRE = 1;
//    IfxScuWdt_setSafetyEndinitInline(password);
}

IFX_INTERRUPT(SCUERU_Int0_Handler, 0, ISR_PRIORITY_ERU_INT0);
void scuEruInt0Handler(){

}
