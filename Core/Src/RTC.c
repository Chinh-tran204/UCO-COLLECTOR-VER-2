/*
 * RTC_Sleep.c
 *
 *  Created on: Sep 10, 2025
 *      Author: Lazy
 */
#include <RTC.h>
#include "main.h"
extern RTC_AlarmTypeDef sAlarm;
extern RTC_TimeTypeDef sTime;
extern RTC_HandleTypeDef hrtc;

uint8_t Set_RTC(uint8_t Hours, uint8_t Minutes, uint8_t Seconds){
	sTime.Hours = Hours;
	sTime.Minutes = Minutes;
	sTime.Seconds = Seconds;
	return HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
}


uint8_t Set_Alarm(uint8_t Hours, uint8_t Minutes, uint8_t Seconds){
	RTC_TimeTypeDef sTime = {0};

	HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);

	//Seconds
	sTime.Seconds += Seconds;
	if (sTime.Seconds >= 60) {
		int temp1 = sTime.Seconds % 60;
		sTime.Seconds = temp1;

		int temp2 = sTime.Seconds / 60;
		sTime.Minutes += temp2;
	}

	//Minutes
	sTime.Minutes += Minutes;
	if (sTime.Minutes >= 60) {
		int temp1 = sTime.Minutes % 60;
		sTime.Minutes = temp1;

		int temp2 = sTime.Minutes / 60;
		sTime.Hours += temp2;
	}

	//Hours
	sTime.Hours += Hours;
	if (sTime.Hours >= 24) {
		int temp1 = sTime.Hours % 24;
		sTime.Hours = temp1;
	}

	sAlarm.AlarmTime.Hours = sTime.Hours;
	sAlarm.AlarmTime.Minutes = sTime.Minutes;
	sAlarm.AlarmTime.Seconds = sTime.Seconds;
	return HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BIN);
}
