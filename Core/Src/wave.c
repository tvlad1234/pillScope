#include "main.h"
#include "scope.h"
#include "wave.h"
extern uint16_t adcBuf[BUFFER_LEN]; // this is where we'll store data

extern int atten;
extern float vdiv;

extern int trigPoint;
extern float trigVoltage;
extern uint8_t trig;

extern float maxVoltage, minVoltage;

float adcToVoltage(uint16_t samp)
{
    return atten * 2 * (((3.3 * samp) / 4096.0) - 1.65);
}

void drawWave()
{
    drawGraticule(64, 96, 16, 4, 4);

    maxVoltage = LOWER_VOLTAGE;
    minVoltage = UPPER_VOLTAGE;

    for (int i = 0; i <= 94; i++)
    {
        float voltage1 = adcToVoltage(adcBuf[i + trigPoint]);
        float voltage2 = adcToVoltage(adcBuf[i + trigPoint + 1]);
        drawLine(i, 31 - (voltage1 * 16 / vdiv), i + 1, 31 - (voltage2 * 16 / vdiv), WHITE);

        if (voltage2 > maxVoltage)
            maxVoltage = voltage2;
        if (voltage2 < minVoltage)
            minVoltage = voltage2;
    }
}

int findTrigger()
{
    int trigPoint = -1;
    int trigLevel = (4096.0 * (trigVoltage / (2.0 * atten) + 1.65)) / 3.3;
    uint8_t trigged = 0;

    for (int i = 1; i < BUFFER_LEN / 2 && !trigged; i++)
        if ((trig == RISING && adcBuf[i] >= trigLevel && adcBuf[i - 1] < trigLevel) || (trig == FALLING && adcBuf[i] <= trigLevel && adcBuf[i - 1] > trigLevel))
        {
            trigPoint = i;
            trigged = 1;
        }

    return trigPoint;
}