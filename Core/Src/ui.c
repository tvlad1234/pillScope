#include "main.h"
#include "scope.h"
#include "ui.h"
#include "wave.h"

#include "usbd_cdc_if.h"

extern uint16_t adcBuf[BUFFER_LEN]; // this is where we'll store data

extern int atten;
extern float vdiv;
extern float trigVoltage;
extern uint8_t trig;
extern int trigPoint;

extern float tdiv;
extern uint32_t measuredFreq;
extern uint32_t sampRate;
extern float maxVoltage, minVoltage;

extern int currentMenu;

uint8_t outputFlag = 0;

void splash()
{
    printString("pillScope\nCompiled ");
    printString(__DATE__);
    flushDisplay();
    HAL_Delay(2500);
}

void ui()
{
    clearDisplay();
    drawWave();
    sideMenu();

    if (outputFlag) // If the computer requested data, we send it. This flag is modified in the USB receive handler in usbd_cdc_if.c
    {
        outputCSV();
        outputFlag = 0;
    }

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
    case 4: // USB menu
        usbMenu();
        break;
    default:
        break;
    }

    if (!HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin))
    {
        currentMenu++;
        if (currentMenu > 4)
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
        if (tdiv > 20)
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

void outputCSV()
{
    char st[10];
    char s1[10];
    uint8_t buffer[30] = "";
    
    setCursor(12, 5);
    setTextColor(BLACK, WHITE);
    printString("Sending data");
    flushDisplay();

    sprintf(buffer, "\033[2J\033[H\033[3J");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Model,TekscopeSW\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Label,CH1\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Waveform Type,ANALOG\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Horizontal Units,s\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    float sampPer = tdiv / 16.0;
    printFloat(sampPer, 2, st);

    sprintf(buffer, "Sample Interval,%sE-06\n\r", st);
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Record Length,%d\n\r", BUFFER_LEN);
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Zero Index,%d\n\r", trigPoint);
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "Vertical Units,V\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, ",\n\rLabels,\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    sprintf(buffer, "TIME,CH1\n\r");
    CDC_Transmit_FS(buffer, strlen(buffer));
    HAL_Delay(5);

    for (int i = 0; i < BUFFER_LEN; i++)
    {
        float voltage = adcToVoltage(adcBuf[i]);
        printFloat(voltage, 1, st);
        printFloat((float)i * sampPer, 3, s1);
        sprintf(buffer, "%sE-06,%s\n\r", s1, st);
        CDC_Transmit_FS(buffer, strlen(buffer));
        HAL_Delay(5);
    }
}

void usbMenu()
{
    setTextColor(BLACK, WHITE);
    setCursor(100, 1);
    printString("USB");
    setTextColor(WHITE, BLACK);
    setCursor(100, 10);
    printString("Send");
    if (!HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin))
    {
        outputCSV();
        HAL_Delay(1000);
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
