/*
	Подключение к WiFi
	Если настроек нет или нет подключения к точке доступа,
	поднять свою точку доступа с captive portal для переадресации
	прямо из настроек wifi телефона. После выбора сети и пароля будет
	подключаться автоматически.
*/

#include <Arduino.h>
#include <WiFiSetup.h>
#include "defines.h"
#include "runningText.h"
#include "web.h"
#include "wifi_init.h"
#include "wifi_init_translation.h"

WiFiSetup wm;

bool wifi_isConnected = false;
bool wifi_isPortal = false;
String wifi_message = "";

void wifi_setup() {
	WiFi.setHostname(gs.clock_name.c_str());
	WiFi.mode(WIFI_STA);
	wm.setWiFiAutoReconnect(true);
	wm.setEnableConfigPortal(false);

	//automatically connect using saved credentials if they exist
	//If connection fails it starts an access point with the specified name
	if(wm.autoConnect()) {
		LOG(println, F("WiFi is connected :)"));
	} else {
		LOG(println, F("WiFi is not connected"));
		if(wm.getWiFiIsSaved()) initRString(txt_wifiNotConnected[gs.language],CRGB::White);
		else wifi_startConfig(true);
	}
}

void wifi_process() {
	if(WiFi.status() == WL_CONNECTED) {
		if( ! wifi_isConnected ) {
			wifi_isConnected = true;
			LOG(println, F("WiFi now is connected"));
		}
	} else {
		if( wifi_isConnected ) {
			wifi_isConnected = false;
			LOG(println, F("No WiFi now"));
		}
		wm.process();
	}
}

String wifi_currentIP() {
	return WiFi.localIP().toString();
}

int8_t wifi_rssi() {
	return WiFi.RSSI();
}

void wifi_resetSettings() {
	wm.resetSettings();
}

// Включение или отключение ConfigPortal для настройки WiFi
void wifi_startConfig(bool fl) {
	if(fl) {
		if(wifi_isConnected) {
			web_disable();
			WiFi.disconnect();
		}
		wm.setConfigPortalBlocking(false);
		#ifdef AP_PASSWORD
		wm.startConfigPortal(_SSID, _PASSWORD);
		#else
		wm.startConfigPortal(_SSID);
		#endif
		LOG(println, F("ConfigPortal is started"));
		char wifi_message_buf[150];
		#ifdef AP_PASSWORD
		snprintf_P(wifi_message_buf, sizeof(wifi_message_buf), txt_wifiMessage[gs.language], _SSID, _PASSWORD);
		#else
		snprintf_P(wifi_message_buf, sizeof(wifi_message_buf), txt_wifiMessage[gs.language], _SSID);
		#endif
		initRString(wifi_message_buf,CRGB::White);
		wifi_isPortal = true;
	} else {
		if(wm.getWiFiIsSaved()) {
			WiFi.begin(wm.getWiFiSSID(),wm.getWiFiPass());
			wifi_message = "";
			initRString(txt_wifiOff[gs.language],CRGB::White);
			wifi_isPortal = false;
			if(wm.getConfigPortalActive()) wm.stopConfigPortal();
			LOG(println, F("ConfigPortal is stopped"));
			return;
		}
		initRString(txt_wifiFirst[gs.language],CRGB::White);
	}
}