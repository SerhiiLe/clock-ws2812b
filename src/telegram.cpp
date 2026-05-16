/*
	"охрана"
	информирование о срабатывании датчика движения
	работа с telegram
*/

#include <Arduino.h>
#include "defines.h"
#include <TelegramESP.h>
#include <WiFiClient.h>
#ifdef ESP32
#include <HTTPClient.h>
#else // ESP8266
#include <ESP8266HTTPClient.h>
#endif
#include "telegram.h"
#include "settings.h"
#include "ntp.h"
#include "barometer.h"
#include "dfplayer.h"

TelegramESP bot;

String telegramCallback(TResult& msg);
bool fl_secretWanted = false;
time_t last_telegram = 0;
int8_t pinned = -1;

// Установка токена и списка подписанных чатов
void setup_telegram() {
	// bot.setChatID(bot.extractFirstNumber(ts.tb_chats)); // не нужно, так как передаётся при каждой отправке
	bot.setBotToken(ts.tb_token);
}

// Инициализация бота
void init_telegram() {
	setup_telegram();
	bot.attachCallback(telegramCallback);
}

// Опрос телеграмм в ожидании команд для обработки
void tb_tick() {
	// проверка времени ускорения работы telegram
	if(last_telegram)
		if(getTimeU() - last_telegram > ts.tb_accelerate) {
			telegramTimer.setInterval(ts.tb_rate * 1000UL);
			last_telegram = 0;
		}
	// собственно сам запрос новых сообщений телеграм
	if (bot.checkMessage(true) < 0) telegramTimer.setNext(ts.tb_ban * 1000UL);
}

void tb_real_send_message(const char* text) {
	#ifdef DEBUG
	// LOG(printf_P, PSTR("Send to telegram: %d\n"), bot.sendMessage(bot.extractFirstNumber(ts.tb_chats), text));
	LOG(printf_P, PSTR("Send to telegram: %d\n"), bot.sendMessageToAll(ts.tb_chats, text));
	#else
	// bot.sendMessage(bot.extractFirstNumber(ts.tb_chats), text);
	bot.sendMessageToAll(ts.tb_chats, text);
	#endif
}

#ifdef ESP32
// Для ESP32 отправка сообщений через очередь. Потому, что на этой платформе есть несколько потоков и надо все сообщения свести в один поток

// заведомо недостижимое число. В реальной жизни врядли будет занято больше одной ячейки
#define QUEUE_SIZE 20
QueueHandle_t strQueue = xQueueCreate(QUEUE_SIZE, sizeof(char*));
char* received = nullptr;

// Постановка сообщения в очередь на отправку
void tb_send_msg(const char* text) {          // или const String&
    if (text == nullptr) return;

    size_t len = strlen(text) + 1;
    char* copy = (char*)malloc(len);   // или  malloc(len); / pvPortMalloc(len);
    if (copy == nullptr) return;

    strcpy(copy, text);

    if (xQueueSend(strQueue, &copy, 0) != pdPASS) {
        free(copy);                    // не удалось отправить — освобождаем free(copy) / vPortFree(copy);
		return;
    }
}

// Отправка сообщений из очереди во все подписанные чаты телеграмм
void tb_send_delayed() {
	while (xQueueReceive(strQueue, &received, 0) == pdPASS) {
		if (received) {
			// LOG(printf, "To send: %s\n", received);
			tb_real_send_message(received);
			free(received);               // ОБЯЗАТЕЛЬНО освобождаем! free() / vPortFree(received)
			vTaskDelay(30);
		}
	}
}

#else
// Отправка сообщения о во все подписанные чаты телеграмм
void tb_send_msg(const char* text) {
	tb_real_send_message(text);
}
#endif
// Отправка сообщений (постановка в очередь)
void tb_send_msg(const String& text) {
	tb_send_msg(text.c_str());
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
String telegramCallback(TResult& msg) {
	char buf[100];

	// выводим ID чата, имя юзера и текст сообщения
	LOG(printf_P, PSTR("From telegram:%lld;%s;%lld;%s.\n"), msg.chatId, msg.from.c_str(), msg.messageId, msg.text.c_str());

	if(ts.tb_rate > ts.tb_accelerated)
		telegramTimer.setInterval(1000U * ts.tb_accelerated);
	last_telegram = getTimeU();

	String original = msg.text;
	msg.text.trim();

	bool fl_start = false;
	if(fl_secretWanted) {
		fl_secretWanted = false;
		if(msg.text == ts.tb_secret) {
			bot.deleteMessages(msg.chatId, {msg.messageId-1, msg.messageId});
			if(ts.tb_chats.length()>0) ts.tb_chats += ",";
			ts.tb_chats += String(msg.chatId);
			save_config_telegram();
			return F("Добро пожаловать!");
		} else {
			bot.deleteMessages(msg.chatId, {msg.messageId-1, msg.messageId});
			return F("Ошибка!");
		}
		fl_start = true;
	}

	msg.text.toLowerCase();

	// проверка авторизован ли этот чат
	bool fl_auth = bot.isWhitelisted(msg.chatId, ts.tb_chats);

	// функции доступные только для авторизированных
	if(fl_auth) {

		// альтернативный способ закрепления датчика
		if (msg.text[0] == '/' && isDigit(msg.text[1])) {
			String n = String(F("pin ")) + msg.text.substring(1);
			msg.text = n;
			LOG(println, String(F("rewrite to: ")) + msg.text);
		}

		// закрепление номера датчика
		if (bot.is_command(msg.text, F("pin"))) {
			int n = constrain(msg.text.substring(msg.text.lastIndexOf(" ")).toInt(), 0, MAX_SENSORS-1);
			if (sensor[n].registered >= getTimeU() - ts.sensor_timeout*60) {
				pinned = n;
				return String(F("Pinned ")) + String(pinned) + String(F("[MENU]Unpin\tHelp"));
			} else {
				return F("format:\npin n\nn=0..9");
			}
		}

		// добавление номера датчика, если он закреплён
		if (pinned >= 0 && (! isDigit(msg.text[0] || msg.text.indexOf("*") > 0 ))) {
			String n = String(pinned) + " " + msg.text;
			msg.text = n;
			LOG(println, String(F("rewrite to: ")) + msg.text);
		}

		// открепить датчик
		if (bot.is_command(msg.text, F("unpin")) || msg.text == "/") {
			pinned = -1;
			msg.text = F("menu");
		}

		if (bot.is_command(msg.text, F("last"))) {
			// ограничение в 48 строк лога из-за ограничение на стек в 2кб. (48*40=1920 + переменные)
			return read_log_file(constrain(msg.text.substring(msg.text.lastIndexOf(" ")).toInt(), 1, 48));
		}
		if (bot.is_command(msg.text, F("uptime"))) {
			return getUptime(buf);
		}
		if (bot.is_command(msg.text, F("on"))) {
			if(!sec_enable) {
				sec_enable = 1;
				save_log_file(SEC_TEXT_ENABLE);
				save_config_security();
			}
			return F("Отсылка сообщений включена.");
		}
		if (bot.is_command(msg.text, F("off"))) {
			if(sec_enable) {
				sec_enable = 0;
				save_log_file(SEC_TEXT_DISABLE);
				save_config_security();
			}
			return F("Отсылка сообщений отключена.");
		}
		if (bot.is_command(msg.text, F("status"))) {
			sprintf_P(buf,PSTR("Датчик: %s.\nПитание: %s.\nОсвещение: %d -> %d."),
				sec_enable?F("включён"):F("отключён"),
				fl_5v?F("сеть"):F("аккумулятор"),
				analogRead(PIN_PHOTO_SENSOR), led_brightness);
			String sensors = buf;
#if USE_I2C == 1
			sensors += String(F("\n")) + currentPressureTemp(buf);
#endif
			for(uint8_t i=0; i<MAX_SENSORS; i++) {
				if(sensor[i].registered >= getTimeU() - ts.sensor_timeout*60) {
					sensors += "\n/" + String(i) + " " + sensor[i].hostname;
				}
			}
			return sensors;
		}
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
				String res;
				if (httpResponseCode == 200) {
					// ответ от датчика запихивается сразу в telegram, обработку делает ArduinoJson
					res = sensor[n].hostname + "\n" + html.getString();
				} else {
					res = sensor[n].hostname + " error: " + String(httpResponseCode);
				}
		        // Free resources
		        html.end();
				return res;
			} else {
				return F("датчик неактивен");
			}
		}
		if (bot.is_command(msg.text, F("logout"))) {
			int pos1 = ts.tb_chats.indexOf(String(msg.chatId));
			if(pos1<0) {
				return F("error");
			}
			int pos2 = ts.tb_chats.indexOf(",", pos1);
			if(pos1>0) pos1--;
			String nt = ts.tb_chats.substring(0,pos1);
			if(pos2>0) nt = ts.tb_chats.substring(pos2);
			ts.tb_chats = nt;
			save_config_telegram();
			fl_start = true;
			fl_auth = false;
			return F("Вышли...");
		}
	} else {
 		if (bot.is_command(msg.text, F("login"))) {
			fl_secretWanted = true;
			return F("Пароль доступа?");
		}
	}
	if (fl_start || bot.is_command(msg.text, F("start")) || bot.is_command(msg.text, F("menu"))) {
		// показать юзер меню (\t - горизонтальное разделение кнопок, \n - вертикальное
  		// (текст сообщения, кнопки)
		if(fl_auth)	return F("/help если надо[MENU]On\tOff\tStatus\tChatID\nLogout\tUptime\tLast 20\tHelp");
		else return F("/help если надо[MENU]Login\tAbout\tChatID");
	}
	if (bot.is_command(msg.text, F("stop"))) {
		// убрать меню
		return F("start - open menu[MENU]");
	}
	if (bot.is_command(msg.text, F("chatid"))) {
		return String(msg.chatId);
	}
	if (bot.is_command(msg.text, F("about"))) {
		return F("Бот для управления датчиком движения в часах. Ничего интересного, можно проходить мимо.");
	}
	if (bot.is_command(msg.text, F("help"))) {
		return F(
			"/Start - показать меню.\n"
			"/Stop - спрятать меню.\n"
			"On/Off - включить/выключить режим охраны.\n"
			"/Status - состояние и список доступных внешних датчиков.\n"
			"/ChatID - id этого чата.\n"
			"Login/Logout - авторизация.\n"
			"/Uptime - время работы.\n"
			"Last X - последние X записей журнала. (число записей: 1-45)\n"
			"0-9 команда или 0-9 команда=значение - управление внешним датчиком\n"
			"0-9 help - запросить список команд у внешнего датчика по номеру\n"
			"pin X - \"закрепить\" номер внешнего датчика\n"
			"/unpin - убрать закрепление\n"
			"show some_text - отобразить текст на экране"
#ifdef SRX
			"\nplay X - запустить проигрываение трека с номером X, 0 - выключить\n"
			"volume X - установить громкость X"
#endif
			);
	}
	if (bot.is_command(msg.text, "show")) {
		// будет конкурировать со строкой отправленой напрямую через сайт
		messages[MESSAGE_WEB].count = 5;
		messages[MESSAGE_WEB].timer.setInterval(20000);
		messages[MESSAGE_WEB].color = 1;
		int pos = original.indexOf(" ");
		if (pos>0) {
			String to_show = original.substring(pos+1);
			messages[MESSAGE_WEB].text = to_show;
			return String(F("was shown: ")) + to_show;
		} else 
			return F("nothing to show");
	}
#ifdef SRX
	if (bot.is_command(msg.text, "play")) {
		int pos = msg.text.lastIndexOf(" ");
		if (pos>0) {
			int track = msg.text.substring(pos+1).toInt();
			mp3_stop(); // остановить то, что сейчас играет
			if (track > 0 && track <= mp3_all) {
				mp3_play(track);
				return String(F("play track: ")) + String(track);
			} else if (track == 0) {
				mp3_stop();
				return F("play stopped");
			} else
				return F("wrong track number");
		} else
			return F("track number is not specified");
	}
	if (bot.is_command(msg.text, "volume")) {
		int pos = msg.text.lastIndexOf(" ");
		if (pos>0) {
			int vol = constrain(msg.text.substring(pos+1).toInt(), 0, 15);
			mp3_volume(vol);
			return String(F("volume set to: ")) + String(vol);
		} else
			return F("volume not specified");
	}
#endif
	return F("Что? Не понятно...");
}
