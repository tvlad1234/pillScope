#include "main.h"
#include "scope.h"
#include "ui.h"
#include "wave.h"
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim3;

uint16_t adcBuf[BUFFER_LEN]; // this is where we'll store data
volatile uint8_t finishedConversion = 0;

int atten = 1;  // Attenuation
float vdiv = 2; // Volts per division

int trigPoint;
float trigVoltage = 0; // Trigger level
uint8_t trig = RISING; // Trigger slope

float tdiv = 100; // microseconds

float maxVoltage, minVoltage;
uint32_t measuredFreq;
uint32_t sampRate;

int currentMenu = 1;

void scopeInit()
{
    ssd1306Begin(SSD1306_SWITCHCAPVCC, &hi2c1, 128, 64);
    splash();

    sampRate = (16000 * 1000) / tdiv;
    setTimerFreq(sampRate);

    HAL_TIM_Base_Start(&htim3);
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcBuf, BUFFER_LEN);
}

void scopeLoop()
{
    if (finishedConversion) // We finished aquiring one buffer
    {

        /// Triggering logic
        trigPoint = findTrigger();
        if (trigPoint != -1)
            HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 0);
        else
            trigPoint = 0;

        ui();

        // Start again
        HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1);
        finishedConversion = 0;
        HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcBuf, BUFFER_LEN);
    }
}

void setTimerFreq(uint32_t freq)
{
    uint16_t arr = (SYSCLK_FREQ / ((CLOCKTIM_PRESC + 1) * freq)) - 1;
    htim3.Instance->ARR = arr;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    finishedConversion = 1;
}