#include "fsm.h"
#include "lkas.h"
#include "stm.h"

// 외부에서 정의된 전역 변수: AEB(긴급 제동) 작동 여부 플래그
// 장애물과의 거리가 10cm 이하일 때 true가 되며, 주행을 중단시킴
extern volatile bool aebFlag;

// 긴급 정지 상태에서 한 번만 부저를 울리기 위한 내부 플래그 (true일 때만 부저 울림)
volatile bool buzzerFlag = TRUE;

// 차량의 현재 상태를 저장하는 전역 변수 (초기 상태는 STATE_IDLE)
VehicleState currentState = STATE_IDLE;


extern volatile bool tofFlag;
volatile bool obstacleDetectedFlag = false;
volatile bool signsRed = false;

/*********************************************************************************************************************/
// 현재 상태에 따라 차량의 동작을 제어하는 상태 머신 처리 함수
// ────────────────────────────────────────────────
// 목적: 차량의 상태(State)에 따라 입력과 센서 값을 기반으로 동작을 분기함
// 상태: IDLE, MANUAL_DRIVE, EMERGENCY_STOP, AUTO_PARK
// 입력: motorState → 사용자 입력 및 속도 상태 포함
/*********************************************************************************************************************/
void handleStateMachine (MotorState *motorState)
{
    // 현재 거리 측정 (TOF 센서) 및 AEB 플래그 갱신
    unsigned int distance = tofGetValue();  // 전방 거리(mm) 측정
    updateAebFlagByTof(distance);           // 거리에 따라 aebFlag 값을 true/false로 설정

    if (signsRed == true) {
        // LKAS/ACC 주행 중이라면 중단합니다.
        if (currentState == STATE_LKAS || currentState == STATE_LKAS_STOPPED || currentState == STATE_LANE_CHANGE) {
            LKAS_Stop(); // LKAS 중단 (모터 정지 및 플래그 초기화)
        }

        currentState = STATE_GET_RIGHT_SIGNS;
    }

    //myPrintf("currentState : %d, currentDuty : %d\n", currentState, motorState->currentDuty);
    // 현재 상태에 따라 동작 분기
    switch (currentState)
    {

        case STATE_LKAS :
        {
            if (aebFlag == true) { // aeb 발동 -> lkas 전용 정지 상태
                LKAS_Stop();
                performEmergencyStop();

                currentState = STATE_LKAS_STOPPED;
                stm0StartTimeout();
                break;
            }

            // ToF 거리가 멀면 ACC 끄고 고정 속도 주행, 가까우면 ACC 켜고 거리 유지
            if (distance < ACC_DEACTIVATION_THRESHOLD) {
                g_accEnable = TRUE;
            }
            else if (distance > ACC_ACTIVATION_THRESHOLD) {
                g_accEnable = FALSE;
            }

            if (g_accEnable == TRUE) {
                ACC_LKAS_Main(distance);
            }
            else {
                LKAS_Main();
            }

            break;
        }

        case STATE_LKAS_STOPPED:
        {
//            motorState->currentDuty = 0; // 속도를 0으로 초기화
//            motorStop();                 // 차량 완전 정지
            if (aebFlag == false) {
                LKAS_Start();
                currentState = STATE_LKAS;
            }

            if (obstacleDetectedFlag == true) { // 타임아웃 이벤트 발생
                currentState = STATE_LANE_CHANGE; // 차선 변경 상태로 변경
            }

            break;
        }

        case STATE_LANE_CHANGE:
        {
            moveForwardLeft(600);
            delayMs(700);
//            while (distance <= 250) {
//                moveForwardLeft(600);
//            }
//            motorStop();
//            moveForward(600);
//            delayMs(50);
            moveForwardRight(600);
            delayMs(1400);
            motorStop();
            delayMs(1000);

            aebFlag = false;
            obstacleDetectedFlag = false;

            currentState = STATE_LKAS;
            LKAS_Start();

            break;
        }

        case STATE_GET_RIGHT_SIGNS:
        {
            motorStop();
            delayMs(100);

            moveForwardRight(600);
            delayMs(1000);

            motorStop();
            delayMs(500);

            moveForward(600);
            delayMs(2000);

            motorStop();




            signsRed = false;
            currentState = STATE_IDLE;

            break;
        }

        // 정지 상태: 아무 동작도 하지 않는 기본 대기 상태
        case STATE_IDLE :
            if (motorState->lastKeyInput == 'l')
            {
                currentState = STATE_LKAS;
                LKAS_Start();
            }
            // 숫자 키(1~9)가 입력되면 수동 주행 상태로 전환
            else if (motorState->lastKeyInput >= '1' && motorState->lastKeyInput <= '9')
            {
                currentState = STATE_MANUAL_DRIVE;
            }

            // 'p' 키가 입력되면 자동 주차 상태로 전환
            else if (motorState->lastKeyInput == 'p')
            {
                currentState = STATE_AUTO_PARK;
            }

            break;

            // 사용자가 입력한 방향키(1~9)에 따라 주행을 수행하는 상태
        case STATE_MANUAL_DRIVE :

            // AEB 발동(앞에 장애물 존재) + 후진 키가 아닌 경우 → 긴급 정지 상태 진입
            if (aebFlag
                    && !(motorState->lastKeyInput == '1' || motorState->lastKeyInput == '2'
                            || motorState->lastKeyInput == '3'))
            {
                currentState = STATE_EMERGENCY_STOP;
            }
            else
            {
                // AEB가 꺼져 있거나, 후진 키(1,2,3)일 경우 정상 주행
                motorUpdateState(motorState);    // 방향/속도 업데이트
                motorRunCommand(motorState);     // 모터 실행 명령 전송
            }
            break;

            // AEB가 작동하여 차량이 강제로 멈춘 상태
        case STATE_EMERGENCY_STOP :

            performEmergencyStop();  // 모든 모터 정지 및 제동 로직 수행

            // 최초 1회만 부저를 울림 (지속 울림 방지)
            if (buzzerFlag)
            {
                emergencyBuzzer();   // 경고음 출력
                buzzerFlag = FALSE;  // 다시 울리지 않도록 플래그 꺼줌
                ledStartBlinking(LED_BOTH);
            }
            // 후진 키(1,2,3)가 입력된 경우 → 사용자가 장애물을 피하려는 의도
            // → 다시 수동 주행 상태로 전환 (후방 거리 확보 여부는 외부 로직에서 판단)
            if (motorState->lastKeyInput == '1' || motorState->lastKeyInput == '2' || motorState->lastKeyInput == '3')
            {
                buzzerFlag = TRUE;    // 다음 번 AEB 때 부저가 다시 울릴 수 있도록 초기화
                ledStopAll();
                currentState = STATE_MANUAL_DRIVE;
            }

            // 또는 AEB가 해제되면(앞에 장애물이 사라짐) → IDLE 상태로 복귀
            else if (!aebFlag)
            {

                buzzerFlag = TRUE;    // 다음 번 AEB 때 부저가 다시 울릴 수 있도록 초기화
                ledStopAll();
                currentState = STATE_IDLE;
            }

            break;

            // 자동 주차 알고리즘이 실행되는 상태
        case STATE_AUTO_PARK :

            autoPark();              // 주차 절차 수행 (장애물 회피, 후진 등 포함 가능)

            currentState = STATE_IDLE; // 주차 완료 후 대기 상태로 전환

            break;

    }
}
