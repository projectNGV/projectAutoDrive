#include "tof.h"

static unsigned int g_TofValue;
volatile bool tofFlag = false;

void tofInit (void)
{
    canInit(BD_500K, CAN_NODE0);
    canRegisterTofCallback(tofUpdateFromCAN);
    g_TofValue = TOF_DEFAULT_VALUE_MM;
    tofFlag = false;
}

void tofOnOff(void)
{
    if (tofFlag)
    {
        tofFlag = false;
    }
    else
    {
        tofFlag = true;
    }
}

void tofUpdateFromCAN (unsigned char *rxData)
{
    if (rxData == NULL) return;

    unsigned short signal_strength = rxData[5] << 8 | rxData[4];

    if (signal_strength != 0)
    {
        g_TofValue = rxData[2] << 16 | rxData[1] << 8 | rxData[0];
        updateAebFlagByTof(g_TofValue);
    }
}

unsigned int tofGetValue (void)
{
    return g_TofValue;
}
