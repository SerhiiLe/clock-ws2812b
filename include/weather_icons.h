#ifndef weather_icons_h
#define weather_icons_h

const char* utf8_to_str(uint32_t packed_utf8);
bool is_weather_symbol(uint32_t code_point);
int16_t draw_weather_icon(uint32_t letter, int16_t offset, int16_t baseline=TEXT_BASELINE);

// Список кодов иконок и их названия
namespace Icons {

constexpr uint32_t Sunny 			= 0xE29880;		// ☀️ Ясно / Солнце
constexpr uint32_t MostlyClear		= 0xF09F8CA4;	// 🌤️ Преимущественно ясно / Малооблачно
constexpr uint32_t PartlyCloudy		= 0xE29B85;		// ⛅ Переменная облачность
constexpr uint32_t Cloudy			= 0xE29881;		// ☁️ Пасмурно / Облако
constexpr uint32_t Foggy			= 0xF09F8CAB;	// 🌫 Туман / Дымка
constexpr uint32_t Rain				= 0xF09F8CA7;	// 🌧️ Дождь / Ливень
constexpr uint32_t Umbrella			= 0xE29894;		// ☔ Зонт под дождем
constexpr uint32_t Snow				= 0xF09F8CA8;	// 🌨️ Снегопад / Снег
constexpr uint32_t Thunderstorm		= 0xE29B88;		// ⛈️ Гроза с дождем
constexpr uint32_t Rise     		= 0xE28697;		// ↗️ Растёт
constexpr uint32_t Fall		        = 0xE28698;		// ↘️ Падает

}


#endif