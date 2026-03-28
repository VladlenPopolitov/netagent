#include "status.h"
#include "protocol.h"
#include "util.h"
// system log output 
#include <syslog.h>

void handle_status(int fd, struct request *req) {
    (void)req;
    ///send_str(fd, "OK status=running\n");
    send_ok(fd);
    send_data(fd, "status", "running");
    send_end(fd);
    if (!debug)
        syslog(LOG_INFO, "OK status=running.");
}
