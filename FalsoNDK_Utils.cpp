#include "FalsoNDK_Utils.h"

#include <sys/time.h>
#include <psp2/kernel/clib.h>

uint64_t AFN_timeMillis() {
    struct timeval te{};
    gettimeofday(&te, nullptr);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

__attribute__((weak))
void falsondk_log(int severity, const char * message) {
    static const char * const prefixes[] = { "D", "W", "E", "F" };
    char buf[2080];
    sceClibSnprintf(buf, sizeof(buf), "%s/FalsoNDK: %s\n", prefixes[severity], message);
    sceClibPrintf(buf);
}

void LOG_ALWAYS_FATAL_IF(bool cond, const char * fmt, ...) {
    if (!cond) return;
    char text[2048];
    va_list list;
    va_start(list, fmt);
    sceClibVsnprintf(text, sizeof(text), fmt, list);
    va_end(list);
    falsondk_log(FALSONDK_LOG_FATAL, text);
    sceClibAbort();
}

__attribute__((noreturn))
void LOG_ALWAYS_FATAL(const char * fmt, ...) {
    char text[2048];
    va_list list;
    va_start(list, fmt);
    sceClibVsnprintf(text, sizeof(text), fmt, list);
    va_end(list);
    falsondk_log(FALSONDK_LOG_FATAL, text);
    sceClibAbort();
}

void ALOGE(const char * fmt, ...) {
    char text[2048];
    va_list list;
    va_start(list, fmt);
    sceClibVsnprintf(text, sizeof(text), fmt, list);
    va_end(list);
    falsondk_log(FALSONDK_LOG_ERROR, text);
}

void ALOGW(const char * fmt, ...) {
    char text[2048];
    va_list list;
    va_start(list, fmt);
    sceClibVsnprintf(text, sizeof(text), fmt, list);
    va_end(list);
    falsondk_log(FALSONDK_LOG_WARN, text);
}

void ALOGD(const char * fmt, ...) {
    char text[2048];
    va_list list;
    va_start(list, fmt);
    sceClibVsnprintf(text, sizeof(text), fmt, list);
    va_end(list);
    falsondk_log(FALSONDK_LOG_DEBUG, text);
}
