// разные функции для работы с LED матрицей

#include <Arduino.h>
// #include <FastLED.h>
#include "defines.h"
#include "leds.h"
#include "runningText.h"
#include "textTiny.h"

CRGB leds[NUM_LEDS], ledsNext[NUM_LEDS];

bool screenIsFree = true; // экран свободен (текст полностью прокручен, слайды выведены)
bool itsTinyText = false; // сейчас время выводить слайды (tiny) вместо бегущей строки

// залить все
void fillAll(CRGB color) {
	for (int i = 0; i < NUM_LEDS; i++) {
		leds[i] = color;
	}
}

// очистить всё
void clearALL() {
	FastLED.clearData();
}

/*
отрисовка буквы
pointer - указатель на первый байт буквы в массиве шрифта
startX - начальная позиция по горизонтали, 0 левый край
startY - начальная позиция по вертикали, 0 нижний край
LH - высота буквы
LW - ширина буквы
color - цвет или режим цвета
index - порядковый номер буквы в тексте, нужно для подсвечивания разными цветами
*/
void drawChar(const uint8_t* pointer, int16_t startX, int16_t startY, uint8_t LW, uint8_t LH, uint32_t color, uint16_t index) {
		uint8_t start_col = 0, finish_col = LW;
	CRGB letterColor;

	uint8_t hs = gs.hue_shift ? hue_shift: 0;
	if(color == 1) letterColor = CHSV(byte((startX << 3) + hs), 255, 255); // цвет в CHSV (прозрачность, оттенок, насыщенность, яркость) (0,0,255 - белый)
	else if(color == 3) letterColor = CHSV(byte((index << 5) + hs), 255, 255);
	else if(color == 5) letterColor = gs.show_time_col[index & 0x07];
	else letterColor = color;

	if( startX < -LW || startX > WIDTH ) return; // буква за пределами видимости, пропустить
	if( startX < 0 ) start_col = -startX;
	if( startX > WIDTH - LW ) finish_col = WIDTH - startX;

	for (int x = start_col; x < finish_col; x++) {
		// отрисовка столбца (x - горизонтальная позиция, y - вертикальная)
		if(color == 2) letterColor = CHSV(byte(((startX + x) << 3) + hs), 255, 255);
		if(color == 4) letterColor = CHSV(byte(((index * LW + x) << 2) + hs), 255, 255);
		uint8_t fontColumn = pgm_read_byte(pointer + x); // все шрифты читаются как однобайтовые, 8 точек высотой!
		for (int y = 0; y < LH; y++) {
			if( y + startY + TEXT_BASELINE >= 0 && y + startY + TEXT_BASELINE < HEIGHT ) { // отображать только видимую часть
				// в этой матрице отрисовка снизу вверх, шрифты записаны сверху вниз
				if( fontColumn & (1 << (LH - 1 - y)) )
					drawPixelXY(startX + x, startY + y + TEXT_BASELINE, led_brightness > 1 && fl_5v ? letterColor: CRGB::Red);
			}
		}
	}
}

// обмен содержимим буферов
void swapBuffers() {
	std::swap_ranges(leds, leds + NUM_LEDS, ledsNext);
}

// копирование текущего буфера
void saveBuffer() {
	memcpy(ledsNext, leds, sizeof(ledsNext));
}

// CrossFade эффект перехода - смешение
bool crossFade(bool init) {
	static int8_t step = 0;
	if (init) {
		swapBuffers(); // вернуть старую картинку
		step = 9; // переход за 8 шагов
	}
	if (!step) return false;
	if (step-- == 1) { // последний шаг - просто поставить нужный буфер, без эффектов
		swapBuffers(); // вернуть новую картинку
		return true;
	}
	for (int i=0; i<NUM_LEDS; i++) {
		leds[i] = blend(leds[i], ledsNext[i], 80); // смешение на каждом шаге 31%
	}
	return true;
}

// дополнительный эффект смены цыфр в циферблате по одной, падением вниз
bool slideDown(bool start) {
	static int8_t step = 0;
	static struct Zones {
		int8_t x, to;
	} zone[6]; // максимальное число сегметов, которые надо будет менять 12:34:56
	static uint8_t zones = 0;

	if (fl_timeNotSync) return false; // костыль для правильного отображения во время загрузки

	if (start) {
		// надо найти все блоки разделённые пробелами и шире 2х пикселей
		zones = 0;
		int t=0, c1=0, c2=0, c3=0;
		bool fl = true;
		for (int x=0; x<WIDTH; x++) {
			c1 = 0; c2 = 0;
			for (int y=0; y<HEIGHT; y++) {
				// получение двух колонок
				c1 |= (getPixColorXY(x,y) ? 1: 0) << y;
				c2 |= (getPixColorXY(x,y, ledsNext) ? 1: 0) << y;
			}
			if (c1 || c2) { // колонки не пустые
				fl &= c1 == c2; // если хотя-бы одна колока не совпала - будет false
				t++; // счётчик не пустых колонок
			} else {
				if (t>2) { // были не пустые колонки, а сейчас пустая. Значит дошли до конца знакоместа
					if (!fl) { zone[zones].x = x-t; zone[zones++].to = x; } // знакоместо отличается, занесенение в очередь
					if (zones == sizeof(zone)/sizeof(Zones)) break;
				}
				fl = true; // взведение флага отличия
				t = 0; // сброс счётчика различий
			}
			c3 += c2;
			// LOG(printf_P, PSTR("sD: x=%i, c1=%i, c2=%i, fl=%i, t=%i, zones=%i, c3=%i\n"), x, c1, c2, fl, t, zones, c3);
		}
		// проверка последней колонки, считаем, что за краем матрицы пустая колонка :)
		if (t>2 && !fl && zones < sizeof(zone)/sizeof(Zones)) { zone[zones].x = WIDTH-t; zone[zones++].to = WIDTH; }
		if (zones && c3) {
			// вернуть старую картинку на отличающихся участках
			for (int i=0; i<zones; i++)
				for(int16_t x=zone[i].x; x<zone[i].to; x++)
					for(int16_t y=0; y<HEIGHT; y++) {
						uint32_t oldP = getPixColorXY(x, y, ledsNext);
						uint32_t newP = getPixColorXY(x, y);
						drawPixelXY(x, y, newP, ledsNext);
						drawPixelXY(x, y, oldP);
					}
			step = HEIGHT; // переход за 8 шагов, высота матрицы
		}
		// LOG(printf_P, PSTR("sD final: zones=%i\n"), zones);
	}
	if (!step || start) return false;
	step--;
	// LOG(printf_P, PSTR("sD step=%i\n"), step);
	for (int i=0; i<zones; i++) {
		// LOG(printf_P, PSTR("zone=%i, x=%i, to=%i\n"), i, zone[i].x, zone[i].to);
		for(int x=zone[i].x; x<zone[i].to; x++) {
			for(int y=0; y<HEIGHT-1; y++) // сдвиг колонки вниз
				drawPixelXY(x, y, getPixColorXY(x, y+1));
			drawPixelXY(x, HEIGHT-1, getPixColorXY(x, (HEIGHT-1)-step, ledsNext)); // дорисовка одной строки текущей колонки данными из второго буфера
		}
	}
	return true;
}

// функция отрисовки точки по координатам X Y
void drawPixelXY(int8_t x, int8_t y, CRGB color, CRGB arr[]) {
	if (x < 0 || x > WIDTH - 1 || y < 0 || y > HEIGHT - 1) return;
	int thisPixel = getPixelNumber(x, y) * SEGMENTS;
	for (int i = 0; i < SEGMENTS; i++) {
		if (arr)
			arr[thisPixel + i] = color;
		else
			leds[thisPixel + i] = color;
	}
}

// функция получения цвета пикселя по его номеру
uint32_t getPixColor(int num, CRGB arr[]) {
	int thisPixel = num * SEGMENTS;
	if (thisPixel < 0 || thisPixel > NUM_LEDS - 1) return 0;
	if (arr)
		return (((uint32_t)arr[thisPixel].r << 16) | ((long)arr[thisPixel].g << 8 ) | (long)arr[thisPixel].b);
	else
		return (((uint32_t)leds[thisPixel].r << 16) | ((long)leds[thisPixel].g << 8 ) | (long)leds[thisPixel].b);
}

// функция получения цвета пикселя в матрице по его координатам
uint32_t getPixColorXY(int8_t x, int8_t y, CRGB arr[]) {
	return getPixColor(getPixelNumber(x, y) * SEGMENTS, arr);
}

// Увеличение яркости до максимума с сохранением оттенка
uint32_t maximizeBrightness(uint32_t color, uint8_t limit)  {
	uint32_t red = (color >> 16) & 255;
	uint32_t green = (color >> 8) & 255;
	uint32_t blue = color & 255;
	uint32_t max = red;
	if( green > max) max = green;
	if( blue > max) max = blue;
	uint32_t factor = ((uint32_t)limit * 256) / max;
	red = (red * factor) >> 8;
	green = (green * factor) >> 8;
	blue = (blue * factor) >> 8;
	return (red << 16) | (green << 8) | blue;
}

// **************** НАСТРОЙКА МАТРИЦЫ ****************
#if (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y

#elif (CONNECTION_ANGLE == 0 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y x

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 0)
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 1 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y x

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y (HEIGHT - y - 1)

#elif (CONNECTION_ANGLE == 2 && STRIP_DIRECTION == 3)
#define _WIDTH HEIGHT
#define THIS_X (HEIGHT - y - 1)
#define THIS_Y (WIDTH - x - 1)

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 2)
#define _WIDTH WIDTH
#define THIS_X (WIDTH - x - 1)
#define THIS_Y y

#elif (CONNECTION_ANGLE == 3 && STRIP_DIRECTION == 1)
#define _WIDTH HEIGHT
#define THIS_X y
#define THIS_Y (WIDTH - x - 1)

#else
#define _WIDTH WIDTH
#define THIS_X x
#define THIS_Y y
#pragma message "Wrong matrix parameters! Set to default"

#endif

// получить номер пикселя в ленте по координатам
uint16_t getPixelNumber(int8_t x, int8_t y) {
	if(gs.turn_display & 1) x = WIDTH - x - 1;
	if(gs.turn_display & 2) y = HEIGHT - y - 1;
	if ((THIS_Y % 2 == 0) || MATRIX_TYPE) {               // если чётная строка
		return (THIS_Y * _WIDTH + THIS_X);
	} else {                                              // если нечётная строка
		return (THIS_Y * _WIDTH + _WIDTH - THIS_X - 1);
	}
}

uint8_t led_brightness = 0;
void set_brightness(uint8_t br) {
	if(br != led_brightness) {
		FastLED.setBrightness(br);
		led_brightness = br;
	}
}

void display_setup() {
	FastLED.addLeds<WS2812B, PIN_LED, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
	set_brightness(BRIGHTNESS);
	if(DEFAULT_POWER > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, DEFAULT_POWER);
	FastLED.clear();
	saveBuffer();

	// проверка правильности ориентации
	// левый нижний угол - начало координат
	drawPixelXY(0, 0, CRGB::Red);             // левый нижний красный
	drawPixelXY(0, HEIGHT - 1, CRGB::Blue);   // левый верхний синий
	drawPixelXY(WIDTH - 1, 0, CRGB::Green);   // правый нижний зелёный

	FastLED.show();
	// delay(1000);
}

void display_tick(bool clear) {
	if(screenIsFree) {
		if (gs.anim && slideDown()) goto doit;
		// if (gs.anim && crossFade()) goto doit;
		return;
	}
	if(itsTinyText) {
		drawSlide();
	} else if(fl_tiny_clock) {
		screenIsFree = true;
		fl_tiny_clock = false;
		// if (gs.anim) crossFade(true);
		if (gs.anim) slideDown(true);
	} else {
		// очистить буфер для заполнения новыми данными, чтобы не накладывались кадры
		// только в режиме циферблата и бегущей строки
		if( clear ) FastLED.clearData();
		drawString();
	}
	doit:
	// питания 5V нет и нет разрешения на отображение - матрица не запитана и нет смысла выводить
	if(!fl_allowLEDS) return;
	// нет отдельного светодиода для индикации движения, засветить нижний левый светодиод зелёным
	#ifndef LED_MOTION
	if(fl_led_motion) drawPixelXY(WIDTH - 1, 0, CRGB::Green);
	#endif
 	// выключение матрицы в ночном режиме и нет движения - погасить все светодиоды
	if(gs.dsp_off && !fl_action_move && !fl_run_allow && !fl_led_motion) FastLED.clearData();

	unsigned long start = 0;
	// вывод часто срывается, по этому выводить кадр пока не получится. Признак неудачи - отклонение > 25%
	do {
		start = micros();
		FastLED.show();
		// if(abs((long)(micros()-start-REFRESH_TIME))>(REFRESH_TIME >> 2))
			// LOG(printf_P, PSTR("время обновления: %d (%d)\n"), micros()-start, REFRESH_TIME);
	} while (abs((long)(micros()-start-REFRESH_TIME))>(REFRESH_TIME >> 2));
}
