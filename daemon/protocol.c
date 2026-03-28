#include "protocol.h"
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

static int safe_write(int fd, const char *buf, size_t len) {
    ssize_t n = write(fd, buf, len);

    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return -2; // timeout
        }
        return -1; // other error
    }

    return 0;
}

int send_str(int fd, const char *s) {
    return safe_write(fd, s, strlen(s));
}

int send_error(int fd, const char *code, const char *msg) {
    char buf[256];
    snprintf(buf, sizeof(buf), "ERROR code=%s msg=%s\n", code, msg);
    return send_str(fd, buf);
}

int send_ok(int fd) {
    return send_str(fd, "OK\n");
}

void parse_request(char *line, struct request *req) {
    req->version = 1; // default
    req->cmd = NULL;
    req->argc = 0;

    char *token;
    while ((token = strsep(&line, " \n")) != NULL) {
        if (*token == '\0') continue;

        char *eq = strchr(token, '=');
        if (!eq) continue;

        *eq = '\0';
        char *key = token;
        char *val = eq + 1;

        if (strcmp(key, "v") == 0) {
            req->version = atoi(val);
        } else if (strcmp(key, "cmd") == 0) {
            req->cmd = val;
        } else if (req->argc < MAX_ARGS) {
            req->args[req->argc].key = key;
            req->args[req->argc].val = val;
            req->argc++;
        }
    }
}

