#include "status.h"
#include "../protocol.h"

void handle_status(int fd, struct request *req) {
    (void)req;
    send_str(fd, "OK status=running\n");
}
