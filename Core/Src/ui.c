#include "main.h"
#include "scope.h"
#include "ui.h"
#include "wave.h"

extern uint16_t adcBuf[BUFFER_LEN]; // this is where we'll store data

extern int atten;
extern float vdiv;
extern float trigVoltage;
extern uint8_t trig;

extern float tdiv;
extern uint32_t measuredFreq;
extern uint32_t sampRate;
extern float maxVoltage, minVoltage;

extern int currentMenu;

void ui()
{
    clearDisplay();
    drawWave();
    sideMenu();
    flushDisplay();
}

void sideMenu()
{
    switch (currentMenu)
    {
    case 1: // Print voltage measurements
        voltageInfo();
        break;
    case 2: // Volts per div and trig
        vDivMenu();
        break;
    case 3: // Time per div
        tDivMenu();
        break;
    default:
        break;
    }

    if (!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin))
    {
        currentMenu++;
        if (currentMenu > 3)
            currentMenu = 1;
        HAL_Delay(100);
    }
}

void voltageInfo()
{
    char st[15];
    printFloat(minVoltage, 1, st);
    setTextColor(BLACK, WHITE);
    setCursor(100, 1);
    printString("Min:");
    setTextColor(WHITE, BLACK);
    setCursor(100, 10);
    printf("%s\n", st);

    printFloat(maxVoltage, 1, st);
    setTextColor(BLACK, WHITE);
    setCursor(100, 21);
    printString("Max:");
    setTextColor(WHITE, BLACK);
    setCursor(100, 30);
    printf("%s\n", st);

    setTextColor(BLACK, WHITE);
    setCursor(100, 41);
    printString("Ppk:");
    setTextColor(WHITE, BLACK);
    setCursor(100, 51);
    printFloat(maxVoltage - minVoltage, 1, st);
    printf("%sV\n", st);
}

void vDivMenu()
{

    static uint8_t sel = 0;

    char st[10];

    setTextColor(BLACK, WHITE);
    setCursor(100, 1);
    printString("Vdiv");

    setTextColor(BLACK, WHITE);
    setCursor(100, 21);
    printString("Trig");

    setTextColor(WHITE, BLACK);
    if (sel == 0)
        setTextColor(BLACK, WHITE);
    setCursor(100, 10);
    printFloat(vdiv, 1, st);
    printf("%sV\n", st);

    setTextColor(WHITE, BLACK);
    if (sel == 1)
    {
        setTextColor(BLACK, WHITE);
        drawFastHLine(0, 31 - (trigVoltage * 16 / vdiv), 96, WHITE);
    }
    setCursor(100, 30);
    printFloat(trigVoltage, 1, st);
    printf("%s\n", st);

    setTextColor(WHITE, BLACK);
    setCursor(100, 40);
    if (sel == 2)
        setTextColor(BLACK, WHITE);
    if (trig == RISING)
        printf("Rise\n");
    else
        printf("Fall\n");

    setTextColor(WHITE, BLACK);
    setCursor(100, 50);
    if (sel == 3)
        setTextColor(BLACK, WHITE);
    printf("%dx\n", atten);

    if (!HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin))
    {
        if (sel == 0) // vdiv
        {
            if (vdiv > 0.5)
                vdiv -= 0.5;
        }
        else if (sel == 1) // trigLevel
        {
            if (trigVoltage > -4)
                trigVoltage -= 0.1;
        }
        else if (sel == 2) // trigType
        {
            trig = FALLING;
        }
        else if (sel == 3) // atten
        {
            atten = 1;
        }

        HAL_Delay(100);
    }

    if (!HAL_GPIO_ReadPin(BTN4_GPIO_Port, BTN4_Pin))
    {
        if (sel == 0) // vdiv
        {
            if (vdiv < 2.5)
                vdiv += 0.5;
        }
        else if (sel == 1) // trigLevel
        {
            if (trigVoltage < 4)
                trigVoltage += 0.1;
        }
        else if (sel == 2) // trigType
        {
            trig = RISING;
        }
        else if (sel == 3) // atten
        {
            atten = 2;
        }
        HAL_Delay(100);
    }

    if (!HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin))
    {
        sel++;
        HAL_Delay(100);
    }
    if (sel > 3)
        sel = 0;
}

void tDivMenu()
{
    char st[10];

    setTextColor(BLACK, WHITE);
    setCursor(100, 1);
    if (tdiv < 1000)
        printString("us/d");
    else
        printString("ms/d");

    setTextColor(WHITE, BLACK);
    setCursor(100, 10);

    if (tdiv < 1000)
        printf("%d\n", (int)tdiv);
    else
        printf("%d\n", (int)tdiv / 1000);

    if (!HAL_GPIO_ReadPin(BTN3_GPIO_Port, BTN3_Pin))
    {
        if (tdiv > 50)
        {
            if (tdiv > 1000)
                tdiv -= 1000;
            else if (tdiv > 100)
                tdiv -= 100;
            else if (tdiv > 10)
                tdiv -= 10;
        }

        sampRate = (16000 * 1000) / tdiv;
        setTimerFreq(sampRate);

        HAL_Delay(100);
    }

    if (!HAL_GPIO_ReadPin(BTN4_GPIO_Port, BTN4_Pin))
    {
        if (tdiv >= 1000)
            tdiv += 1000;
        else if (tdiv >= 100)
            tdiv += 100;
        else if (tdiv >= 10)
            tdiv += 10;

        sampRate = (16000 * 1000) / tdiv;
        setTimerFreq(sampRate);

        HAL_Delay(100);
    }
}

void drawGraticule(int hei, int wit, int pix, int divx, int divy)
{
    for (int i = 0; i <= wit; i += pix)
        dottedVLine(i, 0, hei);

    for (int i = 0; i <= hei; i += pix)
        dottedHLine(0, i, wit);
}

void dottedHLine(int x, int y, int l)
{
    uint8_t col = 1;
    for (int i = 0; i <= l; i++)
    {
        drawPixel(x + i, y, col);
        col = !col;
    }
}

void dottedVLine(int x, int y, int l)
{
    uint8_t col = 1;
    for (int i = 0; i <= l; i++)
    {
        drawPixel(x, y + i, col);
        col = !col;
    }
}
