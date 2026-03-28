#include "server.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <memory.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>

// access only for group netagent
#include <grp.h>

// system log output 
#include <syslog.h>

// peer credentials
#include "auth.h"
#include "policy.h"

#include "protocol.h"
#include "handlers.h"

#define SOCK_PATH "/var/run/netagent.sock"
#define MAX_LINE 4096

static volatile sig_atomic_t stop_flag = 0;
static int server_fd = -1;


static void handle_sigterm(int sig) {
    (void)sig;
    stop_flag = 1;

    if (server_fd >= 0) {
        close(server_fd);  // to wakeup accept()
    }
}

static int setup_socket(void) {
    struct sockaddr_un addr;

    unlink(SOCK_PATH);

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0)
        return -1;

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_PATH);

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(fd);
        return -1;
    }

    chmod(SOCK_PATH, 0660);

    struct group *grp = getgrnam("netagent");
    if (!grp) {
        close(fd);
        return -1;
    }

    chown(SOCK_PATH, 0, grp->gr_gid);

    if (listen(fd, 5) < 0) {
        close(fd);
        return -1;
    }

    return fd;
}

static void setup_timeout(int fd) {
    struct timeval tv = { .tv_sec = 5, .tv_usec = 0 };

    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
}

static void handle_client(int client_fd) {
    struct xucred cred;

    if (get_peer_cred(client_fd, &cred) != 0) {
        send_error(client_fd, "EIO", "cred_failed");
        return;
    }

    setup_timeout(client_fd);

    char buf[MAX_LINE];
    ssize_t n = read(client_fd, buf, sizeof(buf) - 1);

    if (n < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            send_error(client_fd, "ETIMEOUT", "read_timeout");
        else
            send_error(client_fd, "EIO", "read_error");
        return;
    }

    if (n == 0)
        return;

    buf[n] = '\0';

    struct request req;
    parse_request(buf, &req);

    if (!authorize(&cred, &req)) {
        send_error(client_fd, "EPERM", "not_authorized");
        return;
    }

    dispatch(client_fd, &req);
}

int server_run(int debug) {
    int client_fd;

    signal(SIGTERM, handle_sigterm);
    signal(SIGINT, handle_sigterm);

    server_fd = setup_socket();
    if (server_fd < 0) {
        perror("setup_socket");
        return 1;
    }

    if (debug)
        printf("netagentd started\n");
    else
        syslog(LOG_INFO, "netagentd started");

    while (!stop_flag) {
        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            if (stop_flag) break;
            if (errno == EINTR) continue;
            perror("accept");
            break;
        }

        handle_client(client_fd);
        close(client_fd);
    }

    close(server_fd);
    unlink(SOCK_PATH);

    return 0;
}
