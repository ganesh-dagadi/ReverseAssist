#include "logger.h"
#include "esp_log.h"
#include <stdio.h>

#include <stdarg.h>

void log_info(const char* tag, const char* fmt, ...) {
    char buf[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    ESP_LOGI(tag, "%s", buf);
}

void log_debug(const char* tag, const char* fmt, ...) {
    char buf[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ESP_LOGD(tag, "%s", buf);
}

void log_error(const char* tag, const char* fmt, ...) {
    char buf[256];

    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    ESP_LOGE(tag, "%s", buf);
}