/*
	Отрисовка иконок погоды. Отдельно от других символов из-за отдельного цветового решения
	
	Спасибо Gemini и Copilot за наше счастливое детство :)
*/

#include <Arduino.h>
#include "defines.h"
#include "weather_icons.h"

// Всего 9 символов погоды. Каждый символ имеет два фрейма анимации. Некоторые имеют фрейм с наложенной картинкой цветового акцента
namespace WeatheIcons {

// 0 солнечно
const uint8_t Sunny_M[] PROGMEM = {
	0x00, 0x00, 0x42, 0x18, 0x3c, 0x7e, 0x7e, 0x3c, 0x18, 0x42, 0x00, 0x00,
	0x00, 0x81, 0x42, 0x18, 0x3c, 0x7e, 0x7e, 0x3c, 0x18, 0x42, 0x81, 0x00};

// 1 почти чисто, акцент на туче
const uint8_t MostlyClear_M[] PROGMEM = {
	0x00, 0x18, 0x3C, 0x7E, 0x7E, 0x3C, 0x18, 0x00, 0x20, 0x70, 0x78, 0x70,
	0x81, 0x18, 0x3C, 0x7E, 0x7E, 0x3C, 0x18, 0x81, 0x20, 0x70, 0x78, 0x70};

	const uint8_t MostlyClear_O[12] PROGMEM = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x70, 0x78, 0x70};

// 2 переменная облачность, акцент на солнце
const uint8_t PartlyCloudy_M[] PROGMEM = {
	0xE2, 0xC7, 0x0F, 0x27, 0x7A, 0x78, 0x7C, 0x7E, 0x7E, 0x7C, 0x7C, 0x30,
	0xF2, 0xE7, 0x4F, 0x07, 0x22, 0x78, 0x78, 0x7C, 0x7E, 0x7E, 0x7C, 0x7C};

const uint8_t PartlyCloudy_O[] PROGMEM = {0x02, 0x07, 0x0F, 0x07, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// 3 облачно
const uint8_t Cloudy_M[] PROGMEM = {
	0x10, 0x3C, 0x3C, 0x3E, 0x3C, 0x3E, 0x3F, 0x3F, 0x3F, 0x3F, 0x3E, 0x1C,
	0x18, 0x3C, 0x3E, 0x3C, 0x3E, 0x3F, 0x3F, 0x3F, 0x3F, 0x3E, 0x3E, 0x1C};

// 4 туман
const uint8_t Foggy_M[] PROGMEM = {
	0x82, 0x92, 0x91, 0x51, 0x49, 0x4A, 0x8A, 0x92, 0x91, 0x51, 0x49, 0x08,
	0x10, 0x92, 0x92, 0x89, 0x49, 0x49, 0x52, 0x92, 0x92, 0x89, 0x49, 0x41};

// 5 Небольшой дождь, акцент на зонте
const uint8_t Umbrella_M[] PROGMEM = {
	0x04, 0x01, 0x10, 0x18, 0x98, 0xFC, 0x19, 0x18, 0x10, 0x00, 0x04, 0x11,
	0x08, 0x02, 0x10, 0x19, 0x98, 0xFC, 0x18, 0x18, 0x11, 0x00, 0x08, 0x22};

const uint8_t Umbrella_O[] PROGMEM = {0x00, 0x00, 0x10, 0x18, 0x98, 0xFC, 0x18, 0x18, 0x10, 0x00, 0x00, 0x00};

// 6 Дождь / Ливень, акцент на туче
const uint8_t Rain_M[] PROGMEM = {
	0x00, 0x04, 0x8E, 0x2E, 0x0E, 0x8F, 0x2F, 0x0F, 0x8F, 0x2F, 0x0E, 0x04,
	0x00, 0x04, 0x1E, 0x4E, 0x0E, 0x1F, 0x4F, 0x0F, 0x1F, 0x4F, 0x0E, 0x04};

const uint8_t Rain_O[] PROGMEM = {0x00, 0x04, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0E, 0x04};

// 7 Снег
const uint8_t Snow_M[] PROGMEM = {
	0x00, 0x11, 0x00, 0x44, 0x00, 0x11, 0x00, 0x44, 0x00, 0x11, 0x00, 0x44,
	0x00, 0x22, 0x88, 0x00, 0x02, 0x10, 0x80, 0x00, 0x04, 0x21, 0x80, 0x08};

// 8 Гроза, тут акцент и основная картинка меняются местами, так как на фоне неподвижной тучи мелькает молния
const uint8_t Thunderstorm_M[] PROGMEM = {
	0x00, 0x04, 0x0E, 0x0E, 0x0E, 0xCF, 0x6F, 0x3F, 0x0F, 0x0F, 0x0E, 0x04,
	0x00, 0x04, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0E, 0x04};

const uint8_t Thunderstorm_O[] PROGMEM = {0x00, 0x04, 0x0E, 0x0E, 0x0E, 0x0F, 0x0F, 0x07, 0x0F, 0x0F, 0x0E, 0x04};

// сборка в один массив все картинки
const uint8_t* Icons[] PROGMEM = {
	Sunny_M, nullptr, 						// 0 Sunny
	MostlyClear_M, MostlyClear_O,			// 1 Mostly Clear
	PartlyCloudy_M, PartlyCloudy_O,			// 2 Partly Cloudy
	Cloudy_M, nullptr,						// 3 Cloudy
	Foggy_M, nullptr,						// 4 Foggy
	Umbrella_M, Umbrella_O,					// 5 Umbrella
	Rain_M, Rain_O,							// 6 Rain
	Snow_M, nullptr,						// 7 Snow
	Thunderstorm_M, Thunderstorm_O			// 8 Thunderstorm
	};

// сборка в один массив все цвета. нечётные - основной цвет, чётные (вторые в строке) - акцент, если акцента нет, то не используется.
const CRGB Colors[] = {
	CRGB::Yellow, CRGB::Yellow,					// 0 Sunny
	CRGB::Yellow, CHSV(170, 100, 220),			// 1 Mostly Clear
	CHSV(170, 100, 200), CRGB::Yellow,			// 2 Partly Cloudy
	CRGB::LightSteelBlue, CRGB::LightGrey,		// 3 Cloudy
	CHSV(0, 0, 200), CHSV(0, 0, 200),			// 4 Foggy
	CRGB::Aqua, CRGB::GreenYellow,				// 5 Umbrella
	CRGB::Aqua, CRGB::LightSteelBlue,			// 6 Rain
	CRGB::White, CRGB::White,					// 7 Snow
	CRGB::OrangeRed, CHSV(170, 100, 200)		// 8 Thunderstorm
	};

}


// Функция создает временную строку из упакованного uint32_t
const char* utf8_to_str(uint32_t packed_utf8) {
    static char temp_buf[5]; // 4 байта на UTF-8 + 1 байт на '\0'

    // Извлекаем байты (для порядка Big-Endian)
    temp_buf[0] = (char)(packed_utf8 >> 24);
    temp_buf[1] = (char)(packed_utf8 >> 16);
    temp_buf[2] = (char)(packed_utf8 >> 8);
    temp_buf[3] = (char)packed_utf8;
    temp_buf[4] = '\0';

    // Сдвигаем указатель, если символ занял меньше 4 байт
    char* start = temp_buf;
    while (*start == '\0' && start < temp_buf + 4) {
        start++;
    }
    return start;
}

// Проверяет, относится ли буква к погодным символам.
bool is_weather_symbol(uint32_t code) {
    // 1. Базовый блок "Miscellaneous Symbols" (U+2600 - U+26FF)
    // Сюда входят: Солнце (0xE29880), Облако (0xE29881), Зонт (0xE29882), Переменная облачность (0xE29B85), Гроза (0xE29888)
    if (code >= Icons::Sunny && code <= Icons::Thunderstorm)
        return (code == Icons::Sunny || code == Icons::Cloudy ||
				code == Icons::Umbrella || code == Icons::PartlyCloudy ||
				code == Icons::Thunderstorm 
			);

    // 3. Блок "Miscellaneous Symbols and Pictographs" (U+1F300 - U+1F5FF)
    // Сюда входят: Малооблачно(0xF09F8CA4), Дождь (0xF09F8CA7), Снег (0xF09F8CA8), Туман (0xF09F8CAB)
    if (code >= 0xF09F8CA4 && code <= 0xF09F8CAB)
        return (code == Icons::MostlyClear || code == Icons::Rain ||
				code == Icons::Snow || code == Icons::Foggy
			);

    return false;
}

static inline uint8_t getWeatherIconFrame() {
	return (millis() / 300) & 1;
}

// отрисовка иконки погоды
int16_t draw_weather_icon(uint32_t letter, int16_t offset, int16_t baseline) {
	// вычисление адреса символа и акцента
	uint8_t icon = 0; // Sunny
	if (letter == Icons::MostlyClear) icon = 1; 
	else if (letter == Icons::PartlyCloudy) icon = 2;
	else if (letter == Icons::Cloudy) icon = 3;
	else if (letter == Icons::Foggy) icon = 4;
	else if (letter == Icons::Umbrella) icon = 5;
	else if (letter == Icons::Rain) icon = 6;
	else if (letter == Icons::Snow) icon = 7;
	else if (letter == Icons::Thunderstorm) icon = 8;

	// вычисление фазы анимации
	uint8_t shift = (uint8_t)(millis() / 256) & 1;

	// первая стадия - основная картника

	// отрисовка буквы. Все битмапы жестко в разрешении 12x8, по этому константы жестко в коде
	drawChar(WeatheIcons::Icons[icon*2]+(shift*12), offset, baseline, 12, 8, (uint32_t)WeatheIcons::Colors[icon*2]);
	// отрисовка акцента, если он есть
	if (WeatheIcons::Icons[icon*2+1])
		drawChar(WeatheIcons::Icons[icon*2+1], offset, baseline, 12, 8, (uint32_t)WeatheIcons::Colors[icon*2+1]);

	return 12;
}