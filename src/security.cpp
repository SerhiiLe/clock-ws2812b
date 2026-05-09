/*
	"охрана"
	информирование о срабатывании датчика движения
	работа с telegram
*/

#include <Arduino.h>
#include <FastBot.h>
#include <WiFiClient.h>
#ifdef ESP32
#include <HTTPClient.h>
#else // ESP8266
#include <ESP8266HTTPClient.h>
#endif
#include "security.h"
#include "defines.h"
#include "settings.h"
#include "ntp.h"

#define MAX_MESSAGES 10
#define FB_DYNAMIC_HTTP

FastBot tb;

// 0 - ожидание
// 1 - ОК
// 2 - Переполнен по ovf
// 3 - Ошибка телеграм
// 4 - Ошибка подключения
// 5 - не задан chat ID
// 6 - множественная отправка, статус неизвестен

void inMsg(FB_msg& msg);
bool fl_secretWanted = false;
time_t last_telegram = 0;
time_t disable_telegram = 0;
int8_t pinned = -1;

// Установка токена и списка подписанных чатов
void setup_telegram() {
	// tb.setChatID(tb_chats); // не нужно, так как передаётся при каждой отправке
	tb.setToken(ts.tb_token);
}

// Инициализация бота
void init_telegram() {
	setup_telegram();
	tb.setLimit(MAX_MESSAGES);
	tb.setTextMode(FB_TEXT);
	// tb.setPeriod(1000U * tb_rate);
	tb.attach(inMsg);
}

// Опрос телеграмм в ожидании команд для обработки
void tb_tick() {
	if(disable_telegram) {
		if(getTimeU() - disable_telegram > ts.tb_ban) disable_telegram = 0;
	} else {
		// проверка времени ускорения работы telegram
		if(last_telegram)
			if(getTimeU() - last_telegram > ts.tb_accelerate) {
				telegramTimer.setInterval(1000U * ts.tb_rate);
				last_telegram = 0;
			}
		// собственно сам запрос новых сообщений телеграм
		if( tb.tickManual() != 1 ) disable_telegram = getTimeU();
	}
}

// Введение отправки сообщения в один поток с другими сетевыми операциями
String delayedMessage = "";
void tb_send_msg(String s) {
	delayedMessage = s;
}

// Отправка сообщения о сработке датчика во все подписанные чаты телеграмм
void tb_send_delayed() {
	if (!delayedMessage.isEmpty()) {
		#ifdef DEBUG
		LOG(printf_P, PSTR("Send to telegram: %i\n"), tb.sendMessage(delayedMessage,ts.tb_chats));
		#else
		tb.sendMessage(delayedMessage,ts.tb_chats);
		#endif
		delayedMessage = "";
	}
}

// кодирование строки для GET запросов
String urlEncode(String str, bool params = false) {
	unsigned int len = str.length();
	int buffer_size = len * 3 < TELEGRAM_MAX_LENGTH ? len * 3: TELEGRAM_MAX_LENGTH;
	char* encodedString = (char*) malloc(buffer_size * sizeof(char));
	char* p = encodedString;
	char c;
	char code0;
	char code1;
	for(unsigned int i=0; i < len; i++) {
		if(buffer_size+encodedString-p < 3) break;
		c=str.charAt(i);
		if(params && (c == '&' || c == '=')) {
			*p++ = c;
		} else
		if(isalnum(c)) {
			*p++ = c;
		} else {
			code1=(c & 0xf)+'0';
			if((c & 0xf) > 9) {
				code1 = (c & 0xf) - 10 + 'A';
			}
			c = (c>>4) & 0xf;
			code0 = c+'0';
			if(c > 9) {
				code0=c - 10 + 'A';
			}
			*p++ = '%';
			*p++ = code0;
			*p++ = code1;
		}
	}
	*p = 0;
	String result = String(encodedString);
	free(encodedString);
	return result;
}

// Обработка входящего сообщения телеграмм
void inMsg(FB_msg& msg) {
	char buf[100];

	// выводим ID чата, имя юзера и текст сообщения
	LOG(printf_P, PSTR("From telegram:%s;%s;%s;%li;%s.\n"),msg.chatID.c_str(),msg.username.c_str(),msg.first_name.c_str(),msg.ID,msg.text.c_str());

	if(ts.tb_rate > ts.tb_accelerated)
		telegramTimer.setInterval(1000U * ts.tb_accelerated);
	last_telegram = getTimeU();

	msg.text.trim();

	bool fl_start = false;
	if(fl_secretWanted) {
		fl_secretWanted = false;
		if(msg.text == ts.tb_secret) {
			tb.deleteMessage(0, msg.chatID);
			tb.deleteMessage(1, msg.chatID);
			tb.sendMessage(F("Добро пожаловать!"), msg.chatID);
			if(ts.tb_chats.length()>0) ts.tb_chats += ",";
			ts.tb_chats += msg.chatID;
			save_config_main();
		} else {
			tb.deleteMessage(0, msg.chatID);
			tb.deleteMessage(1, msg.chatID);
			tb.sendMessage(F("Ошибка!"), msg.chatID);
			return;
		}
		fl_start = true;
	}

	msg.text.toLowerCase();

	bool fl_auth = (ts.tb_chats.length() > 0 && ts.tb_chats.indexOf(msg.chatID) >= 0);
	// bool fl_auth = true;
	// // проверка авторизован ли этот чат
	// if(ts.tb_chats.length() == 0) fl_auth = false;
	// if(ts.tb_chats.length() > 0 && ts.tb_chats.indexOf(msg.chatID) < 0) fl_auth = false;


	if(fl_auth) {

		// альтернативный способ закрепления датчика
		if (msg.text[0] == '/' && isDigit(msg.text[1])) {
			String n = String(F("pin ")) + msg.text.substring(1);
			msg.text = n;
			LOG(println, String(F("rewrite to: ")) + msg.text);
		}

		// закрепление номера датчика
		if (msg.text.startsWith(F("pin"))) {
			int n = constrain(msg.text.substring(msg.text.lastIndexOf(" ")).toInt(), 0, MAX_SENSORS-1);
			if (sensor[n].registered >= getTimeU() - ts.sensor_timeout*60) {
				pinned = n;
				tb.showMenuText(String(F("Pinned ")) + String(pinned), F("Unpin\tHelp"), msg.chatID);
				return;
			} else {
				tb.sendMessage(F("format:\npin n\nn=0..9"), msg.chatID);
				return;
			}
		}

		// добавление номера датчика, если он закреплён
		if (pinned >= 0 && (! isDigit(msg.text[0] || msg.text.indexOf("*") > 0 ))) {
			String n = String(pinned) + " " + msg.text;
			msg.text = n;
			LOG(println, String(F("rewrite to: ")) + msg.text);
		}

		// открепить датчик
		if (msg.text.endsWith(F("unpin")) || msg.text == "/") {
			pinned = -1;
			msg.text = F("menu");
		} else
		if(msg.text.startsWith(F("last"))) {
			// ограничение в 48 строк лога из-за ограничение на стек в 2кб. (48*40=1920 + переменные)
			tb.sendMessage(read_log_file(constrain(msg.text.substring(msg.text.lastIndexOf(" ")).toInt(), 1, 48)), msg.chatID);
			return;
		} else
		if(msg.text == F("uptime")) {
			tb.sendMessage(getUptime(buf), msg.chatID);
			return;
		} else
		if(msg.text == F("on")) {
			tb.sendMessage(F("Отсылка сообщений включена."), msg.chatID);
			if(!sec_enable) {
				sec_enable = 1;
				save_log_file(SEC_TEXT_ENABLE);
				save_config_security();
			}
			return;
		} else
		if(msg.text == F("off")) {
			tb.sendMessage(F("Отсылка сообщений отключена."), msg.chatID);
			if(sec_enable) {
				sec_enable = 0;
				save_log_file(SEC_TEXT_DISABLE);
				save_config_security();
			}
			return;
		} else
		if(msg.text == F("status")) {
			sprintf_P(buf,PSTR("Датчик: %s.\nПитание: %s.\nОсвещение: %d -> %d."),
				sec_enable?F("включён"):F("отключён"),
				fl_5v?F("сеть"):F("аккумулятор"),
				analogRead(PIN_PHOTO_SENSOR), led_brightness);
			String sensors = buf;
			for(uint8_t i=0; i<MAX_SENSORS; i++) {
				if(sensor[i].registered >= getTimeU() - ts.sensor_timeout*60) {
					sensors += "\n" + String(i) + " " + sensor[i].hostname;
				}
			}
			tb.sendMessage(sensors, msg.chatID);
			return;
		} else
		if (isDigit(msg.text[0])) {
			// запрос внешнего датчика
			int8_t n = constrain(msg.text.toInt(), 0, MAX_SENSORS-1);
			if(sensor[n].registered >= getTimeU() - ts.sensor_timeout*60) {
				String url = String(F("http://")) + sensor[n].ip.toString() + String(F("/api?pin=")) + ts.pin_code + "&";
				int pos = 1;
				int len = msg.text.length();
				if(len<2) {
					url += F("help");
				} else {
					for(; pos<len; pos++)
						if(msg.text.charAt(pos) != ' ') break;
					int pos2 = msg.text.indexOf("=");
					bool fl_free_cmd = msg.text.indexOf(' ',pos) > 0 || msg.text.indexOf('*',pos) > 0;
					if (!fl_free_cmd && pos2>0) {
						url += msg.text.substring(pos,pos2+1);
						if(pos2+1 < len)
							url += urlEncode(msg.text.substring(pos2+1), true);
					} else {
						if (fl_free_cmd || msg.text[pos] == '/')
							url += F("cmd=");
						url += urlEncode(msg.text.substring(pos));
						if (msg.text[pos] != '/')
							url += "=";
					}
				}
				LOG(println, url);
				WiFiClient client;
				HTTPClient html;
				html.begin(client, url.c_str());
				int httpResponseCode = html.GET();
				if (httpResponseCode == 200) {
					// ответ от датчика запихивается сразу в telegram, обработку делает FastBot
					tb.sendMessage(sensor[n].hostname + "\n" + html.getString(), msg.chatID);
				} else {
					tb.sendMessage(sensor[n].hostname + " error: " + String(httpResponseCode), msg.chatID);
				}
		        // Free resources
		        html.end();
				return;
			} else {
				tb.sendMessage(F("датчик неактивен"), msg.chatID);
				return;
			}
		} else
		if(msg.text == F("logout")) {
			int pos1 = ts.tb_chats.indexOf(msg.chatID);
			if(pos1<0) {
				tb.sendMessage(F("err"), msg.chatID);
				return;
			}
			int pos2 = ts.tb_chats.indexOf(",", pos1);
			if(pos1>0) pos1--;
			String nt = ts.tb_chats.substring(0,pos1);
			if(pos2>0) nt = ts.tb_chats.substring(pos2);
			ts.tb_chats = nt;
			save_config_main();
			tb.sendMessage(F("Вышли..."), msg.chatID);
			fl_start = true;
			fl_auth = false;
		}
	} else {
 		if(msg.text == F("login")) {
			tb.sendMessage(F("Пароль доступа?"), msg.chatID);
			fl_secretWanted = true;
			return;
		}
	}
	if(fl_start || msg.text == F("start") || msg.text == F("menu")) {
		// показать юзер меню (\t - горизонтальное разделение кнопок, \n - вертикальное
		// bot.showMenu("Menu1 \t Menu2 \t Menu3 \n Menu4");
		// отправить инлайн меню (\t - горизонтальное разделение кнопок, \n - вертикальное
  		// (текст сообщения, кнопки)
  		// bot.inlineMenu("Choose wisely", "Answer 1 \t Answer 2 \t Answer 3 \n Answer 4");
		if(fl_auth)	tb.showMenuText(F("/help если надо"), F("On\tOff\tStatus\tShow ID\nLogout\tUptime\tLast 20\tHelp"), msg.chatID);
		else tb.showMenuText(F("/help если надо"), F("Login\tAbout\tShow ID"), msg.chatID);
		return;
	} else
	if(msg.text == F("stop")) {
		tb.closeMenu(msg.chatID);
		return;
	} else
	if(msg.text == F("show id")) {
		tb.sendMessage(msg.chatID, msg.chatID);
		return;
	} else
	if(msg.text == F("about")) {
		tb.sendMessage(F("Бот для управления датчиком движения в часах. Ничего интересного, можно проходить мимо."), msg.chatID);
		return;
	} else
	if(msg.text == F("help")) {
		tb.sendMessage(F(
			"Start - показать меню.\n"
			"Stop - спрятать меню.\n"
			"On/Off - включить/выключить режим охраны.\n"
			"Status - состояние и список доступных внешних датчиков.\n"
			"Show ID - id это чата.\n"
			"Login/Logout - авторизация.\n"
			"Uptime - время работы.\n"
			"Last - последние записи журнала. (1-45 - число записей)\n"
			"0-9 команда или 0-9 команда=значение - управление внешним датчиком\n"
			"0-9 help - запросить список команд у внешнего датчика по номеру"
			), msg.chatID);
		return;
	}
	tb.sendMessage(F("Что? Не понятно..."), msg.chatID);
}
