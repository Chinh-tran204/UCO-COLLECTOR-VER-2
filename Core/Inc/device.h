#include "stm32f1xx_hal.h"
#include <stdint.h>

//battery measure
uint8_t batteryCap(void);
//latch open
void buzzer(uint8_t time);
//sensor measure
float distanceCm(void);

void latchOpen(void);
