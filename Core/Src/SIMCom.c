//include all the needed file
#include <stdint.h>
#include <string.h>
#include "SIMCom.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_uart.h"

extern UART_HandleTypeDef huart1;

//ERROR CODE
#define SUCCESS 0
#define ERR_HANDSHAKE 1
#define ERR_BUFFER_OVERFLOW 2
#define ERR_HTTP_FAIL 3
#define ERR_AT_COMMAND 4
#define ERR_NOT_AUTH 5
#define BUFFER_TIME 100
#define HTTP_ACTION_TIME 5000


//variable
static uint16_t receiving = 0;
static uint8_t response[500];
static uint8_t rxData[500];

//cutom delay
void delay_ms(uint32_t delayTime){
  uint32_t startTime = HAL_GetTick();
  while((HAL_GetTick() - startTime <= delayTime) && !receiving){}
}

//str len measuring
size_t strlen(const char *s){
  const char *p = s;
  while (*p) ++p;
  return (size_t)(p - s);
}

//Calling the function again waiting for UART info
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size){
  //copy the array for better handle
  memcpy(response, rxData, Size);
  response[Size] = '\0';      // Null-terminate, safe fro string operation  
  //Usign size as confirm bit
  receiving = Size;
  //reInit the IRQ UART Receive again
  HAL_UARTEx_ReceiveToIdle_IT(&huart1, rxData, 500);
}

////////////////////////////////////////////////SIMCom FUNCTION////////////////////////////////////////////////////////////////
//UART Transmit handle
void Transmit(char *cmd){
  //clear check bit
  receiving = 0;
  //sending the message through the UART
  memset(rxData, 0, sizeof(rxData)); // Clear rxData
  memset(response, 0, sizeof(response));  //clearing response buffer before call back
  HAL_UART_Transmit(&huart1, (uint8_t*)cmd, strlen(cmd), 500);
}


void SIMCom_init(void){
  //enable UART receive interrupt
  HAL_UARTEx_ReceiveToIdle_IT(&huart1, rxData, 500);
}


//Hand Shake Handle
uint8_t SIMCom_handShake(void){
  Transmit("AT\r");
  delay_ms(BUFFER_TIME);
  if (!receiving) return ERR_AT_COMMAND;
  //start the IRQ Receive to wait for message
  if (!strstr((char *)response,"OK")){
    return ERR_HANDSHAKE;
  }
  else return SUCCESS;
}

uint8_t SIMCom_post(float vol, float bat){
  //initial data
  uint8_t trials = 0;
  char data[100];
  //setting data and checking handShake
  if(snprintf(data, sizeof(data),"{\"volume\": %.1f,\"battery\": %.1f}",vol,bat) >= sizeof(data)) return ERR_BUFFER_OVERFLOW;
  if (SIMCom_handShake()) return ERR_HANDSHAKE;
  //config SSL
  Transmit("AT+CSSLCFG=\"enableSNI\",0,1\r");
  delay_ms(BUFFER_TIME);
  if(!strstr((char *)response, "OK") || !receiving) return ERR_AT_COMMAND;
  //init HTTP Service
  Transmit("AT+HTTPINIT\r");
  delay_ms(BUFFER_TIME);
  if(!strstr((char *)response,"OK") || !receiving) return ERR_AT_COMMAND;
  //setting url
  Transmit("AT+HTTPPARA=\"URL\",\"https://api.admin.bi-oil.app/vinschool-machine/\"\r");
  delay_ms(BUFFER_TIME);
  if(!strstr((char *)response, "OK") || !receiving) return ERR_AT_COMMAND;
  //setting content type application/json
  Transmit("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r");
  delay_ms(BUFFER_TIME);
  if(!strstr((char *)response, "OK") || !receiving) return ERR_AT_COMMAND;
  //sending auth code
  Transmit("AT+HTTPPARA=\"USERDATA\",\"Authorization: Basic\"\r");  //need to change to the real auth
  delay_ms(BUFFER_TIME);
  if(!strstr((char *)response, "OK") || !receiving) return ERR_AT_COMMAND;
  //trying http post
  while (trials <= 3){
    uint16_t len = strlen(data);
    char buffer[100];
    snprintf(buffer,sizeof(buffer), "AT+HTTPDATA=%d,10000\r",len);
    Transmit(buffer);
    delay_ms(BUFFER_TIME);
    if(!strstr((char *)response, "DOWNLOAD") || !receiving) return ERR_HTTP_FAIL;
    //transmit data
    Transmit(data);
    delay_ms(HTTP_ACTION_TIME); //4 seconds delay_mss time for SIMCom running
    if(!strstr((char *)response, "OK ") || !receiving) return ERR_HTTP_FAIL;
    //start http post
    Transmit("AT+HTTPACTION=1\r");
    delay_ms(HTTP_ACTION_TIME);
    if(!strstr((char *)response, "OK") || !receiving) return ERR_HTTP_FAIL;
    //reading the msg 
    Transmit("AT+HTTPREAD=0,300\r");
    delay_ms(HTTP_ACTION_TIME);
    //scan for the appropriate msg
    if(strstr((char *)response, "{\"success\":true}")){
      Transmit("AT+HTTPTERM\r");
      if (!receiving) return ERR_AT_COMMAND;
      __HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);
      return SUCCESS;
    }
    trials++;
  }
  //closing the http section
  Transmit("AT+HTTPTERM\r");
  delay_ms(BUFFER_TIME);
  if (!receiving) return ERR_AT_COMMAND;
  __HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);
  return ERR_HTTP_FAIL;
}

uint8_t SIMCom_Get(void){
  //init data
  uint8_t trials = 0;
  uint8_t response[500];
  //init Hand Shake
  if (SIMCom_handShake()) return ERR_HANDSHAKE;
  //enable SNI 
  Transmit("AT+CSSLCFG=\"enableSNI\",0,1\r");
  delay_ms(BUFFER_TIME);
  if(!strstr((char *)response, "OK") || !receiving) return ERR_AT_COMMAND;
  //init HTTP Service
  Transmit("AT+HTTPINIT\r");
  delay_ms(BUFFER_TIME);
  if(!strstr((char *)response,"OK") || !receiving) return ERR_HTTP_FAIL;
  //setting the url
  Transmit("AT+HTTPPARA=\"URL\",\"https://api.admin.bi-oil.app/vinschool-machine/get-auth\"\r");
  delay_ms(BUFFER_TIME);
  if(!strstr((char *)response,"OK") || !receiving) return ERR_AT_COMMAND;
  //setting authorization
  Transmit("AT+HTTPPARA=\"USERDATA\",\"Authorization: Basic\"\r");  //need to change to the real auth
  delay_ms(BUFFER_TIME);
  if(!strstr((char *)response, "OK") || !receiving) return ERR_AT_COMMAND;
  //trying http method
  while (trials <= 3){
  //Start the http get method
    Transmit("AT+HTTPACTION=0\r");
    delay_ms(HTTP_ACTION_TIME);
    if(!strstr((char *)response, "OK") || !receiving) return ERR_HTTP_FAIL;
    //reading the payload
    Transmit("AT+HTTPREAD=0,300\r");
    delay_ms(HTTP_ACTION_TIME);
    if(!receiving) return ERR_AT_COMMAND;
    //scan for the appropriate msg
    if(!strstr((char *)response, "{\"success\":true,\"authorized\":true}")){
      //closing the HTTP section
      Transmit("AT+HTTPTERM\r");
      return SUCCESS; 
    }
    if(!strstr((char *)response, "{\"success\":false,\"authorized\":false}")){
      //closing the HTTP section
      Transmit("AT+HTTPTERM\r");
      return ERR_NOT_AUTH;
    }
    trials++;
  }
  //fail cause get to here and no return
  Transmit("AT+HTTPTERM\r");
  return ERR_HTTP_FAIL;
}