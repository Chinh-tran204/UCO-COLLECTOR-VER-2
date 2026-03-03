/*
 * lcd_12864.h
 *
 *  Created on: Oct 7, 2025
 *      Author: chinhtranhuu
 */


/*
 * Use for lcd 12864 screen
 * I/O connection:
 * MOSI - R/W
 * CLK - E
 * CS - RST
 */

#ifndef INC_LCD_12864_H_
#define INC_LCD_12864_H_

/* Include outer header */
#include "main.h"
#include "u8g2.h"
#include "string.h"


/* Extern variable */
//SPI handle
extern SPI_HandleTypeDef hspi2;


// Define value
#define LOGO_W 64
#define LOGO_H 64

/* LCD 12894 */
void Init_12864(void);
void lcd_logo_12864(void);
/* External function to use outside the lib */
#endif /* INC_LCD_12864_H_ */
