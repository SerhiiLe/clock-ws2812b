/*
Различные запросы из Internet
Погода: https://open-meteo.com/
Цитаты: https://generator-online.com/

Всё в кучу потому, что запросы в общем-то аналогичные и не хочется размазывать один код по разным файлам. Так делать не хорошо.
*/

#include <Arduino.h>
#ifdef ESP32
#include <HTTPClient.h>
#else // ESP8266
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#endif
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include "defines.h"
#include "webClient.h"
#include "settings.h"
#include "ntp.h"
#include "webClient_translation.h"
#include "weather_icons.h"
#include <StringConverters.h>

#ifdef ESP32
WiFiClientSecure WEB_S;
#endif
#ifdef ESP8266
BearSSL::WiFiClientSecure WEB_S;
#endif
WiFiClient WEB_P;
HTTPClient httpReq;

Quote_Server quote;

bool fl_https_notInit = true;

void https_Init() {
	#ifdef ESP8266
	WEB_S.setBufferSizes(1536, 256);
	#endif
	WEB_S.setInsecure();
	fl_https_notInit = false;
}

struct weatherData {
	int utc_offset_seconds;
	int16_t elevation;
	float temperature;
	float apparent_temperature;
	uint8_t humidity;
	uint8_t weather_code;
	uint8_t cloud_cover;
	float pressure;
	float wind_speed;
	float wind_gusts;
	uint16_t wind_direction;
} wd;

struct forecastData {
	uint8_t weather_code;
	float temperature_max;
	float temperature_min;
	float wind_speed;
	float wind_gusts;
	uint16_t wind_direction;
} fd[FORECAST_DAYS];

const char* EMPTY PROGMEM = "";
const char* SPACE PROGMEM = " ";
const char* DOTS PROGMEM = "\xe2\x80\xa6";
const char* SPSTR PROGMEM = " %s ";
const char* SPSTR1 PROGMEM = " %s";

/*
0 	Clear sky
1, 2, 3 	Mainly clear, partly cloudy, and overcast
45, 48 	Fog and depositing rime fog
51, 53, 55 	Drizzle: Light, moderate, and dense intensity
56, 57 	Freezing Drizzle: Light and dense intensity
61, 63, 65 	Rain: Slight, moderate and heavy intensity
66, 67 	Freezing Rain: Light and heavy intensity
71, 73, 75 	Snow fall: Slight, moderate, and heavy intensity
77 	Snow grains
80, 81, 82 	Rain showers: Slight, moderate, and violent
85, 86 	Snow showers slight and heavy
95 * 	Thunderstorm: Slight or moderate
96, 99 * 	Thunderstorm with slight and heavy hail
*/

// расшифровка кода погоды
const char* descript_weather_code(uint8_t code) {

	switch (code) {
		case 0: // Ясно
			return txt_w0[gs.language];
		case 1: // Почти ясно
			return txt_w1[gs.language];
		case 2: // Переменная облачность
			return txt_w2[gs.language];
		case 3: // Облачно
			return txt_w3[gs.language];
		case 45: // Туман
			return txt_w45[gs.language];
		case 48: // Оседающий туман
			return txt_w48[gs.language];
		case 51: // Мряка
			return txt_w51[gs.language];
		case 53: // Лёгкая морось
			return txt_w53[gs.language];
		case 55: // Морось
			return txt_w55[gs.language];
		case 56: // Оседающий иней
			return txt_w56[gs.language];
		case 57: // Сильный иней
			return txt_w57[gs.language];
		case 61: // Небольшой дождь
			return txt_w61[gs.language];
		case 63: // Дождь
			return txt_w63[gs.language];
		case 65: // Сильный дождь
			return txt_w65[gs.language];
		case 66: // Небольшое оледенение
			return txt_w66[gs.language];
		case 67: // Оледенение
			return txt_w67[gs.language];
		case 71: // Небольшой снег
			return txt_w71[gs.language];
		case 73: // Снег
			return txt_w73[gs.language];
		case 75: // Сильный снег
			return txt_w75[gs.language];
		case 77: // Град
			return txt_w77[gs.language];
		case 80: // Небольшой ливень
			return txt_w80[gs.language];
		case 81: // Ливень
			return txt_w81[gs.language];
		case 82: // Сильный ливень
			return txt_w82[gs.language];
		case 85: // Снегопад
			return txt_w85[gs.language];
		case 86: // Сильный снегопад
			return txt_w86[gs.language];
		case 95: // Небольшая гроза
			return txt_w95[gs.language];
		case 96: // Гроза
			return txt_w96[gs.language];
		case 99: // Сильная гроза
			return txt_w99[gs.language];

		default: // непонятно
			return txt_w100[gs.language];
	}
}

uint32_t weather_icon_code(uint8_t code) {
	switch(code) {
		case 0:
			return Icons::Sunny; // Солнечно
		case 1:
			return Icons::MostlyClear; // Почти чисто
		case 2:
			return Icons::PartlyCloudy; // Переменная облачность
		case 3:
			return Icons::Cloudy; // Пасмурно
		case 45:
		case 48:
			return Icons::Foggy; // Туман
		case 51:
		case 53:
		case 55:
		case 56:
		case 57:
			return Icons::Umbrella; // Небольшой дождь
		case 61:
		case 63:
		case 65:
		case 66:
		case 67:
		case 80:
		case 81:
		case 82:
			return Icons::Rain; // Дождь / Ливень
		case 71:
		case 73:
		case 75:
		case 77:
		case 85:
		case 86:
			return Icons::Snow; // Снег
		case 95:
		case 96:
		case 99:
			return Icons::Thunderstorm; // Гроза
		default:
			return (uint32_t)'?'; // WEATHER_ICON_UNKNOWN;
	}
}

int add_temperature(char *buf, float t, bool units, bool dec) {
	switch (ws.u_t)	{ // " %+0.1f°C" если нужен плюс перед цифрой
		case 1: // Фаренгейта
			return sprintf_P(buf, dec?PSTR("%0.1f%S"):PSTR("%1.0f%S"), t*1.8+32, units?PSTR("°F"):EMPTY); 
		case 2: // Кельвина
			return sprintf_P(buf, PSTR("%1.0f%S"), t+273.15, units?PSTR("K"):EMPTY); 
		default: // Целься
			return sprintf_P(buf, dec?PSTR("%+1.1f%S"):PSTR("%+1.0f%S"), t, units?PSTR("°C"):EMPTY); 
	}
}

int add_pressure(char *buf, float p) {
	switch (ws.u_p)	{
		case 1: // kPa
			return sprintf_P(buf, PSTR("%1.1f kPa"), p/10);
		case 2: // Pa
			return sprintf_P(buf, PSTR("%1.0f Pa"), p*100);
		case 3: // mbar
			return sprintf_P(buf, PSTR("%1.0f mbar"), p);
		case 4: // inHg
			return sprintf_P(buf, PSTR("%1.2f inHg"), p*0.02953);
		case 5: // mmHg
			return sprintf_P(buf, PSTR("%1.0f mmHg"), p*0.750063755);
		default: // hPa
			return sprintf_P(buf, PSTR("%1.0f hPa"), p);
	}
}

int add_speed(char *buf, float v, bool units) {
	const char* T = units?PSTR("%1.0f%S"):PSTR("%1.0f");
	switch (ws.u_v)	{
		case 1: // km/h
			return sprintf_P(buf, T, v*3.6, units?txt_wind_speedKM[gs.language]:EMPTY);
		case 2: // mph
			return sprintf_P(buf, T, v*2.2369, units?PSTR("mph"):EMPTY);
		case 3: // kn
			return sprintf_P(buf, T, v*1.9438, units?txt_wind_speedK[gs.language]:EMPTY);
		default: // m/s
			return sprintf_P(buf, T, v, units?txt_wind_speed[gs.language]:EMPTY);
	}
}

// создание строки состояния погоды на основе ответа от сервера
const char* generate_weather_string(char* a) {
	char* pos = a;
	pos += sprintf_P(pos, txt_weather[gs.language]);

	if (ws.weather_icon) {
		pos += sprintf_P(pos, SPSTR1, utf8_to_str(weather_icon_code(wd.weather_code)));
	}
	if (ws.weather_code) {
		pos += sprintf_P(pos, SPACE);
		pos += sprintf_P(pos, descript_weather_code(wd.weather_code), wd.weather_code);
	}
	if (ws.temperature) {
		pos += sprintf_P(pos, SPACE);
		pos += add_temperature(pos, wd.temperature);
	}
	if (ws.a_temperature) {
		pos += sprintf_P(pos, SPSTR, txt_apparent[gs.language]);
		pos += add_temperature(pos, wd.apparent_temperature);
	}
	if (ws.humidity) pos += sprintf_P(pos, PSTR(" %s %u%%"), txt_humidity[gs.language], wd.humidity);
	if (ws.cloud) pos += sprintf_P(pos, PSTR(" %s %u%%"), txt_cloud[gs.language], wd.cloud_cover);
	if (ws.pressure) {
		pos += sprintf_P(pos, SPSTR, txt_pressure[gs.language]);
		pos += add_pressure(pos, wd.pressure);
	}
	if (ws.wind_speed && wd.wind_speed < 2) pos += sprintf_P(pos, txt_calm[gs.language]);
	else {
		if (ws.wind_speed) {
			pos += sprintf_P(pos, SPSTR, txt_wind[gs.language]);
			pos += add_speed(pos, wd.wind_speed, !ws.wind_gusts);
			if (ws.wind_gusts) {
				pos += sprintf_P(pos, DOTS);
				pos += add_speed(pos, wd.wind_gusts);
			}
		}
		if (ws.wind_direction) pos += sprintf_P(pos, PSTR(" %s %i\xc2\xb0"), txt_direction[gs.language], wd.wind_direction);
		if (ws.wind_direction2) {
			const char* wc = nullptr;
			if ( wd.wind_direction > 340 || wd.wind_direction <= 20 ) wc = txt_d_northern[gs.language];
			if ( wd.wind_direction > 20 && wd.wind_direction <= 68 ) wc = txt_d_north_eastern[gs.language];
			if ( wd.wind_direction > 68 && wd.wind_direction <=112 ) wc = txt_d_eastern[gs.language];
			if ( wd.wind_direction > 112 && wd.wind_direction <= 158 ) wc = txt_d_south_eastern[gs.language];
			if ( wd.wind_direction > 158 && wd.wind_direction <= 202 ) wc = txt_d_southern[gs.language];
			if ( wd.wind_direction > 202 && wd.wind_direction <= 248 ) wc = txt_d_south_western[gs.language];
			if ( wd.wind_direction > 248 && wd.wind_direction <= 292 ) wc = txt_d_western[gs.language];
			if ( wd.wind_direction > 292 && wd.wind_direction <= 340 ) wc = txt_d_north_western[gs.language];
			pos += sprintf_P(pos, SPACE);
			pos += sprintf_P(pos, wc);
		}
	}
	return a;
}

uint8_t parseWeather(const char* json) {
	LOG(println, json);

	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, json);
	// Test if parsing succeeds.
	if (error) {
		LOG(printf_P, PSTR("deserializeJson() failed: %s\n"), error.c_str());
		return 0;
	}

	const char current[] = "current";
	wd.utc_offset_seconds = doc[F("utc_offset_seconds")]; // относительно GMT
	wd.elevation = doc[F("elevation")]; // высота над уровнем моря
	time_t cur_time = doc[current][F("time")]; // по GMT
	int16_t interval = doc[current][F("interval")];
	wd.temperature = doc[current][F("temperature_2m")];
	wd.apparent_temperature = doc[current][F("apparent_temperature")];
	wd.humidity = doc[current][F("relative_humidity_2m")];
	wd.weather_code = doc[current][F("weather_code")];
	wd.cloud_cover = doc[current][F("cloud_cover")];
	wd.pressure = doc[current][F("surface_pressure")];
	wd.wind_direction = doc[current][F("wind_direction_10m")];
	wd.wind_speed = doc[current][F("wind_speed_10m")];
	wd.wind_gusts = doc[current][F("wind_gusts_10m")];

	// Составление строки с информацией о погоде
	char txt[512];
	messages[MESSAGE_WEATHER].text = String(generate_weather_string(txt));
	messages[MESSAGE_WEATHER].count = ws.weather ? 100: 0;
	messages[MESSAGE_WEATHER].color = ws.color_mode > 0 ? ws.color_mode: ws.color;

	// Синхронизация часового пояса или летнего времени
	if (gs.tz_adjust && wd.utc_offset_seconds != (gs.tz_shift+gs.tz_dst)*3600) {
		LOG(printf_P,PSTR("Timezone not sync: %+i vs %+i in system. "), wd.utc_offset_seconds/3600, gs.tz_shift+gs.tz_dst);
		if (gs.tz_shift*3600 == wd.utc_offset_seconds) {
			gs.tz_dst = 0;
			LOG(println,PSTR("Remove dst time."));
		} else
		if ((gs.tz_shift+1)*3600 == wd.utc_offset_seconds) {
			gs.tz_dst = 1;
			LOG(println,PSTR("Add dst time"));
		} else {
			gs.tz_shift = wd.utc_offset_seconds / 3600;
			gs.tz_dst = 0;
			LOG(printf_P,PSTR("Set system timezone to %+i\n"), wd.utc_offset_seconds/3600);
		}
		save_config_main();
		delay(1);
		syncTime();
	} else
	if (abs(cur_time + wd.utc_offset_seconds - getTimeU()) > (interval + 100)) {
		LOG(printf_P,PSTR("To big time drift (%+li sec.), request time sync.\n"), cur_time - getTimeU());
		syncTime();
	}

	return 1;
}

const char* generate_forecast_string(char* a) {
	char* pos = a;
	pos += sprintf_P(pos, txt_forecast[gs.language]);
	for(uint8_t i=0; i<ws.forecast_days; i++) {
		pos += sprintf_P(pos, SPSTR1, txt_ForecastDay[i][gs.language]);
		if (ws.weather_iconF) {
			pos += sprintf_P(pos, SPSTR1, utf8_to_str(weather_icon_code(fd[i].weather_code)));
		}
		if (ws.weather_codeF) {
			pos += sprintf_P(pos, SPACE);
			pos += sprintf_P(pos, descript_weather_code(fd[i].weather_code), fd[i].weather_code);
		}
		if (ws.temperatureF) {
			pos += sprintf_P(pos, SPACE);
			pos += add_temperature(pos, fd[i].temperature_min, false, false);
			pos += sprintf_P(pos, DOTS);
			pos += add_temperature(pos, fd[i].temperature_max, true, false);
		}
		if (ws.wind_speedF) {
			pos += sprintf_P(pos, SPSTR, txt_wind[gs.language]);
			pos += add_speed(pos, fd[i].wind_speed, false);
			pos += sprintf_P(pos, DOTS);
			pos += add_speed(pos, fd[i].wind_gusts);
		}
		if (ws.wind_directionF) {
			const char* wc = nullptr;
			if( fd[i].wind_direction > 340 || fd[i].wind_direction <= 20 ) wc = txt_d_northern[gs.language];
			if( fd[i].wind_direction > 20 && fd[i].wind_direction <= 68 ) wc = txt_d_north_eastern[gs.language];
			if( fd[i].wind_direction > 68 && fd[i].wind_direction <=112 ) wc = txt_d_eastern[gs.language];
			if( fd[i].wind_direction > 112 && fd[i].wind_direction <= 158 ) wc = txt_d_south_eastern[gs.language];
			if( fd[i].wind_direction > 158 && fd[i].wind_direction <= 202 ) wc = txt_d_southern[gs.language];
			if( fd[i].wind_direction > 202 && fd[i].wind_direction <= 248 ) wc = txt_d_south_western[gs.language];
			if( fd[i].wind_direction > 248 && fd[i].wind_direction <= 292 ) wc = txt_d_western[gs.language];
			if( fd[i].wind_direction > 292 && fd[i].wind_direction <= 340 ) wc = txt_d_north_western[gs.language];
			pos += sprintf_P(pos, SPACE);
			pos += sprintf_P(pos, wc);
		}
		if (i < ws.forecast_days-1) pos += sprintf_P(pos, PSTR(","));
	}
	return a;
}

uint8_t parseForecast(const char* json) {
	LOG(println, json);

	JsonDocument doc;
	DeserializationError error = deserializeJson(doc, json);
	// Test if parsing succeeds.
	if (error) {
		LOG(printf_P, PSTR("deserializeJson() failed: %s\n"), error.c_str());
		return 0;
	}

	const char daily[] = "daily";
	for(uint8_t i=0; i<FORECAST_DAYS; i++) {
		fd[i].weather_code = doc[daily][F("weather_code")][i];
		fd[i].temperature_max = doc[daily][F("temperature_2m_max")][i];
		fd[i].temperature_min = doc[daily][F("temperature_2m_min")][i];
		fd[i].wind_speed = doc[daily][F("wind_speed_10m_max")][i];
		fd[i].wind_gusts = doc[daily][F("wind_gusts_10m_max")][i];
		fd[i].wind_direction = doc[daily][F("wind_direction_10m_dominant")][i];
	}

	char txt[200*FORECAST_DAYS+50];
	messages[MESSAGE_FORECAST].text = String(generate_forecast_string(txt));
	messages[MESSAGE_FORECAST].count = ws.forecast ? 1440: 0;
	messages[MESSAGE_FORECAST].color = ws.color_modeF > 0 ? ws.color_modeF: ws.colorF;

	return 1;
}

int16_t weatherGetElevation() {
	return wd.elevation;
}

float weatherGetTemperature() {
	return wd.temperature;
}

int32_t weatherGetPressure() {
	return lroundf(wd.pressure*100);
}

// https://api.open-meteo.com/v1/forecast?latitude=46.4857&longitude=30.7438&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code,cloud_cover,surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m&wind_speed_unit=ms&timeformat=unixtime&timezone=auto&past_days=1&forecast_days=1

/*
Статусы ответа:
0 - ошибка обработки запроса
1 - успешно
2 - адрес не работает
3 - ошибка сети
*/

uint8_t weatherUpdate(uint8_t wType) {
	if (fl_https_notInit) https_Init();
	// WiFiClient WEB_client;
	// HTTPClient httpReq;

	char LatLong[50];
	sprintf_P(LatLong, PSTR("latitude=%.4f&longitude=%.4f"), gs.latitude, gs.longitude);
	uint8_t status = 1;
	#ifdef USE_HTTPS
	String req = F("https");
	#else
	String req = F("http");
	#endif
	req += F("://api.open-meteo.com/v1/forecast?");
	req += LatLong;
	if( wType == FORECAST )
		req += F("&daily=weather_code,temperature_2m_max,temperature_2m_min,wind_speed_10m_max,wind_gusts_10m_max,wind_direction_10m_dominant&timezone=auto&wind_speed_unit=ms&forecast_days=" ToSTRING(FORECAST_DAYS));
	else
		req += F("&current=temperature_2m,relative_humidity_2m,apparent_temperature,weather_code,cloud_cover,surface_pressure,wind_speed_10m,wind_direction_10m,wind_gusts_10m&wind_speed_unit=ms&timeformat=unixtime&timezone=auto&past_days=0&forecast_days=1");
	LOG(println, req);
	if (httpReq.begin(WEB_P, req)) {
		int httpCode = httpReq.GET();
		LOG(printf_P, PSTR("http answer code: %i, %s\n"), httpCode, httpReq.errorToString(httpCode).c_str());
		if( httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY )
			if( wType == FORECAST )
				status = parseForecast(httpReq.getString().c_str()); // простой прогноз на 3 дня, включая текущий
			else
				status = parseWeather(httpReq.getString().c_str()); //, _httpReq.getSize()); // текущая погода
		else status = 2;
		httpReq.end();
	} else status = 3;
	return status;
}

// Примитивный парсер json и XML
String digJSON(String& str, const char* search, bool json=true) {
	if( strlen(search) == 0 ) return String("");
	int s1, s2, s3;
	s1 = str.indexOf(search);
	if( s1 > 0 ) {
		s2 = str.indexOf(json ? ":\"": ">", s1);
		if( s2 > 0 ) {
			s2 += json ? 2: 1;
			while( (s3 = str.indexOf(json ? "\"": "</", s2)) > 0 ) { 
				if( !json || (json && str[s3-1] != '\\') )
					return StringConverters::jsonDecode(str.substring(s2, s3));
				s2 = s3+1;
			};
		}
	}
	return String("");
}

// обрезание лишних символов
void myTrim(String& str) {
	str.replace("\n", "");
	str.replace("\r", "");
	str.trim();
}

// выделение текста из json/XML
void parseQuote(String txt, bool type=true) {
	String s = digJSON(txt, quote.quote.c_str(), type);
	if( s.length() > 0 ) myTrim(s);
	// messages[MESSAGE_QUOTE].text = F("Цитата: ");
	messages[MESSAGE_QUOTE].text = String(txt_quote[gs.language]);
	messages[MESSAGE_QUOTE].text += s;
	s = digJSON(txt, quote.author.c_str(), type);
	if( s.length() > 1 ) {
		myTrim(s);
		messages[MESSAGE_QUOTE].text += ( s[0] == '-' || s[1] == ' ' ) ? " " + s: " (" + s + ")"; // perl я программист старый просто
	}
	#ifdef DEBUG
	if( messages[MESSAGE_QUOTE].text.length() <= strlen_P(txt_quote[gs.language]) ) // если не нашли цитату в xml
		LOG(printf_P, PSTR("Error parse JSON/XML.\nSource:\n%s\n"), txt.c_str());
	#endif
}

/*
Запрос новой цитаты
0 - ошибка сети
1 - успех
2 - адрес не работает
*/
uint8_t quoteGet() {
	#ifdef DEBUG
	unsigned long start_time = millis();
	#endif
	uint8_t result = 0;
	if (fl_https_notInit) https_Init();
	bool fl_isSecure = quote.url.indexOf("https://") >= 0;

	String req = quote.url; // F("api.forismatic.com/api/1.0/?method=getQuote&format=text&lang=ru");
	String params = quote.params;
	if( params.length() > 0 ) {
		switch (qs.lang) {
			case 1:
				params += F("en");
				break;
			case 2:
				params += F("ru");
				break;
			case 3:
				params += F("uk");
				break;
		}
		if(quote.method == Q_GET) {
			req += "?";
			req += params;
		}
	}

	LOG(println, req);
	if (httpReq.begin(fl_isSecure? WEB_S: WEB_P, req)) {
		int httpCode;
		if(quote.method) {
			httpReq.addHeader("Content-Type", "application/x-www-form-urlencoded");
			httpCode = httpReq.POST(params);
		} else
			httpCode = httpReq.GET();

		LOG(printf_P, PSTR("http answer code: %i, %s\n"), httpCode, httpReq.errorToString(httpCode).c_str());
		if( httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY ) {
			if(quote.type)
				parseQuote(httpReq.getString(), quote.type == Q_JSON);
			else {
				messages[MESSAGE_QUOTE].text = String(txt_quote[gs.language]);
				messages[MESSAGE_QUOTE].text += httpReq.getString();
			}
			messages[MESSAGE_QUOTE].count = qs.enabled ? 100: 0;
			messages[MESSAGE_QUOTE].timer.setInterval(1000U * (qs.period+1));
			messages[MESSAGE_QUOTE].color = qs.color_mode > 0 ? qs.color_mode: qs.color;
			LOG(printf_P, PSTR("Quote: %s\n"), messages[MESSAGE_QUOTE].text.c_str());
			result = 1;
		} else
			result = 2;
		httpReq.end();
	}
	LOG(printf_P, PSTR("request time is: %lu msec\n"), millis()-start_time);
	return result;
}

// Заполнение структуры с параметрами сервера.
void quotePrepare(bool force) {
	if( !quote.fl_init || force ) {
		switch (qs.server) {
			case 1: // ultragenerator.com
				if(random(0,2)) {
					quote.url = F("https://ultragenerator.com/citaty/handler.php");
					quote.type = Q_XML;
					quote.quote = F("quote");
					quote.author = F("author");
				} else {
					quote.url = F("https://ultragenerator.com/facts/handler.php");
					quote.type = Q_JSON;
					quote.quote = F("text");
					quote.author = "";
				}
				quote.params = ""; // у этого сервиса нет параметров. И вообще не факт, что это самостоятельный сервис
				quote.method = Q_GET;
				break;
			case 2: // own server
				quote.url = qs.url;
				quote.params = qs.params;
				quote.method = qs.method;
				quote.type = qs.type;
				quote.quote = qs.quote_field;
				quote.author = qs.author_field;
				break;
			default: // forismatic.com умер, временная заглушка 
				quote.url = F("https://dummyjson.com/quotes/random"); //http://api.forismatic.com/api/1.0/"); forismatic.com умер :(
				quote.params = ""; //F("method=getQuote&format=text&lang=");
				quote.method = Q_GET;
				quote.type = Q_JSON; //Q_TEXT;
				quote.quote = F("quote");
				quote.author = F("author");
				break;
		}
		quote.fl_init = true;
	}
}

uint8_t quoteUpdate() {
	quotePrepare(qs.server == 1);
	return quoteGet();
}
