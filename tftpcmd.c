#include "uftpd.h"

void tftp_read_cb(uev_t *w, void *arg, int events)
{
    session_t *session = (session_t *)arg;
    ssize_t len;
    struct sockaddr *addr = (struct sockaddr *)&session->client_sa;
    socklen_t addr_len = sizeof(session->client_sa);

    /* Reset inactivity timer. */
	uev_timer_set(&session->timeout_watcher, INACTIVITY_TIMER, 0);

    INFO("Reading tftp...");
    len = recvfrom(session->sd, &session->msg, sizeof(session->msg), 0, addr, &addr_len);
    if (-1 == len) {
        if (errno != EINTR)
            ERR(errno, "Failed reading command/status from client");
        uev_exit(w->ctx);
        return;
    }
    INFO("Reading tftp, %d bytes...", len);
}

int tftp_session(uev_ctx_t *ctx, int sd)
{
    int pid = 0, ret;
    session_t *session;

    session = new_session(ctx, sd, &pid);
    if (!session) {
        // parent
        return pid;
    }
    
    // child process
    uev_io_init(session->ctx, &session->io_watcher, tftp_read_cb, session, session->sd, UEV_READ);
    uev_run(session->ctx, 0);

    ret = del_session(session);
    exit(ret);
}