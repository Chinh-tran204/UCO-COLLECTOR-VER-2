/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
//#include "Ultrasonic.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define UART_RX_BUFFER_SIZE 200
#define LOG_SIZE 200
typedef enum {
	INIT,
	WELCOME,
	MEASURE,
	DISPLAY,
	DISPLAY_FUL,
	WAIT,
	UNLOCK_TRIAL,
	UNLOCK,
	POWER_DOWN,
	FAIL,
	REJECT,
	UPDATE,
	RTC_UPDATE,
	TEST,		//TEST XONG XÓA
	HANGING
} SystemState;
/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
extern uint8_t LOG_buffer[UART_RX_BUFFER_SIZE];
extern char log_msg[LOG_SIZE];
extern volatile bool LOG_dataValid;

/* Changing variable */
// Battery percentage
extern float battery;
// Volume in liter
extern float volume;
/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define BUT_IN_Pin GPIO_PIN_0
#define BUT_IN_GPIO_Port GPIOA
#define BUT_IN_EXTI_IRQn EXTI0_IRQn
#define LOCK_Pin GPIO_PIN_6
#define LOCK_GPIO_Port GPIOA
#define BUT_LED_Pin GPIO_PIN_7
#define BUT_LED_GPIO_Port GPIOA
#define SPI_CS_Pin GPIO_PIN_14
#define SPI_CS_GPIO_Port GPIOB
#define DEVICE_LCD_Pin GPIO_PIN_8
#define DEVICE_LCD_GPIO_Port GPIOA
#define BUZZ_Pin GPIO_PIN_15
#define BUZZ_GPIO_Port GPIOA
#define DEVICE_Pin GPIO_PIN_3
#define DEVICE_GPIO_Port GPIOB
#define U_Echo_Pin GPIO_PIN_4
#define U_Echo_GPIO_Port GPIOB
#define U_Trig_Pin GPIO_PIN_5
#define U_Trig_GPIO_Port GPIOB
#define LED_Pin GPIO_PIN_8
#define LED_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */
void delay_ms(uint32_t delayTime);
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
