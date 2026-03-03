#include "main.h"
#include "Ultrasonic.h"
extern TIM_HandleTypeDef htim1;

uint32_t start_time;
uint32_t end_time;
uint32_t duration;
uint32_t timeout_start;
uint32_t timeout_start_1;

#define MAX_TIMEOUT_US 20000

void delay_us(uint32_t us)
{
	__HAL_TIM_SET_COUNTER(&htim1, 0);
	while (__HAL_TIM_GET_COUNTER(&htim1) < us);
}

float Ultra_ReadDistance(void){
	//TRIG
	HAL_GPIO_WritePin(U_Trig_GPIO_Port, U_Trig_Pin, GPIO_PIN_RESET);
	delay_us(2);
	HAL_GPIO_WritePin(U_Trig_GPIO_Port, U_Trig_Pin, GPIO_PIN_SET);
	delay_us(10);
	HAL_GPIO_WritePin(U_Trig_GPIO_Port, U_Trig_Pin, GPIO_PIN_RESET);

	//Wait Echo to high
	timeout_start = __HAL_TIM_GET_COUNTER(&htim1);
	while (HAL_GPIO_ReadPin(U_Echo_GPIO_Port, U_Echo_Pin) == GPIO_PIN_RESET){
		if (__HAL_TIM_GET_COUNTER(&htim1) - timeout_start > MAX_TIMEOUT_US) {
			return -1.0f;
		}
	}

	//Time start
	start_time = __HAL_TIM_GET_COUNTER(&htim1);

	//Wait Echo to low
	timeout_start_1 = __HAL_TIM_GET_COUNTER(&htim1);
	while (HAL_GPIO_ReadPin(U_Echo_GPIO_Port, U_Echo_Pin) == GPIO_PIN_SET){
		if (__HAL_TIM_GET_COUNTER(&htim1) - timeout_start_1 > MAX_TIMEOUT_US) {
			return -1.0f;
		}
	}

	//Time end
	end_time = __HAL_TIM_GET_COUNTER(&htim1);

	//Calculate duration
	duration = end_time - start_time;

	//Calculate ditance
	return ((float)duration/ 5.80f);
}
