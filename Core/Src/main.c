/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "Ultrasonic.h"
#include "SIMCOM.h"
#include <math.h>
#include "RTC.h"
#include "UI.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* 
* Constant for Volume calculation
* Change base on the specify tank
*/
float  MAX_HEIGHT = 75.2f;
float MIN_HEIGHT = 20.0f;
float AREA = 38.2f;
#define UNLOCK_TIME 3000	//ms
const int type_screen = 128;
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

// const char* data_Post = "{\"volume\": 54,\"battery\": 39}";		//test sim//
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim1;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
/* 
* LOG Channel define variable
* Use for debuging and information exstract from machine if needed */
//LOG buffer for comunication and debuging
uint8_t LOG_buffer[UART_RX_BUFFER_SIZE];
// LOG Flag for check data valid
volatile bool LOG_dataValid = false;
char log_msg[LOG_SIZE];

/* RTC variable for initialization and flag */
RTC_AlarmTypeDef sAlarm = {0};
RTC_TimeTypeDef sTime = {0};
/* 
* API related variable:
* for sim function and sending only
*/
//SIM buffer for post action
char data_PostSimCom[100];
// For testing only
const char* id = "dnMxMDpJakl3Tmpjd09EWTFMV0kzTUdVdE5ESXpPQzFoWW1VNQ==";		//vs10
//Data herder
char data_TransmitHeader[100];
static SIMCOM_Error result = EMPTY;
static uint8_t runtime = 0;
char result_str[100];
/* SIMCom answer */


/* 
* Logic related flag:
* for system state and operation
*/
// Unlock authorization flag 
volatile bool unlockAuth = false;
volatile bool alarmFlag = false;
bool EXTI_FLAG = false;			//true is allow to interupt | false is not allow

/*
* Value variable, changeable
*/
// Battery percentage
float battery = 0.0f;
// Volume in liter
float volume = 0.0f;

/* 
* Unlock waiting buffer
* Add in when detect user press to turn on the machine 
*/
static uint32_t unlockTime = 0;

// Time out variable
static volatile uint32_t timeout = 0;
static volatile bool timeFlag = false;
//display variable


/* Syteam running state variable */
static SystemState state = INIT;		//POWER_DOWN
static SystemState prevState = INIT;		//POWER_DOWN
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM1_Init(void);
static void MX_I2C1_Init(void);
static void MX_RTC_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_ADC2_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* Delay */
void delay_ms(uint32_t delayTime){
	uint32_t startTime = HAL_GetTick();
	while((HAL_GetTick() - startTime <= delayTime)){}
}

void buzzer(uint32_t time){
	uint32_t startTime = HAL_GetTick();
	while((HAL_GetTick() - startTime <= time)){
		HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, 1);
	}
	HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, 0);
}

static bool timeOut(uint32_t time){
	//reset timeout when step change
	//reset timeout when inactive
	if(!timeFlag){
		timeout = HAL_GetTick()+time;
		timeFlag = true;
	}
	if(HAL_GetTick() >= timeout){
		timeFlag = false;
		return true;
	} else{
		return false;
	}
}

/*LOG CHANNEL use for command sending and logging*/
/* LOG("HEADER", "DATA");*/
HAL_StatusTypeDef LOG(char* header, char *cmd){
	memset(log_msg,'\0',UART_RX_BUFFER_SIZE);
	strcpy(log_msg, header);
	strcat(log_msg, cmd);
	strcat(log_msg, "\n"); 																//TODO change to "#"
	HAL_UART_Transmit(&huart1, (uint8_t*)log_msg, strlen(log_msg), 50);
	return HAL_OK;
}

/* Battery measurement */
float Read_Voltage(void){
	HAL_ADC_PollForConversion(&hadc1,1000);
	uint16_t readValue = HAL_ADC_GetValue(&hadc1);
	return ((float)readValue * (11.67/2065)) + 0.36; //4095*16.5;
}

/* test resistor measurement */
float Read_Resistance(void){
	HAL_ADC_PollForConversion(&hadc2,1000);
	uint16_t readValue = HAL_ADC_GetValue(&hadc2);
//	return (float)readValue * (146.0f/1993.0f);

	return (float)readValue;
}
/* UI Handle */
//128 for the 128x64 LCD
//16 for the 16x2 LCD
void UI_Handle(int type){
	if(type == 128){
		UI_12864(state);
	} else if(type == 16){
		UI_16XX(state);
	}
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  MX_ADC2_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  //Ultrasonic Timer
  HAL_TIM_Base_Start(&htim1);
  /* ADC Initialize */
  HAL_ADC_Start(&hadc1);
  HAL_ADC_Start(&hadc2);
  /* ON/OFF peripheral */
  HAL_GPIO_WritePin(BUT_LED_GPIO_Port, BUT_LED_Pin, 0);
  HAL_GPIO_WritePin(DEVICE_GPIO_Port, DEVICE_Pin, 0);
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 0);
  /* Control peripheral */
  HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, 0);
  HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, 0);
  /*init UART commute Receive*/
  HAL_UARTEx_ReceiveToIdle_IT(&huart2, SIM_buffer, UART_RX_BUFFER_SIZE);
  UI_12864(INIT);
  UI_16XX(INIT);
  /* Init delay for sim wakeup */
  delay_ms(1000);
  /* Checking LOG */
  LOG("[DEBUG]","STM32 OK!");
  buzzer(500);
  delay_ms(500);
  buzzer(500);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
	result = EMPTY;
	runtime = 0;
    switch(state) {
      case INIT:
    	  LOG("[DEBUG]","INIT");
    	  /* Control peripheral */
    	  HAL_GPIO_WritePin(BUT_LED_GPIO_Port, BUT_LED_Pin, 1);
    	  HAL_GPIO_WritePin(DEVICE_GPIO_Port, DEVICE_Pin, 1);
    	  HAL_GPIO_WritePin(DEVICE_LCD_GPIO_Port, DEVICE_LCD_Pin, 0);
    	  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 1);
    	  /* init and test all the device */																// TODO: Thêm phần quét các thiết bị để xem có thiết bị nào lỗi không, rồi báo lên màn hình.
    	  UI_Handle(type_screen);
    	  state = WELCOME;
    	  break;
      case WELCOME:
    	  LOG("[DEBUG]","WELCOME");
    	  UI_Handle(type_screen);
    	  delay_ms(1500);
    	  state = MEASURE;
    	  break;
      case MEASURE:
    	  LOG("[DEBUG]","MEASURE");
    	  if(timeOut(5000)){
    		  state = POWER_DOWN;
    		  break;
    	  }
    	  /* Battery measurement */
//        battery = Read_Voltage();
    	  /* Volume measurement */
    	  float distance = Ultra_ReadDistance()/10.0f; //cm
    	  /* Low and High limit consideration */
    	  if(distance >= MAX_HEIGHT) {
    		  distance = MAX_HEIGHT;
    	  } else if(distance < 0.0f) {
    		  distance = 0.0f;
    	  } else {
    		  distance = distance;
    	  }

    	  volume = ((MAX_HEIGHT - distance)/10.0f * AREA);	//(lit)
//    	  volume = Read_Resistance();
//    	  volume = distance;

    	  if(alarmFlag){
    		  alarmFlag = false;
    		  state = UPDATE;
    	  } else if (distance <= MIN_HEIGHT){
    		  state = DISPLAY_FUL;
    	  } else state = DISPLAY;
    	  break;
      case DISPLAY_FUL:
    	  LOG("[DEBUG]", "FULL");
    	  UI_Handle(type_screen);
    	  state = UNLOCK_TRIAL;
    	  // Button pressing handle
    	  while(!HAL_GPIO_ReadPin(BUT_IN_GPIO_Port, BUT_IN_Pin)){}
    	  break;
      case DISPLAY:
    	  LOG("[DEBUG]","DISPLAY");
    	  UI_Handle(type_screen);
    	  state = UNLOCK_TRIAL;
    	  // Button pressing handle
    	  while(!HAL_GPIO_ReadPin(BUT_IN_GPIO_Port, BUT_IN_Pin)){}
    	  break;
      case UNLOCK_TRIAL:
    	  LOG("[DEBUG]","UNLOCK_TRIAL");
    	  if(timeOut(5000)){
    		  state = UPDATE;		//change the state if nothing happen
    		  break;
    	  }
    	  // Scan to see if the user want to unlock
    	  if(!HAL_GPIO_ReadPin(BUT_IN_GPIO_Port, BUT_IN_Pin)){
    		  buzzer(100);			//buzzer and debounce
    		  if(!HAL_GPIO_ReadPin(BUT_IN_GPIO_Port, BUT_IN_Pin)){
    			  unlockTime = HAL_GetTick() + UNLOCK_TIME;		//set the unlock time
    			  UI_clear();
    			  UI_Handle(type_screen);
    			  state = WAIT;
    			  break;
    		  }
    	  }
    	  break;
      case WAIT:
    	  LOG("[DEBUG]","WAIT");
    	  // Check to see if the user hold the button
    	  if(!HAL_GPIO_ReadPin(BUT_IN_GPIO_Port, BUT_IN_Pin)){
          // If user hold the button ecceed the require time, then unlock
    		  if(HAL_GetTick() >= unlockTime){
    			  buzzer(100);
    			  UI_clear();
    			  UI_Handle(type_screen);
    			  state = UNLOCK;
    			  break;
    		  }
          // User leave the button mid press or just accidentally press the button
          } else {
              state = DISPLAY; //return to data display screen and wait for next input if yes
              break;
          }
          break;
      case UNLOCK:
    	  LOG("[DEBUG]","UNLOCK");
    	  result  = SIMCom_Get(id, 5000);
    	  snprintf(result_str, sizeof(result_str), "RESULT: %d", result);
    	  LOG("[SIM]: ",result_str);
    	  if(result == DONE){
    		  if(strstr((char *)SIM_data, "true")){
    			  UI_clear();
    			  UI_Handle(type_screen);
    			  buzzer(1000);
    			  HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, 1);
    			  delay_ms(200); 		//200ms for opening the door
    			  HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, 0);
    			  state = UPDATE;
    		  } else state = REJECT; 		//REJECT
    	  } else state = FAIL;			//FAIL
    	  break;
      case UPDATE:
    	  LOG("[DEBUG]","UPDATE");
    	  UI_Handle(type_screen);
    	  buzzer(100);
    	  snprintf(data_PostSimCom, sizeof(data_PostSimCom), "{\"volume\": %.1f,\"battery\": %.1f}\r", volume, battery);
    	  result  = SIMCom_Post(data_PostSimCom, id, 5000);
    	  snprintf(result_str, sizeof(result_str), "RESULT: %d", result);
    	  LOG("[SIM]: ",result_str);
    	  if(result == DONE){
    		  state = POWER_DOWN;
    	  } else {
    		  state = FAIL;
    	  }
    	  break;
      case FAIL:
    	  LOG("[DEBUG]","FAIL");
		  UI_Handle(type_screen);		//state
		  buzzer(200);
		  delay_ms(200);
		  buzzer(200);
		  delay_ms(200);
		  buzzer(200);
		  delay_ms(200);
		  state = POWER_DOWN;
		  break;
	  case REJECT:
		  LOG("[DEBUG]","REJECT");
		  UI_Handle(type_screen);
		  buzzer(200);
		  delay_ms(200);
		  buzzer(200);
		  delay_ms(200);
		  buzzer(200);
		  delay_ms(200);
		  delay_ms(2000);
		  state = POWER_DOWN;
		  break;
      case POWER_DOWN:
    	  /* SYSTEM SLEEP ENTRY POINT */
    	  /* Final log before sleep */
    	  LOG("[DEBUG]","POWER_DOWN");
    	  UI_12864(WELCOME);
    	  /* Sim sleep */
    	  do{
    		  result = SIM_Sleep(200);
    		  snprintf(result_str, sizeof(result_str), "RESULT: %d", result);
    		  LOG("[SIM]: ",result_str);
    		  runtime += 1;
    	  } while (result!=DONE && runtime < 10);
    	  delay_ms(1000);
    	  /* turn off all peripheral */
    	  HAL_GPIO_WritePin(DEVICE_GPIO_Port, DEVICE_Pin, 0);
    	  HAL_GPIO_WritePin(DEVICE_LCD_GPIO_Port, DEVICE_LCD_Pin, 1);
    	  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, 0);
    	  /* Control peripheral */
    	  HAL_GPIO_WritePin(LOCK_GPIO_Port, LOCK_Pin, 0);
    	  HAL_GPIO_WritePin(BUZZ_GPIO_Port, BUZZ_Pin, 0);

    	  HAL_GPIO_DeInit(BUZZ_GPIO_Port, BUZZ_Pin);
    	  HAL_GPIO_DeInit(BUT_LED_GPIO_Port, BUT_LED_Pin);
    	  HAL_GPIO_DeInit(DEVICE_GPIO_Port, DEVICE_Pin);
    	  HAL_GPIO_DeInit(DEVICE_LCD_GPIO_Port, DEVICE_LCD_Pin);
    	  HAL_GPIO_DeInit(LOCK_GPIO_Port, LOCK_Pin);
    	  HAL_GPIO_DeInit(LED_GPIO_Port, LED_Pin);
    	  HAL_GPIO_DeInit(SPI_CS_GPIO_Port, SPI_CS_Pin);
    	  HAL_GPIO_DeInit(U_Trig_GPIO_Port, U_Trig_Pin);
    	  HAL_GPIO_DeInit(U_Echo_GPIO_Port, U_Echo_Pin );
    	  /* Turn off all control peripheral */
    	  HAL_SPI_MspDeInit(&hspi2);
    	  HAL_I2C_DeInit(&hi2c1);
    	  HAL_ADC_DeInit(&hadc1);
    	  HAL_ADC_DeInit(&hadc2);
    	  /* Reset flag */
    	  LOG_dataValid = false;
    	  SIM_DataValid = false;
    	  EXTI_FLAG = true;
    	  HAL_GPIO_WritePin(BUT_LED_GPIO_Port, BUT_LED_Pin, 0);
    	  /* Go to sleep */
    	  HAL_SuspendTick();
    	  HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    	  /* SYSTEM WAKEUP ENTRY POINT */
    	  /* System boost up */
    	  HAL_ResumeTick();
    	  SystemClock_Config();
    	  MX_GPIO_Init();
    	  buzzer(500);
    	  MX_USART2_UART_Init();
    	  HAL_UARTEx_ReceiveToIdle_IT(&huart2, SIM_buffer, UART_RX_BUFFER_SIZE);
    	  /* SIM wakeup */
    	  do{
    		  result = SIM_Wakeup(200);
    		  snprintf(result_str, sizeof(result_str), "RESULT: %d", result);
    		  LOG("[SIM]: ",result_str);
    		  runtime += 1;
    	  } while (result != DONE && runtime < 10);
    	  /* Turn on all control peripheral */

    	  HAL_SPI_MspInit(&hspi2);
    	  MX_I2C1_Init();
    	  MX_ADC1_Init();
    	  MX_ADC2_Init();
    	  HAL_ADC_Start(&hadc1);
    	  HAL_ADC_Start(&hadc2);
    	  LOG("[DEBUG]","WAKING UP");
    	  break;
      default:
    	  break;
    }
    // Scan state to clear the screen
    if(prevState != state){
    	//clear the screen when state change
    	if(state != UNLOCK_TRIAL && state != WAIT && state != UNLOCK) UI_clear();
    		prevState = state;
    		timeFlag = false;
    	}
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  	}
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC|RCC_PERIPHCLK_ADC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
  hadc2.Instance = ADC2;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc2.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};
  RTC_AlarmTypeDef sAlarm = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 1;
  DateToUpdate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable the Alarm A
  */
  sAlarm.AlarmTime.Hours = 0;
  sAlarm.AlarmTime.Minutes = 5;
  sAlarm.AlarmTime.Seconds = 0;
  sAlarm.Alarm = RTC_ALARM_A;
  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 72-1;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535-1;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LOCK_Pin|BUT_LED_Pin|BUZZ_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SPI_CS_Pin|DEVICE_Pin|U_Trig_Pin|LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(DEVICE_LCD_GPIO_Port, DEVICE_LCD_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : BUT_IN_Pin */
  GPIO_InitStruct.Pin = BUT_IN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BUT_IN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LOCK_Pin BUT_LED_Pin BUZZ_Pin */
  GPIO_InitStruct.Pin = LOCK_Pin|BUT_LED_Pin|BUZZ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI_CS_Pin */
  GPIO_InitStruct.Pin = SPI_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(SPI_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DEVICE_LCD_Pin */
  GPIO_InitStruct.Pin = DEVICE_LCD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DEVICE_LCD_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : DEVICE_Pin */
  GPIO_InitStruct.Pin = DEVICE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DEVICE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : U_Echo_Pin */
  GPIO_InitStruct.Pin = U_Echo_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(U_Echo_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : U_Trig_Pin LED_Pin */
  GPIO_InitStruct.Pin = U_Trig_Pin|LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//uart call back handle

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
	if(huart == &huart2){
		SIM_DataValid = true;
		HAL_UARTEx_ReceiveToIdle_IT(&huart2, SIM_buffer, UART_RX_BUFFER_SIZE);
	}
}

void USB_CDC_RxHandler(uint8_t* Buf, uint32_t Len)
{
	LOG_dataValid = true;
	memcpy(LOG_buffer, Buf, Len);
}

void HAL_RTC_AlarmAEventCallback(RTC_HandleTypeDef *hrtc)
{
	alarmFlag = true;
	EXTI_FLAG = false;
	state = INIT;
	Set_RTC(0, 0, 0); //(Hour, Minute, Second)
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  //scan the pin to see which pin is triggered
	if(GPIO_Pin == BUT_IN_Pin && EXTI_FLAG)
	{
		state = INIT;
		EXTI_FLAG = false;
//		HAL_NVIC_DisableIRQ(EXTI0_IRQn);

	}
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
