#include "main.h"

#define CLOCKTIM_PRESC 0
#define SYSCLK_FREQ 72000000
#define BUFFER_LEN 200

#define UPPER_VOLTAGE (atten * 3.3)
#define LOWER_VOLTAGE (atten * -3.3)

#define RISING 1
#define FALLING 0

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern I2C_HandleTypeDef hi2c1;

extern TIM_HandleTypeDef htim3;

uint16_t adcBuf[BUFFER_LEN]; // this is where we'll store data
volatile uint8_t finishedConversion = 0;

int atten = 1;
float vdiv = 2;        // Volts per division
float trigVoltage = 0; // trigger level
uint8_t trig = RISING;

float tdiv = 100; // microseconds
uint32_t measuredFreq;
uint32_t sampRate;

int currentInfo = 1;

void scopeInit()
{
    ssd1306Begin(SSD1306_SWITCHCAPVCC, &hi2c1, 128, 64);

    sampRate = (16000 * 1000) / tdiv;
    setTimerFreq(sampRate, htim3);

    HAL_TIM_Base_Start(&htim3);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcBuf, BUFFER_LEN);
}

void scopeLoop()
{
    if (finishedConversion) // We finished aquiring one buffer
    {

        /// Triggering logic
        int trigPoint = findTrigger();
        if (trigPoint != -1)
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 0);
        else
            trigPoint = 0;

        clearDisplay();
        drawGraticule(64, 96, 16, 4, 4);

        float maxVoltage = LOWER_VOLTAGE, minVoltage = UPPER_VOLTAGE;
        for (int i = 0; i <= 94; i++)
        {
            float voltage1 = atten * 2 * (((3.3 * adcBuf[i + trigPoint]) / 4096.0) - 1.65);
            float voltage2 = atten * 2 * (((3.3 * adcBuf[i + trigPoint + 1]) / 4096.0) - 1.65);
            drawLine(i, 31 - (voltage1 * 16 / vdiv), i + 1, 31 - (voltage2 * 16 / vdiv), WHITE);

            if (voltage2 > maxVoltage)
                maxVoltage = voltage2;
            if (voltage2 < minVoltage)
                minVoltage = voltage2;
        }

        // UI
        switch (currentInfo)
        {
        case 1: // Print voltage measurements
            voltageInfo(&minVoltage, &maxVoltage);
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
            currentInfo++;
            if (currentInfo > 3)
                currentInfo = 1;
            HAL_Delay(100);
        }

        flushDisplay();

        // Start again
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1);
        finishedConversion = 0;
        HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcBuf, BUFFER_LEN);
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

void voltageInfo(float *miV, float *maV)
{
    char st[15];
    printFloat(*miV, 1, st);
    setTextColor(BLACK, WHITE);
    setCursor(100, 1);
    printString("Min:");
    setTextColor(WHITE, BLACK);
    setCursor(100, 10);
    printf("%s\n", st);

    printFloat(*maV, 1, st);
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
    printFloat(*maV - *miV, 1, st);
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
        else if (sel == 3) //atten
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
         else if (sel == 3) //atten
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
        setTimerFreq(sampRate, htim3);

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
        setTimerFreq(sampRate, htim3);

        HAL_Delay(100);
    }
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

void drawGraticule(int hei, int wit, int pix, int divx, int divy)
{
    for (int i = 0; i <= wit; i += pix)
        dottedVLine(i, 0, hei);

    for (int i = 0; i <= hei; i += pix)
        dottedHLine(0, i, wit);
}

void setTimerFreq(uint32_t freq, TIM_HandleTypeDef tim)
{
    uint16_t arr = (SYSCLK_FREQ / ((CLOCKTIM_PRESC + 1) * freq)) - 1;
    tim.Instance->ARR = arr;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    finishedConversion = 1;
}