/*
 * UI.h
 *
 *  Created on: Oct 7, 2025
 *      Author: chinhtranhuu
 */

#ifndef INC_UI_H_
#define INC_UI_H_
// include main.h for extern variable
#include "main.h"
/* Include lcd library
 * modify base on your need
 * Only use one version in the main code for save
 * */
// LCD 1602 model
#include "lcd_16XX.h"
#define rows 2
// LCD 12864 model
#include "lcd_12864.h"
extern u8g2_t u8g2;
void UI_16XX(SystemState state);
void UI_12864(SystemState state);
void UI_clear(void);
#endif /* INC_UI_H_ */
