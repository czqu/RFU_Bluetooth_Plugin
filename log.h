/**
 * Copyright (c) 2020 rxi
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `log.c` for details.
 */

#ifndef LOG_H
#define LOG_H
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <time.h>
#include <wchar.h>

#define LOG_VERSION "0.1.0"
#ifdef __MINGW32__
#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __FILEW__ WIDEN(__FILE__)
#endif
typedef struct {
    va_list ap;

    const wchar_t *fmt;
    const wchar_t *file;
    struct tm *time;
    void *udata;
    int line;
    int level;
} log_Event;

typedef void (*log_LogFn)(log_Event *ev);

typedef void (*log_LockFn)(bool lock, void *udata);

typedef enum {
    LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL
} LogLevel;

#define log_trace(...) log_log(LOG_TRACE, __FILEW__, __LINE__, __VA_ARGS__)
#define log_debug(...) log_log(LOG_DEBUG, __FILEW__, __LINE__, __VA_ARGS__)
#define log_info(...)  log_log(LOG_INFO,  __FILEW__, __LINE__, __VA_ARGS__)
#define log_warn(...)  log_log(LOG_WARN,  __FILEW__, __LINE__, __VA_ARGS__)
#define log_error(...) log_log(LOG_ERROR, __FILEW__, __LINE__, __VA_ARGS__)
#define log_fatal(...) log_log(LOG_FATAL, __FILEW__, __LINE__, __VA_ARGS__)

const wchar_t *log_level_string(int level);

void log_set_lock(log_LockFn fn, void *udata);

void log_set_level(int level);

void log_set_quiet(bool enable);

int log_add_callback(log_LogFn fn, void *udata, int level);

int log_add_fp(FILE *fp, int level);

void log_log(int level, const wchar_t *file, int line, const wchar_t *fmt, ...);

#endif
#ifdef __cplusplus
}
#endif