#ifndef telegram_h
#define telegram_h

void init_telegram();
void setup_telegram();
void tb_tick();
void tb_send_msg(const char* text);
void tb_send_msg(const String& text);
void tb_send_delayed();

#endif