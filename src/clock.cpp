/*
	Работа с отображением часов (времени, даты)
*/

#include <Arduino.h>
#include "defines.h"
#include "clock.h"
#include "ntp.h"
#include "clock_translation.h"

// вывод в строку текущего времени
const char* clockCurrentText(char *a, bool fl_12) {
	char c = millis() & 512 ?':':' ';
	if( fl_timeNotSync ) {
		sprintf_P(a, PSTR("--%c--"), c);
	} else {
		tm t = getTime();
		uint8_t hour = t.tm_hour;
		if(fl_12) {
			if(hour > 12) hour -= 12;
			if(hour == 0) hour = 12;
		}
		sprintf_P(a, PSTR("%02u%c%02u"), hour, c, t.tm_min);
		if(a[0] == '0') a[0] = ' ';
	}
	return a;
}

// вывод в строку текущей даты
const char* dateCurrentTextShort(char *a, bool tiny) {
	tm t = getTime();
	const char* week = txt_week_short[gs.show_date_short >> 1][gs.language][t.tm_wday];

	if(tiny) sprintf_P(a, PSTR("%s %2u.%02u"), week, t.tm_mday, t.tm_mon +1);
	else sprintf_P(a, PSTR("%s %u.%02u.%u"), week, t.tm_mday, t.tm_mon +1, t.tm_year +1900);
	return a;
}

// вывод в строку даты с названием месяца
const char* dateCurrentTextLong(char *a) {
	tm t = getTime();

	switch (gs.language) {
		case 0: // английский, сначала название месяца, потом число
			sprintf_P(a, F_date_l[gs.language], txt_week_l[gs.language][t.tm_wday], txt_month[gs.language][t.tm_mon], t.tm_mday, t.tm_year +1900);
			break;
		case 1: // русский и украинский, сначала число, потом месяц
		case 2:
			sprintf_P(a, F_date_l[gs.language], txt_week_l[gs.language][t.tm_wday], t.tm_mday, txt_month[gs.language][t.tm_mon], t.tm_year +1900);
			break;
	}

	return a;
}

// вывод в строку текущего времени для крошечного шрифта
const char* clockTinyText(char *a, bool fl_12) {
	char c = millis() & 512 ?':': 0x7f;
	if( fl_timeNotSync ) {
		sprintf_P(a, PSTR("--%c--%c--"), c, c);
	} else {
		tm t = getTime();
		if(fl_12) {
			uint8_t hour = t.tm_hour;
			char AmPm[] = "am";
			if(hour >= 12) {
				hour -= 12;
				strcpy(AmPm,"pm");
			}
			if(hour == 0) hour = 12;
			sprintf_P(a, PSTR("%02u%c%02u\x7f%s"), hour, c, t.tm_min, AmPm);
		} else {
			if( gs.tiny_clock == FONT_TINY ) c = ':'; // если шрифт крошечный, то не моргать
			sprintf_P(a, PSTR("%02u%c%02u%c%02u"), t.tm_hour, c, t.tm_min, c, t.tm_sec);
		}
		if(a[0] == '0') a[0] = ' ';
	}
	return a;
}

// вывод в строку текущей даты в полном варианте для крошечного шрифта
const char* dateCurrentTextTinyFull(char *a) {
	tm t = getTime();

	sprintf_P(a, PSTR(" %s  %2u\n%s %04u"), txt_week_short[gs.show_date_short >> 1][gs.language][t.tm_wday], t.tm_mday, txt_month_short[gs.language][t.tm_mon], t.tm_year +1900);
	return a;
}