#include "device.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_adc.h"
#include "stm32f1xx_hal_adc_ex.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_tim.h"
#include "system_stm32f1xx.h"
#include <stdint.h>

//define the needed constant
#define SEN_TRIGGER 9
#define SEN_ECHO 8 
#define BUZZER GPIO_PIN_7
#define LATCH GPIO_PIN_1
#define BATTERY GPIO_PIN_1
#define MAIN_POWER_1 GPIO_PIN_5
#define MAIN_POWER_2 GPIO_PIN_4

//Hardware timer init - from main
extern TIM_HandleTypeDef htim1;
extern ADC_HandleTypeDef hadc1;


float rounding(float num) {
    return (float)((int)(num * 10.0f + 0.5f)) / 10.0f;
}

//delay mili-second
void delay_ms(uint32_t delayTime){
    uint32_t startTime = HAL_GetTick();
    while(HAL_GetTick() - startTime <= delayTime){}
}
//delay micro second
void delay_us(uint16_t time){
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    HAL_TIM_Base_Start(&htim1);
    while (__HAL_TIM_GET_COUNTER(&htim1) < time);
    HAL_TIM_Base_Stop(&htim1);
}

float batteryCap(void){
    uint32_t ADC_level;
    HAL_ADCEx_Calibration_Start(&hadc1);
    //start the ADC and taking in data
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 200);
    ADC_level = HAL_ADC_GetValue(&hadc1);
    float voltage = ((float)ADC_level/4095.0)*3.3;
    return rounding(voltage);
}
//latch open function
void latchOpen(void){
    HAL_GPIO_WritePin(GPIOB, LATCH, 1U);
    delay_ms(3000);    //3 second open
    HAL_GPIO_WritePin(GPIOB, LATCH, 0U);
}
//buzzer function 
void buzzer(uint16_t time){
    HAL_GPIO_WritePin(GPIOA, BUZZER, 1U);
    delay_ms(time);     //time rign base one user
    HAL_GPIO_WritePin(GPIOA, BUZZER, 0U);
}
//ultra sensor checking
float distanceCm(void){
    //do some things
    float sumUp = 0;
    uint8_t count = 0;
    uint16_t signalOn;
    uint16_t signalOff;
    uint16_t pulseDuration;
    for (uint8_t i=0; i<3; i++) {
        //sending trigger signal
        HAL_GPIO_WritePin(GPIOB, SEN_TRIGGER, 0);
        delay_us(20);
        HAL_GPIO_WritePin(GPIOB, SEN_TRIGGER, 1);
        delay_us(20);
        HAL_GPIO_WritePin(GPIOB, SEN_TRIGGER, 0);
        //capture the echo signal and measure the range
        //start the timer to capture the echo signal
        __HAL_TIM_SET_COUNTER(&htim1, 0);   //reset the timer
        HAL_TIM_Base_Start(&htim1);   //start the timer
        //capture the rising edge with time out 40ms 
        while ((HAL_GPIO_ReadPin(GPIOB, SEN_ECHO) == 0) && (__HAL_TIM_GET_COUNTER(&htim1) < 20000)){
            signalOn = __HAL_TIM_GET_COUNTER(&htim1);
        }
        //capture the falling edge with time out 80ms
        while ((HAL_GPIO_ReadPin(GPIOB, SEN_ECHO) == 1) && (__HAL_TIM_GET_COUNTER(&htim1) < 40000)){
            signalOff = __HAL_TIM_GET_COUNTER(&htim1);
        }
        //stop the timer 
        HAL_TIM_Base_Stop(&htim1);
        //calculate the distance
        //check if the timer is legit
        if (signalOn > signalOff) pulseDuration=0;
        else pulseDuration = signalOff - signalOn;
        float distance = (pulseDuration*0.0343)/2;
        if (distance < 0) distance = 0;
        else if (distance > 250) distance = 250;
        else {
            count += 1;
            sumUp += distance;
        }
    }
    if (count == 0) return 0;
    return rounding(sumUp/count);
}
