/*
	встроенный web сервер для настройки часов
	(для начальной настройки ip и wifi используется wifi_init)
*/

#include <Arduino.h>
#include "defines.h"
#ifdef ESP32
#include <WebServer.h>
#include "mHTTPUpdateServer.h"
#include <ESPmDNS.h>
#include <rom/rtc.h>
#else // ESP8266
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266mDNS.h>
#endif
#include <LittleFS.h>
#include <time.h>
#include <WebServerUtils.h>
#include "web.h"
#include "settings.h"
#include "runningText.h"
#include "textTiny.h"
#include "ntp.h"
#include "rtc.h"
#include "barometer.h"
#include "dfplayer.h"
#include "telegram.h"
#include "clock.h"
#include "wifi_init.h"
#include "webClient.h"
#include "forecaster.h"
#include "web_translation.h"

#define HPP(txt, ...) HTTP.client().printf_P(PSTR(txt), __VA_ARGS__)

#ifdef ESP32
WebServer HTTP(80);
HTTPUpdateServer httpUpdater;
WebServerUtils<WebServer> web(HTTP);
#endif
#ifdef ESP8266
ESP8266WebServer HTTP(80);
ESP8266HTTPUpdateServer httpUpdater;
WebServerUtils<ESP8266WebServer> web(HTTP);
#endif
bool web_isStarted = false;

void save_settings();
void save_telegram();
void save_alarm();
void off_alarm();
void save_text();
void off_text();
void save_quote();
void show_quote();
void save_weather();
void show_weather();
void show_sensors();
void show_forecast();
void show();
void sysinfo();
void play();
void maintence();
void set_clock();
void onoff();
void send();
void logout();
void sensors();
void registration();
void show_status();
void save_cuckoo ();

bool fileSend(String path);
bool fl_mdns = false;

bool fl_playStarted = false;

const char PROGMEM one[] = "1";
const char PROGMEM zero[] = "0";
const char PROGMEM text_plain[] = "text/plain";
const char PROGMEM txt_save[] = "save";

// отключение веб сервера для активации режима настройки wifi
void web_disable() {
	HTTP.stop();
	web_isStarted = false;
	LOG(println, PSTR("HTTP server stoped"));

	#ifdef ESP32
	MDNS.disableWorkstation();
	#else // ESP8266
	MDNS.close();
	#endif
	fl_mdns = false;
	LOG(println, PSTR("MDNS responder stoped"));
}

// отправка простого текста
void text_send(String s, uint16_t r = 200) {
	HTTP.send(r, text_plain, s);
}
void text_send_P(const char* s, uint16_t r = 200) {
	HTTP.send_P(r, text_plain, s);
}
// отправка сообщение "не найдено"
void not_found() {
	text_send(F("Not Found"), 404);
}

// диспетчер вызовов веб сервера
void web_process() {
	if( web_isStarted ) {
		HTTP.handleClient();
		#ifdef ESP8266
		if(fl_mdns) MDNS.update();
		#endif
	} else {
		HTTP.begin();
		// Обработка HTTP-запросов
		HTTP.on(F("/save_settings"), save_settings);
		HTTP.on(F("/save_telegram"), save_telegram);
		HTTP.on(F("/save_alarm"), save_alarm);
		HTTP.on(F("/off_alarm"), off_alarm);
		HTTP.on(F("/save_text"), save_text);
		HTTP.on(F("/off_text"), off_text);
		HTTP.on(F("/save_quote"), save_quote);
		HTTP.on(F("/show_quote"), show_quote);
		HTTP.on(F("/save_weather"), save_weather);
		HTTP.on(F("/show_weather"), show_weather);
		HTTP.on(F("/show_sensors"), show_sensors);
		HTTP.on(F("/show_forecast"), show_forecast);
		HTTP.on(F("/show"), show);
		HTTP.on(F("/sysinfo"), sysinfo);
		HTTP.on(F("/play"), play);
		HTTP.on(F("/clear"), maintence);
		HTTP.on(F("/clock"), set_clock);
		HTTP.on(F("/onoff"), onoff);
		HTTP.on(F("/send"), send);
		HTTP.on(F("/logout"), logout);
		HTTP.on(F("/sensors"), sensors);
		HTTP.on(F("/registration"), registration);
		HTTP.on(F("/status"), show_status);
		HTTP.on(F("/save_cuckoo"), save_cuckoo);
		HTTP.on(F("/who"), [](){
			text_send(gs.clock_name);
		});
		HTTP.onNotFound([](){
			if(!fileSend(HTTP.uri()))
				not_found();
			});
		web_isStarted = true;
  		httpUpdater.setup(&HTTP, gs.web_login, gs.web_password);
		LOG(println, PSTR("HTTP server started"));

		#ifdef ESP32
		if(MDNS.begin(gs.clock_name)) {
		#else // ESP8266
		if(MDNS.begin(gs.clock_name.c_str(), WiFi.localIP())) {
		#endif
			MDNS.addService("http", "tcp", 80);
			fl_mdns = true;
			LOG(println, PSTR("MDNS responder started"));
		}
	}
}

// страничка выхода, будет предлагать ввести пароль, пока он не перестанет совпадать с реальным
void logout() {
	if(gs.web_login.length() > 0 && gs.web_password.length() > 0)
		if(HTTP.authenticate(gs.web_login.c_str(), gs.web_password.c_str()))
			HTTP.requestAuthentication(DIGEST_AUTH);
	if(!fileSend(F("/logged-out.html")))
		not_found();
}

// список файлов, для которых авторизация не нужна, остальные под паролем
bool auth_need(String s) {
	if(s == F("/index.html")) return false;
	if(s == F("/about.html")) return false;
	if(s == F("/send.html")) return false;
	if(s == F("/logged-out.html")) return false;
	if(s.endsWith(F(".js"))) return false;
	if(s.endsWith(F(".css"))) return false;
	if(s.endsWith(F(".ico"))) return false;
	if(s.endsWith(F(".png"))) return false;
	return true;
}

// авторизация. много комментариев из документации, чтобы по новой не искать
bool is_no_auth() {
	// allows you to set the realm of authentication Default:"Login Required"
	// const char* www_realm = "Custom Auth Realm";
	// the Content of the HTML response in case of Unauthorized Access Default:empty
	// String authFailResponse = "Authentication Failed";
	if(gs.web_login.length() > 0 && gs.web_password.length() > 0 )
		if(!HTTP.authenticate(gs.web_login.c_str(), gs.web_password.c_str())) {
			//Basic Auth Method with Custom realm and Failure Response
			//return server.requestAuthentication(BASIC_AUTH, www_realm, authFailResponse);
			//Digest Auth Method with realm="Login Required" and empty Failure Response
			//return server.requestAuthentication(DIGEST_AUTH);
			//Digest Auth Method with Custom realm and empty Failure Response
			//return server.requestAuthentication(DIGEST_AUTH, www_realm);
			//Digest Auth Method with Custom realm and Failure Response
			HTTP.requestAuthentication(DIGEST_AUTH);
			return true;
		}
	return false;
}

// проверка, если файлы локализации с шаблоном "name_ua.ext", то можно кешировать.
bool allowed_cache(const String &p) {
	int indexOf_ = p.indexOf('_');
	return indexOf_ > 0 && p[indexOf_ + 3] == '.'; 
}

// отправка файла
bool fileSend(String path) {
	// если путь пустой - исправить на индексную страничку
	if( path.endsWith("/") ) path += F("index.html");
	// проверка необходимости авторизации
	if(auth_need(path) && is_no_auth()) return false;
	if(!fs_isStarted) {
		// файловая система не загружена, переход на страничку обновления
		HTTP.client().print(PSTR("HTTP/1.1 200\r\nContent-Type: text/html\r\nContent-Length: 80\r\nConnection: close\r\n\r\n<html><body><h1><a href='/update'>File system not exist!</a></h1></body></html>"));
		return true;
	}
	return web.fileSend(path, allowed_cache);
}

/****** обработка разных запросов ******/

// сохранение настроек
void save_settings() {
	if(is_no_auth()) return;
	web.need_save = false;

	if( web.to_int(F("language"), gs.language, 0, 2) ) {
		if( ws.weather ) syncWeatherTimer.setNext(100);
		if( qs.enabled ) quoteUpdateTimer.setNext(500);
		if( ws.forecast ) syncForecastTimer.setNext(1000);
	};
	web.to_string(F("str_hello"), gs.str_hello);
	if(web.to_string(F("clock_name"), gs.clock_name))
		#ifdef ESP32
			if(fl_mdns)	MDNS.setInstanceName(gs.clock_name);
		#else // ESP8266
			if(fl_mdns)	MDNS.setHostname(gs.clock_name.c_str());
		#endif
	web.to_int(F("max_alarm_time"), gs.max_alarm_time, 1, 30);
	web.to_int(F("run_allow"), gs.run_allow, 0, 2);
	web.time(F("run_begin"), gs.run_begin);
	web.time(F("run_end"), gs.run_end);
	web.checkbox(F("dsp_off"), gs.dsp_off);
	web.checkbox(F("wide_font"), gs.wide_font);
	web.checkbox(F("show_move"), gs.show_move);
	web.to_int(F("delay_move"), gs.delay_move, 0, 10);
	web.to_int(F("max_move"), gs.max_move, gs.delay_move, 255);
	bool sync_time = false;
	if( web.to_int(F("tz_shift"), gs.tz_shift, -12, 12) )
		sync_time = true;
	if( web.checkbox(F("tz_dst"), gs.tz_dst) )
		sync_time = true;
	web.checkbox(F("tz_adjust"), gs.tz_adjust);
	web.to_int(F("tiny_clock"), gs.tiny_clock, 0, 8);
	web.to_int(F("dots_style"), gs.dots_style, 0, 11);
	web.checkbox(F("t12h"), gs.t12h);
	web.to_int(F("date_short"), gs.show_date_short, 0, 3);
	web.checkbox(F("tiny_date"), gs.tiny_date);
	if( web.to_int(F("date_period"), gs.show_date_period, 20, 1439) )
		clockDate.setInterval(1000U * gs.show_date_period);
	web.to_int(F("time_color"), gs.show_time_color, 0, 5);
	web.color(F("time_color0"), gs.show_time_color0);
	// цвет часов
	web.color(F("time_color1"), gs.show_time_col[0]);
	gs.show_time_col[1] = gs.show_time_col[0];
	// web.color(F("time_color2"), gs.show_time_col[1]);
	// цвет разделителей
	web.color(F("time_color3"), gs.show_time_col[2]);
	gs.show_time_col[5] = gs.show_time_col[2];
	// цвет минут
	web.color(F("time_color4"), gs.show_time_col[3]);
	gs.show_time_col[4] = gs.show_time_col[3];
	// web.color(F("time_color5"), gs.show_time_col[4]);
	// цвет секунд
	web.color(F("time_color6"), gs.show_time_col[6]);
	gs.show_time_col[7] = gs.show_time_col[6];
	web.to_int(F("date_color"), gs.show_date_color, 0, 4);
	web.color(F("date_color0"), gs.show_date_color0);
	web.to_int(F("hue_shift"), gs.hue_shift, 0, 4);
	bool need_bright = false;
	if( web.to_int(F("bright_mode"), gs.bright_mode, 0, 2) )
		need_bright = true;
	if( web.to_int(F("bright0"), gs.bright0, 1, 255) )
		need_bright = true;
	web.to_int(F("br_boost"), gs.bright_boost, 1, 1000);
	if( web.to_int(F("boost_mode"), gs.boost_mode, 0, 5) )
		sync_time = true;
	if( web.to_int(F("br_add"), gs.bright_add, 1, 255) )
		need_bright = true;
	if( web.to_float(F("latitude"), gs.latitude, -180.0f, 180.0f) )
		sync_time = true;
	if( web.to_float(F("longitude"), gs.longitude, -180.0f, 180.0f) )
		sync_time = true;
	web.time(F("br_begin"), gs.bright_begin);
	web.time(F("br_end"), gs.bright_end);
	if( web.to_int(F("max_power"), gs.max_power, 200, MAX_POWER) )
		if(DEFAULT_POWER > 0) FastLED.setMaxPowerInVoltsAndMilliamps(5, gs.max_power);
	web.to_int(F("turn_display"), gs.turn_display, 0, 3);
	web.to_int(F("volume_start"), gs.volume_start, 1, 30);
	web.to_int(F("volume_finish"), gs.volume_finish, 1, 30);
	gs.volume_finish = constrain(gs.volume_finish, gs.volume_start, 30);
	if( web.to_int(F("volume_period"), gs.volume_period, 1, 30) )
		alarmStepTimer.setInterval(1000U * gs.volume_period);
	if( web.to_int(F("timeout_mp3"), gs.timeout_mp3, 1, 255) )
		timeoutMp3Timer.setInterval(3600000U * gs.timeout_mp3);
	if( web.to_int(F("sync_time_period"), gs.sync_time_period, 1, 255) )
		ntpSyncTimer.setInterval(3600000U * gs.sync_time_period);
	if( web.to_int(F("scroll_period"), gs.scroll_period, 0, 50) )
		scrollTimer.setInterval(60 - gs.scroll_period);
	web.to_int(F("slide_show"), gs.slide_show, 1, 10);
	web.to_int(F("minim_show"), gs.minim_show, 0, 20);
	bool need_web_restart = false;
	if( web.to_string(F("web_login"), gs.web_login) )
		need_web_restart = true;
	if( web.to_string(F("web_password"), gs.web_password) )
		need_web_restart = true;

	HTTP.sendHeader(F("Location"),"/");
	HTTP.send(303);
	delay(1);
	if( web.need_save ) save_config_main();
	// initRString(PSTR("Настройки сохранены"));
	printTinyText(txt_save,1,9);
	if( sync_time ) syncTime();
	if( need_bright ) old_bright_boost = !old_bright_boost;
	if(need_web_restart) httpUpdater.setup(&HTTP, gs.web_login, gs.web_password);
}

// сохранение настроек telegram (охраны)
void save_telegram() {
	if(is_no_auth()) return;
	web.need_save = false;
	bool fl_setTelegram = false;

	web.checkbox(F("use_move"), ts.use_move);
	web.checkbox(F("use_brightness"), ts.use_brightness);
	web.to_string(F("pin_code"), ts.pin_code);
	web.to_int(F("sensor_timeout"), ts.sensor_timeout, 1, 16000);
	web.to_string(F("tb_name"), ts.tb_name);
	if( web.to_string(F("tb_chats"), ts.tb_chats) )
		fl_setTelegram = true;
	if( web.to_string(F("tb_token"), ts.tb_token) )
		fl_setTelegram = true;
	web.to_string(F("tb_secret"), ts.tb_secret);
	if( web.to_int(F("tb_rate"), ts.tb_rate, 0, 3600) )
		telegramTimer.setInterval(1000U * ts.tb_rate);
	web.to_int(F("tb_accelerated"), ts.tb_accelerated, 1, 600);
	web.to_int(F("tb_accelerate"), ts.tb_accelerate, 1, 3600);
	web.to_int(F("tb_ban"), ts.tb_ban, 900, 3600);
	web.to_int(F("rcount"), ts.rcount, 1, 32000);
	web.to_int(F("rint"), ts.rint, 1, 65000);
	web.to_int(F("color_mode"), ts.color_mode, 0, 5);
	web.color(F("color"), ts.color);
	web.to_int(F("melody"), ts.melody, 0, 4096);
	web.to_int(F("vol"), ts.volume, 1, 30);

	HTTP.sendHeader(F("Location"),"/security.html");
	HTTP.send(303);
	delay(1);
	if( web.need_save ) save_config_telegram();
	// initRString(PSTR("Настройки сохранены"));
	printTinyText(txt_save,1,9);
	if(fl_setTelegram) setup_telegram();
}

// перезагрузка часов, сброс ком-порта, отключение сети и диска, чтобы ничего не мешало перезагрузке
void reboot_clock() {
	Serial.flush();
	#ifdef ESP32
	WiFi.getSleep();
	#else // ESP8266
	WiFi.forceSleepBegin(); //disable AP & station by calling "WiFi.mode(WIFI_OFF)" & put modem to sleep
	#endif
	LittleFS.end();
	delay(1000);
	ESP.restart();
}

void reset_settings(int t) {
	switch (t) {
		case 0: { // главная конфигурация
			LOG(println, PSTR("reset settings"));
			if(LittleFS.exists(F("/config.json"))) LittleFS.remove(F("/config.json"));
			} break;
		case 1: { // настройки будильников
			LOG(println, PSTR("reset alarms"));
			if(LittleFS.exists(F("/alarms.json"))) LittleFS.remove(F("/alarms.json"));
			} break;
		case 2: { // настройки бегущих строк
			LOG(println, PSTR("reset texts"));
			if(LittleFS.exists(F("/texts.json"))) LittleFS.remove(F("/texts.json"));
			} break;
		case 3: { // настройки цитат
			LOG(println, PSTR("reset quotes"));
			if(LittleFS.exists(F("/quote.json"))) LittleFS.remove(F("/quote.json"));
			} break;
		case 4: { // настройки сервера погоды
			LOG(println, PSTR("reset weather"));
			if(LittleFS.exists(F("/weather.json"))) LittleFS.remove(F("/weather.json"));
			} break;
		case 5: { // настройки Telegram/security
			LOG(println, PSTR("reset se"));
			if(LittleFS.exists(F("/security.json"))) LittleFS.remove(F("/security.json"));
			} break;
		case 6: { // журнал событий
			LOG(println, PSTR("erase logs"));
			char fileName[32];
			for(int8_t i=0; i<SEC_LOG_COUNT; i++) {
				sprintf_P(fileName, SEC_LOG_FILE, i);
				if( LittleFS.exists(fileName) ) LittleFS.remove(fileName);
			}
			} break;
		default: // такого раздела нет, просто перезагрузится
			break;
	}
}

void maintence() {
	if(is_no_auth()) return;
	HTTP.sendHeader(F("Location"),"/");
	HTTP.send(303); 
	String name = "t";
	if( HTTP.hasArg(name) ) {
		int t = constrain(HTTP.arg(name).toInt(), 0, 255);
		initRString(PSTR("Reset"),1,3);
		if(t == 96) {
			// сброс всех настроек (кроме wifi)
			for(uint8_t i=0; i<=6; i++)
				reset_settings(i);
		} else if(t > 0) {
			// сброс конкретного раздела настроек
			reset_settings(t-1);
		}
		reboot_clock();
	}
}

// сохранение настроек будильника
void save_alarm() {
	if(is_no_auth()) return;
	web.need_save = false;
	uint8_t target = 0;
	uint16_t settings = 512;
	String name = F("target");
	if( HTTP.hasArg(name) ) {
		target = HTTP.arg(name).toInt();
		name = F("time");
		if( HTTP.hasArg(name) ) {
			// выделение часов и минут из строки вида 00:00
			size_t pos = HTTP.arg(name).indexOf(":");
			uint8_t h = constrain(HTTP.arg(name).toInt(), 0, 23);
			uint8_t m = constrain(HTTP.arg(name).substring(pos+1).toInt(), 0, 59);
			if( h != alarms[target].hour || m != alarms[target].minute ) {
				alarms[target].hour = h;
				alarms[target].minute = m;
				web.need_save = true;
			}
		}
		name = F("rmode");
		if( HTTP.hasArg(name) ) settings |= constrain(HTTP.arg(name).toInt(), 0, 3) << 7;
		if( HTTP.hasArg(F("Mo")) ) settings |= 2;
		if( HTTP.hasArg(F("Tu")) ) settings |= 4;
		if( HTTP.hasArg(F("We")) ) settings |= 8;
		if( HTTP.hasArg(F("Th")) ) settings |= 16;
		if( HTTP.hasArg(F("Fr")) ) settings |= 32;
		if( HTTP.hasArg(F("Sa")) ) settings |= 64;
		if( HTTP.hasArg(F("Su")) ) settings |= 1;
		if( settings != alarms[target].settings ) {
			alarms[target].settings = settings;
			web.need_save = true;
		}
		web.to_int(F("melody"), alarms[target].melody, 1, mp3_all);
		web.to_string(F("text"), alarms[target].text);
		web.to_int(F("color_mode"), alarms[target].color_mode, 0, 4);
		web.color(F("color"), alarms[target].color);
	}
	HTTP.sendHeader(F("Location"),F("/alarms.html"));
	HTTP.send(303);
	delay(1);
	if( web.need_save ) save_config_alarms();
	if(fl_playStarted) {
		mp3_stop();
		fl_playStarted = false;
	}
	initRString(txt_alarmOn[gs.language]);
}

// отключение будильника
void off_alarm() {
	if(is_no_auth()) return;
	uint8_t target = 0;
	String name = "t";
	if( HTTP.hasArg(name) ) {
		target = HTTP.arg(name).toInt();
		if( alarms[target].settings & 512 ) {
			alarms[target].settings &= ~(512U);
			save_config_alarms();
			text_send_P(one);
			initRString(txt_alarmOff[gs.language]);
		}
	} else
		text_send_P(zero);
}

// сохранение настроек бегущей строки. Строки настраиваются по одной.
void save_text() {
	if(is_no_auth()) return;
	web.need_save = false;
	uint8_t target = 0;
	uint16_t settings = 512;
	String name = F("target");
	if( HTTP.hasArg(name) ) {
		target = HTTP.arg(name).toInt();
		web.to_string(F("text"), texts[target].text);
		if( web.to_int(F("period"), texts[target].period, 30, 3600) )
			textTimer[target].setInterval(texts[target].period*1000U);
		web.to_int(F("color_mode"), texts[target].color_mode, 0, 5);
		web.color(F("color"), texts[target].color);
		name = F("rmode");
		if( HTTP.hasArg(name) ) settings |= constrain(HTTP.arg(name).toInt(), 0, 3) << 7;
		if( HTTP.hasArg("Mo") ) settings |= 2;
		if( HTTP.hasArg("Tu") ) settings |= 4;
		if( HTTP.hasArg("We") ) settings |= 8;
		if( HTTP.hasArg("Th") ) settings |= 16;
		if( HTTP.hasArg("Fr") ) settings |= 32;
		if( HTTP.hasArg("Sa") ) settings |= 64;
		if( HTTP.hasArg("Su") ) settings |= 1;
		if((settings >> 7 & 3) == 3) { // если режим "до конца дня", то записать текущий день
			tm t = getTime();
			settings |= t.tm_mday << 10;
		} else { // иначе то, что передано формой
			name = "sel_day";
			if( HTTP.hasArg(name) ) settings |= constrain(HTTP.arg(name).toInt(), 1, 31) << 10;
		}
		if( settings != texts[target].repeat_mode ) {
			texts[target].repeat_mode = settings;
			web.need_save = true;
		}
	}
	HTTP.sendHeader(F("Location"),F("/running.html"));
	HTTP.send(303);
	delay(1);
	if( web.need_save ) save_config_texts();
	initRString(txt_textOn[gs.language]);
}

// отключение бегущей строки
void off_text() {
	if(is_no_auth()) return;
	uint8_t target = 0;
	String name = "t";
	if( HTTP.hasArg(name) ) {
		target = HTTP.arg(name).toInt();
		if( texts[target].repeat_mode & 512 ) {
			texts[target].repeat_mode &= ~(512U);
			save_config_texts();
			text_send_P(one);
			initRString(txt_textOff[gs.language]);
		}
	} else
		text_send_P(zero);
}

// костыль для настройки режима повтора. Работает 50/50
void repeat_mode(uint8_t r) {
	static uint8_t old_r = 0;
	if(r==0) {
		if(old_r==1) mp3_disableLoop();
		if(old_r==2) mp3_disableLoopAll();
		// mp3_stop();
	}
	if(r==1) {
		if(old_r==2) mp3_disableLoopAll();
		mp3_enableLoop();
	}
	if(r==2) {
		if(old_r==1) mp3_disableLoop();
		mp3_enableLoopAll();
	}
	if(r==3) mp3_randomAll();
	old_r = r;
}

// обслуживает страничку плейера.
void play() {
	if(is_no_auth()) return;
	uint8_t p = 0; // режим, который надо установить: играть, стоп, переключение и пр.
	uint8_t r = 0; // режим повтора: 0 нет, 1 повторять трек, 2 повторять все, 3 играть случайный трек
	uint8_t v = 15; // уровень громкости
	uint16_t c = 1; // номер трека
	uint8_t f = 0; // номер папки из которой надо запустить трек. Если 0, то глобальный номер
	static uint16_t in_folder = 0;
	int t = 0;
	String name = "p";
	if( HTTP.hasArg(name) ) p = HTTP.arg(name).toInt();
	name = "c";
	if( HTTP.hasArg(name) ) c = constrain(HTTP.arg(name).toInt(), 1, mp3_all);
	name = "r";
	if( HTTP.hasArg(name) ) r = constrain(HTTP.arg(name).toInt(), 0, 3);
	name = "v";
	if( HTTP.hasArg(name) ) v = constrain(HTTP.arg(name).toInt(), 1, 30);
	name = "f";
	if( HTTP.hasArg(name) ) f = constrain(HTTP.arg(name).toInt(), 0, 255);
	switch (p)	{
		case 1: // предыдущий трек
			t = mp3_current - 1;
			if(t<1) t=mp3_all;
			mp3_play(t);
			fl_playStarted = true;
			break;
		case 2: // следующий трек
			t = mp3_current + 1;
			if(t>mp3_all) t=1;
			mp3_play(t);
			fl_playStarted = true;
			break;
		case 3: // играть
			repeat_mode(r);
			delay(10);
			if (v != cur_Volume)
				mp3_volume(v);
			if (f) {
				in_folder = mp3_folderTrackCount(f);
				mp3_playInFolder(f, c);
			} else
				mp3_play(c);
			fl_playStarted = true;
			break;
		case 4: // пауза
			mp3_pause();
			break;
		case 5: // режим
			repeat_mode(r);
			break;
		case 6: // остановить
			mp3_stop();
			fl_playStarted = false;
			break;
		case 7: // тише
			t = cur_Volume - 1;
			if(t<0) t=0;
			mp3_volume(t);
			break;
		case 8: // громче
			t = cur_Volume + 1;
			if(t>30) t=30;
			mp3_volume(t);
			break;
		case 9: // громкость
			mp3_volume(v);
			break;
		default:
			// if(!mp3_isInit) mp3_init();
			// else mp3_reread();
			// if(mp3_isInit) mp3_update();
			if (!f) mp3_reread();
			mp3_update();
			break;
	}
	char buff[20];
	sprintf_P(buff, PSTR("%i:%i:%i:%i"), mp3_current, (f?in_folder:mp3_all), cur_Volume, mp3_isPlay());
	text_send(String(buff));
}

// "proxy" отправка сообщений в телеграм через web запрос, для сторонних устройств
void send() {
	// if(is_no_auth()) return;
	String name = F("pin");
	if( HTTP.hasArg(name) ) {
		if( ts.pin_code == HTTP.arg(name) ) {
			name = F("msg");
			if( HTTP.hasArg(name) ) {
				tb_send_msg(HTTP.arg(name));
				text_send_P(one);
				return;
			}
		}
	}
	text_send_P(zero);
}

// Установка времени. Для крайних случаев, когда интернет отсутствует
void set_clock() {
	if(is_no_auth()) return;
	uint8_t type=0;
	String name = "t";
	if(HTTP.hasArg(name)) {
		struct tm tm;
		type = HTTP.arg(name).toInt();
		if(type==0 || type==1) {
			name = F("time");
			if(HTTP.hasArg(name)) {
				size_t pos = HTTP.arg(name).indexOf(":");
				tm.tm_hour = constrain(HTTP.arg(name).toInt(), 0, 23);
				tm.tm_min = constrain(HTTP.arg(name).substring(pos+1).toInt(), 0, 59);
				name = F("date");
				if(HTTP.hasArg(name)) {
					size_t pos = HTTP.arg(name).indexOf("-");
					tm.tm_year = constrain(HTTP.arg(name).toInt()-1900, 0, 65000);
					tm.tm_mon = constrain(HTTP.arg(name).substring(pos+1).toInt()-1, 0, 11);
					size_t pos2 = HTTP.arg(name).substring(pos+1).indexOf("-");
					tm.tm_mday = constrain(HTTP.arg(name).substring(pos+pos2+2).toInt(), 1, 31);
					name = F("sec");
					if(HTTP.hasArg(name)) {
						tm.tm_sec = constrain(HTTP.arg(name).toInt()+1, 0, 60);
						tm.tm_isdst = gs.tz_dst;
						time_t t = mktime(&tm);
						LOG(printf_P,"web time: %llu\n",t);
						// set the system time
						timeval tv = { t, 0 };
						settimeofday(&tv, nullptr);
					}
				}
			}
		} else {
			syncTime();
		}
		HTTP.sendHeader(F("Location"),F("/maintenance.html"));
		HTTP.send(303);
		delay(1);
	} else {
		tm t = getTime();
		HTTP.client().print(PSTR("HTTP/1.1 200\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{"));
		HPP("\"time\":\"%u\",", t.tm_hour*60+t.tm_min);
		HPP("\"date\":\"%u-%02u-%02u\"}", t.tm_year +1900, t.tm_mon +1, t.tm_mday);
		#ifdef ESP8266
		HTTP.client().stop();
		#endif
	}
}

// Включение/выключение различных режимов
void onoff() {
	if(is_no_auth()) return;
	int8_t a=0;
	bool cond=false;
	String name = "a";
	if(HTTP.hasArg(name)) a = HTTP.arg(name).toInt();
	name = "t";
	if(HTTP.hasArg(name)) {
		if(HTTP.arg(name) == F("demo")) {
			// включает/выключает демо режим
			if(a) fl_demo = !fl_demo;
			cond = fl_demo;
		} else 
		if(HTTP.arg(name) == F("ftp")) {
			// включает/выключает ftp сервер, чтобы не кушал ресурсов просто так
			if(a) ftp_isAllow = !ftp_isAllow;
			cond = ftp_isAllow;
		} else
		if(HTTP.arg(name) == F("security")) {
			// включает/выключает режим "охраны"
			if(a) sec_enable = !(bool)sec_enable;
			cond = sec_enable;
			if(a) {
				save_log_file(cond?SEC_TEXT_ENABLE:SEC_TEXT_DISABLE);
				save_config_security();
			}
		}
	}
	text_send_P(cond?one:zero);
}

#ifdef ESP32
const char* print_full_platform_info(char* buf) {
	int p = 0;
	const char *cpu;
	switch (chip_info.model) {
		case 1: // ESP32
			cpu = "ESP32";
			break;
		case 2: // ESP32-S2
			cpu = "ESP32-S2";
			break;
		case 9: // ESP32-S3
			cpu = "ESP32-S4";
			break;
		case 5: // ESP32-C3
			cpu = "ESP32-C3";
			break;
		case 6: // ESP32-H2
			cpu = "ESP32-H2";
			break;
		case 12: // ESP32-C2
			cpu = "ESP32-C2";
			break;
        case 13: // ESP32-C6
			cpu = "ESP32-C6";
			break;
        case 16: // ESP32-H2
			cpu = "ESP32-H2";
			break;
        case 18: // ESP32-P4
			cpu = "ESP32-P4";
			break;
		case 20: // ESP32-C61
			cpu = "ESP32-C61";
			break;
		case 23: // ESP32-C5
			cpu = "ESP32-C5";
			break;
		case 25: // ESP32-H21
			cpu = "ESP32-H21";
			break;
		case 28: // ESP32-H4
			cpu = "ESP32-H4";
			break;
		default:
			cpu = "unknown";
	}
	p = sprintf(buf, "Chip:%s_r%u/", cpu, chip_info.revision);
	p += sprintf(buf+p, "Cores:%u/%s", chip_info.cores, ESP.getSdkVersion());
	return buf;
}

// декодирование информации о причине перезагрузки ядра
const char* print_reset_reason(char *buf) {
	int p = 0;
	uint8_t old_reason = 127;
	const char *res;
	for(int i=0; i<chip_info.cores; i++) {
		uint8_t reason = rtc_get_reset_reason(i);
		if( old_reason != reason ) {
			old_reason = reason;
			if( p ) p += sprintf(buf+p, ", ");
			switch ( reason ) {
				case 1 : res = "PowerON"; break;        	       /**<1, Vbat power on reset*/
				case 3 : res = "SW_RESET"; break;               /**<3, Software reset digital core*/
				case 4 : res = "OWDT_RESET"; break;             /**<4, Legacy watch dog reset digital core*/
				case 5 : res = "DeepSleep"; break;              /**<5, Deep Sleep reset digital core*/
				case 6 : res = "SDIO_RESET"; break;             /**<6, Reset by SLC module, reset digital core*/
				case 7 : res = "TG0WDT_SYS_RESET"; break;       /**<7, Timer Group0 Watch dog reset digital core*/
				case 8 : res = "TG1WDT_SYS_RESET"; break;       /**<8, Timer Group1 Watch dog reset digital core*/
				case 9 : res = "RTCWDT_SYS_RESET"; break;       /**<9, RTC Watch dog Reset digital core*/
				case 10 : res = "INTRUSION_RESET"; break;       /**<10, Intrusion tested to reset CPU*/
				case 11 : res = "TGWDT_CPU_RESET"; break;       /**<11, Time Group reset CPU*/
				case 12 : res = "SW_CPU_RESET"; break;          /**<12, Software reset CPU*/
				case 13 : res = "RTCWDT_CPU_RESET"; break;      /**<13, RTC Watch dog Reset CPU*/
				case 14 : res = "EXT_CPU_RESET"; break;         /**<14, for APP CPU, reseted by PRO CPU*/
				case 15 : res = "RTCWDT_BROWN_OUT"; break;	    /**<15, Reset when the vdd voltage is not stable*/
				case 16 : res = "RTCWDT_RTC_RESET"; break;      /**<16, RTC Watch dog reset digital core and rtc module*/
				default : res = "NO_MEAN";
			}
			p += sprintf(buf+p, "%s", res);
		}
	}
	return buf;
}
#endif

// Информация о состоянии железки
void sysinfo() {
	if(is_no_auth()) return;
	char buf[100];
	HTTP.client().print(PSTR("HTTP/1.1 200\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n{"));
	HPP("\"Uptime\":\"%s\",", getUptime(buf));
	HPP("\"Time\":\"%s\",", clockCurrentText(buf));
	HPP("\"Date\":\"%s\",", dateCurrentTextLong(buf));
	HPP("\"Sunrise\":\"%u:%02u\",", sunrise / 60, sunrise % 60);
	HPP("\"Sunset\":\"%u:%02u\",", sunset / 60, sunset % 60);
	HPP("\"Illumination\":%i,", analogRead(PIN_PHOTO_SENSOR));
	HPP("\"LedBrightness\":%i,", led_brightness);
	HPP("\"fl_5v\":%i,", fl_5v);
	HPP("\"Rssi\":%i,", wifi_rssi());
	HPP("\"IP\":\"%s\",", wifi_currentIP().c_str());
	HPP("\"Barometer\":%u,", fl_barometerIsInit);
	HPP("\"RTC\":%u,", rtc_enable);
	HPP("\"TimeDrift\":%i,", rtc_enable ? getTimeU() - (gs.tz_shift+gs.tz_dst)*3600 - getRTCTimeU(): 0);
	HPP("\"NVRAM\":%u,", nvram_enable ? eeprom_chip: 0);
	HPP("\"FreeHeap\":%i,", ESP.getFreeHeap());
	#ifdef ESP32
	HPP("\"MaxFreeBlockSize\":%i,", ESP.getMaxAllocHeap());
	HPP("\"HeapFragmentation\":%i,", 100-ESP.getMaxAllocHeap()*100/ESP.getFreeHeap());
	HPP("\"ResetReason\":\"%s\",", print_reset_reason(buf));
	HPP("\"FullVersion\":\"%s\",", print_full_platform_info(buf));
	#else // ESP8266
	HPP("\"MaxFreeBlockSize\":%i,", ESP.getMaxFreeBlockSize());
	HPP("\"HeapFragmentation\":%i,", ESP.getHeapFragmentation());
	HPP("\"ResetReason\":\"%s\",", ESP.getResetReason().c_str());
	HPP("\"FullVersion\":\"%s\",", ESP.getFullVersion().c_str());
	#endif
	HPP("\"CpuFreqMHz\":%i,", ESP.getCpuFreqMHz());
	HPP("\"BuildTime\":\"%s %s\"}", PSTR(__DATE__), PSTR(__TIME__));
	#ifdef ESP8266
	HTTP.client().stop();
	#endif
}

cur_sensor sensor[MAX_SENSORS];

// Информация о зарегистрированных сенсорах
void sensors() {
	if(is_no_auth()) return;
	bool fl = false;
	HTTP.client().print(PSTR("HTTP/1.1 200\r\nContent-Type: application/json\r\nConnection: close\r\n\r\n["));
	for(uint8_t i=0; i<MAX_SENSORS; i++) {
		if(sensor[i].registered >= getTimeU() - ts.sensor_timeout*60 + 60) {
			if(fl) HTTP.client().print(",");
			HPP("{\"num\":%i,", i);
			HPP("\"hostname\":\"%s\",", StringConverters::jsonEscape(sensor[i].hostname).c_str());
			HPP("\"ip\":\"%s\",", sensor[i].ip.toString().c_str());
			HPP("\"timeout\":%i}", ts.sensor_timeout*60 + sensor[i].registered - getTimeU());
			fl = true;
		}
	}
	HTTP.client().print("]");
	#ifdef ESP8266
	HTTP.client().stop();
	#endif
}

// Регистрация сенсора
void registration() {
	// Авторизация примитивная, через shared key, или pin
	// должно быть всего два параметра: pin (shared key) и name - имя устройства
	String name = F("pin");
	if( HTTP.hasArg(name) ) {
		if( ts.pin_code == HTTP.arg(name) ) {
			// pin подошел
			name = F("name");
			if( HTTP.hasArg(name) ) {
				IPAddress sensor_ip = HTTP.client().remoteIP();
				bool fl_found = false;
				// проверка, зарегистрирован ли уже этот ip
				for(uint8_t i=0; i<MAX_SENSORS; i++) {
					if(sensor[i].ip == sensor_ip) {
						// надо ли обновить hostname
						if(sensor[i].hostname != HTTP.arg(name))
							sensor[i].hostname = HTTP.arg(name);
						// обновить время регистрации
						sensor[i].registered = getTimeU();
						fl_found = true;
						break;
					}
				}
				if(!fl_found) {
					// новый сенсор, поиск свободной ячейки
					for(uint8_t i=0; i<MAX_SENSORS; i++) {
						if(sensor[i].registered < getTimeU() - ts.sensor_timeout*60 - 60) {
							// найдена свободная ячейка
							sensor[i].hostname = HTTP.arg(name);
							sensor[i].ip = sensor_ip;
							sensor[i].registered = getTimeU();
							fl_found = true;
							break;
						}
					}
				}
				if(fl_found) {
					LOG(printf_P,PSTR("registered \"%s\" ip %s\n"), HTTP.arg(name).c_str(), sensor_ip.toString().c_str());
					text_send_P(one);
					return;
				}
			}
		}
	}
	// любая ошибка, в том числе закончились свободные ячейки
	text_send_P(zero);
}

// сохранение настроек цитат
void save_quote() {
	if(is_no_auth()) return;
	web.need_save = false;
	bool fl_change_color = false;

	if(web.checkbox(F("enabled"), qs.enabled)) {
		// если цитаты отключили, то сбросить текущую цитату 
		if( qs.enabled == 0 ) messages[MESSAGE_QUOTE].count = 0;
	}
	if(web.to_int(F("period"), qs.period, 120, 3600))
		messages[MESSAGE_QUOTE].timer.setInterval(1000U * qs.period);
	if(web.to_int(F("update"), qs.update, 0, 3))
		quoteUpdateTimer.setInterval(900000U * (qs.update+1));
	if(web.to_int(F("color_mode"), qs.color_mode, 0, 4))
		fl_change_color = true;
	if(web.color(F("color"), qs.color))
		fl_change_color = true;
	web.to_int(F("server"), qs.server, 0, 2);
	web.to_int(F("lang"), qs.lang, 0, 3);
	web.to_string(F("url"), qs.url);
	web.to_string(F("params"), qs.params);
	web.to_int(F("method"), qs.method, 0, 1);
	web.to_int(F("type"), qs.type, 0, 2);
	web.to_string(F("quote_field"), qs.quote_field);
	web.to_string(F("author_field"), qs.author_field);

	if(fl_change_color) messages[MESSAGE_QUOTE].color = qs.color_mode > 0 ? qs.color_mode: qs.color;

	HTTP.sendHeader(F("Location"),"/");
	HTTP.send(303);
	delay(1);
	if( web.need_save ) {
		save_config_quote();
		quote.fl_init = false;
	}
	// initRString(PSTR("Настройки сохранены"));
	printTinyText(txt_save,1,9);
}

void show_quote() {
	if(is_no_auth()) return;
	text_send(messages[MESSAGE_QUOTE].text);
}

const char help[] PROGMEM = R"""(
(h)elp - this help
(m)sg="text" or (t)ext="text" - show this "text" on the matrix
(c)nt=5 - show 5 times (2 by default, max 100)
(i)nt=60 - show with interval 60 seconds (30 by default, max 600)
(d)ue=1 - color mode (1-rainbow or 2-different) or color in hex (FFFFFF or FFF)
)""";

void show() {
	bool fl_web = false;
	bool fl_msg = false;
	bool fl_cnt = false;
	bool fl_int = false;
	bool fl_due = false;
	bool cond = false;
	int num_args = HTTP.args();
	if(num_args==0) {
		HTTP.send_P(200, PSTR("text/plain"), help);
		LOG(println, F("show: send help"));
		return;
	}
	for(int i=0; i<num_args; i++) {
		String arg_name = HTTP.argName(i);
		if(arg_name.startsWith("h")) {
			HTTP.send_P(200, PSTR("text/plain"), help);
			LOG(println, F("show: send help"));
		}
		if(arg_name.startsWith("m") || arg_name.startsWith("t")) {
			messages[MESSAGE_WEB].text = HTTP.arg(i);
			if(messages[MESSAGE_WEB].text.length() > 1) fl_msg = true;
		}
		if(arg_name.startsWith("c")) {
			messages[MESSAGE_WEB].count = constrain(HTTP.arg(i).toInt(), 1, 100);
			fl_cnt = true;
		}
		if(arg_name.startsWith("i")) {
			messages[MESSAGE_WEB].timer.setInterval(constrain(HTTP.arg(i).toInt()*1000, 15000, 600000));
			fl_int = true;
		}
		if(arg_name.startsWith("d")) {
			int color = HTTP.arg(i).toInt();
			if( color > 0 && color < 6 ) {
				messages[MESSAGE_WEB].color = color;
			} else {
				messages[MESSAGE_WEB].color = StringConverters::text_to_color(HTTP.arg(i).c_str());
			}
			fl_due = true;
		}
		if(arg_name.startsWith("w")) {
			fl_web = true;
		}
	}
	if( fl_msg ) {
		if( ! fl_cnt ) messages[MESSAGE_WEB].count = 2;
		if( ! fl_int ) messages[MESSAGE_WEB].timer.setInterval(30000);
		messages[MESSAGE_WEB].timer.setNext(100);
		if( ! fl_due ) messages[MESSAGE_WEB].color = 1;
		cond = true;
	} else
		messages[MESSAGE_WEB].count = 0;
	if( fl_web ) {
		HTTP.sendHeader(F("Location"),"/maintenance.html");
		HTTP.send(303);
		delay(1);
	} else
		text_send_P(cond?one:zero);
}

void save_weather() {
	if(is_no_auth()) return;
	web.need_save = false;
	bool need_weather = false;
	bool need_forecast = false;
	bool fl_change_color = false;
	bool fl_change_colorF = false;

	web.checkbox(F("sensors"), ws.sensors);
	web.to_int(F("term_period"), ws.term_period, 20, 60000);
	web.to_int(F("term_color_mode"), ws.term_color_mode, 0, 4);
	web.color(F("term_color"), ws.term_color);
	web.checkbox(F("tiny_term"), ws.tiny_term);
	web.to_float(F("term_cor"), ws.term_cor, -100.0f, 100.0f, 1.0f);
	web.to_int(F("bar_cor"), ws.bar_cor, -1000, 1000);
	web.to_int(F("term_pool"), ws.term_pool, 30, 600);
	need_weather = web.checkbox(F("weather"), ws.weather);
	if(web.to_int(F("sync_weather_period"), ws.sync_weather_period, 15, 1439))
		syncWeatherTimer.setInterval(60000U * ws.sync_weather_period);
	if(web.to_int(F("show_weather_period"), ws.show_weather_period, 30, 3600))
		messages[MESSAGE_WEATHER].timer.setInterval(1000U * ws.show_weather_period);
	if(web.to_int(F("color_mode"), ws.color_mode, 0, 4))
		fl_change_color = true;
	if(web.color(F("color"), ws.color))
		fl_change_color = true;
	web.checkbox(F("weather_code"), ws.weather_code);
	web.checkbox(F("temperature"), ws.temperature);
	web.checkbox(F("a_temperature"), ws.a_temperature);
	web.checkbox(F("humidity"), ws.humidity);
	web.checkbox(F("cloud"), ws.cloud);
	web.checkbox(F("pressure"), ws.pressure);
	web.checkbox(F("wind_speed"), ws.wind_speed);
	web.checkbox(F("wind_direction"), ws.wind_direction);
	web.checkbox(F("wind_direction2"), ws.wind_direction2);
	web.checkbox(F("wind_gusts"), ws.wind_gusts);
	web.checkbox(F("pressure_dir"), ws.pressure_dir);
	if( web.to_int(F("altitude"), ws.altitude, -1000, 12000) )
		forecaster_setH(ws.altitude);
	need_forecast = web.checkbox(F("forecast"), ws.forecast);
	web.to_int(F("forecast_days"), ws.forecast_days, 1, FORECAST_DAYS);
	if(web.to_int(F("sync_forecast_period"), ws.sync_forecast_period, 1, 12))
		syncForecastTimer.setInterval(3600000U * ws.sync_forecast_period);
	if(web.to_int(F("show_forecast_period"), ws.show_forecast_period, 30, 3600))
		messages[MESSAGE_FORECAST].timer.setInterval(1000U * ws.show_forecast_period);
	if(web.to_int(F("color_modeF"), ws.color_modeF, 0, 4))
		fl_change_colorF = true;
	if(web.color(F("colorF"), ws.colorF))
		fl_change_colorF = true;
	web.checkbox(F("weather_codeF"), ws.weather_codeF);
	web.checkbox(F("temperatureF"), ws.temperatureF);
	web.checkbox(F("wind_speedF"), ws.wind_speedF);
	web.checkbox(F("wind_directionF"), ws.wind_directionF);

	if(fl_change_color) messages[MESSAGE_WEATHER].color = ws.color_mode > 0 ? ws.color_mode: ws.color;
	if(fl_change_colorF) messages[MESSAGE_FORECAST].color = ws.color_modeF > 0 ? ws.color_modeF: ws.colorF;

	HTTP.sendHeader(F("Location"),"/");
	HTTP.send(303);
	delay(1);
	if( web.need_save ) {
		save_config_weather();
		if( ws.weather ) {
			if( need_weather ) syncWeatherTimer.setNext(50);
			char txt[512];
			messages[MESSAGE_WEATHER].text = String(generate_weather_string(txt));
		} else {
			messages[MESSAGE_WEATHER].count = 0;
		}
		if( ws.forecast ) {
			if( need_forecast ) syncForecastTimer.setNext(1000);
			char txt[512];
			messages[MESSAGE_FORECAST].text = String(generate_forecast_string(txt));
		} else {
			messages[MESSAGE_FORECAST].count = 0;
		}
	}
	// initRString(PSTR("Настройки сохранены"));
	printTinyText(txt_save,1,9);
}

void show_sensors() {
	if(is_no_auth()) return;
	char txt[100];
	currentPressureTemp(txt);
	text_send(String(txt));
}

void show_weather() {
	if(is_no_auth()) return;
	char txt[512];
	text_send(String(generate_weather_string(txt)));
}

void show_forecast() {
	if(is_no_auth()) return;
	char txt[512];
	text_send(String(generate_forecast_string(txt)));
}

void show_status() {
	String hostname = StringConverters::jsonEscape(gs.clock_name);
	char buf[150+hostname.length()] = {0};

	char *next = buf;
	#define HPP2(txt, ...) next+=snprintf_P(next, sizeof(buf)-(next-buf+1), PSTR(txt), __VA_ARGS__)

	HPP2("\"hostname\":\"%s\",", hostname.c_str());
	HPP2("\"is_auth\":%i,", HTTP.authenticate(gs.web_login.c_str(), gs.web_password.c_str()) && gs.web_password.length() > 0 ? 1 : 0);
	HPP2("\"use_i2c\":%i,", USE_I2C);
	HPP2("\"use_rtc\":%i,", USE_RTC);
	HPP2("\"use_nvram\":%i,", USE_NVRAM);
	HPP2("\"use_bmp\":%i,", USE_BMP);
	HPP2("\"use_mp3\":%i,",
		#ifdef SRX
		1 
		#else
		0 
		#endif
	);
	HPP2("\"lang\":\"%s\"}", TXT_LANGUAGE[gs.language]);

	size_t total = next-buf;
	#undef HPP2

	HPP("HTTP/1.1 200\r\nContent-Type: application/json\r\nContent-Length: %d\r\nConnection: close\r\n\r\n{", total+1);
	HTTP.client().write((const char*)buf, total);
}

void save_cuckoo () {
	if(is_no_auth()) return;
	web.need_save = false;

	web.checkbox(F("enable"), cs.enable);
	web.to_int(F("folder"), cs.folder, 1, 99);
	web.to_int(F("zero"), cs.zero, 0, 255);
	web.to_int(F("cuckoo"), cs.cuckoo, 0, 255);
	web.to_int(F("vol"), cs.volume, 1, 30);

	HTTP.sendHeader(F("Location"),F("/maintenance.html"));
	HTTP.send(303);
	delay(1);
	if( web.need_save )	save_config_cuckoo();
	printTinyText(txt_save,1,9);
}