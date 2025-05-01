// экспериментальная реализация проигрывателя, срисовано с примеров  Makuna/DFMiniMp3

/*
	Работа с платой DFPlayer mini.
	Получить стабильную работу согласно документации не получилось.
	Не работают:
		запросы на число файлов в каталоге,
		прямое изменение громкости
		прямое указание номера файла
	По этому все файлы выбираются прямым сквозным порядковым номером,
	функцией "следующий"-"предыдущий" файл, перебором.
	Громкость выбирается так-же, "вверх"-"вниз"
*/

#include <Arduino.h>
#include "defines.h"
#include "dfplayer.h"

int mp3_all = 0;
int mp3_current = 1;
int8_t cur_Volume = 15;
bool mp3_isInit = false;
bool mp3_isReady = false;

#ifdef SRX
// плата установлена, подготовка к инициализации драйвера

// #define DfMiniMp3Debug Serial // Для вывода обмена данных с dfPlayer в консоль
#include <DFMiniMp3E.h>

// callback class для асинхронных вызовов из драйвера
class Mp3Notify {
public:
    // required type
    typedef void TargetType;

    // required method even though it doesn't do anything
    static void SetTarget(TargetType*) { }

    static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action) {
        if (source & DfMp3_PlaySources_Sd) {
            LOG(print, PSTR("SD Card, "));
        }
        if (source & DfMp3_PlaySources_Usb) {
            LOG(print, PSTR("USB Disk, "));
        }
        if (source & DfMp3_PlaySources_Flash) {
            LOG(print, PSTR("Flash, "));
        }
        LOG(println, action);
    }

    // required method
    static void OnError(uint16_t errorCode) {
        // see DfMp3_Error for code meaning
		LOG(printf_P, PSTR("DFPlayerError: %u "), errorCode);
		switch (errorCode) {
			case DfMp3_Error_Busy:				//  busy
				LOG(println, PSTR("Card not found"));
				break;
			case DfMp3_Error_Sleeping:			//  frame not received    sleep
				LOG(println, PSTR("Sleeping"));
				break;
			case DfMp3_Error_SerialWrongStack:	//  verification error    frame not received
				LOG(println, PSTR("Get wrong stack"));
				break;
			case DfMp3_Error_CheckSumNotMatch:	//                        checksum
				LOG(println, PSTR("Check sum not match"));
				break;
			case DfMp3_Error_FileIndexOut:		//  folder out of scope   track out of scope
				LOG(println, PSTR("File index out of bound"));
				break;
			case DfMp3_Error_FileMismatch:		//  folder not found      track not found
				LOG(println, PSTR("Cannot find file"));
				break;
			case DfMp3_Error_Advertise:			//                        only allowed while playing     advertisement not allowed
				LOG(println, PSTR("Advertisement not allowed"));
				break;
			case DfMp3_Error_SdReadFail:		//                        SD card failed
				LOG(println, PSTR("SD card failed"));
				break;
			case DfMp3_Error_FlashReadFail:		//                        Flash mem failed
				LOG(println, PSTR("Flash mem failed"));
				break;
			case DfMp3_Error_EnteredSleep:		//                        entered sleep
				LOG(println, PSTR("Entered sleep"));
				break;
			case DfMp3_Error_RxTimeout:
				mp3_isInit = false;
				LOG(println, PSTR("Rx timeout"));
				break;
			case DfMp3_Error_PacketSize:
				LOG(println, PSTR("Packet size mismatch"));
				break;
			case DfMp3_Error_PacketHeader:
				LOG(println, PSTR("Packet header broken"));
				break;
			case DfMp3_Error_PacketChecksum:
				LOG(println, PSTR("Packed checksum error"));
				break;
			case DfMp3_Error_General:
				LOG(println, PSTR("General error"));
				break;
			default:
				LOG(println, PSTR("Unknown error"));
				break;
		}
	}

    // required method
    static void OnPlayFinished([[maybe_unused]] DfMp3_PlaySources source, uint16_t track) {
		LOG(printf_P, PSTR("Number: %i. Play Finished!\n"), track);
		// dfPlayer.stop();
	}

    // required method
    static void OnPlaySourceOnline(DfMp3_PlaySources source) {
        PrintlnSourceAction(source, "online");
		mp3_isInit = true;
		mp3_reread();
    }

    // required method
    static void OnPlaySourceInserted(DfMp3_PlaySources source) {
        PrintlnSourceAction(source, "inserted");
		delay(10);
		mp3_reread();
    }

    // required method
    static void OnPlaySourceRemoved(DfMp3_PlaySources source) {
        PrintlnSourceAction(source, "removed");
		mp3_all = 0;
    }
};

// Подготовка объекта драйвера dfPlayer (dfmp3)

#if ESP32 == 1
		#define mp3Serial Serial1
#else // ESP8266
	#include <SoftwareSerial.h>
	EspSoftwareSerial::UART mp3Serial;
	// SoftwareSerial mp3Serial(SRX, STX); // если не хочется устанавливать EspSoftSerial. Работает менее стабильно.
#endif

typedef DFMiniMp3<Mp3Notify, Mp3ChipType, Mp3ChipTimeout>DfMp3;
DfMp3 dfmp3;

// инициализация происходит в момент первого обращения.
void checkInit() {
	if( ! mp3_isInit || timeoutMp3Timer.isReady() ) mp3_init();
}

// проверка играет ли dfPlayer
bool mp3_isPlay() {
	checkInit();
	DfMp3_Status status = dfmp3.getStatus();
	return status.state == DfMp3_StatusState_Playing;
	// return dfPlayer.readState() & 1;
}

// обновление номера трека, который сейчас играет
void mp3_update() {
	if(mp3_isPlay() || mp3_current>mp3_all ) {
		int mp3_new = mp3_current;
		for(uint8_t cnt = 0; cnt < 10; cnt++) {
			mp3_new = dfmp3.getCurrentTrack(DfMp3_PlaySource_Sd);
			if( mp3_new > mp3_all ) {
				delay(20);
				continue;
			}
			mp3_current = mp3_new;
			break;
		}
	}
}

void dfSerialInit() {
	// в ESP32 функции не привязаны к ножкам, инициализация указывает какие ножки задействовать и отключает их от другого функционала
	#if ESP32 == 1
		mp3Serial.begin(9600, SERIAL_8N1, SRX, STX);
	#else
		mp3Serial.begin(9600, SWSERIAL_8N1, SRX, STX, false); // EspSoftwareSerial
		// mp3Serial.begin(9600); // SoftwareSerial
	#endif
}

// DFPlayer медленный и любит при каждом чихе отваливаться, по этому много проверок и задержек. Некрасиво, но работает достаточно устойчиво.
void mp3_init() {
	LOG(println, PSTR("init mp3 player"));
	if( mp3_isInit ) {
		mp3Serial.flush();
		dfmp3.reset();
	} else {
		mp3Serial.flush();
		mp3_isReady = dfmp3.begin(mp3Serial);
		LOG(printf_P, PSTR("mp3_isReady: %i\n"), mp3_isReady);
		if(mp3_isReady) {
			mp3_isInit = true;
			LOG(printf_P, PSTR("Software Version %u\n"), dfmp3.getSoftwareVersion());
			delay(10);
			dfmp3.setPlaybackSource(DfMp3_PlaySource_Sd);
			delay(10);
			mp3_reread();
		}
	}
	if(mp3_isReady) {
		delay(10);
		dfmp3.setEq(DfMp3_Eq_Normal);
		delay(10);
		mp3_volume(1,false);
	}
	if(dfmp3.getCurrentTrack(DfMp3_PlaySource_Sd)>mp3_all) {
		// dfmp3.start();
		delay(10);
		mp3_update();
		// dfmp3.stop();
		// delay(10);
	}
}

void mp3_volume(uint8_t t, boolean p) {
	checkInit();
	dfmp3.setVolume(t);
	if (p) cur_Volume = t;
	delay(10);

	// этот блок не должен исполняться, если всё нормально. Но жалко выкидывать.
	if( dfmp3.getVolume() != t ) {
		int cur = 0, old = 0, cnt = 0;
		delay(10);
		while(true) {
			cur = dfmp3.getVolume();
			if( cur==t ) {
				if (p) cur_Volume = t;
				break;
			}
			if( cur==old || cur<0 || cur>30 ) {
				if( cnt++ > 20 ) {
					mp3_init();
					delay(80);
					cnt = 0;
					old = 0;
				}
				delay(20);
				continue;
			} else cnt = 0;
			old=cur;
			if( cur<t ) {
				dfmp3.increaseVolume();
				delay(10);
			}
			if( cur>t ) {
				dfmp3.decreaseVolume();
				delay(10);
			}
		}
	}

	timeoutMp3Timer.reset();
}

void mp3_play(int t) {
	checkInit();
	if( mp3_all == 0 ) return;
	if( t < 1 || t > mp3_all ) return;
	LOG(printf_P, PSTR("want track: %i\n"),t);
	dfmp3.playGlobalTrack(t);
	delay(10);

	// дальше два блока, которые не должны исполняться, если всё правильно настроено. Оставлено потому, что жалко выкидывать.
	if( ! mp3_isPlay() ) {
		dfmp3.start();
		delay(100);
	} else delay(10);

	// чувствую себя Трампом:"Этого не должно было случится!".
	if(dfmp3.getCurrentTrack(DfMp3_PlaySource_Sd) != t) {
		int cur = 0, old = 0, cnt = 0;
		delay(10);
		while(true) {
			cur = dfmp3.getCurrentTrack(DfMp3_PlaySource_Sd);
			LOG(printf_P, PSTR("track: %i\n"),cur);
			if( cur==t ) break;
			if( cur==old || cur<=0 || cur > mp3_all ) {
				if( cnt++ > 20 ) {
					mp3_init();
					dfmp3.start();
					delay(80);
					cnt = 0;
					old = 0;
				}
				delay(20);
				continue;
			} else cnt = 0;
			old=cur;
			if( cur<t ) {
				if( t-cur < (mp3_all >> 1) )
					dfmp3.nextTrack();
				else
					dfmp3.prevTrack();
				delay(10);
			}
			if (cur>t) {
				if( cur-t < (mp3_all >> 1) )
					dfmp3.prevTrack();
				else
					dfmp3.nextTrack();
				delay(10);
			}
		}
	}

	mp3_volume(cur_Volume);
	mp3_current = t;
	timeoutMp3Timer.reset();
}

void mp3_reread() {
	checkInit();
	mp3_all = dfmp3.getTotalTrackCount(DfMp3_PlaySource_Sd);
	// mp3_isReady = mp3_all == 0 ? false: true;
}

void mp3_start() {
	checkInit();
	dfmp3.start();
}

void mp3_pause() {
	checkInit();
	dfmp3.pause();
}

void mp3_stop() {
	checkInit();
	dfmp3.stop();
}

void mp3_enableLoop() {
	checkInit();
	dfmp3.setRepeatPlayCurrentTrack(true);
}

void mp3_disableLoop() {
	checkInit();
	dfmp3.setRepeatPlayCurrentTrack(false);
}

void mp3_enableLoopAll() {
	checkInit();
	dfmp3.setRepeatPlayAllInRoot(true);
}

void mp3_disableLoopAll() {
	checkInit();
	dfmp3.setRepeatPlayAllInRoot(false);
}

void mp3_randomAll() {
	checkInit();
	dfmp3.playRandomTrackFromAll();
}

// проверка сообщений от dfPlayer.
void mp3_check() {
	dfmp3.loop();
}

#else
// заглушки, если плата DFPlayer не установлена
bool mp3_isPlay() {return true;}
void mp3_volume(uint8_t t, boolean p) {}
void dfSerialInit() {}
void mp3_init() {mp3_isInit = true; mp3_isReady = true;}
void mp3_check() {}
void mp3_play(int t) {}
void mp3_reread() {}
void mp3_update() {}
void mp3_start() {}
void mp3_pause() {}
void mp3_stop() {}
void mp3_enableLoop() {}
void mp3_disableLoop() {}
void mp3_enableLoopAll() {}
void mp3_disableLoopAll() {}
void mp3_randomAll() {}
void mp3_next() {}
void mp3_previous() {}

#endif