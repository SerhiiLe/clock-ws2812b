#ifndef WIFI_INIT_TRANSLATION_H
#define WIFI_INIT_TRANSLATION_H

const char PROGMEM T_wifiNotConnected_en[] = "WiFi not connected!";
const char PROGMEM T_wifiNotConnected_ru[] = "WiFi не подключился!";
const char PROGMEM T_wifiNotConnected_ua[] = "WiFi не підключився!";
const char* PROGMEM txt_wifiNotConnected[LANGUAGES] = {T_wifiNotConnected_en, T_wifiNotConnected_ru, T_wifiNotConnected_ua};

#ifdef AP_PASSWORD
const char PROGMEM T_wifiMessage_en[] = "To configure WiFi connect to \"%s\" password \"%s\", IP: 192.168.4.1";
const char PROGMEM T_wifiMessage_ru[] = "Для настройки WiFi подключитесь к \"%s\" с паролем \"%s\", IP: 192.168.4.1";
const char PROGMEM T_wifiMessage_ua[] = "Для настройки WiFi підключіться до \"%s\" з паролем \"%s\", IP: 192.168.4.1";
const char* PROGMEM txt_wifiMessage[LANGUAGES] = {T_wifiMessage_en, T_wifiMessage_ru, T_wifiMessage_ua};
#else // AP_PASSWORD
const char PROGMEM T_wifiMessage_en[] = "To configure WiFi connect to \"%s\", IP: 192.168.4.1";
const char PROGMEM T_wifiMessage_ru[] = "Для настройки WiFi подключитесь к \"%s\", IP: 192.168.4.1";
const char PROGMEM T_wifiMessage_ua[] = "Для настройки WiFi підключіться до \"%s\", IP: 192.168.4.1";
const char* PROGMEM txt_wifiMessage[LANGUAGES] = {T_wifiMessage_en, T_wifiMessage_ru, T_wifiMessage_ua};
#endif // AP_PASSWORD

const char PROGMEM T_wifiOff_en[] = "WiFi for setup is disabled.";
const char PROGMEM T_wifiOff_ru[] = "WiFi для настройки отключен.";
const char PROGMEM T_wifiOff_ua[] = "WiFi для налаштування вимкнено.";
const char* PROGMEM txt_wifiOff[LANGUAGES] = {T_wifiOff_en, T_wifiOff_ru, T_wifiOff_ua};

const char PROGMEM T_wifiFirst_en[] = "First you need to set up WiFi.";
const char PROGMEM T_wifiFirst_ru[] = "Сначала нужно настроить WiFi.";
const char PROGMEM T_wifiFirst_ua[] = "Спочатку потрібно налаштувати WiFi.";
const char* PROGMEM txt_wifiFirst[LANGUAGES] = {T_wifiFirst_en, T_wifiFirst_ru, T_wifiFirst_ua};

#endif // WIFI_INIT_TRANSLATION_H