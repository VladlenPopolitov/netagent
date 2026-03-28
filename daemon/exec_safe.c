#include "exec_safe.h"
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>

int exec_capture_output(const char *path, char *const argv[], char *outbuf, size_t outlen) {
    int pipefd[2];
    if (pipe(pipefd) < 0) return -1;

    pid_t pid = fork();
    if (pid < 0) return -1;

    if (pid == 0) {
        // дочерний процесс
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        execve(path, argv, NULL);
        _exit(127);
    }

    // родительский процесс
    close(pipefd[1]);
    ssize_t n = read(pipefd[0], outbuf, outlen-1);
    close(pipefd[0]);

    int status;
    waitpid(pid, &status, 0);

    if (n >= 0) outbuf[n] = '\0';
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) return -1;

    return 0;
}

int run_exec(const char *path, char *const argv[]) {
    pid_t pid = fork();
    if (pid < 0)
        return -1;

    if (pid == 0) {
        execv(path, argv);
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
        return -1;

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return -1;
}
