#ifndef webClient_h
#define webClient_h

uint8_t weatherUpdate(uint8_t wType=0);
const char* generate_weather_string(char* a);
const char* generate_forecast_string(char* a);
int16_t weatherGetElevation();
float weatherGetTemperature();
int16_t weatherGetPressure();
void quoteUpdate();
void quotePrepare(bool force=false);

#define Q_TEXT 0
#define Q_JSON 1
#define Q_XML 2

#define Q_GET 0
#define Q_POST 1

#define WEATHER 0	// текущая погода
#define FORECAST 1	// прогноз погоды

#define FORECAST_DAYS 3 // количество дней для прогноза погоды

#endif