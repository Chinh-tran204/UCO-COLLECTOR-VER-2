/*
 * UI.c
 *
 *  Created on: Oct 7, 2025
 *      Author: chinhtranhuu
 */

/*
 * Use for display or modify the content on the lcd only and nothing more
 */

/* Include header */
#include "UI.h"

#define lang 1			//1 for vietnam and 2 for english

/* for 1602 lcd screen */
const char* vietnam_ver_1602[] = {
		"XIN CHAO",
		"!!!DAY!!!",
		"KL: %.1f KG",
		"TT: %.1f L",
		"XAC NHAN...",
		"DANG THU MO CUA!",
		"GIU NUT DE MO!",
		"CUA DA MO...",
		"DANG CAP NHAT!",
		"!!!LOI!!!",
		"YEU CAU THAT BAI",
		"XIN THU LAI!"
};
const char* english_ver_1602[] = {
		"WELCOME!",
		"!!FULL!!",
		"WT: %.1f KG",
		"VL: %.1f L",
		"CONFIRM...",
		"TRY TO OPEN DOOR",
		"HOLD TO UNLOCK",
		"OPENING DOOR",
		"UPDATING DATA!",
		"!!ERROR!!",
		"REQUEST AUT FAIL",
		"TRY AGAIN!.."
};

/* For 12864 lcd screen */
const char* vietnam_ver_12864[] = {
		"!!!DAY!!!",
		"KHOI LUONG DAU",
		"THE TICH DAU",
		"XAC NHAN...",
		"<DANG THU MO CUA>",
		"THU MO CUA",
		"<> Giu nut de mo <>",
		"DANG MO CUA",
		"<> Chu y cua mo <>",
		"CAP NHAT...",
		"!ERROR!",
		"THAT BAI",
		"<> Chua thong qua <>"
};
const char* english_ver_12864[] = {
		"!!FULL!!",
		"UC-OIL WEIGHTS",
		"UC-OIL VOLUME",
		"CONFIRM!...",
		"<REQUESTING AUTH>",
		"OPEN TRIAL",
		"<> Hold to unlock <>",
		"UNLOCK DOOR",
		"<> Door is open <>",
		"UPDATING...",
		"!ERROR!",
		"!FAILED!",
		"<> Not authorized <>"
};



const char* lang_selector(int type, int index){
	if (type == 16){
		if (lang == 1){
			return vietnam_ver_1602[index];
		} else if (lang == 2){
			return english_ver_1602[index];
		}
	} else {
		if (lang == 1){
			return vietnam_ver_12864[index];
		} else if (lang == 2){
			return english_ver_12864[index];
		}
	}
};

char displayContent[20];
/* For display handle on the 1602 lcd screen and other */
void UI_16XX(SystemState state){
	switch(state){
		case INIT:
			Init(rows);
			lcd_clear();
			break;
		case WELCOME:
			lcd_SetCursor(2,0);
			lcd_print("ECO OIL COOP");
			lcd_SetCursor(4, 1);
			lcd_print(lang_selector(16,0));
			break;
		case DISPLAY_FUL:
			lcd_SetCursor(4,0);
			lcd_print(lang_selector(16,1));
			break;
		case DISPLAY:
        	lcd_SetCursor(0,0);
        	snprintf(displayContent, sizeof(displayContent), lang_selector(16,2), volume*0.9f);		//%.1f
            lcd_print(displayContent);
            lcd_SetCursor(0,1);
            snprintf(displayContent, sizeof(displayContent), lang_selector(16,3), volume);			//%.1f
			lcd_print(displayContent);
			break;
		case WAIT:
	     	lcd_SetCursor(3,0);
			lcd_print(lang_selector(16,4));
			lcd_SetCursor(0,1);
			lcd_print(lang_selector(16,5));
			break;
		case UNLOCK_TRIAL:
			lcd_SetCursor(1,0);
			lcd_print(lang_selector(16,6));
			break;
		case UNLOCK:
        	lcd_SetCursor(2,0);
            lcd_print(lang_selector(16,7));
            break;

		case UPDATE:
        	lcd_SetCursor(1,0);
            lcd_print(lang_selector(16,8));
        	break;
		case FAIL:
        	lcd_SetCursor(4,0);
            lcd_print(lang_selector(16,9));
        	break;
		case REJECT:
        	lcd_SetCursor(0,0);
            lcd_print(lang_selector(16,10));
            lcd_SetCursor(2,1);
            lcd_print(lang_selector(16,11));
        	break;
		default:
			break;
	}
}

/* For display handle on the 12864 ST720 lcd only and nothing more */
void UI_12864(SystemState state){
	switch(state){
		case INIT:
			Init_12864();
			break;
		case WELCOME:
			lcd_logo_12864();
			break;
		case DISPLAY_FUL:
			u8g2_SetFont(&u8g2, u8g2_font_ncenB18_tr);
			u8g2_DrawStr(&u8g2, 13,41,lang_selector(1,0));
			break;
		case DISPLAY:
			u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
			u8g2_DrawStr(&u8g2, 3,13,lang_selector(1,1));
			u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
			snprintf(displayContent, sizeof(displayContent), "%.1f KG", volume*0.9f);
			u8g2_DrawStr(&u8g2, 3,30,displayContent);
			u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
			snprintf(displayContent, sizeof(displayContent), "%.1f L", volume);
			u8g2_DrawStr(&u8g2, 3,42,lang_selector(1,2));
			u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
			u8g2_DrawStr(&u8g2, 3,58,displayContent);
			break;
		case WAIT:
			u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
			u8g2_DrawStr(&u8g2, 15,25,lang_selector(1,3));
			u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
			u8g2_DrawStr(&u8g2, 1,50,lang_selector(1,4));
			break;
		case UNLOCK_TRIAL:
			u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
			u8g2_DrawStr(&u8g2, 12,25,lang_selector(1,5));
			u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
			u8g2_DrawStr(&u8g2, 5,50,lang_selector(1,6));
			break;
		case UNLOCK:
			u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
			u8g2_DrawStr(&u8g2, 10,25,lang_selector(1,7));
			u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
			u8g2_DrawStr(&u8g2, 9,50,lang_selector(1,8));
			break;
		case UPDATE:
			u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
			u8g2_DrawStr(&u8g2, 0,38,lang_selector(1,9));
			break;
		case FAIL:
			u8g2_SetFont(&u8g2, u8g2_font_ncenB18_tr);
			u8g2_DrawStr(&u8g2, 6,41,lang_selector(1,10));
			break;
		case REJECT:
			u8g2_SetFont(&u8g2, u8g2_font_ncenB14_tr);
			u8g2_DrawStr(&u8g2, 15,25,lang_selector(1,11));
			u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
			u8g2_DrawStr(&u8g2, 6,50,lang_selector(1,12));
			break;
		default:
			break;
	}
	//update the present content buffer
	u8g2_UpdateDisplay(&u8g2);
}


void UI_clear(void){
	//clear the display
	lcd_clear();
	u8g2_ClearBuffer(&u8g2);
}
