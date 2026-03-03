#ifndef INC_LCD_16XX_H_
#define INC_LCD_16XX_H_

/* User include file header */
#include "main.h"
#include "stm32f1xx_hal.h"
#include <stdint.h>
#include "string.h"

/*
 * Extern variable
 * Use for lcd 1602 screen
 * I/O connection:
 * lcd_SDA - SDA
 * lcs_SCL - SCL
 * Like most I2C device setup
 */
extern I2C_HandleTypeDef hi2c1;


/* LCD 1602 */
/* Command */
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

/* Entry Mode */
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

/* Display On/Off */
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

/* Cursor Shift */
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

/* Function Set */
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

/* Backlight */
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

/* Enable Bit */
#define ENABLE 0x04

/* Read Write Bit */
#define RW 0x0

/* Register Select Bit */
#define RS 0x01

/* Device I2C Address */
#define DEVICE_ADDR     (0x27 << 1)

void Init(uint8_t rows);
void lcd_clear();
void Home();
void NoDisplay();
void Display();
void NoBlink();
void Blink();
void NoCursor();
void Cursor();
void ScrollDisplayLeft();
void ScrollDisplayRight();
void PrintLeft();
void PrintRight();
void LeftToRight();
void RightToLeft();
void ShiftIncrement();
void ShiftDecrement();
void NoBacklight();
void Backlight();
void AutoScroll();
void NoAutoScroll();
void CreateSpecialChar(uint8_t, uint8_t[]);
void PrintSpecialChar(uint8_t);
void lcd_SetCursor(uint8_t, uint8_t);
void SetBacklight(uint8_t new_val);
void LoadCustomCharacter(uint8_t char_num, uint8_t *rows);
void lcd_print(const char[]);

#endif /* LIQUIDCRYSTAL_I2C_H_ */
