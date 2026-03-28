#include <string.h>
#include <stdio.h>

#include "list.h"
#include "protocol.h"
#include "exec_safe.h"

void handle_list_interfaces(int fd, struct request *req) {
    (void)req;

    char buf[MAX_OUTPUT];
    char *args[] = { "ifconfig", "-l", NULL };

    if (exec_capture_output("/sbin/ifconfig", args, buf, sizeof(buf)) != 0) {
        send_error(fd, "EIO", "ifconfig_failed");
        return;
    }

    send_ok(fd);

    char *tok = strtok(buf, " \n");
    while (tok) {
        ///dprintf(fd, "DATA iface=%s\n", tok);
        send_data(fd, "iface", tok);
        tok = strtok(NULL, " \n");
    }

    ///send_str(fd, "END\n");
    send_end(fd);
}