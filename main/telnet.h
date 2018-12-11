#include <stdarg.h>

#ifdef __cplusplus__
extern "C" {
#endif

void telnet_esp32_listenForClients(void (*callbackParam)(uint8_t *buffer, size_t size));
void telnet_esp32_sendData(uint8_t *buffer, size_t size);
int telnet_esp32_vprintf(const char *fmt, va_list va);

#ifdef __cplusplus__
}
#endif
