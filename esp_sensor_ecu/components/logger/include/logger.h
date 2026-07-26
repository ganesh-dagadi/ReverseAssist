#ifndef LOGGER
#define LOGGER


void log_info(const char* tag, const char* fmt, ...);
void log_debug(const char* tag, const char* fmt, ...);
void log_error(const char* tag, const char* fmt, ...);


#endif