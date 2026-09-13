#ifndef LOGGER
#define LOGGER
#include <stdio.h>

void log_info(const char* tag, const char* fmt, ...);
void log_debug(const char* tag, const char* fmt, ...);
void log_error(const char* tag, const char* fmt, ...);
void log_dump(const char* tag, uint8_t* buf, size_t len);


#endif