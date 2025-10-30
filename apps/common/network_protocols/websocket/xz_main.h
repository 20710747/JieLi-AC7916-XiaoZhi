
#ifndef XZ_MAIN_H
#define XZ_MAIN_H

// 全局session_id变量
extern char g_session_id[64];
extern bool g_session_id_received;

void send_tag_to_websocket(const char* tag_text);

#endif
