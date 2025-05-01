//include all the needed file
#include <string.h>
#include "SIMCom.h"

extern UART_HandleTypeDef huart1;

//ERROR CODE
#define SUCCESS 0
#define ERR_HANDSHAKE 1
#define ERR_BUFFER_OVERFLOW 2
#define ERR_HTTP_FAIL 3
#define ERR_AT_COMMAND 4
#define ERR_NOT_AUTH 5

//variable
uint8_t rxData[500];

//cutom delay
void delay_ms(uint32_t delayTime){
  uint32_t startTime = HAL_GetTick();
  uint32_t currentTime = HAL_GetTick();
  while(currentTime - startTime <= delayTime){
    currentTime = HAL_GetTick();
  }
}

//str len measuring
size_t strlen(const char *s){
  const char *p = s;
  while (*p) ++p;
  return (size_t)(p - s);
}



////////////////////////////////////////////////SIMCom FUNCTION////////////////////////////////////////////////////////////////
//UART Transmit handle
void Transmit(char *cmd){ 
  //reset the data, clear it for clean
  memset(rxData, 0, sizeof(rxData));
  //sending the message through the UART
  HAL_UART_Transmit(&huart1, (uint8_t*)cmd, strlen(cmd), 500);
}

//UART READ
void Receive(void){
  //reset the data, clear it for clean
  memset(rxData, 0, sizeof(rxData));
  //Start UART receive
  HAL_UART_Receive(&huart1, rxData, 100, 500);
}

//Hand Shake Handle
uint8_t SIMCom_handShake(void){
  Transmit("AT\r");
  delay_ms(100);
  Receive();
  if (!strstr((char *)rxData,"OK")){
    return ERR_HANDSHAKE;
  }
  else return SUCCESS;
}

uint8_t SIMCom_post(double vol, double bat){
  //initial data
  uint8_t trials = 0;
  char data[100];
  uint8_t response[500];
  //setting data and checking handShake
  if(snprintf(data, sizeof(data),"{\"volume\": %f,\"battery\": %.2f}",vol,bat) >= sizeof(data)) return ERR_BUFFER_OVERFLOW;
  if (SIMCom_handShake()) return ERR_HANDSHAKE;
  //config SSL
  Transmit("AT+CSSLCFG=\"enableSNI\",0,1\r");
  delay_ms(100);
  Receive();
  if(!strstr((char *)rxData, "OK")) return ERR_AT_COMMAND;
  //init HTTP Service
  Transmit("AT+HTTPINIT\r");
  delay_ms(100);
  Receive();
  if(!strstr((char *)rxData,"OK")) return ERR_HTTP_FAIL;
  //setting url
  Transmit("AT+HTTPPARA=\"URL\",\"https://api.admin.bi-oil.app/vinschool-machine/\"\r");
  delay_ms(100);
  Receive();
  if(!strstr((char *)rxData, "OK")) return ERR_HTTP_FAIL;
  //setting content type application/json
  Transmit("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r");
  delay_ms(100);
  Receive();
  if(!strstr((char *)rxData, "OK")) return ERR_HTTP_FAIL;
  //sending auth code
  Transmit("AT+HTTPPARA=\"USERDATA\",\"Authorization: Basic\"\r");  //need to change to the real auth
  delay_ms(100);
  Receive();
  if(!strstr((char *)rxData, "OK")) return ERR_HTTP_FAIL;
  //trying http post
  while (trials <= 3){
    uint16_t len = strlen(data);
    char buffer[100];
    snprintf(buffer,sizeof(buffer), "AT+HTTPDATA=%d,10000\r",len);
    Transmit(buffer);
    delay_ms(200);
    Receive();
    if(!strstr((char *)rxData, "DOWNLOAD")) return ERR_HTTP_FAIL;
    //transmit data
    Transmit(data);
    delay_ms(4000); //4 seconds delay_mss time for SIMCom running
    //start http post
    Transmit("AT+HTTPACTION=1\r");
    delay_ms(4000);
    Receive();
    if(!strstr((char *)rxData, "OK")) return ERR_HTTP_FAIL;
    //reading the msg 
    Transmit("AT+HTTPREAD=0,300\r");
    delay_ms(2000);
    memset(response, 0, sizeof(response));
    HAL_UART_Receive(&huart1, response, sizeof(response) - 1, 500);
    //scan for the appropriate msg
    if(!strstr((char *)response, "{\"success\":true}")){
      Transmit("AT+HTTPTERM\r");
      return SUCCESS;
    }
    trials++;
  }
  //closing the http section
  Transmit("AT+HTTPTERM\r");
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
  if(!strstr((char *)rxData, "OK")) return ERR_AT_COMMAND;
  delay_ms(100);
  //init HTTP Service
  Transmit("AT+HTTPINIT\r");
  if(!strstr((char *)rxData,"OK")) return ERR_HTTP_FAIL;
  delay_ms(100);
  //setting the url
  Transmit("AT+HTTPPARA=\"URL\",\"https://api.admin.bi-oil.app/vinschool-machine/get-auth\"\r");
  if(!strstr((char *)rxData,"OK")) return ERR_AT_COMMAND;
  delay_ms(100);
  //setting authorization
  Transmit("AT+HTTPPARA=\"USERDATA\",\"Authorization: Basic\"\r");  //need to change to the real auth
  if(!strstr((char *)rxData, "OK")) return ERR_AT_COMMAND;
  delay_ms(200);
  //trying http method
  while (trials <= 3){
  //Start the http get method
    Transmit("AT+HTTPACTION=0\r");
    delay_ms(4000);
    Receive();
    if(!strstr((char *)rxData, "OK")) return ERR_AT_COMMAND;
    //reading the payload
    Transmit("AT+HTTPREAD=0,300\r");
    delay_ms(3000);
    memset(response, 0, sizeof(response));
    HAL_UART_Receive(&huart1, response, sizeof(response) - 1, 500);
    //scan for the appropriate msg
    if(!strstr((char *)response, "{\"success\":true,\"authorized\":true}")){
      //closing the HTTP section
      Transmit("AT+HTTPTERM\r");
      return SUCCESS;
    }
    trials++;
  }
  //fail cause get to here and no return
  Transmit("AT+HTTPTERM\r");
  return ERR_HTTP_FAIL;
}