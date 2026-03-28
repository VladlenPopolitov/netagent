#pragma once
#include <sys/types.h>

#define MAX_OUTPUT 4096

// Возвращает 0 при успехе, -1 при ошибке
// outbuf будет заполнен выводом команды (stdout+stderr)
int exec_capture_output(const char *path, char *const argv[], char *outbuf, size_t outlen);
int run_exec(const char *path, char *const argv[]);
