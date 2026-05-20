#ifndef TELEGRAM_TRANSLATION_H
#define TELEGRAM_TRANSLATION_H

const char PROGMEM T_TB_welcome_en[] = "Welcome!";
const char PROGMEM T_TB_welcome_ru[] = "Добро пожаловать!";
const char PROGMEM T_TB_welcome_ua[] = "Ласкаво просимо!";
const char* PROGMEM txt_TB_welcome[LANGUAGES] = {T_TB_welcome_en, T_TB_welcome_ru, T_TB_welcome_ua};

const char PROGMEM T_TB_error_en[] = "Error!";
const char PROGMEM T_TB_error_ru[] = "Ошибка!";
const char PROGMEM T_TB_error_ua[] = "Помилка!";
const char* PROGMEM txt_TB_error[LANGUAGES] = {T_TB_error_en, T_TB_error_ru, T_TB_error_ua};

const char PROGMEM T_TB_send_enabled_en[] = "Sending messages is enabled";
const char PROGMEM T_TB_send_enabled_ru[] = "Отправка сообщений включена";
const char PROGMEM T_TB_send_enabled_ua[] = "Відправка повідомлень увімкнена";
const char* PROGMEM txt_TB_send_enabled[LANGUAGES] = {T_TB_send_enabled_en, T_TB_send_enabled_ru, T_TB_send_enabled_ua};

const char PROGMEM T_TB_send_disabled_en[] = "Sending messages is disabled";
const char PROGMEM T_TB_send_disabled_ru[] = "Отправка сообщений отключена";
const char PROGMEM T_TB_send_disabled_ua[] = "Відправка повідомлень вимкнена";
const char* PROGMEM txt_TB_send_disabled[LANGUAGES] = {T_TB_send_disabled_en, T_TB_send_disabled_ru, T_TB_send_disabled_ua};

const char PROGMEM T_TB_status_en[] = "Sensor: %s.\nPower: %s.\nLighting: %d -> %d.";
const char PROGMEM T_TB_status_ru[] = "Датчик: %s.\nПитание: %s.\nОсвещение: %d -> %d.";
const char PROGMEM T_TB_status_ua[] = "Датчик: %s.\nЖивлення: %s.\nОсвітлення: %d -> %d.";
const char* PROGMEM txt_TB_status[LANGUAGES] = {T_TB_status_en, T_TB_status_ru, T_TB_status_ua};

const char PROGMEM T_TB_help_if_en[] = "/help if needed";
const char PROGMEM T_TB_help_if_ru[] = "/help если надо";
const char PROGMEM T_TB_help_if_ua[] = "/help якщо треба";
const char* PROGMEM txt_TB_help_if[LANGUAGES] = {T_TB_help_if_en, T_TB_help_if_ru, T_TB_help_if_ua};

const char PROGMEM T_TB_about_en[] = "A bot for controlling the motion sensor in the clock. Nothing interesting, you can pass by.";
const char PROGMEM T_TB_about_ru[] = "Бот для управления датчиком движения в часах. Ничего интересного, можете пройти мимо.";
const char PROGMEM T_TB_about_ua[] = "Бот для керування датчиком руху в годинах. Нічого цікавого, можете пройти мимо.";
const char* PROGMEM txt_TB_about[LANGUAGES] = {T_TB_about_en, T_TB_about_ru, T_TB_about_ua};

const char PROGMEM T_TB_help_en[] =
	"Available commands:\n"
	"/Start (menu) - show the menu.\n"
	"/Stop - hide the menu.\n"
	"On/Off - enable/disable security mode.\n"
	"/Status - status and list of available external sensors.\n"
	"/ChatID - this chat's ID.\n"
	"Login/Logout - authorization.\n"
	"/Uptime - uptime.\n"
	"Last X - the last X log entries. (number of entries: 1-45)\n"
	"0-9 command or 0-9 command=value - control an external sensor.\n"
	"0-9 help - request a list of commands from an external sensor by number.\n"
	"pin X - pin the external sensor number.\n"
	"/unpin - unpin.\n"
	"show some_text - display text on the screen"
#ifdef SRX
	"play X - start playing track number X, 0 - turn off\n"
	"volume X - set volume to X"
#endif
;
const char PROGMEM T_TB_help_ru[] = 
	"Доступные команды:\n"
	"/Start (menu) - показать меню.\n"
	"/Stop - спрятать меню.\n"
	"On/Off - включить/выключить режим охраны.\n"
	"/Status - состояние и список доступных внешних датчиков.\n"
	"/ChatID - id этого чата.\n"
	"Login/Logout - авторизация.\n"
	"/Uptime - время работы.\n"
	"Last X - последние X записей журнала. (число записей: 1-45)\n"
	"0-9 команда или 0-9 команда=значение - управление внешним датчиком.\n"
	"0-9 help - запросить список команд у внешнего датчика по номеру.\n"
	"pin X - \"закрепить\" номер внешнего датчика.\n"
	"/unpin - убрать закрепление.\n"
	"show some_text - отобразить текст на экране."
#ifdef SRX
	"\nplay X - запустить проигрываение трека с номером X, 0 - выключить\n"
	"volume X - установить громкость X"
#endif
;
const char PROGMEM T_TB_help_ua[] = 
	"Доступні команди:\n" 
	"/Start (menu) - показати меню.\n" 
	"/Stop - заховати меню.\n" 
	"On/Off - увімкнути/вимкнути режим охорони.\n" 
	"/Status - стан та список доступних зовнішніх датчиків.\n" 
	"/ChatID - id цього чату.\n" 
	"Login/Logout - авторизація.\n" 
	"/Uptime - час роботи.\n" 
	"Last X - останні X записи журналу. (число записів: 1-45)\n" 
	"0-9 команда або 0-9 команда = значення - управління зовнішнім датчиком.\n" 
	"0-9 help - запросити список команд у зовнішнього датчика за номером.\n" 
	"pin X - \"закріпити\" номер зовнішнього датчика.\n" 
	"/unpin - прибрати закріплення.\n" 
	"show some_text - відобразити текст на екрані."
#ifdef SRX 
	"\nplay X - запустити програвання треку з номером X, 0 - вимкнути\n" 
	"volume X - встановити гучність X"
#endif
;
const char* PROGMEM txt_TB_help[LANGUAGES] = {T_TB_help_en, T_TB_help_ru, T_TB_help_ua};

const char PROGMEM T_TB_nothing_to_show_en[] = "Nothing to show.";
const char PROGMEM T_TB_nothing_to_show_ru[] = "Нечего показывать.";
const char PROGMEM T_TB_nothing_to_show_ua[] = "Нічого показувати.";
const char* PROGMEM txt_TB_nothing_to_show[LANGUAGES] = {T_TB_nothing_to_show_en, T_TB_nothing_to_show_ru, T_TB_nothing_to_show_ua};

const char PROGMEM T_TB_shown_en[] = "Was shown: ";
const char PROGMEM T_TB_shown_ru[] = "Показано: ";
const char PROGMEM T_TB_shown_ua[] = "Показано: ";
const char* PROGMEM txt_TB_shown[LANGUAGES] = {T_TB_shown_en, T_TB_shown_ru, T_TB_shown_ua};

const char PROGMEM T_TB_played_en[] = "Play track: ";
const char PROGMEM T_TB_played_ru[] = "Проигрывается трек: ";
const char PROGMEM T_TB_played_ua[] = "Програється трек: ";
const char* PROGMEM txt_TB_played[LANGUAGES] = {T_TB_played_en, T_TB_played_ru, T_TB_played_ua};

const char PROGMEM T_TB_play_stopped_en[] = "Play stopped.";
const char PROGMEM T_TB_play_stopped_ru[] = "Проигрывание остановлено.";
const char PROGMEM T_TB_play_stopped_ua[] = "Програвання зупинено.";
const char* PROGMEM txt_TB_play_stopped[LANGUAGES] = {T_TB_play_stopped_en, T_TB_play_stopped_ru, T_TB_play_stopped_ua};

const char PROGMEM T_TB_wrong_track_en[] = "Wrong track number.";
const char PROGMEM T_TB_wrong_track_ru[] = "Неверный номер трека.";
const char PROGMEM T_TB_wrong_track_ua[] = "Невірний номер трека.";
const char* PROGMEM txt_TB_wrong_track[LANGUAGES] = {T_TB_wrong_track_en, T_TB_wrong_track_ru, T_TB_wrong_track_ua};

const char PROGMEM T_TB_track_not_specified_en[] = "Track number is not specified.";
const char PROGMEM T_TB_track_not_specified_ru[] = "Номер трека не указан.";
const char PROGMEM T_TB_track_not_specified_ua[] = "Номер трека не вказано.";
const char* PROGMEM txt_TB_track_not_specified[LANGUAGES] = {T_TB_track_not_specified_en, T_TB_track_not_specified_ru, T_TB_track_not_specified_ua};

const char PROGMEM T_TB_volume_set_en[] = "Volume set to: ";
const char PROGMEM T_TB_volume_set_ru[] = "Громкость установлена на: ";
const char PROGMEM T_TB_volume_set_ua[] = "Гучність встановлена на: ";
const char* PROGMEM txt_TB_volume_set_to[LANGUAGES] = {T_TB_volume_set_en, T_TB_volume_set_ru, T_TB_volume_set_ua};

const char PROGMEM T_TB_volume_not_specified_en[] = "Volume not specified.";
const char PROGMEM T_TB_volume_not_specified_ru[] = "Громкость не указана.";
const char PROGMEM T_TB_volume_not_specified_ua[] = "Гучність не вказана.";
const char* PROGMEM txt_TB_volume_not_specified[LANGUAGES] = {T_TB_volume_not_specified_en, T_TB_volume_not_specified_ru, T_TB_volume_not_specified_ua};

const char PROGMEM T_TB_dont_understand_en[] = "I don't understand.";
const char PROGMEM T_TB_dont_understand_ru[] = "Я не понимаю.";
const char PROGMEM T_TB_dont_understand_ua[] = "Я не розумію.";
const char* PROGMEM txt_TB_dont_understand[LANGUAGES] = {T_TB_dont_understand_en, T_TB_dont_understand_ru, T_TB_dont_understand_ua};

#endif