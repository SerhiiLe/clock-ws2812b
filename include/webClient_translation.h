// файл с переводами строк на разные языки
#ifndef WEBCLIENT_TRANSLATIONS_H
#define WEBCLIENT_TRANSLATIONS_H

// webClient.cpp

#include "webClient.h"

// переводим в строку, чтобы можно было использовать в запросах
// https://api.open-meteo.com/v1/forecast?latitude=46.4857&longitude=30.7438&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code,cloud_cover,surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m&wind_speed_unit=ms&timeformat=unixtime&timezone=auto&past_days=1&forecast_days=1
#define ToSTRING_(x) #x
#define ToSTRING(x) ToSTRING_(x)

// Описание погоды

/*
WMO Weather interpretation codes (WW)
Code	Description
0	Clear sky
1, 2, 3	Mainly clear, partly cloudy, and overcast
45, 48	Fog and depositing rime fog
51, 53, 55	Drizzle: Light, moderate, and dense intensity
56, 57	Freezing Drizzle: Light and dense intensity
61, 63, 65	Rain: Slight, moderate and heavy intensity
66, 67	Freezing Rain: Light and heavy intensity
71, 73, 75	Snow fall: Slight, moderate, and heavy intensity
77	Snow grains
80, 81, 82	Rain showers: Slight, moderate, and violent
85, 86	Snow showers slight and heavy
95 *	Thunderstorm: Slight or moderate
96, 99 *	Thunderstorm with slight and heavy hail
*/

// переводил как смог, ногами не бить!

const char PROGMEM T_weather_en[] = "Weather:";
const char PROGMEM T_weather_ru[] = "Погода:";
const char* PROGMEM txt_weather[LANGUAGES] = {T_weather_en, T_weather_ru, T_weather_ru};

const char PROGMEM T_w0_en[] = "Clear sky";
const char PROGMEM T_w0_ru[] = "Ясно";
const char* PROGMEM txt_w0[LANGUAGES] = {T_w0_en, T_w0_ru, T_w0_ru};

const char PROGMEM T_w1_en[] = "Mainly clear";
const char PROGMEM T_w1_ru[] = "Почти ясно";
const char PROGMEM T_w1_ua[] = "Переважно ясно";
const char* PROGMEM txt_w1[LANGUAGES] = {T_w1_en, T_w1_ru, T_w1_ua};

const char PROGMEM T_w2_en[] = "Partly cloudy";
const char PROGMEM T_w2_ru[] = "Переменная облачность";
const char PROGMEM T_w2_ua[] = "Мінлива хмарність";
const char* PROGMEM txt_w2[LANGUAGES] = {T_w2_en, T_w2_ru, T_w2_ua};

const char PROGMEM T_w3_en[] = "Overcast";
const char PROGMEM T_w3_ru[] = "Облачно";
const char PROGMEM T_w3_ua[] = "Хмарно";
const char* PROGMEM txt_w3[LANGUAGES] = {T_w3_en, T_w3_ru, T_w3_ua};

const char PROGMEM T_w45_en[] = "Fog";
const char PROGMEM T_w45_ru[] = "Туман";
const char* PROGMEM txt_w45[LANGUAGES] = {T_w45_en, T_w45_ru, T_w45_ru};

const char PROGMEM T_w48_en[] = "Depositing rime fog";
const char PROGMEM T_w48_ru[] = "Оседающий туман";
const char PROGMEM T_w48_ua[] = "Невеликий туман";
const char* PROGMEM txt_w48[LANGUAGES] = {T_w48_en, T_w48_ru, T_w48_ua};

const char PROGMEM T_w51_en[] = "Light drizzle";
const char PROGMEM T_w51_ru[] = "Мряка";
const char PROGMEM T_w51_ua[] = "Морок";
const char* PROGMEM txt_w51[LANGUAGES] = {T_w51_en, T_w51_ru, T_w51_ua};

const char PROGMEM T_w53_en[] = "Drizzle";
const char PROGMEM T_w53_ru[] = "Лёгкая морось";
const char PROGMEM T_w53_ua[] = "Легка мряка";
const char* PROGMEM txt_w53[LANGUAGES] = {T_w53_en, T_w53_ru, T_w53_ua};

const char PROGMEM T_w55_en[] = "Dense drizzle";
const char PROGMEM T_w55_ru[] = "Морось";
const char PROGMEM T_w55_ua[] = "Мряка";
const char* PROGMEM txt_w55[LANGUAGES] = {T_w55_en, T_w55_ru, T_w55_ua};

const char PROGMEM T_w56_en[] = "Freezing drizzle";
const char PROGMEM T_w56_ru[] = "Оседающий иней";
const char PROGMEM T_w56_ua[] = "Невеликий іній";
const char* PROGMEM txt_w56[LANGUAGES] = {T_w56_en, T_w56_ru, T_w56_ua};

const char PROGMEM T_w57_en[] = "Heavy freezing drizzle";
const char PROGMEM T_w57_ru[] = "Сильный иней";
const char PROGMEM T_w57_ua[] = "Сильний іній";
const char* PROGMEM txt_w57[LANGUAGES] = {T_w57_en, T_w57_ru, T_w57_ua};

const char PROGMEM T_w61_en[] = "Slight rain";
const char PROGMEM T_w61_ru[] = "Небольшой дождь";
const char PROGMEM T_w61_ua[] = "Невеликий дощ";
const char* PROGMEM txt_w61[LANGUAGES] = {T_w61_en, T_w61_ru, T_w61_ua};

const char PROGMEM T_w63_en[] = "Rain";
const char PROGMEM T_w63_ru[] = "Дождь";
const char PROGMEM T_w63_ua[] = "Дощ";
const char* PROGMEM txt_w63[LANGUAGES] = {T_w63_en, T_w63_ru, T_w63_ua};

const char PROGMEM T_w65_en[] = "Heavy rain";
const char PROGMEM T_w65_ru[] = "Сильный дождь";
const char PROGMEM T_w65_ua[] = "Сильний дощ";
const char* PROGMEM txt_w65[LANGUAGES] = {T_w65_en, T_w65_ru, T_w65_ua};

const char PROGMEM T_w66_en[] = "Freezing rain";
const char PROGMEM T_w66_ru[] = "Небольшое оледенение";
const char PROGMEM T_w66_ua[] = "Невелика ожеледиця";
const char* PROGMEM txt_w66[LANGUAGES] = {T_w66_en, T_w66_ru, T_w66_ua};

const char PROGMEM T_w67_en[] = "Heavy freezing rain";
const char PROGMEM T_w67_ru[] = "Оледенение";
const char PROGMEM T_w67_ua[] = "Ожеледиця";
const char* PROGMEM txt_w67[LANGUAGES] = {T_w67_en, T_w67_ru, T_w67_ua};

const char PROGMEM T_w71_en[] = "Snow fall";
const char PROGMEM T_w71_ru[] = "Небольшой снег";
const char PROGMEM T_w71_ua[] = "Невеликий сніг";
const char* PROGMEM txt_w71[LANGUAGES] = {T_w71_en, T_w71_ru, T_w71_ua};

const char PROGMEM T_w73_en[] = "Moderate snow fall";
const char PROGMEM T_w73_ru[] = "Снег";
const char PROGMEM T_w73_ua[] = "Сніг";
const char* PROGMEM txt_w73[LANGUAGES] = {T_w73_en, T_w73_ru, T_w73_ua};

const char PROGMEM T_w75_en[] = "Heavy snow fall";
const char PROGMEM T_w75_ru[] = "Сильный снег";
const char PROGMEM T_w75_ua[] = "Сильний сніг";
const char* PROGMEM txt_w75[LANGUAGES] = {T_w75_en, T_w75_ru, T_w75_ua};

const char PROGMEM T_w77_en[] = "Snow grains";
const char PROGMEM T_w77_ru[] = "Снежная крупа";
const char PROGMEM T_w77_ua[] = "Сніжна крупа";
const char* PROGMEM txt_w77[LANGUAGES] = {T_w77_en, T_w77_ru, T_w77_ua};

const char PROGMEM T_w80_en[] = "Light rain showers";
const char PROGMEM T_w80_ru[] = "Небольшой ливень";
const char PROGMEM T_w80_ua[] = "Невелика злива";
const char* PROGMEM txt_w80[LANGUAGES] = {T_w80_en, T_w80_ru, T_w80_ua};

const char PROGMEM T_w81_en[] = "Moderate rain showers";
const char PROGMEM T_w81_ru[] = "Ливень";
const char PROGMEM T_w81_ua[] = "Злива";
const char* PROGMEM txt_w81[LANGUAGES] = {T_w81_en, T_w81_ru, T_w81_ua};

const char PROGMEM T_w82_en[] = "Heavy rain showers";
const char PROGMEM T_w82_ru[] = "Сильный ливень";
const char PROGMEM T_w82_ua[] = "Сильна злива";
const char* PROGMEM txt_w82[LANGUAGES] = {T_w82_en, T_w82_ru, T_w82_ua};

const char PROGMEM T_w85_en[] = "Light snow showers";
const char PROGMEM T_w85_ru[] = "Снегопад";
const char PROGMEM T_w85_ua[] = "Снігопад";
const char* PROGMEM txt_w85[LANGUAGES] = {T_w85_en, T_w85_ru, T_w85_ua};

const char PROGMEM T_w86_en[] = "Heavy snow showers";
const char PROGMEM T_w86_ru[] = "Сильный снегопад";
const char PROGMEM T_w86_ua[] = "Сильний снігопад";
const char* PROGMEM txt_w86[LANGUAGES] = {T_w86_en, T_w86_ru, T_w86_ua};

const char PROGMEM T_w95_en[] = "Slight thunderstorm";
const char PROGMEM T_w95_ru[] = "Небольшая гроза";
const char PROGMEM T_w95_ua[] = "Невелика гроза";
const char* PROGMEM txt_w95[LANGUAGES] = {T_w95_en, T_w95_ru, T_w95_ua};

const char PROGMEM T_w96_en[] = "Thunderstorm";
const char PROGMEM T_w96_ru[] = "Гроза с градом";
const char PROGMEM T_w96_ua[] = "Гроза з градом";
const char* PROGMEM txt_w96[LANGUAGES] = {T_w96_en, T_w96_ru, T_w96_ua};

const char PROGMEM T_w99_en[] = "Heavy hail";
const char PROGMEM T_w99_ru[] = "Сильный град";
const char PROGMEM T_w99_ua[] = "Сильний град";
const char* PROGMEM txt_w99[LANGUAGES] = {T_w99_en, T_w99_ru, T_w99_ua};

const char PROGMEM T_w100_en[] = "Unknown%u";
const char PROGMEM T_w100_ru[] = "Неизвестно%u";
const char PROGMEM T_w100_ua[] = "Невідомо%u";
const char* PROGMEM txt_w100[LANGUAGES] = {T_w100_en, T_w100_ru, T_w100_ua};

const char PROGMEM T_apparent_en[] = "apparent as";
const char PROGMEM T_apparent_ru[] = "по ощущениям";
const char PROGMEM T_apparent_ua[] = "відчувається як";
const char* PROGMEM txt_apparent[LANGUAGES] = {T_apparent_en, T_apparent_ru, T_apparent_ua};

const char PROGMEM T_humidity_en[] = "Humidity";
const char PROGMEM T_humidity_ru[] = "Влажность";
const char PROGMEM T_humidity_ua[] = "Вологість";
const char* PROGMEM txt_humidity[LANGUAGES] = {T_humidity_en, T_humidity_ru, T_humidity_ua};

const char PROGMEM T_pressure_en[] = "Pressure";
const char PROGMEM T_pressure_ru[] = "Давление";
const char PROGMEM T_pressure_ua[] = "Тиск";
const char* PROGMEM txt_pressure[LANGUAGES] = {T_pressure_en, T_pressure_ru, T_pressure_ua};

const char PROGMEM T_cloud_en[] = "Cloudiness";
const char PROGMEM T_cloud_ru[] = "Облачность";
const char PROGMEM T_cloud_ua[] = "Хмарність";
const char* PROGMEM txt_cloud[LANGUAGES] = {T_cloud_en, T_cloud_ru, T_cloud_ua};

const char PROGMEM T_calm_en[] = " Calm";
const char PROGMEM T_calm_ru[] = " Штиль";
const char PROGMEM T_calm_ua[] = " Без вітру";
const char* PROGMEM txt_calm[LANGUAGES] = {T_calm_en, T_calm_ru, T_calm_ua};

const char PROGMEM T_wind_en[] = "Wind";
const char PROGMEM T_wind_ru[] = "Ветер";
const char PROGMEM T_wind_ua[] = "Вітер";
const char* PROGMEM txt_wind[LANGUAGES] = {T_wind_en, T_wind_ru, T_wind_ua};

const char PROGMEM T_wind_speed_en[] = "m/s";
const char PROGMEM T_wind_speed_ru[] = "м/с";
const char* PROGMEM txt_wind_speed[LANGUAGES] = {T_wind_speed_en, T_wind_speed_ru, T_wind_speed_ru};

const char PROGMEM T_direction_en[] = "direction";
const char PROGMEM T_direction_ru[] = "направление";
const char PROGMEM T_direction_ua[] = "напрямок";
const char* PROGMEM txt_direction[LANGUAGES] = {T_direction_en, T_direction_ru, T_direction_ua};

const char PROGMEM T_d_northern_en[] = "Northern";
const char PROGMEM T_d_northern_ru[] = "Северный";
const char PROGMEM T_d_northern_ua[] = "Північний";
const char* PROGMEM txt_d_northern[LANGUAGES] = {T_d_northern_en, T_d_northern_ru, T_d_northern_ua};

const char PROGMEM T_d_north_eastern_en[] = "North-Eastern";
const char PROGMEM T_d_north_eastern_ru[] = "Северо-восточный";
const char PROGMEM T_d_north_eastern_ua[] = "Північно-східний";
const char* PROGMEM txt_d_north_eastern[LANGUAGES] = {T_d_north_eastern_en, T_d_north_eastern_ru, T_d_north_eastern_ua};

const char PROGMEM T_d_eastern_en[] = "Eastern";
const char PROGMEM T_d_eastern_ru[] = "Восточный";
const char PROGMEM T_d_eastern_ua[] = "Східний";
const char* PROGMEM txt_d_eastern[LANGUAGES] = {T_d_eastern_en, T_d_eastern_ru, T_d_eastern_ua};

const char PROGMEM T_d_south_eastern_en[] = "South-Eastern";
const char PROGMEM T_d_south_eastern_ru[] = "Юго-восточный";
const char PROGMEM T_d_south_eastern_ua[] = "Південно-східний";
const char* PROGMEM txt_d_south_eastern[LANGUAGES] = {T_d_south_eastern_en, T_d_south_eastern_ru, T_d_south_eastern_ua};

const char PROGMEM T_d_southern_en[] = "Southern";
const char PROGMEM T_d_southern_ru[] = "Южный";
const char PROGMEM T_d_southern_ua[] = "Південний";
const char* PROGMEM txt_d_southern[LANGUAGES] = {T_d_southern_en, T_d_southern_ru, T_d_southern_ua};

const char PROGMEM T_d_south_western_en[] = "South-Western";
const char PROGMEM T_d_south_western_ru[] = "Юго-западный";
const char PROGMEM T_d_south_western_ua[] = "Південно-західний";
const char* PROGMEM txt_d_south_western[LANGUAGES] = {T_d_south_western_en, T_d_south_western_ru, T_d_south_western_ua};

const char PROGMEM T_d_western_en[] = "Western";
const char PROGMEM T_d_western_ru[] = "Западный";
const char PROGMEM T_d_western_ua[] = "Західний";
const char* PROGMEM txt_d_western[LANGUAGES] = {T_d_western_en, T_d_western_ru, T_d_western_ua};

const char PROGMEM T_d_north_western_en[] = "North-Western";
const char PROGMEM T_d_north_western_ru[] = "Северо-западный";
const char PROGMEM T_d_north_western_ua[] = "Північно-західний";
const char* PROGMEM txt_d_north_western[LANGUAGES] = {T_d_north_western_en, T_d_north_western_ru, T_d_north_western_ua};

const char PROGMEM T_quote_en[] = "Quote";
const char PROGMEM T_quote_ru[] = "Цитата";
const char* PROGMEM txt_quote[LANGUAGES] = {T_quote_en, T_quote_ru, T_quote_ru};

const char PROGMEM T_forecast_en[] = "Forecast:";
const char PROGMEM T_forecast_ru[] = "Прогноз:";
const char* PROGMEM txt_forecast[LANGUAGES] = {T_forecast_en, T_forecast_ru, T_forecast_ru};

const char PROGMEM T_Today_en[] = "Today";
const char PROGMEM T_Today_ru[] = "Сегодня";
const char PROGMEM T_Today_ua[] = "Сьогодні";

const char PROGMEM T_Tomorrow_en[] = "Tomorrow";
const char PROGMEM T_Tomorrow_ru[] = "Завтра";

const char PROGMEM T_AfterTomorrow_en[] = "After tomorrow";
const char PROGMEM T_AfterTomorrow_ru[] = "Послезавтра";
const char PROGMEM T_AfterTomorrow_ua[] = "Післязавтра";

const char* PROGMEM txt_ForecastDay[FORECAST_DAYS][LANGUAGES] = {
	{T_Today_en, T_Today_ru, T_Today_ua},
	{T_Tomorrow_en, T_Tomorrow_ru, T_Tomorrow_ru},
	{T_AfterTomorrow_en, T_AfterTomorrow_ru, T_AfterTomorrow_ua}
};

#endif