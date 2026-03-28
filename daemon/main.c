#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <syslog.h>

#include "server.h"
#include "util.h"

int main(int argc, char **argv) {

    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        debug = 1;
    }

    if (!debug) {
        openlog("netagentd", LOG_PID | LOG_NDELAY, LOG_DAEMON);

        if (daemon(0, 0) < 0) {
            perror("daemon");
            exit(1);
        }
    }

    int rc = server_run(debug);

    if (!debug)
        closelog();

    return rc;
}
