#pragma once

#define MAX_ARGS 32

struct arg {
    char *key;
    char *val;
};

struct request {
    int version;
    char *cmd;
    struct arg args[MAX_ARGS];
    int argc;
};
