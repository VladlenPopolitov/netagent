// netagentd - minimal daemon with key=value protocol (v=1)

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

// for fork/exec/wait
#include <sys/wait.h>
#include <ctype.h>

// system log output 
#include <syslog.h>

// access only for group netagent
#include <grp.h>

// safe call to external program and read output
#include "exec_safe.h"

// request format
#include "request.h"

// peer credentials
#include "auth.h"
#include "policy.h"

// timeout for read from socket
#include <sys/time.h>

// protocal functions
#include "protocol.h"

// for SIGTERM handling
#include <signal.h>


int debug = 1;


// allow only safe interface names: wlan0, em0, etc.
static int is_valid_iface(const char *s) {
    if (!s || !*s) return 0;
    for (; *s; s++) {
        if (!isalnum((unsigned char)*s))
            return 0;
    }
    return 1;
}

// minimal SSID validation (no spaces version for now)
static int is_valid_ssid(const char *s) {
    if (!s || !*s) return 0;
    for (; *s; s++) {
        if (*s == '"' || *s == '\'' || *s == '\n')
            return 0;
    }
    return 1;
}

static int run_exec(const char *path, char *const argv[]) {
    if (debug) 
    {
        fprintf(stderr,"%s:",path);
        char *const *argvv=argv;
        while (*argvv!=NULL)
        {
            fprintf(stderr,"%s ",*argvv);
            argvv++;
        }
        fprintf(stderr,"\n");
    }
    
    pid_t pid = fork();
    if (pid < 0) return -1;

    if (pid == 0) {
        execv(path, argv);
        _exit(127); // exec failed
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
        return -1;
    fprintf(stderr, "status raw=%d\n", status);
    fprintf(stderr, "WIFEXITED=%d WIFSIGNALED=%d\n",
        WIFEXITED(status), WIFSIGNALED(status));

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return -1;
}



static void handle_status(int fd, struct request *req) {
    (void)req;
    
    ///dprintf(fd, "OK status=running\n");
    send_ok(fd);
    send_data(fd, "status", "running");
    send_end(fd);

    if (!debug)
        syslog(LOG_INFO, "OK status=running.");
}

static void handle_list_interfaces(int fd, struct request *req) {
    (void)req;

    char buf[MAX_OUTPUT];
    char *args[] = { "ifconfig", "-l", NULL };

    if (exec_capture_output("/sbin/ifconfig", args, buf, sizeof(buf)) != 0) {
        send_error(fd, "EIO", "ifconfig_failed");
        return;
    }

    if (send_ok(fd) < 0) {
        syslog(LOG_WARNING, "write failed");
    }

    char *tok = strtok(buf, " \n");
    while (tok) {
        ///dprintf(fd, "DATA iface=%s\n", tok);
        send_data(fd, "iface", tok);
        tok = strtok(NULL, " \n");
    }

    ///send_str(fd, "END\n");
    send_end(fd);
}



int main(int argc, char **argv) {
    int client_fd;
    struct sockaddr_un addr;
    int retcode = 0;

    if (argc > 1 && strcmp(argv[1], "-d") == 0) {
        debug = 1;
    } else {
        debug = 0;
    }

    signal(SIGTERM, handle_sigterm);
    signal(SIGINT, handle_sigterm); // for Ctrl+C

    /* use syslog for output information */
    if (!debug) {
        openlog("netagentd", LOG_PID | LOG_NDELAY, LOG_DAEMON);
    }
    if (!debug) {
        if (daemon(0, 0) < 0) {
            perror("daemon");
            exit(1);
        }
    }
    unlink(SOCK_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCK_PATH);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        if (!debug)
            syslog(LOG_ERR, "bind failed: %s", strerror(errno));
        else
            perror("bind");
        exit(1);
    }

    chmod(SOCK_PATH, 0660);
    {
        struct group *grp = getgrnam("netagent");
        if (!grp) {
            fprintf(stderr, "Group netagent not found\n");
            retcode = -1;
            goto exitlabel;
            
        }
        chown(SOCK_PATH, 0, grp->gr_gid);
    }

    if (listen(server_fd, 5) < 0) {
        perror("listen");
        retcode = -1;
        goto exitlabel;
        
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

        struct xucred cred;

        if (get_peer_cred(client_fd, &cred) != 0) {
            send_error(client_fd, "EIO", "cred_failed");
            close(client_fd);
            continue;
        }

        gid_t netagent_gid;
        if (get_netagent_gid(&netagent_gid) != 0) {
            send_error(client_fd, "EIO", "group_not_found");
            close(client_fd);
            continue;
        }

        if (!cred_in_group(&cred, netagent_gid)) {
            send_error(client_fd, "EPERM", "not_authorized");
            close(client_fd);
            continue;
        }

        struct timeval tv;
        tv.tv_sec = 5;   // 5 seconds
        tv.tv_usec = 0;

        setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        setsockopt(client_fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

        char buf[MAX_LINE];
        ssize_t n = read(client_fd, buf, sizeof(buf) - 1);

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                send_error(client_fd, "ETIMEOUT", "read_timeout");
            } else {
                send_error(client_fd, "EIO", "read_error");
            }
            close(client_fd);
            continue;
        }

        if (n > 0) {
            buf[n] = '\0';

            struct request req;
            parse_request(buf, &req);

            // authorize user
            if (!authorize(&cred, &req)) {
                send_error(client_fd, "EPERM", "not_authorized");
                close(client_fd);
                continue;
            }
            dispatch(client_fd, &req);
        }
    }
    
    exitlabel:
    close(server_fd);
    unlink(SOCK_PATH);

    if (debug) 
        printf("netagentd stopped\n");
    else
        syslog(LOG_INFO, "netagentd stopped");


    if (!debug)
        closelog();

    return retcode;
}
