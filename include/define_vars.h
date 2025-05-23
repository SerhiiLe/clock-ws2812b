#ifndef define_vars_h
#define define_vars_h

#include <FastLED.h>
#include "leds.h"
extern CRGB leds[];
extern uint8_t led_brightness; // текущая яркость

extern bool fs_isStarted;
extern bool wifi_isConnected;
extern bool wifi_isPortal;
extern String wifi_message;
extern bool fl_demo;
extern time_t last_telegram;
extern bool ftp_isAllow;
extern bool fl_5v;
extern bool fl_allowLEDS;
extern bool fl_timeNotSync;
extern bool fl_needStartTime;
extern bool fl_ntpRequestIsSend;
extern bool fl_led_motion;
extern bool nvram_enable;
extern uint8_t rtc_enable;
extern uint8_t fl_barometerIsInit;
extern uint8_t eeprom_chip;
extern uint8_t rtc_chip;
extern uint8_t address_bme280;
extern unsigned long last_time_display;
extern bool fl_action_move;
extern bool fl_run_allow;

// таймеры должны быть доступны в разных местах
#include "timerMinim.h"
extern timerMinim scrollTimer;          // таймер скроллинга
extern timerMinim autoBrightnessTimer;  // Таймер отслеживания показаний датчика света при включенной авторегулировки яркости матрицы
extern timerMinim ntpSyncTimer;         // Таймер синхронизации времени с NTP-сервером
extern timerMinim scrollTimer;          // Таймер задержки между обновлениями бегущей строки, определяет скорость движения
extern timerMinim clockDate;            // Таймер периодичности вывода даты в виде бегущей строки (длительность примерно 15 секунд)
extern timerMinim textTimer[];          // Таймеры бегущих строк
extern timerMinim telegramTimer;		// Таймер периодичности опроса новых сообщений
extern timerMinim alarmStepTimer;		// Таймер увеличения громкости будильника
extern timerMinim timeoutMp3Timer;
extern timerMinim showTermTimer;
extern timerMinim syncWeatherTimer;
extern timerMinim quoteUpdateTimer;
extern timerMinim forecasterTimer;
extern timerMinim syncForecastTimer;

// управление плейером
extern int mp3_all;
extern int mp3_current;
extern int8_t cur_Volume;
extern bool mp3_isInit;

/*** определение глобальных перемененных, которые станут настройками ***/
// файл config.json
struct Global_Settings {
	uint8_t language = DEFAULT_LANGUAGE; // язык отображения
	String str_hello = "Start"; // строка которая выводится в момент запуска часов
	String clock_name = "clock"; // название часов для mDNS
	uint8_t max_alarm_time = 5; // максимальное время работы будильника
	uint8_t run_allow = 0; // режим работы бегущей строки
	uint16_t run_begin = 0; // время начала работы бегущей строки
	uint8_t dsp_off = 1; // выключать дисплей во время ночного режима
	uint16_t run_end = 1439; // время окончания работы бегущей строки
	uint8_t wide_font = 0; // использовать обычный широкий шрифт
	uint8_t show_move = 1; // включение светодиода датчика движения
	uint8_t delay_move = 5; // задержка срабатывания датчика движения (если есть ложные срабатывания)
	uint8_t max_move = 30; // максимальное время работы матрицы при питании от аккумулятора
	int8_t tz_shift = TIMEZONE; // временная зона, смещение локального времени относительно Гринвича
	uint8_t tz_dst = DSTSHIFT; // смещение летнего времени
	uint8_t tz_adjust = 0; // корректировать часовой пояс по серверу погоды
	uint8_t show_date_short = 0; // показывать дату в коротком формате
	uint8_t tiny_date = 0; // выводить дату крошечными цифрами
	uint16_t show_date_period = 30; // периодичность вывода даты в секундах
	uint8_t tiny_clock = 0; // вариант циферблата
	uint8_t dots_style = 0; // вариант отображение разделителя (двоеточия)
	uint8_t t12h = 0; // отображение в 24 или 12 часовом формате (am/pm)
	uint8_t show_time_color = 0; // режим выбора цветов циферблата часов
	uint32_t show_time_color0 = 0xFFFFFF; // цвет цифр часов (белый)
	uint32_t show_time_col[8] = {0xF6D32D,0xF6D32D,0x4444FF,0x57E389,0x57E389,0x4444FF,0xF6D32D,0xF6D32D}; // отдельно для каждой цифры
	uint8_t show_date_color = 0; // режим выбора цветов даты
	uint32_t show_date_color0 = 0xFFFFFF; // цвет даты
	uint8_t hue_shift = 0; // сдвигать гамму в зависимости от времени
	uint8_t bright_mode = 1; // режим яркости матрицы (авто или ручной)
	uint8_t bright0 = 50; // яркость матрицы средняя (1-255)
	uint16_t bright_boost = 100; // усиление показателей датчика яркости в процентах (1-250)
	uint8_t boost_mode = 0; // режим дополнительного увеличения яркости
	uint8_t bright_add = 1; // на сколько дополнительно увеличивать яркость
	float latitude = 0.0f; // географическая широта
	float longitude = 0.0f; // географическая долгота
	uint16_t bright_begin = 0; // время начала дополнительного увеличения яркости
	uint16_t bright_end = 0; // время окончания дополнительного увеличения яркости
	uint32_t max_power = DEFAULT_POWER; // максимальное потребление матрицы (чтобы блок питания тянул)
	uint8_t turn_display = 0; // перевернуть картинку
	uint8_t volume_start = 5; // начальная громкость будильника
	uint8_t volume_finish = 30; // конечная громкость будильника
	uint8_t volume_period = 5; // период в сек увеличения громкости на единицу
	uint8_t timeout_mp3 = 36; // таймаут до принудительного сброса модуля mp3, в часах
	uint8_t sync_time_period = 8; // периодичность синхронизации ntp, в часах
	uint8_t scroll_period = 25; // 60 - задержка между обновлениями бегущей строки, определяет скорость движения
	uint8_t slide_show = 2; // время показа одного слайда в режиме крошечных цифр
	uint8_t minim_show = 5; // минимальное время показа циферблата
	uint8_t reserved1 = 0; // зарезервировано для будущего использования
	String web_login = "admin"; // логин для вэб
	String web_password = ""; // пароль для вэб
};
extern Global_Settings gs;

// файл telegram.json
struct Telegram_Settings {
	uint8_t use_move = 1; // использовать датчик движения как датчик сигнализации
	uint8_t use_brightness = 1; // использовать датчик освещения как датчик сигнализации
	String pin_code = "def555"; // пин-код доступа к отправке сообщений в телеграм другим устройствам
	uint16_t sensor_timeout = 20; // время в течении которого сенсор считается действующим, в минутах
	String tb_name = ""; // имя бота, адрес. Свободная строка, только для справки
	String tb_chats = ""; // чаты из которых разрешено принимать команды
	String tb_secret = ""; // пароль для подключения функции управления из чата в телеграм
	String tb_token = ""; // API токен бота
	uint16_t tb_rate = 300; // интервал запроса новых команд в секундах
	uint16_t tb_accelerated = TELEGRAM_ACCELERATED; // ускоренный интервал запроса новых команд в секундах
	uint16_t tb_accelerate = TELEGRAM_ACCELERATE; // время в течении которого будет работать ускорение
	uint16_t tb_ban = TELEGRAM_BAN; // время на которе прекращается опрос новых сообщений, после сбоя, в секундах
};
extern Telegram_Settings ts;

struct cur_alarm {
	uint16_t settings = 0;	// настройки (побитовое поле)
	uint8_t hour = 0;	// часы
	uint8_t minute = 0;	// минуты
	uint16_t melody = 1;	// номер мелодии
	String text = "";	// текст который надо отобразить во время работы будильника
	uint8_t color_mode = 0; // режим цвета, как везде (0 )
	uint32_t color = 0xFFFFFF; // по умолчанию - белый
};
extern cur_alarm alarms[];

struct cur_text {
	String text = "";	// текст который надо отобразить
	uint8_t color_mode = 0; // режим цвета, как везде (0 )
	uint32_t color = 0xFFFFFF; // по умолчанию - белый
	uint16_t period = 60; // период повтора в секундах
	uint16_t repeat_mode = 0; // режим повтора (0 пока активно, 1 до конца дня, 2 день недели, 3 день месяца)
};
extern cur_text texts[];

extern uint8_t sec_enable;
extern uint8_t sec_curFile;

extern uint16_t sunrise; // время восхода в минутах от начала суток
extern uint16_t sunset; // время заката в минутах от начала суток
extern bool old_bright_boost; // флаг для изменения уровня яркости

struct Weather_Settings {
	uint8_t sensors = 0;
	uint16_t term_period = 60;
	uint8_t term_color_mode = 0; // режим цвета, как везде (0 )
	uint32_t term_color = 0xFFFFFF; // по умолчанию - белый
	uint8_t tiny_term = 0;
	float term_cor = -1.5f;
	int16_t bar_cor = 7;
	uint16_t term_pool = 120;
	uint8_t weather = 0;
	uint8_t sync_weather_period = 30;
	uint8_t show_weather_period = 120;
	uint8_t color_mode = 0; // режим цвета, как везде (0 )
	uint32_t color = 0xFFFFFF; // по умолчанию - белый
	uint8_t weather_code = 1;
	uint8_t temperature = 1;
	uint8_t a_temperature = 1;
	uint8_t humidity = 1;
	uint8_t cloud = 1;
	uint8_t pressure = 1;
	uint8_t wind_speed = 1;
	uint8_t wind_direction = 1;
	uint8_t wind_direction2 = 1;
	uint8_t wind_gusts = 1;
	uint8_t pressure_dir = 1;
	int16_t altitude = 50;
	uint8_t forecast = 1;
	uint8_t forecast_days = 2;
	uint8_t sync_forecast_period = 6;
	uint8_t show_forecast_period = 120;
	uint8_t color_modeF = 0; // режим цвета, как везде (0 )
	uint32_t colorF = 0xFFFFFF; // по умолчанию - белый
	uint8_t weather_codeF = 1;
	uint8_t temperatureF = 1;
	uint8_t wind_speedF = 1;
	uint8_t wind_directionF = 1;
};
extern Weather_Settings ws;

struct Quote_Settings {
	uint8_t enabled = 0;
	uint8_t period = 120;
	uint8_t color_mode = 0; // режим цвета, как везде (0 )
	uint32_t color = 0xFFFFFF; // по умолчанию - белый
	uint8_t update = 1;
	uint8_t server = 0;
	uint8_t lang = 2;
	String url = "";
	String params = "";
	uint8_t method = 0;
	uint8_t type = 0;
	String quote_field;
	String author_field;
};
extern Quote_Settings qs;

struct Quote_Server {
	bool fl_init = false;
	String url;
	String params;
	String quote;
	String author;
	uint8_t method = 0;
	uint8_t type = 0;
};
extern Quote_Server quote;

struct temp_text {
	String text = "";	// текст который надо будет выводить
	timerMinim timer;	// таймер с отсчётом интервалов показа
	int16_t count = 0;	// число повторов
	uint32_t color = 0xFFFFFF; // по умолчанию - белый
};
extern temp_text messages[];


/*** определение массива временных строк и их назначение ***/

// номера временных строк, определяют приоритет вывода
#define MESSAGE_WEB 0       // номер сообщения отправленного через WEB или MQTT
#define MESSAGE_WEATHER 1   // номер сообщения с информацией о погоде
#define MESSAGE_QUOTE 2     // номер сообщения с цитатой
#define MESSAGE_FORECAST 3   // номер сообщения с прогнозом погоды
// и того:
#define MAX_TMP_MESSAGES 4	// количество слотов для временных строк

/*** определение массива сенсоров (внешних устройств, входящих в сеть) ***/

#include <IPAddress.h>
struct cur_sensor {
	String hostname;
	IPAddress ip = {0,0,0,0}; // IPADDR_NONE;
	time_t registered = 0;
};
extern cur_sensor sensor[];

extern const byte fontSemicolon[][4] PROGMEM;
extern bool fl_tiny_clock;
extern bool screenIsFree;
extern uint8_t hue_shift;

#define LANGUAGES 3 // количество языков, нужно для проверки корректности при компиляции, но не для работы

// номера шрифтов для циферблата
#define FONT_NORMAL 0
#define FONT_HIGHT 1
#define FONT_BOLD 2
#define FONT_BOLD2 3
#define FONT_WIDE 4
#define FONT_NARROW 5
#define FONT_NARROW2 6
#define FONT_DIGIT 7
#define FONT_DIGIT2 8
#define FONT_TINY 9

#ifdef ESP32
#define SPIFFS LittleFS
#define FORMAT_LITTLEFS_IF_FAILED true
extern esp_chip_info_t chip_info;
#endif

//----------------------------------------------------
#if defined(LOG)
#undef LOG
#endif

#ifdef DEBUG
	//#define LOG                   Serial
	#define LOG(func, ...) Serial.func(__VA_ARGS__)
#else
	#define LOG(func, ...) ;
#endif

#if RELAY_TYPE == 1
	#define RELAY_OFF 0
	#define RELAY_OP(var) var
#else
	#define RELAY_OFF 1
	#define RELAY_OP(var) !var
#endif

#endif