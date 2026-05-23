#ifndef dfplayer_h
#define dfplayer_h

// возвращает, играет ли сейчас плейер
bool mp3_isPlay();
// устанавливает громкость (t= от 1 до 30). p - запоминать ли уровень (p=false - не запоминать)
void mp3_volume(uint8_t t, boolean p=true);
// инициализация Serial для dfPlayer.
void dfSerialInit();
// инициализация плейера
void mp3_init();
// Проверка данных от плейера
void mp3_check();
// играть трек с номером t
void mp3_play(int t);
// перечитать список файлов
void mp3_reread();
// обновить номер текущего трека
void mp3_update();
// запустить
void mp3_start();
void mp3_pause();
void mp3_stop();
void mp3_enableLoop();
void mp3_disableLoop();
void mp3_enableLoopAll();
void mp3_disableLoopAll();
void mp3_randomAll();
// запуск воспроизведения конкретного трека по номеру папки и номеру трека
void mp3_playInFolder(uint8_t folder, uint8_t track);
// запуск воспроизведения конкретного трека по номеру из папки mp3
void mp3_playInMp3(uint16_t track);
uint16_t mp3_folderTrackCount(uint8_t folder);

#endif