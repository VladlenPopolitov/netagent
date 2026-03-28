#include "handlers.h"
#include "request.h"
#include "protocol.h"

#ifdef _DETEME_IT
void dispatch_old(int fd, struct request *req) {
    if (req->version != 1) {
        send_error(fd, "ENOTSUP", "unsupported_version");
        return;
    }

    if (!req->cmd) {
        send_error(fd, "EINVAL", "missing_cmd");
        return;
    }

    if (strcmp(req->cmd, "STATUS") == 0) {
        handle_status(fd, req);
    } else if (strcmp(req->cmd, "LIST_INTERFACES") == 0) {
        handle_list_interfaces(fd, req);
    } else if (strcmp(req->cmd, "CONNECT") == 0) {
        handle_connect(fd, req);
    } else {
        send_error(fd, "ENOTSUP", "unknown_command");
    }
}

#endif

#include <string.h>

#include "handlers.h"
#include "protocol.h"

// forward declarations (или include отдельных файлов)
void handle_status(int fd, struct request *req);
void handle_list_interfaces(int fd, struct request *req);
void handle_connect(int fd, struct request *req);

struct handler_entry {
    const char *cmd;
    void (*fn)(int, struct request *);
};

static struct handler_entry table[] = {
    { "STATUS", handle_status },
    { "LIST_INTERFACES", handle_list_interfaces },
    { "CONNECT", handle_connect },
    { NULL, NULL }
};

void dispatch(int fd, struct request *req) {
    if (req->version != 1) {
        send_error(fd, "ENOTSUP", "unsupported_version");
        return;
    }

    if (!req->cmd) {
        send_error(fd, "EINVAL", "missing_cmd");
        return;
    }

    for (int i = 0; table[i].cmd; i++) {
        if (strcmp(req->cmd, table[i].cmd) == 0) {
            table[i].fn(fd, req);
            return;
        }
    }

    send_error(fd, "ENOTSUP", "unknown_command");
}

