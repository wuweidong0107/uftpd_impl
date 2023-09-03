#include "uftpd.h"

int open_socket(int port, int type, const char *desc)
{
    int sd, err, val = 1;
    socklen_t len = sizeof(struct sockaddr);
    struct sockaddr_in server;

    sd = socket(AF_INET, type | SOCK_NONBLOCK, 0);
    if (sd < 0) {
        ERR(errno, "Failed creating %s server socket", desc);
        return -1;
    }

    err = setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val));
    if (err != 0)
        WARN(errno, "Failed setting SO_REUSEADDR on %s socket", desc);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    if (bind(sd, (struct sockaddr *)&server, len) < 0) {
        if (EACCES != errno) {
            WARN(errno, "Failed binding to port %d, maybe another %s server is already running", port, desc);
        }
        close(sd);
        return -1;
    }
    return sd;
}

int set_nonblock(int fd)
{
    int flags;
    flags = fcntl(fd, F_GETFL, 0);
    if (!flags)
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    return fd;
}

static void inactivity_cb(uev_t *w, void *arg, int events)
{
    uev_ctx_t *ctx = (uev_ctx_t *)arg;
    INFO("Inactivity timer, exiting...");
    uev_exit(ctx);
}

session_t* new_session(uev_ctx_t *ctx, int sd, int *rc)
{
    session_t *session = NULL;
    pid_t pid;
    
    pid = fork();
    if (pid) {
        INFO("Created new client session as PID %d", pid);
        *rc = pid;
        return NULL;
    }

    ctx = calloc(1, sizeof(uev_ctx_t));
    if (!ctx) {
        ERR(errno, "Failed allocating session event context");
        exit(1);
    }

    uev_init(ctx);

    session = calloc(1, sizeof(session_t));
    if (!session) {
        ERR(errno, "Failed allocating session context");
        goto fail;
    }
    session->sd = set_nonblock(sd);
    session->ctx = ctx;

    uev_timer_init(ctx, &session->timeout_watcher, inactivity_cb, ctx, INACTIVITY_TIMER, 0);
    return session;
fail:
    if (ctx)
        free(ctx);
    *rc = -1;
    return NULL;
}

int del_session(session_t *session)
{
    if (!session)
        return -1;

    if (session->ctx)
        free(session->ctx);

    free(session);
    return 0;
}