/*
	"охрана"
	информирование о срабатывании датчика движения
	работа с telegram
*/

#include <Arduino.h>
#include "defines.h"
#include "telegram.h"
#include "telegram_translation.h"
#include "settings.h"
#include "ntp.h"
#include "barometer.h"
#include "dfplayer.h"
#include <StringConverters.h>
#include <AsyncTelegramAPI.h>
#include "asyncHTTP_global.h"

AsyncTelegramAPI bot;

String telegramCallback(TResult& msg);
bool fl_secretWanted = false;
int8_t pinned = -1;

// Установка токена и списка подписанных чатов
void setup_telegram() {
	// bot.setChatID(bot.extractFirstNumber(ts.tb_chats)); // не нужно, так как передаётся при каждой отправке
	bot.setBotToken(ts.tb_token);
	bot.setInteractiveWindow(ts.tb_accelerate, ts.tb_accelerated, ts.tb_rate);
	bot.triggerInteractive();
}

// Инициализация бота
void init_telegram() {
	setup_telegram();
	bot.attachCallback(telegramCallback);
	bot.start();
}

// Опрос телеграмм в ожидании команд для обработки
void tb_tick() {
	// если ошибка сети, то временно приостановить опросы
	if (bot.checkMessage() < 0) bot.setNext(ts.tb_ban);
}

// Отправка сообщений (постановка в очередь)
void tb_send_msg(const String& text) {
	// [[maybe_unused]] bool r = bot.sendMessage(bot.extractFirstNumber(ts.tb_chats), text); // отсылка только на первый номер
	[[maybe_unused]] bool r = bot.sendMessageToAll(ts.tb_chats, text); // отсылка на все номера
	LOG(printf_P, PSTR("Send to telegram: %d\n"), r);
}

// Обработка входящего сообщения телеграмм
String telegramCallback(TResult& msg) {
	char buf[100];

	// выводим ID чата, имя юзера и текст сообщения
	LOG(printf_P, PSTR("From telegram:%lld;%s;%lld;%s.\n"), msg.chatId, msg.from.c_str(), msg.messageId, msg.text.c_str());

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
			return txt_TB_welcome[gs.language];// F("Добро пожаловать!");
		} else {
			bot.deleteMessages(msg.chatId, {msg.messageId-1, msg.messageId});
			return txt_TB_error[gs.language];// F("Ошибка!");
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

		// открепить датчик
		if (bot.is_command(msg.text, F("unpin")) || msg.text == "/") {
			pinned = -1;
			msg.text = F("menu");
		} else

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
			return txt_TB_send_enabled[gs.language];
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
			sprintf_P(buf, txt_TB_status[gs.language],
				sec_enable?F("On"):F("Off"),
				fl_5v?F("+5V"):F("battery"),
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
							url += StringConverters::urlEncode(msg.text.substring(pos2+1), true);
					} else {
						if (fl_free_cmd || msg.text[pos] == '/')
							url += F("cmd=");
						url += StringConverters::urlEncode(msg.text.substring(pos));
						if (msg.text[pos] != '/')
							url += "=";
					}
				}
				LOG(println, url);
				int64_t c_id = msg.chatId;
				httpClient.get(url, [c_id, n](AsyncHttpResponse& res) {
					// ответ от датчика запихивается сразу в telegram, обработку делает ArduinoJson
					String answer = res.isSuccess() ?
						sensor[n].hostname + "\n" + res.body:
						sensor[n].hostname + " error: " + res.errorString();
					bot.sendMessage(c_id, answer);
				});
				return "";
			} else {
				return F("датчик неактивен");
			}
		}
		if (bot.is_command(msg.text, "show") || bot.is_command(msg.text, "send")) {
			// будет конкурировать со строкой отправленной напрямую через сайт
			messages[MESSAGE_WEB].count = ts.rcount;
			messages[MESSAGE_WEB].timer.setInterval(1000UL * ts.rint);
			messages[MESSAGE_WEB].timer.setNext(10); // подвинуть начало показа, чтобы сразу
			messages[MESSAGE_WEB].color = ts.color_mode > 0 ? ts.color_mode: ts.color;
			if (ts.melody) {
				mp3_disableLoopAll();
				mp3_disableLoop();
				mp3_volume(ts.volume);
				mp3_play(ts.melody);
			}
			int pos = original.indexOf(" ");
			if (pos>0) {
				String to_show = original.substring(pos+1);
				messages[MESSAGE_WEB].text = to_show;
				return String(txt_TB_shown[gs.language]) + to_show;
			} else 
				return txt_TB_nothing_to_show[gs.language];
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
					return txt_TB_play_stopped[gs.language];
				} else
					return txt_TB_wrong_track[gs.language];
			} else
				return txt_TB_track_not_specified[gs.language];
		}
		if (bot.is_command(msg.text, "volume")) {
			int pos = msg.text.lastIndexOf(" ");
			if (pos>0) {
				int vol = constrain(msg.text.substring(pos+1).toInt(), 0, 15);
				mp3_volume(vol);
				return String(txt_TB_volume_set_to[gs.language]) + String(vol);
			} else
				return txt_TB_volume_not_specified[gs.language];
		}
#endif
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
	} else { // конец команд для авторизированных, дальше команды доступные всем
 		if (bot.is_command(msg.text, F("login"))) {
			fl_secretWanted = true;
			return F("Пароль доступа?");
		}
	}
	if (fl_start || bot.is_command(msg.text, F("start")) || bot.is_command(msg.text, F("menu"))) {
		// показать юзер меню (\t - горизонтальное разделение кнопок, \n - вертикальное
  		// (текст сообщения, кнопки)
		if(fl_auth)	return String(txt_TB_help_if[gs.language]) + F("[MENU]On\tOff\tStatus\tChatID\nLogout\tUptime\tLast 20\tHelp");
		else return String(txt_TB_help_if[gs.language]) + F("[MENU]Login\tAbout\tChatID");
	}
	if (bot.is_command(msg.text, F("stop"))) {
		// убрать меню
		return F("start - open menu[MENU]");
	}
	if (bot.is_command(msg.text, F("chatid"))) {
		return String(msg.chatId);
	}
	if (bot.is_command(msg.text, F("about"))) {
		return txt_TB_about[gs.language];
	}
	if (bot.is_command(msg.text, F("help"))) {
		return txt_TB_help[gs.language];
	}
	return txt_TB_dont_understand[gs.language];
}
