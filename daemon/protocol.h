#pragma once

#include <stddef.h>
#include "request.h"

// parsing
void parse_request(char *line, struct request *req);

// sending
int send_str(int fd, const char *s);
int send_ok(int fd);
int send_error(int fd, const char *code, const char *msg);
int send_data(int fd, const char *key, const char *val);
int send_end(int fd);
