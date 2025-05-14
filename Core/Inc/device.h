#include "stm32f1xx_hal.h"
#include <stdint.h>

//battery measure
float batteryCap(void);
//latch open
void buzzer(uint16_t time);
//sensor measure
float distanceCm(void);

void latchOpen(void);
