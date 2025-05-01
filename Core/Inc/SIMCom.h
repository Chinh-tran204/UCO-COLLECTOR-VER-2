#include "stm32f1xx_hal.h"

uint8_t SIMCom_handShake(void); //handShake
uint8_t SIMCom_post(double vol, double bat); //post method
uint8_t SIMCom_Get(void); //get method