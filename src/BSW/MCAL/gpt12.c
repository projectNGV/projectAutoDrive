#include "gpt12.h"

volatile uint16 g_beepInterval = 8000;

// GPT1 타이머 인터럽트 핸들러 - 0.5초마다 BUZZER 토글
IFX_INTERRUPT(IsrGpt1T3Handler, 0, ISR_PRIORITY_GPT1T3_TIMER);
void IsrGpt1T3Handler(void)
{
    // 부저 토글
    buzzerToggle();

    // 주기 감소 (더 이상 줄이지 않도록 최소 한계 체크)
//    if (g_beepInterval > BEEP_MIN_INTERVAL)
//    {
//        g_beepInterval -= BEEP_INTERVAL_STEP;
//        if (g_beepInterval < BEEP_MIN_INTERVAL)
//        {
//            g_beepInterval = BEEP_MIN_INTERVAL;
//        }
//    }

    // 새로운 주기로 타이머 설정
    MODULE_GPT120.T3.B.T3 = g_beepInterval;
}

// GPT2 T6 인터럽트 핸들러 - 0.5초마다 LED 토글
IFX_INTERRUPT(IsrGpt2T6Handler, 0, ISR_PRIORITY_GPT2T6_TIMER);
void IsrGpt2T6Handler(void)
{
    ledUpdateBlinking();
}

// buzzer용 gpt타이머
void gpt1_init (void)
{
    Ifx_GPT12_T3CON_Bits *t3con = (Ifx_GPT12_T3CON_Bits*) &MODULE_GPT120.T3CON.B;

    t3con->T3M = 0;     // 타이머 모드
    t3con->BPS1 = 2;    // 분주기 설정 (25MHz 기준)
    t3con->T3I = 7;     // 인터럽트 우선순위
    t3con->T3UD = 1;    // 카운트 다운 모드
    MODULE_GPT120.T3.B.T3 = g_beepInterval; // 타이머 값

    // 인터럽트 설정 (일단 비활성화 상태)
    Ifx_SRC_SRCR_Bits* src = (Ifx_SRC_SRCR_Bits*) &MODULE_SRC.GPT12.GPT12[0].T3.B;
    src->SRPN = ISR_PRIORITY_GPT1T3_TIMER;
    src->TOS = 0;
    src->CLRR = 1;
    src->SRE = 0; // Enable은 외부에서

    // 타이머 시작
    t3con->T3R = 1;
}

// led용 gpt타이머
void gpt2_init (void)
{
    Ifx_GPT12_T6CON_Bits *t6con = (Ifx_GPT12_T6CON_Bits*) &MODULE_GPT120.T6CON.B;

    t6con->T6M = 0;         // 타이머 모드
    t6con->BPS2 = 2;        // 분주기 설정
    t6con->T6I = 7;         // 인터럽트 우선순위
    t6con->T6UD = 1;        // 카운트 다운
    t6con->T6SR = 1;        // 자동 리로드

    // 0.5초 타이머 값 (24414 = 대략 0.5초)
    MODULE_GPT120.T6.B.T6 = 24414;
    MODULE_GPT120.CAPREL.B.CAPREL = 24414;

    // 인터럽트 설정
    Ifx_SRC_SRCR_Bits *src = (Ifx_SRC_SRCR_Bits*) &MODULE_SRC.GPT12.GPT12[0].T6.B;
    src->SRPN = ISR_PRIORITY_GPT2T6_TIMER;
    src->TOS = 0;
    src->CLRR = 1;
    src->SRE = 1;

    // 타이머 시작
    t6con->T6R = 1;
}

// GPT1 인터럽트 Enable/Disable 제어
void Gpt1_Interrupt_Enable (void)
{
    MODULE_SRC.GPT12.GPT12[0].T3.B.SRE = 1;
}

void Gpt1_Interrupt_Disable (void)
{
    MODULE_SRC.GPT12.GPT12[0].T3.B.SRE = 0;
}

// GPT2 인터럽트 Enable/Disable 제어
void Gpt2_Interrupt_Enable (void)
{
    MODULE_SRC.GPT12.GPT12[0].T6.B.SRE = 1;
}

void Gpt2_Interrupt_Disable (void)
{
    MODULE_SRC.GPT12.GPT12[0].T6.B.SRE = 0;
}

// GPT12 전체 초기화
void gpt12Init ()
{
    IfxScuWdt_clearCpuEndinit(IfxScuWdt_getGlobalEndinitPassword());
    MODULE_GPT120.CLC.U = 0;
    IfxScuWdt_setCpuEndinit(IfxScuWdt_getGlobalEndinitPassword());

    gpt1_init();
    gpt2_init();
}
